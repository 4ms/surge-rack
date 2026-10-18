# Upstream source map

These 78 production files derive from pinned sources already present in the
repository's submodules. The map distinguishes whole-file adaptations,
extracted methods and embedded data. Files under `../dsp/` are the port's
engine/host integration; they are reviewed separately from these imports.

Initialize submodules before following local source links. The top-level Surge
revision is `65d381578fe9d896e3cb56e0d44c1f89c12a2cd4`. Nested libraries use the
revisions pinned by that checkout:

| Library | Revision |
| --- | --- |
| `surge/libs/sst/sst-filters` | `9eb74b595c0fc244fdd67452fc17948edec4cf28` |
| `surge/libs/sst/sst-basic-blocks` | `ae9c7e4f3ab58c9b26c5b45995a8adc16ec0830d` |
| `surge/libs/sst/sst-effects` | `43a54d94370986f7fe92931493435f2a8b67603f` |
| `surge/libs/eurorack/eurorack` | `9f934b8bd7301f4b9cb0a0e8a7e58b9bc1f17297` |

## Adaptation categories

- **Host extraction:** effect kernels retain selected original engine methods
  while replacing desktop storage/base classes with `*_support.h` adapters.
  `*Host.h` files extract utilities such as stereo width or storage lookups.
  The extracted code should be compared at method level, not treated as an
  independent new algorithm.
- **Ownership and reset:** String owns preallocated delay lines; Twist resets
  Plaits/resampler storage in place; effect adapters expose native reset paths.
  Imported Rack paths preserve different initialization/history where required.
  Reset changes are behavior-sensitive even when steady-state DSP is unchanged.
- **Platform semantics:** Alias uses explicit modular conversions; Bonsai
  defines noise wrapping without signed overflow; SIMD buffers are aligned;
  stmlib RNG updates use lock-free atomics. Concurrent RNG call ordering is not
  a promise of identical multi-module noise sequences.
- **Block/storage changes:** Chorus refreshes interpolation padding over
  eight-sample writes. Neuron omits unused delay channels. Reverb2 and Spring
  adapt delay storage; Spring also shares identical reflection histories.
  Spring 0.1.47 prepares reflection tap interpolation at control rate and caches
  unchanged coefficients, invalidating them during preparation. Its host SIMD
  adapter stores native vector values to avoid ARM aggregate copies. See the
  [measured optimization](../hardware/2026-10-18-spring.md).
  Review capacities and boundary/history behavior alongside the algorithms.
- **Defined initial state and bounds:** SST coefficient scratch arrays are
  initialized; several dependencies add bounds or finite-domain handling.
  These edits need individual classification. They are not all automatically
  justified by the target architecture.
- **Existing behavior deviations:** Chow mixing, Distortion's drive ramp,
  Exciter's exponential bound and Ensemble's invalid-period handling are
  tracked in [UPSTREAM_ISSUES.md](../UPSTREAM_ISSUES.md). The list is incomplete;
  it does not certify all other adaptations as equivalent.

Additional family details: LFO omits Formula/MSEG evaluation absent from the
Rack LFO. Twist omits the desktop external-FM resampling path absent from its
Rack interface. `FXModControl.h` bounds the sine-table index and exposes reset
state. String also adjusts delay bounds for oversampling and wraps multi-cycle
exciter phases. Consult the implementations for the exact conditions.

## File-level origins

Paths in the right column are relative to the repository root. Formatting and
namespace changes make raw line diffs noisy; compare the relevant declarations
and method bodies after applying the same formatter to temporary copies. Keep
numerical, reset and storage differences visible during that comparison.

