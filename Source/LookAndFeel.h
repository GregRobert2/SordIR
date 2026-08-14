#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class SordLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SordLookAndFeel();

    // Colours
    static const juce::Colour bg;
    static const juce::Colour bg2;
    static const juce::Colour bg3;
    static const juce::Colour border;
    static const juce::Colour gold;
    static const juce::Colour goldDim;
    static const juce::Colour textMain;
    static const juce::Colour textDim;
    static const juce::Colour textFaint;
    static const juce::Colour green;
    static const juce::Colour red;

    // Overrides
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    void drawComboBox (juce::Graphics&, int w, int h, bool isDown,
                       int, int, int, int, juce::ComboBox&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool highlighted, bool down) override;

    juce::Font getLabelFont (juce::Label&) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override;

    juce::Font getTextButtonFont (juce::TextButton&, int) override;
};
