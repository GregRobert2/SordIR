#include "ConvolutionEngine.h"

void ConvolutionEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    convolution.prepare(spec);
    isPrepared = true;
}

void ConvolutionEngine::loadImpulseResponse (juce::AudioBuffer<float>&& irBuffer,
                                              double irSampleRate)
{
    convolution.loadImpulseResponse(std::move(irBuffer),
                                    irSampleRate,
                                    juce::dsp::Convolution::Stereo::yes,
                                    juce::dsp::Convolution::Trim::yes,
                                    juce::dsp::Convolution::Normalise::no);
}

void ConvolutionEngine::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    if (!isPrepared) return;
    convolution.process(context);
}

void ConvolutionEngine::reset()
{
    convolution.reset();
}
