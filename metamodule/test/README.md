# MetaModule test patches

Patches for checking the Surge modules in the MetaModule headless simulator
(and on hardware). Each feeds In 1/In 2 to the module and Out 1/Out 2 from it.

| Patch | Checks |
|---|---|
| `chorus.yml` | Chorus with its default (first preset) settings |
| `chorus_mix0.yml` | Mix at 0: output is the input, delayed by 7 samples |
| `chorus_mod1_rate.yml` | In 2 into Mod 1, Mod 1 to Rate at full depth |
| `chorus_rack_state.yml` | Module state as the Rack plugin saves it (`vcvModuleStates`) |
| `unison.yml` | UnisonHelper driving a Sine VCO as its sub VCO 1 (Out 1), and a CV Expander on its right (an expander connection) spreading In 2 (Out 2) |

## Building the simulator with this plugin

The surge-rack MetaModule plugins need v2.4.1-surge001 or later of the MetaModule firmware repo, which
contains some widget fixes and the simulator's `--dump-elements` option.

Clone the MetaModule firmware repo and check out a tag at least v2.4.1-surge001:
```
git clone https://github.com/4ms/metamodule
cd metamodule
git checkout v2.4.1-surge001
git submodule update --init --recursive
```

From the metamodule repo, build the headless simulator:
```
cd simulator
cmake --fresh --preset headless -DLOG_LEVEL=DEBUG \
    -DEXT_BUILTIN_BRAND_PATHS=<surge-rack>/metamodule \
    -DEXT_BUILTIN_BRAND_LIBNAMES=SurgeXT \
    -DEXT_BUILTIN_BRAND_SLUGS=SurgeXTRack
cmake --build build-headless
```

## Running

Once the simulator is built, for modules that don't load wavetables from the
filesystem, you can run a patch and get the output like this:

```
cd <surge-rack>/metamodule
mkdir tmp && cd tmp

# Generate an input wav file:
python3 ../test/wavtool.py gen sine.wav 2 220 330 0.5

# Run the headless simulator and get an output wav:
<metamodule-repo>/simulator/build-headless/simulator -p ../test/patches/chorus.yml --in sine.wav --out out.wav -n 96000

# Analyze the output wav:
python3 ../test/wavtool.py stats out.wav 0.5
```

If you need to test modules like Wavetable VCO which read files from the filesystem,
then you have to link or copy them to `SurgeXTRack/build/surge-data/` relative to the current
directory, as is shown in the next example.

Often you will want to do a "smoke" test on all the modules (i.e. do the
modules even run without crashing or generating NaN/inf). Do it like this:

```
cd <surge-rack>/metamodule
mkdir tmp && cd tmp

# Link the assets (wavetables) into the current dir
ln -s <metamodule-repo>/simulator/build-headless/builtins/SurgeXT/assets SurgeXTRack

# Run a patch for each module in the headless simulator:
python3 ../test/check-modules.py <metamodule-repo>/simulator/build-headless/simulator [SLUG ...]

# And/or run the headless simulator and get an output wav (see previous example):
<metamodule-repo>/simulator/build-headless/simulator -p ../test/patches/unison.yml --in sine.wav --out out.wav -n 96000
```

check-modules.py runs a stereo sine into each FX, runs each VCO unpatched, and
runs the other modules in specific ways (e.g. the envelopes get a gate). It
fails a module whose output is silent or has NaN/inf.

To print a module's elements (what the MetaModule shows and can map), and its heap use:

```
<metamodule-repo>/simulator/build-headless/simulator --dump-elements SurgeXTRack:SurgeXTFXChorus
```

The element groups and names in `plugin-mm.json` (a "Mod N" group of the "ModN to X" params for
each modulation input) are written from these dumps. After adding modules, regenerate them and
check them, from the same directory:

```
cd <surge-rack>/metamodule

# Generate plugin-mm.json (calls --dump-elements and parses it):
python3 scripts/element_layout.py <metamodule-repo>/simulator/build-headless/simulator [SLUG ...]

# run the simulator's checks on the plugin-mm.json
<metamodule-repo>/simulator/build-headless/simulator --check-element-layout \
    --plugin-mm-json plugin-mm.json \
    --plugin-json ../plugin.json
```
