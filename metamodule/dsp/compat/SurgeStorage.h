// SPDX-License-Identifier: GPL-3.0-or-later
// Embedded stand-in for Surge's SurgeStorage.h, including the parts of Parameter.h,
// FxStorage and Tunings that the compiled oscillators and effects use. It shadows the
// real header because dsp/compat precedes surge/src/common on the include path.
//
// Only sample rates, the 12-TET pitch/omega, dB and envelope-rate tables, a random
// source and parameter values are provided; no patch, tuning library, filesystem or
// GUI support is linked. Table functions match SurgeStorage.cpp; control-type ranges
// are generated from Parameter.cpp into ParameterRanges.h.
#pragma once
#include "globals.h"
#include "Wavetable.h"
#include "basic_dsp.h"
#include "SurgeParamConfig.h"
#include "Tunings.h"
#include "db_table.h"
#include "filter_tuning.h"
#include "modulation_clock.h"
#include "sst/basic-blocks/tables/SincTableProvider.h"
// Surge's SurgeStorage.h provides the filter library through FilterConfiguration.h.
#include "sst/filters.h"
#include "sst/waveshapers/WaveshaperConfiguration.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>
#ifndef TXT_SIZE
#define TXT_SIZE 256
#endif

constexpr int FIRipolI16_N = 8, FIRoffsetI16 = 4;
constexpr int FIRipol_N = 12;
constexpr int FIRipol_M = 256, FIRoffset = FIRipol_N >> 1;
const int n_fx_params = 12;

union pdata
{
    int i;
    bool b;
    float f;
};
enum valtypes
{
    vt_int = 0,
    vt_bool,
    vt_float,
};
static constexpr int n_fxws = 8;
static constexpr std::array<sst::waveshapers::WaveshaperType, n_fxws> FXWaveShapers = {
    sst::waveshapers::WaveshaperType::wst_soft,    sst::waveshapers::WaveshaperType::wst_hard,
    sst::waveshapers::WaveshaperType::wst_asym,    sst::waveshapers::WaveshaperType::wst_sine,
    sst::waveshapers::WaveshaperType::wst_digital, sst::waveshapers::WaveshaperType::wst_ojd,
    sst::waveshapers::WaveshaperType::wst_fwrectify, sst::waveshapers::WaveshaperType::wst_fuzzsoft};
#include "ParameterRanges.h"

struct ParamUserData
{
    virtual ~ParamUserData() = default;
};
struct CountedSetUserData : ParamUserData
{
    virtual int getCountedSetSize() const = 0;
};
struct Parameter;
// UI-only hooks referenced by init_ctrltypes; never called on MetaModule.
struct ParameterDynamicNameFunction
{
    virtual const char *getName(const Parameter *) const = 0;
    virtual ~ParameterDynamicNameFunction() = default;
};
struct ParameterDynamicBoolFunction
{
    virtual bool getValue(const Parameter *p) const = 0;
    virtual ~ParameterDynamicBoolFunction() = default;
};
struct ParameterDynamicDeactivationFunction : ParameterDynamicBoolFunction
{
    virtual Parameter *getPrimaryDeactivationDriver(const Parameter *) const { return nullptr; }
};
struct ParameterExternalFormatter : ParamUserData
{
    virtual bool formatValue(const Parameter *, float, char *, int) const = 0;
    virtual bool formatAltValue(const Parameter *, float, char *txt, int) const
    {
        txt[0] = 0;
        return true;
    }
    virtual bool stringToValue(const Parameter *, const char *, float &) const = 0;
};
struct ParameterDiscreteIndexRemapper : ParamUserData
{
    virtual int remapStreamedIndexToDisplayIndex(int i) const = 0;
    virtual std::string nameAtStreamedIndex(int i) const = 0;
    virtual bool hasGroupNames() const { return false; }
    virtual std::string groupNameAtStreamedIndex(int) const { return ""; }
    virtual bool sortGroupNames() const { return true; }
    virtual bool useRemappedOrderingForGroupsIfNotSorted() const { return false; }
    virtual bool supportsTotalIndexOrdering() const { return false; }
    virtual const std::vector<int> totalIndexOrdering() const { return {}; }
};
enum osc_type
{
    ot_twist = 10
};
struct SurgeStorage;
namespace Surge::Memory
{
struct SurgeMemoryPools;
}

struct Parameter
{
    pdata val{}, val_min{}, val_max{}, val_default{};
    int valtype{vt_float}, ctrltype{ct_none};
    int id{}, param_id_in_scene{}, deform_type{}, scene{1}, ctrlgroup_entry{};
    int posy{}, posy_offset{};
    bool extend_range{}, absolute{}, deactivated{}, temposync{}, modulateable{};
    const char *name{""};
    struct
    {
        float a{1}, b{1};
    } displayInfo;
    ParameterDynamicNameFunction *dynamicName{};
    ParameterDynamicBoolFunction *dynamicBipolar{};
    ParameterDynamicDeactivationFunction *dynamicDeactivation{};
    SurgeStorage *storage{};

