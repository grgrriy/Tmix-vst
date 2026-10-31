#include "PluginEditor.h"
static const juce::Colour bg { 0xff10121a }, panel { 0xff1a1e2a }, accent { 0xffad91ff }, muted { 0xff9298aa };

TMixAudioProcessorEditor::TMixAudioProcessorEditor(TMixAudioProcessor& p) : AudioProcessorEditor(&p), processor(p)
{
    setSize(620, 500);
    title.setText("T-Mix", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(30.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colours::white); addAndMakeVisible(title);
    subtitle.setText("INTELLIGENT MIXING SYSTEM  /  V 0.2", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    subtitle.setColour(juce::Label::textColourId, muted); addAndMakeVisible(subtitle);
    auto setupCombo = [this](juce::ComboBox& c) {
        c.setColour(juce::ComboBox::backgroundColourId, panel);
        c.setColour(juce::ComboBox::textColourId, juce::Colours::white);
        c.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff34394a)); addAndMakeVisible(c);
    };
    setupCombo(roleBox); roleBox.addItemList(TMixAudioProcessor::roleNames(), 1);
    roleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters, "role", roleBox);
    setupCombo(modeBox); modeBox.addItemList({ "AUTO MIX", "AI ASSIST" }, 1);
    modeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters, "mode", modeBox);
    setupCombo(genreBox); genreBox.addItemList({ "Neutral", "Modern Pop", "Electronic", "Rock", "Cinematic", "Hip-Hop" }, 1);
    genreAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters, "genre", genreBox);

    auto setupSlider = [this](juce::Slider& s, const juce::String& suffix) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20); s.setTextValueSuffix(suffix);
        s.setColour(juce::Slider::rotarySliderFillColourId, accent);
        s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        s.setColour(juce::Slider::textBoxBackgroundColourId, panel); addAndMakeVisible(s);
    };
    setupSlider(amount, "");
    amount.textFromValueFunction = [](double v) { return juce::String(juce::roundToInt(v * 100.0)) + "%"; };
    amount.valueFromTextFunction = [](const juce::String& t) { return t.getFloatValue() / 100.0; };
    setupSlider(output, " dB"); setupSlider(width, "x");
    amountAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "amount", amount);
    outputAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "output", output);
    widthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "width", width);

    auto setupToggle = [this](juce::ToggleButton& b, const juce::String& t) {
        b.setButtonText(t); b.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
        b.setColour(juce::ToggleButton::tickColourId, accent); addAndMakeVisible(b);
    };
    setupToggle(eq,"Smart EQ"); setupToggle(comp,"Compressor"); setupToggle(sat,"Saturation");
    setupToggle(stereo,"Stereo Control"); setupToggle(bypass,"Bypass");
    eqAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"eq",eq);
    compAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"comp",comp);
    satAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"sat",sat);
    stereoAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"stereo",stereo);
    bypassAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"bypass",bypass);
    for (auto* l : { &levelLabel, &peakLabel, &grLabel, &correlationLabel }) {
        l->setColour(juce::Label::textColourId, juce::Colours::white);
        l->setFont(juce::Font(juce::FontOptions(13.0f))); addAndMakeVisible(*l);
    }
    analyzeButton.setColour(juce::TextButton::buttonColourId, accent);
    analyzeButton.setColour(juce::TextButton::textColourOffId, bg);
    analyzeButton.onClick = [this] { analyzeButton.setButtonText("ANALYZING LIVE INPUT..."); };
    addAndMakeVisible(analyzeButton); startTimerHz(12);
}

void TMixAudioProcessorEditor::timerCallback()
{
    levelLabel.setText("RMS  " + juce::String(juce::Decibels::gainToDecibels(processor.inputRms.load(), -100.0f),1)+" dB",juce::dontSendNotification);
    peakLabel.setText("PEAK  "+juce::String(processor.peakDb.load(),1)+" dB",juce::dontSendNotification);
    grLabel.setText("GR  "+juce::String(processor.gainReduction.load()*100.0f,0)+"%",juce::dontSendNotification);
    correlationLabel.setText("CORR  "+juce::String(processor.stereoCorrelation.load(),2),juce::dontSendNotification);
}
void TMixAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(panel); g.fillRoundedRectangle(18,92,584,104,12);
    g.fillRoundedRectangle(18,208,584,100,12); g.fillRoundedRectangle(18,320,584,160,12);
    g.setColour(accent); g.fillRoundedRectangle(18,92,4,104,2);
    g.setColour(muted); g.setFont(11.0f);
    g.drawText("TRACK INTELLIGENCE",32,99,180,16,juce::Justification::left);
    g.drawText("LIVE SIGNAL ANALYSIS",32,215,180,16,juce::Justification::left);
    g.drawText("PROCESSING CHAIN",32,327,180,16,juce::Justification::left);
    g.setColour(juce::Colour(0xff34394a)); g.drawHorizontalLine(306,32,588);
}
void TMixAudioProcessorEditor::resized()
{
    title.setBounds(22,12,180,42); subtitle.setBounds(24,51,260,20); bypass.setBounds(510,24,90,24);
    roleBox.setBounds(32,133,270,34); modeBox.setBounds(316,133,130,34); genreBox.setBounds(458,133,130,34);
    levelLabel.setBounds(32,245,130,25); peakLabel.setBounds(170,245,130,25);
    grLabel.setBounds(310,245,100,25); correlationLabel.setBounds(420,245,150,25);
    analyzeButton.setBounds(32,275,175,24);
    amount.setBounds(26,345,120,110); output.setBounds(158,345,120,110); width.setBounds(290,345,120,110);
    eq.setBounds(435,350,155,22); comp.setBounds(435,376,155,22); sat.setBounds(435,402,155,22); stereo.setBounds(435,428,155,22);
}
