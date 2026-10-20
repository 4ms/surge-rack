// SPDX-License-Identifier: GPL-3.0-or-later
// The interpolated decibel table from SurgeStorage.cpp (table_dB, db_to_linear).
#pragma once
#include <cmath>
namespace surge_mm
{
struct DecibelTable
{
    float values[512];
    DecibelTable()
    {
        for (int i = 0; i < 512; ++i)
            values[i] = std::pow(10.f, .05f * (float(i) - 384));
    }
    static const DecibelTable &get()
    {
        static const DecibelTable table;
        return table;
    }
};
inline float db_to_linear(float x)
{
    x += 384;
    int e = int(x);
    float a = x - e;
    const auto &v = DecibelTable::get().values;
    return (1 - a) * v[e & 511] + a * v[(e + 1) & 511];
}
} // namespace surge_mm
