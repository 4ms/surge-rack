// Temporary plugin entry point while the modules are brought up one at a time.
// Once every model builds for MetaModule, this is replaced by src/SurgeXT.cpp.

#include "rack.hpp"

#include "SurgeStorage.h"
#include "dsp/Effect.h"
#include "dsp/Oscillator.h"

rack::Plugin *pluginInstance;

void init(rack::Plugin *p) { pluginInstance = p; }

// Bring-up only: an exported function (so the linker keeps it) that reaches the engine
// code the modules will use, to find unresolved symbols and estimate the plugin size.
// It is never called.
extern "C" __attribute__((visibility("default"))) void surge_mm_link_probe()
{
    SurgeStorage::SurgeStorageConfig config;
    config.suppliedDataPath = SurgeStorage::skipPatchLoadDataPathSentinel;
    auto storage = new SurgeStorage(config);
    auto &patch = storage->getPatch();

    for (int t = 0; t < n_fx_types; t++)
        delete spawn_effect(t, storage, &patch.fx[0], patch.globaldata);

    static unsigned char oscbuffer alignas(16)[oscillator_buffer_size];
    for (int t = 0; t < n_osc_types; t++)
        spawn_osc(t, storage, &patch.scene[0].osc[0], patch.scenedata[0], oscbuffer);

    delete storage;
}
