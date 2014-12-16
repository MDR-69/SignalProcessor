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
      tooltipWindow(nullptr, 700),
      infoLabel (String::empty),
      averagingBufferLabel ("", "Averaging Buffer Size:"),
      fftAveragingWindowLabel ("", "Averaging FFT Window:"),
      inputSensitivityLabel ("", "Input Sensitivity:"),
      beatDetectionWindowLabel ("", "Beat Detection Window:"),
      channelLabel ("", "Channel Number:"),
      averagingBufferSlider ("averagingBuffer"),
      fftAveragingWindowSlider ("fftAveragingWindow"),
      inputSensitivitySlider ("inputSensitivity"),
      beatDetectionWindowSlider ("beatDetectionWindow"),
      sendTimeInfoButton("Send TimeInfo"),
      sendSignalLevelButton("Send SignalLevel"),
      sendSignalInstantValButton("Send SignalInstantVal"),
      sendImpulseButton("Send Impulse"),
      sendFFTButton("Send Signal FFT"),
      monoStereoButton ("Stereo Processing"),
      logarithmicFFTButton("Logarithmic FFT"),
      sendOSCButton("Send OSC Data"),
      sendBinaryUDPButton("Send UDP Data"),
      sendTimeInfoButtonLabel ("", "Send Time Info"),
      sendSignalLevelButtonLabel ("", "Send Signal Level"),
      sendSignalInstantValButtonLabel ("", "Send Signal Instant Value"),
      sendImpulseButtonLabel ("", "Send Impulse"),
      sendFFTButtonLabel ("", "Send Signal FFT "),
      monoStereoButtonLabel ("", "Stereo Processing"),
      logarithmicFFTButtonLabel("", "Logarithmic FFT"),
      sendOSCButtonLabel ("", "Send Data Using OSC"),
      sendBinaryUDPButtonLabel ("", "Send Data Using UDP"),
      logoButton("PlayMe Signal Processor"),
      channelComboBox ("channel"),
      bigFont("standard 07_57", 45.0f, 0),
      pluginFont("standard 07_57", 25.0f, 0),
      smallFont("standard 07_57", 15.0f, 0)
{
    
    // This is where our plugin's editor size is set.
    setSize (500, 444);
    
    slaf = new SquareLookAndFeel();
    setupSquareLookAndFeelColours (*slaf);
    
    // Set the tooltips' look&feel
    tooltipWindow.setLookAndFeel(slaf);
    
    // add some sliders..
    addAndMakeVisible (averagingBufferSlider);
    averagingBufferSlider.setLookAndFeel(slaf);
    averagingBufferSlider.setSliderStyle (Slider::LinearBar);
    averagingBufferSlider.addListener (this);
    averagingBufferSlider.setRange (64, 4096, 1.0);
    averagingBufferSlider.setValue(getProcessor().averagingBufferSize);
    averagingBufferSlider.setBounds (20, 230, 150, 20);
    
    addAndMakeVisible (fftAveragingWindowSlider);
    fftAveragingWindowSlider.setLookAndFeel(slaf);
    fftAveragingWindowSlider.setSliderStyle (Slider::LinearBar);
    fftAveragingWindowSlider.addListener (this);
    fftAveragingWindowSlider.setRange (1, 20, 1);
    fftAveragingWindowSlider.setValue(getProcessor().fftAveragingWindow);
    fftAveragingWindowSlider.setBounds (20, 270, 150, 20);
    
    addAndMakeVisible (inputSensitivitySlider);
    inputSensitivitySlider.setLookAndFeel(slaf);
    inputSensitivitySlider.setSliderStyle (Slider::LinearBar);
    inputSensitivitySlider.addListener (this);
    inputSensitivitySlider.setRange (0.0, 5.0, 0.01);
    inputSensitivitySlider.setValue(getProcessor().inputSensitivity);
    inputSensitivitySlider.setBounds (20, 310, 150, 20);
    
    addAndMakeVisible (beatDetectionWindowSlider);
    beatDetectionWindowSlider.setLookAndFeel(slaf);
    beatDetectionWindowSlider.setSliderStyle (Slider::LinearBar);
    beatDetectionWindowSlider.addListener (this);
    beatDetectionWindowSlider.setRange (2.0, 16.0, 1);
    beatDetectionWindowSlider.setValue(getProcessor().averageEnergyBufferSize);
    beatDetectionWindowSlider.setBounds (20, 350, 150, 20);
    
    addAndMakeVisible (sendTimeInfoButton);
    sendTimeInfoButton.setLookAndFeel(slaf);
    sendTimeInfoButton.addListener (this);
    sendTimeInfoButton.changeWidthToFitText();
    sendTimeInfoButton.setBounds (getWidth() - 50, 170, 18, 18);
    sendTimeInfoButton.setColour (Label::textColourId, Colours::white);
    sendTimeInfoButton.setButtonText("");
    sendTimeInfoButton.setTooltip("Check this to send periodically the DAW's playback status, the BPM as well as the current position. These messages are approximately sent every 50 ms");
    addAndMakeVisible (sendSignalLevelButton);
    sendSignalLevelButton.setLookAndFeel(slaf);
    sendSignalLevelButton.addListener (this);
    sendSignalLevelButton.changeWidthToFitText();
    sendSignalLevelButton.setBounds (getWidth() - 50, 192, 18, 18);
    sendSignalLevelButton.setColour (Label::textColourId, Colours::white);
    sendSignalLevelButton.setButtonText("");
    sendSignalLevelButton.setTooltip("Check this to send periodically this track's averaged signal level. You can configure the frequency at which this plugin sends these messages by modifying the averaging buffer value (shorter values mean more messages)");
    addAndMakeVisible (sendSignalLevelButton);
    
    sendSignalInstantValButton.setLookAndFeel(slaf);
    sendSignalInstantValButton.addListener (this);
    sendSignalInstantValButton.changeWidthToFitText();
    sendSignalInstantValButton.setBounds (getWidth() - 50, 214, 18, 18);                //Val a changer
    sendSignalInstantValButton.setColour (Label::textColourId, Colours::white);
    sendSignalInstantValButton.setButtonText("");
    sendSignalInstantValButton.setTooltip("Check this to send periodically this track's instant signal level. You can configure the frequency at which this plugin sends these messages by modifying the number of samples to skip");
    
    addAndMakeVisible (sendImpulseButton);
    sendImpulseButton.setLookAndFeel(slaf);
    sendImpulseButton.addListener (this);
    sendImpulseButton.changeWidthToFitText();
    sendImpulseButton.setBounds (getWidth() - 50, 236, 18, 18);
    sendImpulseButton.setColour (Label::textColourId, Colours::white);
    sendImpulseButton.setButtonText("");
    sendImpulseButton.setTooltip("Check this to send the beats detected by the plugin. Whenever a beat is detected, the red circle in the center of the GUI will flash");
    
    addAndMakeVisible (sendFFTButton);
    sendFFTButton.setLookAndFeel(slaf);
    sendFFTButton.addListener (this);
    sendFFTButton.changeWidthToFitText();
    sendFFTButton.setBounds (getWidth() - 50, 258, 18, 18);
    sendFFTButton.setColour (Label::textColourId, Colours::white);
    sendFFTButton.setButtonText("");
    sendFFTButton.setTooltip("Check this to send a frequency analysis of the input source. Compared to the other analysis, this tends to be more expensive in terms of CPU resources, so only check this if you plan to use it");
    
    addAndMakeVisible (monoStereoButton);
    monoStereoButton.addListener (this);
    monoStereoButton.changeWidthToFitText();
    monoStereoButton.setBounds (getWidth() - 50, 280, 18, 18);
    monoStereoButton.setColour (Label::textColourId, Colours::white);
    monoStereoButton.setLookAndFeel(slaf);
    monoStereoButton.setButtonText("");
    monoStereoButton.setTooltip("Check this to consider the input source as a stereo source for the signal level analysis. To use less resources, the default behaviour only checks the left input. Considering your source stereo is only useful if you have very agressive left/right panning effects");

    addAndMakeVisible (logarithmicFFTButton);
    logarithmicFFTButton.addListener (this);
    logarithmicFFTButton.changeWidthToFitText();
    logarithmicFFTButton.setBounds (getWidth() - 50, 302, 18, 18);
    logarithmicFFTButton.setColour (Label::textColourId, Colours::white);
    logarithmicFFTButton.setLookAndFeel(slaf);
    logarithmicFFTButton.setButtonText("");
    logarithmicFFTButton.setTooltip("Check this to reorganize the FFT in a logarithmic way, as well as normalize its values (the frequency band with the max intensity will have its intensity equal to 1). If unchecked, the message contains all 2048 bands. If checked, the message contains 12 bands :\n11025 to 22050 Hz\n5512 to 11025 Hz\n2756 to 5512 Hz\n1378 to 2756 Hz\n689 to 1378 Hz\n344 to 689 Hz\n172 to 344 Hz\n86 to 172 Hz\n43 to 86 Hz\n22 to 43 Hz\n11 to 22 Hz\n0 to 11 Hz");
    
    addAndMakeVisible (sendOSCButton);
    sendOSCButton.setLookAndFeel(slaf);
    sendOSCButton.addListener (this);
    sendOSCButton.changeWidthToFitText();
    sendOSCButton.setBounds (getWidth() - 50, 352, 18, 18);
    sendOSCButton.setColour (Label::textColourId, Colours::white);
    sendOSCButton.setButtonText("");
    sendOSCButton.setTooltip("Check this to consider the input source as a stereo source for the signal level analysis. To use less resources, the default behaviour only checks the left input. Considering your source stereo is only useful if you have very agressive left/right panning effects");

    addAndMakeVisible (sendBinaryUDPButton);
    sendBinaryUDPButton.setLookAndFeel(slaf);
    sendBinaryUDPButton.addListener (this);
    sendBinaryUDPButton.changeWidthToFitText();
    sendBinaryUDPButton.setBounds (getWidth() - 50, 374, 18, 18);
    sendBinaryUDPButton.setColour (Label::textColourId, Colours::white);
    sendBinaryUDPButton.setButtonText("");
    sendBinaryUDPButton.setTooltip("Send data using raw binary UDP. This is the most effective way to use this plugin. The following ports are used by this plugin :\nSignal level: " + String(getProcessor().portNumberSignalLevel) + ". Signal instant value: " + String(getProcessor().portNumberSignalInstantVal) + ". Impulse: " + String(getProcessor().portNumberImpulse) + ". TimeInfo: " + String(getProcessor().portNumberTimeInfo) + ". FFT Data: " + String(getProcessor().portNumberFFT));

    
    sendTimeInfoButton.setToggleState(getProcessor().sendTimeInfo, dontSendNotification);
    sendSignalLevelButton.setToggleState(getProcessor().sendSignalLevel, dontSendNotification);
    sendSignalInstantValButton.setToggleState(getProcessor().sendSignalInstantVal, dontSendNotification);
    sendImpulseButton.setToggleState(getProcessor().sendImpulse, dontSendNotification);
    sendFFTButton.setToggleState(getProcessor().sendFFT, dontSendNotification);
    monoStereoButton.setToggleState(getProcessor().monoStereo, dontSendNotification);
    logarithmicFFTButton.setToggleState(getProcessor().logarithmicFFT, dontSendNotification);
    sendOSCButton.setToggleState(getProcessor().sendOSC, dontSendNotification);
    sendBinaryUDPButton.setToggleState(getProcessor().sendBinaryUDP, dontSendNotification);

    sendTimeInfoButtonLabel.attachToComponent (&sendTimeInfoButton, true);
    sendTimeInfoButtonLabel.setFont(smallFont);
    sendTimeInfoButtonLabel.setColour(Label::textColourId, Colours::white);
    sendSignalLevelButtonLabel.attachToComponent (&sendSignalLevelButton, true);
    sendSignalLevelButtonLabel.setFont(smallFont);
    sendSignalLevelButtonLabel.setColour(Label::textColourId, Colours::white);
    sendSignalInstantValButtonLabel.attachToComponent (&sendSignalInstantValButton, true);
    sendSignalInstantValButtonLabel.setFont(smallFont);
    sendSignalInstantValButtonLabel.setColour(Label::textColourId, Colours::white);
    sendImpulseButtonLabel.attachToComponent (&sendImpulseButton, true);
    sendImpulseButtonLabel.setFont(smallFont);
    sendImpulseButtonLabel.setColour(Label::textColourId, Colours::white);
    sendFFTButtonLabel.attachToComponent (&sendFFTButton, true);
    sendFFTButtonLabel.setFont(smallFont);
    sendFFTButtonLabel.setColour(Label::textColourId, Colours::white);
    monoStereoButtonLabel.attachToComponent (&monoStereoButton, true);
    monoStereoButtonLabel.setFont(smallFont);
    monoStereoButtonLabel.setColour(Label::textColourId, Colours::white);
    logarithmicFFTButtonLabel.attachToComponent (&logarithmicFFTButton, true);
    logarithmicFFTButtonLabel.setFont(smallFont);
    logarithmicFFTButtonLabel.setColour(Label::textColourId, Colours::white);
    sendOSCButtonLabel.attachToComponent (&sendOSCButton, true);
    sendOSCButtonLabel.setFont(smallFont);
    sendOSCButtonLabel.setColour(Label::textColourId, Colours::white);
    sendBinaryUDPButtonLabel.attachToComponent (&sendBinaryUDPButton, true);
    sendBinaryUDPButtonLabel.setFont(smallFont);
    sendBinaryUDPButtonLabel.setColour(Label::textColourId, Colours::white);
    
    addAndMakeVisible(channelComboBox);
    channelComboBox.setBounds (20, 185, 150, 20);
    channelComboBox.setEditableText (false);
    channelComboBox.setJustificationType (Justification::centred);
    String channelComboBoxElements[] = {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"};
    for (int i = 1; i <= sizeof(channelComboBoxElements)/sizeof(channelComboBoxElements[0]); ++i)
        channelComboBox.addItem (channelComboBoxElements[i-1], i);
    channelComboBox.setSelectedId (getProcessor().channel);
    channelComboBox.addListener (this);
    channelComboBox.setLookAndFeel(slaf);
    
    // add some labels for the sliders..
    averagingBufferLabel.attachToComponent (&averagingBufferSlider, false);
    averagingBufferLabel.setFont(smallFont);
    averagingBufferLabel.setColour(Label::textColourId, Colours::white);
    averagingBufferLabel.setTooltip("Change this parameter to adjust the instant signal value's averaging window. Shorter values mean more precise signal values, and more messages sent");
    
    fftAveragingWindowLabel.attachToComponent (&fftAveragingWindowSlider, false);
    fftAveragingWindowLabel.setFont(smallFont);
    fftAveragingWindowLabel.setColour(Label::textColourId, Colours::white);
    fftAveragingWindowLabel.setTooltip("Change this parameter to set the the resulting FFT's averaging window (n * 4096 samples). Lower values induce a more reactive FFT, higher values mean that transient shots will be more or less ignored");
    
    inputSensitivityLabel.attachToComponent (&inputSensitivitySlider, false);
    inputSensitivityLabel.setFont(smallFont);
    inputSensitivityLabel.setColour(Label::textColourId, Colours::white);
    inputSensitivityLabel.setTooltip("Change this parameter to adjust the instant signal's level. This is useful if you want to keep a track's volume low in your DAW, but the program consuming this data expects a normal signal");
    
    beatDetectionWindowLabel.attachToComponent (&beatDetectionWindowSlider, false);
    beatDetectionWindowLabel.setFont(smallFont);
    beatDetectionWindowLabel.setColour(Label::textColourId, Colours::white);
    beatDetectionWindowLabel.setTooltip("Change this parameter to adjust the beat detection's window size. Lower values mean the analysis will be more reactive, but also more prone to false detections due to the sound's release. Tip : for highly rhythmic and precise sources, use low values (ex: short kicks, snares), whereas for sources with a long sustain (ex: bass, guitar), use higher values");
    
    channelLabel.attachToComponent (&channelComboBox, false);
    channelLabel.setFont(smallFont);
    channelLabel.setColour(Label::textColourId, Colours::white);
    channelLabel.setTooltip("Set the channel ID for the source to analyze. This is used by the remote application to know what kind of data is processed (ie 'is it a kick or a snare ?')");
    
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
    
    startTimer (20);
    
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
    g.setColour (Colours::white);
    
    g.drawLine(20, 20, getWidth() - 20, 20, 8);
    g.drawLine(20, 90, getWidth() - 20, 90, 8);

    g.setFont (bigFont);
    g.drawFittedText ("Signal Processor",
                      0, 110, getWidth(), 45,
                      Justification::centred, 1);

    g.setFont (pluginFont);
    g.drawFittedText ("This plugin is to be used together with Strobot",
                      0, getHeight()/2 - 20, getWidth(), getHeight(),
                      Justification::centred, 1);
    
    g.setColour(Colours::red);
    g.fillEllipse(getWidth()/2 + (1.0 - lastDisplayedBeatIntensity)*20, getHeight()/2 + 30 + (1.0 - lastDisplayedBeatIntensity)*20, lastDisplayedBeatIntensity*40, lastDisplayedBeatIntensity*40);

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
    fftAveragingWindowSlider.setValue (ourProcessor.fftAveragingWindow, dontSendNotification);
    
    float newBeatIntensity = ourProcessor.beatIntensity;
    if (lastDisplayedBeatIntensity != newBeatIntensity) {
        lastDisplayedBeatIntensity = newBeatIntensity;
        repaint();
    }
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
    else if (slider == &beatDetectionWindowSlider)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::averageEnergyBufferSizeParam,
                                                  (int) beatDetectionWindowSlider.getValue());
    }
    else if (slider == &fftAveragingWindowSlider)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::fftAveragingWindowParam,
                                                  (int) fftAveragingWindowSlider.getValue());
    }
}

