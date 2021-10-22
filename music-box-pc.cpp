/******************************************/
/*
  playsaw.cpp
  by Gary P. Scavone, 2006

  This program will output sawtooth waveforms
  of different frequencies on each channel.
*/
/******************************************/

#include "RtAudio.h"
#include <iostream>
#include <cstdlib>
#include <map>
#include "Player.h"
/*
typedef char MY_TYPE;
#define FORMAT RTAUDIO_SINT8
#define SCALE  127.0
*/

typedef signed short MY_TYPE;
#define FORMAT RTAUDIO_SINT16
#define SCALE  32767.0

/*
typedef S24 MY_TYPE;
#define FORMAT RTAUDIO_SINT24
#define SCALE  8388607.0

typedef signed long MY_TYPE;
#define FORMAT RTAUDIO_SINT32
#define SCALE  2147483647.0

typedef float MY_TYPE;
#define FORMAT RTAUDIO_FLOAT32
#define SCALE  1.0

typedef double MY_TYPE;
#define FORMAT RTAUDIO_FLOAT64
#define SCALE  1.0
*/

// Platform-dependent sleep routines.
#if defined( __WINDOWS_ASIO__ ) || defined( __WINDOWS_DS__ ) || defined( __WINDOWS_WASAPI__ )
#include <windows.h>
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
#include <unistd.h>
#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

#define BASE_RATE 0.005
#define TIME   1.0


void errorCallback(RtAudioError::Type type, const std::string &errorText)
{
	// This example error handling function does exactly the same thing
	// as the embedded RtAudio::error() function.
	std::cout << "in errorCallback" << std::endl;
	if (type == RtAudioError::WARNING)
		std::cerr << '\n' << errorText << "\n\n";
	else if (type != RtAudioError::WARNING)
		throw(RtAudioError(errorText, type));
}

unsigned int channels;
RtAudio::StreamOptions options;
unsigned int frameCounter = 0;
bool checkCount = false;
unsigned int nFrames = 0;
const unsigned int callbackReturnValue = 1;


void ShowAudioDevice(void)
{
	// Create an api map.
	std::map<int, std::string> apiMap;
	apiMap[RtAudio::MACOSX_CORE] = "OS-X Core Audio";
	apiMap[RtAudio::WINDOWS_ASIO] = "Windows ASIO";
	apiMap[RtAudio::WINDOWS_DS] = "Windows Direct Sound";
	apiMap[RtAudio::WINDOWS_WASAPI] = "Windows WASAPI";
	apiMap[RtAudio::UNIX_JACK] = "Jack Client";
	apiMap[RtAudio::LINUX_ALSA] = "Linux ALSA";
	apiMap[RtAudio::LINUX_PULSE] = "Linux PulseAudio";
	apiMap[RtAudio::LINUX_OSS] = "Linux OSS";
	apiMap[RtAudio::RTAUDIO_DUMMY] = "RtAudio Dummy";

	std::vector< RtAudio::Api > apis;
	RtAudio::getCompiledApi(apis);

	std::cout << "\nRtAudio Version " << RtAudio::getVersion() << std::endl;

	std::cout << "\nCompiled APIs:\n";
	for (unsigned int i = 0; i < apis.size(); i++)
		std::cout << "  " << apiMap[apis[i]] << std::endl;

	RtAudio audio;
	RtAudio::DeviceInfo info;

	std::cout << "\nCurrent API: " << apiMap[audio.getCurrentApi()] << std::endl;

	unsigned int devices = audio.getDeviceCount();
	std::cout << "\nFound " << devices << " device(s) ...\n";

	for (unsigned int i = 0; i < devices; i++) {
		info = audio.getDeviceInfo(i);

		std::cout << "\nDevice Name = " << info.name << '\n';
		std::cout << "Device ID = " << i << '\n';
		if (info.probed == false)
			std::cout << "Probe Status = UNsuccessful\n";
		else {
			std::cout << "Probe Status = Successful\n";
			std::cout << "Output Channels = " << info.outputChannels << '\n';
			std::cout << "Input Channels = " << info.inputChannels << '\n';
			std::cout << "Duplex Channels = " << info.duplexChannels << '\n';
			if (info.isDefaultOutput) std::cout << "This is the default output device.\n";
			else std::cout << "This is NOT the default output device.\n";
			if (info.isDefaultInput) std::cout << "This is the default input device.\n";
			else std::cout << "This is NOT the default input device.\n";
			if (info.nativeFormats == 0)
				std::cout << "No natively supported data formats(?)!";
			else {
				std::cout << "Natively supported data formats:\n";
				if (info.nativeFormats & RTAUDIO_SINT8)
					std::cout << "  8-bit int\n";
				if (info.nativeFormats & RTAUDIO_SINT16)
					std::cout << "  16-bit int\n";
				if (info.nativeFormats & RTAUDIO_SINT24)
					std::cout << "  24-bit int\n";
				if (info.nativeFormats & RTAUDIO_SINT32)
					std::cout << "  32-bit int\n";
				if (info.nativeFormats & RTAUDIO_FLOAT32)
					std::cout << "  32-bit float\n";
				if (info.nativeFormats & RTAUDIO_FLOAT64)
					std::cout << "  64-bit float\n";
			}
			if (info.sampleRates.size() < 1)
				std::cout << "No supported sample rates found!";
			else {
				std::cout << "Supported sample rates = ";
				for (unsigned int j = 0; j < info.sampleRates.size(); j++)
					std::cout << info.sampleRates[j] << " ";
			}
			std::cout << std::endl;
		}
	}
}
//#define USE_INTERLEAVED
#if defined( USE_INTERLEAVED )

