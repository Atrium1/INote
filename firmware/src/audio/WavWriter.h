#pragma once

#include <Arduino.h>
#include <FS.h>

class WavWriter {
 public:
  bool open(const String& path, uint32_t sampleRate, uint16_t bits, uint16_t channels);
  bool writeSamples(const uint8_t* data, size_t len);
  bool close();
  bool isOpen() const { return open_; }
  uint32_t dataBytes() const { return dataBytes_; }

 private:
  File file_;
  bool open_ = false;
  uint32_t sampleRate_ = 16000;
  uint16_t bits_ = 16;
  uint16_t channels_ = 1;
  uint32_t dataBytes_ = 0;
  void writeHeaderPlaceholder();
  void patchHeader();
};
