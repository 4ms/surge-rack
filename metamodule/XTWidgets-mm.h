/*
 * SurgeXT for VCV Rack - a Surge Synth Team product
 *
 * Helpers/classes for MetaModule build
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


#pragma once

// Widgets used only on the MetaModule. Included at the end of XTWidgets.h.

#include "XTWidgets.h"

namespace sst::surgext_rack::widgets
{
// A graphic display around a knob or slider, which draws the ring (or bars) of its mod depth
// knob (a ModRingKnob, or a VerticalSliderModulator) for the Mod button that's on (the mod depth
// knobs are invisible)
template <typename ModKnob> struct ModArcDisplay : rack::widget::Widget
{
    modules::XTModule *module{nullptr};
    std::array<ModKnob *, modules::XTModule::numModButtons> rings{};

    // The mod depths at the last step, to see which one changed
    std::array<float, modules::XTModule::numModButtons> lastDepths{};
    bool haveLastDepths{false};

    ModArcDisplay(modules::XTModule *module, const rack::Vec &center, const rack::Vec &size)
        : module(module)
    {
        box.size = size;
        box.pos = center.minus(size.div(2));
    }

    void step() override
    {
        if (module)
        {
            showChangedModDepth();
            module->stepModButtons();
        }
        rack::widget::Widget::step();
    }

    // Changing a mod depth (by its knob, or a mapping) turns its Mod button on, so it's shown
    // without having to turn on the Mod button first
    void showChangedModDepth()
    {
        for (int m = 0; m < modules::XTModule::numModButtons; ++m)
        {
            auto *pq = rings[m] ? rings[m]->getParamQuantity() : nullptr;
            if (!pq)
                continue;

            auto v = pq->getValue();
            if (haveLastDepths && v != lastDepths[m])
                module->params[module->modButtonParam0 + m].setValue(1.f);
            lastDepths[m] = v;
        }
        haveLastDepths = true;
    }

    void draw(const DrawArgs &args) override
    {
        if (!module)
            return;
        auto m = module->activeModButton();
        if (m >= 0 && rings[m])
            rings[m]->drawWidget(args.vg);
    }
};

// The Mod 1-4 buttons, which are params on MetaModule (see XTModule::config)
struct ModToggleParamButton : rack::app::SvgSwitch, style::StyleParticipant
{
    ModToggleParamButton()
    {
        auto compDir = style()->skinAssetDir() + "/components";
        addFrame(rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/mod-button.svg")));
        addFrame(
            rack::Svg::load(rack::asset::plugin(pluginInstance, compDir + "/mod-button-on.svg")));
    }

    void onStyleChanged() override {}
};

// The MetaModule draws an LCD as one graphic display: the LCDBackground and the widgets on it.
// Those stay children of the module widget (params must, for their positions), so this draws
// them without owning them.
struct LCDDisplay : public rack::widget::Widget
{
    std::vector<rack::widget::Widget *> layers;

    explicit LCDDisplay(LCDBackground *bg)
    {
        box = bg->box;
        layers.push_back(bg);
    }

    // The firmware's Widget::step() doesn't step the children, so the module widget doesn't
    // step these (and the plots compute their curves in step())
    void step() override
    {
        for (auto *w : layers)
            w->step();
    }

    void draw(const DrawArgs &args) override { drawLayers(args, 0); }
    void drawLayer(const DrawArgs &args, int layer) override { drawLayers(args, layer); }

    void drawLayers(const DrawArgs &args, int layer)
    {
        for (auto *w : layers)
        {
            if (!w->isVisible())
                continue;

            auto childArgs = args;
            childArgs.clipBox = w->box.zeroPos();
            nvgSave(args.vg);
            nvgTranslate(args.vg, w->box.pos.x - box.pos.x, w->box.pos.y - box.pos.y);

            // The firmware's ParamWidget::draw() doesn't draw its children, which is where
            // the LCD params draw their text
            bool isParam = dynamic_cast<rack::app::ParamWidget *>(w) != nullptr;
            if (layer == 0)
                isParam ? w->rack::widget::Widget::draw(childArgs) : w->draw(childArgs);
            else
                isParam ? w->rack::widget::Widget::drawLayer(childArgs, layer)
                        : w->drawLayer(childArgs, layer);

            nvgRestore(args.vg);
        }
    }
};

} // namespace sst::surgext_rack::widgets
