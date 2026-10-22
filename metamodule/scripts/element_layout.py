"""Write the element groups, names, and order of each module in plugin-mm.json.

  python3 element_layout.py SIMULATOR [SLUG ...]

Run it from the simulator run directory (see ../test/README.md). It reads each module's
elements with the simulator's --dump-elements, then sets the module's entry in plugin-mm.json:
  - "groups": a "Mod N" group for each modulation input, with its "Show Mod N" button
    first, then the "ModN to X" params in the order of their knobs, then the "Mod N In" jack
  - "names": "X" for each "ModN to X" param, "Mod N In" for each modulation input jack,
    "X Mod Display" for the display behind the X knob (which draws its mod depth arcs),
    "LCD Display" for the one other display, and shorter names for some long jack names
  - "order": the panel's knobs and sliders, then its switches, then the params on the LCD,
    each in reading order, then the Mod groups, then the jacks in reading order, then the
    LCD Display. Everything else follows, in the module's own order.
Elements are given by name, or by index ("param:N", "in:N", "out:N") if another element
has the same name. Other entries in plugin-mm.json are kept.
"""
import json, os, re, subprocess, sys
from collections import namedtuple

HERE = os.path.dirname(os.path.abspath(__file__))
PLUGIN_MM_JSON = os.path.join(HERE, '..', 'plugin-mm.json')

MOD_PARAM = re.compile(r'^Mod ?(\d+) to (.+)$')
SHOW_MOD = re.compile(r'^Show Mod (\d+)$')
# The modules name their modulation inputs differently (and QuadAD and QuadLFO count from 0)
MOD_JACK = re.compile(r'^(Modulation Signal|Modulator|Mod Input|Mod) \d+( In)?$')

SHORT_NAMES = [
    (re.compile(r'^Left \(or Mono merged\) Out$'), 'Left/Mono Out'),
]

ELEMENT = re.compile(r'^\[\s*\d+\] (\S+)\s+x=\s*(\S+) y=\s*(\S+) w=\s*(\S+) h=\s*(\S+) "([^"]*)"'
                     r'(?: (param|input|output|light)=(\d+))?')

DISPLAY = 'DynamicGraphicDisplay'
KNOBS = {'Knob', 'KnobSnapped', 'Slider'}
SWITCHES = {'FlipSwitch', 'SlideSwitch'}
JACKS = {'JackInput', 'JackOutput'}
REF_PREFIX = {'param': 'param', 'input': 'in', 'output': 'out', 'light': 'light'}

# Elements whose centers are closer than this vertically are in the same row
ROW_MM = 5


class Element(namedtuple('Element', 'kind x y w h name id_kind id')):
    @property
    def center(self):
        return self.x + self.w / 2, self.y + self.h / 2

    def inside(self, other):
        cx, cy = self.center
        return other.x <= cx <= other.x + other.w and other.y <= cy <= other.y + other.h


def elements_of(sim, slug):
    r = subprocess.run([sim, '--dump-elements', f'SurgeXTRack:{slug}'], capture_output=True, text=True)
    elements = []
    for m in map(ELEMENT.match, r.stdout.splitlines()):
        if m:
            kind, x, y, w, h, name, id_kind, id = m.groups()
            elements.append(Element(kind, float(x), float(y), float(w), float(h), name, id_kind,
                                    None if id is None else int(id)))
    if r.returncode != 0 or not elements:
        sys.exit(f'{slug}: simulator --dump-elements failed ({r.returncode})\n{r.stderr[-2000:]}')
    return elements


def reading_order(elements):
    rows = []
    for el in sorted(elements, key=lambda el: el.center[1]):
        if rows and el.center[1] - rows[-1][0].center[1] < ROW_MM:
            rows[-1].append(el)
        else:
            rows.append([el])
    return [el for row in rows for el in sorted(row, key=lambda el: el.center[0])]


def layout(elements):
    # The firmware matches names ignoring case
    count = {}
    for el in elements:
        count[el.name.lower()] = count.get(el.name.lower(), 0) + 1

    def ref(el):
        if count[el.name.lower()] == 1 or el.id is None:
            return el.name
        return f'{REF_PREFIX[el.id_kind]}:{el.id}'

    # The mod params of a knob are at its center, and so is the display behind it
    mod_targets = {}
    for el in elements:
        m = MOD_PARAM.match(el.name)
        if m and el.id_kind == 'param':
            mod_targets.setdefault(el.center, m.group(2))

    def mod_target(el):
        cx, cy = el.center
        return next((t for (x, y), t in mod_targets.items() if abs(x - cx) < 0.05 and abs(y - cy) < 0.05), None)

    shows = {}
    mod_params = {}
    names = {}
    lcds = [el for el in elements if el.kind == DISPLAY and mod_target(el) is None]
    for el in elements:
        m = MOD_PARAM.match(el.name)
        show = SHOW_MOD.match(el.name)
        if m and el.id_kind == 'param':
            mod_params.setdefault(f'Mod {m.group(1)}', []).append(el)
            names[ref(el)] = m.group(2)
        elif show and el.id_kind == 'param':
            shows[f'Mod {show.group(1)}'] = el
        elif el.kind == DISPLAY and (target := mod_target(el)):
            names[ref(el)] = f'{target} Mod Display'
        elif el.kind == DISPLAY and len(lcds) == 1:
            names[ref(el)] = 'LCD Display'
        else:
            for pattern, short in SHORT_NAMES:
                if pattern.match(el.name):
                    names[ref(el)] = pattern.sub(short, el.name)

    # Mod N's input is the Nth one
    mod_jacks = sorted((el for el in elements if el.kind == 'JackInput' and MOD_JACK.match(el.name)),
                       key=lambda el: el.id)
    mod_jacks = {f'Mod {n}': el for n, el in enumerate(mod_jacks, 1)}

    groups = {}
    for group in sorted(shows.keys() | mod_params.keys()):
        members = [shows[group]] if group in shows else []
        members += reading_order(mod_params.get(group, []))
        if jack := mod_jacks.get(group):
            members.append(jack)
            names[ref(jack)] = f'{group} In'
        groups[group] = [ref(el) for el in members]

    grouped = {r for members in groups.values() for r in members}
    params = [el for el in elements if el.id_kind == 'param' and ref(el) not in grouped]
    on_lcd = [el for el in params if any(el.inside(lcd) for lcd in lcds)]
    on_panel = [el for el in params if el not in on_lcd]

    order = []
    order += [ref(el) for el in reading_order([el for el in on_panel if el.kind in KNOBS])]
    order += [ref(el) for el in reading_order([el for el in on_panel if el.kind not in KNOBS])]
    order += [ref(el) for el in reading_order(on_lcd)]
    order += sorted(groups)
    jacks = [el for el in elements if el.kind in JACKS and ref(el) not in grouped]
    order += [ref(el) for el in reading_order(jacks)]
    order += [ref(el) for el in lcds if len(lcds) == 1]

    return groups, names, order


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
        groups, display, order = layout(elements_of(sim, slug))
        for key, val in (('groups', groups), ('names', display), ('order', order)):
            if val:
                module[key] = val
            else:
                module.pop(key, None)
        print(f'{slug}: {len(groups)} groups, {sum(map(len, groups.values()))} grouped, {len(display)} names')

    with open(PLUGIN_MM_JSON, 'w') as f:
        f.write(dump(data))


if __name__ == '__main__':
    main()
