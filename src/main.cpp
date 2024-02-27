#include "pico/stdlib.h"
#include "stdio.h"
#include "config.h"
#include "hardware/i2c.h"
#include "tusb.h"
//#include "param.h"
#include "hardware/watchdog.h"
#include "tools.h"
//#include "param.h"

#include "ws2812.h"
#include "EMFButton.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "hardware/timer.h"

#include "lora_receiver.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

// pio 1  sm 3 is used for RGB led                                                                    (it uses 4 bytes, no irq and no dma)     
// LED = 16


int32_t i2cError = 0;

// to do
//     manage the led e.g. change color when lora send/receive data
//     remove use of configIsSaved

//EMFButton btn (3, 0); // button object will be associated to the boot button of rp2040; requires a special function to get the state (see tool.cpp)
                       // parameters are not used with RP2040 boot button 

CONFIG config;
bool configIsValid = false;
bool configIsSaved = true ;
bool configIsValidPrev = true;
bool blinking = true ;
uint8_t ledState = STATE_NO_SIGNAL;
uint8_t prevLedState = STATE_NO_SIGNAL;

uint32_t lastBlinkMillis;

extern uint8_t oXsGpsPdop ;    // gps precision sent by oxs
extern uint8_t oXsLastGpsDelay ; // delay since last gps fix at oxs side
extern uint8_t oXsPacketRssi ;   // RSSI of last packet received by oXs

extern uint32_t loraLastPacketReceivedMillis ;
uint32_t prevLoraLastPacketReceivedMillis ;
extern uint32_t loraLastGpsPacketReceivedMillis ;
extern int loraRxPacketRssi ;
extern float loraRxPacketSnr ;
extern int32_t lastGpsLon ; 
extern int32_t lastGpsLat ;    

// values to display on oled
uint8_t lonDegree ;
uint8_t lonMinute ;
uint8_t lonSeconde ;
uint8_t lonSecDec ;
uint8_t latDegree ;
uint8_t latMinute ;
uint8_t latSeconde ;
uint8_t latSecDec ;

SSD1306AsciiAvrI2c oled; // instance of oled display

uint8_t loraDebugCode ;
uint8_t countTxToDebug = 0 ; // just to debug
uint8_t countRxToDebug = 0 ; // just to debug

char const *oXsLastGpsDelayText[8] = { "No" , "<1 s" , "<10 s" , "<1 m" ,"<10 m" , "<1 h" , "<10 h" , ">10 h" } ;


void print2Pos(uint8_t n) { // print on oled adding a space before if n is less than 10 
  if ( n < 10 ) oled.print(" "); 
  oled.print(n);
}


void convertLonLat( int32_t GPS_LatLon, uint8_t &_degree , uint8_t & _minute , uint8_t & _seconde, uint8_t & _secDec  ) {
  uint32_t GPS_LatLonAbs ;
  uint32_t minute7decimals ;
  uint32_t seconde7decimals ;
  uint32_t secondeDec7decimals ;
  //Serial.print("LonLat= ") ; Serial.println( GPS_LatLon ) ;
  GPS_LatLonAbs = ( GPS_LatLon < 0 ? - GPS_LatLon : GPS_LatLon)  ; 
  _degree = ( GPS_LatLonAbs / 10000000 )  ;                              // extract the degre without  decimal
  minute7decimals = (GPS_LatLonAbs % 10000000 ) * 60;
  _minute =  minute7decimals / 10000000 ;
  seconde7decimals = (minute7decimals % 10000000 ) * 60 ;
  _seconde = seconde7decimals / 10000000 ;
  secondeDec7decimals =  ( seconde7decimals % 10000000 ) * 10 ;
  _secDec = secondeDec7decimals / 10 ;
  //Serial.print("degree= ") ; Serial.println( (int32_t)_degree ) ;
}


// I2C use I2C_PORT
void setupI2c(){
    if ( config.pinScl == 255 || config.pinSda == 255) return; // skip if pins are not defined
    // initialize I2C     
    i2c_init( I2C_PORT, 400 * 1000);
    gpio_set_function(config.pinSda, GPIO_FUNC_I2C);
    gpio_set_function(config.pinScl, GPIO_FUNC_I2C);
    gpio_pull_up(config.pinSda);
    gpio_pull_up(config.pinScl); 
}

