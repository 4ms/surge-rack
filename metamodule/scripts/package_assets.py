#!/usr/bin/env python3
"""Stage panel assets and upstream notices for an offline MetaModule ARM build."""
import argparse
import gzip
import hashlib
import io
import json
import re
import shutil
import subprocess
import tarfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OWNER = 'surge-rack-metamodule-package-assets-v1'
MANIFEST = 'package-assets.json'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def json_bytes(value):
    return (json.dumps(value, indent=2, sort_keys=True) + '\n').encode('utf-8')


def collect_notices(sdk):
    # Copy these files byte-for-byte. Identical files share one archive entry.
    required = [
        ('repository', 'LICENSE'), ('repository', 'surge/LICENSE'),
        ('repository', 'surge/libs/sst/sst-basic-blocks/LICENSE.md'),
        ('repository', 'surge/libs/sst/sst-effects/LICENSE.md'),
        ('repository', 'surge/libs/sst/sst-filters/LICENSE.md'),
        ('repository', 'surge/libs/sst/sst-waveshapers/LICENSE.md'),
        ('repository', 'surge/libs/eurorack/eurorack/stmlib/LICENSE'),
        ('repository', 'surge/libs/simde/COPYING'),
        ('repository', 'surge/libs/libsamplerate/COPYING'),
        ('repository', 'surge/libs/fmt/LICENSE.rst'),
        ('repository', 'res/xt/fonts/quicksand/SIL Open Font License.txt'),
        ('sdk', 'LICENSE.md'), ('sdk', 'LICENSE-GPLv3.txt'),
        ('sdk', 'cpputil/LICENSE'), ('sdk', 'plugin-libc/LICENSE'),
        ('sdk', 'rack-interface/LICENSE.md'), ('sdk', 'rack-interface/LICENSE-GPLv3.txt'),
        ('sdk', 'rack-interface/LICENSE-dist.md'),
    ]
    roots = {'repository': ROOT, 'sdk': sdk}
    contents, entries = {}, []

    def add(kind, source, data):
        if not data.strip():
            raise ValueError(f'Empty notice: {source}')
        digest = sha(data)
        target = f'texts/{digest}.txt'
        contents[target] = data
        entries.append(dict(kind=kind, source=source, file=target, sha256=digest))

    for origin, relative in required:
        add('license-file', f'{origin}/{relative}', (roots[origin] / relative).read_bytes())
    readme = (ROOT / 'README.md').read_text(encoding='utf-8')
    section = readme[readme.index('## License and Copyright'):].split('\n## ', 1)[0]
    add('repository-attribution', 'repository/README.md#license-and-copyright', section.encode('utf-8'))

    # Include leading copyright/license notices from the dependency trees used
    # by this build, including their per-file MIT/BSD/GPL acknowledgments. This
    # is deliberately broader than the linked symbols; it is not an SBOM.
    scan_roots = [
        ('repository', 'surge/src/common/dsp'),
        *[('repository', f'surge/libs/sst/{name}/include') for name in
          ('sst-basic-blocks', 'sst-effects', 'sst-filters', 'sst-waveshapers')],
        ('repository', 'surge/libs/eurorack/eurorack/plaits'),
        ('repository', 'surge/libs/eurorack/eurorack/clouds'),
        ('repository', 'surge/libs/libsamplerate/src'),
        ('repository', 'surge/libs/eurorack/eurorack/stmlib'),
        ('repository', 'surge/libs/simde/simde'), ('repository', 'surge/libs/fmt/include'),
        ('sdk', 'core-interface'), ('sdk', 'rack-interface/include'),
        ('sdk', 'cpputil'), ('sdk', 'plugin-libc/glue'),
    ]
    for origin, relative in scan_roots:
        directory = roots[origin] / relative
        if not directory.is_dir():
            raise ValueError(f'Missing notice source tree: {directory}')
        for path in sorted(directory.rglob('*')):
            if path.suffix not in ('.h', '.hpp', '.cpp', '.cc', '.c'):
                continue
            data = path.read_bytes()
            match = re.match(rb'\s*((?:(?://[^\n]*(?:\n|$))|(?:/\*[\s\S]*?\*/)|\s)+)', data)
            if match and re.search(rb'copyright|license|permission', match[1], re.I):
                add('source-preamble', f'{origin}/{path.relative_to(roots[origin]).as_posix()}', match[1].strip())
    if not any('plaits/dsp/voice.h' in e['source'] for e in entries):
        raise ValueError('Plaits copyright notice was not collected')
    entries.sort(key=lambda item: (item['source'], item['kind']))
    contents['index.json'] = json_bytes(dict(format=1, entries=entries))
    contents['README.txt'] = (
        'Surge XT MetaModule upstream notices\n\n'
        'index.json maps upstream source paths to the verbatim license files or\n'
        'leading source notices in texts/. Identical byte sequences are stored once.\n'
        'repository/ paths refer to the surge-rack checkout; sdk/ paths refer to\n'
        'the SDK used for this build. SDK notices include its upstream notice bundle.\n'
        'Source preambles cover dependency trees, not only linked symbols. This\n'
        'collection is not a corresponding-source archive or a completed release audit.\n'
    ).encode('utf-8')
    output = io.BytesIO()
    with gzip.GzipFile(fileobj=output, mode='wb', compresslevel=9, mtime=0) as compressed:
        with tarfile.open(fileobj=compressed, mode='w', format=tarfile.USTAR_FORMAT) as archive:
            for name, data in sorted(contents.items()):
                entry = tarfile.TarInfo(name)
                entry.size = len(data)
                entry.mode = 0o644
                entry.mtime = 0
                archive.addfile(entry, io.BytesIO(data))
    return output.getvalue(), len(entries), len(contents) - 2


