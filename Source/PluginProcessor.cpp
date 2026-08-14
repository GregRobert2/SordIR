#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

// ─── Preset list ──────────────────────────────────────────────────────────────
void SordIRAudioProcessor::buildPresetList()
{
    irPresets.clear();

    auto addIR = [&](const char* name, const char* cat,
                     const char* data, int dataSize)
    {
        IRPreset p;
        p.name       = name;
        p.category   = cat;
        p.binaryName = data;
        p.binarySize = dataSize;

        // duration filled lazily in loadIRByIndex
        irPresets.add(p);
    };

    addIR("Straight",      "Trumpet",  BinaryData::Trpt_Straight_wav,    BinaryData::Trpt_Straight_wavSize);
    addIR("Bucket",        "Trumpet",  BinaryData::Trpt_Bucket_wav,       BinaryData::Trpt_Bucket_wavSize);
    addIR("Cup",           "Trumpet",  BinaryData::Trpt_Cup_wav,          BinaryData::Trpt_Cup_wavSize);
    addIR("Harmon",        "Trumpet",  BinaryData::Trpt_Harmon_wav,       BinaryData::Trpt_Harmon_wavSize);

    addIR("Straight",      "Trombone", BinaryData::Trom_Straight_wav,     BinaryData::Trom_Straight_wavSize);
    addIR("Bucket",        "Trombone", BinaryData::Trom_Bucket_wav,       BinaryData::Trom_Bucket_wavSize);
    addIR("Cup",           "Trombone", BinaryData::Trom_Cup_wav,          BinaryData::Trom_Cup_wavSize);
    addIR("Harmon",        "Trombone", BinaryData::Trom_Harmon_wav,       BinaryData::Trom_Harmon_wavSize);
    addIR("Harmon Stem",   "Trombone", BinaryData::Trom_Harmon_Stem_wav,  BinaryData::Trom_Harmon_Stem_wavSize);

    addIR("Straight",      "Horn",     BinaryData::Horn_Straight_wav,     BinaryData::Horn_Straight_wavSize);
    addIR("Brass Stop",    "Horn",     BinaryData::Horn_Brass_Stop_wav,   BinaryData::Horn_Brass_Stop_wavSize);

    addIR("Straight",      "Tuba",     BinaryData::Tuba_Straight_wav,     BinaryData::Tuba_Straight_wavSize);

    addIR("Sordino Strings","Strings", BinaryData::Sordino_1_wav,         BinaryData::Sordino_1_wavSize);

    addIR("Studio Depth",  "Spaces",   BinaryData::IR_to_create_depth_wav, BinaryData::IR_to_create_depth_wavSize);
    addIR("Concert Hall",  "Spaces",   BinaryData::AIRSTUDIOSIRBRASS_wav,  BinaryData::AIRSTUDIOSIRBRASS_wavSize);

    addIR("Large Hall",     "Bricasti", BinaryData::Bricasti_Large_Hall_wav,   BinaryData::Bricasti_Large_Hall_wavSize);
    addIR("Brass Hall",     "Bricasti", BinaryData::Bricasti_Brass_Hall_wav,   BinaryData::Bricasti_Brass_Hall_wavSize);
    addIR("Amsterdam",      "Bricasti", BinaryData::Bricasti_Amsterdam_wav,    BinaryData::Bricasti_Amsterdam_wavSize);
    addIR("Boston",         "Bricasti", BinaryData::Bricasti_Boston_wav,       BinaryData::Bricasti_Boston_wavSize);
    addIR("Worcester",      "Bricasti", BinaryData::Bricasti_Worcester_wav,    BinaryData::Bricasti_Worcester_wavSize);
    addIR("Mechanics Hall", "Bricasti", BinaryData::Bricasti_Mechanics_wav,    BinaryData::Bricasti_Mechanics_wavSize);
    addIR("Saint Gerold",   "Bricasti", BinaryData::Bricasti_Saint_Gerold_wav, BinaryData::Bricasti_Saint_Gerold_wavSize);
}

float SordIRAudioProcessor::getCurrentIRDuration() const
{
    if (currentIRIndex < 0 || currentIRIndex >= irPresets.size()) return 0.f;
    const auto& p = irPresets[currentIRIndex];
    if (p.sampleRate <= 0.0 || p.numSamples <= 0) return 0.f;
    float size = *apvts.getRawParameterValue(ParamID::size);
    return (float)(p.numSamples / p.sampleRate) * size;
}

