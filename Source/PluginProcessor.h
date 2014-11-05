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
#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"
#include "udp_client_server.h"
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
    
    void calculateFFT ();
    
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
    
    void defineDefaultSignalMessages();
    void defineSignalMessagesChannel();

    
    float denormalize(float input);
    
    AudioPlayHead::CurrentPositionInfo lastPosInfo;
    
    //==============================================================================
    // Default parameter values
    const int defaultAveragingBufferSize     = 2048;
    const int defaultFftBufferSize           = 4096;
    const bool defaultSendTimeInfo           = false;
    const bool defaultSendSignalLevel        = true;
    const bool defaultSendImpulse            = true;
    const bool defaultMonoStereo             = false;        //Mono processing
    const float defaultInputSensitivity      = 1.0;
    const int defaultChannel                 = 1;
    const int defaultAverageEnergyBufferSize = 8.0;
    
    //==============================================================================
    enum Parameters
    {
        averagingBufferSizeParam = 0,
        fftBufferSizeParam,
        inputSensitivityParam,
        sendTimeInfoParam,
        sendSignalLevelParam,
        sendImpulseParam,
        channelParam,
        monoStereoParam,
        averageEnergyBufferSizeParam,
        totalNumParams
    };
    
    int channel;
    int averagingBufferSize;
    int fftBufferSize;
    float inputSensitivity;
    bool sendTimeInfo    = true;
    bool sendSignalLevel = true;
    bool sendImpulse     = true;
    bool sendFFT         = false;
    bool monoStereo;         //false -> mono
    int averageEnergyBufferSize;
    
    //==============================================================================
    // Variables used by the audio algorithm
    int nbBufValProcessed = 0;
    float signalSum = 0;
    // Used for beat detection
    float signalAverageEnergy = 0;
    float signalInstantEnergy = 0;
    const int thresholdFactor = 4;
    int samplesSinceLastTimeInfoTransmission = 0;                   //The time message is to be sent every timeInfoCycle (if active)

    // Set to 1.0f when a beat is detected
    float beatIntensity = 0.1f;
    
    //==============================================================================
    // Socket used to forward data to the Processing application, and the variables associated with it
    const int portNumberSignalLevel = 7001;
    const int portNumberImpulse     = 7002;
    const int portNumberTimeInfo    = 7003;
    const int portNumberFFT         = 7004;
    const int nbOfSamplesToSkip     = 6;
    const int timeInfoCycle         = 2048;       //Send the time info message every 2048 samples, that's about 50ms
    
    udp_client udpClientTimeInfo;
    udp_client udpClientSignalLevel;
    udp_client udpClientImpulse;
    udp_client udpClientFFT;
    
    char* dataArrayTimeInfo;
    char* dataArrayImpulse;
    char* dataArrayLevel;
    char* dataArrayFFT;
    
    //==============================================================================
    // Small optimisation : always use the same SignalMessages objects, it saves creating a new one every time
    Impulse impulse;
    SignalLevel signal;
    TimeInfo timeInfo;
    FFT fft;
    
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalProcessorAudioProcessor)
    
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
