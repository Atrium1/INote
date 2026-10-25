#pragma once

#include <Arduino.h>

namespace AudioCapture {
bool begin();
bool startRecording(const String& wavPath);
bool isRecording();
uint32_t recordingElapsedMs();
// Call frequently while recording to drain I2S → file
void poll();
bool stopRecording(uint32_t& outDurationMs);
void setPaEnabled(bool on);  // keep false in v0.1
}  // namespace AudioCapture