def stage(sdk, build_dir):
    sdk, build_dir = sdk.resolve(strict=True), build_dir.resolve(strict=True)
    output = build_dir / 'surge-mm-package-assets'
    # Resolve and validate the exact managed directory before any recursive
    # removal. Never remove an unrelated pre-existing directory or follow a link.
    if output.is_symlink() or output.resolve().parent != build_dir:
        raise ValueError(f'Unsafe package asset directory: {output}')
    if output.exists():
        owner = output / MANIFEST
        if not owner.is_file() or json.loads(owner.read_text(encoding='utf-8')).get('owner') != OWNER:
            raise ValueError(f'Refusing to replace unmanaged directory: {output}')

    # Read and validate inputs before replacing the previous staging directory.
    notices, count, unique = collect_notices(sdk)
    assets = {p.relative_to(ROOT / 'metamodule/assets').as_posix(): p.read_bytes()
              for p in sorted((ROOT / 'metamodule/assets').rglob('*')) if p.is_file()}
    if any(name in assets for name in (MANIFEST, 'NOTICES.txt', 'licenses.tar.gz')):
        raise ValueError('Generated notice names collide with source assets')
    assets['licenses.tar.gz'] = notices
    assets['NOTICES.txt'] = (
        'Surge XT Native for MetaModule - experimental port\n\n'
        'Code: Surge Synth Team and contributors, GPL-3.0-or-later, with dependency\n'
        'terms retained in licenses.tar.gz. The archive includes full license texts,\n'
        'SDK notices and source copyright/license preambles, with an index.\n\n'
        'Artwork: original res/xt SVGs copyright Steve and Pierre, distributed by\n'
        'surge-rack under Creative Commons Attribution-NonCommercial-ShareAlike 4.0.\n'
        'Source: https://github.com/surge-synthesizer/surge-rack/tree/main/res/xt\n'
        'Terms: https://creativecommons.org/licenses/by-nc-sa/4.0/\n'
        'MetaModule adaptations rasterize the original artwork and alter labels,\n'
        'control placement and CV layouts. These artwork adaptations retain\n'
        'CC BY-NC-SA 4.0. This artwork license is distinct from the code license.\n\n'
        'Quicksand: copyright 2011 Andrew Paglinawan, SIL Open Font License 1.1;\n'
        'its original license and attribution are included in licenses.tar.gz.\n\n'
        'Source project: https://github.com/surge-synthesizer/surge-rack\n'
        'Native source, adapters and build instructions: metamodule/ in the working\n'
        'checkout. This experimental local build may contain unpublished changes.\n'
        'package-assets.json records local source fingerprints; these fingerprints\n'
        'do not substitute for a corresponding-source release.\n'
    ).encode('utf-8')
    fingerprints = {}
    for path in sorted((ROOT / 'metamodule').rglob('*')):
        if path.is_file() and (path.suffix in ('.h', '.hh', '.cc', '.cpp', '.py', '.json', '.md', '.yml') or path.name in ('CMakeLists.txt', 'converter-requirements.txt')):
            fingerprints[path.relative_to(ROOT).as_posix()] = sha(path.read_bytes())
    revisions = {}
    for name, path in [('repository', ROOT), ('surge', ROOT / 'surge'), ('sdk', sdk)]:
        revisions[name] = subprocess.check_output(['git', '-C', str(path), 'rev-parse', 'HEAD'], text=True).strip()
    manifest = dict(owner=OWNER, format=1, files={n: sha(b) for n, b in sorted(assets.items())},
                    source_fingerprints=fingerprints, revisions=revisions,
                    notice_entries=count, unique_notice_texts=unique)
    if output.exists():
        shutil.rmtree(output)
    output.mkdir()
    (output / MANIFEST).write_bytes(json_bytes({'owner': OWNER}))
    for name, data in assets.items():
        target = output / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    (output / MANIFEST).write_bytes(json_bytes(manifest))
    print(f'Staged {len(assets)} assets; {count} notice entries, {len(notices)} compressed notice bytes')
    return output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', required=True, type=Path)
    parser.add_argument('--build-dir', required=True, type=Path)
    args = parser.parse_args()
    stage(args.sdk, args.build_dir)


if __name__ == '__main__':
    main()
