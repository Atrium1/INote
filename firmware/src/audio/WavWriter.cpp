#include "WavWriter.h"

#include <SD.h>

namespace {
void writeU32(File& f, uint32_t v) {
  f.write((uint8_t)(v & 0xff));
  f.write((uint8_t)((v >> 8) & 0xff));
  f.write((uint8_t)((v >> 16) & 0xff));
  f.write((uint8_t)((v >> 24) & 0xff));
}
void writeU16(File& f, uint16_t v) {
  f.write((uint8_t)(v & 0xff));
  f.write((uint8_t)((v >> 8) & 0xff));
}
}  // namespace

bool WavWriter::open(const String& path, uint32_t sampleRate, uint16_t bits, uint16_t channels) {
  close();
  file_ = SD.open(path, FILE_WRITE);
  if (!file_) return false;
  sampleRate_ = sampleRate;
  bits_ = bits;
  channels_ = channels;
  dataBytes_ = 0;
  open_ = true;
  writeHeaderPlaceholder();
  return true;
}

void WavWriter::writeHeaderPlaceholder() {
  // 44-byte header with zero sizes; patched on close
  static const char riff[4] = {'R', 'I', 'F', 'F'};
  file_.write((const uint8_t*)riff, 4);
  writeU32(file_, 0);
  static const char wave[4] = {'W', 'A', 'V', 'E'};
  file_.write((const uint8_t*)wave, 4);
  static const char fmt[4] = {'f', 'm', 't', ' '};
  file_.write((const uint8_t*)fmt, 4);
  writeU32(file_, 16);
  writeU16(file_, 1);  // PCM
  writeU16(file_, channels_);
  writeU32(file_, sampleRate_);
  const uint32_t byteRate = sampleRate_ * channels_ * (bits_ / 8);
  writeU32(file_, byteRate);
  writeU16(file_, channels_ * (bits_ / 8));
  writeU16(file_, bits_);
  static const char data[4] = {'d', 'a', 't', 'a'};
  file_.write((const uint8_t*)data, 4);
  writeU32(file_, 0);
}

void WavWriter::patchHeader() {
  if (!open_) return;
  file_.seek(4);
  writeU32(file_, 36 + dataBytes_);
  file_.seek(40);
  writeU32(file_, dataBytes_);
}

bool WavWriter::writeSamples(const uint8_t* data, size_t len) {
  if (!open_) return false;
  size_t w = file_.write(data, len);
  dataBytes_ += w;
  return w == len;
}

bool WavWriter::close() {
  if (!open_) return true;
  patchHeader();
  file_.flush();
  file_.close();
  open_ = false;
  return true;
}
