# Surge XT for MetaModule

This experimental native port includes all 46 modules from Surge XT for VCV
Rack, plus 13 destination-specific modulation matrix variants. Oscillators and
effects expose fixed CV inputs and Rack-style panels. The two Unison wrappers
use the SDK Rack adapter for expander support.

The current development version is 0.1.47, including the Spring Reverb optimization.

For implementation review, start with the [design guide](REVIEW_GUIDE.md) and
[upstream adaptation map](runtime/UPSTREAM_MAP.md). Hardware CPU measurements and
validation results are in the [pull request description](https://github.com/Ericxgao/surge-rack/pull/1).

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

## Compatibility and limits

Most native engines support four voices; Unison has 16 slots across four banks.
See [known issues](UPSTREAM_ISSUES.md) for upstream bugs and intentional behavior
deviations. The [pull request](https://github.com/Ericxgao/surge-rack/pull/1) records
the tested workloads and remaining validation limits. Original DSP licenses and
attribution remain applicable.