| Runtime file | Pinned source | Adaptation |
| --- | --- | --- |
| [AliasOscillator.cpp](AliasOscillator.cpp) | [surge/src/common/dsp/oscillators/AliasOscillator.cpp](../../surge/src/common/dsp/oscillators/AliasOscillator.cpp) | Adapted copy; compare with the pinned source. |
| [effect-compat/ChorusHost.h](effect-compat/ChorusHost.h) | [surge/src/common/dsp/Effect.h](../../surge/src/common/dsp/Effect.h)<br>[surge/src/common/dsp/utilities/DSPUtils.h](../../surge/src/common/dsp/utilities/DSPUtils.h) | Extracted host utility methods; not a complete upstream file. |
| [effect-compat/ChorusKernel.h](effect-compat/ChorusKernel.h) | [surge/src/common/dsp/effects/ChorusEffect.h](../../surge/src/common/dsp/effects/ChorusEffect.h)<br>[surge/src/common/dsp/effects/ChorusEffectImpl.h](../../surge/src/common/dsp/effects/ChorusEffectImpl.h) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/chowdsp/bbd_utils/BBDDelayLine.h](effect-compat/chowdsp/bbd_utils/BBDDelayLine.h) | [surge/src/common/dsp/effects/chowdsp/bbd_utils/BBDDelayLine.h](../../surge/src/common/dsp/effects/chowdsp/bbd_utils/BBDDelayLine.h)<br>[surge/src/common/dsp/effects/chowdsp/bbd_utils/BBDDelayLine.cpp](../../surge/src/common/dsp/effects/chowdsp/bbd_utils/BBDDelayLine.cpp) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/chowdsp/bbd_utils/BBDFilterBank.h](effect-compat/chowdsp/bbd_utils/BBDFilterBank.h) | [surge/src/common/dsp/effects/chowdsp/bbd_utils/BBDFilterBank.h](../../surge/src/common/dsp/effects/chowdsp/bbd_utils/BBDFilterBank.h) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/chowdsp/bbd_utils/BBDNonlin.h](effect-compat/chowdsp/bbd_utils/BBDNonlin.h) | [surge/src/common/dsp/effects/chowdsp/bbd_utils/BBDNonlin.h](../../surge/src/common/dsp/effects/chowdsp/bbd_utils/BBDNonlin.h) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/chowdsp/exciter/LevelDetector.h](effect-compat/chowdsp/exciter/LevelDetector.h) | [surge/src/common/dsp/effects/chowdsp/exciter/LevelDetector.h](../../surge/src/common/dsp/effects/chowdsp/exciter/LevelDetector.h) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/chowdsp/shared/chowdsp_DelayInterpolation.h](effect-compat/chowdsp/shared/chowdsp_DelayInterpolation.h) | [surge/src/common/dsp/effects/chowdsp/shared/chowdsp_DelayInterpolation.h](../../surge/src/common/dsp/effects/chowdsp/shared/chowdsp_DelayInterpolation.h) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/chowdsp/shared/chowdsp_DelayLine.cpp](effect-compat/chowdsp/shared/chowdsp_DelayLine.cpp) | [surge/src/common/dsp/effects/chowdsp/shared/chowdsp_DelayLine.cpp](../../surge/src/common/dsp/effects/chowdsp/shared/chowdsp_DelayLine.cpp) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/chowdsp/shared/chowdsp_DelayLine.h](effect-compat/chowdsp/shared/chowdsp_DelayLine.h) | [surge/src/common/dsp/effects/chowdsp/shared/chowdsp_DelayLine.h](../../surge/src/common/dsp/effects/chowdsp/shared/chowdsp_DelayLine.h) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/chowdsp/shared/omega.h](effect-compat/chowdsp/shared/omega.h) | [surge/src/common/dsp/effects/chowdsp/shared/omega.h](../../surge/src/common/dsp/effects/chowdsp/shared/omega.h) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/chowdsp/shared/Oversampling.h](effect-compat/chowdsp/shared/Oversampling.h) | [surge/src/common/dsp/effects/chowdsp/shared/Oversampling.h](../../surge/src/common/dsp/effects/chowdsp/shared/Oversampling.h) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/chowdsp/shared/SmoothedValue.h](effect-compat/chowdsp/shared/SmoothedValue.h) | [surge/src/common/dsp/effects/chowdsp/shared/SmoothedValue.h](../../surge/src/common/dsp/effects/chowdsp/shared/SmoothedValue.h) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/chowdsp/shared/wdf_sse.h](effect-compat/chowdsp/shared/wdf_sse.h) | [surge/src/common/dsp/effects/chowdsp/shared/wdf_sse.h](../../surge/src/common/dsp/effects/chowdsp/shared/wdf_sse.h) | Embedded dependency adaptation; ownership, initialization and bounds require review. |
| [effect-compat/ChowKernel.h](effect-compat/ChowKernel.h) | [surge/src/common/dsp/effects/chowdsp/CHOWEffect.h](../../surge/src/common/dsp/effects/chowdsp/CHOWEffect.h)<br>[surge/src/common/dsp/effects/chowdsp/CHOWEffect.cpp](../../surge/src/common/dsp/effects/chowdsp/CHOWEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/CombulatorKernel.h](effect-compat/CombulatorKernel.h) | [surge/src/common/dsp/effects/CombulatorEffect.h](../../surge/src/common/dsp/effects/CombulatorEffect.h)<br>[surge/src/common/dsp/effects/CombulatorEffect.cpp](../../surge/src/common/dsp/effects/CombulatorEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/DistortionHost.h](effect-compat/DistortionHost.h) | [surge/src/common/SurgeStorage.cpp](../../surge/src/common/SurgeStorage.cpp)<br>[surge/src/common/FilterConfiguration.h](../../surge/src/common/FilterConfiguration.h) | Extracted host utility methods; not a complete upstream file. |
| [effect-compat/DistortionKernel.h](effect-compat/DistortionKernel.h) | [surge/src/common/dsp/effects/DistortionEffect.h](../../surge/src/common/dsp/effects/DistortionEffect.h)<br>[surge/src/common/dsp/effects/DistortionEffect.cpp](../../surge/src/common/dsp/effects/DistortionEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/EnsembleHost.h](effect-compat/EnsembleHost.h) | [surge/src/common/SurgeStorage.cpp](../../surge/src/common/SurgeStorage.cpp)<br>[surge/src/common/dsp/Effect.h](../../surge/src/common/dsp/Effect.h) | Extracted host utility methods; not a complete upstream file. |
| [effect-compat/EnsembleKernel.h](effect-compat/EnsembleKernel.h) | [surge/src/common/dsp/effects/BBDEnsembleEffect.h](../../surge/src/common/dsp/effects/BBDEnsembleEffect.h)<br>[surge/src/common/dsp/effects/BBDEnsembleEffect.cpp](../../surge/src/common/dsp/effects/BBDEnsembleEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/ExciterKernel.h](effect-compat/ExciterKernel.h) | [surge/src/common/dsp/effects/chowdsp/ExciterEffect.h](../../surge/src/common/dsp/effects/chowdsp/ExciterEffect.h)<br>[surge/src/common/dsp/effects/chowdsp/ExciterEffect.cpp](../../surge/src/common/dsp/effects/chowdsp/ExciterEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/FrequencyShifterHost.h](effect-compat/FrequencyShifterHost.h) | [surge/src/common/SurgeStorage.cpp](../../surge/src/common/SurgeStorage.cpp)<br>[surge/src/common/dsp/utilities/DSPUtils.h](../../surge/src/common/dsp/utilities/DSPUtils.h) | Extracted host utility methods; not a complete upstream file. |
| [effect-compat/FrequencyShifterKernel.h](effect-compat/FrequencyShifterKernel.h) | [surge/src/common/dsp/effects/FrequencyShifterEffect.h](../../surge/src/common/dsp/effects/FrequencyShifterEffect.h)<br>[surge/src/common/dsp/effects/FrequencyShifterEffect.cpp](../../surge/src/common/dsp/effects/FrequencyShifterEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/LFOHost.h](effect-compat/LFOHost.h) | [surge/src/common/SurgeStorage.cpp](../../surge/src/common/SurgeStorage.cpp) | Extracted host utility methods; not a complete upstream file. |
| [effect-compat/LFOKernel.h](effect-compat/LFOKernel.h) | [surge/src/common/dsp/modulators/LFOModulationSource.h](../../surge/src/common/dsp/modulators/LFOModulationSource.h)<br>[surge/src/common/dsp/modulators/LFOModulationSource.cpp](../../surge/src/common/dsp/modulators/LFOModulationSource.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/NeuronHost.h](effect-compat/NeuronHost.h) | [surge/src/common/dsp/Effect.h](../../surge/src/common/dsp/Effect.h) | Extracted host utility methods; not a complete upstream file. |
| [effect-compat/NeuronKernel.h](effect-compat/NeuronKernel.h) | [surge/src/common/dsp/effects/chowdsp/NeuronEffect.h](../../surge/src/common/dsp/effects/chowdsp/NeuronEffect.h)<br>[surge/src/common/dsp/effects/chowdsp/NeuronEffect.cpp](../../surge/src/common/dsp/effects/chowdsp/NeuronEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/NimbusKernel.h](effect-compat/NimbusKernel.h) | [surge/src/common/dsp/effects/NimbusEffect.h](../../surge/src/common/dsp/effects/NimbusEffect.h)<br>[surge/src/common/dsp/effects/NimbusEffect.cpp](../../surge/src/common/dsp/effects/NimbusEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/portable_intrinsics.h](effect-compat/portable_intrinsics.h) | [surge/src/common/dsp/vembertech/portable_intrinsics.h](../../surge/src/common/dsp/vembertech/portable_intrinsics.h) | Portable SIMD compatibility definitions. |
| [effect-compat/ResonatorHost.h](effect-compat/ResonatorHost.h) | [surge/src/common/SurgeStorage.cpp](../../surge/src/common/SurgeStorage.cpp)<br>[surge/src/common/dsp/effects/ResonatorEffect.cpp](../../surge/src/common/dsp/effects/ResonatorEffect.cpp) | Extracted host utility methods; not a complete upstream file. |
| [effect-compat/ResonatorKernel.h](effect-compat/ResonatorKernel.h) | [surge/src/common/dsp/effects/ResonatorEffect.h](../../surge/src/common/dsp/effects/ResonatorEffect.h)<br>[surge/src/common/dsp/effects/ResonatorEffect.cpp](../../surge/src/common/dsp/effects/ResonatorEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/Reverb2Host.h](effect-compat/Reverb2Host.h) | [surge/src/common/dsp/Effect.h](../../surge/src/common/dsp/Effect.h) | Extracted host utility methods; not a complete upstream file. |
| [effect-compat/Reverb2Kernel.h](effect-compat/Reverb2Kernel.h) | [surge/src/common/dsp/effects/Reverb2Effect.h](../../surge/src/common/dsp/effects/Reverb2Effect.h)<br>[surge/src/common/dsp/effects/Reverb2Effect.cpp](../../surge/src/common/dsp/effects/Reverb2Effect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/RingModKernel.h](effect-compat/RingModKernel.h) | [surge/src/common/dsp/effects/RingModulatorEffect.h](../../surge/src/common/dsp/effects/RingModulatorEffect.h)<br>[surge/src/common/dsp/effects/RingModulatorEffect.cpp](../../surge/src/common/dsp/effects/RingModulatorEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/RotaryHost.h](effect-compat/RotaryHost.h) | [surge/src/common/SurgeStorage.cpp](../../surge/src/common/SurgeStorage.cpp)<br>[surge/src/common/dsp/Effect.h](../../surge/src/common/dsp/Effect.h)<br>[surge/src/common/FilterConfiguration.h](../../surge/src/common/FilterConfiguration.h) | Extracted host utility methods; not a complete upstream file. |
| [effect-compat/RotaryKernel.h](effect-compat/RotaryKernel.h) | [surge/src/common/dsp/effects/RotarySpeakerEffect.h](../../surge/src/common/dsp/effects/RotarySpeakerEffect.h)<br>[surge/src/common/dsp/effects/RotarySpeakerEffect.cpp](../../surge/src/common/dsp/effects/RotarySpeakerEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/spring/BilinearUtils.h](effect-compat/spring/BilinearUtils.h) | [surge/src/common/dsp/effects/chowdsp/shared/BilinearUtils.h](../../surge/src/common/dsp/effects/chowdsp/shared/BilinearUtils.h) | Embedded spring storage/host adaptation; see family notes below. |
| [effect-compat/spring/chowdsp_IIR.h](effect-compat/spring/chowdsp_IIR.h) | [surge/src/common/dsp/effects/chowdsp/shared/chowdsp_IIR.h](../../surge/src/common/dsp/effects/chowdsp/shared/chowdsp_IIR.h) | Embedded spring storage/host adaptation; see family notes below. |
| [effect-compat/spring/ReflectionNetwork.h](effect-compat/spring/ReflectionNetwork.h) | [surge/src/common/dsp/effects/chowdsp/spring_reverb/ReflectionNetwork.h](../../surge/src/common/dsp/effects/chowdsp/spring_reverb/ReflectionNetwork.h) | Embedded spring storage/host adaptation; see family notes below. |
| [effect-compat/spring/SchroederAllpass.h](effect-compat/spring/SchroederAllpass.h) | [surge/src/common/dsp/effects/chowdsp/spring_reverb/SchroederAllpass.h](../../surge/src/common/dsp/effects/chowdsp/spring_reverb/SchroederAllpass.h) | Embedded spring storage/host adaptation; see family notes below. |
| [effect-compat/spring/Shelf.h](effect-compat/spring/Shelf.h) | [surge/src/common/dsp/effects/chowdsp/shared/Shelf.h](../../surge/src/common/dsp/effects/chowdsp/shared/Shelf.h) | Embedded spring storage/host adaptation; see family notes below. |
| [effect-compat/spring/SpringKernel.h](effect-compat/spring/SpringKernel.h) | [surge/src/common/dsp/effects/chowdsp/SpringReverbEffect.h](../../surge/src/common/dsp/effects/chowdsp/SpringReverbEffect.h)<br>[surge/src/common/dsp/effects/chowdsp/SpringReverbEffect.cpp](../../surge/src/common/dsp/effects/chowdsp/SpringReverbEffect.cpp) | Embedded spring storage/host adaptation; see family notes below. |
| [effect-compat/spring/SpringReverbProc.h](effect-compat/spring/SpringReverbProc.h) | [surge/src/common/dsp/effects/chowdsp/spring_reverb/SpringReverbProc.h](../../surge/src/common/dsp/effects/chowdsp/spring_reverb/SpringReverbProc.h)<br>[surge/src/common/dsp/effects/chowdsp/spring_reverb/SpringReverbProc.cpp](../../surge/src/common/dsp/effects/chowdsp/spring_reverb/SpringReverbProc.cpp) | Embedded spring storage/host adaptation; see family notes below. |
| [effect-compat/spring/StateVariableFilter.h](effect-compat/spring/StateVariableFilter.h) | [surge/src/common/dsp/effects/chowdsp/shared/StateVariableFilter.h](../../surge/src/common/dsp/effects/chowdsp/shared/StateVariableFilter.h)<br>[surge/src/common/dsp/effects/chowdsp/shared/StateVariableFilter.cpp](../../surge/src/common/dsp/effects/chowdsp/shared/StateVariableFilter.cpp) | Embedded spring storage/host adaptation; see family notes below. |
| [effect-compat/SSEComplex.h](effect-compat/SSEComplex.h) | [surge/src/common/dsp/utilities/SSEComplex.h](../../surge/src/common/dsp/utilities/SSEComplex.h) | Adapted copy; compare with the pinned source. |
| [effect-compat/sst/basic-blocks/modulators/FXModControl.h](effect-compat/sst/basic-blocks/modulators/FXModControl.h) | [surge/libs/sst/sst-basic-blocks/include/sst/basic-blocks/modulators/FXModControl.h](../../surge/libs/sst/sst-basic-blocks/include/sst/basic-blocks/modulators/FXModControl.h) | SST adaptation; see family notes below. |
| [effect-compat/sst/effects/Bonsai.h](effect-compat/sst/effects/Bonsai.h) | [surge/libs/sst/sst-effects/include/sst/effects/Bonsai.h](../../surge/libs/sst/sst-effects/include/sst/effects/Bonsai.h) | SST adaptation; see family notes below. |
| [effect-compat/sst/effects/Flanger.h](effect-compat/sst/effects/Flanger.h) | [surge/libs/sst/sst-effects/include/sst/effects/Flanger.h](../../surge/libs/sst/sst-effects/include/sst/effects/Flanger.h) | SST adaptation; see family notes below. |
| [effect-compat/sst/effects/Reverb1.h](effect-compat/sst/effects/Reverb1.h) | [surge/libs/sst/sst-effects/include/sst/effects/Reverb1.h](../../surge/libs/sst/sst-effects/include/sst/effects/Reverb1.h) | SST adaptation; see family notes below. |
| [effect-compat/TreeMonsterHost.h](effect-compat/TreeMonsterHost.h) | [surge/src/common/dsp/Effect.h](../../surge/src/common/dsp/Effect.h) | Extracted host utility methods; not a complete upstream file. |
| [effect-compat/TreeMonsterKernel.h](effect-compat/TreeMonsterKernel.h) | [surge/src/common/dsp/effects/TreemonsterEffect.h](../../surge/src/common/dsp/effects/TreemonsterEffect.h)<br>[surge/src/common/dsp/effects/TreemonsterEffect.cpp](../../surge/src/common/dsp/effects/TreemonsterEffect.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [effect-compat/VocoderIntrinsics.h](effect-compat/VocoderIntrinsics.h) | [surge/src/common/dsp/vembertech/portable_intrinsics.h](../../surge/src/common/dsp/vembertech/portable_intrinsics.h) | Portable SIMD compatibility definitions. |
| [effect-compat/VocoderKernel.h](effect-compat/VocoderKernel.h) | [surge/src/common/dsp/effects/VocoderEffect.h](../../surge/src/common/dsp/effects/VocoderEffect.h)<br>[surge/src/common/dsp/effects/VocoderEffect.cpp](../../surge/src/common/dsp/effects/VocoderEffect.cpp)<br>[surge/src/common/dsp/filters/VectorizedSVFilter.h](../../surge/src/common/dsp/filters/VectorizedSVFilter.h)<br>[surge/src/common/dsp/filters/VectorizedSVFilter.cpp](../../surge/src/common/dsp/filters/VectorizedSVFilter.cpp) | Extracted engine declarations/methods, embedded host names and lifecycle adaptations. |
| [filter-compat/sst/filters/BiquadFilter.h](filter-compat/sst/filters/BiquadFilter.h) | [surge/libs/sst/sst-filters/include/sst/filters/BiquadFilter.h](../../surge/libs/sst/sst-filters/include/sst/filters/BiquadFilter.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/CutoffWarp.h](filter-compat/sst/filters/CutoffWarp.h) | [surge/libs/sst/sst-filters/include/sst/filters/CutoffWarp.h](../../surge/libs/sst/sst-filters/include/sst/filters/CutoffWarp.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/DiodeLadder.h](filter-compat/sst/filters/DiodeLadder.h) | [surge/libs/sst/sst-filters/include/sst/filters/DiodeLadder.h](../../surge/libs/sst/sst-filters/include/sst/filters/DiodeLadder.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/FilterCoefficientMaker.h](filter-compat/sst/filters/FilterCoefficientMaker.h) | [surge/libs/sst/sst-filters/include/sst/filters/FilterCoefficientMaker.h](../../surge/libs/sst/sst-filters/include/sst/filters/FilterCoefficientMaker.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/FilterCoefficientMaker_Impl.h](filter-compat/sst/filters/FilterCoefficientMaker_Impl.h) | [surge/libs/sst/sst-filters/include/sst/filters/FilterCoefficientMaker_Impl.h](../../surge/libs/sst/sst-filters/include/sst/filters/FilterCoefficientMaker_Impl.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/FilterConfiguration.h](filter-compat/sst/filters/FilterConfiguration.h) | [surge/libs/sst/sst-filters/include/sst/filters/FilterConfiguration.h](../../surge/libs/sst/sst-filters/include/sst/filters/FilterConfiguration.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/HalfRateFilter.h](filter-compat/sst/filters/HalfRateFilter.h) | [surge/libs/sst/sst-filters/include/sst/filters/HalfRateFilter.h](../../surge/libs/sst/sst-filters/include/sst/filters/HalfRateFilter.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/K35Filter.h](filter-compat/sst/filters/K35Filter.h) | [surge/libs/sst/sst-filters/include/sst/filters/K35Filter.h](../../surge/libs/sst/sst-filters/include/sst/filters/K35Filter.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/OBXDFilter.h](filter-compat/sst/filters/OBXDFilter.h) | [surge/libs/sst/sst-filters/include/sst/filters/OBXDFilter.h](../../surge/libs/sst/sst-filters/include/sst/filters/OBXDFilter.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/QuadFilterUnit.h](filter-compat/sst/filters/QuadFilterUnit.h) | [surge/libs/sst/sst-filters/include/sst/filters/QuadFilterUnit.h](../../surge/libs/sst/sst-filters/include/sst/filters/QuadFilterUnit.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/QuadFilterUnit_Impl.h](filter-compat/sst/filters/QuadFilterUnit_Impl.h) | [surge/libs/sst/sst-filters/include/sst/filters/QuadFilterUnit_Impl.h](../../surge/libs/sst/sst-filters/include/sst/filters/QuadFilterUnit_Impl.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/ResonanceWarp.h](filter-compat/sst/filters/ResonanceWarp.h) | [surge/libs/sst/sst-filters/include/sst/filters/ResonanceWarp.h](../../surge/libs/sst/sst-filters/include/sst/filters/ResonanceWarp.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/TriPoleFilter.h](filter-compat/sst/filters/TriPoleFilter.h) | [surge/libs/sst/sst-filters/include/sst/filters/TriPoleFilter.h](../../surge/libs/sst/sst-filters/include/sst/filters/TriPoleFilter.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/TuningProvider.h](filter-compat/sst/filters/TuningProvider.h) | [surge/libs/sst/sst-filters/include/sst/filters/TuningProvider.h](../../surge/libs/sst/sst-filters/include/sst/filters/TuningProvider.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters/VintageLadders.h](filter-compat/sst/filters/VintageLadders.h) | [surge/libs/sst/sst-filters/include/sst/filters/VintageLadders.h](../../surge/libs/sst/sst-filters/include/sst/filters/VintageLadders.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/filters.h](filter-compat/sst/filters.h) | [surge/libs/sst/sst-filters/include/sst/filters.h](../../surge/libs/sst/sst-filters/include/sst/filters.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/utilities/globals.h](filter-compat/sst/utilities/globals.h) | [surge/libs/sst/sst-filters/include/sst/utilities/globals.h](../../surge/libs/sst/sst-filters/include/sst/utilities/globals.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [filter-compat/sst/utilities/SincTable.h](filter-compat/sst/utilities/SincTable.h) | [surge/libs/sst/sst-filters/include/sst/utilities/SincTable.h](../../surge/libs/sst/sst-filters/include/sst/utilities/SincTable.h) | Pinned filter header; coefficient scratch arrays initialized where present. |
| [plaits-compat/stmlib/utils/random.cc](plaits-compat/stmlib/utils/random.cc) | [surge/libs/eurorack/eurorack/stmlib/utils/random.cc](../../surge/libs/eurorack/eurorack/stmlib/utils/random.cc) | Shared RNG state made atomic; original LCG constants retained. |
| [plaits-compat/stmlib/utils/random.h](plaits-compat/stmlib/utils/random.h) | [surge/libs/eurorack/eurorack/stmlib/utils/random.h](../../surge/libs/eurorack/eurorack/stmlib/utils/random.h) | Shared RNG state made atomic; original LCG constants retained. |
| [StringOscillator.cpp](StringOscillator.cpp) | [surge/src/common/dsp/oscillators/StringOscillator.cpp](../../surge/src/common/dsp/oscillators/StringOscillator.cpp) | Adapted copy; compare with the pinned source. |
| [table-generated/table_resources.cpp](table-generated/table_resources.cpp) | [surge/resources/data/wavetables/Basic/Sine To Sawtooth.wt](../../surge/resources/data/wavetables/Basic/Sine%20To%20Sawtooth.wt)<br>[surge/resources/surge-shared/windows.wt](../../surge/resources/surge-shared/windows.wt) | Embedded data derived from these two binary tables; not handwritten DSP. |
| [table-generated/Wavetable.cpp](table-generated/Wavetable.cpp) | [surge/src/common/dsp/Wavetable.cpp](../../surge/src/common/dsp/Wavetable.cpp) | Adapted copy; compare with the pinned source. |
| [table-generated/WindowOscillator.cpp](table-generated/WindowOscillator.cpp) | [surge/src/common/dsp/oscillators/WindowOscillator.cpp](../../surge/src/common/dsp/oscillators/WindowOscillator.cpp) | Adapted copy; compare with the pinned source. |
| [TwistOscillator.cpp](TwistOscillator.cpp) | [surge/src/common/dsp/oscillators/TwistOscillator.cpp](../../surge/src/common/dsp/oscillators/TwistOscillator.cpp) | Adapted copy; compare with the pinned source. |
