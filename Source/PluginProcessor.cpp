/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include <cstdlib>
#include <fstream>
#include "PluginProcessor.h"
#include "PluginEditor.h"

using boost::asio::local::stream_protocol;

//==============================================================================
SignalProcessorAudioProcessor::SignalProcessorAudioProcessor()
: channel(defaultChannel),
  averagingBufferSize(defaultAveragingBufferSize),
  inputSensitivity(defaultInputSensitivity),
  mode(defaultMode),
  monoStereo(defaultMonoStereo),
  signalLevelSocket(myIO_service),
  impulseSocket(myIO_service),
  signalLevelEndpoint(boost::asio::ip::address::from_string("127.0.0.1"), portNumberSignalLevel),
  impulseEndpoint(boost::asio::ip::address::from_string("127.0.0.1"), portNumberImpulse),
  datastringLevel(""),
  datastringImpulse("")
{
    lastPosInfo.resetToDefault();
//    averagingBufferSize             = defaultAveragingBufferSize;
//    inputSensitivity                = defaultInputSensitivity;
//    mode                            = defaultMode;
//    monoStereo                      = defaultMonoStereo;
//    channel                         = defaultChannel;
    
    try {
        std::cout << "SignalLevel Initial connection\n";
        signalLevelSocket.connect(signalLevelEndpoint);
        connectionEstablished_signalLevel = true;
    }
    catch (std::exception e) {
        std::cout << "SignalLevel - Couldn't do the initial connection\n";
    }

    try {
        std::cout << "Impulse Initial connection\n";
        impulseSocket.connect(impulseEndpoint);
        connectionEstablished_impulse = true;
    }
    catch (std::exception e) {
        std::cout << "Impulse - Couldn't do the initial connection\n";
    }
}

SignalProcessorAudioProcessor::~SignalProcessorAudioProcessor()
{
    signalLevelSocket.close();
    //impulseSocket.close();
    std::cout << "PlayMeSignalProcessor socket closed\n";
}





//==============================================================================
const String SignalProcessorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int SignalProcessorAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float SignalProcessorAudioProcessor::getParameter (int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case averagingBufferSizeParam:      return averagingBufferSize;
        case inputSensitivityParam:         return inputSensitivity;
        case modeParam:                     return mode;
        case channelParam:                  return channel;
        case monoStereoParam:               return monoStereo;
        default:                            return 0.0f;
    }
}

float SignalProcessorAudioProcessor::getParameterDefaultValue (int index)
{
    switch (index)
    {
        case averagingBufferSizeParam:      return defaultAveragingBufferSize;
        case inputSensitivityParam:         return defaultInputSensitivity;
        case modeParam:                     return defaultMode;
        case channelParam:                  return defaultChannel;
        case monoStereoParam:               return defaultMonoStereo;
        default:                            break;
    }
    
    return 0.0f;
}

void SignalProcessorAudioProcessor::setParameter (int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case averagingBufferSizeParam:      averagingBufferSize = newValue;  defineSignalMessagesAveragingBuffer(); break;
        case inputSensitivityParam:         inputSensitivity    = newValue;  break;
        case modeParam:                     mode                = newValue;  break;
        case channelParam:                  channel             = newValue;  defineSignalMessagesChannel();  break;
        case monoStereoParam:               monoStereo          = newValue;  break;
        default:                            break;
    }
}

const String SignalProcessorAudioProcessor::getParameterName (int index)
{
    switch (index)
    {
        case averagingBufferSizeParam:     return "Averaging Buffer Size";
        case inputSensitivityParam:        return "Input Sensitivity";
        case modeParam:                    return "Mode";
        case channelParam:                 return "Channel Number";
        case monoStereoParam:              return "Mono / Stereo";
        default:                           break;
    }
    return String::empty;
}

const String SignalProcessorAudioProcessor::getParameterText (int index)
{
    return String (getParameter (index), 2);
}

const String SignalProcessorAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String SignalProcessorAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool SignalProcessorAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool SignalProcessorAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool SignalProcessorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SignalProcessorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SignalProcessorAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double SignalProcessorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SignalProcessorAudioProcessor::getNumPrograms()
{
    return 0;
}

int SignalProcessorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SignalProcessorAudioProcessor::setCurrentProgram (int index)
{
}

const String SignalProcessorAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void SignalProcessorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SignalProcessorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    keyboardState.reset();
}

void SignalProcessorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    keyboardState.reset();
}

void SignalProcessorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // I've added this to avoid people getting screaming feedback
    // when they first compile the plugin, but obviously you don't need to
    // this code if your algorithm already fills all the output channels.
//    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
//        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Now pass any incoming midi messages to our keyboard state object, and let it
    // add messages to the buffer if the user is clicking on the on-screen keys
    //keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);
    
    
    //////////////////////////////////////////////////////////////////
    // Audio processing takes place here !
    
    // If the signal is defined by the user as mono, no need to check the second channel
    for (int channel = 0; channel < std::min(monoStereo,getNumInputChannels()); ++channel)
    {
        const float* channelData = buffer.getReadPointer (channel);
        
        //Only read one value out of nbOfSamplesToSkip, it's faster this way
        for (int i=0; i<buffer.getNumSamples(); i+=nbOfSamplesToSkip) {
            //The objective is to get an average of the signal's amplitude -> use the absolute value
            signalSum += std::abs(channelData[i]);
        }
    }
    
    nbBufValProcessed += buffer.getNumSamples();
    
    //Must be calculated before the instant signal, or else the beat effect will be minimized
    signalAverageEnergy = denormalize(((signalAverageEnergy * averageSignalWeight) + signalInstantEnergy) / averageEnergyBufferSize);
    signalInstantEnergy = signalSum / (averagingBufferSize * monoStereo);

    
    // If the instant signal energy is thresholdFactor times greater than the average energy, consider that a beat is detected
    if (signalInstantEnergy > signalAverageEnergy*thresholdFactor) {

        //Set the new signal Average Energy to the value of the instant energy, to avoid having bursts of false beat detections
        signalAverageEnergy = signalInstantEnergy;
        
        //Send the impulse message (which was pre-generated earlier)
        sendImpulseMessage(datastringImpulse);
        
    }
    
    
    if (nbBufValProcessed >= averagingBufferSize) {
        
        signal.set_signallevel(inputSensitivity * signalInstantEnergy);
        signal.SerializeToString(&datastringLevel);
        
        sendSignalLevelMessage(datastringLevel);
        
        nbBufValProcessed = 0;
        signalSum = 0;
    }
    
    // ask the host for the current time so we can display it...
    AudioPlayHead::CurrentPositionInfo newTime;
    
    if (getPlayHead() != nullptr && getPlayHead()->getCurrentPosition (newTime))
    {
        // Successfully got the current time from the host..
        lastPosInfo = newTime;
    }
    else
    {
        // If the host fails to fill-in the current time, we'll just clear it to a default..
        lastPosInfo.resetToDefault();
    }

}

float SignalProcessorAudioProcessor::denormalize(float input) {
    float temp = input + 1.0f;
    return temp - 1.0f;
}

//==============================================================================


void SignalProcessorAudioProcessor::defineSignalMessagesChannel() {
    
//    boost::asio::ip::tcp::endpoint newSignalLevelEndpoint(boost::asio::ip::address::from_string("127.0.0.1"), portNumberSignalLevel);
//    boost::asio::ip::tcp::endpoint newImpulseEndpoint(boost::asio::ip::address::from_string("127.0.0.1"), portNumberImpulse);
//    
//    signalLevelEndpoint = newSignalLevelEndpoint;
//    impulseEndpoint = newImpulseEndpoint;
//    connectionEstablished_impulse = false;
//    connectionEstablished_signalLevel = false;
//    signalLevelSocket.close();
//    impulseSocket.close();
    
    signal.set_signalid(channel);
    
    //It is possible to pre-serialize impulse messages here, as the message will never change
    impulse.set_signalid(channel);
    impulse.SerializeToString(&datastringImpulse);
    
    std::cout << "Reset channel as " << channel << "\n";
}

