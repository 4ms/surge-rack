// Temporary plugin entry point while the modules are brought up one at a time.
// Once every model builds for MetaModule, this is replaced by src/SurgeXT.cpp.

#include "SurgeXT.h"
#include "XTStyle.h"

rack::Plugin *pluginInstance;

void init(rack::Plugin *p)
{
    pluginInstance = p;

    p->addModel(modelFXChorus);

    sst::surgext_rack::style::XTStyle::initialize();
}
