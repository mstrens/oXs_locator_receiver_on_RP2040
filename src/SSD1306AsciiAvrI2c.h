/* Arduino SSD1306Ascii Library
 * Copyright (C) 2015 by William Greiman
 *
 * This file is part of the Arduino SSD1306Ascii Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SSD1306Ascii Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * @file SSD1306AsciiAvrI2c.h
 * @brief Class for I2C displays using AvrI2c.
 */
#ifndef SSD1306AsciiAvrI2c_h
#define SSD1306AsciiAvrI2c_h
//#include "utility/AvrI2c.h"
#include "SSD1306Ascii.h"
/**
 * @class SSD1306AsciiAvrI2c
 * @brief Class for I2C displays on AVR.
 *
 * Uses the AvrI2c class that is smaller and faster than the
 * Wire library.
 */
class SSD1306AsciiAvrI2c : public SSD1306Ascii {
 public:
  /**
   * @brief Initialize the display controller.
   *
   * @param[in] dev A device initialization structure.
   * @param[in] i2cAddr The I2C address of the display controller.
   */
  void begin(const DevType* dev, uint8_t i2cAddr) { // dev contains the parameters for the display (col, row, ...)
    m_nData = 0;
    m_i2cAddr = i2cAddr;

    //m_i2c.begin(AVRI2C_FASTMODE);  removed by ms because already done in setup() from main.cpp
    init(dev); // in class ssd1306Ascii ; 
  }
  /**
   * @brief Initialize the display controller.
   *
   * @param[in] dev A device initialization structure.
   * @param[in] i2cAddr The I2C address of the display controller.
   * @param[in] rst The display controller reset pin.
   */
  //void begin(const DevType* dev, uint8_t i2cAddr, uint8_t rst) {
  //  
  //  begin(dev, i2cAddr);
  //}

  /**
   * @brief Set the I2C bit rate.
   *
   * @param[in] frequency Desired frequency in Hz.
   *            Valid range for a 16 MHz board is about 40 kHz to 444,000 kHz.
   */
  //void setI2cClock(uint32_t frequency) {m_i2c.setClock(frequency);}

 protected:
/*
  void writeDisplay(uint8_t b, uint8_t mode) {
    if ((m_nData && mode == SSD1306_MODE_CMD)) {
      m_i2c.stop();
      m_nData = 0;
    }
    if (m_nData == 0) {
      m_i2c.start((m_i2cAddr << 1) | I2C_WRITE);
      m_i2c.write(mode == SSD1306_MODE_CMD ? 0X00 : 0X40);
    }
    m_i2c.write(b);
    if (mode == SSD1306_MODE_RAM_BUF) {
      m_nData++;
    } else {
      m_i2c.stop();
      m_nData = 0;
    }
  }
*/
  void writeDisplay(uint8_t b, uint8_t mode) {
    if ((m_nData && mode == SSD1306_MODE_CMD)) {
        // when there are char in the buffer and we get a new command, send first the buffer
        i2c_write_blocking(I2C_PORT, m_i2cAddr, m_buffer, m_nData, false);
        m_nData = 0;
    }
    if (mode == SSD1306_MODE_CMD) {
        m_buffer[0] = 0X0; // for a command we first send 00 and then the command
        m_buffer[1] = b;
        i2c_write_blocking(I2C_PORT, m_i2cAddr, m_buffer, 2 , false);
        m_nData = 0;
    } else if (mode == SSD1306_MODE_RAM_BUF){
        if ( m_nData > 250) return;
        if ( m_nData == 0) {
            m_buffer[0] = 0x40;
            m_nData = 1;
        }
        m_buffer[m_nData++] = b;
    }
  }

 protected:
  //AvrI2c m_i2c;
  uint8_t m_i2cAddr;
  uint8_t m_nData;
  uint8_t m_buffer[1000];
  
};
#endif  // SSD1306AsciiAvrI2c_h
