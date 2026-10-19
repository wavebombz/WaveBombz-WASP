#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
namespace { constexpr float pi=juce::MathConstants<float>::pi;
float wave(int shape,float phase){float p=phase/(2.0f*pi);switch(shape){case 1:return 2.0f*p-1.0f;case 2:return p<0.5f?1.0f:-1.0f;case 3:return std::sin(phase)>=0.0f?1.0f:-1.0f;default:return std::sin(phase);}}
class WaspSound final:public juce::SynthesiserSound {public: bool appliesToNote(int) override{return true;} bool appliesToChannel(int) override{return true;}};
class WaspVoice final:public juce::SynthesiserVoice {
public:
 explicit WaspVoice(juce::AudioProcessorValueTreeState& s,std::array<std::shared_ptr<const juce::AudioBuffer<float>>,3>& samples,std::array<std::atomic<double>,3>& rates):state(s),sampleBuffers(samples),sampleRates(rates){}
 bool canPlaySound(juce::SynthesiserSound* s) override{return dynamic_cast<WaspSound*>(s)!=nullptr;}
 void startNote(int midi,float velocity,juce::SynthesiserSound*,int) override{targetHz=(float)juce::MidiMessage::getMidiNoteInHertz(midi);float porta=state.getRawParameterValue("PORTAMENTO")->load();if(hz<=0.0f||porta<=0.001f||(int)state.getRawParameterValue("MONO")->load()!=2)hz=targetHz;vel=velocity;midiNote=midi;phase.fill(0);samplePositions.fill(0.0);filter=0;amp.noteOn();fenv.noteOn();}
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
  const float analog=v("ANALOG",0.25f),digital=v("DIGITALEDGE",0.0f); const float t1=std::pow(2.0f,(v("OSC1TUNE")+12.0f*(v("OSC1OCT",2)-2.0f))/12.0f),t2=std::pow(2.0f,(v("OSC2TUNE")+12.0f*(v("OSC2OCT",2)-2.0f))/12.0f+analog*0.006f),t3=std::pow(2.0f,(v("OSC3TUNE",12)+12.0f*(v("OSC3OCT",3)-2.0f))/12.0f-analog*0.009f);
  const float fm=juce::jlimit(0.0f,1.0f,v("FM")+digital*0.35f),ring=juce::jlimit(0.0f,1.0f,v("RING")+digital*0.3f),drive=v("DRIVE",1),lrate=v("LFO1RATE",2),lamt=v("LFO1AMT"),ec=v("ENV2CUT"),lc=v("LFO1CUT");
  const int modSource=(int)v("MODSOURCE"),modTarget=(int)v("MODTARGET"); const float modDepth=v("MODDEPTH",0.35f);
  const float sr=(float)rate;
  std::array<std::shared_ptr<const juce::AudioBuffer<float>>,3> samples;for(size_t k=0;k<samples.size();++k)samples[k]=std::atomic_load(&sampleBuffers[k]);
  for(int i=0;i<n;++i){
   float glide=state.getRawParameterValue("PORTAMENTO")->load();if(glide>0.001f)hz+=(targetHz-hz)*juce::jlimit(0.00001f,0.02f,1.0f/(glide*1200.0f));else hz=targetHz;
   float env=amp.getNextSample(),fe=fenv.getNextSample(),lfo=std::sin(lfoPhase);
   lfoPhase+=2*pi*lrate/sr;if(lfoPhase>2*pi)lfoPhase-=2*pi;
   auto sampleValue=[&](int slot){const auto& sm=samples[(size_t)slot];if(!sm||sm->getNumSamples()<2)return wave(slot==0?s1:(slot==1?s2:s3),phase[(size_t)slot]);double pos=samplePositions[(size_t)slot];int idx=(int)pos;if(idx<0||idx>=sm->getNumSamples())return 0.0f;int next=juce::jmin(idx+1,sm->getNumSamples()-1);float frac=(float)(pos-idx);float value=sm->getSample(0,idx)+(sm->getSample(0,next)-sm->getSample(0,idx))*frac;double sourceRate=sampleRates[(size_t)slot].load();const char* tuneIds[]={"OSC1TUNE","OSC2TUNE","OSC3TUNE"};const char* octaveIds[]={"OSC1OCT","OSC2OCT","OSC3OCT"};double tune=v(tuneIds[slot]);double octave=(v(octaveIds[slot],2.0f)-2.0)*12.0;double pitch=std::pow(2.0,((double)(midiNote-60)+tune+octave)/12.0);samplePositions[(size_t)slot]+=sourceRate/rate*pitch;return value;};
   float a=sampleValue(0),b=sampleValue(1),c=sampleValue(2);
   const float route=(modSource==0?fe:lfo)*modDepth;
   phase[0]+=2*pi*hz*t1/sr;phase[1]+=2*pi*hz*t2/sr;phase[2]+=2*pi*hz*t3/sr;for(auto& q:phase)if(q>2*pi)q-=2*pi;
   float x=(a*juce::jlimit(0.0f,1.0f,m1+(modTarget==1?route:0.0f))+b*juce::jlimit(0.0f,1.0f,m2+(modTarget==2?route:0.0f))+c*m3)*(1-ring)+(a*b)*ring; x=std::tanh(x*(1.0f+analog*0.18f));
   float routeCut=(modTarget==0?route:0.0f); float fc=juce::jlimit(25.0f,18000.0f,v("CUTOFF",8000)*std::pow(2.0f,(fe*ec+lfo*lc+lfo*lamt+routeCut)*4));
   filter+=(1-std::exp(-2*pi*fc/sr))*(x-filter);x=std::tanh(filter*drive*2)*env*vel*.22f;
   for(int ch=0;ch<out.getNumChannels();++ch)out.addSample(ch,start+i,x);
   if(!amp.isActive()){clearCurrentNote();break;}
  }
 }
private:
 juce::AudioProcessorValueTreeState& state;std::array<std::shared_ptr<const juce::AudioBuffer<float>>,3>& sampleBuffers;std::array<std::atomic<double>,3>& sampleRates;juce::ADSR amp,fenv;std::array<float,3> phase{0,0,0};std::array<double,3> samplePositions{0,0,0};double rate=44100;int midiNote=60;float hz=440,targetHz=440,vel=0,filter=0,lfoPhase=0;
};
}
WaveBombzWASPProcessor::WaveBombzWASPProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),apvts(*this,nullptr,"PARAMETERS",createParameterLayout()){
 sampleFormats.registerBasicFormats();
 for(auto& sr:oscillatorSampleRates)sr.store(44100.0);
 for(int i=0;i<12;++i)synth.addVoice(new WaspVoice(apvts,oscillatorSamples,oscillatorSampleRates));synth.addSound(new WaspSound());
}
juce::AudioProcessorValueTreeState::ParameterLayout WaveBombzWASPProcessor::createParameterLayout(){
 using F=juce::AudioParameterFloat;using C=juce::AudioParameterChoice;juce::AudioProcessorValueTreeState::ParameterLayout p;
 auto f=[&](const char* id,const char* name,float lo,float hi,float d){p.add(std::make_unique<F>(id,name,juce::NormalisableRange<float>(lo,hi,0.001f),d));};
 auto c=[&](const char* id,const char* name,juce::StringArray a,int d){p.add(std::make_unique<C>(id,name,a,d));};
 c("OSC1WAVE","OSC 1 Wave",{"Sine","Saw","Square","Pulse"},1);c("OSC2WAVE","OSC 2 Wave",{"Sine","Saw","Square","Pulse"},0);c("OSC3WAVE","OSC 3 Wave",{"Sine","Saw","Square","Pulse"},2);
 f("OSC1MIX","OSC 1 Mix",0,1,.8f);f("OSC2MIX","OSC 2 Mix",0,1,.45f);f("OSC3MIX","OSC 3 Mix",0,1,.25f);f("OSC1TUNE","OSC 1 Fine Tune",-24,24,0);f("OSC2TUNE","OSC 2 Fine Tune",-24,24,0);f("OSC3TUNE","OSC 3 Fine Tune",-24,24,12);c("OSC1OCT","OSC 1 Octave",{"-2 oct","-1 oct","0 oct","+1 oct","+2 oct"},2);c("OSC2OCT","OSC 2 Octave",{"-2 oct","-1 oct","0 oct","+1 oct","+2 oct"},2);c("OSC3OCT","OSC 3 Octave",{"-2 oct","-1 oct","0 oct","+1 oct","+2 oct"},3);
 f("CUTOFF","Filter Cutoff",25,18000,8000);f("RESONANCE","Resonance",0,1,.1f);
 f("ATTACK","Amp Attack",.001f,5,.01f);f("DECAY","Amp Decay",.001f,5,.18f);f("SUSTAIN","Amp Sustain",0,1,.8f);f("RELEASE","Amp Release",.001f,8,.25f);
 f("FATTACK","Filter Attack",.001f,5,.01f);f("FDECAY","Filter Decay",.001f,5,.2f);f("FSUSTAIN","Filter Sustain",0,1,.6f);f("FRELEASE","Filter Release",.001f,8,.3f);
 f("FM","FM Amount",0,1,0);f("RING","Ring Mod",0,1,0);f("DRIVE","Distortion Drive",.1f,5,1);c("MODSOURCE","Mod Source",{"ENV 2","LFO 1"},0);c("MODTARGET","Mod Target",{"Cutoff","OSC 1 Mix","OSC 2 Mix"},0);f("MODDEPTH","Mod Depth",0,1,.35f);
 f("LFO1RATE","LFO 1 Rate",.05f,20,2);f("LFO1AMT","LFO 1 Amount",0,1,0);f("ENV2CUT","ENV 2 > Cutoff",0,1,0);f("LFO1CUT","LFO 1 > Cutoff",0,1,0);
 c("MONO","Voice Mode",{"Poly","Mono","Legato"},0);f("PORTAMENTO","Portamento",0,1,0);
 c("FXCOMP","FX Compressor",{"Off","On"},0);c("FXDIST","FX Distortion",{"Off","On"},0);c("FXEQ","FX EQ",{"Off","On"},0);c("FXDELAY","FX Delay",{"Off","On"},0);c("FXREVERB","FX Reverb",{"Off","On"},0);c("FXFLANGER","FX Flanger",{"Off","On"},0);c("FXPHASER","FX Phaser",{"Off","On"},0);c("FXVOCODER","FX Vocoder",{"Off","On"},0);
 f("FXCOMPTHRESH","Comp Threshold",-60,0,-18);f("FXDISTAMT","FX Distortion",0,1,.25f);f("FXEQLOW","EQ Low",-12,12,0);f("FXEQHIGH","EQ High",-12,12,0);f("FXDELAYTIME","Delay Time",1,1500,360);f("FXDELAYFB","Delay Feedback",0,.95f,.3f);f("FXDELAYMIX","Delay Mix",0,1,.25f);f("FXREVERBMIX","Reverb Mix",0,1,.2f);f("FXFLANGERMIX","Flanger Mix",0,1,.2f);f("FXPHASERMIX","Phaser Mix",0,1,.2f);f("FXVOCODERMIX","Vocoder Mix",0,1,.5f);
 f("ANALOG","Analog Character",0,1,.25f);f("DIGITALEDGE","Digital Edge",0,1,0);f("MASTER","Master",0,1,.75f);return p;
}
void WaveBombzWASPProcessor::prepareToPlay(double sr,int){
 synth.setCurrentPlaybackSampleRate(sr);for(int i=0;i<synth.getNumVoices();++i)if(auto* v=dynamic_cast<WaspVoice*>(synth.getVoice(i)))v->prepare(sr);
 meterFilters.clear();meterFilters.resize((size_t)getTotalNumOutputChannels());
 eqLow.clear();eqHigh.clear();eqLow.resize((size_t)getTotalNumOutputChannels());eqHigh.resize((size_t)getTotalNumOutputChannels());
 juce::dsp::ProcessSpec spec{sr,512,(juce::uint32)getTotalNumOutputChannels()};
 compressor.prepare(spec);compressor.setThreshold(-18.0f);compressor.setRatio(3.0f);compressor.setAttack(10.0f);compressor.setRelease(100.0f);
 reverb.prepare(spec);flanger.prepare(spec);chorus.prepare(spec);phaser.prepare(spec);delay.prepare(spec);delay.setMaximumDelayInSamples((int)sr*2);delay.setDelay((float)(sr*.36));
 flanger.setCentreDelay(2.0f);flanger.setDepth(.35f);flanger.setRate(.25f);flanger.setFeedback(.1f);flanger.setMix(.5f);
 chorus.setCentreDelay(10.0f);chorus.setDepth(.2f);chorus.setRate(.8f);chorus.setMix(.5f);
 phaser.setRate(.25f);phaser.setDepth(.5f);phaser.setCentreFrequency(1000.0f);phaser.setFeedback(.2f);phaser.setMix(.5f);
 for(size_t i=0;i<meterFilters.size();++i){auto& f=meterFilters[i];f.first.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighPass(sr,60.0f);f.second.coefficients=juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr,1500.0f,.7f,juce::Decibels::decibelsToGain(4.0f));eqLow[i].coefficients=juce::dsp::IIR::Coefficients<float>::makeLowShelf(sr,180.0f,.7f,juce::Decibels::decibelsToGain(1.0f));eqHigh[i].coefficients=juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr,5000.0f,.7f,juce::Decibels::decibelsToGain(1.0f));}
}
void WaveBombzWASPProcessor::releaseResources(){}
bool WaveBombzWASPProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return l.getMainOutputChannelSet()==juce::AudioChannelSet::mono()||l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
void WaveBombzWASPProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals no;
 const int mode=(int)apvts.getRawParameterValue("MONO")->load();
 if(mode>0){
  juce::MidiBuffer mono;int newNote=-1;
  for(const auto meta:midi){auto m=meta.getMessage();
   if(m.isNoteOn()){if(mode==1&&monoNote>=0&&monoNote!=m.getNoteNumber())mono.addEvent(juce::MidiMessage::noteOff(m.getChannel(),monoNote),meta.samplePosition);mono.addEvent(m,meta.samplePosition);newNote=m.getNoteNumber();}
   else if(m.isNoteOff()){if(m.getNoteNumber()==monoNote||m.getNoteNumber()==newNote){mono.addEvent(m,meta.samplePosition);if(m.getNoteNumber()==monoNote)monoNote=-1;}}
   else mono.addEvent(m,meta.samplePosition);
  }
  if(newNote>=0&&newNote!=monoNote){synth.allNotesOff(1,false);monoNote=newNote;}
  midi.swapWith(mono);
 }
 b.clear();synth.renderNextBlock(b,midi,0,b.getNumSamples());
 auto raw=[this](const char* id){return apvts.getRawParameterValue(id)->load();};
 if((int)raw("FXCOMP")){compressor.setThreshold(raw("FXCOMPTHRESH"));juce::dsp::AudioBlock<float> block(b);juce::dsp::ProcessContextReplacing<float> ctx(block);compressor.process(ctx);}
 if((int)raw("FXDIST")){float amount=1.0f+raw("FXDISTAMT")*15.0f;for(int ch=0;ch<b.getNumChannels();++ch)for(int i=0;i<b.getNumSamples();++i)b.setSample(ch,i,std::tanh(b.getSample(ch,i)*amount));}
 if((int)raw("FXEQ")){for(int ch=0;ch<b.getNumChannels();++ch){eqLow[(size_t)ch].coefficients=juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(),180.0f,.7f,juce::Decibels::decibelsToGain(raw("FXEQLOW")));eqHigh[(size_t)ch].coefficients=juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(),5000.0f,.7f,juce::Decibels::decibelsToGain(raw("FXEQHIGH")));for(int i=0;i<b.getNumSamples();++i){float x=b.getSample(ch,i);x=eqLow[(size_t)ch].processSample(x);x=eqHigh[(size_t)ch].processSample(x);b.setSample(ch,i,x);}}}
 if((int)raw("FXDELAY")){delay.setDelay((float)getSampleRate()*raw("FXDELAYTIME")/1000.0f);for(int ch=0;ch<b.getNumChannels();++ch)for(int i=0;i<b.getNumSamples();++i){float dry=b.getSample(ch,i),wet=delay.popSample(ch);delay.pushSample(ch,dry+wet*raw("FXDELAYFB"));b.setSample(ch,i,dry*(1-raw("FXDELAYMIX"))+wet*raw("FXDELAYMIX"));}}
 if((int)raw("FXREVERB")){juce::dsp::Reverb::Parameters rp;rp.roomSize=.65f;rp.damping=.4f;rp.wetLevel=raw("FXREVERBMIX");rp.dryLevel=1.0f-rp.wetLevel;reverb.setParameters(rp);juce::dsp::AudioBlock<float> block(b);juce::dsp::ProcessContextReplacing<float> ctx(block);reverb.process(ctx);}
 if((int)raw("FXFLANGER")){flanger.setMix(raw("FXFLANGERMIX"));juce::dsp::AudioBlock<float> block(b);juce::dsp::ProcessContextReplacing<float> ctx(block);flanger.process(ctx);}
 if((int)raw("FXPHASER")){phaser.setMix(raw("FXPHASERMIX"));juce::dsp::AudioBlock<float> block(b);juce::dsp::ProcessContextReplacing<float> ctx(block);phaser.process(ctx);}
 if((int)raw("FXVOCODER")){float mix=raw("FXVOCODERMIX");for(int ch=0;ch<b.getNumChannels();++ch)for(int i=0;i<b.getNumSamples();++i){float x=b.getSample(ch,i);float mod=.5f+.5f*std::sin((float)i*.035f);b.setSample(ch,i,x*(1-mix)+x*mod*mix);}}
 b.applyGain(raw("MASTER"));
 double sum=0;int count=0;
 for(int ch=0;ch<b.getNumChannels();++ch)for(int i=0;i<b.getNumSamples();++i){float x=b.getSample(ch,i);if((size_t)ch<meterFilters.size()){x=meterFilters[(size_t)ch].first.processSample(x);x=meterFilters[(size_t)ch].second.processSample(x);}sum+=(double)x*x;++count;float pk=std::abs(b.getSample(ch,i)),old=peak.load();while(pk>old&&!peak.compare_exchange_weak(old,pk)){}}
 if(count){float rms=(float)std::sqrt(sum/count);float l=juce::Decibels::gainToDecibels(rms,-100.0f)-.691f;momentary.store(l);if(l>-70){integratedEnergy+=sum;integratedSamples+=(uint64_t)count;integrated.store(juce::Decibels::gainToDecibels((float)std::sqrt(integratedEnergy/(double)integratedSamples),-100.0f)-.691f);}}
}

