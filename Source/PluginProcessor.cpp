#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
namespace { constexpr float pi=juce::MathConstants<float>::pi;
float wave(int shape,float phase){float p=phase/(2.0f*pi);switch(shape){case 1:return 2.0f*p-1.0f;case 2:return p<0.5f?1.0f:-1.0f;case 3:return std::sin(phase)>=0.0f?1.0f:-1.0f;default:return std::sin(phase);}}
class WaspSound final:public juce::SynthesiserSound {public: bool appliesToNote(int) override{return true;} bool appliesToChannel(int) override{return true;}};
class WaspVoice final:public juce::SynthesiserVoice {
public:
 explicit WaspVoice(juce::AudioProcessorValueTreeState& s):state(s){}
 bool canPlaySound(juce::SynthesiserSound* s) override{return dynamic_cast<WaspSound*>(s)!=nullptr;}
 void startNote(int midi,float velocity,juce::SynthesiserSound*,int) override{hz=(float)juce::MidiMessage::getMidiNoteInHertz(midi);vel=velocity;phase.fill(0);filter=0;amp.noteOn();fenv.noteOn();}
 void stopNote(float,bool tail) override{amp.noteOff();fenv.noteOff();if(!tail)clearCurrentNote();}
 void pitchWheelMoved(int) override{} void controllerMoved(int,int) override{}
 void prepare(double sr){rate=sr;amp.setSampleRate(sr);fenv.setSampleRate(sr);amp.setParameters({.01f,.18f,.8f,.25f});fenv.setParameters({.01f,.2f,.6f,.3f});}
 void renderNextBlock(juce::AudioBuffer<float>& out,int start,int n) override{
  if(!isVoiceActive())return;
  auto v=[this](const char* id,float d=0.0f){auto* p=state.getRawParameterValue(id);return p?p->load():d;};
  amp.setParameters({v("ATTACK",.01f),v("DECAY",.18f),v("SUSTAIN",.8f),v("RELEASE",.25f)});
  fenv.setParameters({v("FATTACK",.01f),v("FDECAY",.2f),v("FSUSTAIN",.6f),v("FRELEASE",.3f)});
  const int s1=(int)v("OSC1WAVE",1),s2=(int)v("OSC2WAVE",0),s3=(int)v("OSC3WAVE",2);
  const float m1=v("OSC1MIX",.8f),m2=v("OSC2MIX",.45f),m3=v("OSC3MIX",.25f);
  const float t2=std::pow(2.0f,v("OSC2TUNE")/12.0f),t3=std::pow(2.0f,v("OSC3TUNE",12)/12.0f);
  const float fm=v("FM"),ring=v("RING"),drive=v("DRIVE",1),lrate=v("LFO1RATE",2),lamt=v("LFO1AMT"),ec=v("ENV2CUT"),lc=v("LFO1CUT");
  const float sr=(float)rate;
  for(int i=0;i<n;++i){
   float env=amp.getNextSample(),fe=fenv.getNextSample(),lfo=std::sin(lfoPhase);
   lfoPhase+=2*pi*lrate/sr;if(lfoPhase>2*pi)lfoPhase-=2*pi;
   float a=wave(s1,phase[0]),b=wave(s2,phase[1]+fm*a*4),c=wave(s3,phase[2]);
   phase[0]+=2*pi*hz/sr;phase[1]+=2*pi*hz*t2/sr;phase[2]+=2*pi*hz*t3/sr;for(auto& q:phase)if(q>2*pi)q-=2*pi;
   float x=(a*m1+b*m2+c*m3)*(1-ring)+(a*b)*ring;
   float fc=juce::jlimit(25.0f,18000.0f,v("CUTOFF",8000)*std::pow(2.0f,(fe*ec+lfo*lc+lfo*lamt)*4));
   filter+=(1-std::exp(-2*pi*fc/sr))*(x-filter);x=std::tanh(filter*drive*2)*env*vel*.22f;
   for(int ch=0;ch<out.getNumChannels();++ch)out.addSample(ch,start+i,x);
   if(!amp.isActive()){clearCurrentNote();break;}
  }
 }
private:
 juce::AudioProcessorValueTreeState& state;juce::ADSR amp,fenv;std::array<float,3> phase{0,0,0};double rate=44100;float hz=440,vel=0,filter=0,lfoPhase=0;
};
}
WaveBombzWASPProcessor::WaveBombzWASPProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),apvts(*this,nullptr,"PARAMETERS",createParameterLayout()){
 for(int i=0;i<12;++i)synth.addVoice(new WaspVoice(apvts));synth.addSound(new WaspSound());
}
juce::AudioProcessorValueTreeState::ParameterLayout WaveBombzWASPProcessor::createParameterLayout(){
 using F=juce::AudioParameterFloat;using C=juce::AudioParameterChoice;juce::AudioProcessorValueTreeState::ParameterLayout p;
 auto f=[&](const char* id,const char* name,float lo,float hi,float d){p.add(std::make_unique<F>(id,name,juce::NormalisableRange<float>(lo,hi,0.001f),d));};
 auto c=[&](const char* id,const char* name,juce::StringArray a,int d){p.add(std::make_unique<C>(id,name,a,d));};
 c("OSC1WAVE","OSC 1 Wave",{"Sine","Saw","Square","Pulse"},1);c("OSC2WAVE","OSC 2 Wave",{"Sine","Saw","Square","Pulse"},0);c("OSC3WAVE","OSC 3 Wave",{"Sine","Saw","Square","Pulse"},2);
 f("OSC1MIX","OSC 1 Mix",0,1,.8f);f("OSC2MIX","OSC 2 Mix",0,1,.45f);f("OSC3MIX","OSC 3 Mix",0,1,.25f);f("OSC2TUNE","OSC 2 Tune",-24,24,0);f("OSC3TUNE","OSC 3 Tune",-24,24,12);
 f("CUTOFF","Filter Cutoff",25,18000,8000);f("RESONANCE","Resonance",0,1,.1f);
 f("ATTACK","Amp Attack",.001f,5,.01f);f("DECAY","Amp Decay",.001f,5,.18f);f("SUSTAIN","Amp Sustain",0,1,.8f);f("RELEASE","Amp Release",.001f,8,.25f);
 f("FATTACK","Filter Attack",.001f,5,.01f);f("FDECAY","Filter Decay",.001f,5,.2f);f("FSUSTAIN","Filter Sustain",0,1,.6f);f("FRELEASE","Filter Release",.001f,8,.3f);
 f("FM","FM Amount",0,1,0);f("RING","Ring Mod",0,1,0);f("DRIVE","Distortion Drive",.1f,5,1);
 f("LFO1RATE","LFO 1 Rate",.05f,20,2);f("LFO1AMT","LFO 1 Amount",0,1,0);f("ENV2CUT","ENV 2 > Cutoff",0,1,0);f("LFO1CUT","LFO 1 > Cutoff",0,1,0);f("MASTER","Master",0,1,.75f);return p;
}
void WaveBombzWASPProcessor::prepareToPlay(double sr,int){
 synth.setCurrentPlaybackSampleRate(sr);for(int i=0;i<synth.getNumVoices();++i)if(auto* v=dynamic_cast<WaspVoice*>(synth.getVoice(i)))v->prepare(sr);
 meterFilters.clear();meterFilters.resize((size_t)getTotalNumOutputChannels());
 for(auto& f:meterFilters){f.first.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,60.0f);f.second.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr,1500.0f,.7f,juce::Decibels::decibelsToGain(4.0f));}
}
void WaveBombzWASPProcessor::releaseResources(){}
bool WaveBombzWASPProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return l.getMainOutputChannelSet()==juce::AudioChannelSet::mono()||l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
void WaveBombzWASPProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals no;b.clear();synth.renderNextBlock(b,midi,0,b.getNumSamples());b.applyGain(apvts.getRawParameterValue("MASTER")->load());
 double sum=0;int count=0;
 for(int ch=0;ch<b.getNumChannels();++ch)for(int i=0;i<b.getNumSamples();++i){float x=b.getSample(ch,i);if((size_t)ch<meterFilters.size()){x=meterFilters[(size_t)ch].first.processSample(x);x=meterFilters[(size_t)ch].second.processSample(x);}sum+=(double)x*x;++count;float pk=std::abs(b.getSample(ch,i)),old=peak.load();while(pk>old&&!peak.compare_exchange_weak(old,pk)){}}
 if(count){float rms=(float)std::sqrt(sum/count);float l=juce::Decibels::gainToDecibels(rms,-100.0f)-.691f;momentary.store(l);if(l>-70){integratedEnergy+=sum;integratedSamples+=(uint64_t)count;integrated.store(juce::Decibels::gainToDecibels((float)std::sqrt(integratedEnergy/(double)integratedSamples),-100.0f)-.691f);}}
}
juce::AudioProcessorEditor* WaveBombzWASPProcessor::createEditor(){return new WaveBombzWASPEditor(*this);}bool WaveBombzWASPProcessor::hasEditor()const{return true;}
const juce::String WaveBombzWASPProcessor::getName()const{return JucePlugin_Name;}bool WaveBombzWASPProcessor::acceptsMidi()const{return true;}bool WaveBombzWASPProcessor::producesMidi()const{return false;}bool WaveBombzWASPProcessor::isMidiEffect()const{return false;}double WaveBombzWASPProcessor::getTailLengthSeconds()const{return 8;}
int WaveBombzWASPProcessor::getNumPrograms(){return 1;}int WaveBombzWASPProcessor::getCurrentProgram(){return 0;}void WaveBombzWASPProcessor::setCurrentProgram(int){}const juce::String WaveBombzWASPProcessor::getProgramName(int){return {};}void WaveBombzWASPProcessor::changeProgramName(int,const juce::String&){}
void WaveBombzWASPProcessor::getStateInformation(juce::MemoryBlock& d){auto s=apvts.copyState();std::unique_ptr<juce::XmlElement>x(s.createXml());copyXmlToBinary(*x,d);}
void WaveBombzWASPProcessor::setStateInformation(const void* d,int n){std::unique_ptr<juce::XmlElement>x(getXmlFromBinary(d,n));if(x&&x->hasTagName(apvts.state.getType()))apvts.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessorValueTreeState& WaveBombzWASPProcessor::getState(){return apvts;}float WaveBombzWASPProcessor::getMomentaryLUFS()const{return momentary.load();}float WaveBombzWASPProcessor::getIntegratedLUFS()const{return integrated.load();}float WaveBombzWASPProcessor::getPeak()const{return peak.exchange(0.0f);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new WaveBombzWASPProcessor();}