// Interleaved buffers
int audioRenderCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *data)
{
	unsigned int i, j;
	extern unsigned int channels;
	MY_TYPE *buffer = (MY_TYPE *)outputBuffer;
	double *lastValues = (double *)data;

	if (status)
		std::cout << "Stream underflow detected!" << std::endl;

	for (i = 0; i < nBufferFrames; i++) {
		for (j = 0; j < channels; j++) {
			*buffer++ = (MY_TYPE)(lastValues[j] * SCALE * 0.5);
			lastValues[j] += BASE_RATE * (j + 1 + (j*0.1));
			if (lastValues[j] >= 1.0) lastValues[j] -= 2.0;
		}
	}

	frameCounter += nBufferFrames;
	if (checkCount && (frameCounter >= nFrames)) return callbackReturnValue;
	return 0;
}

#else // Use non-interleaved buffers

int audioRenderCallback(void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
	double /*streamTime*/, RtAudioStreamStatus status, void *data)
{
	unsigned int i, j;
	extern unsigned int channels;
	MY_TYPE *buffer = (MY_TYPE *)outputBuffer;
	Player *pPlayer = (Player *)data;

	if (status)
		std::cout << "Stream underflow detected!" << std::endl;

	for (i = 0; i < nBufferFrames; i++) {
		Player32kProc(pPlayer);
		int32_t rawSynthOutput = pPlayer->mainSynthesizer.mixOut >> 8;
		if (rawSynthOutput < -32768)
			rawSynthOutput = -32768;
		else if (rawSynthOutput > 32767)
			rawSynthOutput = 32767;
		*buffer++ = (MY_TYPE)rawSynthOutput;
	}

	memcpy(buffer, outputBuffer, nBufferFrames * sizeof(MY_TYPE));

	PlayerProcess(pPlayer);

	return 0;
}
#endif

int main(int argc, char *argv[])
{
	unsigned int bufferFrames, fs, device = 0, offset = 0;

	ShowAudioDevice();
	RtAudio dac;
	Player player;
	if (dac.getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		exit(1);
	}

	channels = 2;
	fs = 32000;
	device = 0;
	PlayerInit(&player);



	// Let RtAudio print messages to stderr.
	dac.showWarnings(true);

	// Set our stream parameters for output only.
	bufferFrames = 64;
	RtAudio::StreamParameters oParams;
	oParams.deviceId = device;
	oParams.nChannels = channels;
	oParams.firstChannel = offset;

	if (device == 0)
		oParams.deviceId = dac.getDefaultOutputDevice();

	options.flags = RTAUDIO_HOG_DEVICE;
	options.flags |= RTAUDIO_SCHEDULE_REALTIME;
#if !defined( USE_INTERLEAVED )
	options.flags |= RTAUDIO_NONINTERLEAVED;
#endif
	try {
		dac.openStream(&oParams, NULL, FORMAT, fs, &bufferFrames, &audioRenderCallback, (void *)&player, &options, &errorCallback);
		dac.startStream();
		PlayerPlay(&player);
	}
	catch (RtAudioError& e) {
		e.printMessage();
		goto cleanup;
	}

	if (checkCount) {
		while (dac.isStreamRunning() == true) SLEEP(100);
	}
	else {
		char input;
		//std::cout << "Stream latency = " << dac.getStreamLatency() << "\n" << std::endl;
		std::cout << "\nPlaying ... press <enter> to quit (buffer size = " << bufferFrames << ").\n";
		std::cin.get(input);

		try {
			// Stop the stream
			dac.stopStream();
		}
		catch (RtAudioError& e) {
			e.printMessage();
		}
	}

cleanup:
	if (dac.isStreamOpen()) dac.closeStream();

	return 0;
}
