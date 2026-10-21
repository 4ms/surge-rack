// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "rack_trigger.h"
#include <cmath>
#include <limits>

namespace surge_mm
{
// ClockProcessor in src/XTModule.h retains the last tempo and Schmitt state
// through disconnects and format changes. Keep those rules for imported patches.
struct RackClock
{
    RackTrigger gate;
    int elapsed{-1};
    float ratio{1}, inverse{1}, last_bpm{-1}, last_volts{-11};
    bool connected{}, synced{};
    // Returns true when the clock starts or stops driving the tempo (where Rack's
    // QuadLFO resets its amplitudes).
    bool process(float volts, bool patched, bool bpm, float sample_rate)
    {
        if (!patched)
        {
            bool changed = elapsed >= 0 || connected;
            if (changed)
                synced = false;
            elapsed = -1;
            connected = false;
            return changed;
        }
        bool changed = false;
        if (bpm)
        {
            if (!connected)
            {
                synced = true;
                changed = true;
            }
            connected = true;
            if (volts != last_volts)
            {
                // Rack computes both fields from the double-precision ratio.
                double value = std::pow(2.0, volts);
                ratio = value;
                inverse = 1.f / value;
            }
            last_volts = volts;
        }
        else
        {
            if (gate.process(volts))
            {
                if (elapsed > 0 && elapsed < sample_rate * 6)
                {
                    float rate = 60 * sample_rate / elapsed;
                    if (std::abs(rate - std::round(rate)) < .015f)
                        rate = std::round(rate);
                    if (rate != last_bpm)
                    {
                        ratio = rate / 120;
                        inverse = 120 / rate;
                    }
                    last_bpm = rate;
                }
                else
                {
                    synced = true;
                    changed = true;
                }
                elapsed = 0;
            }
            // Saturate after a very long stopped clock rather than overflowing.
            if (elapsed >= 0 && elapsed < std::numeric_limits<int>::max())
                ++elapsed;
        }
        return changed;
    }
};
} // namespace surge_mm
