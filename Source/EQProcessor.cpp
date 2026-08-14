#include "EQProcessor.h"
#include "ParamIDs.h"

void EQProcessor::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    auto monoSpec = spec;
    monoSpec.numChannels = 1;
    filterChainL.prepare(monoSpec);
    filterChainR.prepare(monoSpec);
    isPrepared = true;
    updateCoefficients();
}

void EQProcessor::prepareWithEQParams (const juce::dsp::ProcessSpec& spec,
                                        juce::AudioProcessorValueTreeState& apvts)
{
    sampleRate = spec.sampleRate;

    const juce::String freqIDs[] = { ParamID::eq0Freq, ParamID::eq1Freq, ParamID::eq2Freq, ParamID::eq3Freq };
    const juce::String gainIDs[] = { ParamID::eq0Gain, ParamID::eq1Gain, ParamID::eq2Gain, ParamID::eq3Gain };
    const juce::String qIDs[]    = { ParamID::eq0Q,    ParamID::eq1Q,    ParamID::eq2Q,    ParamID::eq3Q    };

    for (size_t i = 0; i < 4; ++i) {
        bands[i].freq = *apvts.getRawParameterValue(freqIDs[i]);
        bands[i].gain = *apvts.getRawParameterValue(gainIDs[i]);
        bands[i].q    = *apvts.getRawParameterValue(qIDs[i]);
    }

    auto monoSpec = spec;
    monoSpec.numChannels = 1;
    filterChainL.prepare(monoSpec);
    filterChainR.prepare(monoSpec);
    isPrepared = true;
    updateCoefficients();
}

void EQProcessor::updateCoefficients()
{
    if (!isPrepared) return;
    const double sr = sampleRate;

    auto c0 = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sr, bands[0].freq, bands[0].q, juce::Decibels::decibelsToGain(bands[0].gain));
    *filterChainL.get<0>().coefficients = *c0;
    *filterChainR.get<0>().coefficients = *c0;

    auto c1 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, bands[1].freq, bands[1].q, juce::Decibels::decibelsToGain(bands[1].gain));
    *filterChainL.get<1>().coefficients = *c1;
    *filterChainR.get<1>().coefficients = *c1;

    auto c2 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, bands[2].freq, bands[2].q, juce::Decibels::decibelsToGain(bands[2].gain));
    *filterChainL.get<2>().coefficients = *c2;
    *filterChainR.get<2>().coefficients = *c2;

    auto c3 = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sr, bands[3].freq, bands[3].q, juce::Decibels::decibelsToGain(bands[3].gain));
    *filterChainL.get<3>().coefficients = *c3;
    *filterChainR.get<3>().coefficients = *c3;
}

void EQProcessor::processBuffer (juce::AudioBuffer<float>& buffer)
{
    if (!isPrepared) return;

    const int numSamples = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();

    if (numCh > 0) {
        float* ch0 = buffer.getWritePointer(0);
        juce::dsp::AudioBlock<float> block(&ch0, 1, 0, (size_t)numSamples);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        filterChainL.process(ctx);
    }

    if (numCh > 1) {
        float* ptrs[1] = { buffer.getWritePointer(1) };
        juce::dsp::AudioBlock<float> block(ptrs, 1, 0, (size_t)numSamples);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        filterChainR.process(ctx);
    }
}
