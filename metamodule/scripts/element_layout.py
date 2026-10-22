"""Write the element groups and names of each module in plugin-mm.json.

  python3 element_layout.py SIMULATOR [SLUG ...]

Run it from the simulator run directory (see ../test/README.md). It reads each module's
elements with the simulator's --dump-elements, then sets the module's entry in plugin-mm.json:
  - "groups": a "Mod N" group for each modulation input, with the "ModN to X" params
  - "names": "X" for each "ModN to X" param, and shorter names for some long jack names
Elements are given by name, or as "param:N" if another element has the same name.
Other entries in plugin-mm.json are kept.
"""
import json, os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
PLUGIN_MM_JSON = os.path.join(HERE, '..', 'plugin-mm.json')

MOD_PARAM = re.compile(r'^Mod(\d+) to (.+)$')

SHORT_NAMES = {
    'Left (or Mono merged) Out': 'Left/Mono Out',
}

ELEMENT = re.compile(r'^\[\s*\d+\] \S+\s.*?"([^"]*)"(?: param=(\d+))?')


def element_names(sim, slug):
    r = subprocess.run([sim, '--dump-elements', f'SurgeXTRack:{slug}'], capture_output=True, text=True)
    elements = [m.groups() for m in map(ELEMENT.match, r.stdout.splitlines()) if m]
    if r.returncode != 0 or not elements:
        sys.exit(f'{slug}: simulator --dump-elements failed ({r.returncode})\n{r.stderr[-2000:]}')
    return elements


def layout(elements):
    # The firmware matches names ignoring case
    count = {}
    for name, _ in elements:
        count[name.lower()] = count.get(name.lower(), 0) + 1

    groups = {}
    display = {}
    for name, param_id in elements:
        ref = name if count[name.lower()] == 1 or param_id is None else f'param:{param_id}'
        m = MOD_PARAM.match(name)
        if m and param_id is not None:
            groups.setdefault(f'Mod {m.group(1)}', []).append(ref)
            display[ref] = m.group(2)
        elif name in SHORT_NAMES:
            display[ref] = SHORT_NAMES[name]
    return dict(sorted(groups.items())), display


def dump(data):
    # One line per group, rather than one line per group member
    text = json.dumps(data, indent=4, ensure_ascii=False)
    return re.sub(r'\[\n\s+("[^\]]*?)\n\s+\]',
                  lambda m: '[' + ', '.join(s.strip() for s in m.group(1).split(',\n')) + ']', text) + '\n'


def main():
    sim = os.path.abspath(sys.argv[1])
    only = set(sys.argv[2:])

    with open(PLUGIN_MM_JSON) as f:
        data = json.load(f)

    for module in data['MetaModuleIncludedModules']:
        slug = module['slug']
        if only and slug not in only:
            continue
        groups, display = layout(element_names(sim, slug))
        for key, val in (('groups', groups), ('names', display)):
            if val:
                module[key] = val
            else:
                module.pop(key, None)
        print(f'{slug}: {len(groups)} groups, {sum(map(len, groups.values()))} grouped, {len(display)} names')

    with open(PLUGIN_MM_JSON, 'w') as f:
        f.write(dump(data))


if __name__ == '__main__':
    main()