void setupConfig(){
    config.version = CONFIG_VERSION;
    config.pinSda = _pinSda;
    config.pinScl = _pinScl;
    config.pinSpiCs = _pinSpiCs;
    config.pinSpiSck = _pinSpiSck;
    config.pinSpiMosi = _pinSpiMosi;
    config.pinSpiMiso = _pinSpiMiso;
    #if _pinSda!=2 && _pinSda!=6 && _pinSda!=10 && _pinSda!=14 && _pinSda!=18 && _pinSda!=22 && _pinSda!=26  
    #error "Wrong GPIO for SDA"
    #endif
    #if _pinScl!=3 && _pinScl!=7 && _pinScl!=11 && _pinScl!=15 && _pinScl!=19 && _pinScl!=23 && _pinScl!=27  
    #error "Wrong GPIO for SCLA"
    #endif
    #if _pinSpiCs > 29
    #error "Wrong GPIO for CS"
    #endif
    #if _pinSpiSck != 10 && _pinSpiSck != 14 &&_pinSpiSck != 26
    #error "Wrong GPIO for SCK"
    #endif
    #if _pinSpiMosi != 11 && _pinSpiMosi != 15 &&_pinSpiMosi != 27
    #error "Wrong GPIO for MOSI"
    #endif
    #if _pinSpiMISO != 8 && _pinSpiMISO != 12 && _pinSpiMiso != 24 &&_pinSpiMiso != 28
    #error "Wrong GPIO for MISO"
    #endif
    configIsValid = true;
}

void setColorState(){    // set the colors based on the RF link
    lastBlinkMillis = millisRp(); // reset the timestamp for blinking
    switch (ledState) {
        case STATE_OK:
            setRgbColorOn(0, 10, 0); //green
            break;
        case STATE_PARTLY_OK:
            setRgbColorOn(10, 5, 0); //yellow
            break;
        case STATE_FAILSAFE:
            setRgbColorOn(0, 0, 10); //blue
            break;
        case STATE_GYRO_CAL_MIXER_NOT_DONE:
            setRgbColorOn(10, 0, 0); //red
            break;
        case STATE_GYRO_CAL_MIXER_DONE:
            setRgbColorOn(10, 5, 0); //yellow
            break;
        case STATE_GYRO_CAL_LIMIT:
            setRgbColorOn(0, 0, 10); //blue
            break;
        default:
            setRgbColorOn(10, 0, 0); //red
            break;     
    }
}

/*
enum bootButtonStates_t {NOT_ARMED, ARMED, SAVED};
bootButtonStates_t bootButtonState =  NOT_ARMED;

void handleBootButton(){ 
    // check boot button; after double click, change LED to fix blue and next HOLD within 5 sec save the current channnels as Failsafe values
    static uint32_t bootArmedMillis = 0;
    btn.tick();
    if ( ( btn.hasClicks() == 2) && ( (millisRp() - lastRcChannels ) < 100 ) ) { // double click + a recent frame exist
        bootArmedMillis = millisRp();
        bootButtonState =  ARMED;
        setRgbColorOn(0, 0, 10); //blue
        //printf("armed\n");
    } else if (bootButtonState == ARMED) {
        if (btn.hasOnlyHeld() && ( (millisRp() - lastRcChannels ) < 100 )) {     // saved when long hold + recent frame exist
            bootButtonState =  SAVED;
            setRgbColorOn(5, 5, 5);
            bootArmedMillis = millisRp();
            cpyChannelsAndSaveConfig();   // copy the channels values and save them into the config.
            //printf("saving failsafe\n");
        } else if ( (millisRp() - bootArmedMillis) > 5000) {
            bootButtonState =  NOT_ARMED;  // reset if no hold withing the 5 sec
            setColorState();               // restore colors based on RF link
            //printf("loosing armed\n");
        }
    } else if ((bootButtonState == SAVED) && ((millisRp() - bootArmedMillis) > 2000) ){
        bootButtonState =  NOT_ARMED;  // reset after 2 sec
        setColorState();               // restore colors based on RF link 
        //printf("done\n");
    }
}
*/