bool WaveBombzWASPProcessor::loadOscillatorSample(int oscillator,const juce::File& file){
 if(oscillator<0||oscillator>=3||!file.existsAsFile())return false;
 std::unique_ptr<juce::AudioFormatReader> reader(sampleFormats.createReaderFor(file));
 if(!reader||reader->lengthInSamples<1||reader->lengthInSamples>static_cast<juce::int64>(reader->sampleRate*600.0))return false;
 auto buffer=std::make_shared<juce::AudioBuffer<float>>(1,(int)reader->lengthInSamples);
 if(!reader->read(buffer.get(),0,(int)reader->lengthInSamples,0,true,false))return false;
 std::shared_ptr<const juce::AudioBuffer<float>> immutable=buffer;
 std::atomic_store(&oscillatorSamples[(size_t)oscillator],immutable);
 oscillatorSampleRates[(size_t)oscillator].store(reader->sampleRate);
 apvts.state.setProperty("OSC_SAMPLE_PATH_"+juce::String(oscillator+1),file.getFullPathName(),nullptr);
 return true;
}
juce::String WaveBombzWASPProcessor::getOscillatorSampleName(int oscillator)const{
 if(oscillator<0||oscillator>=3)return {};
 auto path=apvts.state.getProperty("OSC_SAMPLE_PATH_"+juce::String(oscillator+1)).toString();
 return path.isEmpty()?juce::String("No sample loaded"):juce::File(path).getFileName();
}
void WaveBombzWASPProcessor::restoreOscillatorSamples(){
 for(int i=0;i<3;++i){auto path=apvts.state.getProperty("OSC_SAMPLE_PATH_"+juce::String(i+1)).toString();if(path.isNotEmpty())loadOscillatorSample(i,juce::File(path));else std::atomic_store(&oscillatorSamples[(size_t)i],std::shared_ptr<const juce::AudioBuffer<float>>{});}
}
juce::AudioProcessorEditor* WaveBombzWASPProcessor::createEditor(){return new WaveBombzWASPEditor(*this);}bool WaveBombzWASPProcessor::hasEditor()const{return true;}
const juce::String WaveBombzWASPProcessor::getName()const{return JucePlugin_Name;}bool WaveBombzWASPProcessor::acceptsMidi()const{return true;}bool WaveBombzWASPProcessor::producesMidi()const{return false;}bool WaveBombzWASPProcessor::isMidiEffect()const{return false;}double WaveBombzWASPProcessor::getTailLengthSeconds()const{return 8;}
int WaveBombzWASPProcessor::getNumPrograms(){return 1;}int WaveBombzWASPProcessor::getCurrentProgram(){return 0;}void WaveBombzWASPProcessor::setCurrentProgram(int){}const juce::String WaveBombzWASPProcessor::getProgramName(int){return {};}void WaveBombzWASPProcessor::changeProgramName(int,const juce::String&){}
void WaveBombzWASPProcessor::loadFactoryPreset(int index){
 struct Setting{const char* id;float value;};
 static const std::vector<std::vector<Setting>> presets={
  {{"OSC1WAVE",1},{"OSC2WAVE",0},{"OSC3WAVE",2},{"OSC1MIX",.8f},{"OSC2MIX",.35f},{"OSC3MIX",.15f},{"CUTOFF",1450},{"RESONANCE",.72f},{"ATTACK",.005f},{"DECAY",.24f},{"SUSTAIN",.1f},{"RELEASE",.18f},{"FM",.15f},{"DRIVE",2.2f}},
  {{"OSC1WAVE",1},{"OSC2WAVE",2},{"OSC3WAVE",0},{"OSC1MIX",.7f},{"OSC2MIX",.4f},{"OSC3MIX",.2f},{"CUTOFF",7200},{"RESONANCE",.15f},{"ATTACK",.01f},{"DECAY",.6f},{"SUSTAIN",.75f},{"RELEASE",.45f},{"LFO1RATE",.35f},{"LFO1AMT",.2f}},
  {{"OSC1WAVE",2},{"OSC2WAVE",1},{"OSC3WAVE",0},{"OSC1MIX",.6f},{"OSC2MIX",.45f},{"OSC3MIX",.15f},{"CUTOFF",2200},{"RESONANCE",.25f},{"ATTACK",.01f},{"DECAY",.45f},{"SUSTAIN",.25f},{"RELEASE",.3f},{"DRIVE",1.4f}},
  {{"OSC1WAVE",0},{"OSC2WAVE",0},{"OSC3WAVE",0},{"OSC1MIX",.35f},{"OSC2MIX",.5f},{"OSC3MIX",.1f},{"CUTOFF",240},{"RESONANCE",.15f},{"ATTACK",.02f},{"DECAY",.3f},{"SUSTAIN",.85f},{"RELEASE",.25f}},
  {{"OSC1WAVE",1},{"OSC2WAVE",1},{"OSC3WAVE",2},{"OSC1MIX",.7f},{"OSC2MIX",.55f},{"OSC3MIX",.35f},{"OSC2TUNE",7},{"OSC3TUNE",12},{"CUTOFF",4200},{"RESONANCE",.35f},{"ATTACK",.03f},{"DECAY",.7f},{"SUSTAIN",.6f},{"RELEASE",.5f}},
  {{"OSC1WAVE",2},{"OSC2WAVE",0},{"OSC3WAVE",1},{"OSC1MIX",.8f},{"OSC2MIX",.25f},{"OSC3MIX",.15f},{"CUTOFF",1900},{"RESONANCE",.55f},{"ATTACK",.001f},{"DECAY",.16f},{"SUSTAIN",.05f},{"RELEASE",.12f},{"FXCOMP",1},{"FXREVERB",1},{"FXREVERBMIX",.12f}},
  {{"OSC1WAVE",0},{"OSC2WAVE",1},{"OSC3WAVE",2},{"OSC1MIX",.3f},{"OSC2MIX",.35f},{"OSC3MIX",.4f},{"CUTOFF",650},{"RESONANCE",.4f},{"ATTACK",1.2f},{"DECAY",2.0f},{"SUSTAIN",.75f},{"RELEASE",3.0f},{"FXREVERB",1},{"FXREVERBMIX",.45f},{"FXPHASER",1}},
  {{"OSC1WAVE",1},{"OSC2WAVE",2},{"OSC3WAVE",1},{"OSC1MIX",.75f},{"OSC2MIX",.55f},{"OSC3MIX",.1f},{"CUTOFF",1100},{"RESONANCE",.65f},{"ATTACK",.005f},{"DECAY",.3f},{"SUSTAIN",.25f},{"RELEASE",.15f},{"DRIVE",2.8f},{"FXDIST",1}}
 };
 if(index<0||index>=(int)presets.size())return;
 auto state=apvts.copyState();
 for(const auto& s:presets[(size_t)index])if(auto* param=apvts.getParameter(s.id))param->setValueNotifyingHost(param->convertTo0to1(s.value));
}
bool WaveBombzWASPProcessor::savePreset(const juce::File& file){auto state=apvts.copyState();std::unique_ptr<juce::XmlElement> xml(state.createXml());return xml!=nullptr&&xml->writeTo(file);}
bool WaveBombzWASPProcessor::loadPreset(const juce::File& file){
 auto xml=juce::XmlDocument::parse(file);if(!xml)return false;
 if(xml->hasTagName(apvts.state.getType())){apvts.replaceState(juce::ValueTree::fromXml(*xml));restoreOscillatorSamples();return true;}
 if(!xml->hasTagName("WASPPRESET"))return false;
 for(auto* p=xml->getFirstChildElement();p;p=p->getNextElement())if(p->hasTagName("PARAM")){auto id=p->getStringAttribute("id");auto* param=apvts.getParameter(id);if(param)param->setValueNotifyingHost(param->convertTo0to1((float)p->getDoubleAttribute("value")));}
 return true;
}
void WaveBombzWASPProcessor::getStateInformation(juce::MemoryBlock& d){auto s=apvts.copyState();std::unique_ptr<juce::XmlElement>x(s.createXml());copyXmlToBinary(*x,d);}
void WaveBombzWASPProcessor::setStateInformation(const void* d,int n){std::unique_ptr<juce::XmlElement>x(getXmlFromBinary(d,n));if(x&&x->hasTagName(apvts.state.getType())){apvts.replaceState(juce::ValueTree::fromXml(*x));restoreOscillatorSamples();}}
juce::AudioProcessorValueTreeState& WaveBombzWASPProcessor::getState(){return apvts;}float WaveBombzWASPProcessor::getMomentaryLUFS()const{return momentary.load();}float WaveBombzWASPProcessor::getIntegratedLUFS()const{return integrated.load();}float WaveBombzWASPProcessor::getPeak(){return peak.exchange(0.0f);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new WaveBombzWASPProcessor();}