// ─── Parameter layout ─────────────────────────────────────────────────────────
juce::AudioProcessorValueTreeState::ParameterLayout SordIRAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterInt>
        (juce::ParameterID{ParamID::irIndex, 1}, "IR Index", 0, 21, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>
        (juce::ParameterID{ParamID::predelay, 1}, "Pre-Delay",
         juce::NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>
        (juce::ParameterID{ParamID::size, 1}, "Size",
         juce::NormalisableRange<float>(0.25f, 4.f, 0.01f), 1.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>
        (juce::ParameterID{ParamID::dryWet, 1}, "Dry/Wet",
         juce::NormalisableRange<float>(0.f, 1.f, 0.001f), 1.f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>
        (juce::ParameterID{ParamID::gainOut, 1}, "Output Gain",
         juce::NormalisableRange<float>(-24.f, 12.f, 0.1f), 0.f));

    params.push_back(std::make_unique<juce::AudioParameterBool>
        (juce::ParameterID{ParamID::eqOn, 1}, "EQ On", true));

    const struct { juce::String fId, gId, qId, fName, gName, qName; float fDefault; } bands[] = {
        { ParamID::eq0Freq, ParamID::eq0Gain, ParamID::eq0Q, "EQ Low Freq",    "EQ Low Gain",    "EQ Low Q",    100.f  },
        { ParamID::eq1Freq, ParamID::eq1Gain, ParamID::eq1Q, "EQ LowMid Freq", "EQ LowMid Gain", "EQ LowMid Q", 500.f  },
        { ParamID::eq2Freq, ParamID::eq2Gain, ParamID::eq2Q, "EQ HiMid Freq",  "EQ HiMid Gain",  "EQ HiMid Q",  2000.f },
        { ParamID::eq3Freq, ParamID::eq3Gain, ParamID::eq3Q, "EQ High Freq",   "EQ High Gain",   "EQ High Q",   8000.f },
    };

    for (auto& b : bands) {
        params.push_back(std::make_unique<juce::AudioParameterFloat>
            (juce::ParameterID{b.fId, 1}, b.fName,
             juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), b.fDefault));
        params.push_back(std::make_unique<juce::AudioParameterFloat>
            (juce::ParameterID{b.gId, 1}, b.gName,
             juce::NormalisableRange<float>(-12.f, 12.f, 0.1f), 0.f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>
            (juce::ParameterID{b.qId, 1}, b.qName,
             juce::NormalisableRange<float>(0.1f, 8.f, 0.01f), 0.707f));
    }

    return { params.begin(), params.end() };
}

// ─── Constructor ──────────────────────────────────────────────────────────────
SordIRAudioProcessor::SordIRAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    buildPresetList();

    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p))
            apvts.addParameterListener(rp->getParameterID(), this);
}

SordIRAudioProcessor::~SordIRAudioProcessor()
{
    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p))
            apvts.removeParameterListener(rp->getParameterID(), this);
}

// ─── Prepare ─────────────────────────────────────────────────────────────────
void SordIRAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = 2;

    convEngine.prepare(spec);
    irEQ.prepare(spec);

    // Pre-delay: max 100ms
    int maxDelay = (int)(sampleRate * 0.1) + samplesPerBlock + 1;
    predelayBuffer.setSize(2, maxDelay);
    predelayBuffer.clear();
    predelayWritePos = 0;

    loadIRByIndex(currentIRIndex);
}

void SordIRAudioProcessor::releaseResources() {}

