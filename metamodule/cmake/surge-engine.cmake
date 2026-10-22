# Builds the Surge engine for MetaModule from the surge submodule's own CMake,
# with the same options the Rack plugin uses (see the root CMakeLists.txt).
#
# This is used both for hardware (the SDK's ARM toolchain) and for the simulator,
# which builds the plugin natively as a built-in brand.
#
# Sets SURGE_MM_ENGINE_LIBS: the static libraries the plugin must link directly.

if(TARGET arch_mp15x_a7)
    set(SURGE_MM_HARDWARE TRUE)
else()
    set(SURGE_MM_HARDWARE FALSE)
endif()

set(CMAKE_POLICY_VERSION_MINIMUM "3.5" CACHE STRING "Fix up libsamplerate")

set(SURGE_BUILD_32BIT_LINUX TRUE CACHE BOOL "Allow 32 bit to try")
set(SURGE_SKIP_JUCE_FOR_RACK TRUE CACHE BOOL "skip surge Juce build")
set(SURGE_SKIP_JUCE_DSP TRUE CACHE BOOL "skip juce dsp")
set(SURGE_SKIP_LUA TRUE CACHE BOOL "skip surge Lua build")
set(SURGE_SKIP_AIRWINDOWS TRUE CACHE BOOL "skip surge airwindows build")
set(SURGE_SKIP_ODDSOUND_MTS TRUE CACHE BOOL "skip surge oddsound-mts build")
set(SURGE_SKIP_VST3 TRUE CACHE BOOL "skip surge VST3 build")
set(SURGE_SKIP_ALSA TRUE CACHE BOOL "skip surge Alsa build")
set(SURGE_SKIP_STANDALONE TRUE CACHE BOOL "skip surge standalone build")
set(SURGE_SKIP_PIE_CHANGE TRUE CACHE BOOL "skip pie")
set(SURGE_SKIP_WERROR TRUE CACHE BOOL "skip surge -Werror")
set(SURGE_BUILD_TESTRUNNER FALSE CACHE BOOL "skip surge tests")
set(SURGE_COMPILE_BLOCK_SIZE 8 CACHE STRING "surge compile block size")

# Use the SDK's simde, which rack-interface also uses (as the Rack build uses the Rack SDK's simde).
# The simulator's stand-in SDK dir doesn't have it, so use the firmware's copy of the SDK.
if(DEFINED METAMODULE_FW_DIR)
    set(SURGE_MM_SDK_DIR ${METAMODULE_FW_DIR}/metamodule-plugin-sdk)
else()
    set(SURGE_MM_SDK_DIR ${METAMODULE_SDK_DIR})
endif()
set(SURGE_SIMDE_PATH "${SURGE_MM_SDK_DIR}/rack-interface/dep/include" CACHE STRING "surge simde is the MetaModule SDK simde")

# The firmware does not export std::filesystem, so build ghc::filesystem into the plugin
set(SST_PLUGININFRA_FILESYSTEM_FORCE_GHC ON CACHE BOOL "Force GHC filesystem implementation")

# newlib only defines M_PI etc. when non-ANSI names are visible
add_compile_definitions(_DEFAULT_SOURCE)

# _stricmp, and on hardware std::mutex etc. (see compat/metamodule_prefix.hh)
set(SURGE_MM_COMPAT_DIR ${CMAKE_CURRENT_LIST_DIR}/../compat)
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include ${SURGE_MM_COMPAT_DIR}/metamodule_prefix.hh>")
if(SURGE_MM_HARDWARE)
    add_compile_definitions(SURGE_MM_NO_GTHREADS)
endif()

# Surge generates version.cpp from the top-level project's version, which is the simulator's
# project in simulator builds. Use the Rack build's project version (1.0).
set(CMAKE_PROJECT_VERSION_MAJOR 1)
set(CMAKE_PROJECT_VERSION_MINOR 0)

# The simulator's firmware already has a pffft target (surge-common doesn't use it)
if(TARGET pffft)
    set(SURGE_LIBS_PROVIDED_BY_PARENT pffft)
endif()

add_subdirectory(${SURGE_RACK_DIR}/surge ${CMAKE_CURRENT_BINARY_DIR}/surge EXCLUDE_FROM_ALL)

