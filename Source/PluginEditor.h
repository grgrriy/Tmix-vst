#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TMixAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TMixAudioProcessorEditor(TMixAudioProcessor&);
    ~TMixAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    TMixAudioProcessor& processor;
    juce::ComboBox roleBox, modeBox, genreBox;
    juce::Slider amount, output, width;
    juce::ToggleButton eq, comp, sat, stereo, bypass;
    juce::Label title, subtitle, levelLabel, peakLabel, grLabel, correlationLabel;
    juce::TextButton analyzeButton { "LIVE ANALYSIS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> roleAttach, modeAttach, genreAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttach, outputAttach, widthAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqAttach, compAttach, satAttach, stereoAttach, bypassAttach;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TMixAudioProcessorEditor)
};
