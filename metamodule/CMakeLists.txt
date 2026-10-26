cmake_minimum_required(VERSION 3.22)

if(NOT "${METAMODULE_SDK_DIR}" STREQUAL "")
	message("METAMODULE_SDK_DIR set by CMake variable ${METAMODULE_SDK_DIR}")
elseif (DEFINED ENV{METAMODULE_SDK_DIR})
    set(METAMODULE_SDK_DIR "$ENV{METAMODULE_SDK_DIR}")
	message("METAMODULE_SDK_DIR set by environment variable ${METAMODULE_SDK_DIR}")
else()
    set(METAMODULE_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../metamodule-plugin-sdk")
	message("METAMODULE_SDK_DIR set to default: ${METAMODULE_SDK_DIR}")
endif()

include(${METAMODULE_SDK_DIR}/plugin.cmake)

project(SurgeXTMetaModulePlugin
    DESCRIPTION "SurgeXT Plugin for MetaModule"
    LANGUAGES   C CXX ASM
)

set(SURGE_RACK_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# The Surge engine (surge-common and its libraries), configured for MetaModule
include(${CMAKE_CURRENT_LIST_DIR}/cmake/surge-engine.cmake)

add_library(SurgeXT STATIC)

target_sources(SurgeXT PRIVATE
    ${SURGE_RACK_DIR}/src/Delay.cpp
    ${SURGE_RACK_DIR}/src/DelayLineByFreq.cpp
    ${SURGE_RACK_DIR}/src/DelayLineByFreqExpanded.cpp
    ${SURGE_RACK_DIR}/src/DigitalRingMod.cpp
    ${SURGE_RACK_DIR}/src/EGxVCA.cpp
    ${SURGE_RACK_DIR}/src/FX.cpp
    ${SURGE_RACK_DIR}/src/LFO.cpp
    ${SURGE_RACK_DIR}/src/Mixer.cpp
    ${SURGE_RACK_DIR}/src/ModMatrix.cpp
    ${SURGE_RACK_DIR}/src/QuadAD.cpp
    ${SURGE_RACK_DIR}/src/QuadLFO.cpp
    ${SURGE_RACK_DIR}/src/SurgeXT.cpp
    ${SURGE_RACK_DIR}/src/UnisonHelper.cpp
    ${SURGE_RACK_DIR}/src/VCF.cpp
    ${SURGE_RACK_DIR}/src/VCO.cpp
    ${SURGE_RACK_DIR}/src/Waveshaper.cpp
    ${SURGE_RACK_DIR}/src/XTModule.cpp
    ${SURGE_RACK_DIR}/src/XTModuleWidget.cpp
    ${SURGE_RACK_DIR}/src/XTStyle.cpp
    ${SURGE_RACK_DIR}/src/XTWidgets.cpp
)

# sst-rackhelpers is header-only. It's not linked as a library, since create_plugin()
# would try to package the objects of an INTERFACE library.
target_include_directories(SurgeXT PRIVATE
    ${SURGE_RACK_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}
    ${SURGE_RACK_DIR}/libs/sst-rackhelpers/include
)

# The SDK requires C++20. Surge needs u8string() to return std::string, as in Rack's C++20 build.
set_property(TARGET SurgeXT PROPERTY CXX_STANDARD 20)
target_compile_options(SurgeXT PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-char8_t>)

# Same as the Rack build (sst-rackhelpers and the Surge headers compare int with size_t). And the
# modules add enums of different types for param ids, which is deprecated in C++20 only (the Rack
# build is C++17).
target_compile_options(SurgeXT PRIVATE -Wno-sign-compare
                       "$<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion;-Wno-deprecated-enum-float-conversion>")

# create_plugin() packages the objects of the libraries linked directly to SurgeXT
# (not their dependencies), so every static library in the engine is listed here.
target_link_libraries(SurgeXT PRIVATE ${SURGE_MM_ENGINE_LIBS})

#######################################

if("${INSTALL_DIR}" STREQUAL "")
    set(INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/metamodule-plugins)
endif()

# The plugin's assets: metamodule/assets (faceplates, etc.) and the Surge data that the
# Rack plugin also ships in build/surge-data/ (see the root Makefile's dist target)
set(SURGE_MM_ASSETS ${CMAKE_CURRENT_BINARY_DIR}/assets)
file(GLOB_RECURSE SURGE_MM_ASSET_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/assets/*)
file(REMOVE_RECURSE ${SURGE_MM_ASSETS})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/assets/ DESTINATION ${SURGE_MM_ASSETS})
# The plugin's license notice, at the top level of the .mmplugin
file(COPY ${CMAKE_CURRENT_LIST_DIR}/LICENSE.txt DESTINATION ${SURGE_MM_ASSETS})
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/LICENSE.txt)
file(COPY
    ${SURGE_RACK_DIR}/surge/resources/surge-shared/configuration.xml
    ${SURGE_RACK_DIR}/surge/resources/surge-shared/windows.wt
    ${SURGE_RACK_DIR}/surge/resources/data/wavetables
    ${SURGE_RACK_DIR}/surge/resources/data/fx_presets
    ${SURGE_RACK_DIR}/res/surge_extra_data/fx_presets
    DESTINATION ${SURGE_MM_ASSETS}/build/surge-data)
# An index of the Surge data, which ghc::filesystem lists directories from (see
# compat/posix/asset_dirs.h): each path relative to build/surge-data, whether it's a directory, and
# its size. Written only when it changes, so the plugin isn't rebuilt on every configure.
set(SURGE_MM_DATA ${SURGE_MM_ASSETS}/build/surge-data)
file(GLOB_RECURSE SURGE_MM_DATA_PATHS LIST_DIRECTORIES true RELATIVE ${SURGE_MM_DATA} ${SURGE_MM_DATA}/*)
list(SORT SURGE_MM_DATA_PATHS)
set(SURGE_MM_DATA_INDEX "")
foreach(path IN LISTS SURGE_MM_DATA_PATHS)
    if(IS_DIRECTORY ${SURGE_MM_DATA}/${path})
        set(is_dir true)
        set(size 0)
    else()
        set(is_dir false)
        file(SIZE ${SURGE_MM_DATA}/${path} size)
    endif()
    string(REPLACE "\\" "\\\\" path "${path}")
    string(REPLACE "\"" "\\\"" path "${path}")
    string(APPEND SURGE_MM_DATA_INDEX "{\"${path}\", ${is_dir}, ${size}},\n")
endforeach()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/surge_data_index.inc.tmp "${SURGE_MM_DATA_INDEX}")
configure_file(${CMAKE_CURRENT_BINARY_DIR}/surge_data_index.inc.tmp
    ${CMAKE_CURRENT_BINARY_DIR}/gen/surge_data_index.inc COPYONLY)
target_sources(SurgeXT PRIVATE ${CMAKE_CURRENT_LIST_DIR}/compat/posix/asset_dirs.cc)
target_include_directories(SurgeXT PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen)

# The font the LCDs draw text with (the firmware loads TrueType fonts only), and its license
file(COPY ${SURGE_RACK_DIR}/res/xt/fonts/quicksand/Quicksand-Bold.ttf
    "${SURGE_RACK_DIR}/res/xt/fonts/quicksand/SIL Open Font License.txt"
    DESTINATION ${SURGE_MM_ASSETS}/xt/fonts/quicksand)

# create_plugin() makes a presets/ dir next to the plugin.json it is given,
# so give it a copy in the build dir instead of the Rack plugin's plugin.json.
configure_file(${SURGE_RACK_DIR}/plugin.json ${CMAKE_CURRENT_BINARY_DIR}/rack-plugin/plugin.json COPYONLY)

create_plugin(
    SOURCE_LIB      SurgeXT
    PLUGIN_NAME     SurgeXTRack
    PLUGIN_JSON     ${CMAKE_CURRENT_BINARY_DIR}/rack-plugin/plugin.json
    SOURCE_ASSETS   ${SURGE_MM_ASSETS}
    DESTINATION     ${INSTALL_DIR}
)
