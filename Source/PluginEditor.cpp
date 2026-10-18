#include "PluginEditor.h"
namespace {const juce::Colour black(13,15,17),panel(25,28,31),yellow(255,205,0),dim(150,153,155);}
WaspLookAndFeel::WaspLookAndFeel(){setColour(juce::Slider::textBoxTextColourId,yellow);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);setColour(juce::Label::textColourId,yellow);}
void WaspLookAndFeel::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&){
 float size=(float)juce::jmin(w,h)-14,cx=x+w*.5f,cy=y+h*.5f;auto r=juce::Rectangle<float>(cx-size*.5f,cy-size*.5f,size,size);
 g.setColour(juce::Colours::black);g.fillEllipse(r.translated(2,4));g.setGradientFill(juce::ColourGradient(juce::Colour(76,79,82),r.getX(),r.getY(),juce::Colour(18,20,22),r.getRight(),r.getBottom(),false));g.fillEllipse(r);
 g.setColour(juce::Colour(95,99,102));g.drawEllipse(r,1.5f);float angle=start+pos*(end-start);juce::Path arc;arc.addCentredArc(cx,cy,size*.43f,size*.43f,0,start,angle,true);g.setColour(yellow);g.strokePath(arc,juce::PathStrokeType(3.5f));
 juce::Path pointer;pointer.addRoundedRectangle(-1.7f,-size*.34f,3.4f,size*.2f,1.5f);g.fillPath(pointer,juce::AffineTransform::rotation(angle).translated(cx,cy));g.setColour(juce::Colours::white.withAlpha(.08f));g.drawEllipse(r.reduced(5),1.0f);
}
WaveBombzWASPEditor::WaveBombzWASPEditor(WaveBombzWASPProcessor& p):AudioProcessorEditor(&p),processor(p){
 setLookAndFeel(&look);setSize(1000,700);
 title.setText("WaveBombz WASP",juce::dontSendNotification);title.setFont(juce::Font(juce::FontOptions(30.0f,juce::Font::bold)));title.setColour(juce::Label::textColourId,yellow);addAndMakeVisible(title);
 subtitle.setText("TRI-OSC PERFORMANCE SYNTH  /  MODULATION MATRIX",juce::dontSendNotification);subtitle.setFont(juce::Font(12));subtitle.setColour(juce::Label::textColourId,dim);addAndMakeVisible(subtitle);
 oscLabel.setText("OSCILLATORS",juce::dontSendNotification);filterLabel.setText("FILTER / DRIVE",juce::dontSendNotification);envLabel.setText("AMP ENVELOPE",juce::dontSendNotification);lfoLabel.setText("LFO / MOD MATRIX",juce::dontSendNotification);
 for(auto* l:{&oscLabel,&filterLabel,&envLabel,&lfoLabel}){l->setFont(juce::Font(juce::FontOptions(14,juce::Font::bold)));l->setColour(juce::Label::textColourId,yellow);addAndMakeVisible(*l);}
 addKnob("OSC1MIX","OSC 1 MIX");addKnob("OSC2MIX","OSC 2 MIX");addKnob("OSC3MIX","OSC 3 MIX");addKnob("CUTOFF","CUTOFF");addKnob("RESONANCE","RESONANCE");addKnob("DRIVE","DRIVE");
 addKnob("ATTACK","ATTACK");addKnob("DECAY","DECAY");addKnob("SUSTAIN","SUSTAIN");addKnob("RELEASE","RELEASE");addKnob("LFO1RATE","LFO RATE");addKnob("LFO1AMT","LFO AMOUNT");addKnob("ENV2CUT","ENV 2 > CUT");addKnob("LFO1CUT","LFO > CUT");addKnob("MASTER","MASTER");
 modTitle.setText("MODULATION ROUTING",juce::dontSendNotification);modTitle.setColour(juce::Label::textColourId,yellow);modTitle.setFont(juce::Font(juce::FontOptions(13,juce::Font::bold)));addAndMakeVisible(modTitle);
 modSource.addItem("ENV 2",1);modSource.addItem("LFO 1",2);modSource.setSelectedId(1);addAndMakeVisible(modSource);
 modTarget.addItem("Cutoff",1);modTarget.addItem("OSC 1 Mix",2);modTarget.addItem("OSC 2 Mix",3);modTarget.setSelectedId(1);addAndMakeVisible(modTarget);
 modDepth.setSliderStyle(juce::Slider::LinearHorizontal);modDepth.setRange(0,1,.01);modDepth.setValue(.35);modDepth.setTextBoxStyle(juce::Slider::TextBoxRight,false,60,20);addAndMakeVisible(modDepth);
 modDepthLabel.setText("DEPTH",juce::dontSendNotification);modDepthLabel.setColour(juce::Label::textColourId,dim);addAndMakeVisible(modDepthLabel);
 meterText.setText("MOMENTARY  -100 LUFS     INTEGRATED  -100 LUFS     PEAK  -inf dBFS",juce::dontSendNotification);meterText.setColour(juce::Label::textColourId,yellow);meterText.setFont(juce::Font(juce::FontOptions(13,juce::Font::bold)));addAndMakeVisible(meterText);startTimerHz(12);
}
WaveBombzWASPEditor::~WaveBombzWASPEditor(){setLookAndFeel(nullptr);}
void WaveBombzWASPEditor::addKnob(const juce::String& id,const juce::String& label){
 auto s=std::make_unique<juce::Slider>();s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,72,18);s->setName(label);s->setTooltip(label+" — right-click to edit value");addAndMakeVisible(*s);
 attachments.push_back(std::make_unique<SliderAttachment>(processor.getState(),id,*s));knobs.push_back(std::move(s));
}
void WaveBombzWASPEditor::paint(juce::Graphics& g){
 g.fillAll(black);g.setGradientFill(juce::ColourGradient(juce::Colour(36,39,41),0,0,black,(float)getWidth(),(float)getHeight(),true));g.fillAll();g.setColour(yellow);g.fillRect(0,76,getWidth(),3);
 auto box=[&](int x,int y,int w,int h){g.setColour(juce::Colours::black.withAlpha(.7f));g.fillRoundedRectangle((float)x+3,(float)y+4,(float)w,(float)h,9);g.setColour(panel);g.fillRoundedRectangle((float)x,(float)y,(float)w,(float)h,9);};
 box(18,92,305,180);box(335,92,305,180);box(18,286,620,180);box(652,92,330,374);g.setColour(yellow);g.fillRect(18,485,964,1);
 g.setColour(yellow.withAlpha(.13f));for(int i=0;i<28;++i)g.fillRect(20+i*35,670,20,2);
}
void WaveBombzWASPEditor::resized(){
 title.setBounds(22,12,430,40);subtitle.setBounds(24,50,500,20);oscLabel.setBounds(30,98,180,22);filterLabel.setBounds(348,98,220,22);envLabel.setBounds(30,292,220,22);lfoLabel.setBounds(665,98,250,22);
 auto place=[&](int i,int x,int y,int w=90,int h=100){knobs[(size_t)i]->setBounds(x,y,w,h);};
 place(0,35,125);place(1,125,125);place(2,215,125);place(3,355,125);place(4,450,125);place(5,545,125);
 place(6,45,320,100);place(7,150,320,100);place(8,255,320,100);place(9,360,320,100);
 place(10,670,135);place(11,765,135);place(12,670,250);place(13,765,250);place(14,865,250);
 modTitle.setBounds(668,365,280,22);modSource.setBounds(668,394,125,28);modTarget.setBounds(802,394,145,28);modDepthLabel.setBounds(668,426,55,22);modDepth.setBounds(720,426,225,24);meterText.setBounds(22,500,950,28);
}
void WaveBombzWASPEditor::timerCallback(){float pk=processor.getPeak();meterText.setText("MOMENTARY  "+juce::String(processor.getMomentaryLUFS(),1)+" LUFS     INTEGRATED  "+juce::String(processor.getIntegratedLUFS(),1)+" LUFS     PEAK  "+(pk>0?juce::String(juce::Decibels::gainToDecibels(pk),1):juce::String("-inf"))+" dBFS",juce::dontSendNotification);}
