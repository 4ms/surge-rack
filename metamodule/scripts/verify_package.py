#!/usr/bin/env python3
"""Check the finished ARM archive, panel/metadata coverage and bundled notices."""
import argparse
import hashlib
import io
import json
import re
import tarfile
from pathlib import Path, PurePosixPath


def require(condition, message):
    if not condition:
        raise ValueError(message)


def verify(data, metadata_dir, assets_dir, expected_version=None):
    with tarfile.open(fileobj=io.BytesIO(data)) as archive:
        files = {}
        seen = set()
        for member in archive.getmembers():
            path = PurePosixPath(member.name)
            require(not path.is_absolute() and '..' not in path.parts and '\\' not in member.name,
                    f'Unsafe archive path: {member.name}')
            require(path.parts and path.parts[0] == 'SurgeXTNative', f'Wrong plugin root: {member.name}')
            require(member.name not in seen, f'Duplicate archive entry: {member.name}')
            seen.add(member.name)
            require(member.isfile() or member.isdir(), f'Unexpected archive member type: {member.name}')
            if member.isfile():
                files[path.relative_to('SurgeXTNative').as_posix()] = archive.extractfile(member).read()

    def read(name):
        require(name in files, f'Missing packaged file: {name}')
        return files[name]

    plugin = json.loads(read('plugin.json'))
    mm = json.loads(read('plugin-mm.json'))
    for name, packed in [('plugin.json', plugin), ('plugin-mm.json', mm)]:
        require(packed == json.loads((metadata_dir / name).read_text(encoding='utf-8')),
                f'Packaged {name} differs from current metadata')
    require(re.fullmatch(r'\d+\.\d+\.\d+', plugin.get('version', '')), 'Invalid plugin release version')
    if expected_version is not None:
        require(plugin['version'] == expected_version,
                f'Packaged version {plugin["version"]} differs from expected version {expected_version}')
    modules = [m['slug'] for m in plugin['modules']]
    require(len(set(modules)) == len(modules), 'Duplicate module slug')
    require(set(modules) == {m['slug'] for m in mm['MetaModuleIncludedModules']}, 'Metadata module lists differ')
    for slug in modules:
        require(read(slug + '.png').startswith(b'\x89PNG\r\n\x1a\n'), f'Missing/invalid panel for {slug}')
    elf = read('SurgeXTNative.so')
    require(elf[:6] == b'\x7fELF\x01\x01' and len(elf) >= 20 and int.from_bytes(elf[18:20], 'little') == 40,
            'Plugin executable is not 32-bit little-endian ARM ELF')
    require(len([name for name in files if re.fullmatch(r'SDK-\d+\.\d+', name)]) == 1, 'Missing/ambiguous SDK version stamp')
    manifest = json.loads(read('package-assets.json'))
    require(manifest == json.loads((assets_dir / 'package-assets.json').read_text(encoding='utf-8')),
            'Packaged asset manifest differs from the staging manifest')
    require(manifest.get('owner') == 'surge-rack-metamodule-package-assets-v1', 'Unknown asset manifest format')
    for name, digest in manifest['files'].items():
        require(hashlib.sha256(read(name)).hexdigest() == digest, f'Asset checksum mismatch: {name}')
    allowed = set(manifest['files']) | {'package-assets.json', 'plugin.json', 'plugin-mm.json', 'SurgeXTNative.so'}
    require(all(name in allowed or name.startswith('presets/') or re.fullmatch(r'SDK-\d+\.\d+', name)
                for name in files), 'Unexpected/stale file in plugin package')
    attribution = read('NOTICES.txt').decode('utf-8')
    for text in ('Steve and Pierre', 'CC BY-NC-SA 4.0', 'Andrew Paglinawan', 'GPL-3.0-or-later'):
        require(text in attribution, f'Missing attribution: {text}')
    with tarfile.open(fileobj=io.BytesIO(read('licenses.tar.gz')), mode='r:gz') as notices:
        members = notices.getmembers()
        require(all(m.isfile() for m in members), 'Non-file member in notice archive')
        require(len(members) == len(set(m.name for m in members)), 'Duplicate notice archive entry')
        require(all(m.name in ('README.txt', 'index.json') or re.fullmatch(r'texts/[a-f0-9]{64}\.txt', m.name)
                    for m in members), 'Unexpected notice archive member')
        def notice_bytes(name):
            return notices.extractfile(name).read()
        index = json.loads(notice_bytes('index.json'))
        sources = {item['source'] for item in index['entries']}
        # Independent coverage gate for the dependency families used by this port.
        for source in (
            'repository/LICENSE', 'repository/surge/LICENSE',
            'repository/surge/libs/sst/sst-basic-blocks/LICENSE.md',
            'repository/surge/libs/sst/sst-effects/LICENSE.md',
            'repository/surge/libs/sst/sst-filters/LICENSE.md',
            'repository/surge/libs/sst/sst-waveshapers/LICENSE.md',
            'repository/surge/libs/eurorack/eurorack/stmlib/LICENSE',
            'repository/surge/libs/eurorack/eurorack/plaits/dsp/voice.h',
            'repository/surge/libs/eurorack/eurorack/clouds/dsp/granular_processor.cc',
            'repository/surge/libs/libsamplerate/COPYING',
            'repository/surge/libs/simde/COPYING', 'repository/surge/libs/fmt/LICENSE.rst',
            'repository/res/xt/fonts/quicksand/SIL Open Font License.txt',
            'sdk/LICENSE.md', 'sdk/cpputil/LICENSE', 'sdk/plugin-libc/LICENSE',
            'sdk/rack-interface/LICENSE.md', 'sdk/rack-interface/LICENSE-dist.md',
        ):
            require(source in sources, f'Missing upstream notice: {source}')
        for item in index['entries']:
            require(re.fullmatch(r'texts/[a-f0-9]{64}\.txt', item['file']), 'Invalid indexed notice path')
            require(hashlib.sha256(notice_bytes(item['file'])).hexdigest() == item['sha256'],
                    f'Notice checksum mismatch: {item["source"]}')
        require(len(index['entries']) == manifest['notice_entries'], 'Notice entry count mismatch')
    return dict(version=plugin['version'], modules=len(modules), notice_entries=len(index['entries']), package_bytes=len(data))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--package', type=Path, required=True)
    parser.add_argument('--metadata-dir', type=Path, required=True)
    parser.add_argument('--assets-dir', type=Path, required=True)
    parser.add_argument('--expected-version', help='Require the configured/release version, in addition to matching source metadata')
    args = parser.parse_args()
    result = verify(args.package.read_bytes(), args.metadata_dir, args.assets_dir, args.expected_version)
    print('Verified ARM package:', json.dumps(result, sort_keys=True))


if __name__ == '__main__':
    main()
