#include "LookAndFeel.h"

const juce::Colour SordLookAndFeel::bg        { 0xff0d0d0b };
const juce::Colour SordLookAndFeel::bg2       { 0xff131310 };
const juce::Colour SordLookAndFeel::bg3       { 0xff1a1a16 };
const juce::Colour SordLookAndFeel::border    { 0xff2a2a22 };
const juce::Colour SordLookAndFeel::gold      { 0xffc8a96e };
const juce::Colour SordLookAndFeel::goldDim   { 0xff6a5a38 };
const juce::Colour SordLookAndFeel::textMain  { 0xffd8d0c0 };
const juce::Colour SordLookAndFeel::textDim   { 0xff7a7260 };
const juce::Colour SordLookAndFeel::textFaint { 0xff3a3830 };
const juce::Colour SordLookAndFeel::green     { 0xff6aac7a };
const juce::Colour SordLookAndFeel::red       { 0xffc05040 };

SordLookAndFeel::SordLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, bg);
    setColour(juce::Label::textColourId, textDim);
    setColour(juce::ComboBox::backgroundColourId, bg2);
    setColour(juce::ComboBox::textColourId, textMain);
    setColour(juce::ComboBox::outlineColourId, border);
    setColour(juce::PopupMenu::backgroundColourId, bg2);
    setColour(juce::PopupMenu::textColourId, textMain);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, goldDim.withAlpha(0.4f));
    setColour(juce::PopupMenu::highlightedTextColourId, gold);
    setColour(juce::Slider::rotarySliderFillColourId, gold);
    setColour(juce::Slider::rotarySliderOutlineColourId, bg3);
    setColour(juce::Slider::thumbColourId, textMain);
    setColour(juce::Slider::trackColourId, gold);
    setColour(juce::Slider::backgroundColourId, bg3);
    setColour(juce::TextButton::buttonColourId, bg2);
    setColour(juce::TextButton::textColourOffId, textDim);
    setColour(juce::TextButton::textColourOnId, gold);
}

