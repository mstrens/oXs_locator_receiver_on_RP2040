#pragma once

#include <ctype.h>
#include "pico/stdlib.h"

 //uint8_t * find(uint8_t * search, uint8_t in , uint16_t max); // search for first occurence of search string in "in" buffer  
//bool parseOneSequencer(); // fill a table with 7 integers for a sequencer; format is e.g. { 1 2 3 5 6 7}
bool getAllSequencers();    // get all sequencers, sequences and steps (call parseOneSequencer)
bool parseOneSequencer(); // get one sequencer and all his sequences and steps in seqdefsTemp[] and stepsTemp.
bool parseOneSequence(); // get one sequence and his steps in stepsTemp.
bool parseOneStep();      // parse one step and save parameter in stepsTemp[]
//bool parseOneStep();      // fill a table with sequence parameter and with step parameter.
void printSequencers();
void setupSequencers();

//bool getSequencers();
//bool getStepsSequencers();
void checkSequencers();
void saveSequencers(); // save all sequencers definitions


void setupGyroMixer();
void saveGyroMixer();  // save the gyro mixer collected during the learning process (mixer calibration)
void printGyro(); 
void printGyroMixer();

bool getPid(uint8_t mode);  // get all pid parameters for one mode; return true if valid; config is then updated
