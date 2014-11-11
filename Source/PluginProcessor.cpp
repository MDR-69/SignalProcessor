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

//==============================================================================
SignalProcessorAudioProcessor::SignalProcessorAudioProcessor()
: channel(defaultChannel),
  averagingBufferSize(defaultAveragingBufferSize),
  fftBandNb(defaultfftBandNb),
  inputSensitivity(defaultInputSensitivity),
  monoStereo(defaultMonoStereo),
  logarithmicFFT(defaultLogarithmicFFT),
  averageEnergyBufferSize(defaultAverageEnergyBufferSize),
  oscTransmissionSocket( IpEndpointName( "127.0.0.1", portNumberOSC )),
  udpClientTimeInfo("127.0.0.1", portNumberTimeInfo),
  udpClientSignalLevel("127.0.0.1", portNumberSignalLevel),
  udpClientImpulse("127.0.0.1", portNumberImpulse),
  udpClientFFT("127.0.0.1", portNumberFFT)
{
    // FFT-related initialization
    // Initialize the FFT data buffer - used to store the input data provided by the DAW
    fftBuffer           = (float *) malloc(sizeof(float) * N);

    // Setup the DFT routines
    zop_Setup = vDSP_DFT_zop_CreateSetup(0, N, vDSP_DFT_FORWARD);
    zrop_Setup = vDSP_DFT_zrop_CreateSetup(zop_Setup, N, vDSP_DFT_FORWARD);
    
    // Allocate memory for the arrays. Malloc is more appropriated in this case because it ensures the data is contiguous
    float *BufferMemory     = (float *) malloc(N * sizeof *BufferMemory);
    float *ObservedMemory   = (float *) malloc(N * sizeof *ObservedMemory);
    
    // Assign half of BufferMemory to reals and half to imaginaries.
    Buffer = { BufferMemory, BufferMemory + N/2 };
    
    // Assign half of ObservedMemory to reals and half to imaginaries.
    Observed = { ObservedMemory, ObservedMemory + N/2 };
    
    // Allocate memory for the FFT log buffer
    logFFTResult      = (float *) malloc(sizeof(float) * logFFTNbOfBands);
    
    //Initialize the OSC output buffer
    oscOutputBuffer   = new char[oscOutputBufferSize];
    oscOutputStream   = new osc::OutboundPacketStream(oscOutputBuffer, oscOutputBufferSize);
    
    //Build the default Signal Messages, and preallocate the char* which will receive their serialized data
    defineDefaultSignalMessages();
    
    dataArrayImpulse   = new char[impulse.ByteSize()];
    dataArrayLevel     = new char[signal.ByteSize()];
    dataArrayTimeInfo  = new char[timeInfo.ByteSize()];
    dataArrayLogFFT    = new char[logFft.ByteSize()];
    dataArrayLinearFFT = new char[linearFft.ByteSize()];
    
    lastPosInfo.resetToDefault();

}

SignalProcessorAudioProcessor::~SignalProcessorAudioProcessor()
{
    // Release all allocated memory
    delete [] dataArrayImpulse;
    delete [] dataArrayLevel;
    delete [] dataArrayTimeInfo;
    delete [] dataArrayLogFFT;
    delete [] dataArrayLinearFFT;
    delete [] oscOutputBuffer;
    delete oscOutputStream;
    
    free(fftBuffer);
    free(logFFTResult);
	vDSP_DFT_DestroySetup(zop_Setup);
	vDSP_DFT_DestroySetup(zrop_Setup);
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
        case fftBandNbParam:                return fftBandNb;
        case inputSensitivityParam:         return inputSensitivity;
        case sendTimeInfoParam:             return sendTimeInfo;
        case sendSignalLevelParam:          return sendSignalLevel;
        case sendImpulseParam:              return sendImpulse;
        case sendFFTParam:                  return sendFFT;
        case channelParam:                  return channel;
        case monoStereoParam:               return monoStereo;
        case logarithmicFFTParam:           return logarithmicFFT;
        case averageEnergyBufferSizeParam:  return averageEnergyBufferSize;
        case sendOSCParam:                  return sendOSC;
        case sendBinaryUDPParam:            return sendBinaryUDP;
        default:                            return 0.0f;
    }
}

