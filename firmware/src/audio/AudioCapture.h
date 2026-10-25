#pragma once

#include <Arduino.h>

namespace AudioCapture {
bool begin();

bool startRecording(const String& wavPath);
bool isRecording();
uint32_t recordingElapsedMs();
void poll();
bool stopRecording(uint32_t& outDurationMs);

// Speaker playback (WAV from SD). Call pollPlayback() while playing.
bool startPlayback(const String& wavPath);
bool isPlaying();
void pollPlayback();
void stopPlayback();

void setPaEnabled(bool on);
void setAudioPower(bool on);  // Waveshare Audio_PWR_PIN (GPIO42)
}  // namespace AudioCapture
