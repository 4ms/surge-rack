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

## Surge bugs fixed in the 4ms forks

These affect desktop Surge too. They are candidates for upstream pull requests; see
[UPSTREAM_MAP.md](UPSTREAM_MAP.md) for the commits.

| Finding | Fix |
| --- | --- |
| CHOW computes a dry/wet blend but returns the wet signal, so Mix has no effect | Copy the blend back (the port applies Mix in imported patches too, unlike Rack) |
| Distortion's drive ramp uses a two-step divisor over four oversampled steps; a falling drive passes through zero | Divide by the oversampling factor |
| Exciter's diode exponential can overflow | Bounded exponent in LevelDetector |
| Ensemble loops forever for nonpositive or NaN BBD periods; the BBD nonlinearity reads past its table | Bounded period and index |
| chowdsp DelayLine writes one element past its buffer | Wrap pointers at totalSize |
| Spring's damping filter goes NaN below 36.7 kHz | Clamp the filter cutoff below Nyquist |
| Chorus reads stale interpolation padding when BLOCK_SIZE < 12; its LFO phase can run away | Refresh the padding per block; wrap with fmod |
| Alias converts negative or large floats to unsigned (UB; ARM saturates) | Convert through wider integers |
| String's comb taps exceed the delay at 2x oversampling and low pitch | Bound by the oversampling level |
| Wavetable reads past its last frame when Continuous Morph is toggled at the end of the range | Re-check the frame index |
| TreeMonster, Rotary, Distortion and the LFO read uninitialized state | Value-initialize it |

## Port behavior that differs from Rack

| Area | Behavior | Notes |
| --- | --- | --- |
| Rack Ring Modulator Poly | The Rack wrapper does not refresh the oversampled external carrier in Poly; imported patches use Mono carrier handling | Source is the Rack wrapper |
| Combulator | Natively, the envelope follower follows the current sample rate; imported patches keep the creation rate like Rack | |
| Reverb2 (imported) | Allpasses are sized for 192 kHz, so Size CV beyond about 3.5 (48 kHz) or 2.5 (96 kHz) clamps the longest allpass delays at 32767 samples instead of 131071 | Memory: 8.3 MB instead of 13 MB per voice |
| DigitalRingMod Link | Feeds the first stage's output to the second, as labelled; Rack copies input A | Changes imported patches with Link on |
| DigitalRingMod legacy CXOR modes | Run their legacy kernels; Rack falls through to plain ring modulation | |
| VCF, Waveshaper | Modulation is clamped to the knob range; Rack lets CV exceed it | Imported patches with large CV |
| VCO | DC blocker defaults off natively (Rack: on); the reset trigger's low threshold is 0.1 V | Imported patches restore DC blocking |
| Oscillator random phases | Each voice's storage starts the same xorshift seed, so voices and instances share start phases after a simultaneous reset | Rack shares one randomly seeded RNG |
| EGxVCA (imported) | Level is cubed without Rack's clamp at 0, so negative Level phase-inverts | |

## Numerical changes from building against upstream

- Oscillators, Waveshaper and Unison now use Surge's interpolated pitch and dB tables
  rather than exact exp2/pow, matching desktop Surge and Rack. Outputs differ by
  interpolation error. Long high-drive Waveshaper renders diverge noticeably through
  its stateful shapers.
- With the tables, Classic and SHNoise no longer hang on extreme pitch modulation.

## Review limits

The suite contains 46 original module types plus modulation helpers. Simulator
and host evidence do not establish CPU/RAM headroom for every hardware patch.
Factory-preset/streaming coverage, remaining lifecycle audits (including
Resonator), and hardware CPU/RAM profiling remain follow-up work. The latest
review build still needs a physical Wi-Fi/control/save-reload smoke check.
