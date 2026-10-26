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

#ifndef SURGE_XT_RACK_SRC_XTMODULEWIDGET_H
#define SURGE_XT_RACK_SRC_XTMODULEWIDGET_H

#include "rack.hpp"
#include "XTStyle.h"
#include <array>
#include "XTWidgets.h"

namespace sst::surgext_rack::widgets
{
struct XTModuleWidget : public virtual rack::ModuleWidget, style::StyleParticipant
{
    virtual void onStyleChanged() override { dirtyFB(this); }

    void dirtyFB(rack::Widget *w)
    {
        auto f = dynamic_cast<rack::FramebufferWidget *>(w);
        if (f)
            f->dirty = true;
        for (auto c : w->children)
            dirtyFB(c);
    }

    void onHoverKey(const HoverKeyEvent &e) override
    {
        if (e.action == GLFW_PRESS && (e.mods & GLFW_MOD_ALT))
        {
            if (e.key >= '1' && e.key <= '4')
            {
                auto whichMod = e.key - '1';
                selectModulator(whichMod);
                e.consume(this);
                return;
            }
        }
        ModuleWidget::onHoverKey(e);
    }

    int snapNamesEvery{0};
    double lastSnapTime{-100};
    void step() override
    {
        // snap every 2 seconds, check every 5 frames
        if (snapNamesEvery == 0)
        {
            snapNamesEvery = 5;
            if (module)
            {
                auto tnow = rack::system::getTime();
                if (tnow - lastSnapTime > 1.0)
                {
                    auto xtm = dynamic_cast<modules::XTModule *>(module);
                    if (xtm)
                        xtm->snapCalculatedNames();
                    lastSnapTime = tnow;
                }
            }
        }
        snapNamesEvery--;
        ModuleWidget::step();
    }

#ifndef METAMODULE
    // For making the MetaModule faceplates (see metamodule/scripts/faceplates.py): with
    // SURGEXT_PANEL_ONLY set in the environment, draw only the panel art and the labels (the
    // ones that change show their default text). The params, ports, lights, and LCDs are
    // MetaModule elements.
    static bool panelOnly()
    {
        static bool res = std::getenv("SURGEXT_PANEL_ONLY") != nullptr;
        return res;
    }

    static bool isPanelArt(rack::widget::Widget *w)
    {
        return dynamic_cast<widgets::Background *>(w) || dynamic_cast<widgets::Label *>(w) ||
               dynamic_cast<widgets::GroupLabel *>(w) ||
               dynamic_cast<widgets::OutputDecoration *>(w);
    }

    void draw(const DrawArgs &args) override
    {
        if (!panelOnly())
        {
            ModuleWidget::draw(args);
            return;
        }
        for (auto *c : children)
            if (c->isVisible() && isPanelArt(c))
                drawChild(c, args);
    }

    void drawLayer(const DrawArgs &args, int layer) override
    {
        if (!panelOnly())
            ModuleWidget::drawLayer(args, layer);
    }
#endif

#ifdef METAMODULE
    // MetaModule makes a graphic display of each child widget that isn't a param, port, or
    // light. The panel art (background, labels) is in the faceplate image there, and the LCDs
    // are drawn by addLCDDisplays(), so add those widgets as plain children.
    using rack::ModuleWidget::addChild;
    void addChild(rack::widget::Widget *w)
    {
        if (auto *sw = dynamic_cast<widgets::ActivateKnobSwitch *>(w))
            sw->addFrames();

        if (dynamic_cast<rack::app::ParamWidget *>(w) || dynamic_cast<rack::app::PortWidget *>(w) ||
            dynamic_cast<rack::app::LightWidget *>(w))
            rack::ModuleWidget::addChild(w);
        else
            rack::widget::Widget::addChild(w);
    }

