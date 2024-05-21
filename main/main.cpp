#include "AudioCodecs/CodecSBC.h"
#include "AudioTools.h"
#include "Communication/ESPNowStream.h"

#define SBC_SR 32000
#define SBC_CH 1
#define SBC_BPS 16

#define CD2_SR 8000
#define CD2_CH 1
#define CD2_BPS 16

ESPNowStream espNow;

// Choose input/output
#define AUDIO_IN
#ifndef AUDIO_IN
#define AUDIO_OUT
#endif

// Choose codec
#define SBC_USE
#ifndef SBC_USE
#define CD2_USE
#endif

#ifdef SBC_USE
AudioInfo info(SBC_SR, SBC_CH, SBC_BPS);
#else
AudioInfo codec2Info(CD2_SR, CD2_CH, CD2_BPS);
#endif

#if defined(AUDIO_IN) && defined(SBC_USE)
EncodedAudioStream encoder(&espNow, new SBCEncoder());
#elif defined(AUDIO_IN) && defined(CD2_USE)
EncodedAudioStream encoder(&espNow, new Codec2Encoder());
#elif defined(AUDIO_OUT) && defined(SBC_USE)
EncodedAudioStream decoder(&espNow, new SBCDecoder(256));
#elif defined(AUDIO_OUT) && defined(CD2_USE)
EncodedAudioStream decoder(&espNow, new Codec2Decoder(3200));
#endif

#if defined(AUDIO_IN)
AnalogAudioStream adc;
StreamCopy copier(encoder, adc);
#else defined(AUDIO_OUT)
AnalogAudioStream dac;
StreamCopy copier(decoder, espNow);
#endif

const char *peers[] = {"XX:XX:XX:XX:XX:XX"};
const char myMAC[] = "XX:XX:XX:XX:XX:XX";

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  auto nowConfig = espNow.defaultConfig();
  nowConfig.mac_address = myMAC;
  espNow.begin(nowConfig);
  espNow.addPeers(peers);

#if defined(AUDIO_IN)
  // start ADC
  auto adcConfig = adc.defaultConfig(RX_MODE);
  adcConfig.copyFrom(info);
  adc.begin(adcConfig);
  // start encoder
  encoder.begin(info);
  Serial.println("Sender started...");
#elif defined(AUDIO_OUT)
  // start DAC
  auto dacConfig = dac.defaultConfig(TX_MODE);
  dacConfig.copyFrom(info);
  dac.begin(dacConfig);
  // start decoder
  decoder.begin(info);
  Serial.println("Receiver started...");
#endif
}

void loop() { copier.copy(); }