float SignalProcessorAudioProcessor::getParameterDefaultValue (int index)
{
    switch (index)
    {
        case averagingBufferSizeParam:      return defaultAveragingBufferSize;
        case fftBandNbParam:                return defaultfftBandNb;
        case inputSensitivityParam:         return defaultInputSensitivity;
        case sendTimeInfoParam:             return defaultSendTimeInfo;
        case sendSignalLevelParam:          return defaultSendSignalLevel;
        case sendImpulseParam:              return defaultSendImpulse;
        case sendFFTParam:                  return defaultsendFFT;
        case channelParam:                  return defaultChannel;
        case monoStereoParam:               return defaultMonoStereo;
        case logarithmicFFTParam:           return defaultLogarithmicFFT;
        case averageEnergyBufferSizeParam:  return defaultAverageEnergyBufferSize;
        case sendOSCParam:                  return defaultSendOSC;
        case sendBinaryUDPParam:            return defaultSendBinaryUDP;
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
        case fftBandNbParam:                fftBandNb               = newValue;  break;
        case inputSensitivityParam:         inputSensitivity        = newValue;  break;
        case sendTimeInfoParam:             sendTimeInfo            = newValue;  break;
        case sendSignalLevelParam:          sendSignalLevel         = newValue;  break;
        case sendImpulseParam:              sendImpulse             = newValue;  break;
        case sendFFTParam:                  sendFFT                 = newValue;  break;
        case channelParam:                  channel                 = newValue;  defineSignalMessagesChannel();  break;
        case monoStereoParam:               monoStereo              = newValue;  break;
        case logarithmicFFTParam:           logarithmicFFT          = newValue;  break;
        case averageEnergyBufferSizeParam:  averageEnergyBufferSize = newValue;  break;
        case sendOSCParam:                  sendOSC                 = newValue;  break;
        case sendBinaryUDPParam:            sendBinaryUDP           = newValue;  break;
        default:                            break;
    }
}

