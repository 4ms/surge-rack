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

# Surge's version.cpp is generated from the top-level project's version (as in the Rack build)
project(SurgeXTMetaModulePlugin VERSION 1.0
    DESCRIPTION "SurgeXT Plugin for MetaModule"
    LANGUAGES   C CXX ASM
)

set(SURGE_RACK_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# The Surge engine (surge-common and its libraries), configured for MetaModule
include(${CMAKE_CURRENT_LIST_DIR}/cmake/surge-engine.cmake)

add_library(SurgeXT STATIC)

target_sources(SurgeXT PRIVATE
    bringup_init.cpp
)

target_include_directories(SurgeXT PRIVATE
    ${SURGE_RACK_DIR}/src
)

# The SDK requires C++20. Surge needs u8string() to return std::string, as in Rack's C++20 build.
set_property(TARGET SurgeXT PROPERTY CXX_STANDARD 20)
target_compile_options(SurgeXT PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-char8_t>)

# create_plugin() packages the objects of the libraries linked directly to SurgeXT
# (not their dependencies), so every static library in the engine is listed here.
target_link_libraries(SurgeXT PRIVATE ${SURGE_MM_ENGINE_LIBS})

#######################################

if("${INSTALL_DIR}" STREQUAL "")
    set(INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/metamodule-plugins)
endif()

# create_plugin() makes a presets/ dir next to the plugin.json it is given,
# so give it a copy in the build dir instead of the Rack plugin's plugin.json.
configure_file(${SURGE_RACK_DIR}/plugin.json ${CMAKE_CURRENT_BINARY_DIR}/rack-plugin/plugin.json COPYONLY)

create_plugin(
    SOURCE_LIB      SurgeXT
    PLUGIN_NAME     SurgeXTRack
    PLUGIN_JSON     ${CMAKE_CURRENT_BINARY_DIR}/rack-plugin/plugin.json
    SOURCE_ASSETS   ${CMAKE_CURRENT_LIST_DIR}/assets
    DESTINATION     ${INSTALL_DIR}
)
