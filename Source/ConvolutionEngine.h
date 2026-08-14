#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class ConvolutionEngine
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void loadImpulseResponse (juce::AudioBuffer<float>&& irBuffer, double irSampleRate);
    void process (juce::dsp::ProcessContextReplacing<float>& context);
    void reset();

private:
    juce::dsp::Convolution convolution { juce::dsp::Convolution::NonUniform { 512 } };
    double sampleRate  = 44100.0;
    bool   isPrepared  = false;
};
