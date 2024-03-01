# openXsensor (oXs) locator receiver on RP2040 board
## This project can be used with an openXsensor to locate a Rc model that has been lost.


The model is normally connected to the handset but when the model is on the ground, the range is quite limitted. 
So if a model is lost at more than a few hundreed meters, the handset will not get any telemetry data anymore. 
oXs locator allows to use a separate connection (with LORA modules) in order to have an extended range and have a chance to find back a lost model.
This is possible because LORA modules use a lower frequency, a lower transmitting speed and a special protocol for long range.
The LORA modules are SX1276/RFM95 that are small and easily available (e.g. Aliexpress, ebay, amazon)
\
\
The principle is the following:
* You have to build 2 devices: 
    * an oXs device with the sensors you want (ideally a GPS and optionally e.g. vario, voltages, current, ...) and a SX1276/RFM95 module.
    * a "locator receiver" device composed with:
        * an RP2040 board (e.g. a RP2040 Zero)
        * a second SX1276/RFM95 module
        * a display 0.96 pouces OLED 128X64 I2C SSD1306.

* Normally:
    * the locator receiver is not in use (power off).
    * oXs is installed in the model and transmits the sensor data's over the normal RC Rx/Tx link. The SX1276 module in oXs listen to the locator receiver from time to time (it does not tranmit) 
* When a model is lost:
    * the locator receiver" is powered on. It starts sending requests to oXs.    
    * When the SX1276/RFM95 module in oXs receives a request, it replies with a small message containing the GPS coordinates and some data over the quality of the signal.
    * the display on the locator receiver shows those data's as wel as the quality of the signal received and the time enlapsed since the last received message.


Note: the range of communication between two SX1276 modules is normally several time bigger than the common RC 2.4G link.   
If oXs and locator receiver are both on the ground, it can be that there are to far away to communicate with each other.
But there are 2 ways to extend the range:
* use a directional antena on the locator receiver. The advantage of this solution is that, if you get a communication, you can use the system as a goniometer (looking at the quality of the signal) to know the direction of the lost model. This even works if you have no GPS connected to oXs. The drawback is that a directional antenna is not as small as a simple wire.
* put the locator receiver (which is still a small device: about 3X3 cm) on another model and fly over expected lost aera. In this case, the range can be more than 10 km and the chance is very high that a communication can be achieved between the 2 modules. Even if the communication is broken when the model used for searching goes back on the ground, you will know the location of the lost model because the display will still display the last received GPS coordinates.



An oXs device with a SX1276/RFM95 does not perturb the 2.4G link and consumes only a few milliAmp because it remains normally in listening mode and when sending it is just a few % of the time. So, in order to increase the reliability of the system, it is possible to power oXs with a separate 1S lipo battery of e.g. 200/500 mAh. This should allow the system to work for several hours.

Note: the locator transmitter stay in sleep mode most of the time. Once every 55 sec, it starts listening to the receiver for 5 sec. If the receiver is not powered on, the transmitter never get a request and so never sent data.
When powered on, the receiver sent a request every 1 sec. At least 55 sec later (when entering listening mode), the transmitter should get this request and then reply immediately. It will then reply to each new request (so every 1 sec). It go back to the sleep mode if it does not get a request within the 60 sec.



To build oXs, please check and use the project "oXs_on_RP2040" (on github) 


## --------- Wiring --------------------
|RP2040 (gpio/pin)|RFM95 module|Display 0.96 pouces OLED 128X64 I2C SSD1306|
|--------|-------------------|-------------------|
|15         | Chip Select| |
|26         | SCK| |
|27      | MOSI| |
|28     | MISO| |
|10| | SDA|  
|11| | SCL |
|Grnd|Grnd|Grnd|
|3.3V|Vcc|Vcc|



Within some limits, you can change the GPIO's being used in the config.h file but may not use the same gpio for 2 purposes.


RP2040 could be powered by e.g. a 1S lipo battery connected to RP2040 Vcc and Grnd pins.
If your use a power source of more than 5 Volt, then it could be that you have to add a voltage regulator (e.g. RP2040 zero board has a max input voltage of 5.5 Volt) 

## ------------------ Led -------------------
When a RP2040-Zero or RP2040-TINY is used, the firmware will handle a RGB led (internally connected to gpio16).
* when led blinks, it means that program is running; otherwise receiver is blocked and must be resetted
* color depends on the connection
    * Blue = No connection has been establish
    * Red = Connection has been establish but has been lost since more than 2 sec (display gives the enlapse time)
    * Green = Connection is established.
    
Note: some users got a RP2040-zero or RP2040-TINY where red and green colors are inverted.
If you got such a device and want to get the "normal" colors, you can change the config.h file to invert the 2 colors.

Please note that other boards do not have a RGB led on gpio16 and so this does not applies.

## --------- Software -------------------
This software has been developped using the RP2040 SDK provided by Rapsberry.

If you just want to use it, there is (in most cases) no need to install/use any tool.
* download from github the zip file containing all files and unzip them where you want.
* in your folder, there is a file named oXs_receiver.uf2; this is a compiled version of this software that can be directly uploaded and configured afterwards
* insert the USB cable in the RP2040 board
* press on the "boot" button on the RP2040 board while you insert the USB cable in your PC.
* this will enter the RP2040 in a special bootloader mode and your pc should show a new drive named RPI-RP2
* copy and paste (or drag and drop) the oXs.uf2 file to this new drive
* the file should be automatically picked up by the RP2040 bootloader and flashed
* the RPI_RP2 drive should disapear from the PC.

Developers can change the firmware, compile and flash it with VScode and Rapsberry SDK tools.  
An easy way to install those tools is to follow the tutorials provided by Rapsberry.  
In particular for Windows there is currently an installer. See : https://github.com/raspberrypi/pico-setup-windows/blob/master/docs/tutorial.md

Once the tools are installed, copy all files provided on github on you PC (keeping the same structure).  
Open VScode and then select menu "File" + item "Open Folder". Select the folder where you copied the files.  
In VScode, press CTRL+SHIFT+P and in the input line that appears, enter (select) CMake: Configure + ENTER  
This will create some files needed for the compilation.  
To compile, select the "CMake" icon on the left vertical pannel (rectangle with a triangle inside).  
Move the cursor on the line oXs [oXs.elf]; an icon that look like an open box with some dots apears; click on it.  
Compilation should start. When done a new file oXs_receiver.uf2 should be created.  
For more info on VScode and SDK look at tutorials on internet.  

Note :  the file config.h contains some #define that can easily be changed to change some advanced parameters.
