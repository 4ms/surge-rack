# Validation scope

The existing executable evidence applies to the frozen 0.1.45 source bundles.
The PR uses those production sources, with checked-in runtime adaptations,
formatting, explicit mapping names and reorganized build history. **The current
PR tree and intermediate commits have not received a new build or runtime run.**

## Recorded results

| Area | Evidence | What it establishes |
| --- | --- | --- |
| ARM source build | 94 compiled units; 758 discovered inputs; ELF SHA-256 `e7118b380689fe0eb97a8ffdc9fdacecfbdc5a0ee641800f9a1b73f08ee8cfad` | Frozen source bundle builds and matches the delivered ELF |
| Headless firmware simulator | 46 actual Rack exports, 92 load/control/reload renders | Basic integration and saved-patch round trips for all original module types |
| GUI simulator | 59 layouts; 46 panel captures | Panel/asset registration and visible layouts |
| Host DSP checks | Last full run: 72 suites; subsequent 0.1.45 Reverb and Reverb-history checks passed | Tested DSP and lifecycle cases, not exhaustive fidelity |
| Desktop Hub | Clean source build; three export/expander/network suites; Windows DLL loading | Combined converter and companion fixes work in the recorded desktop configuration |
| PR source inspection | All 160 recorded native project inputs present after the documented header relocation | The known missing-header defect is resolved; not a compiler result |

[The evidence record](docs/validation-0.1.45.json) identifies the frozen source
archives, their hashes, and selected results from the original reports. It is
an evidence summary, not a runnable harness. Source archives, logs and runtime
fixtures are not embedded in this PR; their hashes identify the material
needed to reproduce those earlier runs.

The ARM source archive is `SurgeXTNative-0.1.45-source.tar.gz`, SHA-256
`5224d62debb2323470eae95163657763cbcf4b13db4e203993aceec9205b2176`.
Its build used GNU Arm Embedded 12.3.Rel1. The delivered plugin archive is
SHA-256 `e2c0c49458f768c2f43bb06e393b0e597d2ac681e3519915b78e65830bdecc4d`.
Archive byte identity is not implied by ELF identity: compressed notice
contents differed between build environments while the uncompressed contents
and ELF matched.

## Outstanding checks

The latest physical-device Wi-Fi/audio/control/save-reload smoke check remains
unverified. Older device playback does not qualify this exact revision.
Hardware CPU/RAM headroom, helper scheduling latency, exhaustive parameter
combinations and remaining lifecycle fidelity work (including Resonator)
remain open. The port is a draft, not a hardware-qualified release.

To check a fresh PR checkout, initialize its pinned submodules and build with
the commands in [README.md](README.md), using a separate SDK checkout. Compare
the resulting registered module lists and packaging checks before moving to a
device smoke check. The simulator and desktop Hub require their respective
companion environments; building the native plugin alone does not exercise
Rack conversion.
