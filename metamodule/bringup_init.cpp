// Temporary plugin entry point while the modules are brought up one at a time.
// Once every model builds for MetaModule, this is replaced by src/SurgeXT.cpp.

#include "SurgeXT.h"
#include "XTStyle.h"

rack::Plugin *pluginInstance;

void init(rack::Plugin *p)
{
    pluginInstance = p;

    p->addModel(modelVCOClassic);
    p->addModel(modelVCOModern);
    p->addModel(modelVCOWavetable);
    p->addModel(modelVCOWindow);
    p->addModel(modelVCOSine);
    p->addModel(modelVCOFM2);
    p->addModel(modelVCOFM3);
    p->addModel(modelVCOSHNoise);
    p->addModel(modelVCOAlias);
    p->addModel(modelVCOString);
    p->addModel(modelVCOTwist);

    p->addModel(modelFXReverb);
    p->addModel(modelFXPhaser);
    p->addModel(modelFXRotarySpeaker);
    p->addModel(modelFXDistortion);
    p->addModel(modelFXFrequencyShifter);
    p->addModel(modelFXChorus);
    p->addModel(modelFXVocoder);
    p->addModel(modelFXReverb2);
    p->addModel(modelFXFlanger);
    p->addModel(modelFXRingMod);
    p->addModel(modelFXNeuron);
    p->addModel(modelFXResonator);
    p->addModel(modelFXChow);
    p->addModel(modelFXExciter);
    p->addModel(modelFXEnsemble);
    p->addModel(modelFXCombulator);
    p->addModel(modelFXSpringReverb);
    p->addModel(modelFXTreeMonster);
    p->addModel(modelFXBonsai);
    p->addModel(modelFXNimbus);

    sst::surgext_rack::style::XTStyle::initialize();
}