void fillOled() {
  static uint8_t countToDebug = 0 ; // just to debug
  //oled.clear() ;
  oled.setCursor(0,0) ;
//  oled.print("Tx= ");
//  oled.print(countTxToDebug) ;
//  oled.setCursor(0,2) ;
//  oled.print("Rx= ");
//  oled.print(countRxToDebug) ;
  
  
  if (lastGpsLon >= 0) {
    oled.print("E ");
  } else {
    oled.print("O ");
  }
  print2Pos(lonDegree) ; oled.print("  ");
  print2Pos(lonMinute) ; oled.print("' ");
  print2Pos(lonSeconde) ; oled.print(".");
  oled.print(lonSecDec); oled.print("\""); oled.clearToEOL() ;
  
  oled.setCursor(0,1) ;
  if (lastGpsLat >= 0) {
    oled.print("N ");
  } else {
    oled.print("S ");
  }
  print2Pos(latDegree) ; oled.print("  ");
  print2Pos(latMinute) ; oled.print("' ");
  print2Pos(latSeconde) ; oled.print(".");
  oled.print(latSecDec); oled.print("\"");oled.clearToEOL() ;
  
  oled.setCursor(0, 2) ; oled.print("dop "); oled.print(oXsGpsPdop); oled.print( " ") ;
  oled.setCursor(75, 2) ; oled.print("GPS "); oled.print(oXsLastGpsDelayText[oXsLastGpsDelay]); oled.clearToEOL() ;
  oled.setCursor(0, 3) ; oled.print("oXs Rssi = "); oled.print(oXsPacketRssi);  oled.clearToEOL() ;
  //oled.setCursor(0, 4) ; oled.print("Last gps rec.  "); oled.print( ( millis() - loraLastPacketReceivedMillis ) /1000); oled.print( " s") ; oled.clearToEOL() ;
  
  oled.setCursor(0, 5) ; oled.print("Last pack rec. ");
  uint32_t delayLastPacketReceived ;
  delayLastPacketReceived = millisRp() - loraLastPacketReceivedMillis ;
  if ( delayLastPacketReceived < 60000 )  {
    oled.print( delayLastPacketReceived /1000); oled.print( " s   ") ;
  } else {
    oled.print( delayLastPacketReceived /60000); oled.print( " m   ") ;
  }
  oled.setCursor(0, 7) ; oled.print("RSSI = "); oled.print(loraRxPacketRssi); oled.print( "   ") ;
  oled.setCursor(75, 7) ;
  oled.print("SNR = "); oled.print(loraRxPacketSnr); oled.clearToEOL() ;

}




void setup() {
  stdio_init_all();
  //bool clockChanged; 
  //clockChanged = set_sys_clock_khz(133000, false);
  set_sys_clock_khz(133000, false);
  #ifdef DEBUG
  uint16_t counter = 10;                      // after an upload, watchdog_cause_reboot is true.
  //if ( watchdog_caused_reboot() ) counter = 0; // avoid the UDC wait time when reboot is caused by the watchdog   
  while ( (!tud_cdc_connected()) && (counter--)) { 
  //while ( (!tud_cdc_connected()) ) { 
    sleep_ms(100);
    //toggleRgb();
    }
  sleep_ms(2000);  // in debug mode, wait a little to let USB on PC be able to display all messages
  uint8_t a1[2] = {1, 2};
  int debug = 2;
  debugAX("aa", a1 , 2);
  #endif
  
  /*
    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
    } else {
        printf("Clean boot\n");
    }
  */  
  setupConfig(); // retrieve the config parameters (crsf baudrate, voltage scale & offset, type of gps, failsafe settings)  
  setupLed();
  setRgbColorOn(0,0,10);  // switch to blue during the setup of different sensors/pio/uart

  if (configIsValid) { // continue with setup only if config is valid 
        watchdog_enable(3500, 0); // require an update once every 500 msec
        setupI2c();      // setup I2C
        oled.begin(&Adafruit128x64, I2C_ADDRESS);
        oled.setFont(System5x7);
        oled.clear();
        oled.print("Hello world!");
  } 
  setRgbColorOn(10,0,0); // set color on red (= no signal)
  // to detect end of setup
  //printf("end of set up\n");
  //while (1) {watchdog_update();};
}


