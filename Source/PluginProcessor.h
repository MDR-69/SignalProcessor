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


// vDSP related definitions
// Calculate the number of elements in an array.
#define	NumberOf(a)	(sizeof (a) / sizeof *(a))
// Number of signal samples to use.
#define	SampleLength		320
// Sampling frequency (in Hz) - Valid lengths for vDSP_DFT_zrop_CreateSetup in Mac OS X 10.7 are f * 2**n, where f is 3, 5, or 15, and 5 <= n.
#define	SamplingFrequency	3266

static const double_t TwoPi = 0x3.243f6a8885a308d313198a2e03707344ap1;


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
    const int defaultAveragingBufferSize     = 2048;
    const int defaultFftBufferSize           = 4096;
    const bool defaultSendTimeInfo           = false;
    const bool defaultSendSignalLevel        = true;
    const bool defaultSendImpulse            = true;
    const bool defaultsendFFT                = false;
    const bool defaultMonoStereo             = false;        //Mono processing
    const bool defaultSendBinaryUDP          = true;
    const bool defaultSendOSC                = false;
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
        sendFFTParam,
        channelParam,
        monoStereoParam,
        averageEnergyBufferSizeParam,
        sendOSCParam,
        sendBinaryUDPParam,
        totalNumParams
    };
    
    int channel;
    int averagingBufferSize;
    int fftBufferSize;
    float inputSensitivity;
    
    bool sendBinaryUDP   = true;
    bool sendOSC         = false;
    
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
    
    // Used by FFT computations
    //--------------------------
    // Define the state for the pseudo-random number generator.
    float* fftBuffer;                                           // Buffer used to store any incoming input data
    int fftBufferIndex = 0;                                     // Index where the data should be written
    UInt32 log2N          = 10; // 1024 samples
    UInt32 N              = (1 << log2N);
    FFTSetup FFTSettings;
    COMPLEX_SPLIT FFTData;
    float * hammingWindow;
    
    //==============================================================================
    // Socket used to forward data to the Processing application, and the variables associated with it
    const int portNumberSignalLevel = 7001;
    const int portNumberImpulse     = 7002;
    const int portNumberTimeInfo    = 7003;
    const int portNumberFFT         = 7004;
    const int nbOfSamplesToSkip     = 6;
    const int timeInfoCycle         = 2048;       //Send the time info message every 2048 samples, that's about 50ms
    const String udpIpAddress       = "127.0.0.1";
    
    //==============================================================================
    // OSC Functions
    osc::OutboundPacketStream* oscOutputStream;
    void sendOSC_TimeInfo();
    void sendOSC_SignalLevel();
    void sendOSC_Impulse();
    void sendOSC_FFT();
    
    // OSC socket and output buffer
    const int portNumberOSC           = 9000;
    const int oscOutputBufferSize     = 384;            //Should be enough
    char* oscOutputBuffer;
    UdpTransmitSocket oscTransmissionSocket;
    
    // Messages used for OSC transmission
    const char* fftOSCString          = "FFT";
    const char* signalLevelOSCString  = "SIGLEVEL";
    const char* impulseOSCString      = "IMPULSE";
    const char* timeInfoOSCString     = "TIMEINFO";
    
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
