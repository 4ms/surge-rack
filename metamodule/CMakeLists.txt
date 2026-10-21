cmake_minimum_required(VERSION 3.22)
if(NOT METAMODULE_SDK_DIR)
    if(DEFINED ENV{METAMODULE_SDK_DIR})
        set(METAMODULE_SDK_DIR "$ENV{METAMODULE_SDK_DIR}")
    else()
        set(METAMODULE_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../metamodule-plugin-sdk")
    endif()
endif()
include("${METAMODULE_SDK_DIR}/plugin.cmake")
# The SDK packages plugin.json verbatim. Keep the displayed and build versions
# together, and reconfigure automatically whenever the metadata changes.
file(READ "${CMAKE_CURRENT_LIST_DIR}/plugin.json" SURGE_MM_METADATA_JSON)
string(JSON SURGE_MM_VERSION GET "${SURGE_MM_METADATA_JSON}" version)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/plugin.json")
project(SurgeXTNative VERSION "${SURGE_MM_VERSION}" LANGUAGES C CXX)
set(SURGE_DIR "${CMAKE_CURRENT_LIST_DIR}/../surge")
set(COMMON "${SURGE_DIR}/src/common")
# Checked-in embedded adaptations of the pinned Surge sources.
set(SURGE_MM_RUNTIME "${CMAKE_CURRENT_LIST_DIR}/runtime")
set(EURORACK "${SURGE_DIR}/libs/eurorack/eurorack")
# The default wavetable and the Window oscillator's table, embedded from surge/resources.
set(TABLE_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/table_resources.cpp")
set(TABLE_FILES
    "${SURGE_DIR}/resources/data/wavetables/Basic/Sine To Sawtooth.wt"
    "${SURGE_DIR}/resources/surge-shared/windows.wt")
add_custom_command(OUTPUT "${TABLE_RESOURCES}"
    COMMAND ${CMAKE_COMMAND} -DSURGE_DIR=${SURGE_DIR} -DOUT=${TABLE_RESOURCES}
        -P ${CMAKE_CURRENT_LIST_DIR}/cmake/embed_tables.cmake
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/cmake/embed_tables.cmake ${TABLE_FILES}
    VERBATIM)
set(EFFECT_COMPAT "${SURGE_MM_RUNTIME}/effect-compat")
file(GLOB_RECURSE PLAITS_SOURCES CONFIGURE_DEPENDS "${EURORACK}/plaits/dsp/*.cc")
set(NIMBUS_SRC_SOURCES
    ${SURGE_DIR}/libs/libsamplerate/src/samplerate.c
    ${SURGE_DIR}/libs/libsamplerate/src/src_sinc.c
    ${SURGE_DIR}/libs/libsamplerate/src/src_linear.c
    ${SURGE_DIR}/libs/libsamplerate/src/src_zoh.c)
set_source_files_properties(${NIMBUS_SRC_SOURCES} PROPERTIES COMPILE_DEFINITIONS
    "HAVE_STDBOOL_H=1;ENABLE_SINC_FAST_CONVERTER;PACKAGE=\"libsamplerate\";VERSION=\"0.2.1\"")
add_library(surge-mm-dsp STATIC
    ${NIMBUS_SRC_SOURCES}
    ${EURORACK}/clouds/dsp/correlator.cc
    ${EURORACK}/clouds/dsp/granular_processor.cc
    ${EURORACK}/clouds/dsp/mu_law.cc
    ${EURORACK}/clouds/dsp/pvoc/frame_transformation.cc
    ${EURORACK}/clouds/dsp/pvoc/phase_vocoder.cc
    ${EURORACK}/clouds/dsp/pvoc/stft.cc
    ${EURORACK}/clouds/resources.cc
    dsp/lfo_dsp.cpp
    dsp/delay_dsp.cpp
    dsp/tuned_delay_plus_dsp.cpp
    dsp/tuned_delay_dsp.cpp
    dsp/nimbus_dsp.cpp
    dsp/reverb_dsp.cpp
    dsp/reverb2_dsp.cpp
    dsp/spring_dsp.cpp
    dsp/combulator_dsp.cpp
    dsp/resonator_dsp.cpp
    dsp/vocoder_dsp.cpp
    dsp/treemonster_dsp.cpp
    dsp/ringmod_dsp.cpp
    dsp/frequency_shifter_dsp.cpp
    dsp/bonsai_dsp.cpp
    dsp/exciter_dsp.cpp
    dsp/chow_dsp.cpp
    dsp/neuron_dsp.cpp
    dsp/distortion_dsp.cpp
    dsp/ensemble_dsp.cpp
    dsp/chorus_dsp.cpp
    dsp/rotary_dsp.cpp
    dsp/flanger_dsp.cpp
    dsp/phaser_dsp.cpp
    dsp/engine.cpp
    dsp/compat/SurgeStorage.cpp
    ${COMMON}/dsp/Effect.cpp
    ${COMMON}/dsp/effects/VocoderEffect.cpp
    ${COMMON}/dsp/effects/ResonatorEffect.cpp
    ${COMMON}/dsp/effects/CombulatorEffect.cpp
    ${COMMON}/dsp/effects/FrequencyShifterEffect.cpp
    ${COMMON}/dsp/effects/TreemonsterEffect.cpp
    ${COMMON}/dsp/effects/RotarySpeakerEffect.cpp
    ${COMMON}/dsp/effects/DistortionEffect.cpp
    ${COMMON}/dsp/effects/RingModulatorEffect.cpp
    ${COMMON}/dsp/effects/chowdsp/CHOWEffect.cpp
    ${COMMON}/dsp/effects/chowdsp/ExciterEffect.cpp
    ${COMMON}/dsp/effects/chowdsp/NeuronEffect.cpp
    ${COMMON}/dsp/effects/chowdsp/SpringReverbEffect.cpp
    ${COMMON}/dsp/effects/chowdsp/spring_reverb/SpringReverbProc.cpp
    ${COMMON}/dsp/effects/chowdsp/shared/StateVariableFilter.cpp
    ${COMMON}/dsp/effects/Reverb2Effect.cpp
    ${COMMON}/dsp/effects/BBDEnsembleEffect.cpp
    ${COMMON}/dsp/effects/NimbusEffect.cpp
    ${COMMON}/dsp/effects/chowdsp/bbd_utils/BBDDelayLine.cpp
    ${COMMON}/dsp/filters/VectorizedSVFilter.cpp
    dsp/table_data.cpp
    ${COMMON}/dsp/Wavetable.cpp
    ${COMMON}/dsp/oscillators/WindowOscillator.cpp
    ${COMMON}/dsp/effects/chowdsp/exciter/LevelDetector.cpp
    ${TABLE_RESOURCES}
    ${COMMON}/dsp/oscillators/WavetableOscillator.cpp
    ${COMMON}/dsp/oscillators/AliasOscillator.cpp
    ${COMMON}/dsp/oscillators/StringOscillator.cpp
    ${COMMON}/dsp/oscillators/TwistOscillator.cpp
    ${PLAITS_SOURCES}
    ${EURORACK}/plaits/resources.cc
    ${EURORACK}/stmlib/dsp/units.cc
    ${EURORACK}/stmlib/dsp/atan.cc
    ${EURORACK}/stmlib/utils/random.cc
    dsp/filter_dsp.cpp
    dsp/utility_dsp.cpp
    dsp/envelope_dsp.cpp
    dsp/quad_lfo_dsp.cpp
    dsp/eg_vca_dsp.cpp
    dsp/unison_dsp.cpp
    ${COMMON}/dsp/oscillators/ClassicOscillator.cpp
    ${COMMON}/dsp/oscillators/SampleAndHoldOscillator.cpp
    ${COMMON}/dsp/oscillators/FM2Oscillator.cpp
    ${COMMON}/dsp/oscillators/FM3Oscillator.cpp
    ${COMMON}/dsp/oscillators/ModernOscillator.cpp
    ${COMMON}/dsp/oscillators/SineOscillator.cpp)
# Only the Effect base class; the port instantiates each effect directly.
set_source_files_properties(${COMMON}/dsp/Effect.cpp PROPERTIES COMPILE_DEFINITIONS SURGE_EFFECT_BASE_ONLY)
set_target_properties(surge-mm-dsp PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
# Delay capacities sized for the 192 kHz ceiling instead of Surge's desktop ones:
# Reverb2's allpasses (4.7 MB less per voice) and Spring's main delay and allpasses.
target_compile_definitions(surge-mm-dsp PRIVATE SURGE_COMPILE_BLOCK_SIZE=8 TEST=1
    SURGE_REVERB2_MAX_ALLPASS_LEN=32768 SURGE_SPRING_DELAY_SIZE=32768
    SURGE_SPRING_ALLPASS_DELAY_SIZE=1024)
target_include_directories(surge-mm-dsp PRIVATE
    ${SURGE_DIR}/libs/libsamplerate/include
    ${EFFECT_COMPAT} ${COMMON}/dsp/effects ${EURORACK} ${SURGE_DIR}/libs/eurorack/special_include
    dsp/compat ${COMMON} ${COMMON}/dsp ${COMMON}/dsp/oscillators ${COMMON}/dsp/utilities
    ${COMMON}/dsp/vembertech ${COMMON}/dsp/filters ${SURGE_DIR}/src ${SURGE_DIR}/libs/simde
    ${SURGE_DIR}/libs/sst/sst-basic-blocks/include
    ${SURGE_DIR}/libs/sst/sst-effects/include
    ${SURGE_DIR}/libs/fmt/include
    ${SURGE_DIR}/libs/sst/sst-filters/include
    ${SURGE_DIR}/libs/sst/sst-waveshapers/include)
target_include_directories(surge-mm-dsp PUBLIC dsp)
add_library(SurgeXTNative STATIC plugin.cpp)
set_target_properties(SurgeXTNative PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES)
target_link_libraries(SurgeXTNative PRIVATE surge-mm-dsp)
if(NOT INSTALL_DIR)
    set(INSTALL_DIR "${CMAKE_BINARY_DIR}/metamodule-plugins")
endif()
create_plugin(SOURCE_LIB SurgeXTNative PLUGIN_NAME SurgeXTNative
    PLUGIN_JSON ${CMAKE_CURRENT_LIST_DIR}/plugin.json
    SOURCE_ASSETS ${CMAKE_CURRENT_LIST_DIR}/assets DESTINATION ${INSTALL_DIR})
if(TARGET arch_mp15x_a7)
    # The SDK's custom ELF link consumes surge-mm-dsp objects but only
    # declares the top-level archive as a file dependency. A DSP-only edit
    # must relink the ELF before packaging, even if plugin.cpp is unchanged.
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/SurgeXTNative-debug.so"
        APPEND DEPENDS surge-mm-dsp "$<TARGET_FILE:surge-mm-dsp>")
endif()
