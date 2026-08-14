#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "ConvolutionEngine.h"
#include "EQProcessor.h"
#include "ParamIDs.h"

struct IRPreset {
    juce::String name;
    juce::String category;
    const char*  binaryName;
    int          binarySize;
    int          numSamples  = 0;
    double       sampleRate  = 44100.0;
};

struct UserPreset {
    juce::String name;
    int    irIndex        = 0;
    juce::String customIRPath;
    float  predelay       = 0.f;
    float  size           = 1.f;
    float  dryWet         = 1.f;
    float  gainOut        = 0.f;
    bool   eqOn           = true;
    float  eq0Freq = 100.f, eq0Gain = 0.f;
    float  eq1Freq = 500.f, eq1Gain = 0.f;
    float  eq2Freq = 2000.f,eq2Gain = 0.f;
    float  eq3Freq = 8000.f,eq3Gain = 0.f;
};

class SordIRAudioProcessor : public juce::AudioProcessor,
                              public juce::AudioProcessorValueTreeState::Listener
{
public:
    SordIRAudioProcessor();
    ~SordIRAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Sord IR"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override    { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;
    void parameterChanged (const juce::String& paramID, float newValue) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    const juce::Array<IRPreset>& getIRPresets() const { return irPresets; }
    void loadIRByIndex (int index);
    void loadCustomIR  (const juce::File& file);
    int  getCurrentIRIndex() const { return currentIRIndex; }
    float getOutputLevel() const { return outputLevelAtomic.load(); }

    // Returns duration in seconds of the current IR (after size scaling)
    float getCurrentIRDuration() const;

    void saveUserPreset   (const juce::String& name);
    void loadUserPreset   (int index);
    void deleteUserPreset (int index);
    void loadUserPresetsFromDisk();
    const juce::Array<UserPreset>& getUserPresets() const { return userPresets; }
    juce::File getUserPresetsDir() const;

private:
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    ConvolutionEngine convEngine;
    EQProcessor       irEQ;

    juce::Array<IRPreset>   irPresets;
    juce::Array<UserPreset> userPresets;

    void buildPresetList();
    void saveUserPresetToDisk (const UserPreset& p);
    void deleteUserPresetFromDisk (const juce::String& name);
    UserPreset captureCurrentState (const juce::String& name);
    void applyUserPreset (const UserPreset& p);

    int currentIRIndex = 0;

    // Pre-delay: circular buffer
    juce::AudioBuffer<float> predelayBuffer;
    int predelayWritePos    = 0;

    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;

    void updateEQOnIR();
    std::atomic<float> outputLevelAtomic { 0.f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SordIRAudioProcessor)
};