void SignalProcessorAudioProcessorEditor::buttonClicked (Button* button)
{
    
    if (button == &monoStereoButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::monoStereoParam,
                                                  button->getToggleState());
    }
    if (button == &logarithmicFFTButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::logarithmicFFTParam,
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
    else if (button == &sendSignalInstantValButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::sendSignalInstantValParam,
                                                  button->getToggleState());
    }
    else if (button == &sendImpulseButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::sendImpulseParam,
                                                  button->getToggleState());
    }
    else if (button == &sendFFTButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::sendFFTParam,
                                                  button->getToggleState());
    }
    else if (button == &sendOSCButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::sendOSCParam,
                                                  button->getToggleState());
    }
    else if (button == &sendBinaryUDPButton)
    {
        getProcessor().setParameterNotifyingHost (SignalProcessorAudioProcessor::sendBinaryUDPParam,
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
    laf.setColour (Slider::trackColourId, Colours::white);
    laf.setColour (Slider::textBoxTextColourId, Colours::white);
    
    laf.setColour (TextButton::buttonColourId, Colours::white);
    laf.setColour (TextButton::textColourOffId, baseColour);
    laf.setColour (TextButton::buttonOnColourId, laf.findColour (TextButton::textColourOffId));
    laf.setColour (TextButton::textColourOnId, laf.findColour (TextButton::buttonColourId));
    laf.setColour (ToggleButton::textColourId, Colours::white);
    
    laf.setColour (Label::textColourId, Colours::white);
    
    laf.setColour (ComboBox::textColourId, Colours::white);
    laf.setColour (ComboBox::backgroundColourId, baseColour);
    laf.setColour (ComboBox::buttonColourId, Colours::black);
    laf.setColour (ComboBox::outlineColourId, Colours::transparentWhite);
    
    laf.setColour(TooltipWindow::textColourId, Colours::black);
    laf.setColour(TooltipWindow::backgroundColourId, Colour::fromRGB(192, 0, 0));

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

                                         
                                         
