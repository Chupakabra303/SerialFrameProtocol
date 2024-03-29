/*
 *  SerialFrameProtocol_1_3.h
 *
 *  Created on: 05.04.2023
 *  Author: Barbot A.
 */

#if !defined(SERIALFRAMEPROTOCOL_H_)
#define SERIALFRAMEPROTOCOL_H_

#include <Arduino.h>
//#include <Stream.h>

#if !defined(FRAME_BUFF_SIZE)
  #define FRAME_BUFF_SIZE 128
#endif

//#define LRC // включить LRC вместо CRC16

#if defined(LRC)
  // #warning "LRC"
  #define CHECK_LEN 1
  class Check {
    public:
      inline void init() { tempLRC = 0; }
      inline void update(uint8_t ch) { tempLRC += ch; }
      inline uint8_t finalize() { return -tempLRC; }
    private:
      uint8_t tempLRC;
  };
#else
  // #elif defined CRC16
  // #warning "CRC16"
  #define CHECK_LEN 2
  // width=16 poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1 residue=0x0000 name="CRC-16/IBM-3740"
  static const uint16_t CRC16Table[] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
        0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
        0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
        0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
        0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
        0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
        0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
        0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
        0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
        0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
        0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
        0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
        0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
        0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
  };

  class Check {
    public:
      inline void init() { tempCRC = 0xFFFF; }
      inline void update(uint8_t ch) { tempCRC = (tempCRC << 8) ^ CRC16Table[(tempCRC >> 8) ^ ch]; }
      /* void update(uint8_t ch) {
        tempCRC ^= uint16_t(ch) << 8;
        for (int i = 0; i < 8; i++)
          tempCRC = tempCRC & 0x8000 ? (tempCRC << 1) ^ 0x1021 : tempCRC << 1;
      } */
      inline uint16_t finalize() { return tempCRC; }
    private:
      uint16_t tempCRC = 0xFFFF;
  };
#endif

class SerialFrameProtocol : public Print {
  public:
    SerialFrameProtocol(Stream &stream) : serialPort(stream) {}
    uint8_t frameBuffer[FRAME_BUFF_SIZE]{0};
    size_t frameLength;
    bool frameCompleted;
    bool frameError;

    using Print::write; // pull in write(str) and write(buf, size) from Print
    size_t write(uint8_t);
    template <typename T> size_t write(T *buffer, size_t size) { // для записи массива с размером
      return write((const uint8_t *)buffer, size);
    }
    template <typename T> size_t write(const T &val) {
      return write((const uint8_t *)&val, sizeof(val));
    }
    size_t write(const __FlashStringHelper *f) { // Экспериментальная функция, позволяет писать F("") строку через вызов write()
      return print(f);
    }
    template <typename T> size_t writeT(const T &val) {
      return write(val);
    }
    template <typename T1, typename... T> size_t writeT(const T1 &val, const T&... vals) { // для записи кортежа параметров
      write(val);
      return writeT(vals...);
    }
    inline void frameStart() { frameCheck.init(); }
    template <typename... T> void frameStart(const T&... vals); // для кортежа
    void frameEnd();
    void framedCallbackWrite(void (* customFrame)());
    void framedCallbackWrite(void (* customFrame)(SerialFrameProtocol &));
    //void framePrint(void (* customFrame)(SerialFrameProtocol &)) { frameWrite(customFrame); } // алиас
    template <typename... T> void framedPrint(const T&...);
    template <typename... T> void framedWrite(const T&...); // для массива с размером
    template <typename... T> void framedWriteT(const T&...); // для кортежа
    bool frameReceiver();
    void frameClear(bool clear = true);

/* --------------- Custom protocol API ------------------ */
    /* template <typename... T> void logPrint(T...);
    template <typename... T> void logWrite(T...); // для массива с размером
    template <typename... T> void logWriteT(T...); // для кортежа */
    uint8_t cmdReceiver();
    void cmdClear();
    uint8_t cmdQueue;
    size_t cmdLength;
    // virtual void clear(void) { usb_serial_flush_input(); }
  private:
    Stream &serialPort;
    bool escapeStart;
    Check frameReceiveCheck, frameCheck;
};