void SignalProcessorAudioProcessor::defineSignalMessagesAveragingBuffer() {

    //signal.set_buffersize(averagingBufferSize);
    
}


void SignalProcessorAudioProcessor::sendSignalLevelMessage(std::string datastring) {

    try {
        
        if (connectionEstablished_signalLevel == false) {
            std::cout << "Trying to reconnect\n";
            // Close the old, and try a new connection to the Java server - if impossible, an
            // exception is raised and the following instructions are not executed
            signalLevelSocket.close();
            signalLevelSocket.connect(signalLevelEndpoint);
            connectionEstablished_signalLevel = true;
        }
        
        // Sync write
        boost::asio::write(signalLevelSocket, boost::asio::buffer(datastring), ignored_error);
        // std::cout << "Error code : " << ignored_error << "   Size returned by write : " << size_written << " \n";
        
        // If the returned errorcode is different from 0 ("no error"), reset the server connection
        if (ignored_error.value() != 0) {
            std::cout << "Set the connection to false\n";
            connectionEstablished_signalLevel = false;
            signalLevelSocket.close();
        }
            
    } catch (const std::exception & e) {
        std::cout << "Caught an error while trying to initialize the socket - the Java server might not be ready\n";
        //std::cerr << e.what();
    }


}

void SignalProcessorAudioProcessor::sendImpulseMessage(std::string datastring) {
    
    try {
        
        if (connectionEstablished_impulse == false) {
            std::cout << "Trying to reconnect\n";
            // Close the old, and try a new connection to the Java server - if impossible, an
            // exception is raised and the following instructions are not executed
            impulseSocket.close();
            impulseSocket.connect(impulseEndpoint);
            connectionEstablished_impulse = true;
        }

        boost::asio::write(impulseSocket, boost::asio::buffer(datastring), ignored_error);
        // std::cout << "Wrote Impulse : " << testString << "  - size : " << writtensize << "  - result : " << ignored_error << "\n";

        
        // If the returned errorcode is different from 0 ("no error"), reset the server connection
        if (ignored_error.value() != 0) {
            std::cout << "Set the connection to false\n";
            connectionEstablished_impulse = false;
            impulseSocket.close();
        }
        
    } catch (const std::exception & e) {
        std::cout << "Caught an error while trying to initialize the socket - the Java server might not be ready\n";
        //std::cerr << e.what();
    }
    
    
}


//==============================================================================
bool SignalProcessorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SignalProcessorAudioProcessor::createEditor()
{
    return new SignalProcessorAudioProcessorEditor (*this);
}

//==============================================================================
void SignalProcessorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // Create an outer XML element..
    XmlElement xml ("MYPLUGINSETTINGS");
    
    // add some attributes to it..
    xml.setAttribute ("uiWidth", lastUIWidth);
    xml.setAttribute ("uiHeight", lastUIHeight);
    xml.setAttribute ("averagingBufferSize", averagingBufferSize);
    xml.setAttribute ("inputSensitivity", inputSensitivity);
    xml.setAttribute ("mode", mode);
    xml.setAttribute ("channel", channel);
    xml.setAttribute ("monoStereo", monoStereo);

    std::cout << "I just wrote channel : " << channel << "\n";
    
    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void SignalProcessorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // now pull out our parameters..
            lastUIWidth         = xmlState->getIntAttribute ("uiWidth", lastUIWidth);
            lastUIHeight        = xmlState->getIntAttribute ("uiHeight", lastUIHeight);
            averagingBufferSize = xmlState->getIntAttribute ("averagingBufferSize", averagingBufferSize);
            inputSensitivity    = (float) xmlState->getDoubleAttribute ("inputSensitivity", inputSensitivity);
            mode                = xmlState->getIntAttribute ("mode", mode);
            channel             = xmlState->getIntAttribute ("channel", channel);
            monoStereo          = xmlState->getIntAttribute ("monoStereo", monoStereo);
            std::cout << "I just read channel : " << channel << "\n";
        }
    }
}



//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SignalProcessorAudioProcessor();
}
