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

#include "../JuceLibraryCode/JuceHeader.h"  // Juce librairies : used to generate the AU/VST wrapper
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include "osc/OscOutboundPacketStream.h"    // used to output OSC
#include "ip/UdpSocket.h"                   // used to output OSC
#include "udp_client_server.h"              // used to output Protobuf binary
#include "SignalMessages.pb.h"              // protobuf messages definition
#include "math.h"
#include <time.h>                           // used to create random FFT-related functions
#include <Accelerate/Accelerate.h>          // the Accelerate headers are needed to use vDSP


#define Log2N	12u		// Base-two logarithm of number of elements.
#define	N	(1u<<Log2N)	// Number of elements (4096 for Log2N=12)

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

    //==============================================================================
    // Utility functions and variables
    float denormalize(float input);
    AudioPlayHead::CurrentPositionInfo lastPosInfo;
    
    //==============================================================================
    // Default parameter values
    const int defaultAveragingBufferSize        = 2048;
    const float defaultfftAveragingWindow       = 8;
    const bool defaultSendTimeInfo              = false;
    const bool defaultSendSignalLevel           = true;
    const bool defaultSendSignalInstantVal      = true;
    const bool defaultSendImpulse               = true;
    const bool defaultsendFFT                   = false;
    const bool defaultMonoStereo                = false;
    const bool defaultLogarithmicFFT            = true;
    const bool defaultSendBinaryUDP             = true;
    const bool defaultSendOSC                   = false;
    const float defaultInputSensitivity         = 1.0;
    const float defaultInstValGain              = 1.0;
    const float defaultInstValNbOfSamplesToSkip = 192;
    const int defaultChannel                    = 1;
    const int defaultAverageEnergyBufferSize    = 8.0;
    
    //==============================================================================
    enum Parameters
    {
        averagingBufferSizeParam = 0,
        fftAveragingWindowParam,
        inputSensitivityParam,
        sendTimeInfoParam,
        sendSignalLevelParam,
        sendSignalInstantValParam,
        sendImpulseParam,
        sendFFTParam,
        channelParam,
        monoStereoParam,
        logarithmicFFTParam,
        averageEnergyBufferSizeParam,
        sendOSCParam,
        sendBinaryUDPParam,
        instValGainParam,
        instValNbOfSamplesToSkipParam,
        totalNumParams
    };
    
    int channel;
    int averagingBufferSize;
    float fftAveragingWindow;              //Defined as float to be able to divide by its value
    float inputSensitivity;
    
    bool sendBinaryUDP        = true;
    bool sendOSC              = false;
    
    bool sendTimeInfo         = true;
    bool sendSignalLevel      = true;
    bool sendSignalInstantVal = true;
    bool sendImpulse          = true;
    bool sendFFT              = false;
    bool monoStereo           = false;         //false -> mono
    bool logarithmicFFT       = true;
    int averageEnergyBufferSize;
    
    //==============================================================================
    // Variables used by the audio algorithm
    // Used by the signal average value
    int nbBufValProcessed = 0;
    float signalSum = 0;
    // Used by the instant signal value
    int instantSigValNbOfSamplesToSkip;
    float instantSigValGain;
    int instantSigValNbOfSamplesSkipped = 0;
    // Used for beat detection
    float signalAverageEnergy = 0;
    float signalInstantEnergy = 0;
    const int thresholdFactor = 4;
    int samplesSinceLastTimeInfoTransmission = 0;                   //The time message is to be sent every timeInfoCycle (if active)

    // Set to 1.0f when a beat is detected
    float beatIntensity = 0.1f;
    
    // Used by FFT computations
    //--------------------------
    void computeFFT();                              // Execute the FFT computation sequence
    void computeLogFFT();
    float findSignalFrequency();                    // Extract the signal's fundamental frequency
    
    float* fftBuffer;                               // Buffer used to store any incoming input data
    int fftBufferIndex = 0;                         // Index where the data should be written in the temp fftBuffer
    const vDSP_Stride Stride = 1;                   // All the samples are to be processed -> stride = 1
    float *logFFTResult;                            // Array to hold the log result of the computed FFT
    const int logFFTNbOfBands = 12;                 // Number of bands to have in the logFFTResult
    DSPSplitComplex Buffer;
    DSPSplitComplex Observed;
    vDSP_DFT_Setup zop_Setup;
    vDSP_DFT_Setup zrop_Setup;

    //==============================================================================
    // Functions used to output the different available messages
    void sendImpulseMsg();
    void sendSignalLevelMsg();
    void sendSignalInstantValMsg(float val);
    void sendTimeinfoMsg();
    void sendFFTMsg();
    
    //==============================================================================
    // Socket used to forward data to the Processing application, and the variables associated with it
    const int portNumberSignalLevel      = 7001;
    const int portNumberSignalInstantVal = 7002;
    const int portNumberImpulse          = 7003;
    const int portNumberTimeInfo         = 7004;
    const int portNumberFFT              = 7005;
    const int nbOfSamplesToSkip          = 6;
    const int timeInfoCycle              = 1024;         // Send the time info message every 2048 samples, that's about 25ms
    const String udpIpAddress            = "127.0.0.1";
    
    //==============================================================================
    // OSC Functions
    osc::OutboundPacketStream* oscOutputStream;
    void sendOSC_TimeInfo();
    void sendOSC_SignalLevel();
    void sendOSC_SignalInstantVal(float val);
    void sendOSC_Impulse();
    void sendOSC_FFT();
    
    // OSC socket and output buffer
    const int portNumberOSC           = 9000;
    const int oscOutputBufferSize     = 384;            //Should be enough
    char* oscOutputBuffer;
    UdpTransmitSocket oscTransmissionSocket;
    
    // Messages used for OSC transmission
    const char* fftOSCString               = "FFT";
    const char* signalLevelOSCString       = "SIGLEVEL";
    const char* signalInstantValOSCString  = "SIGINSTVAL";
    const char* impulseOSCString           = "IMPULSE";
    const char* timeInfoOSCString          = "TIMEINFO";
    
    udp_client udpClientTimeInfo;
    udp_client udpClientSignalLevel;
    udp_client udpClientSignalInstantVal;
    udp_client udpClientImpulse;
    udp_client udpClientFFT;
    
    char* dataArrayTimeInfo;
    char* dataArrayImpulse;
    char* dataArrayLevel;
    char* dataArrayInstantVal;
    char* dataArrayLogFFT;
    char* dataArrayLinearFFT;
    
    //==============================================================================
    // Small optimisation : always use the same SignalMessages objects, it saves allocating a new one every time
    Impulse impulse;
    SignalLevel signal;
    SignalInstantVal instantVal;
    TimeInfo timeInfo;
    LinearFFT linearFft;
    LogFFT logFft;
    
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalProcessorAudioProcessor)
    
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
