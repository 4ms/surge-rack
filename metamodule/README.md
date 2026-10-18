# Surge XT for MetaModule

This experimental native port includes all 46 modules from Surge XT for VCV
Rack, plus 13 destination-specific modulation matrix variants. Oscillators and
effects expose fixed CV inputs and Rack-style panels. The two Unison wrappers
use the SDK Rack adapter for expander support.

The current development version is 0.1.47. It includes the Spring Reverb
optimization measured on hardware: the fixed demo dropped from 121.46% to
71.34% mean CPU at 48 kHz / 256 samples, with zero overruns in the final tests.
See the [CPU measurements](hardware/2026-10-18-spring.md) for workload and artifact
identities; these measurements used development packages, not a full PR rebuild.

For implementation review, start with the [design guide](REVIEW_GUIDE.md),
[upstream adaptation map](runtime/UPSTREAM_MAP.md) and
[validation record](VALIDATION.md).

## Building

Embedded compatibility sources are checked in under `runtime/`; Python is used
only for packaging and package verification.

Initialize the repository's pinned submodules and provide the MetaModule plugin
SDK separately. The reviewed ARM build uses GNU Arm Embedded 12.3.Rel1, CMake,
Python 3 and the SDK's build tools. Put the arm-none-eabi tools on PATH. On
Windows, run from an MSYS2 environment:

```sh
git submodule update --init --recursive
cmake -S metamodule -B build/metamodule -G "MSYS Makefiles" \
  -DMETAMODULE_SDK_DIR=/absolute/path/to/metamodule-plugin-sdk
cmake --build build/metamodule --parallel 8
```

The SDK produces SurgeXTNative.mmplugin. Packaging includes panel assets and
upstream license notices, then verifies the package metadata and ARM binary.
SDK/firmware 2.4 supports the named Module View groups used by matrix helpers.

## Rack patches

The matching Surge conversion change in the 4ms MetaModule Hub is required for
Rack imports. Configure knob/jack mappings in Rack and use the Hub's normal
Save or Wi-Fi Send action. The converter maps source module names and state to
the native plugin and inserts visible modulation depth helpers only where
needed. Users do not need to run a Python converter.

Depths remain editable on the helpers. Routes omitted from an export need to
be patched manually or assigned in Rack and exported again. Group labels are
static for each helper type. Reverb imports require the matching 0.1.45 Hub
conversion marker; resend older patches to select the compatibility behavior.

## Existing validation and limits

The frozen 0.1.45 source bundles have clean ARM and Hub build evidence, host
DSP results, and simulator load/control/save-reload coverage for all 46 Rack
module types. The 0.1.45 hardware sweep measured all 46 fixed native demos;
Spring was the only overloaded patch. Development 0.1.47 fixes that measured
case and passes targeted host/audio checks. The current PR organization has
not received a complete ARM build. See [VALIDATION.md](VALIDATION.md) for artifact
identities, the targeted PR-source check and the scope of each result.

The remaining review acceptance check is a physical-device Wi-Fi transfer,
audio and control check, followed by saving and reloading the patch. Most native
engines support four voices; Unison has 16 slots across four banks.

See [known issues](UPSTREAM_ISSUES.md) for upstream bugs and existing intentional
behavior deviations. Further fidelity audits and broad hardware profiling are
follow-up work. Original DSP licenses and attribution remain applicable.
