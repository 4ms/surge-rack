# Upstream issues and review scope

The review milestone is a usable MetaModule port, including normal Rack Hub
Save/Wi-Fi export, fixed CV inputs and editable modulation depths. Port defects
(missing controls, incorrect routing, state loss and incompatible export) remain
in scope. New fixes to Surge's own DSP are deferred as issues.

This is a local issue list, not a claim that tickets have been filed upstream.
Findings refer to this repository's pinned dependencies, not current upstream.

## Reproduced: Vocoder filter instability

Source: `VectorizedSVFilter.cpp` and `VocoderEffect.cpp` in the pinned Surge
dependency. An impulse with normalized frequency 0.45, Q=10, spread=0.04 grows
instead of decaying. Frequency 0.375, Q=5, spread=0.08 also reproduces it. Signed
resonance CV can reach lower Q than the panel knob's range.

The port preserves the original filter arithmetic without the experimental
frequency clamp or adaptive damping workaround. A retained local Vocoder reproducer compares the production and
source filters until the instability is reproduced; this does not play audio.
Vocoder routing/voice-history fixes remain and are tested separately.

## Existing deviations to review separately

The port includes the following deviations from its pinned source. Their
disposition requires review; they are not claims of exact upstream equivalence.

| Finding | Existing local behavior | Follow-up |
| --- | --- | --- |
| Chow calculates a dry/wet blend but returns the wet buffers | `runtime/effect-compat/ChowKernel.h` copies the blend back | Decide disposition of the existing deviation; record source issue |
| Legacy Distortion advances a drive ramp over four oversampled steps using a two-step divisor | `runtime/effect-compat/DistortionKernel.h` changes the divisor; a falling ramp can otherwise cross zero | Keep the raw reproducer; review existing deviation separately |
| Rack Ring Modulator Poly path does not refresh the oversampled external carrier or auto-select it | The imported path enables Mono carrier handling in imported Poly mode | Source is the Rack wrapper, not the sound engine; review separately |
| Exciter diode exponential can overflow | `runtime/effect-compat/chowdsp/exciter/LevelDetector.h` limits the exponential argument | Record existing numerical deviation separately |
| Ensemble can loop indefinitely for nonpositive BBD periods | `runtime/effect-compat/chowdsp/bbd_utils/BBDDelayLine.h` bounds invalid periods/tick density | Record termination behavior separately; do not confuse it with audio equivalence |
| Combulator envelope coefficient is set at construction rate | Native mode recalculates it; imported mode retains Rack behavior | Native extension only; imported behavior already preserved |

Other bounds/initialization edits in the embedded runtime adaptations still
need classification. ARM integer semantics, SIMD alignment, ownership changes
and defined initial storage can be necessary port adaptations; they are not
evidence that the source sound algorithm should be changed. This list is an
initial audit, not an exhaustive certification.

## Review limits

The suite contains 46 original module types plus modulation helpers. Simulator
and host evidence do not establish CPU/RAM headroom for every hardware patch.
Factory-preset/streaming coverage, remaining lifecycle audits (including
Resonator), and hardware CPU/RAM profiling remain follow-up work. The latest
review build still needs a physical Wi-Fi/control/save-reload smoke check.
