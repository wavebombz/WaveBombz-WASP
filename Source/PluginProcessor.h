#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <array>
#include <vector>
#include <utility>
#include <memory>
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
 bool savePreset(const juce::File&); bool loadPreset(const juce::File&); void loadFactoryPreset(int index);
 bool loadOscillatorSample(int oscillator, const juce::File& file); juce::String getOscillatorSampleName(int oscillator) const;
 juce::AudioProcessorValueTreeState& getState(); float getMomentaryLUFS() const; float getIntegratedLUFS() const; float getPeak();
 static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:
 juce::AudioProcessorValueTreeState apvts; juce::Synthesiser synth;
 juce::AudioFormatManager sampleFormats;
 std::array<std::shared_ptr<const juce::AudioBuffer<float>>,3> oscillatorSamples{};
 std::array<std::atomic<double>,3> oscillatorSampleRates{};
 void restoreOscillatorSamples();
 std::vector<std::pair<juce::dsp::IIR::Filter<float>,juce::dsp::IIR::Filter<float>>> meterFilters;
 juce::dsp::Compressor<float> compressor;
 juce::dsp::Reverb reverb;
 juce::dsp::Chorus<float> flanger, chorus;
 juce::dsp::Phaser<float> phaser;
 juce::dsp::DelayLine<float,juce::dsp:: DelayLineInterpolationTypes::Linear> delay { 192000 };
 std::vector<juce::dsp::IIR::Filter<float>> eqLow, eqHigh;
 std::atomic<float> momentary{-100.0f}, integrated{-100.0f}, peak{0.0f};
 double integratedEnergy=0.0; uint64_t integratedSamples=0; int monoNote=-1;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveBombzWASPProcessor)
};