#define MAIN_LOOP 0 // used to measure the elapse time in debug
void loop() {
  //debugBootButton();
    //startTimerUs(MAIN_LOOP);                            // start a timer to measure enlapsed time

    if ((configIsValid) and (configIsSaved)) {
      watchdog_update();
    }
  //alarmTimerUs(MAIN_LOOP, 1000);    //  print a warning if enlapsed time exceed xx usec

  watchdog_update();
  
  //handleUSBCmd();  // process the commands received from the USB
  tud_task();      // I suppose that this function has to be called periodicaly
  if ( configIsValidPrev != configIsValid) {
    configIsValidPrev = configIsValid;
    if ((configIsValid) and (configIsSaved)){
        blinking = true; // setRgbColorOn(0,10,0); // red , green , blue
    } else {
        blinking = false; // setRgbColorOn(10,0,0);
        setRgbOn();  
    }
  }
  //printf("before handleBootButton\n");sleep_ms(100); 
  //handleBootButton(); // check boot button; after double click, change LED to fix blue and next HOLD within 5 sec save the current channnels as Failsafe values
  //printf("after handleBootButton\n");sleep_ms(100); 
  //if (( bootButtonState == ARMED) || ( bootButtonState == SAVED)){
  //  //setRgbColorOn(0, 0, 10); //blue
  //} else if ( ledState != prevLedState){
  if ( ledState != prevLedState){
    //printf(" %d\n ",ledState);
    prevLedState = ledState;
    setColorState();     
  } else if ( blinking && (( millisRp() - lastBlinkMillis) > 300 ) ){
    toggleRgb();
    lastBlinkMillis = millisRp();
  }
  //if (get_bootsel_button()) {
  //  printf("p\n");
  //} 
  //enlapsedTime(0);
  //printf("end of loop\n");sleep_ms(100); 
  if (config.pinSpiCs != 255) {
      loraDebugCode = loraHandle();
    if ( loraDebugCode ) fillOled() ; // this for debugging
    if ( loraDebugCode == 1 ) countTxToDebug++ ;
    if ( loraDebugCode == 2 ) countRxToDebug++ ; 
    
    if ( loraLastPacketReceivedMillis != prevLoraLastPacketReceivedMillis ) { // if we receive a new packet, then print
        prevLoraLastPacketReceivedMillis = loraLastPacketReceivedMillis ;
        
        convertLonLat( lastGpsLon , lonDegree , lonMinute , lonSeconde , lonSecDec ) ; 
        //Serial.print("lonDegree back= ") ; Serial.println(lonDegree) ; 
        convertLonLat( lastGpsLat , latDegree , latMinute , latSeconde , latSecDec ) ;
        fillOled() ;
        //Serial.print( "At=") ;  Serial.print( loraLastPacketReceivedMillis ) ; Serial.print( " lastGpsAt=") ;  Serial.print( loraLastGpsPacketReceivedMillis ) ;
        //Serial.print( " pdop=") ;  Serial.print( oXsGpsPdop ) ; Serial.print( " GpsDelay=") ;  Serial.print( oXsLastGpsDelay ) ;
        //Serial.print( " oXsRSSI=") ;  Serial.print( oXsPacketRssi ) ;   
        //Serial.print( " RxRSSI=") ;  Serial.print( loraRxPacketRssi ) ; Serial.print( " RxSNR=") ;  Serial.print( loraRxPacketSnr ) ; 
        //Serial.print( " Long=") ; Serial.print( lonDegree ) ; Serial.print( "° ") ; Serial.print( lonMinute ) ; Serial.print( "min ") ; Serial.print( lonSeconde ) ; Serial.print( "sec ") ; Serial.print( lonSecDec ) ;
        //Serial.print( " Lat=") ; Serial.print( latDegree ) ; Serial.print( "° ") ; Serial.print( latMinute ) ; Serial.print( "min ") ; Serial.print( latSeconde ) ; Serial.print( "sec ") ; Serial.print( latSecDec ) ;
        //Serial.println( " ") ;
    }
  }  
}





int main(){
  setup();
  while(1) loop();
}