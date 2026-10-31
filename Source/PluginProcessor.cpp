#include "PluginProcessor.h"
#include "PluginEditor.h"

TMixAudioProcessor::TMixAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "TMIX_STATE", createParameters())
{}

juce::StringArray TMixAudioProcessor::roleNames()
{
    return { "Guitar", "Bass", "Synth", "Piano", "Orchestra", "Vocal",
             "Vocal Chop", "Kick", "Snare", "Hi-Hat", "Percussion",
             "Drum Bus", "Master", "FX", "Custom" };
}

juce::AudioProcessorValueTreeState::ParameterLayout TMixAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<juce::AudioParameterChoice>("role", "Track Type", roleNames(), 0));
    p.add(std::make_unique<juce::AudioParameterFloat>("amount", "Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.65f));
    p.add(std::make_unique<juce::AudioParameterChoice>("mode", "Mix Mode", juce::StringArray { "AUTO MIX", "AI ASSIST" }, 0));
    p.add(std::make_unique<juce::AudioParameterChoice>("genre", "Style", juce::StringArray { "Neutral", "Modern Pop", "Electronic", "Rock", "Cinematic", "Hip-Hop" }, 0));
    p.add(std::make_unique<juce::AudioParameterFloat>("width", "Stereo Width", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));
    p.add(std::make_unique<juce::AudioParameterFloat>("output", "Output",
        juce::NormalisableRange<float>(-18.0f, 12.0f, 0.1f), 0.0f, "dB"));
    p.add(std::make_unique<juce::AudioParameterBool>("eq", "Smart EQ", true));
    p.add(std::make_unique<juce::AudioParameterBool>("comp", "Compressor", true));
    p.add(std::make_unique<juce::AudioParameterBool>("sat", "Saturation", false));
    p.add(std::make_unique<juce::AudioParameterBool>("stereo", "Stereo Control", false));
    p.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
    return p;
}

void TMixAudioProcessor::prepareToPlay(double sr, int)
{
    currentSampleRate = sr;
    juce::dsp::ProcessSpec spec { sr, 2048, (juce::uint32)getTotalNumOutputChannels() };
    for (auto* f : { &highPassL, &highPassR, &lowShelfL, &lowShelfR,
                     &presenceL, &presenceR, &airL, &airR }) f->prepare(spec);
    envelope = 0.0f;
}

void TMixAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int chs = juce::jmin(2, buffer.getNumChannels());
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, n);

    const bool bypass = parameters.getRawParameterValue("bypass")->load() > 0.5f;
    const float requestedAmount = parameters.getRawParameterValue("amount")->load();
    const int mode = (int)parameters.getRawParameterValue("mode")->load();
    const int role = (int)parameters.getRawParameterValue("role")->load();
    const int genre = (int)parameters.getRawParameterValue("genre")->load();
    const float amount = requestedAmount * (mode == 1 ? 0.55f : 1.0f); // Assist is deliberately gentler.
    const float outputGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("output")->load());
    const float width = parameters.getRawParameterValue("width")->load();

    float sumSq = 0.0f, peak = 0.0f, cross = 0.0f, leftSq = 0.0f, rightSq = 0.0f;
    for (int i = 0; i < n; ++i) {
        float l = chs > 0 ? buffer.getReadPointer(0)[i] : 0.0f;
        float r = chs > 1 ? buffer.getReadPointer(1)[i] : l;
        sumSq += l*l + (chs > 1 ? r*r : 0.0f);
        leftSq += l*l; rightSq += r*r; cross += l*r;
        peak = juce::jmax(peak, juce::jmax(std::abs(l), std::abs(r)));
    }
    const float rms = std::sqrt(sumSq / juce::jmax(1, n * chs));
    inputRms.store(rms);
    peakDb.store(juce::Decibels::gainToDecibels(peak, -100.0f));
    stereoCorrelation.store(cross / std::sqrt(juce::jmax(1.0e-12f, leftSq * rightSq)));
    if (bypass || chs == 0) return;

    // Role-aware conservative starting curves; style adds small tonal intent.
    float hp = 32.0f, lowDb = 0.0f, presenceDb = 0.0f, airDb = 0.0f;
    switch (role) {
        case Bass: hp=25; lowDb=0.5f; presenceDb=0.4f; break;
        case Kick: hp=25; lowDb=0.8f; presenceDb=0.5f; break;
        case Guitar: hp=75; presenceDb=0.7f; airDb=0.2f; break;
        case Piano: hp=55; presenceDb=0.4f; airDb=0.3f; break;
        case Vocal: case VocalChop: hp=85; presenceDb=0.8f; airDb=0.5f; break;
        case Orchestra: hp=45; presenceDb=0.2f; airDb=0.2f; break;
        case HiHat: hp=300; airDb=0.5f; break;
        case Master: hp=20; break;
        default: hp=35; break;
    }
    if (genre == 1) { presenceDb += 0.25f; airDb += 0.2f; }
    if (genre == 2) { lowDb += 0.2f; presenceDb += 0.2f; }
    if (genre == 3) { lowDb += 0.2f; presenceDb += 0.35f; }
    if (genre == 4) { airDb += 0.25f; }
    if (genre == 5) { lowDb += 0.25f; }

    auto coeff = [this](float freq, float gain, bool highPass = false) {
        if (highPass) return juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, freq);
        return juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, freq, 0.8f,
                    juce::Decibels::decibelsToGain(gain));
    };
    *highPassL.coefficients = *coeff(hp, 0, true); *highPassR.coefficients = *coeff(hp, 0, true);
    *lowShelfL.coefficients = *coeff(120, lowDb*amount); *lowShelfR.coefficients = *coeff(120, lowDb*amount);
    *presenceL.coefficients = *coeff(3200, presenceDb*amount); *presenceR.coefficients = *coeff(3200, presenceDb*amount);
    *airL.coefficients = *coeff(10500, airDb*amount); *airR.coefficients = *coeff(10500, airDb*amount);

    const bool useEq = parameters.getRawParameterValue("eq")->load() > 0.5f;
    const bool useComp = parameters.getRawParameterValue("comp")->load() > 0.5f;
    const bool useSat = parameters.getRawParameterValue("sat")->load() > 0.5f;
    const bool useWidth = parameters.getRawParameterValue("stereo")->load() > 0.5f;
    const float threshold = juce::Decibels::decibelsToGain(-16.0f + (1.0f-amount)*8.0f);
    const float attack = std::exp(-1.0f / (0.012f*(float)currentSampleRate));
    const float release = std::exp(-1.0f / (0.140f*(float)currentSampleRate));
    float lastGR = 0.0f;

    // Stereo-linked detector avoids image wandering: one gain envelope controls L/R.
    for (int i=0; i<n; ++i) {
        float samples[2] = { chs>0 ? buffer.getWritePointer(0)[i] : 0.0f,
                             chs>1 ? buffer.getWritePointer(1)[i] : 0.0f };
        if (useEq) {
            samples[0]=highPassL.processSample(samples[0]); samples[0]=lowShelfL.processSample(samples[0]);
            samples[0]=presenceL.processSample(samples[0]); samples[0]=airL.processSample(samples[0]);
            if (chs>1) {
                samples[1]=highPassR.processSample(samples[1]); samples[1]=lowShelfR.processSample(samples[1]);
                samples[1]=presenceR.processSample(samples[1]); samples[1]=airR.processSample(samples[1]);
            }
        }
        const float detector = juce::jmax(std::abs(samples[0]), chs>1 ? std::abs(samples[1]) : 0.0f);
        envelope = detector > envelope ? attack*envelope+(1-attack)*detector : release*envelope+(1-release)*detector;
        float gain = 1.0f;
        if (useComp && envelope > threshold) {
            const float over = envelope / threshold;
            gain = std::pow(over, -0.45f*amount);
            lastGR = 1.0f-gain;
        }
        for (int ch=0; ch<chs; ++ch) {
            float s = samples[ch] * gain;
            if (useSat) s = std::tanh(s*(1.0f+amount*1.5f))/std::tanh(1.0f+amount*1.5f);
            buffer.getWritePointer(ch)[i] = s;
        }
        if (chs>1 && useWidth) {
            float l=buffer.getWritePointer(0)[i], r=buffer.getWritePointer(1)[i];
            const float mid=(l+r)*0.5f, side=(l-r)*0.5f*width;
            buffer.getWritePointer(0)[i]=(mid+side)*outputGain;
            buffer.getWritePointer(1)[i]=(mid-side)*outputGain;
        } else {
            for (int ch=0; ch<chs; ++ch) buffer.getWritePointer(ch)[i] *= outputGain;
        }
    }
    gainReduction.store(lastGR);
}

juce::AudioProcessorEditor* TMixAudioProcessor::createEditor() { return new TMixAudioProcessorEditor(*this); }

void TMixAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void TMixAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TMixAudioProcessor(); }