    // Adds a graphic display for each LCDBackground, which draws it and the widgets centered on
    // it (preset selector, plots, LCD params). Call it after adding all the children.
    void addLCDDisplays()
    {
        // In Rack, the LCD background of the VCO panel (used by the VCOs, VCF and Waveshaper) is
        // in the panel SVG. This is its rect there, in px.
        bool hasVCOPanel = false;
        for (auto *c : children)
            if (auto *bg = dynamic_cast<widgets::Background *>(c))
                hasVCOPanel = hasVCOPanel || bg->panelName == "BlankVCO";
        if (hasVCOPanel)
        {
            auto svgToPx = rack::app::RACK_GRID_WIDTH * layout::LayoutConstants::numberOfScrews / 172.8f;
            auto x = 12.08506f * svgToPx;
            addChild(widgets::LCDBackground::createAtYPosition(
                25.408199f * svgToPx, 106.22314f * svgToPx, layout::LayoutConstants::numberOfScrews,
                widgets::LCDBackground::posx - x));
        }

        std::vector<widgets::LCDDisplay *> lcds;
        for (auto *c : children)
            if (auto *bg = dynamic_cast<widgets::LCDBackground *>(c))
                lcds.push_back(new widgets::LCDDisplay(bg));

        for (auto *c : children)
        {
            for (auto *lcd : lcds)
            {
                if (c != lcd->layers.front() && lcd->box.contains(c->box.getCenter()))
                {
                    lcd->layers.push_back(c);
                    break;
                }
            }
        }

        for (auto *lcd : lcds)
            rack::ModuleWidget::addChild(lcd);
    }
#endif

    virtual void selectModulator(int whichMod) {}

    virtual void appendModuleSpecificMenu(rack::ui::Menu *menu) {}

    template <typename T> void addClockMenu(rack::ui::Menu *menu)
    {
        typedef typename T::clockProcessor_t cp_t;
        auto xtm = static_cast<T *>(module);

        menu->addChild(new rack::ui::MenuSeparator);
        auto t = xtm->clockProc.clockStyle;
        menu->addChild(
            rack::createMenuItem("Clock in QuarterNotes", CHECKMARK(t == cp_t::QUARTER_NOTE),
                                 [xtm]() { xtm->clockProc.clockStyle = cp_t::QUARTER_NOTE; }));

        menu->addChild(
            rack::createMenuItem("Clock in BPM CV", CHECKMARK(t == cp_t::BPM_VOCT),
                                 [xtm]() { xtm->clockProc.clockStyle = cp_t::BPM_VOCT; }));
    }

    // Adds each selection with checkmark and a lambda to set the PA
    void addSelectionMenu(rack::ui::Menu *menu, rack::ParamQuantity *pq,
                          std::vector<std::pair<std::string, int>> values)
    {
        auto pqv = (int)std::round(pq->getValue());
        for (const auto &[l, v] : values)
        {
            menu->addChild(rack::createMenuItem(l, CHECKMARK(v == pqv),
                                                [pq, val = v]() { pq->setValue(val); }));
        }
    }

    void addSelectionMenu(rack::ui::Menu *menu, rack::ParamQuantity *pq)
    {
        auto sq = dynamic_cast<rack::SwitchQuantity *>(pq);
        if (!sq)
        {
            return;
        }
        auto pqv = (int)std::round(pq->getValue());
        for (int i = pq->getMinValue(); i <= pq->getMaxValue(); ++i)
        {
            menu->addChild(rack::createMenuItem(sq->labels[i], CHECKMARK(i == pqv),
                                                [pq, val = i]() { pq->setValue(val); }));
        }
    }
    virtual void appendContextMenu(rack::ui::Menu *menu) override;

  protected:
    void resetStyleCouplingToModule();
    void toggleCoupleToGlobalStyle();
};

template <typename M, bool useAnimValues = true> struct DirtyHelper
{
    M *module{nullptr};
    float lastValue{0};
    int par{0};
    int isModulated{false};

    inline bool dirty()
    {
        if (!module)
            return false;

        auto r = module->paramQuantities[par]->getValue();
        if constexpr (useAnimValues)
        {
            if (isModulated)
                r = module->modAssist.values[par][0];
        }
        auto v = r != lastValue;
        lastValue = r;
        return v;
    }
};

// The Model for the module widget W and its module W::M
template <typename W> rack::Model *createXTModel(const std::string &slug)
{
#ifdef METAMODULE
    return rack::createModel<typename W::M, WithFaceplate<W>>(slug);
#else
    return rack::createModel<typename W::M, W>(slug);
#endif
}
} // namespace sst::surgext_rack::widgets

#endif