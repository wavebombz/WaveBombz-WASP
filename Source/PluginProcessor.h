#pragma once
#include <JuceHeader.h>
#include <atomic>
class WaveBombzWASPProcessor final : public juce::AudioProcessor {
public:
 WaveBombzWASPProcessor(); ~WaveBombzWASPProcessor() override = default;
 void prepareToPlay(double,int) override; void releaseResources() override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override;
 const juce::String getName() const override; bool acceptsMidi() const override; bool producesMidi() const override; bool isMidiEffect() const override;
 double getTailLengthSeconds() const override; int getNumPrograms() override; int getCurrentProgram() override; void setCurrentProgram(int) override;
 const juce::String getProgramName(int) override; void changeProgramName(int,const juce::String&) override;
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState& getState(); float getMomentaryLUFS() const; float getIntegratedLUFS() const; float getPeak() const;
 static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:
 juce::AudioProcessorValueTreeState apvts; juce::Synthesiser synth;
 std::vector<std::pair<juce::dsp::IIR::Filter<float>,juce::dsp::IIR::Filter<float>>> meterFilters;
 std::atomic<float> momentary{-100.0f}, integrated{-100.0f}, peak{0.0f};
 double integratedEnergy=0.0; uint64_t integratedSamples=0;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveBombzWASPProcessor)
};
