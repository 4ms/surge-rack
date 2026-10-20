// SPDX-License-Identifier: GPL-3.0-or-later
// Embedded stand-in for Surge's SurgeMemoryPools.h, which pulls in the full SurgeStorage.
// Only String uses a pool; it is plain heap allocation here.
#pragma once
#include "SurgeStorage.h"
#include "SSESincDelayLine.h"
namespace Surge::Memory
{
struct SurgeMemoryPools
{
    struct StringDelayLines
    {
        SSESincDelayLine<16384> *getItem(const float *sinctable)
        {
            return new SSESincDelayLine<16384>(sinctable);
        }
        void returnItem(SSESincDelayLine<16384> *line) { delete line; }
    } stringDelayLines;
};
} // namespace Surge::Memory