// ─── Process ─────────────────────────────────────────────────────────────────
void SordIRAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    float dryWet  = *apvts.getRawParameterValue(ParamID::dryWet);
    float gainOut = juce::Decibels::decibelsToGain(
                        apvts.getRawParameterValue(ParamID::gainOut)->load());

    // Keep dry copy BEFORE predelay
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // ── Pre-delay ── (circular buffer, sample-accurate)
    int delaySamples = (int)(apvts.getRawParameterValue(ParamID::predelay)->load()
                             * 0.001f * (float)currentSampleRate + 0.5f);
    delaySamples = juce::jlimit(0, predelayBuffer.getNumSamples() - 1, delaySamples);

    if (delaySamples > 0) {
        int bufLen = predelayBuffer.getNumSamples();
        for (int ch = 0; ch < juce::jmin(numChannels, 2); ++ch) {
            const auto* src = buffer.getReadPointer(ch);
            auto*       dst = buffer.getWritePointer(ch);
            auto*       db  = predelayBuffer.getWritePointer(ch);
            int wp = predelayWritePos;
            for (int i = 0; i < numSamples; ++i) {
                db[wp] = src[i];
                int readPos = (wp - delaySamples + bufLen) % bufLen;
                dst[i] = db[readPos];
                wp = (wp + 1) % bufLen;
            }
        }
        predelayWritePos = (predelayWritePos + numSamples) % predelayBuffer.getNumSamples();
    }

    // ── Convolution ──
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    convEngine.process(ctx);

    // ── Dry/Wet mix (before output gain, using original dry signal) ──
    float wet = dryWet;
    float dry = 1.0f - dryWet;
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* w = buffer.getWritePointer(ch);
        auto* d = dryBuffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            w[i] = w[i] * wet + d[i] * dry;
    }

    // ── Output Gain (applied after mix) ──
    buffer.applyGain(gainOut);

    // ── Output level for VU meter ──
    float rms = 0.f;
    for (int ch = 0; ch < numChannels; ++ch)
        rms += buffer.getRMSLevel(ch, 0, numSamples);
    rms /= (float)numChannels;
    outputLevelAtomic.store(rms);
}

// ─── IR loading ───────────────────────────────────────────────────────────────
void SordIRAudioProcessor::loadIRByIndex (int index)
{
    if (!juce::isPositiveAndBelow(index, irPresets.size())) return;
    currentIRIndex = index;
    const auto& p = irPresets[index];

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(
            std::make_unique<juce::MemoryInputStream>(p.binaryName, (size_t)p.binarySize, false)));
    if (!reader) return;

    // Store duration info for UI display
    irPresets.getReference(index).numSamples = (int)reader->lengthInSamples;
    irPresets.getReference(index).sampleRate = reader->sampleRate;

    // Apply size stretch
    float size = *apvts.getRawParameterValue(ParamID::size);
    int origLen = (int)reader->lengthInSamples;
    int newLen  = juce::jmax(1, (int)(origLen * size));

    juce::AudioBuffer<float> orig((int)reader->numChannels, origLen);
    reader->read(&orig, 0, origLen, 0, true, true);

    // Resample for size
    juce::AudioBuffer<float> irBuffer((int)reader->numChannels, newLen);
    for (int ch = 0; ch < (int)reader->numChannels; ++ch) {
        for (int i = 0; i < newLen; ++i) {
            float srcIdx = (float)i / (float)size;
            int   s0 = juce::jlimit(0, origLen-1, (int)srcIdx);
            int   s1 = juce::jlimit(0, origLen-1, s0 + 1);
            float frac = srcIdx - s0;
            irBuffer.setSample(ch, i,
                orig.getSample(ch, s0) * (1.f - frac) +
                orig.getSample(ch, s1) * frac);
        }
    }

    // Apply EQ to IR
    bool eqEnabled = *apvts.getRawParameterValue(ParamID::eqOn) > 0.5f;
    if (eqEnabled) {
        juce::dsp::ProcessSpec irSpec;
        irSpec.sampleRate       = reader->sampleRate;
        irSpec.maximumBlockSize = (juce::uint32)irBuffer.getNumSamples();
        irSpec.numChannels      = (juce::uint32)irBuffer.getNumChannels();
        irEQ.prepareWithEQParams(irSpec, apvts);
        irEQ.processBuffer(irBuffer);
    }

    // ── Gain compensation ─────────────────────────────────────────────────────
    // Energy normalisation: gain = 1 / sqrt(sum(ir²) / nChannels)
    // This is the physically correct method for convolution unity gain.
    // It ensures E[output_RMS] ≈ E[input_RMS] (Parseval / energy conservation).
    // Peak normalisation would cause saturation; RMS-target causes -12 dB loss.
    {
        double sumSq = 0.0;
        const int nCh = irBuffer.getNumChannels();
        for (int ch = 0; ch < nCh; ++ch) {
            const float* rd = irBuffer.getReadPointer(ch);
            for (int i = 0; i < irBuffer.getNumSamples(); ++i)
                sumSq += (double)rd[i] * (double)rd[i];
        }
        double L2perCh = std::sqrt(sumSq / (double)juce::jmax(1, nCh));
        if (L2perCh > 1e-8)
            irBuffer.applyGain((float)(1.0 / L2perCh));
    }

    convEngine.loadImpulseResponse(std::move(irBuffer), reader->sampleRate);
}

