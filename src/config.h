#pragma once

#include <stdint.h>
#define VERSION "0.0.2"

//#define DEBUG  // force the MCU to wait for some time for the USB connection; still continue if not connected

// --------- Default Gpio's -------------

#define _pinSpiCs   15   // 0/29
#define _pinSpiSck  26   // 10, 14, 26 (for spi1)  or 2, 6, 18, 22 (for spi0)
#define _pinSpiMosi 27     // 11, 15, 27 (for spi1)  or 3, 7, 18, 23 (for spi0)
#define _pinSpiMiso 28     // 8, 12, 24, 28 (for spi1) or 0, 4, 16, 20 (for spio)

#define _pinSda  10        //2, 6, 10, 14, 18, 22, 26   
#define _pinScl  11        //3, 7, 11, 15, 19, 23, 27

#define _pinLed  16
#define _ledInverted 'N'    // set on Y if you get inverted colors

// ------------- model locator -------------
// next lines allow to select the frequency being used by the locator (in 3 bytes most, mid, less).
// It must be the same values on oXs side and on locator receiver side
#define LORA_REG_FRF_MSB                            0x06  //frequency (in steps of 61.035 Hz)
#define LORA_REG_FRF_MID                            0x07  //frequency
#define LORA_REG_FRF_LSB                            0x08  //frequency

#define SPI_PORT spi1  // do not change     
// ------------ oled ------------------------
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

#define I2C_PORT i2c1              // do not change 


// --------- Reserve for developer. ---------
#define CONFIG_VERSION 8

struct CONFIG{
    uint8_t version = CONFIG_VERSION;
    // for oled I2C
    uint8_t pinSda  ;
    uint8_t pinScl ;
    // Led
    uint8_t pinLed ; 
    uint8_t ledInverted; 
    // for Lora locator
    uint8_t pinSpiCs;
    uint8_t pinSpiSck;
    uint8_t pinSpiMosi;
    uint8_t pinSpiMiso;
};

enum LEDState{
    STATE_OK= 0,
    STATE_PARTLY_OK,
    STATE_FAILSAFE,
    STATE_NO_SIGNAL,
    STATE_GYRO_CAL_MIXER_NOT_DONE,
    STATE_GYRO_CAL_MIXER_DONE,
    STATE_GYRO_CAL_LIMIT,
};
