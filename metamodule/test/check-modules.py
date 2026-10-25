"""Check modules: run each module in plugin-mm.json through the headless simulator
   and make sure they don't crash

  python3 check-modules.py SIMULATOR [SLUG ...]

Run it from a directory with a SurgeXTRack link to the plugin's staged assets (see README.md).
In 1/In 2 are a stereo sine, or with gate=True, a 5V sine and a 4Hz 0-10V gate. By default
each FX gets them on its L/R inputs, other modules run with nothing patched, and Out 1/Out 2
come from the module's first two outputs; SETUPS changes that per module. A module fails if
the simulator fails, if its output has NaN/inf, or if it's silent (unless silent_ok).
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
{outs}
  static_knobs:
{knobs}
  mapped_knobs: []
  midi_maps:
    set: []
  midi_poly_num: 0
  valid_knob_set_names: 0
  vcvModuleStates: []
"""

STEREO = {0: [0], 1: [1]}

# slug: how it differs from the default setup: ins (In 1/In 2 to these module inputs), outs
# (module outputs for Out 1/Out 2), knobs (param: value), gate (use the gate input), silent_ok
SETUPS = {
    'SurgeXTFXVocoder': dict(ins={0: [0, 2], 1: [1, 3]}),  # inputs 2/3 are SIDEBAND_L/R
    'SurgeXTVCF': dict(ins=STEREO),
    'SurgeXTWaveshaper': dict(ins=STEREO),
    'SurgeXTDelay': dict(ins=STEREO, knobs={9: 0.5}),  # Mix 50%: the 1s delay is the whole test
    'SurgeXTDelayLineByFreq': dict(ins=STEREO),
    'SurgeXTDelayLineByFreqExpanded': dict(ins=STEREO),
    'SurgeXTMixer': dict(ins=STEREO),
    'SurgeXTMixerSlider': dict(ins=STEREO),
    'SurgeXTDigitalRingMod': dict(ins={0: [0, 2], 1: [1, 3]}),  # A and B of CXOR 1
    'SurgeXTEGxVCA': dict(ins={0: [0, 1], 1: [2]}, outs=[0, 2], gate=True),  # Out 2: envelope
    'SurgeXTQuadAD': dict(ins={1: [0, 1, 2, 3]}, gate=True),
    'SurgeXTLFO': dict(ins={1: [0]}, outs=[1, 0], gate=True),  # LFO wave, wave x envelope
    'SurgeXTModMatrix': dict(ins=STEREO, silent_ok=True),  # mod depths are 0 by default
    # These need sub VCOs and a UnisonHelper on the left: see patches/unison.yml
    'SurgeXTUnisonHelper': dict(silent_ok=True),
    'SurgeXTUnisonHelperCVExpander': dict(silent_ok=True),
}


def yaml_knobs(knobs):
    if not knobs:
        return '    []'
    return '\n'.join(f'    - module_id: 1\n      param_id: {p}\n      value: {v}'
                     for p, v in knobs.items())


def yaml_ins(ins):
    if not ins:
        return '    []'
    return '\n'.join(f'    - panel_jack_id: {panel}\n      ins:\n' +
                     '\n'.join(f'        - module_id: 1\n          jack_id: {j}' for j in jacks)
                     for panel, jacks in ins.items())


def yaml_outs(outs):
    return '\n'.join(f'    - panel_jack_id: {panel}\n      out:\n        module_id: 1\n'
                     f'        jack_id: {j}' for panel, j in enumerate(outs))


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

    sr = 48000
    if not os.path.exists('smoke_in.wav'):
        wavtool.write('smoke_in.wav', [(0.5 * math.sin(2 * math.pi * 220 * i / sr),
                                        0.5 * math.sin(2 * math.pi * 330 * i / sr))
                                       for i in range(SAMPLES)])
    if not os.path.exists('smoke_gate.wav'):
        wavtool.write('smoke_gate.wav', [(5 * math.sin(2 * math.pi * 220 * i / sr),
                                          10.0 if (i * 8 // sr) % 2 == 0 else 0.0)
                                         for i in range(SAMPLES)])

    failed = []
    for slug in slugs:
        setup = SETUPS.get(slug, {})
        ins = setup.get('ins', STEREO if slug.startswith('SurgeXTFX') else {})
        outs = setup.get('outs', [0, 1])
        silent_ok = setup.get('silent_ok', False)
        gate = setup.get('gate', False)
        infile = 'smoke_gate.wav' if gate else 'smoke_in.wav'
        with open('smoke.yml', 'w') as f:
            f.write(PATCH.format(name=slug, slug=slug, ins=yaml_ins(ins), outs=yaml_outs(outs),
                                knobs=yaml_knobs(setup.get('knobs'))))
        if os.path.exists('smoke_out.wav'):
            os.remove('smoke_out.wav')

        r = subprocess.run([sim, '-p', 'smoke.yml', '--in', infile, '--out', 'smoke_out.wav',
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
            elif rms < 1e-4 and not silent_ok:
                problems.append(f'ch{c} silent')
        summary = '  '.join(f'ch{c} rms={rms:.4f} peak={peak:.4f}' for c, (rms, peak, _) in enumerate(st))
        print(f'{"FAIL" if problems else "ok  "} {slug:28} {summary}  {"; ".join(problems)}')
        if problems:
            failed.append(slug)

    print(f'\n{len(slugs) - len(failed)}/{len(slugs)} passed')
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