void SordIRAudioProcessor::loadCustomIR (const juce::File& file)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(file));
    if (!reader) return;

    juce::AudioBuffer<float> irBuffer((int)reader->numChannels,
                                       (int)reader->lengthInSamples);
    reader->read(&irBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    convEngine.loadImpulseResponse(std::move(irBuffer), reader->sampleRate);
    currentIRIndex = -1;
}

// ─── Parameter listener ───────────────────────────────────────────────────────
void SordIRAudioProcessor::parameterChanged (const juce::String& paramID, float newValue)
{
    if (paramID == ParamID::irIndex)
        juce::MessageManager::callAsync([this, v=(int)newValue]{ loadIRByIndex(v); });

    if (paramID == ParamID::size ||
        paramID == ParamID::eqOn ||
        paramID.startsWith("eq"))
        juce::MessageManager::callAsync([this]{ loadIRByIndex(currentIRIndex); });
}

// ─── State ───────────────────────────────────────────────────────────────────
void SordIRAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SordIRAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// ─── Editor ───────────────────────────────────────────────────────────────────
juce::AudioProcessorEditor* SordIRAudioProcessor::createEditor()
{
    return new SordIRAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SordIRAudioProcessor();
}

// ─── User Presets ─────────────────────────────────────────────────────────────
juce::File SordIRAudioProcessor::getUserPresetsDir() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
           .getChildFile("Sord").getChildFile("IR Presets");
}

UserPreset SordIRAudioProcessor::captureCurrentState (const juce::String& name)
{
    UserPreset p;
    p.name      = name;
    p.irIndex   = currentIRIndex;
    p.predelay  = *apvts.getRawParameterValue(ParamID::predelay);
    p.size      = *apvts.getRawParameterValue(ParamID::size);
    p.dryWet    = *apvts.getRawParameterValue(ParamID::dryWet);
    p.gainOut   = *apvts.getRawParameterValue(ParamID::gainOut);
    p.eqOn      = *apvts.getRawParameterValue(ParamID::eqOn) > 0.5f;
    p.eq0Freq   = *apvts.getRawParameterValue(ParamID::eq0Freq);
    p.eq0Gain   = *apvts.getRawParameterValue(ParamID::eq0Gain);
    p.eq1Freq   = *apvts.getRawParameterValue(ParamID::eq1Freq);
    p.eq1Gain   = *apvts.getRawParameterValue(ParamID::eq1Gain);
    p.eq2Freq   = *apvts.getRawParameterValue(ParamID::eq2Freq);
    p.eq2Gain   = *apvts.getRawParameterValue(ParamID::eq2Gain);
    p.eq3Freq   = *apvts.getRawParameterValue(ParamID::eq3Freq);
    p.eq3Gain   = *apvts.getRawParameterValue(ParamID::eq3Gain);
    return p;
}

void SordIRAudioProcessor::applyUserPreset (const UserPreset& p)
{
    auto setParam = [&](const juce::String& id, float v) {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(
                param->getNormalisableRange().convertTo0to1(v));
    };
    setParam(ParamID::predelay, p.predelay);
    setParam(ParamID::size,     p.size);
    setParam(ParamID::dryWet,   p.dryWet);
    setParam(ParamID::gainOut,  p.gainOut);
    setParam(ParamID::eqOn,     p.eqOn ? 1.f : 0.f);
    setParam(ParamID::eq0Freq,  p.eq0Freq); setParam(ParamID::eq0Gain, p.eq0Gain);
    setParam(ParamID::eq1Freq,  p.eq1Freq); setParam(ParamID::eq1Gain, p.eq1Gain);
    setParam(ParamID::eq2Freq,  p.eq2Freq); setParam(ParamID::eq2Gain, p.eq2Gain);
    setParam(ParamID::eq3Freq,  p.eq3Freq); setParam(ParamID::eq3Gain, p.eq3Gain);

    if (p.irIndex >= 0)
        loadIRByIndex(p.irIndex);
    else if (p.customIRPath.isNotEmpty())
        loadCustomIR(juce::File(p.customIRPath));
}

