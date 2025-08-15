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

add_library(SurgeXT STATIC)

set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

target_sources(SurgeXT
    PRIVATE
    ${SOURCE_DIR}/src/VCO.cpp
)

add_subdirectory(${SOURCE_DIR}/libs/sst-rackhelpers ${CMAKE_CURRENT_BINARY_DIR}/sst-rackhelpers)

set(SURGE_BUILD_32BIT_LINUX TRUE CACHE BOOL "Allow 32 bit to try")
set(SURGE_SKIP_JUCE_FOR_RACK TRUE CACHE BOOL "skip surge Juce build")
set(SURGE_SKIP_LUA TRUE CACHE BOOL "skip surge Lua build")
set(SURGE_SKIP_AIRWINDOWS TRUE CACHE BOOL "skip surge airwindows build")
set(SURGE_SKIP_ODDSOUND_MTS TRUE CACHE BOOL "skip surge oddsound-mts build")
set(SURGE_SKIP_VST3 TRUE CACHE BOOL "skip surge VST3 build")
set(SURGE_SKIP_ALSA TRUE CACHE BOOL "skip surge Alsa build")
set(SURGE_SKIP_STANDALONE TRUE CACHE BOOL "skip surge standalone build")
set(SURGE_COMPILE_BLOCK_SIZE 8 CACHE STRING "surge compile block size")

add_subdirectory(${SOURCE_DIR}/surge ${CMAKE_CURRENT_BINARY_DIR}/surge)

target_include_directories(SurgeXT PRIVATE
    ${SOURCE_DIR}/src
    # ${SOURCE_DIR}/surge/src/common
)

set_property(TARGET SurgeXT PROPERTY CXX_STANDARD 23)


target_link_libraries(SurgeXT INTERFACE 
    surge-common 
    # juce_dsp_rack_sub 
    # tinyxml
    # strnatcmp 
    # samplerate 
    # sst-plugininfra 
    # fmt 
    # sqlite 
    # eurorack 
    # airwindows 
    sst-basic-blocks
)

#######################################

if("${INSTALL_DIR}" STREQUAL "")
    set(INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/metamodule-plugins)
endif()

create_plugin(
    SOURCE_LIB      SurgeXT                                          # This is the cmake target name (defined in add_target)
    PLUGIN_NAME     SurgeXT                                          # This must match the brand "slug" used in VCV Rack
    PLUGIN_JSON     ${SOURCE_DIR}/plugin.json                      # Path to the plugin.json file used by VCV Rack
    SOURCE_ASSETS   ${CMAKE_CURRENT_LIST_DIR}/assets               # Path to the assets/ dir containing the PNGs
    DESTINATION     ${INSTALL_DIR}                                 # Dir where plugin .mmplugin file should be put
)

