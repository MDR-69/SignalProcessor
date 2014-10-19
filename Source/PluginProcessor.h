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

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include "SignalMessages.pb.h"

//==============================================================================
/**
 */
class SignalProcessorAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    SignalProcessorAudioProcessor();
    ~SignalProcessorAudioProcessor();
    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    
    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;
    
    //==============================================================================
    const String getName() const;
    
    int getNumParameters();
    
    float getParameter (int index);
    float getParameterDefaultValue (int index);
    void setParameter (int index, float newValue);
    
    const String getParameterName (int index);
    const String getParameterText (int index);
    
    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;
    
    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;
    
    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);
    
    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);
    
    void defineSignalMessagesChannel();
    void defineSignalMessagesAveragingBuffer();
    void defineSignalMessagesTimeInfo();
    void sendSignalLevelMessage(std::string datastring);
    void sendImpulseMessage(std::string datastring);
    void sendTimeInfoMessage(std::string datastring);
    
    float denormalize(float input);
    
    // this keeps a copy of the last set of time info that was acquired during an audio
    // callback - the UI component will read this and display it.
    AudioPlayHead::CurrentPositionInfo lastPosInfo;
    
    // these are used to persist the UI's size - the values are stored along with the
    // filter's other parameters, and the UI component will update them when it gets
    // resized.
    int lastUIWidth, lastUIHeight;
    
    
    //==============================================================================
    // Default parameter values
    const int defaultAveragingBufferSize = 2048;
    const bool defaultSendTimeInfo = false;
    const bool defaultSendSignalLevel = true;
    const bool defaultSendImpulse = true;
    const bool defaultMonoStereo = false;        //Mono processing
    const float defaultInputSensitivity = 1.0;
    const int defaultChannel = 1;
    
    
    
    //==============================================================================
    enum Parameters
    {
        averagingBufferSizeParam = 0,
        inputSensitivityParam,
        sendTimeInfoParam,
        sendSignalLevelParam,
        sendImpulseParam,
        channelParam,
        monoStereoParam,
        totalNumParams
    };
    
    int channel;
    int averagingBufferSize;
    float inputSensitivity;
    bool sendTimeInfo    = true;
    bool sendSignalLevel = true;
    bool sendImpulse     = true;
    bool monoStereo;         //false -> mono
    
    
    //==============================================================================
    // Variables used by the audio algorithm
    int nbBufValProcessed = 0;
    float signalSum = 0;
    // Used for beat detection
    float signalAverageEnergy = 0;
    float signalInstantEnergy = 0;
    const int thresholdFactor = 4;
    const int averageEnergyBufferSize = 20;                         //16 times the size of the buffer set by the DAW
    const int averageSignalWeight = averageEnergyBufferSize - 1;    //Just a const variable to gain a substraction operation
    int samplesSinceLastTimeInfoTransmission = 0;                   //The time message is to be sent every timeInfoCycle (if active)
    
    //==============================================================================
    // Socket used to forward data to the Processing application, and the variables associated with it
    const int portNumberTimeInfo    = 7000;
    const int portNumberSignalLevel = 8000;
    const int portNumberImpulse     = 9000;
    const int nbOfSamplesToSkip     = 6;
    const int timeInfoCycle         = 4096;       //Send the time info message every 4096 samples, that's about 100ms
    bool connectionEstablished_timeInfo = false;
    bool connectionEstablished_signalLevel = false;
    bool connectionEstablished_impulse = false;
    boost::asio::io_service myIO_service;
    boost::asio::ip::tcp::socket timeInfoSocket;
    boost::asio::ip::tcp::socket signalLevelSocket;
    boost::asio::ip::tcp::socket impulseSocket;
    boost::asio::ip::tcp::endpoint timeInfoEndpoint;
    boost::asio::ip::tcp::endpoint signalLevelEndpoint;
    boost::asio::ip::tcp::endpoint impulseEndpoint;
    boost::system::error_code ignored_error;
    std::string datastringTimeInfo;
    std::string datastringLevel;
    std::string datastringImpulse;
    
    //==============================================================================
    // Small optimisation : always use the same SignalMessages objects, it saves creating a new one every time
    Impulse impulse;
    SignalLevel signal;
    TimeInfo timeInfo;
    
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalProcessorAudioProcessor)
    
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
