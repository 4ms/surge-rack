/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * A set of modules expressing Surge XT into the VCV Rack Module Ecosystem
 *
 * Copyright 2019 - 2024, Various authors, as described in the github
 * transaction log.
 *
 * Surge XT for VCV Rack is released under the GNU General Public License
 * 3.0 or later (GPL-3.0-or-later). A copy of the license is in this
 * repository in the file "LICENSE" or at:
 *
 * or at https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Surge XT for VCV Rack is available at
 * https://github.com/surge-synthesizer/surge-rack/
 */

#include "SurgeXT.h"
#include "XTStyle.h"
#ifndef METAMODULE
#include "XTModuleWidget.h"
#endif

namespace logger = rack::logger;
rack::Plugin *pluginInstance;

__attribute__((__visibility__("default"))) void init(rack::Plugin *p)
{
    pluginInstance = p;
    INFO("[SurgeXTRack] initializing");

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

    // Add the ported ones
    p->addModel(modelSurgeVCF);
    p->addModel(modelSurgeDelay);
    p->addModel(modelSurgeDelayLineByFreq);
    p->addModel(modelSurgeDelayLineByFreqExpanded);
    p->addModel(modelSurgeWaveshaper);
    p->addModel(modelSurgeLFO);
    p->addModel(modelSurgeMixer);
    p->addModel(modelSurgeMixerSlider);
    p->addModel(modelSurgeModMatrix);
    p->addModel(modelSurgeDigitalRingMods);

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

    p->addModel(modelEGxVCA);
    p->addModel(modelQuadAD);
    p->addModel(modelQuadLFO);

    p->addModel(modelUnisonHelper);
    p->addModel(modelUnisonHelperCVExpander);

#ifndef METAMODULE
    // Rack's screenshots make the module widgets without a module, so the dynamic labels have
    // no text. For the MetaModule faceplates, give them a new module so they show their default.
    if (sst::surgext_rack::widgets::XTModuleWidget::panelOnly())
    {
        struct PanelOnlyModel : rack::plugin::Model
        {
            rack::plugin::Model *model;
            PanelOnlyModel(rack::plugin::Model *m) : rack::plugin::Model(*m), model(m) {}
            rack::engine::Module *createModule() override { return model->createModule(); }
            rack::app::ModuleWidget *createModuleWidget(rack::engine::Module *m) override
            {
                if (!m)
                {
                    // The widget removes its module from the engine when it's deleted
                    m = model->createModule();
                    APP->engine->addModule(m);
                }
                return model->createModuleWidget(m);
            }
        };
        for (auto &m : p->models)
            m = new PanelOnlyModel(m);
    }
#endif

    sst::surgext_rack::style::XTStyle::initialize();
}
