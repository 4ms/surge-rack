# Embedded runtime sources

These production C++ source/header adaptations are based on the frozen
0.1.45 ARM build, with the measured 0.1.47 Spring optimization applied to
`spring/ReflectionNetwork.h` and `spring/SpringReverbProc.h` under `effect-compat`.
They are checked in directly so the compiler inputs are
visible and configuring the plugin needs no source-generation step.
The CMake build still uses the pinned upstream submodules for unmodified DSP.

Upstream Surge revision: `65d381578fe9d896e3cb56e0d44c1f89c12a2cd4`.

| Paths | Origin |
| --- | --- |
| AliasOscillator.cpp, StringOscillator.cpp, TwistOscillator.cpp | surge/src/common/dsp/oscillators |
| effect-compat/*Kernel.h and *Host.h | surge/src/common/dsp/effects and modulators; embedded storage adapters |
| effect-compat/chowdsp and spring | surge/src/common/dsp/effects/chowdsp |
| effect-compat/sst | surge/libs/sst/sst-basic-blocks and sst-effects |
| filter-compat | surge/libs/sst/sst-filters/include |
| plaits-compat | surge/libs/eurorack/eurorack/stmlib/utils |
| table-generated/Wavetable.cpp and WindowOscillator.cpp | surge/src/common/dsp/Wavetable.cpp and dsp/oscillators/WindowOscillator.cpp |
| table-generated/table_resources.cpp | surge/resources/data/wavetables/Basic/Sine To Sawtooth.wt and surge/resources/surge-shared/windows.wt |

Adapters remove desktop-only dependencies and use the native port's storage,
block sizing and parameter interfaces. Existing behavior deviations are listed
in ../UPSTREAM_ISSUES.md. Upstream licenses and attribution remain applicable;
the package includes the original dependency notices. Original copyright and
license headers are preserved where present. Host shims use the repository's
GPL-3.0-or-later license.

The [file-level source map](UPSTREAM_MAP.md) gives exact local upstream paths,
identifies combined/extracted sources and describes the main adaptation areas.
Consult it when updating pinned dependencies. The map does not certify every
change as behavior-neutral. Known deviations require separate review.

See the [PR validation summary](https://github.com/Ericxgao/surge-rack/pull/1#validation-and-limits)
for the relationship between the measured development packages and these sources.
