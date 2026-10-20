// SPDX-License-Identifier: GPL-3.0-or-later
// Out-of-line SurgeStorage members, from SurgeStorage.cpp.
#include "SurgeStorage.h"
#include "sst/waveshapers.h"

float SurgeStorage::lookup_waveshape(sst::waveshapers::WaveshaperType entry, float x) const
{
    x *= 32.f;
    x += 512.f;
    int e = (int)x;
    float a = x - (float)e;

    if (e > 0x3fd)
        return 1;
    if (e < 1)
        return -1;

    const auto &waveshapers = sst::waveshapers::globalWaveshaperTables.waveshapers[(int)entry];
    return (1 - a) * waveshapers[e & 0x3ff] + a * waveshapers[(e + 1) & 0x3ff];
}

float SurgeStorage::lookup_waveshape_warp(sst::waveshapers::WaveshaperType entry, float x) const
{
    x *= 256.f;
    x += 512.f;

    int e = (int)x;
    float a = x - (float)e;

    const auto &waveshapers = sst::waveshapers::globalWaveshaperTables.waveshapers[(int)entry];
    return (1 - a) * waveshapers[e & 0x3ff] + a * waveshapers[(e + 1) & 0x3ff];
}
