#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class EQProcessor
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void prepareWithEQParams (const juce::dsp::ProcessSpec& spec,
                               juce::AudioProcessorValueTreeState& apvts);
    void processBuffer (juce::AudioBuffer<float>& buffer);

private:
    using FilterChain = juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>>;

    FilterChain filterChainL, filterChainR;
    double sampleRate = 44100.0;

    struct BandParams { float freq, gain, q; };
    std::array<BandParams, 4> bands {{
        {100.f,  0.f, 0.707f},
        {500.f,  0.f, 0.707f},
        {2000.f, 0.f, 0.707f},
        {8000.f, 0.f, 0.707f}
    }};
    bool isPrepared = false;
    void updateCoefficients();
};
