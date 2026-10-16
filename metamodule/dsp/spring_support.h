// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <sst/utilities/globals.h>
#include "effect_config.h"
#include <sst/basic-blocks/dsp/BlockInterpolators.h>
#include <sst/basic-blocks/dsp/FastMath.h>
#include <sst/basic-blocks/mechanics/block-ops.h>
#include <chowdsp/shared/chowdsp_DelayLine.h>
#include <chowdsp/shared/SmoothedValue.h>
#include <array>
#include <random>
namespace surge_mm::spring
{
constexpr int BLOCK_SIZE = 8, BLOCK_SIZE_QUAD = 2;
using lipol_ps_blocksz = sst::basic_blocks::dsp::lipol_sse<8, false>;
using chowdsp::DelayLine;
using chowdsp::SmoothedValue;
namespace DelayLineInterpolationTypes = chowdsp::DelayLineInterpolationTypes;
namespace ValueSmoothingTypes = chowdsp::ValueSmoothingTypes;
namespace mech = sst::basic_blocks::mechanics;
inline float clamp01(float x)
{
    return std::clamp(x, 0.f, 1.f);
}
inline double dB_to_linear(double x)
{
    return std::pow(10.0, .05 * x);
}
inline float vSum(__m128 x)
{
    auto a = _mm_add_ps(x, _mm_movehl_ps(x, x));
    a = _mm_add_ss(a, _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 1)));
    float f;
    _mm_store_ss(&f, a);
    return f;
}
// Four lanes with JUCE's arithmetic but no desktop JUCE dependency. Alignment
// stays at float alignment so the shared byte-backed Chow buffer is sufficient.
struct SpringVector
{
    using value_type = float;
    float lane[4]{};
    SpringVector() = default;
    SpringVector(float x)
    {
        std::fill_n(lane, 4, x);
    }
    static SpringVector fromRawArray(const float *p)
    {
        SpringVector r;
        _mm_storeu_ps(r.lane, _mm_loadu_ps(p));
        return r;
    }
    void copyToRawArray(float *p) const
    {
        _mm_storeu_ps(p, _mm_loadu_ps(lane));
    }
    float &operator[](size_t i)
    {
        return lane[i];
    }
    SpringVector &operator+=(SpringVector b)
    {
        *this = *this + b;
        return *this;
    }
    friend SpringVector operator+(SpringVector a, SpringVector b)
    {
        SpringVector r;
        _mm_storeu_ps(r.lane, _mm_add_ps(_mm_loadu_ps(a.lane), _mm_loadu_ps(b.lane)));
        return r;
    }
    friend SpringVector operator-(SpringVector a, SpringVector b)
    {
        SpringVector r;
        _mm_storeu_ps(r.lane, _mm_sub_ps(_mm_loadu_ps(a.lane), _mm_loadu_ps(b.lane)));
        return r;
    }
    friend SpringVector operator*(SpringVector a, SpringVector b)
    {
        SpringVector r;
        _mm_storeu_ps(r.lane, _mm_mul_ps(_mm_loadu_ps(a.lane), _mm_loadu_ps(b.lane)));
        return r;
    }
};
static_assert(sizeof(SpringVector) == 16 && alignof(SpringVector) == alignof(float));
// Bounds include the maximum parameter values at the highest accepted rate.
static_assert((1000 + .099 * 192000) * 1.07 < 32768);
// Reflection taps retain the original full ring, with one shared history.
static_assert(.00335 * 192000 < 1024);
struct SpringStorage : EffectGlobal
{
};
union SpringValue
{
    float f{};
    int i;
};
struct SpringSettings
{
};
struct SpringBase
{
    SpringStorage *storage;
    SpringSettings *fxdata;
    SpringValue *pd;
    float *pd_float[8];
    SpringBase(SpringStorage *s, SpringSettings *f, SpringValue *v) : storage(s), fxdata(f), pd(v)
    {
        for (unsigned i = 0; i < 8; ++i)
            pd_float[i] = &v[i].f;
    }
};
} // namespace surge_mm::spring
