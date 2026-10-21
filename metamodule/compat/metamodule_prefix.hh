#pragma once

// Force-included in every C++ file of the Surge engine and the plugin
// (see cmake/surge-engine.cmake). Provides what the MetaModule's libc and
// libstdc++ lack and Surge expects from a desktop OS. SURGE_MM_NO_GTHREADS is
// defined for hardware builds (not the simulator, which uses the host's libc++).

#ifdef SURGE_MM_NO_GTHREADS
#include "std_mutex.hh"
#endif

#include <strings.h>

// Surge's globals.h only defines this for MAC or LINUX builds
static inline int _stricmp(const char *s1, const char *s2) { return strcasecmp(s1, s2); }
