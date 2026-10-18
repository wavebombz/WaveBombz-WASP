#include "PluginEditor.h"
namespace {const juce::Colour black(13,15,17),panel(25,28,31),yellow(255,205,0),dim(150,153,155);}
WaspLookAndFeel::WaspLookAndFeel(){setColour(juce::Slider::textBoxTextColourId,yellow);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);setColour(juce::Label::textColourId,yellow);}
void WaspLookAndFeel::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&){
 float size=(float)juce::jmax(1,juce::jmin(w,h)-14),cx=x+w*.5f,cy=y+h*.5f;auto r=juce::Rectangle<float>(cx-size*.5f,cy-size*.5f,size,size);
 g.setColour(juce::Colours::black);g.fillEllipse(r.translated(2,4));g.setGradientFill(juce::ColourGradient(juce::Colour(76,79,82),r.getX(),r.getY(),juce::Colour(18,20,22),r.getRight(),r.getBottom(),false));g.fillEllipse(r);
 g.setColour(juce::Colour(95,99,102));g.drawEllipse(r,1.5f);float angle=start+pos*(end-start);juce::Path arc;arc.addCentredArc(cx,cy,size*.43f,size*.43f,0,start,angle,true);g.setColour(yellow);g.strokePath(arc,juce::PathStrokeType(3.5f));
 juce::Path pointer;pointer.addRoundedRectangle(-1.7f,-size*.34f,3.4f,size*.2f,1.5f);g.fillPath(pointer,juce::AffineTransform::rotation(angle).translated(cx,cy));g.setColour(juce::Colours::white.withAlpha(.08f));g.drawEllipse(r.reduced(5),1.0f);
}
void WaspLookAndFeel::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool highlighted,bool down){
 auto r=b.getLocalBounds().toFloat().reduced(1.0f);auto base=b.getToggleState()?juce::Colour(255,190,0):juce::Colour(31,34,37);if(down)base=base.brighter(.18f);else if(highlighted)base=base.brighter(.10f);
 g.setColour(juce::Colours::black.withAlpha(.8f));g.fillRoundedRectangle(r.translated(1.5f,2.5f),5.0f);g.setGradientFill(juce::ColourGradient(base.brighter(.16f),r.getX(),r.getY(),base.darker(.35f),r.getX(),r.getBottom(),false));g.fillRoundedRectangle(r,5.0f);g.setColour(juce::Colour(255,205,0).withAlpha(b.getToggleState()?.95f:.55f));g.drawRoundedRectangle(r,5.0f,1.0f);
}
void WaspLookAndFeel::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool){g.setColour(b.getToggleState()?juce::Colour(18,19,20):juce::Colour(238,239,240));g.setFont(juce::Font(juce::FontOptions(11.0f,juce::Font::bold)));g.drawFittedText(b.getButtonText(),b.getLocalBounds().reduced(4),juce::Justification::centred,1);}
void WaspLookAndFeel::drawComboBox(juce::Graphics& g,int w,int h,bool,int,int,int,int,juce::ComboBox& box){auto r=juce::Rectangle<float>(0,0,(float)w,(float)h).reduced(1);g.setColour(juce::Colours::black.withAlpha(.7f));g.fillRoundedRectangle(r.translated(1,2),4);g.setGradientFill(juce::ColourGradient(juce::Colour(45,48,51),0,0,juce::Colour(15,17,19),0,(float)h,false));g.fillRoundedRectangle(r,4);g.setColour(juce::Colour(255,205,0).withAlpha(box.hasKeyboardFocus(true)?.95f:.48f));g.drawRoundedRectangle(r,4,1);juce::Path p;p.addTriangle((float)w-16,(float)h*.42f,(float)w-8,(float)h*.42f,(float)w-12,(float)h*.60f);g.setColour(juce::Colour(255,205,0));g.fillPath(p);}
void WaspLookAndFeel::drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float,float,juce::Slider& s){auto r=juce::Rectangle<float>((float)x,(float)y+(float)h*.40f,(float)w,juce::jmax(3.0f,(float)h*.20f));g.setColour(juce::Colour(7,8,9));g.fillRoundedRectangle(r,3);auto fill=r.withWidth(juce::jlimit(0.0f,r.getWidth(),pos-(float)x));g.setGradientFill(juce::ColourGradient(juce::Colour(255,220,70),fill.getX(),fill.getY(),juce::Colour(226,143,0),fill.getX(),fill.getBottom(),false));g.fillRoundedRectangle(fill,3);g.setColour(juce::Colour(255,205,0).withAlpha(.7f));g.drawRoundedRectangle(r,3,1);g.setColour(juce::Colour(255,245,190));g.fillRoundedRectangle(juce::Rectangle<float>(pos-4,(float)y+2,8,(float)h-4),2);}
WaveBombzWASPEditor::WaveBombzWASPEditor(WaveBombzWASPProcessor& p):AudioProcessorEditor(&p),processor(p){
 setOpaque(true);setWantsKeyboardFocus(false);setLookAndFeel(&look);
 synthPageButton.onClick=[this]{setFxPage(false);}; fxPageButton.onClick=[this]{setFxPage(true);};
 for(auto* b:{&synthPageButton,&fxPageButton}){b->setColour(juce::TextButton::buttonColourId,juce::Colour(42,45,48));b->setColour(juce::TextButton::buttonOnColourId,yellow);b->setColour(juce::TextButton::textColourOffId,yellow);b->setColour(juce::TextButton::textColourOnId,black);addAndMakeVisible(*b);}
 title.setText("WaveBombz WASP",juce::dontSendNotification);title.setFont(juce::Font(juce::FontOptions(30.0f,juce::Font::bold)));title.setColour(juce::Label::textColourId,yellow);addAndMakeVisible(title);
 factoryPreset.addItem("Acid Bite",1);factoryPreset.addItem("Neon Pulse",2);factoryPreset.addItem("Brass Stab",3);factoryPreset.addItem("Deep Sub",4);factoryPreset.addItem("Wasp Lead",5);factoryPreset.addItem("Tech Pluck",6);factoryPreset.addItem("Dark Drone",7);factoryPreset.addItem("Retro Bass",8);factoryPreset.setSelectedId(1);factoryPreset.onChange=[this]{int id=factoryPreset.getSelectedId();if(id>=1&&id<=8)processor.loadFactoryPreset(id-1);else if(id>=9&&(size_t)(id-9)<bankFiles.size())processor.loadPreset(bankFiles[(size_t)(id-9)]);};addAndMakeVisible(factoryPreset);
 savePresetButton.onClick=[this]{fileChooser=std::make_unique<juce::FileChooser>("Save WaveBombz WASP preset",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("WaveBombz-Preset.wasp"),"*.wasp");fileChooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles,[this](const juce::FileChooser& fc){auto f=fc.getResult();if(f!=juce::File{}){if(f.getFileExtension().isEmpty())f=f.withFileExtension(".wasp");processor.savePreset(f);}});};addAndMakeVisible(savePresetButton);
 loadPresetButton.onClick=[this]{fileChooser=std::make_unique<juce::FileChooser>("Load WaveBombz WASP preset",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.wasp");fileChooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[this](const juce::FileChooser& fc){auto f=fc.getResult();if(f.existsAsFile())processor.loadPreset(f);});};addAndMakeVisible(loadPresetButton);
 openBankButton.onClick=[this]{fileChooser=std::make_unique<juce::FileChooser>("Load WaveBombz WASP soundbank folder",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory));fileChooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectDirectories,[this](const juce::FileChooser& fc){auto dir=fc.getResult();if(!dir.isDirectory())return;bankFiles.clear();factoryPreset.clear(juce::dontSendNotification);const char* names[]={"Acid Bite","Neon Pulse","Brass Stab","Deep Sub","Wasp Lead","Tech Pluck","Dark Drone","Retro Bass"};for(int i=0;i<8;++i)factoryPreset.addItem(names[i],i+1);auto files=dir.findChildFiles(juce::File::findFiles,false,"*.wasp");for(auto& f:files){bankFiles.push_back(f);factoryPreset.addItem(f.getFileNameWithoutExtension(),9+(int)bankFiles.size()-1);}if(!bankFiles.empty())factoryPreset.setSelectedId(9);});};addAndMakeVisible(openBankButton);
 voiceMode.addItem("POLY",1);voiceMode.addItem("MONO",2);voiceMode.addItem("LEGATO",3);addAndMakeVisible(voiceMode);voiceAttachment=std::make_unique<ComboAttachment>(processor.getState(),"MONO",voiceMode);
 portamento.setSliderStyle(juce::Slider::LinearHorizontal);portamento.setTextBoxStyle(juce::Slider::TextBoxRight,false,48,18);portamento.setTooltip("Portamento time");addAndMakeVisible(portamento);portaAttachment=std::make_unique<SliderAttachment>(processor.getState(),"PORTAMENTO",portamento);
 subtitle.setText("TRI-OSC PERFORMANCE SYNTH  /  MODULATION MATRIX",juce::dontSendNotification);subtitle.setFont(juce::Font(juce::FontOptions(12.0f)));subtitle.setColour(juce::Label::textColourId,dim);addAndMakeVisible(subtitle);
 oscLabel.setText("OSCILLATORS",juce::dontSendNotification);filterLabel.setText("FILTER / DRIVE",juce::dontSendNotification);envLabel.setText("AMP ENVELOPE",juce::dontSendNotification);lfoLabel.setText("LFO / MOD MATRIX",juce::dontSendNotification);
 for(auto* l:{&oscLabel,&filterLabel,&envLabel,&lfoLabel}){l->setFont(juce::Font(juce::FontOptions(14,juce::Font::bold)));l->setColour(juce::Label::textColourId,yellow);addAndMakeVisible(*l);}
 addKnob("OSC1MIX","OSC 1 MIX");addKnob("OSC2MIX","OSC 2 MIX");addKnob("OSC3MIX","OSC 3 MIX");
 const char* waveIds[]={"OSC1WAVE","OSC2WAVE","OSC3WAVE"};
 const char* tuneIds[]={"OSC1TUNE","OSC2TUNE","OSC3TUNE"};
 const char* octaveIds[]={"OSC1OCT","OSC2OCT","OSC3OCT"};
 for(size_t i=0;i<oscillatorWaveSelectors.size();++i){
  const auto oscName="OSC "+juce::String((int)i+1);
  auto& waveSelector=oscillatorWaveSelectors[i];sampleButtons[i].setButtonText("LOAD SAMPLE");waveSelector.addItem("SINE",1);waveSelector.addItem("SAW",2);waveSelector.addItem("SQUARE",3);waveSelector.addItem("PULSE",4);waveSelector.setTooltip(oscName+" waveform: choose the basic oscillator shape. A loaded sample takes priority until cleared.");addAndMakeVisible(waveSelector);
  oscillatorWaveAttachments[i]=std::make_unique<ComboAttachment>(processor.getState(),waveIds[i],waveSelector);
  auto& tune=oscillatorTuneSliders[i];tune.setSliderStyle(juce::Slider::LinearHorizontal);tune.setRange(-24.0,24.0,0.1);tune.setTextBoxStyle(juce::Slider::TextBoxRight,false,42,16);tune.setTooltip(oscName+" fine tune: adjusts pitch by semitones from -24 to +24; small changes detune the oscillator.");addAndMakeVisible(tune);
  oscillatorTuneAttachments[i]=std::make_unique<SliderAttachment>(processor.getState(),tuneIds[i],tune);
  oscillatorTuneLabels[i].setText("TUNE (st)",juce::dontSendNotification);oscillatorTuneLabels[i].setColour(juce::Label::textColourId,dim);oscillatorTuneLabels[i].setFont(juce::Font(juce::FontOptions(9.0f,juce::Font::bold)));oscillatorTuneLabels[i].setTooltip("Fine pitch adjustment in semitones.");addAndMakeVisible(oscillatorTuneLabels[i]);
  auto& octave=oscillatorOctaveSelectors[i];octave.addItem("-2 OCT",1);octave.addItem("-1 OCT",2);octave.addItem("0 OCT",3);octave.addItem("+1 OCT",4);octave.addItem("+2 OCT",5);octave.setTooltip(oscName+" octave: transposes this oscillator by two octaves down/up in 12-semitone steps.");addAndMakeVisible(octave);
  oscillatorOctaveAttachments[i]=std::make_unique<ComboAttachment>(processor.getState(),octaveIds[i],octave);
  oscillatorOctaveLabels[i].setText("OCTAVE",juce::dontSendNotification);oscillatorOctaveLabels[i].setColour(juce::Label::textColourId,dim);oscillatorOctaveLabels[i].setFont(juce::Font(juce::FontOptions(9.0f,juce::Font::bold)));oscillatorOctaveLabels[i].setTooltip("Octave transpose: -2, -1, 0, +1 or +2 octaves.");addAndMakeVisible(oscillatorOctaveLabels[i]);
  sampleButtons[i].setTooltip("Load an audio sample for this oscillator. Supported: WAV, AIFF, FLAC, OGG and MP3. Sample playback follows this oscillator's tune and octave settings.");
  sampleButtons[i].onClick=[this,i]{fileChooser=std::make_unique<juce::FileChooser>("Load sample for OSC "+juce::String((int)i+1),juce::File::getSpecialLocation(juce::File::userMusicDirectory),"*.wav;*.aif;*.aiff;*.flac;*.ogg;*.mp3");fileChooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[this,i](const juce::FileChooser& fc){auto file=fc.getResult();if(file.existsAsFile()&&processor.loadOscillatorSample((int)i,file))sampleButtons[i].setButtonText(file.getFileNameWithoutExtension());});};
  addAndMakeVisible(sampleButtons[i]);
 }addKnob("CUTOFF","CUTOFF");addKnob("RESONANCE","RESONANCE");addKnob("DRIVE","DRIVE");
 addKnob("ATTACK","ATTACK");addKnob("DECAY","DECAY");addKnob("SUSTAIN","SUSTAIN");addKnob("RELEASE","RELEASE");addKnob("LFO1RATE","LFO RATE");addKnob("LFO1AMT","LFO AMOUNT");addKnob("ENV2CUT","ENV 2 → CUTOFF");addKnob("LFO1CUT","LFO 1 → CUTOFF");addKnob("MASTER","MASTER");addKnob("ANALOG","ANALOG CHARACTER");addKnob("DIGITALEDGE","DIGITAL EDGE");
 modTitle.setText("MODULATION ROUTING",juce::dontSendNotification);modTitle.setColour(juce::Label::textColourId,yellow);modTitle.setFont(juce::Font(juce::FontOptions(13,juce::Font::bold)));addAndMakeVisible(modTitle);
 modSource.addItem("ENV 2",1);modSource.addItem("LFO 1",2);addAndMakeVisible(modSource);sourceAttachment=std::make_unique<ComboAttachment>(processor.getState(),"MODSOURCE",modSource);
 modTarget.addItem("Cutoff",1);modTarget.addItem("OSC 1 Mix",2);modTarget.addItem("OSC 2 Mix",3);addAndMakeVisible(modTarget);targetAttachment=std::make_unique<ComboAttachment>(processor.getState(),"MODTARGET",modTarget);
 modDepth.setSliderStyle(juce::Slider::LinearHorizontal);modDepth.setTextBoxStyle(juce::Slider::TextBoxRight,false,60,20);addAndMakeVisible(modDepth);depthAttachment=std::make_unique<SliderAttachment>(processor.getState(),"MODDEPTH",modDepth);
 modDepthLabel.setText("DEPTH",juce::dontSendNotification);modDepthLabel.setColour(juce::Label::textColourId,dim);addAndMakeVisible(modDepthLabel);
 const char* fxIds[]={"FXCOMP","FXDIST","FXEQ","FXDELAY","FXREVERB","FXFLANGER","FXPHASER","FXVOCODER"};
 const char* fxNames[]={"COMP","DISTORTION","EQ","DELAY","REVERB","FLANGER","PHASER","VOCODER"};
 for(size_t i=0;i<fxSwitches.size();++i){fxSwitches[i]=std::make_unique<juce::ComboBox>();fxSwitches[i]->addItem("OFF",1);fxSwitches[i]->addItem("ON",2);addAndMakeVisible(*fxSwitches[i]);fxSwitchLabels[i].setText(fxNames[i],juce::dontSendNotification);fxSwitchLabels[i].setColour(juce::Label::textColourId,yellow);fxSwitchLabels[i].setFont(juce::Font(juce::FontOptions(10,juce::Font::bold)));addAndMakeVisible(fxSwitchLabels[i]);fxAttachments[i]=std::make_unique<ComboAttachment>(processor.getState(),fxIds[i],*fxSwitches[i]);}
 const char* fxParamIds[]={"FXCOMPTHRESH","FXDISTAMT","FXEQLOW","FXEQHIGH","FXDELAYTIME","FXDELAYMIX","FXREVERBMIX","FXVOCODERMIX"};
 const char* fxParamNames[]={"COMP THRESHOLD","DIST AMOUNT","LOW SHELF","HIGH SHELF","DELAY TIME","DELAY MIX","REVERB MIX","VOCODER MIX"};
 const float fxMin[]={-60,0,-12,-12,1,0,0,0},fxMax[]={0,1,12,12,1500,1,1,1};
 for(size_t i=0;i<fxControls.size();++i){fxControls[i]=std::make_unique<juce::Slider>();fxControls[i]->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);fxControls[i]->setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,16);fxControls[i]->setName(fxParamNames[i]);fxControls[i]->setRange(fxMin[i],fxMax[i],i==4?1:.1);addAndMakeVisible(*fxControls[i]);fxControlLabels[i].setText(fxParamNames[i],juce::dontSendNotification);fxControlLabels[i].setColour(juce::Label::textColourId,yellow);fxControlLabels[i].setFont(juce::Font(juce::FontOptions(10,juce::Font::bold)));fxControlLabels[i].setJustificationType(juce::Justification::centred);addAndMakeVisible(fxControlLabels[i]);fxControlAttachments[i]=std::make_unique<SliderAttachment>(processor.getState(),fxParamIds[i],*fxControls[i]);}
 meterText.setText("MOMENTARY LOUDNESS  -100 LUFS     INTEGRATED LOUDNESS  -100 LUFS     PEAK  -inf dBFS",juce::dontSendNotification);meterText.setColour(juce::Label::textColourId,yellow);meterText.setFont(juce::Font(juce::FontOptions(13,juce::Font::bold)));addAndMakeVisible(meterText);setFxPage(false);setSize(1080,900);startTimerHz(12);
}
WaveBombzWASPEditor::~WaveBombzWASPEditor(){stopTimer();setLookAndFeel(nullptr);}
void WaveBombzWASPEditor::setFxPage(bool showFx){
 showFxPage=showFx;
 synthPageButton.setToggleState(!showFx,juce::dontSendNotification);fxPageButton.setToggleState(showFx,juce::dontSendNotification);
 for(auto& k:knobs)k->setVisible(!showFx);for(auto& s:oscillatorWaveSelectors)s.setVisible(!showFx);for(auto& b:sampleButtons)b.setVisible(!showFx);for(auto& s:oscillatorTuneSliders)s.setVisible(!showFx);for(auto& s:oscillatorOctaveSelectors)s.setVisible(!showFx);for(auto& l:oscillatorTuneLabels)l.setVisible(!showFx);for(auto& l:oscillatorOctaveLabels)l.setVisible(!showFx);
 for(auto* l:{&oscLabel,&filterLabel,&envLabel,&lfoLabel,&modTitle,&modDepthLabel})l->setVisible(!showFx);
 modSource.setVisible(!showFx);modTarget.setVisible(!showFx);modDepth.setVisible(!showFx);
 for(auto& s:fxSwitches)s->setVisible(showFx);for(auto& l:fxSwitchLabels)l.setVisible(showFx);
 for(auto& s:fxControls)s->setVisible(showFx);for(auto& l:fxControlLabels)l.setVisible(showFx);
 resized();repaint();
}
void WaveBombzWASPEditor::addKnob(const juce::String& id,const juce::String& label){
 auto s=std::make_unique<juce::Slider>();s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,72,18);s->setName(label);s->setTooltip(label+" — right-click to edit value");addAndMakeVisible(*s);
 attachments.push_back(std::make_unique<SliderAttachment>(processor.getState(),id,*s));knobs.push_back(std::move(s));
}
void WaveBombzWASPEditor::paint(juce::Graphics& g){
 const auto W=(float)getWidth(),H=(float)getHeight();g.fillAll(black);
 g.setGradientFill(juce::ColourGradient(juce::Colour(39,41,43),0,0,juce::Colour(8,9,10),W,H,true));g.fillAll();
 // Brushed-metal header and restrained honeycomb-inspired detail.
 g.setGradientFill(juce::ColourGradient(juce::Colour(34,36,38),0,0,juce::Colour(10,11,12),W,74,false));g.fillRect(0,0,W,76);
 g.setColour(yellow.withAlpha(.9f));g.fillRect(0,75,W,2);g.setColour(yellow.withAlpha(.07f));for(int i=0;i<36;++i){float xx=(float)i*31.0f;g.drawLine(xx,0,xx+22,74,1.0f);}
 g.setColour(juce::Colour(255,205,0).withAlpha(.22f));g.drawRect(1,1,getWidth()-2,getHeight()-2,1);
 auto box=[&](int x,int y,int w,int h,const juce::String& name){auto r=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h);g.setColour(juce::Colours::black.withAlpha(.78f));g.fillRoundedRectangle(r.translated(2,4),8);g.setGradientFill(juce::ColourGradient(juce::Colour(35,38,40),r.getX(),r.getY(),juce::Colour(15,17,18),r.getRight(),r.getBottom(),false));g.fillRoundedRectangle(r,8);g.setColour(yellow.withAlpha(.72f));g.drawRoundedRectangle(r,8,1.1f);g.setColour(yellow.withAlpha(.11f));g.fillRoundedRectangle(r.getX()+1,r.getY()+1,r.getWidth()-2,25,7);g.setColour(yellow);g.fillRect(r.getX()+10,r.getY()+8,3,12);};
 if(showFxPage){
   box(18,92,getWidth()-36,700,"FX");g.setColour(yellow);g.setFont(juce::Font(juce::FontOptions(18.0f,juce::Font::bold)));g.drawText("EFFECTS RACK  /  PROCESSING CHAIN",38,97,getWidth()-76,24,juce::Justification::centredLeft);g.setColour(yellow.withAlpha(.28f));g.fillRect(35,126,getWidth()-70,1);
   const juce::Colour fxColours[]={juce::Colour(255,153,32),juce::Colour(55,155,255),juce::Colour(65,220,170),juce::Colour(190,95,255)};
   for(int i=0;i<4;++i){float xx=34.0f+i*258.0f;g.setColour(fxColours[i].withAlpha(.10f));g.fillRoundedRectangle(xx,202,238,10,4);g.setColour(fxColours[i].withAlpha(.7f));g.fillRect(xx,202,238,2);}
   return;
 }
 box(18,92,305,213,"OSC 1");box(335,92,305,213,"OSC 2");box(18,310,620,180,"FILTER");box(652,92,410,374,"MODULATION");box(18,535,1044,270,"MASTER");
 // Accent rails and subtle technical grid keep the dark interface readable.
 g.setColour(yellow.withAlpha(.7f));g.fillRect(18,92,305,2);g.fillRect(335,92,305,2);g.fillRect(18,310,620,2);g.fillRect(652,92,410,2);g.fillRect(18,535,1044,2);g.fillRect(18,505,1044,1);
 g.setColour(juce::Colour(255,205,0).withAlpha(.07f));for(int i=0;i<29;++i)g.fillRect(22+i*36,670,20,2);
 g.setColour(juce::Colour(255,255,255).withAlpha(.025f));for(int y=90;y<H;y+=5)g.fillRect(0,(float)y,W,1);
}
void WaveBombzWASPEditor::resized(){
 if (knobs.size() < 17) return;
 title.setBounds(22,8,350,40);subtitle.setBounds(24,47,500,20);synthPageButton.setBounds(365,18,76,30);fxPageButton.setBounds(445,18,58,30);factoryPreset.setBounds(515,20,145,28);savePresetButton.setBounds(666,20,100,28);loadPresetButton.setBounds(770,20,100,28);openBankButton.setBounds(874,20,100,28);voiceMode.setBounds(978,20,80,28);portamento.setBounds(885,52,170,20);oscLabel.setBounds(30,98,180,22);filterLabel.setBounds(348,98,220,22);envLabel.setBounds(30,316,220,22);lfoLabel.setBounds(665,98,250,22);
 auto place=[&](int i,int x,int y,int w=90,int h=100){knobs[(size_t)i]->setBounds(x,y,w,h);};
 place(0,35,125,90,82);place(1,125,125,90,82);place(2,215,125,90,82);
 for(size_t i=0;i<3;++i){int x=27+(int)i*98;oscillatorTuneLabels[i].setBounds(x+3,184,84,12);oscillatorTuneSliders[i].setBounds(x,196,91,20);oscillatorOctaveLabels[i].setBounds(x+3,216,84,12);oscillatorOctaveSelectors[i].setBounds(x,228,91,22);oscillatorWaveSelectors[i].setBounds(x,252,91,22);sampleButtons[i].setBounds(x,276,91,23);}place(3,355,125);place(4,450,125);place(5,545,125);
 place(6,45,344,100);place(7,150,344,100);place(8,255,344,100);place(9,360,344,100);
 place(10,670,135);place(11,765,135);place(15,865,135);place(16,960,135);place(12,670,250);place(13,765,250);place(14,865,250);
 modTitle.setBounds(668,365,280,22);modSource.setBounds(668,394,125,28);modTarget.setBounds(802,394,145,28);modDepthLabel.setBounds(668,426,55,22);modDepth.setBounds(720,426,225,24);for(size_t i=0;i<fxSwitches.size();++i){int col=(int)(i%4),row=(int)(i/4);fxSwitchLabels[i].setBounds(35+col*255,541+row*34,150,18);fxSwitches[i]->setBounds(35+col*255,558+row*34,150,24);}
 for(size_t i=0;i<fxControls.size();++i){int col=(int)(i%4),row=(int)(i/4);fxControlLabels[i].setBounds(170+col*220,615+row*85,135,16);fxControls[i]->setBounds(190+col*220,630+row*85,95,80);}
 meterText.setBounds(22,865,1030,28);
 if(showFxPage){
   for(size_t i=0;i<fxSwitches.size();++i){int col=(int)(i%4),row=(int)(i/4);fxSwitchLabels[i].setBounds(42+col*258,125+row*32,170,20);fxSwitches[i]->setBounds(42+col*258,146+row*32,170,25);}
   for(size_t i=0;i<fxControls.size();++i){int col=(int)(i%4),row=(int)(i/4);fxControlLabels[i].setBounds(30+col*260,230+row*230,210,20);fxControls[i]->setBounds(70+col*260,255+row*230,130,125);}
   meterText.setBounds(22,815,1030,28);
 }
}
void WaveBombzWASPEditor::timerCallback(){float pk=processor.getPeak();meterText.setText("MOMENTARY LOUDNESS  "+juce::String(processor.getMomentaryLUFS(),1)+" LUFS     INTEGRATED LOUDNESS  "+juce::String(processor.getIntegratedLUFS(),1)+" LUFS     PEAK  "+(pk>0?juce::String(juce::Decibels::gainToDecibels(pk),1):juce::String("-inf"))+" dBFS",juce::dontSendNotification);}
