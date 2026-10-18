# Validation scope

The broad executable evidence applies to the frozen 0.1.45 source bundles.
Development 0.1.47 adds a measured Spring Reverb overload fix. The PR carries
that fix in its checked-in runtime sources, with formatting and explicit mapping
names from the earlier review. **The current PR tree and intermediate commits
have not received a complete ARM build or a separate hardware run.** The targeted
Spring source check is described below.

## Hardware overload measurements, 2026-10-18

On USB004 firmware at 48 kHz / 256 samples, the verified 0.1.45 development
package's 46 fixed native demos processed 86,459 blocks. Spring was the only
overloaded case: 121.46% mean / 123.76% peak CPU, 952 overruns in 952 blocks.
The [complete table](hardware/2026-10-18.md) includes each workload and evidence
hashes. These are whole-patch measurements, including stimulus/support modules.

The 0.1.47 Spring optimization retains the DSP network and sample/control rates;
there is no Spring oversampling to disable. It keeps native SIMD values in
registers, prepares reflection interpolation at control rate and caches unchanged
coefficients while preserving RNG, Chaos smoothing, Knock and reset behavior.

| 0.1.47 workload | Mean CPU | Peak CPU | Overruns / processed blocks |
| --- | ---: | ---: | ---: |
| Original fixed Spring demo | 71.34% | 73.93% | 0 / 5,691 |
| High controls | 71.06% | 73.14% | 0 / 5,654 |
| Audio-rate CV and Knock | 81.56% | 86.75% | 0 / 5,629 |

Each live run lasted 30 seconds. A separate five-second buffered capture of the
fixed demo measured 71.40% mean / 73.81% peak and zero overruns in 940 blocks.
These are single stereo Spring workloads. Maximum polyphony and other rates
remain outside this evidence. See [the Spring record](hardware/2026-10-18-spring.md)
and [receipt](hardware/2026-10-18-spring.json) for package/ELF hashes and audio checks.

Four targeted development host suites and 79 Spring firmware-simulator renders
passed. Three renders against the frozen 0.1.45 simulator matched exactly for
fixed/high settings and differed by at most 0.0000028611 V under audio-rate CV.
The three changed PR C++ files match those measured sources after the existing
PR formatter; both source hashes are recorded in the receipt. This establishes
source correspondence, not byte identity of an unbuilt full PR package.

The development Spring kernel regression was also compiled directly against
this PR's `runtime/` and `dsp/` headers with MinGW GCC, C++23 and `-O2`.
That targeted executable passed: cached/uncached modulation and full-capacity
comparisons had zero maximum error, including 8–192 kHz, ring wraps, Knock,
rate/reset changes, decay and allocation checks. Its compiler dependency file
confirms that the adapted headers came from the PR worktree. Fixture, executable,
dependency-list and result hashes are recorded in the Spring receipt. The test
fixture remains in the development tooling, outside this production PR.

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
The fixed native workload CPU results above are measured. RAM headroom,
per-helper scheduling latency, exhaustive parameter combinations and remaining
lifecycle fidelity work (including Resonator) remain open. The port remains experimental.

To check a fresh PR checkout, initialize its pinned submodules and build with
the commands in [README.md](README.md), using a separate SDK checkout. Compare
the resulting registered module lists and packaging checks before moving to a
device smoke check. The simulator and desktop Hub require their respective
companion environments; building the native plugin alone does not exercise
Rack conversion.
