// SPDX-License-Identifier: GPL-3.0-or-later
// Embedded stand-in for the few JUCE types used by Surge's chowdsp code (delay lines and
// the Spring Reverb's SIMD allpasses). Storage is a std::vector; no JUCE is linked.
#pragma once
#include "globals.h" // __m128, through SIMDe on ARM
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#define jassert(x) ((void)0)
#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(className)                                    \
    className(const className &) = delete;                                                         \
    className &operator=(const className &) = delete;

namespace juce
{
using uint32 = uint32_t;
template <class T> T jmax(T a, T b) { return std::max(a, b); }
template <class T> T jlimit(T lo, T hi, T x) { return std::clamp(x, lo, hi); }
template <class T> bool isPositiveAndNotGreaterThan(T value, T upper)
{
    return value >= T(0) && value <= upper;
}
// Resizing to the same size keeps the allocation, so re-preparing does not allocate.
template <class T> struct HeapBlock
{
    std::vector<T> data;
};
namespace dsp
{
struct ProcessSpec
{
    double sampleRate;
    uint32_t maximumBlockSize, numChannels;
};
template <class T> class AudioBlock
{
    T *data{};
    size_t channels{}, samples{};

  public:
    AudioBlock() = default;
    AudioBlock(HeapBlock<char> &heap, size_t numChannels, size_t numSamples)
        : channels(numChannels), samples(numSamples)
    {
        heap.data.resize(numChannels * numSamples * sizeof(T));
        data = reinterpret_cast<T *>(heap.data.data());
    }
    size_t getNumChannels() const { return channels; }
    size_t getNumSamples() const { return samples; }
    T *getChannelPointer(size_t channel) const { return data + channel * samples; }
    void clear()
    {
        if (data)
            std::fill_n(data, channels * samples, T{});
    }
    void copyFrom(const AudioBlock &other) { std::copy_n(other.data, channels * samples, data); }
};
template <class T> struct SIMDRegister;
// Four float lanes with the operations the allpasses use. The value is a native register
// (GCC otherwise copies an aggregate through scalar registers and the stack on ARM), but
// only float-aligned, because delay-line storage is a byte buffer.
template <> struct SIMDRegister<float>
{
    using value_type = float;
    using ElementType = float;
    typedef __m128 Register __attribute__((aligned(4)));
    Register value;
    SIMDRegister() : value(_mm_setzero_ps()) {}
    SIMDRegister(float x) : value(_mm_set1_ps(x)) {}
    explicit SIMDRegister(__m128 x) : value(x) {}
    static SIMDRegister fromRawArray(const float *p) { return SIMDRegister(_mm_loadu_ps(p)); }
    void copyToRawArray(float *p) const { _mm_storeu_ps(p, value); }
    float &operator[](size_t i) { return reinterpret_cast<float *>(&value)[i]; }
    SIMDRegister &operator+=(SIMDRegister b) { return *this = *this + b; }
    friend SIMDRegister operator+(SIMDRegister a, SIMDRegister b)
    {
        return SIMDRegister(_mm_add_ps(a.value, b.value));
    }
    friend SIMDRegister operator-(SIMDRegister a, SIMDRegister b)
    {
        return SIMDRegister(_mm_sub_ps(a.value, b.value));
    }
    friend SIMDRegister operator*(SIMDRegister a, SIMDRegister b)
    {
        return SIMDRegister(_mm_mul_ps(a.value, b.value));
    }
};
static_assert(sizeof(SIMDRegister<float>) == 16 && alignof(SIMDRegister<float>) == alignof(float));
} // namespace dsp
} // namespace juce
