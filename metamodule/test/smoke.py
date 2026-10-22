"""Smoke test: run each module in plugin-mm.json through the headless simulator.

  python3 smoke.py SIMULATOR [SLUG ...]

Run it from a directory with a SurgeXTRack link to the plugin's staged assets (see README.md).
Each FX gets a stereo sine on its L/R inputs (and the Vocoder also on its sideband/modulator
inputs, without which it's silent); each VCO runs with nothing patched. Out 1/Out 2
come from the module's first two outputs. A module fails if the simulator fails, or if its
output is silent or has NaN/inf.
"""
import json, math, os, subprocess, sys

import wavtool

HERE = os.path.dirname(os.path.abspath(__file__))
SAMPLES = 48000

PATCH = """PatchData:
  patch_name: {name}
  description: SurgeXT smoke test
  module_slugs:
    0: HubMedium
    1: SurgeXTRack:{slug}
  int_cables: []
  mapped_ins:
{ins}
  mapped_outs:
    - panel_jack_id: 0
      out:
        module_id: 1
        jack_id: 0
    - panel_jack_id: 1
      out:
        module_id: 1
        jack_id: 1
  static_knobs: []
  mapped_knobs: []
  midi_maps:
    set: []
  midi_poly_num: 0
  valid_knob_set_names: 0
  vcvModuleStates: []
"""

FX_INS = """    - panel_jack_id: 0
      ins:
        - module_id: 1
          jack_id: 0
    - panel_jack_id: 1
      ins:
        - module_id: 1
          jack_id: 1"""

# FX inputs 2 and 3 are SIDEBAND_L/R
VOCODER_INS = """    - panel_jack_id: 0
      ins:
        - module_id: 1
          jack_id: 0
        - module_id: 1
          jack_id: 2
    - panel_jack_id: 1
      ins:
        - module_id: 1
          jack_id: 1
        - module_id: 1
          jack_id: 3"""


def stats(path):
    frames = wavtool.read(path)[SAMPLES // 4:]  # skip the start (presets loading, filters settling)
    res = []
    for c in range(2):
        x = [f[c] for f in frames]
        bad = sum(1 for v in x if not math.isfinite(v))
        y = [v for v in x if math.isfinite(v)]
        rms = math.sqrt(sum(v * v for v in y) / max(1, len(y)))
        peak = max((abs(v) for v in y), default=0)
        res.append((rms, peak, bad))
    return res


def main():
    sim = os.path.abspath(sys.argv[1])
    slugs = sys.argv[2:]
    if not slugs:
        with open(os.path.join(HERE, '..', 'plugin-mm.json')) as f:
            slugs = [m['slug'] for m in json.load(f)['MetaModuleIncludedModules']]

    if not os.path.exists('smoke_in.wav'):
        sr = 48000
        wavtool.write('smoke_in.wav', [(0.5 * math.sin(2 * math.pi * 220 * i / sr),
                                        0.5 * math.sin(2 * math.pi * 330 * i / sr))
                                       for i in range(SAMPLES)])

    failed = []
    for slug in slugs:
        if slug == 'SurgeXTFXVocoder':
            ins = VOCODER_INS
        elif slug.startswith('SurgeXTFX'):
            ins = FX_INS
        else:
            ins = '    []'
        with open('smoke.yml', 'w') as f:
            f.write(PATCH.format(name=slug, slug=slug, ins=ins))
        if os.path.exists('smoke_out.wav'):
            os.remove('smoke_out.wav')

        r = subprocess.run([sim, '-p', 'smoke.yml', '--in', 'smoke_in.wav', '--out', 'smoke_out.wav',
                            '-n', str(SAMPLES)], capture_output=True, text=True)
        if r.returncode != 0 or not os.path.exists('smoke_out.wav'):
            print(f'FAIL {slug}: simulator exited with {r.returncode}')
            print('\n'.join('    ' + l for l in (r.stdout + r.stderr).splitlines()[-15:]))
            failed.append(slug)
            continue

        st = stats('smoke_out.wav')
        problems = []
        for c, (rms, _, bad) in enumerate(st):
            if bad:
                problems.append(f'ch{c} has {bad} NaN/inf')
            elif rms < 1e-4:
                problems.append(f'ch{c} silent')
        summary = '  '.join(f'ch{c} rms={rms:.4f} peak={peak:.4f}' for c, (rms, peak, _) in enumerate(st))
        print(f'{"FAIL" if problems else "ok  "} {slug:28} {summary}  {"; ".join(problems)}')
        if problems:
            failed.append(slug)

    print(f'\n{len(slugs) - len(failed)}/{len(slugs)} passed')
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