    void set_user_data(ParamUserData *) {}
    void set_extend_range(bool v) { extend_range = v; }
    void set_name(const char *n) { name = n; }
    void set_type(int t)
    {
        ctrltype = t;
        posy_offset = 0;
        dynamicName = nullptr;
        dynamicBipolar = nullptr;
        dynamicDeactivation = nullptr;
        displayInfo = {};
        // Parameter.cpp's default for types without a range
        valtype = vt_int;
        val_min.i = std::numeric_limits<int>::min();
        val_max.i = std::numeric_limits<int>::max();
        val_default.i = 0;
        set_parameter_range(*this, t);
    }
    // Parameter::get_extended for the control types in ParameterRanges.h
    float get_extended(float f) const
    {
        if (!extend_range)
            return f;
        switch (ctrltype)
        {
        case ct_freq_fm2_offset:
        case ct_freq_shift:
            return 100.f * f;
        case ct_decibel_extendable:
            return 3.f * f;
        case ct_decibel_narrow_extendable:
            return 5.f * f;
        case ct_oscspread:
        case ct_oscspread_bipolar:
            return 12.f * f;
        case ct_osc_feedback:
            return 8.f * f - 4.f * f;
        case ct_osc_feedback_negative:
            return 4.f * f;
        case ct_fmratio:
            if (f > 16)
                return ((f - 16) * 31.f / 16.f + 1);
            return -((16 - f) * 31.f / 16.f + 1);
        case ct_percent_with_extend_to_bipolar_static_default:
            return 2 * f - 1;
        default:
            return f;
        }
    }
    // Port helpers: map a 0..1 control onto the parameter's range.
    void set_normalized(float v)
    {
        v = std::clamp(v, 0.f, 1.f);
        if (valtype == vt_int)
            val.i = val_min.i + int(std::round(v * (val_max.i - val_min.i)));
        else
            val.f = val_min.f + v * (val_max.f - val_min.f);
    }
    float normalized() const
    {
        return valtype == vt_int ? float(val.i - val_min.i) / (val_max.i - val_min.i)
                                 : (val.f - val_min.f) / (val_max.f - val_min.f);
    }
};

// Read-only table views keep the large buffers/pointer arrays shared across voices.
struct WavetableView
{
    int size{}, size_po2{}, flags{};
    unsigned n_tables{};
    float dt{};
    float *const (*TableF32WeakPointers)[max_subtables]{};
    short *const (*TableI16WeakPointers)[max_subtables]{};
    void bind(const Wavetable &w)
    {
        size = w.size;
        size_po2 = w.size_po2;
        flags = w.flags;
        n_tables = w.n_tables;
        dt = w.dt;
        TableF32WeakPointers = w.TableF32WeakPointers;
        TableI16WeakPointers = w.TableI16WeakPointers;
    }
};
struct OscillatorStorage : ParamUserData
{
    WavetableView wt;
    std::array<Parameter, 7> p{};
    Parameter retrigger, type;
    struct
    {
        int nData{};
        std::array<float, 16> data{};
    } extraConfig;
};
struct FxStorage
{
    Parameter type, return_level, p[n_fx_params];
    FxStorage()
    {
        for (int i = 0; i < n_fx_params; ++i)
            p[i].id = i;
    }
};

struct SurgeStorage
{
    float samplerate{48000}, samplerate_inv{1.f / 48000};
    double dsamplerate{48000}, dsamplerate_inv{1.0 / 48000};
    double dsamplerate_os{96000}, dsamplerate_os_inv{1.0 / 96000};
    float temposyncratio{1}, temposyncratio_inv{1};

    struct Patch
    {
        Parameter character;
        bool correctlyTuneCombFilter{true};
        // Effect init_ctrltypes hooks index this; they are never evaluated here.
        FxStorage *fx{};
        // Alias "Memory From..." sources; never selected on MetaModule (engine maps them out).
        inline static const uint8_t scenedata[256]{}, stepsequences[256]{};
        struct DawExtra
        {
            uint8_t bytes[256];
        };
        inline static const DawExtra dawExtraState{};
        struct Scene
        {
            OscillatorStorage osc[3];
        };
        inline static Scene scene[2]{};
    } patch;
    Patch *_patch{&patch};
    Patch &getPatch() { return patch; }

