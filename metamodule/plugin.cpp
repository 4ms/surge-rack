// SPDX-License-Identifier: GPL-3.0-or-later
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/register_module.hh"
#include "engine.h"
#include <array>
#include <string>

#include "oscillator_processor.h"
#include "oscillator_panel.h"
extern "C" void init()
{
    register_direct_oscillator<surge_mm::Model::Classic>("Classic", "SurgeXTNative/Classic.png");
    register_direct_oscillator<surge_mm::Model::Modern>("Modern", "SurgeXTNative/Modern.png");
    register_direct_oscillator<surge_mm::Model::Sine>("Sine", "SurgeXTNative/Sine.png");
    register_direct_oscillator<surge_mm::Model::FM2>("FM2", "SurgeXTNative/FM2.png");
    register_direct_oscillator<surge_mm::Model::FM3>("FM3", "SurgeXTNative/FM3.png");
    register_direct_oscillator<surge_mm::Model::SHNoise>("SHNoise", "SurgeXTNative/SHNoise.png");
    register_direct_oscillator<surge_mm::Model::Alias>("Alias", "SurgeXTNative/Alias.png");
    register_direct_oscillator<surge_mm::Model::String>("String", "SurgeXTNative/String.png");
    register_direct_oscillator<surge_mm::Model::Twist>("Twist", "SurgeXTNative/Twist.png");
    register_direct_oscillator<surge_mm::Model::Wavetable>("Wavetable",
                                                           "SurgeXTNative/Wavetable.png");
    register_direct_oscillator<surge_mm::Model::Window>("Window", "SurgeXTNative/Window.png");
}
