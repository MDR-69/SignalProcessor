/*
  ==============================================================================

    PluginEditor.cpp
    PlayMe / Martin Di Rollo - 2014
    GUI of the audio plugin - no actual data processing is done here

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SignalProcessorAudioProcessorEditor::SignalProcessorAudioProcessorEditor (SignalProcessorAudioProcessor& owner)
    : AudioProcessorEditor (owner),
      audioProcessorInstance(owner),
      infoLabel (String::empty),
      averagingBufferLabel ("", "Averaging Buffer (samples):"),
      inputSensitivityLabel ("", "Input Sensitivity:"),
      channelLabel ("", "Channel Number:"),
      averagingBufferSlider ("averagingBuffer"),
      inputSensitivitySlider ("inputSensitivity"),
      sendTimeInfoButton("Send TimeInfo"),
      sendSignalLevelButton("Send SignalLevel"),
      sendImpulseButton("Send Impulse"),
      monoStereoButton ("Stereo Processing"),
      logoButton("PlayMe Signal Processor"),
      channelComboBox ("channel"),
      pluginFont("standard 07_57", 25.0f, 0),
      smallFont("standard 07_57", 14.0f, 0)
{
    
    // This is where our plugin's editor size is set.
    setSize (500, 500);
    
    slaf = new SquareLookAndFeel();
    setupSquareLookAndFeelColours (*slaf);
    
    // add some sliders..
    addAndMakeVisible (averagingBufferSlider);
    averagingBufferSlider.setLookAndFeel(slaf);
    averagingBufferSlider.setSliderStyle (Slider::LinearBar);
    averagingBufferSlider.addListener (this);
    averagingBufferSlider.setRange (64, 4096, 64);
    averagingBufferSlider.setValue(getProcessor().averagingBufferSize);
    averagingBufferSlider.setBounds (20, 240, 150, 20);
    
    addAndMakeVisible (inputSensitivitySlider);
    inputSensitivitySlider.setLookAndFeel(slaf);
    inputSensitivitySlider.setSliderStyle (Slider::LinearBar);
    inputSensitivitySlider.addListener (this);
    inputSensitivitySlider.setRange (0.0, 1.0, 0.01);
    inputSensitivitySlider.setValue(getProcessor().inputSensitivity);
    inputSensitivitySlider.setBounds (20, 280, 150, 20);
    
    addAndMakeVisible (sendTimeInfoButton);
    sendTimeInfoButton.setLookAndFeel(slaf);
    sendTimeInfoButton.addListener (this);
    sendTimeInfoButton.changeWidthToFitText();
    sendTimeInfoButton.setBounds (300, 180, 140, 20);
    sendTimeInfoButton.setColour (Label::textColourId, Colours::white);
    addAndMakeVisible (sendSignalLevelButton);
    sendSignalLevelButton.setLookAndFeel(slaf);
    sendSignalLevelButton.addListener (this);
    sendSignalLevelButton.changeWidthToFitText();
    sendSignalLevelButton.setBounds (300, 200, 140, 20);
    sendSignalLevelButton.setColour (Label::textColourId, Colours::white);
    addAndMakeVisible (sendImpulseButton);
    sendImpulseButton.setLookAndFeel(slaf);
    sendImpulseButton.addListener (this);
    sendImpulseButton.changeWidthToFitText();
    sendImpulseButton.setBounds (300, 220, 140, 20);
    sendImpulseButton.setColour (Label::textColourId, Colours::white);
    addAndMakeVisible (monoStereoButton);
    monoStereoButton.addListener (this);
    monoStereoButton.changeWidthToFitText();
    monoStereoButton.setBounds (300, 240, 140, 20);
    monoStereoButton.setColour (Label::textColourId, Colours::white);
    monoStereoButton.setLookAndFeel(slaf);

    
    sendTimeInfoButton.setToggleState(getProcessor().sendTimeInfo, dontSendNotification);
    sendSignalLevelButton.setToggleState(getProcessor().sendSignalLevel, dontSendNotification);
    sendImpulseButton.setToggleState(getProcessor().sendImpulse, dontSendNotification);
    monoStereoButton.setToggleState(getProcessor().monoStereo, dontSendNotification);
    
    addAndMakeVisible(channelComboBox);
    channelComboBox.setLookAndFeel(slaf);
    channelComboBox.setBounds (10, 185, 100, 24);
    channelComboBox.setEditableText (false);
    channelComboBox.setJustificationType (Justification::centred);
    String channelComboBoxElements[] = {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"};
    for (int i = 1; i <= sizeof(channelComboBoxElements)/sizeof(channelComboBoxElements[0]); ++i)
        channelComboBox.addItem (channelComboBoxElements[i-1], i);
    channelComboBox.setSelectedId (getProcessor().channel);
    channelComboBox.addListener (this);

    
    // add some labels for the sliders..
    averagingBufferLabel.attachToComponent (&averagingBufferSlider, false);
    averagingBufferLabel.setFont(smallFont);
    averagingBufferLabel.setColour(Label::textColourId, Colours::white);
    
    inputSensitivityLabel.attachToComponent (&inputSensitivitySlider, false);
    inputSensitivityLabel.setFont(smallFont);
    inputSensitivityLabel.setColour(Label::textColourId, Colours::white);

    logoImage = ImageCache::getFromMemory (BinaryData::logo_white_png, BinaryData::logo_white_pngSize);
    logoButton.setImages (true, true, true,
                  logoImage, 1.0f, Colours::transparentBlack,
                  logoImage, 1.0f, Colours::transparentBlack,
                  logoImage, 1.0f, Colours::transparentBlack,
                  0.5f);
    addAndMakeVisible (logoButton);
    logoButton.addListener (this);
    logoButton.setCentrePosition(getWidth()/2, 35 + logoImage.getHeight()/2);
    

    // add a label that will display the current timecode and status..
    addAndMakeVisible (infoLabel);
    infoLabel.setColour (Label::textColourId, Colours::white);
    infoLabel.setFont(smallFont);
    infoLabel.setCentrePosition(getWidth()/2, getHeight() - 40);
//    infoLabel.setBounds (10, getHeight()-40, getWidth() - 20, 25);
    
    startTimer (50);
    
}

SignalProcessorAudioProcessorEditor::~SignalProcessorAudioProcessorEditor()
{
    delete(slaf);
}

//==============================================================================
void SignalProcessorAudioProcessorEditor::paint (Graphics& g)
{
    
    g.setGradientFill (ColourGradient (Colours::grey, 0, 0,
                                       Colours::black, 0, (float) getHeight(), false));
    g.fillAll();
    
    //g.setColour (Colours::white);
    g.setFont (pluginFont);
    g.setColour (Colours::white);
    
    g.drawLine(20, 20, getWidth() - 20, 20, 8);
    g.drawLine(20, 90, getWidth() - 20, 90, 8);
    
    g.drawFittedText ("This plugin is to be used together with Strobot",
                      0, getHeight()/2 - 20, getWidth(), getHeight(),
                      Justification::centred, 1);
    
}


//==============================================================================
// This timer periodically checks whether any of the filter's parameters have changed...
void SignalProcessorAudioProcessorEditor::timerCallback()
{
    SignalProcessorAudioProcessor& ourProcessor = getProcessor();
    
    AudioPlayHead::CurrentPositionInfo newPos (ourProcessor.lastPosInfo);
    
    if (lastDisplayedPosition != newPos)
        displayPositionInfo (newPos);
    
    //To be set later, to update any parameter !!!
    averagingBufferSlider.setValue (ourProcessor.averagingBufferSize, dontSendNotification);
    inputSensitivitySlider.setValue (ourProcessor.inputSensitivity, dontSendNotification);
    
    
}

// This is our Slider::Listener callback, when the user drags a slider.
void SignalProcessorAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    
    if (slider == &averagingBufferSlider)
    {
        // It's vital to use setParameterNotifyingHost to change any parameters that are automatable
        // by the host, rather than just modifying them directly, otherwise the host won't know
        // that they've changed.
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::averagingBufferSizeParam,
                                                  (int) averagingBufferSlider.getValue());
    }
    else if (slider == &inputSensitivitySlider)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::inputSensitivityParam,
                                                  (float) inputSensitivitySlider.getValue());
    }
}

void SignalProcessorAudioProcessorEditor::buttonClicked (Button* button)
{
    
    if (button == &monoStereoButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::monoStereoParam,
                                                  button->getToggleState());
    }
    else if (button == &sendTimeInfoButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::sendTimeInfoParam,
                                                  button->getToggleState());
    }
    else if (button == &sendSignalLevelButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::sendSignalLevelParam,
                                                  button->getToggleState());
    }
    else if (button == &sendImpulseButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::sendImpulseParam,
                                                  button->getToggleState());
    }
}

void SignalProcessorAudioProcessorEditor::comboBoxChanged (ComboBox* comboBox)
{
    
    if (comboBox == &channelComboBox)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::channelParam, comboBox-> getSelectedId());
    }
    
}

void SignalProcessorAudioProcessorEditor::textEditorReturnKeyPressed (TextEditor& editor)
{
}

void SignalProcessorAudioProcessorEditor::textEditorTextChanged (TextEditor& editor)
{
}

void SignalProcessorAudioProcessorEditor::textEditorEscapeKeyPressed (TextEditor&)
{
}

void SignalProcessorAudioProcessorEditor::textEditorFocusLost (TextEditor& editor)
{
}


//==============================================================================
// GUI Look and Feel functions
void SignalProcessorAudioProcessorEditor::setupSquareLookAndFeelColours (LookAndFeel& laf)
{
    const Colour baseColour (Colours::red);
    laf.setColour (Slider::thumbColourId, Colour::greyLevel (0.95f));
    laf.setColour (Slider::textBoxOutlineColourId, Colours::transparentWhite);
    laf.setColour (Slider::rotarySliderFillColourId, baseColour);
    laf.setColour (Slider::rotarySliderOutlineColourId, Colours::white);
    laf.setColour (Slider::trackColourId, Colours::black);
    
    laf.setColour (TextButton::buttonColourId, Colours::white);
    laf.setColour (TextButton::textColourOffId, baseColour);
     
    laf.setColour (TextButton::buttonOnColourId, laf.findColour (TextButton::textColourOffId));
    laf.setColour (TextButton::textColourOnId, laf.findColour (TextButton::buttonColourId));
}
 
void SignalProcessorAudioProcessorEditor::setAllLookAndFeels (LookAndFeel* laf)
{
//    for (int i = 0; i < demoComp.getNumChildComponents(); ++i)
//        if (Component* c = demoComp.getChildComponent (i))
//            c->setLookAndFeel (laf);
}

//==============================================================================
// quick-and-dirty function to format a timecode string
static const String timeToTimecodeString (const double seconds)
{
    const double absSecs = fabs (seconds);
    
    const int hours =  (int) (absSecs / (60.0 * 60.0));
    const int mins  = ((int) (absSecs / 60.0)) % 60;
    const int secs  = ((int) absSecs) % 60;
    
    String s (seconds < 0 ? "-" : "");
    
    s << String (hours).paddedLeft ('0', 2) << ":"
    << String (mins) .paddedLeft ('0', 2) << ":"
    << String (secs) .paddedLeft ('0', 2) << ":"
    << String (roundToInt (absSecs * 1000) % 1000).paddedLeft ('0', 3);
    
    return s;
}

// quick-and-dirty function to format a bars/beats string
static const String ppqToBarsBeatsString (double ppq, double /*lastBarPPQ*/, int numerator, int denominator)
{
    if (numerator == 0 || denominator == 0)
        return "1|1|0";
    
    const int ppqPerBar = (numerator * 4 / denominator);
    const double beats  = (fmod (ppq, ppqPerBar) / ppqPerBar) * numerator;
    
    const int bar    = ((int) ppq) / ppqPerBar + 1;
    const int beat   = ((int) beats) + 1;
    const int ticks  = ((int) (fmod (beats, 1.0) * 960.0 + 0.5));
    
    String s;
    s << bar << '|' << beat << '|' << ticks;
    return s;
}

// Updates the text in our position label.
void SignalProcessorAudioProcessorEditor::displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos)
{
    lastDisplayedPosition = pos;
    String displayText;
    displayText.preallocateBytes (128);
    
    displayText << String (pos.bpm, 2) << " bpm, "
    << pos.timeSigNumerator << '/' << pos.timeSigDenominator
    << "  -  " << timeToTimecodeString (pos.timeInSeconds)
    << "  -  " << ppqToBarsBeatsString (pos.ppqPosition, pos.ppqPositionOfLastBarStart,
                                        pos.timeSigNumerator, pos.timeSigDenominator);
    
    if (pos.isRecording)
        displayText << "  (recording)";
    else if (pos.isPlaying)
        displayText << "  (playing)";
    
    infoLabel.setText ("[" + SystemStats::getJUCEVersion() + "]   " + displayText, dontSendNotification);
}

                                         
                                         
