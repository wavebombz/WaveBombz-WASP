#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class WaspLookAndFeel final : public juce::LookAndFeel_V4 {
public: WaspLookAndFeel(); void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};
class WaveBombzWASPEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public: explicit WaveBombzWASPEditor(WaveBombzWASPProcessor&); ~WaveBombzWASPEditor() override; void paint(juce::Graphics&) override; void resized() override;
private:
 void timerCallback() override; void addKnob(const juce::String&,const juce::String&);
 using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment; using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
 WaveBombzWASPProcessor& processor; WaspLookAndFeel look;
 juce::Label title,subtitle,meterText,modTitle,oscLabel,filterLabel,envLabel,lfoLabel,modDepthLabel;
 std::vector<std::unique_ptr<juce::Slider>> knobs; std::vector<std::unique_ptr<SliderAttachment>> attachments; std::unique_ptr<ComboAttachment> sourceAttachment,targetAttachment; std::unique_ptr<SliderAttachment> depthAttachment;
 juce::ComboBox modSource,modTarget; juce::Slider modDepth;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveBombzWASPEditor)
};
