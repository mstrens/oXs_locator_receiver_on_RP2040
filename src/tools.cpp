#include "tools.h"
#include "pico/stdlib.h"
#include "config.h"
#include "stdio.h"
//#include "pico/util/queue.h"
//#include "pico/multicore.h"
#include "math.h"


// useful for button but moved to button2.cpp
/////////////////////////////////////////////////////////////////
// Added by Mstrens to be able to get state of boot button on rp2040
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
// Picoboard has a button attached to the flash CS pin, which the bootrom
// checks, and jumps straight to the USB bootcode if the button is pressed
// (pulling flash CS low). We can check this pin in by jumping to some code in
// SRAM (so that the XIP interface is not required), floating the flash CS
// pin, and observing whether it is pulled low.
//
// This doesn't work if others are trying to access flash at the same time,
// e.g. XIP streamer, or the other core.
/*
bool __no_inline_not_in_flash_func(get_bootsel_button)() {
    
    //return false; // to debug - to be modified
    const uint CS_PIN_INDEX = 1;
    //startTimerUs(0);
    if( multicoreIsRunning) multicore_lockout_start_blocking();
    // Must disable interrupts, as interrupt handlers may be in flash, and we
    // are about to temporarily disable flash access!
    uint32_t flags = save_and_disable_interrupts();

    // Set chip select to Hi-Z
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // Note we can't call into any sleep functions in flash right now
    //for (volatile int i = 0; i < 1000; ++i);

    // The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
    // Note the button pulls the pin *low* when pressed.
    bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));

    // Need to restore the state of chip select, else we are going to have a
    // bad time when we return to code in flash!
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);
    
    if (multicoreIsRunning) multicore_lockout_end_blocking();
    //getTimerUs(0);
    return button_state;
}
*/

int32_t int_round(int32_t n, uint32_t d)
{
    if (d <= 0) return n;
    int32_t offset;
    offset = ((n >= 0) ? d : -(int32_t) d) ;
    offset = offset >>1; 
    //printf("n=%d   d=%d   offset %d  return %d\n", n , d , offset, (n+offset)/ (int32_t)d);
    return (n + offset) / (int32_t) d;
}


uint32_t millisRp(){
    return  to_ms_since_boot( get_absolute_time());
}

uint32_t microsRp() {
    return  to_us_since_boot(get_absolute_time ());
}

void waitUs(uint32_t delayUs){
    uint32_t nowUs = microsRp();
    while (( microsRp() - nowUs) < delayUs) {microsRp();}
}

void enlapsedTime(uint8_t idx){
    static uint32_t prevTime[10] = {0};
    uint32_t currTime;
    if (idx >= sizeof(prevTime)) return ;
    currTime = microsRp() ;
    printf("Eus%d=%d\n", idx , currTime-prevTime[idx]);
    prevTime[idx]=currTime;
}

uint32_t startAtUs[10] = {0};
void startTimerUs(uint8_t idx){
    if (idx >= sizeof(startAtUs)) return ;
    startAtUs[idx] = microsRp() ;
}

void alarmTimerUs(uint8_t idx, uint32_t alarmExceedUs){
    if (idx >= sizeof(startAtUs)) return ;
    if (( microsRp()-startAtUs[idx]) > alarmExceedUs) { 
        printf("FSus %d= %d\n", idx , microsRp()-startAtUs[idx]);
    }    

}

void getTimerUs(uint8_t idx){
    if (idx >= sizeof(startAtUs)) return ;
    printf("FSus %d= %d\n", idx , microsRp()-startAtUs[idx]);
}


bool msgEverySec(uint8_t idx){  // return true when more than 1 sec
    static uint32_t prevMs[5] = {0};
    if (idx >= 5) return false;
    if ((millisRp() - prevMs[idx]) > 1000){
        prevMs[idx]= millisRp();
        return true;
    }
    return false;
}