    // Tuning: MetaModule is always 12-TET, like sst-filters' default TuningProvider.
    enum TuningApplicationMode
    {
        RETUNE_ALL = 0,
        RETUNE_MIDI_ONLY = 1
    } tuningApplicationMode{RETUNE_ALL};
    void *oddsound_mts_client{};
    bool oddsound_mts_active_as_client{}, isStandardTuning{true};
    struct
    {
        static double logScaledFrequencyForMidiNote(int n) { return double(n) / 12.0; }
    } currentTuning;

    uint32_t random_state{0x13579bdf};
    WavetableView WindowWT;
    Surge::Memory::SurgeMemoryPools *memoryPools{};
    int16_t *sinctableI16{};
    float *sinctable{}, *sinctable1X{};
    float (*audio_in)[64]{};
    float audio_in_nonOS alignas(16)[2][BLOCK_SIZE]{};

    SurgeStorage()
    {
        static sst::basic_blocks::tables::SurgeSincTableProvider sinc;
        sinctable = sinc.sinctable;
        sinctable1X = sinc.sinctable1X;
        sinctableI16 = sinc.sinctableI16;
        patch.character.val.i = 1;
    }
    SurgeStorage(const SurgeStorage &) = delete;
    SurgeStorage &operator=(const SurgeStorage &) = delete;
    void setSamplerate(float sr)
    {
        samplerate = sr;
        samplerate_inv = 1.f / sr;
        dsamplerate = sr;
        dsamplerate_inv = 1.0 / sr;
        dsamplerate_os = 2.0 * sr;
        dsamplerate_os_inv = .5 / sr;
        tuning.set_samplerate(sr);
        envelope.set_samplerate(sr);
    }

    // Random sources. rand, rand_u32 and rand_01 (oscillators) step one xorshift state
    // per storage. rand_pm1 (Combulator noise) follows Surge's RNGGen: minstd_rand with a
    // uniform -1..1 distribution, seeded per storage, or a source shared between storages.
    std::minstd_rand pm1_generator{1};
    std::uniform_real_distribution<float> pm1_distribution{-1.f, 1.f};
    float (*shared_pm1)(void *){};
    void *shared_pm1_context{};
    void seed_pm1(unsigned seed)
    {
        pm1_generator.seed(seed);
        pm1_distribution.reset();
    }
    uint32_t rand() { return rand_u32(); }
    uint32_t rand_u32()
    {
        rand_01();
        return random_state;
    }
    float rand_01()
    {
        random_state ^= random_state << 13;
        random_state ^= random_state >> 17;
        random_state ^= random_state << 5;
        return (random_state >> 8) * (1.f / 16777216.f);
    }
    float rand_pm1()
    {
        return shared_pm1 ? shared_pm1(shared_pm1_context) : pm1_distribution(pm1_generator);
    }

    // 12-TET pitch and omega tables
    static constexpr double MIDI_0_FREQ = surge_mm::FilterTuning::MIDI_0_FREQ;
    static constexpr int tuningConstantNote = 60;
    static constexpr float tuningPitch = 32.f, tuningPitchInv = 1.f / 32.f;
    float note_to_pitch(float x) const { return surge_mm::FilterTuning::note_to_pitch(x); }
    float note_to_pitch_inv(float x) const { return note_to_pitch_inv_ignoring_tuning(x); }
    float note_to_pitch_ignoring_tuning(float x) const
    {
        return surge_mm::FilterTuning::note_to_pitch_ignoring_tuning(x);
    }
    float note_to_pitch_inv_ignoring_tuning(float x) const
    {
        return surge_mm::FilterTuning::note_to_pitch_inv_ignoring_tuning(x);
    }
    float note_to_pitch_tuningctr(float x) const
    {
        return note_to_pitch(x + tuningConstantNote) * tuningPitchInv;
    }
    float note_to_pitch_inv_tuningctr(float x) const
    {
        return note_to_pitch_inv(x + tuningConstantNote) * tuningPitch;
    }
    void note_to_omega_ignoring_tuning(float x, float &sinu, float &cosi, float sr) const
    {
        tuning.note_to_omega_ignoring_tuning(x, sinu, cosi, sr);
    }

    float db_to_linear(float x) const { return surge_mm::db_to_linear(x); }
    float envelope_rate_linear(float x) const { return envelope.envelope_rate_linear(x); }
    float envelope_rate_linear_nowrap(float x) const
    {
        return envelope.envelope_rate_linear_nowrap(x);
    }
    // Defined in SurgeStorage.cpp so the waveshaper tables are instantiated once.
    float lookup_waveshape(sst::waveshapers::WaveshaperType entry, float x) const;
    float lookup_waveshape_warp(sst::waveshapers::WaveshaperType entry, float x) const;

  private:
    surge_mm::FilterTuning tuning;
    surge_mm::EnvelopeClock envelope;
};