/* uint8_t SerialFrameProtocol::LRC(const uint8_t *auchMsg, size_t usDataLen) {
  uint8_t uchLRC = 0;  //  LRC char initialized 
  while (usDataLen--) uchLRC += *auchMsg++;
  return -uchLRC;
} */

template <typename... T> void SerialFrameProtocol::frameStart(const T&... vals) { // для кортежа
  frameStart();
  writeT(vals...);
}

void SerialFrameProtocol::frameEnd() {
#if CHECK_LEN == 1
  write(frameCheck.finalize());
#elif CHECK_LEN == 2
  write(uint16_t(frameCheck.finalize() << 8 | frameCheck.finalize() >> 8)); // big-endian CRC16
#endif
  serialPort.write('\n'); // raw write '\n'
}

void SerialFrameProtocol::framedCallbackWrite(void (* customFrame)()) {
  frameStart();
  customFrame();
  frameEnd();
}

void SerialFrameProtocol::framedCallbackWrite(void (* customFrame)(SerialFrameProtocol &sfp)) {
  frameStart();
  customFrame(*this);
  frameEnd();
}

template <typename... T> void SerialFrameProtocol::framedPrint(const T&... vals) {
  frameStart();
  print(vals...);
  frameEnd();
}

template <typename... T> void SerialFrameProtocol::framedWrite(const T&... vals) { // для массива с размером
  frameStart();
  write(vals...);
  frameEnd();
}

template <typename... T> void SerialFrameProtocol::framedWriteT(const T&... vals) { // для кортежа
  frameStart();
  writeT(vals...);
  frameEnd();
}

size_t SerialFrameProtocol::write(uint8_t ch) {
  size_t n = 0;
  if (ch == '\\') {
    n = serialPort.write('\\');
    n += serialPort.write('e');
  } else if (ch == '\n') {
    n = serialPort.write('\\');
    n += serialPort.write('n');
  } else {
    n = serialPort.write(ch);
  }
  frameCheck.update(ch);
  return n; 
}

bool SerialFrameProtocol::frameReceiver() {
  if (frameCompleted) 
    return true;
  while (serialPort.available()) {
    uint8_t ch = (uint8_t)serialPort.read();
    if (ch == '\n') {
      frameCompleted  = true;
      if (frameLength < (CHECK_LEN + 1) || frameLength > FRAME_BUFF_SIZE) {
        frameError = true;
      } else {
        //frameFullLRC = LRC(frameBuffer, frameLength);
        if (frameReceiveCheck.finalize() != 0)
          frameError = true;
      }
      break;
    } else if (frameLength >= FRAME_BUFF_SIZE) {
      ;
    } else if (escapeStart) {
      escapeStart = false;
      if (ch == 'e')
        ch = '\\';
      else if (ch == 'n')
        ch = '\n';
      frameBuffer[frameLength++] = ch; // если экранируется не специальный символ, поэтому пишем в буфер его же
      frameReceiveCheck.update(ch);
    } else if (ch == '\\') { // найден экранирующий символ
      escapeStart = true;
    } else {
      frameBuffer[frameLength++] = ch;
      frameReceiveCheck.update(ch);
    }
  }
  return frameCompleted;
}

void SerialFrameProtocol::frameClear(bool clear) {
  if (clear) {
    frameCompleted  = false;
    escapeStart = false;
    frameLength = 0;
    frameError = false;
    frameReceiveCheck.init();
  }
}

/* --------------- Custom protocol API ------------------ */

uint8_t SerialFrameProtocol::cmdReceiver() {
  if (frameReceiver()) {
    if (!frameError && frameLength >= (CHECK_LEN + 2) && frameBuffer[0] == 'C') {
      cmdQueue = frameBuffer[1];
      cmdLength = frameLength - CHECK_LEN; // cmdLength = frameLength; // v1.2
    }
    frameClear();
  }
  return cmdQueue;
}

void SerialFrameProtocol::cmdClear() {
  cmdQueue = 0;
  cmdLength = 0; // v1.2
}

#endif