if(SURGE_MM_HARDWARE)
    # ghc::filesystem does not recognize the MetaModule; it provides POSIX calls like Linux,
    # with the SDK's dirent.h (which has no d_type) and a stub statvfs()
    target_compile_definitions(filesystem PUBLIC GHC_OS_DETECTED GHC_OS_LINUX GHC_NO_DIRENT_D_TYPE)
    target_include_directories(filesystem BEFORE PRIVATE
        ${SURGE_MM_SDK_DIR}/core-interface/filesystem
        ${SURGE_MM_COMPAT_DIR}/posix)
    target_compile_options(filesystem PRIVATE "SHELL:-include ${SURGE_MM_COMPAT_DIR}/posix/ghc_prefix.h")
endif()

# surge-common only links simde for APPLE/UNIX/WIN32 targets
target_link_libraries(surge-common PUBLIC surge::simde)

# Build the engine as no desktop platform (not MAC, LINUX or WINDOWS), including in the simulator,
# so it doesn't use the OS paths or other platform code.
foreach(prop COMPILE_DEFINITIONS INTERFACE_COMPILE_DEFINITIONS)
    get_target_property(defs surge-common ${prop})
    if(defs)
        list(REMOVE_ITEM defs MAC=1 LINUX=1 WINDOWS=1)
        set_target_properties(surge-common PROPERTIES ${prop} "${defs}")
    endif()
endforeach()

# Not used by the Rack modules, and they need an OS (threads, dlfcn, sqlite's VFS) or JUCE:
# - PatchDB (patch browser database, sqlite)
# - SurgeSynthesizer (the synth itself, threads)
# - sst-plugininfra's platform code (paths, misc); its filesystem, tinyxml and strnatcmp are kept
# - sst-filters-extras (filter plotting with JUCE; its header is used with the compat JUCE)
foreach(prop LINK_LIBRARIES INTERFACE_LINK_LIBRARIES)
    get_target_property(libs surge-common ${prop})
    list(REMOVE_ITEM libs surge::sqlite sst-plugininfra sst-filters-extras)
    set_target_properties(surge-common PROPERTIES ${prop} "${libs}")
endforeach()
get_target_property(srcs surge-common SOURCES)
list(REMOVE_ITEM srcs
    PatchDB.cpp
    PatchDBQueryParser.cpp
    SurgeSynthesizer.cpp
    SurgeSynthesizerIO.cpp
)
set_target_properties(surge-common PROPERTIES SOURCES "${srcs}")
target_include_directories(surge-common PUBLIC ${SURGE_RACK_DIR}/surge/libs/sst/sst-plugininfra/include)

# The VCO and VCF modules use FPUStateGuard from sst-plugininfra's cpufeatures, which
# needs no platform code without MAC, LINUX or WINDOWS (it's a no-op on 32-bit ARM)
target_sources(surge-common PRIVATE ${SURGE_RACK_DIR}/surge/libs/sst/sst-plugininfra/src/cpufeatures.cpp)

# SurgeStorage still makes a PatchDB, so stub it
target_sources(surge-common PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../stubs/PatchDB.cpp)

# POSIX calls that ghc::filesystem uses and the firmware does not provide
if(SURGE_MM_HARDWARE)
    target_sources(surge-common PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../stubs/posix.c)
endif()

# A few chowdsp effects (Spring Reverb, Neuron) use JUCE's delay line types
target_include_directories(surge-common PUBLIC ${SURGE_MM_COMPAT_DIR})

# The VCF's response curve (DSPExternalAdapterUtils) uses sst-filters' FilterPlotter, with the
# compat juce::dsp::FFT
target_include_directories(surge-common PRIVATE ${SURGE_RACK_DIR}/surge/libs/sst/sst-filters/include-extras)

# Same warning suppressions as the Rack build
target_compile_options(surge-common PRIVATE -Wno-sign-compare -Wno-ignored-qualifiers -Wno-unused-value -Wno-uninitialized
                       -Wno-implicit-fallthrough -Wno-char-subscripts -Wno-unused-local-typedefs -Wno-range-loop-construct
                       -Wno-nonnull -Wno-reorder
                       $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-but-set-variable -Wno-stringop-truncation -Wno-class-memaccess
                         -Wno-maybe-uninitialized -Wno-dangling-reference -Wno-restrict>)
target_compile_options(eurorack PRIVATE -Wno-unused-local-typedefs)
target_compile_options(sst-waveshapers INTERFACE -Wno-unused-variable)
target_compile_options(sst-filters INTERFACE -Wno-unused-variable)
target_compile_options(tinyxml PRIVATE -Wno-implicit-fallthrough -Wno-sign-compare)
target_compile_options(tuning-library INTERFACE $<$<CXX_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>)

set(SURGE_MM_ENGINE_LIBS
    surge-common
    airwindows
    eurorack
    samplerate
    fmt
    tinyxml
    strnatcmp
    filesystem
)