const String SignalProcessorAudioProcessor::getParameterName (int index)
{
    switch (index)
    {
        case averagingBufferSizeParam:     return "Averaging Buffer Size"; break;
        case fftBandNbParam:               return "Number of FFT Bands";   break;
        case inputSensitivityParam:        return "Input Sensitivity";     break;
        case sendTimeInfoParam:            return "Send Time Info";        break;
        case sendSignalLevelParam:         return "Send Signal Level";     break;
        case sendImpulseParam:             return "Send Impulses";         break;
        case sendFFTParam:                 return "Send FFT Analysis";     break;
        case channelParam:                 return "Channel Number";        break;
        case monoStereoParam:              return "Mono / Stereo";         break;
        case logarithmicFFTParam:          return "Logarithmic FFT";       break;
        case averageEnergyBufferSizeParam: return "Beat Detection Window"; break;
        case sendOSCParam:                 return "Send Data Using OSC";   break;
        case sendBinaryUDPParam:           return "Send Data Using UDP";   break;
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

// A lot of optimization is to be done using vDSP functions ! For now, do it simple, then do it right
//    Vector multiply with scalar
//    vDSP_zvzsml
    

    
    // If the signal is defined by the user as mono, no need to check the second channel
    int numberOfChannels = (monoStereo==false) ? 1 : getNumInputChannels();
    for (int channel = 0; channel < numberOfChannels; channel++)
    {
        const float* channelData = buffer.getReadPointer (channel);
        
        //Only read one value out of nbOfSamplesToSkip, it's faster this way
        for (int i=0; i<buffer.getNumSamples(); i+=nbOfSamplesToSkip) {
            //The objective is to get an average of the signal's amplitude -> use the absolute value
            signalSum += std::abs(channelData[i]);
            
            //Optimization, use Mean square of vector.
            //extern void vDSP_measqv
        }
        
    }
    
    nbBufValProcessed += buffer.getNumSamples();
    samplesSinceLastTimeInfoTransmission += buffer.getNumSamples();

    if (sendFFT == true) {
        // For the FFT, only check the left channel (mono), it's not very useful the work twice. This could be changed in the future if a case where this is needed were to appear
        for (int i=0; i<buffer.getNumSamples(); i+=1) {
            // Move the available buffer in the processed data buffer (for FFT)
            *(fftBuffer + fftBufferIndex) = *(buffer.getReadPointer(0) + i);
            fftBufferIndex += 1;
            if (fftBufferIndex >= N) {
                computeFFT();
            }
        }
    }
    
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
            //Send the impulse message (which was pre-generated earlier)
            sendImpulseMsg();
        }
    }
    
    if (nbBufValProcessed >= averagingBufferSize) {
        if (sendSignalLevel == true) {
            sendSignalLevelMsg();
        }
        
        nbBufValProcessed = 0;
        signalSum = 0;
    }
    
    
    if (samplesSinceLastTimeInfoTransmission >= timeInfoCycle) {
        // Ask the host for the current time
        if (sendTimeInfo == true) {
            sendTimeinfoMsg();
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
// FFT Functions

void SignalProcessorAudioProcessor::computeFFT() {
    
    // Reinterpret the signal in fftBuffer as an interleaved-data complex vector and use vDSP_ctoz to move the data to a separated-data complex vector. The stride is equal to 2 because the imaginary elements are skipped
    vDSP_ctoz((DSPComplex *) fftBuffer, 2*Stride, &Buffer, 1, N/2);
    
    // Perform a real-to-complex DFT.
    vDSP_DFT_Execute(zrop_Setup,
                     Buffer.realp, Buffer.imagp,
                     Observed.realp, Observed.imagp);

    // Send the FFT message over the network
    sendFFTMsg();
    
    // Reinitialize fftBufferIndex to start writing the temp data back from the start of the table
    fftBufferIndex = 0;
    
    // If the FFT is set to logarithmic, transform the linear result array in a new log one
    if (logarithmicFFT) {
        computeLogFFT();
    }
    
}

float SignalProcessorAudioProcessor::findSignalFrequency() {
    
    float maxVal = 0;
    int maxValPos = 0;
    for (int i=0; i < N/2; i += 1) {
        if (maxVal < *(Observed.realp + i)) {
            maxVal = *(Observed.realp + i);
            maxValPos = i;
        }
        else {
            // This function aims to find the approximate fundamental
            // The algorithm could be improved
        }
    }
    
    if (maxVal < 0.1) {
        // No real frequency could be found (the source is silent)
        return -1;
    }
    else if (abs(*(Observed.realp + maxValPos - 1)) > abs(*(Observed.realp + maxValPos + 1))) {
        return (getSampleRate() / N) *
        (abs(maxValPos*(*(Observed.realp + maxValPos))) + abs((maxValPos-1)*(*(Observed.realp + maxValPos-1)))) /
        (abs(*(Observed.realp + maxValPos - 1)) + abs(*(Observed.realp + maxValPos))) ;            }
    else {
        return (getSampleRate() / N) *
        (abs(maxValPos*(*(Observed.realp + maxValPos))) + abs((maxValPos+1)*(*(Observed.realp + maxValPos+1)))) /
        (abs(*(Observed.realp + maxValPos + 1)) + abs(*(Observed.realp + maxValPos))) ;
    }
}

// Calculate a normalized intensity for the 12 frequency bands
void SignalProcessorAudioProcessor::computeLogFFT() {
    
    //First, reinitialize the array
    for (int i= 0; i<logFFTNbOfBands; i++)  { *(logFFTResult + i) = 0; }
    
    //Every element inside Observed.realp contains the energy for a frequency band with (getSampleRate() / N) Hz width (10.76Hz at a 44100Hz sample rate)
    *(logFFTResult + 0)         = abs(*(Observed.realp + 0));                                  //Energy in the 0 to 11 Hz band
    *(logFFTResult + 1)         = abs(*(Observed.realp + 1));                                  //Energy in the 11 to 22 Hz band
    for (int i=2;i<4;i++)       { *(logFFTResult + 2) += abs(*(Observed.realp + i)); }         //Energy in the 22 to 43 Hz band
    for (int i=4;i<8;i++)       { *(logFFTResult + 3) += abs(*(Observed.realp + i)); }         //Energy in the 43 to 86 Hz band
    for (int i=8;i<16;i++)      { *(logFFTResult + 4) += abs(*(Observed.realp + i)); }         //Energy in the 86 to 172 Hz band
    for (int i=16;i<32;i++)     { *(logFFTResult + 5) += abs(*(Observed.realp + i)); }         //Energy in the 172 to 344 Hz band
    for (int i=32;i<64;i++)     { *(logFFTResult + 6) += abs(*(Observed.realp + i)); }         //Energy in the 344 to 689 Hz band
    for (int i=64;i<128;i++)    { *(logFFTResult + 7) += abs(*(Observed.realp + i)); }         //Energy in the 689 to 1378 Hz band
    for (int i=128;i<256;i++)   { *(logFFTResult + 8) += abs(*(Observed.realp + i)); }         //Energy in the 1378 to 2756 Hz band
    for (int i=256;i<512;i++)   { *(logFFTResult + 9) += abs(*(Observed.realp + i)); }         //Energy in the 2756 to 5512 Hz band
    for (int i=512;i<1024;i++)  { *(logFFTResult + 10) += abs(*(Observed.realp + i)); }        //Energy in the 5512 to 11025 Hz band
    for (int i=1024;i<2048;i++) { *(logFFTResult + 11) += abs(*(Observed.realp + i)); }        //Energy in the 11025 to 22050 Hz band
    
    //Normalize the values
    float maxVal = 0;
    for (int j=0; j< 12; j++) {
        maxVal = std::max(*(logFFTResult + j), maxVal);
    }
    if (maxVal > 50) {
        for (int i= 0; i<logFFTNbOfBands; i++)  { *(logFFTResult + i) /= maxVal; }
    }
    else {
        for (int i= 0; i<logFFTNbOfBands; i++)  { *(logFFTResult + i) = 0; }
    }
    
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
    
    logFft.set_signalid(channel);
    logFft.set_fundamentalfreq(0.0);
    logFft.set_band1(0.0);
    logFft.set_band2(0.0);
    logFft.set_band3(0.0);
    logFft.set_band4(0.0);
    logFft.set_band5(0.0);
    logFft.set_band6(0.0);
    logFft.set_band7(0.0);
    logFft.set_band8(0.0);
    logFft.set_band9(0.0);
    logFft.set_band10(0.0);
    logFft.set_band11(0.0);
    logFft.set_band12(0.0);
    
    linearFft.set_signalid(channel);
    linearFft.set_fundamentalfreq(0.0);
    for (int i=0; i<N/2; i++) {             //Initialize the char array with the max size
        linearFft.add_data(0.0);
    }
}

void SignalProcessorAudioProcessor::defineSignalMessagesChannel() {
    
    signal.set_signalid(channel);
    logFft.set_signalid(channel);
    linearFft.set_signalid(channel);
    
    //It is possible to pre-serialize impulse messages here, as the message will never change
    impulse.set_signalid(channel);
    impulse.SerializeToArray(dataArrayImpulse, impulse.GetCachedSize());
    
}

//==============================================================================
void SignalProcessorAudioProcessor::sendImpulseMsg() {
    beatIntensity = 1.0f;
    if (sendBinaryUDP) {
        udpClientImpulse.send(dataArrayImpulse, impulse.GetCachedSize());
    }
    if (sendOSC) {
        oscOutputStream->Clear();
        *oscOutputStream << osc::BeginBundleImmediate
        << osc::BeginMessage( "IMPLS" )
        << channel << osc::EndMessage
        << osc::EndBundle;
        oscTransmissionSocket.Send( oscOutputStream->Data(), oscOutputStream->Size() );
    }
}

void SignalProcessorAudioProcessor::sendSignalLevelMsg() {
    if (sendBinaryUDP) {
        signal.set_signallevel(inputSensitivity * signalInstantEnergy);
        signal.SerializeToArray(dataArrayLevel, signal.GetCachedSize());
        udpClientSignalLevel.send(dataArrayLevel, signal.GetCachedSize());
    }
    if (sendOSC) {
        //Example of an OSC signal level message : SIGLVL1/0.23245
        oscOutputStream->Clear();
        *oscOutputStream << osc::BeginBundleImmediate
        << osc::BeginMessage( "SIGLVL" )
        << channel << "/"
        << (inputSensitivity * signalInstantEnergy) << osc::EndMessage
        << osc::EndBundle;
        oscTransmissionSocket.Send( oscOutputStream->Data(), oscOutputStream->Size() );
    }
}

void SignalProcessorAudioProcessor::sendTimeinfoMsg() {
    AudioPlayHead::CurrentPositionInfo currentTime;
    if (getPlayHead() != nullptr && getPlayHead()->getCurrentPosition (currentTime))
    {
        // Update the variable used to display the latest time in the GUI
        lastPosInfo = currentTime;
        
        // Successfully got the current time from the host, set the pulses-per-quarter-note value inside the timeInfo message
        if (sendBinaryUDP) {
            timeInfo.set_position((float)currentTime.ppqPosition);
            timeInfo.set_isplaying(currentTime.isPlaying);
            timeInfo.set_tempo((float)currentTime.bpm);
            timeInfo.SerializeToArray(dataArrayTimeInfo, timeInfo.GetCachedSize());
            udpClientTimeInfo.send(dataArrayTimeInfo, timeInfo.GetCachedSize());
        }
        if (sendOSC) {
            oscOutputStream->Clear();
            *oscOutputStream << osc::BeginBundleImmediate
            << osc::BeginMessage( "TIME" )
            << ((float)currentTime.ppqPosition) << osc::EndMessage
            << osc::BeginMessage( "BPM" )
            << ((float)currentTime.bpm) << osc::EndMessage
            << osc::EndBundle;
            oscTransmissionSocket.Send( oscOutputStream->Data(), oscOutputStream->Size() );
        }
    }
}

void SignalProcessorAudioProcessor::sendFFTMsg() {

    if (logarithmicFFT) {
        if (sendBinaryUDP) {
            logFft.set_signalid(channel);
            logFft.set_fundamentalfreq(findSignalFrequency());
            logFft.set_band1(*(logFFTResult + 0));
            logFft.set_band2(*(logFFTResult + 1));
            logFft.set_band3(*(logFFTResult + 2));
            logFft.set_band4(*(logFFTResult + 3));
            logFft.set_band5(*(logFFTResult + 4));
            logFft.set_band6(*(logFFTResult + 5));
            logFft.set_band7(*(logFFTResult + 6));
            logFft.set_band8(*(logFFTResult + 7));
            logFft.set_band9(*(logFFTResult + 8));
            logFft.set_band10(*(logFFTResult + 9));
            logFft.set_band11(*(logFFTResult + 10));
            logFft.set_band12(*(logFFTResult + 11));
            logFft.SerializeToArray(dataArrayLogFFT, logFft.GetCachedSize());
            
            udpClientFFT.send(dataArrayLogFFT, logFft.GetCachedSize());
        }
        if (sendOSC) {
            oscOutputStream->Clear();
            *oscOutputStream << osc::BeginBundleImmediate
            << osc::BeginMessage( "FFT" )
            << channel
            << (*(logFFTResult + 0))
            << (*(logFFTResult + 1))
            << (*(logFFTResult + 2))
            << (*(logFFTResult + 3))
            << (*(logFFTResult + 4))
            << (*(logFFTResult + 5))
            << (*(logFFTResult + 6))
            << (*(logFFTResult + 7))
            << (*(logFFTResult + 8))
            << (*(logFFTResult + 9))
            << (*(logFFTResult + 10))
            << (*(logFFTResult + 11))
            << osc::EndMessage
            << osc::EndBundle;
            oscTransmissionSocket.Send( oscOutputStream->Data(), oscOutputStream->Size() );
        }
    }
    // Linear FFT
    else {
        std::cout << "Trying to send a linear FFT message, TBIL\n";
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
    xml.setAttribute ("averagingBufferSize", averagingBufferSize);
    xml.setAttribute ("fftBandNb", fftBandNb);
    xml.setAttribute ("inputSensitivity", inputSensitivity);
    xml.setAttribute ("sendTimeInfo", sendTimeInfo);
    xml.setAttribute ("sendSignalLevel", sendSignalLevel);
    xml.setAttribute ("sendImpulse", sendImpulse);
    xml.setAttribute ("sendFFT", sendFFT);
    xml.setAttribute ("channel", channel);
    xml.setAttribute ("monoStereo", monoStereo);
    xml.setAttribute ("logarithmicFFT", logarithmicFFT);
    xml.setAttribute ("averageEnergyBufferSize", averageEnergyBufferSize);
    xml.setAttribute ("sendOSC", sendOSC);
    xml.setAttribute ("sendBinaryUDP", sendBinaryUDP);
    
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
        // make sure that it's actually our type of XML object
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // now pull out our parameters..
            averagingBufferSize     = xmlState->getIntAttribute  ("averagingBufferSize", averagingBufferSize);
            fftBandNb               = xmlState->getIntAttribute  ("fftBandNb", fftBandNb);
            inputSensitivity        = (float) xmlState->getDoubleAttribute ("inputSensitivity", inputSensitivity);
            sendTimeInfo            = xmlState->getBoolAttribute ("sendTimeInfo", sendTimeInfo);
            sendSignalLevel         = xmlState->getBoolAttribute ("sendSignalLevel", sendSignalLevel);
            sendImpulse             = xmlState->getBoolAttribute ("sendImpulse", sendImpulse);
            sendFFT                 = xmlState->getBoolAttribute ("sendFFT", sendFFT);
            channel                 = xmlState->getIntAttribute  ("channel", channel);
            monoStereo              = xmlState->getBoolAttribute ("monoStereo", monoStereo);
            logarithmicFFT          = xmlState->getBoolAttribute ("logarithmicFFT", logarithmicFFT);
            averageEnergyBufferSize = xmlState->getIntAttribute  ("averageEnergyBufferSize", averageEnergyBufferSize);
            sendOSC                 = xmlState->getBoolAttribute ("sendOSC", sendOSC);
            sendBinaryUDP           = xmlState->getBoolAttribute ("sendBinaryUDP", sendBinaryUDP);
        }
    }
    
    //Build the default Signal Messages, and preallocate the char* which will receive their serialized data
    defineDefaultSignalMessages();
}



//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SignalProcessorAudioProcessor();
}
