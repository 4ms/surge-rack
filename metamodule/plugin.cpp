// SPDX-License-Identifier: GPL-3.0-or-later
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/register_module.hh"
#include "engine.h"
#include <array>
#include <string>

#include "oscillator_processor.h"
#include "oscillator_panel.h"
#include "utility_panels.h"
#include "modulation_panels.h"
#include "quad_lfo_panel.h"
#include "eg_vca_panel.h"
#include "filter_panels.h"
#include "unison_panels.h"
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
    register_mixer<false>();
    register_mixer<true>();
    register_digital_ring();
    register_mod_matrix();
    register_quad_ad();
    register_quad_lfo();
    register_eg_vca();
    register_filter<surge_mm::FilterModule::VCF>();
    register_filter<surge_mm::FilterModule::Waveshaper>();
    register_unison_helper();
    register_unison_expander();
}
