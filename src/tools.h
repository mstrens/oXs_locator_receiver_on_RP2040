#pragma once

#include "pico/stdlib.h"
//#include "Button2.h" moved to Button2.cpp
//#include "stdio.h"



#define SAVE_CONFIG_ID 0XFF


int32_t int_round(int32_t n, uint32_t d);

uint32_t millisRp() ;

uint32_t microsRp();

void waitUs(uint32_t delayUs);

bool __no_inline_not_in_flash_func(get_bootsel_button)();

typedef struct
{
    uint8_t type;
    int32_t data;
} queue_entry_t;

void sent2Core0( uint8_t fieldType, int32_t value);

void enlapsedTime(uint8_t idx);
void startTimerUs(uint8_t idx);                            // start a timer to measure enlapsed time
void alarmTimerUs(uint8_t idx, uint32_t alarmExceedUs);    //  print a warning if enlapsed time exceed xx usec
void getTimerUs(uint8_t idx);                              // print always the enlapsed time

bool msgEverySec(uint8_t idx);                              // return true when more than 1 sec since previous call (per idx)

void calculateAirspeed();

void fillFields( uint8_t forcedFields);

uint16_t swapBinary(uint16_t value) ;

int16_t swapBinary(int16_t value) ;

uint32_t swapBinary(uint32_t value) ;

int32_t swapBinary(int32_t value) ;


#define DO_DEBUG
extern int debug ; // set from conf file at runtime
#ifdef DO_DEBUG
  #define debugP(fmt, ...)  do { if(debug == 1){printf((fmt), ##__VA_ARGS__);} } while (0)
  #define debugAX(txt , a, n)  do { if(debug == 1){printf((txt)); for(uint8_t i = 0; i < n; i++){\
    printf(" %2X",a[i]);} printf("\n");} } while (0)
  
#else
   #define debugP(fmt, ...){}
//   #define debugAX(fmt , uint8_t * a[], uint8_t n) {}
#define debugAX(txt , a, n) {} 
#endif
//  #define debug_print(fmt, ...)  do { if(debug == 1){plog(__FILE__, ___FUNCTION__, __LINE__, ((fmt)), ##__VA_ARGS__);} } while (0)
//  #define debug_print(fmt, ...)  do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)


