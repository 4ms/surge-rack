// SPDX-License-Identifier: GPL-3.0-or-later
// Embedded stand-in for tuning-library's Tunings.h: MetaModule is always 12-TET.
#pragma once
#include "filter_tuning.h"
namespace Tunings
{
constexpr double MIDI_0_FREQ = surge_mm::FilterTuning::MIDI_0_FREQ;
}
