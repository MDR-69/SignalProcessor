/*
 ==============================================================================
 
    PluginProcessor.cpp
    PlayMe / Martin Di Rollo - 2014
    Analyze the incoming audio data (without modifying it), and if requested
    by the user, send specific messages in order to transmit the data to
    another application
    The signal processing is done here, inside the processBlock function
 
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
  monoStereo(defaultMonoStereo),
  averageEnergyBufferSize(defaultAverageEnergyBufferSize),
  udpClientTimeInfo("127.0.0.1", portNumberTimeInfo),
  udpClientSignalLevel("127.0.0.1", portNumberSignalLevel),
  udpClientImpulse("127.0.0.1", portNumberImpulse)
{
    
    //Build the default Signal Messages, and preallocate the char* which will receive their serialized data
    defineDefaultSignalMessages();
    
    dataArrayImpulse  = new char[impulse.ByteSize()];
    dataArrayLevel    = new char[signal.ByteSize()];
    dataArrayTimeInfo = new char[timeInfo.ByteSize()];
    
    lastPosInfo.resetToDefault();

}

SignalProcessorAudioProcessor::~SignalProcessorAudioProcessor()
{
    
    delete [] dataArrayImpulse;
    delete [] dataArrayLevel;
    delete [] dataArrayTimeInfo;
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
        case sendTimeInfoParam:             return sendTimeInfo;
        case sendSignalLevelParam:          return sendSignalLevel;
        case sendImpulseParam:              return sendImpulse;
        case channelParam:                  return channel;
        case monoStereoParam:               return monoStereo;
        case averageEnergyBufferSizeParam:  return averageEnergyBufferSize;
        default:                            return 0.0f;
    }
}

float SignalProcessorAudioProcessor::getParameterDefaultValue (int index)
{
    switch (index)
    {
        case averagingBufferSizeParam:      return defaultAveragingBufferSize;
        case inputSensitivityParam:         return defaultInputSensitivity;
        case sendTimeInfoParam:             return defaultSendTimeInfo;
        case sendSignalLevelParam:          return defaultSendSignalLevel;
        case sendImpulseParam:              return defaultSendImpulse;
        case channelParam:                  return defaultChannel;
        case monoStereoParam:               return defaultMonoStereo;
        case averageEnergyBufferSizeParam:  return defaultAverageEnergyBufferSize;
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
        case averagingBufferSizeParam:      averagingBufferSize     = newValue;  break;
        case inputSensitivityParam:         inputSensitivity        = newValue;  break;
        case sendTimeInfoParam:             sendTimeInfo            = newValue;  break;
        case sendSignalLevelParam:          sendSignalLevel         = newValue;  break;
        case sendImpulseParam:              sendImpulse             = newValue;  break;
        case channelParam:                  channel                 = newValue;  defineSignalMessagesChannel();  break;
        case monoStereoParam:               monoStereo              = newValue;  break;
        case averageEnergyBufferSizeParam:  averageEnergyBufferSize = newValue;  break;
        default:                            break;
    }
}

const String SignalProcessorAudioProcessor::getParameterName (int index)
{
    switch (index)
    {
        case averagingBufferSizeParam:     return "Averaging Buffer Size"; break;
        case inputSensitivityParam:        return "Input Sensitivity";     break;
        case sendTimeInfoParam:            return "Send Time Info";        break;
        case sendSignalLevelParam:         return "Send Signal Level";     break;
        case sendImpulseParam:             return "Send Impulses";         break;
        case channelParam:                 return "Channel Number";        break;
        case monoStereoParam:              return "Mono / Stereo";         break;
        case averageEnergyBufferSizeParam: return "Beat Detection Window"; break;
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
}

void SignalProcessorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void SignalProcessorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    //////////////////////////////////////////////////////////////////
    // Audio processing takes place here !
    
    // If the signal is defined by the user as mono, no need to check the second channel
    int numberOfChannels = (monoStereo==false) ? 1 : getNumInputChannels();
    for (int channel = 0; channel < numberOfChannels; channel++)
    {
        const float* channelData = buffer.getReadPointer (channel);
        
        //Only read one value out of nbOfSamplesToSkip, it's faster this way
        for (int i=0; i<buffer.getNumSamples(); i+=nbOfSamplesToSkip) {
            //The objective is to get an average of the signal's amplitude -> use the absolute value
            signalSum += std::abs(channelData[i]);
        }
    }
    
    nbBufValProcessed += buffer.getNumSamples();
    samplesSinceLastTimeInfoTransmission += buffer.getNumSamples();
    
    //Must be calculated before the instant signal, or else the beat effect will be minimized
    signalAverageEnergy = denormalize(((signalAverageEnergy * (averageEnergyBufferSize-1)) + signalInstantEnergy) / averageEnergyBufferSize);
    signalInstantEnergy = signalSum / (averagingBufferSize * numberOfChannels);

    if (sendImpulse == true) {
        // Fade the beat detection image (variable used by the editor)
        if (beatIntensity > 0.1) {
            beatIntensity = std::max(0.1, beatIntensity - 0.05);
        }
        else {
            beatIntensity = 0.1;
        }
        
    }
    else {
        beatIntensity = 0;
    }
    
    // If the instant signal energy is thresholdFactor times greater than the average energy, consider that a beat is detected
    if (signalInstantEnergy > signalAverageEnergy*thresholdFactor) {
        
        //Set the new signal Average Energy to the value of the instant energy, to avoid having bursts of false beat detections
        signalAverageEnergy = signalInstantEnergy;
        
        if (sendImpulse == true) {
            beatIntensity = 1.0f;
            //Send the impulse message (which was pre-generated earlier)
            udpClientImpulse.send(dataArrayImpulse, impulse.GetCachedSize());
        }
    }
    

    
    if (nbBufValProcessed >= averagingBufferSize) {
        if (sendSignalLevel == true) {
            signal.set_signallevel(inputSensitivity * signalInstantEnergy);
            signal.SerializeToArray(dataArrayLevel, signal.GetCachedSize());
            udpClientSignalLevel.send(dataArrayLevel, signal.GetCachedSize());
        }
        
        nbBufValProcessed = 0;
        signalSum = 0;
    }
    
    
    if (samplesSinceLastTimeInfoTransmission >= timeInfoCycle) {
        // Ask the host for the current time
        if (sendTimeInfo == true) {
            AudioPlayHead::CurrentPositionInfo currentTime;
            if (getPlayHead() != nullptr && getPlayHead()->getCurrentPosition (currentTime))
            {
                // Update the variable used to display the latest time in the GUI
                lastPosInfo = currentTime;
                
                // Successfully got the current time from the host, set the pulses-per-quarter-note value inside the timeInfo message
                timeInfo.set_position((float)currentTime.ppqPosition);
                timeInfo.set_isplaying(currentTime.isPlaying);
                timeInfo.set_tempo((float)currentTime.bpm);
                timeInfo.SerializeToArray(dataArrayTimeInfo, timeInfo.GetCachedSize());
                udpClientTimeInfo.send(dataArrayTimeInfo, timeInfo.GetCachedSize());
            }
        }
        else {
            // Don't send the current time, set the GUI info to a default value
            lastPosInfo.resetToDefault();
        }
        samplesSinceLastTimeInfoTransmission = 0;
    }
}

float SignalProcessorAudioProcessor::denormalize(float input) {
    float temp = input + 1.0f;
    return temp - 1.0f;
}

//==============================================================================


// Build the default signal messages
void SignalProcessorAudioProcessor::defineDefaultSignalMessages() {
    
    signal.set_signalid(channel);
    signal.set_signallevel(0.0);
    
    impulse.set_signalid(channel);
    impulse.SerializeToArray(dataArrayImpulse, impulse.GetCachedSize());
    
    timeInfo.set_isplaying(false);
    timeInfo.set_position(0.0);
    timeInfo.set_tempo(120.0);
    
}

void SignalProcessorAudioProcessor::defineSignalMessagesChannel() {
    
    signal.set_signalid(channel);
    
    //It is possible to pre-serialize impulse messages here, as the message will never change
    impulse.set_signalid(channel);
    impulse.SerializeToArray(dataArrayImpulse, impulse.GetCachedSize());
    
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
    xml.setAttribute ("averagingBufferSize", averagingBufferSize);
    xml.setAttribute ("inputSensitivity", inputSensitivity);
    xml.setAttribute ("sendTimeInfo", sendTimeInfo);
    xml.setAttribute ("sendSignalLevel", sendSignalLevel);
    xml.setAttribute ("sendImpulse", sendImpulse);
    xml.setAttribute ("channel", channel);
    xml.setAttribute ("monoStereo", monoStereo);
    xml.setAttribute ("averageEnergyBufferSize", averageEnergyBufferSize);
    
    std::cout << "SignalProcessor - Successfully saved XML plugin info for channel : " << channel << "\n";
    
    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void SignalProcessorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore the parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // now pull out our parameters..
            averagingBufferSize     = xmlState->getIntAttribute ("averagingBufferSize", averagingBufferSize);
            inputSensitivity        = (float) xmlState->getDoubleAttribute ("inputSensitivity", inputSensitivity);
            sendTimeInfo            = xmlState->getBoolAttribute ("sendTimeInfo", sendTimeInfo);
            sendSignalLevel         = xmlState->getBoolAttribute ("sendSignalLevel", sendSignalLevel);
            sendImpulse             = xmlState->getBoolAttribute ("sendImpulse", sendImpulse);
            channel                 = xmlState->getIntAttribute ("channel", channel);
            monoStereo              = xmlState->getBoolAttribute ("monoStereo", monoStereo);
            averageEnergyBufferSize = xmlState->getIntAttribute ("averageEnergyBufferSize", averageEnergyBufferSize);
        }
    }
}



//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SignalProcessorAudioProcessor();
}
