// SPDX-License-Identifier: GPL-3.0-or-later
// Stand-ins for Surge's MSEG and Formula modulation helpers, which need the full MSEG model
// and (for Formula) Lua. LFOModulationSource.h includes them from its own directory, where
// the include path cannot shadow them, so their include guards are defined here first; this
// header is included by the SurgeStorage stand-in, which the LFO includes before them.
// MetaModule's LFO offers neither shape: evaluating one aborts.
#pragma once
#define SURGE_SRC_COMMON_DSP_MODULATORS_MSEGMODULATIONHELPER_H
#define SURGE_SRC_COMMON_DSP_MODULATORS_FORMULAMODULATIONHELPER_H
#include <cstdlib>
#include <optional>
#include <string>

struct SurgeStorage;
struct MSEGStorage;
struct FormulaModulatorStorage;

namespace Surge::MSEG
{
struct EvaluatorState
{
    bool released{}, retrigger_FEG{}, retrigger_AEG{};
    void seed(long) {}
};
[[noreturn]] inline float valueAt(int, float, float, MSEGStorage *, EvaluatorState *,
                                  bool = false)
{
    std::abort();
}
} // namespace Surge::MSEG

namespace Surge::Formula
{
static constexpr int max_formula_outputs = 8;
struct EvaluatorState
{
    bool released{}, isVoice{}, useEnvelope{true}, retrigger_AEG{}, retrigger_FEG{},
        raisedError{};
    int activeoutputs{};
    float del{}, a{}, h{}, dec{}, s{}, r{}, rate{}, amp{}, phase{}, deform{}, tempo{}, songpos{};
    std::optional<std::string> error;
};
inline void initEvaluatorState(EvaluatorState &s) { s = {}; }
inline void cleanEvaluatorState(EvaluatorState &s) { s = {}; }
[[noreturn]] inline void prepareForEvaluation(SurgeStorage *, FormulaModulatorStorage *,
                                              EvaluatorState &, bool)
{
    std::abort();
}
[[noreturn]] inline void valueAt(int, float, SurgeStorage *, FormulaModulatorStorage *,
                                 EvaluatorState *, float *)
{
    std::abort();
}
} // namespace Surge::Formula