// ── Rotary (knob) ────────────────────────────────────────────────────────────
void SordLookAndFeel::drawRotarySlider (juce::Graphics& g,
    int x, int y, int w, int h,
    float sliderPos, float startAngle, float endAngle,
    juce::Slider& slider)
{
    const float cx = x + w * 0.5f, cy = y + h * 0.5f;
    const float r  = juce::jmin(w, h) * 0.5f - 4.f;

    // Track arc
    juce::Path track;
    track.addCentredArc(cx, cy, r, r, 0.f, startAngle, endAngle, true);
    g.setColour(bg3);
    g.strokePath(track, juce::PathStrokeType(2.f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    // Value arc
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    juce::Path valueArc;
    valueArc.addCentredArc(cx, cy, r, r, 0.f, startAngle, angle, true);
    g.setColour(gold);
    g.strokePath(valueArc, juce::PathStrokeType(2.f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    // Knob body
    const float kr = r - 6.f;
    juce::ColourGradient bodyGrad(bg3.brighter(0.1f), cx - kr*0.3f, cy - kr*0.3f,
                                   bg.darker(0.2f),   cx + kr*0.3f, cy + kr*0.3f, true);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(cx - kr, cy - kr, kr*2, kr*2);
    g.setColour(border.brighter(0.2f));
    g.drawEllipse(cx - kr, cy - kr, kr*2, kr*2, 1.f);

    // Pointer dot
    const float dotR = 2.5f, dotDist = kr - 5.f;
    const float dotX = cx + dotDist * std::sin(angle);
    const float dotY = cy - dotDist * std::cos(angle);
    g.setColour(gold);
    g.fillEllipse(dotX - dotR, dotY - dotR, dotR*2, dotR*2);
}

// ── Linear slider (vertical fader) ──────────────────────────────────────────
void SordLookAndFeel::drawLinearSlider (juce::Graphics& g,
    int x, int y, int w, int h,
    float sliderPos, float /*minPos*/, float /*maxPos*/,
    juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearVertical) {
        const float trackW = 5.f;
        const float cx = x + w * 0.5f;
        const float tx = cx - trackW * 0.5f;
        const float trackTop    = (float)y + 6.f;
        const float trackBottom = (float)(y + h) - 6.f;
        const float trackH      = trackBottom - trackTop;

        // Shadow behind track
        g.setColour(juce::Colour(0xff050504));
        g.fillRoundedRectangle(tx - 1.f, trackTop - 1.f, trackW + 2.f, trackH + 2.f, 3.f);

        // Track BG — subtle groove
        juce::ColourGradient bgGrad(bg.darker(0.3f), cx, trackTop,
                                     bg3,             cx, trackBottom, false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(tx, trackTop, trackW, trackH, 2.5f);

        // Track fill: from thumb to bottom (below = active), gold-tinted
        const float fillTop    = juce::jlimit(trackTop, trackBottom, sliderPos);
        const float fillBottom = trackBottom;
        if (fillBottom > fillTop) {
            juce::ColourGradient fillGrad(goldDim.brighter(0.5f), cx, fillTop,
                                           goldDim.darker(0.2f),  cx, fillBottom, false);
            g.setGradientFill(fillGrad);
            g.fillRoundedRectangle(tx, fillTop, trackW, fillBottom - fillTop, 2.5f);
        }

        // Track border
        g.setColour(border.brighter(0.3f));
        g.drawRoundedRectangle(tx, trackTop, trackW, trackH, 2.5f, 0.7f);

        // Zero-dB marker (at 0dB position on range -24 to +12)
        // Approximate: 24/(24+12) = 0.667 from top
        const float zeroY = trackTop + trackH * (24.f / 36.f);
        g.setColour(goldDim.withAlpha(0.6f));
        g.drawLine(tx - 3.f, zeroY, tx + trackW + 3.f, zeroY, 0.8f);

        // === Thumb ===
        const float tw = (float)w - 4.f;  // nearly full width
        const float th = 16.f;
        const float tx2 = x + 2.f;
        const float ty2 = juce::jlimit(trackTop - th*0.5f,
                                        trackBottom - th*0.5f,
                                        sliderPos - th * 0.5f);

        // Thumb shadow
        g.setColour(juce::Colour(0x40000000));
        g.fillRoundedRectangle(tx2 + 1.f, ty2 + 2.f, tw, th, 3.f);

        // Thumb body — gradient top to bottom
        juce::ColourGradient thumbGrad(bg3.brighter(0.35f), tx2, ty2,
                                        bg.darker(0.15f),   tx2, ty2 + th, false);
        g.setGradientFill(thumbGrad);
        g.fillRoundedRectangle(tx2, ty2, tw, th, 3.f);

        // Thumb highlight top edge
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.fillRoundedRectangle(tx2 + 1.f, ty2 + 1.f, tw - 2.f, 3.f, 1.5f);

        // Thumb border — gold tint
        g.setColour(gold.withAlpha(slider.isMouseOverOrDragging() ? 0.85f : 0.45f));
        g.drawRoundedRectangle(tx2, ty2, tw, th, 3.f, 1.f);

        // 3 grip lines in center of thumb
        const float midY = ty2 + th * 0.5f;
        const float lineSpacing = 3.5f;
        g.setColour(gold.withAlpha(0.45f));
        for (int i = -1; i <= 1; ++i) {
            float ly = midY + i * lineSpacing;
            g.drawLine(tx2 + 5.f, ly, tx2 + tw - 5.f, ly, 0.8f);
        }
    }
}

// ── ComboBox ─────────────────────────────────────────────────────────────────
void SordLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h,
    bool /*isDown*/, int, int, int, int, juce::ComboBox& box)
{
    g.setColour(bg2);
    g.fillRoundedRectangle(0, 0, (float)w, (float)h, 2.f);
    g.setColour(border);
    g.drawRoundedRectangle(0.5f, 0.5f, w-1.f, h-1.f, 2.f, 1.f);

    // Arrow
    const float ax = w - 16.f, ay = h*0.5f;
    juce::Path arrow;
    arrow.addTriangle(ax, ay-3.f, ax+8.f, ay-3.f, ax+4.f, ay+3.f);
    g.setColour(textFaint.brighter(0.5f));
    g.fillPath(arrow);
}

// ── Toggle button ────────────────────────────────────────────────────────────
void SordLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& btn,
    bool highlighted, bool /*down*/)
{
    const bool on = btn.getToggleState();
    const float w = (float)btn.getWidth(), h = (float)btn.getHeight();

    // LED circle
    const float r = 4.f, cx = r + 4.f, cy = h*0.5f;
    g.setColour(on ? green : red);
    g.fillEllipse(cx - r, cy - r, r*2, r*2);
    if (on) {
        g.setColour(green.withAlpha(0.3f));
        g.fillEllipse(cx - r*2, cy - r*2, r*4, r*4);
    }

    // Text
    g.setColour(on ? textMain : textDim);
    g.setFont(juce::Font(juce::FontOptions().withName("Barlow Condensed").withHeight(10.f)));
    g.drawText(btn.getButtonText(), (int)(cx + r + 6), 0, (int)(w - cx - r - 8), (int)h,
               juce::Justification::centredLeft, false);
}

juce::Font SordLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font(juce::FontOptions().withName("Barlow Condensed").withHeight(10.f));
}
juce::Font SordLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font(juce::FontOptions().withName("Barlow Condensed").withHeight(11.f));
}
juce::Font SordLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return juce::Font(juce::FontOptions().withName("Barlow Condensed").withHeight(10.f));
}
