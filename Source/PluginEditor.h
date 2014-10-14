/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#include <fstream>



//==============================================================================
/** This is the editor component that the plugin will display
 */
class SignalProcessorAudioProcessorEditor  : public AudioProcessorEditor,
                                             public SliderListener,
                                             public ComboBoxListener,
                                             public ButtonListener,
                                             public TextEditor::Listener,
                                             public Timer
{
public:
    SignalProcessorAudioProcessorEditor(SignalProcessorAudioProcessor&);

    ~SignalProcessorAudioProcessorEditor();
    
    //==============================================================================
    void timerCallback() override;
    void paint (Graphics&) override;
    void resized() override;
    void sliderValueChanged (Slider*) override;
    void buttonClicked (Button*) override;
    void comboBoxChanged (ComboBox*) override;
    void textEditorReturnKeyPressed (TextEditor& editor) override;
    void textEditorTextChanged (TextEditor& editor) override;
    void textEditorEscapeKeyPressed (TextEditor&) override;
    void textEditorFocusLost (TextEditor& editor) override;
    
private:
    SignalProcessorAudioProcessor& audioProcessorInstance;
    
    MidiKeyboardComponent midiKeyboard;
    Label infoLabel, averagingBufferLabel, inputSensitivityLabel, channelLabel, javaAppPathLabel;
    Slider averagingBufferSlider, inputSensitivitySlider;
    ToggleButton sendTimeInfoButton, sendSignalLevelButton, sendImpulseButton, monoStereoButton;
    ImageButton logoButton;
    ComboBox channelComboBox;
    ScopedPointer<ResizableCornerComponent> resizer;
    ComponentBoundsConstrainer resizeLimits;
    TextEditor textEditorProcessingPath;
    
    AudioPlayHead::CurrentPositionInfo lastDisplayedPosition;
    
    SignalProcessorAudioProcessor& getProcessor() const
    {
        return static_cast<SignalProcessorAudioProcessor&> (processor);
    }
    
    void displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos);

    String logoFilePath = "data/logo_white.png";
};


#endif  // PLUGINEDITOR_H_INCLUDED
