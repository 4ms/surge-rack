// SPDX-License-Identifier: GPL-3.0-or-later
// Helpers for hosting Surge's Effect subclasses (built against compat/SurgeStorage.h).
#pragma once
#include "SurgeStorage.h"
#include <new>
namespace surge_mm
{
// Replace an effect with a fresh, initialized instance in place, as Surge does when an
// effect is loaded, without allocating on the audio thread. Only for effects whose
// constructor does not allocate.
template <typename E> void reset_effect(E &effect, SurgeStorage *storage, FxStorage *fxdata,
                                        pdata *values)
{
    effect.~E();
    new (&effect) E(storage, fxdata, values);
    effect.init();
}
} // namespace surge_mm