void SordIRAudioProcessor::saveUserPresetToDisk (const UserPreset& p)
{
    auto dir = getUserPresetsDir();
    dir.createDirectory();
    auto file = dir.getChildFile(p.name + ".sordpreset");

    juce::XmlElement xml("SordPreset");
    xml.setAttribute("name",      p.name);
    xml.setAttribute("irIndex",   p.irIndex);
    xml.setAttribute("customIR",  p.customIRPath);
    xml.setAttribute("predelay",  p.predelay);
    xml.setAttribute("size",      p.size);
    xml.setAttribute("dryWet",    p.dryWet);
    xml.setAttribute("gainOut",   p.gainOut);
    xml.setAttribute("eqOn",      p.eqOn ? 1 : 0);
    xml.setAttribute("eq0Freq",   p.eq0Freq); xml.setAttribute("eq0Gain", p.eq0Gain);
    xml.setAttribute("eq1Freq",   p.eq1Freq); xml.setAttribute("eq1Gain", p.eq1Gain);
    xml.setAttribute("eq2Freq",   p.eq2Freq); xml.setAttribute("eq2Gain", p.eq2Gain);
    xml.setAttribute("eq3Freq",   p.eq3Freq); xml.setAttribute("eq3Gain", p.eq3Gain);
    xml.writeTo(file);
}

void SordIRAudioProcessor::deleteUserPresetFromDisk (const juce::String& name)
{
    getUserPresetsDir().getChildFile(name + ".sordpreset").deleteFile();
}

void SordIRAudioProcessor::loadUserPresetsFromDisk()
{
    userPresets.clear();
    auto dir = getUserPresetsDir();
    if (!dir.exists()) return;
    for (auto& f : dir.findChildFiles(juce::File::findFiles, false, "*.sordpreset")) {
        if (auto xml = juce::XmlDocument::parse(f)) {
            UserPreset p;
            p.name         = xml->getStringAttribute("name");
            p.irIndex      = xml->getIntAttribute   ("irIndex", 0);
            p.customIRPath = xml->getStringAttribute("customIR");
            p.predelay     = (float)xml->getDoubleAttribute("predelay", 0.0);
            p.size         = (float)xml->getDoubleAttribute("size",     1.0);
            p.dryWet       = (float)xml->getDoubleAttribute("dryWet",   1.0);
            p.gainOut      = (float)xml->getDoubleAttribute("gainOut",  0.0);
            p.eqOn         = xml->getIntAttribute("eqOn", 1) != 0;
            p.eq0Freq      = (float)xml->getDoubleAttribute("eq0Freq", 100.0);
            p.eq0Gain      = (float)xml->getDoubleAttribute("eq0Gain", 0.0);
            p.eq1Freq      = (float)xml->getDoubleAttribute("eq1Freq", 500.0);
            p.eq1Gain      = (float)xml->getDoubleAttribute("eq1Gain", 0.0);
            p.eq2Freq      = (float)xml->getDoubleAttribute("eq2Freq", 2000.0);
            p.eq2Gain      = (float)xml->getDoubleAttribute("eq2Gain", 0.0);
            p.eq3Freq      = (float)xml->getDoubleAttribute("eq3Freq", 8000.0);
            p.eq3Gain      = (float)xml->getDoubleAttribute("eq3Gain", 0.0);
            if (p.name.isNotEmpty()) userPresets.add(p);
        }
    }
}

void SordIRAudioProcessor::saveUserPreset (const juce::String& name)
{
    auto p = captureCurrentState(name);
    for (int i = 0; i < userPresets.size(); ++i) {
        if (userPresets[i].name == name) { userPresets.set(i, p); saveUserPresetToDisk(p); return; }
    }
    userPresets.add(p);
    saveUserPresetToDisk(p);
}

void SordIRAudioProcessor::loadUserPreset (int index)
{
    if (juce::isPositiveAndBelow(index, userPresets.size()))
        applyUserPreset(userPresets[index]);
}

void SordIRAudioProcessor::deleteUserPreset (int index)
{
    if (!juce::isPositiveAndBelow(index, userPresets.size())) return;
    deleteUserPresetFromDisk(userPresets[index].name);
    userPresets.remove(index);
}
