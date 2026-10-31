#pragma once
#include <JuceHeader.h>
#include <atomic>

class TMixAudioProcessor : public juce::AudioProcessor
{
public:
    enum Role {
        Guitar, Bass, Synth, Piano, Orchestra, Vocal, VocalChop,
        Kick, Snare, HiHat, Percussion, DrumBus, Master, FX, Custom
    };

    TMixAudioProcessor();
    ~TMixAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "T-Mix"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> inputRms { 0.0f }, peakDb { -100.0f }, gainReduction { 0.0f };
    std::atomic<float> centroidHz { 0.0f };
    std::atomic<float> stereoCorrelation { 1.0f };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    static juce::StringArray roleNames();

private:
    juce::dsp::IIR::Filter<float> highPassL, highPassR, lowShelfL, lowShelfR,
                                  presenceL, presenceR, airL, airR;
    double currentSampleRate = 44100.0;
    float envelope = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TMixAudioProcessor)
};
