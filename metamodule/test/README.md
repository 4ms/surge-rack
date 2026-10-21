# MetaModule test patches

Patches for checking the Surge modules in the MetaModule headless simulator
(and on hardware). Each feeds In 1/In 2 to the module and Out 1/Out 2 from it.

| Patch | Checks |
|---|---|
| `chorus.yml` | Chorus with its default (first preset) settings |
| `chorus_mix0.yml` | Mix at 0: output is the input, delayed by 7 samples |
| `chorus_mod1_rate.yml` | In 2 into Mod 1, Mod 1 to Rate at full depth |
| `chorus_rack_state.yml` | Module state as the Rack plugin saves it (`vcvModuleStates`) |

## Building the simulator with this plugin

This needs the `surge-rack-port` branch of the MetaModule firmware repo and of its
`metamodule-plugin-sdk` submodule, which have the Rack interface changes the plugin needs
(params added with `addChild()`, faceplates for panels drawn in code, `rack::event`)
and the simulator's `--dump-elements` option.

```
cmake -S <metamodule>/simulator -B sim-build -G Ninja -DHEADLESS=ON \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DEXT_BUILTIN_BRAND_PATHS=<surge-rack>/metamodule \
    -DEXT_BUILTIN_BRAND_LIBNAMES=SurgeXT \
    -DEXT_BUILTIN_BRAND_SLUGS=SurgeXTRack
cmake --build sim-build
```

Add `-DOMIT_BRAND_<Brand>=ON` for brands you don't need (see the simulator's `headless-min` preset).

## Running

The headless simulator opens files relative to the current directory, and the
modules read their data from `SurgeXTRack/build/surge-data/`, so run it from a
directory with a `SurgeXTRack` link to the plugin's staged assets:

```
mkdir run && cd run
ln -s <sim-build>/builtins/SurgeXT/assets SurgeXTRack
cp <surge-rack>/metamodule/test/patches/*.yml .
python3 <surge-rack>/metamodule/test/wavtool.py gen sine.wav 2 220 330 0.5
<sim-build>/simulator -p chorus.yml --in sine.wav --out out.wav -n 96000
python3 <surge-rack>/metamodule/test/wavtool.py stats out.wav 0.5
```

`wavtool.py` writes and reads the simulator's 32-bit float WAV files (`gen`, `stats`, `diff`).

To print a module's elements (what the MetaModule shows and can map), and its heap use:

```
<sim-build>/simulator --dump-elements SurgeXTRack:SurgeXTFXChorus
```
