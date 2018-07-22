/*
 * Copyright (c) 2017, NXP Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * @file    finproj.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"

volatile unsigned long time_elapsed=0;

char *months[12] = {
"Jan",
"Feb",
"Mar",
"Apr",
"May",
"Jun",
"Jul",
"Aug",
"Sep",
"Oct",
"Nov",
"Dec"
};

uint8_t eomYear[14][2] = {
  {0,0},      // Begin
  {31,31},    // Jan
  {59,60},    // Feb
  {90,91},    // Mar
  {120,121},  // Apr
  {151,152},  // May
  {181,182},  // Jun
  {212,213},  // Jul
  {243,244},  // Aug
  {273,274},  // Sep
  {304,305},  // Oct
  {334,335},  // Nov
  {365,366},  // Dec
  {366,367}   // overflow
};


unsigned long pulseStartTime = 0;
unsigned long pulseEndTime = 0;
unsigned long frameEndTime = 0;
int wasMark = 0;
uint8_t framePosition = 0;
uint8_t bitPosition = 1;
char lastBit = ' ';
uint8_t errors[10] = { 1,1,1,1,1,1,1,1,1,1 };
uint8_t errIdx = 0;
uint8_t bitError = 0;
int frameError = 0;

// WWVB time variables
uint8_t wwvb_hour = 0;
uint8_t wwvb_minute = 0;
uint8_t wwvb_day = 0;
uint8_t wwvb_year = 0;

/* WWVB time format struct - acts as an overlay on wwvbRxBuffer to extract time/date data.
 * This points to a 64 bit buffer wwvbRxBuffer that the bits get inserted into as the
 * incoming data stream is received.  (Thanks to Capt.Tagon @ duinolab.blogspot.com)
 */
struct wwvbBuffer {
  unsigned long long U12       :4;  // no value, empty four bits only 60 of 64 bits used
  unsigned long long Frame     :2;  // framing
  unsigned long long Dst       :2;  // dst flags
  unsigned long long Leapsec   :1;  // leapsecond
  unsigned long long Leapyear  :1;  // leapyear
  unsigned long long U11       :1;  // no value
  unsigned long long YearOne   :4;  // year (5 -> 2005)
  unsigned long long U10       :1;  // no value
  unsigned long long YearTen   :4;  // year (5 -> 2005)
  unsigned long long U09       :1;  // no value
  unsigned long long OffVal    :4;  // offset value
  unsigned long long U08       :1;  // no value
  unsigned long long OffSign   :3;  // offset sign
  unsigned long long U07       :2;  // no value
  unsigned long long DayOne    :4;  // day ones
  unsigned long long U06       :1;  // no value
  unsigned long long DayTen    :4;  // day tens
  unsigned long long U05       :1;  // no value
  unsigned long long DayHun    :2;  // day hundreds
  unsigned long long U04       :3;  // no value
  unsigned long long HourOne   :4;  // hours ones
  unsigned long long U03       :1;  // no value
  unsigned long long HourTen   :2;  // hours tens
  unsigned long long U02       :3;  // no value
  unsigned long long MinOne    :4;  // minutes ones
  unsigned long long U01       :1;  // no value
  unsigned long long MinTen    :3;  // minutes tens
};

struct wwvbBuffer * wwvbFrame;
unsigned long long receiveBuffer;
unsigned long long lastFrameBuffer;

void processBit() {


  // determine the width of the received pulse
  uint8_t pulseWidth;
  pulseWidth=pulseEndTime - pulseStartTime;

  // Attempt to decode the pulse into an Unweighted bit (0),
  // a Weighted bit (1), or a Frame marker.

  // Pulses < 0.2 sec are an error in reception.
  if (pulseWidth < 100) {
  buffer(-2);
  bitError++;
  wasMark = 0;

  // 0.2 sec pulses are an Unweighted bit (0)
  } else if (pulseWidth < 400) {
    buffer(0);
    wasMark = 0;

  // 0.5 sec pulses are a Weighted bit (1)
  } else if (pulseWidth < 700) {
    buffer(1);
    wasMark = 0;

  // 0.8 sec pulses are a Frame Marker
  } else if (pulseWidth < 900) {

    // Two Frame Position markers in a row indicate an
    // end/beginning of frame marker
    if (wasMark) {

	 // For the display update
	 lastBit = '*';

	 // Verify that our position data jives with this being
	 // a Frame start/end marker
	 if ( (framePosition == 6) &&
	      (bitPosition == 60)  &&
              (bitError == 0)) {

           // Process a received frame
	   frameEndTime = pulseStartTime;
           lastFrameBuffer = receiveBuffer;
           logFrameError(0);


           frameError = 1;

	 // Reset the position counters
	 framePosition = 0;
	 bitPosition = 1;
	 wasMark = 0;
         bitError = 0;
	 receiveBuffer = 0;

    // Otherwise, this was just a regular frame position marker
    } else {

	 buffer(-1);
	 wasMark = 1;

    }

  // Pulses > 0.8 sec are an error in reception
  } else {
    buffer(-2);
    bitError++;
    wasMark = 0;
  }

  // Reset everything if we have more than 60 bits in the frame.  This means
  // the frame markers went missing somewhere along the line
  if (frameError == 1 || bitPosition > 60) {


        // Reset all of the frame pointers and flags
        frameError = 0;
        logFrameError(1);
        framePosition = 0;
        bitPosition = 1;
        wasMark = 0;
        bitError = 0;
        receiveBuffer = 0;
  }

}
}


void logFrameError(int err) {

  // Add a 1 to log errors to the buffer
  uint8_t add = err?1:0;
  errors[errIdx] = add;

  // and move the buffer loop-around pointer
  if (++errIdx >= 10) {
    errIdx = 0;
  }
}

uint8_t sumFrameErrors() {

  // Sum all of the values in our error buffer
  uint8_t i, rv;
  for (i=0; i< 10; i++) {
    rv += errors[i];
  }

  return rv;
}

void debugPrintFrame() {

  char time[255];
  uint8_t minTen = (uint8_t) wwvbFrame->MinTen;
  uint8_t minOne = (uint8_t) wwvbFrame->MinOne;
  uint8_t hourTen = (uint8_t) wwvbFrame->HourTen;
  uint8_t hourOne = (uint8_t) wwvbFrame->HourOne;
  uint8_t dayHun = (uint8_t) wwvbFrame->DayHun;
  uint8_t dayTen = (uint8_t) wwvbFrame->DayTen;
  uint8_t dayOne = (uint8_t) wwvbFrame->DayOne;
  uint8_t yearOne = (uint8_t) wwvbFrame->YearOne;
  uint8_t yearTen = (uint8_t) wwvbFrame->YearTen;

  uint8_t wwvb_minute = (10 * minTen) + minOne;
  uint8_t wwvb_hour = (10 * hourTen) + hourOne;
  uint8_t wwvb_day = (100 * dayHun) + (10 * dayTen) + dayOne;
  uint8_t wwvb_year = (10 * yearTen) + yearOne;

  fprintf(time, "\nFrame Decoded: %0.2i:%0.2i  %0.3i  20%0.2i\n",
          wwvb_hour, wwvb_minute, wwvb_day, wwvb_year);

}

void buffer(uint8_t bit) {

  // Process our bits
  if (bit == 0) {
    lastBit = '0';

  } else if (bit == 1) {
    lastBit = '1';

  } else if (bit == -1) {
    lastBit = 'M';
    framePosition++;

  } else if (bit == -2) {
    lastBit = 'X';
  }

  // Push the bit into the buffer.  The 0s and 1s are the only
  // ones we care about.
  if (bit < 0) { bit = 0; }
  receiveBuffer = receiveBuffer | ( (unsigned long long) bit << (64 - bitPosition) );

  // And increment the counters that keep track of where we are
  // in the frame.
  bitPosition++;
}

void incrementWwvbMinute(void) {

  // Increment the Time and Date
  if (++(wwvbFrame->MinOne) == 10) {
	  wwvbFrame->MinOne = 0;
	  wwvbFrame->MinTen++;
  }

  if (wwvbFrame->MinTen == 6) {
	  wwvbFrame->MinTen = 0;
	  wwvbFrame->HourOne++;
  }

  if (wwvbFrame->HourOne == 10) {
	  wwvbFrame->HourOne = 0;
	  wwvbFrame->HourTen++;
  }

  if ( (wwvbFrame->HourTen == 2) && (wwvbFrame->HourOne == 4) ) {
	  wwvbFrame->HourTen = 0;
	  wwvbFrame->HourOne = 0;
	  wwvbFrame->DayOne++;
  }

  if (wwvbFrame->DayOne == 10) {
	  wwvbFrame->DayOne = 0;
	  wwvbFrame->DayTen++;
  }

  if (wwvbFrame->DayTen == 10) {
	  wwvbFrame->DayTen = 0;
	  wwvbFrame->DayHun++;
  }

  if ( (wwvbFrame->DayHun == 3) &&
       (wwvbFrame->DayTen == 6) &&
       (wwvbFrame->DayOne == (6 + (uint8_t) wwvbFrame->Leapyear)) ) {
	   // Happy New Year.
	   wwvbFrame->DayHun = 0;
	   wwvbFrame->DayTen = 0;
	   wwvbFrame->DayOne = 1;
           wwvbFrame->YearOne++;
  }

  if (wwvbFrame->YearOne == 10) {
    wwvbFrame->YearOne = 0;
    wwvbFrame->YearTen++;
  }

  if (wwvbFrame->YearTen == 10) {
    wwvbFrame->YearTen = 0;
  }

}




//////////////////////////////////////////////////////////////////////////////////////////////////

void delay(int);
//dleay in milliseconds
void delay(int mil)
{
    unsigned long current_time=time_elapsed;
    while(1)
    	if((time_elapsed-current_time)>=mil)
    		break;
}

int main(void) {

  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
	/* Init FSL debug console. */
    BOARD_InitDebugConsole();


    __disable_irq();
    //Setup radio rx pin
    NVIC_DisableIRQ(PORTC_PORTD_IRQn);
    SIM->SCGC5|=SIM_SCGC5_PORTD_MASK ;
    //Rx on WWVB clock
    //Set D7 as GPIO, and get pull down resistor
    PORTD->PCR[7]|=0x103;
    //set D7 as input
    PTD->PDDR&=~(0b10000000);
    PORTD->PCR[7]&=~0xF0000;
    PORTD->PCR[7]|=0xB0000;//either edge
    NVIC_SetPriority(PORTC_PORTD_IRQn,128);
    NVIC_ClearPendingIRQ(PORTC_PORTD_IRQn);
    NVIC_EnableIRQ(PORTC_PORTD_IRQn);

    //timer0

    SIM->SOPT2|=0x01000000;
    SIM->SCGC6|=SIM_SCGC6_TPM0_MASK;//turn on timer 0
    SIM->SCGC6|=SIM_SCGC6_TPM1_MASK;//turn on timer 1//timers have same freq, but different period of overflow flags
    TPM0->SC=0;//disable it
    TPM0->SC=0x07;//prescaler, take main clock and divide by 128 0x00=1,0x01=2 0x10=4 etc
    TPM0->MOD=0x177;//vale choosen so clock has 1ms overflow
    TPM0->SC|=0x80;//or TPM_SC_TOF_MASK(0x80)//Clear overflow
    TPM0->SC|=0x40;//TPM_SC_TOIE_MASK;//timer overflow interupt enable
    TPM0->SC|=0x08;//enable timer
    NVIC->ISER[0]|=0x00020000;//IRQ 17, Itnerupt for TPM0, allows table of contents fo find it

    TPM1->SC=0x0;
    TPM1->SC=0x07;
    TPM1->MOD=0x7FFF;// half the value of the timer 0;
    TPM1->SC|=0x80;//or TPM_SC_TOF_MASK(0x80)//Clear overflow
    TPM1->SC|=0x40;
    TPM1->SC|=0x08;
    NVIC->ISER[0]|=0x00040000;//IRQ18

    __enable_irq();


    lastFrameBuffer = 0;
    receiveBuffer = 0;
    wwvbFrame = (struct wwvbBuffer *) &lastFrameBuffer;

    //P1 on Radio rx to GPIO
    PORTD->PCR[5]|=0x100;
    //P1 on Radio rx to output
    PTD->PDDR|=(0b100000);
    //P1 on Radio rx to 0
    PTD->PCOR|=0b100000;
    //P1 on Radio rx to 1
    PTD->PTOR|=0b100000;
    delay(1000);
    //P1 on Radio rx to 0
    PTD->PCOR|=0b100000;

    //Enable on motor
    PORTD->PCR[4]|=0x100;
    //motor logic 1
    PORTD->PCR[6]|=0x100;
    //Motor logic 2
    PORTD->PCR[2]|=0x100;

    PTD->PDDR|=(0b10000);
    PTD->PDDR|=(0b100);
    PTD->PDDR|=(0b1000000);

    PTD->PCOR|=(0b10000);
    PTD->PCOR|=(0b100);
    PTD->PCOR|=(0b1000000);

    PTD->PSOR|=(0b10000);
    PTD->PSOR|=(0b100);

    delay(1000);

    PTD->PSOR|=(0b10000);
    PTD->PCOR|=(0b100);
    PTD->PSOR|=(0b1000000);
	PTD->PTOR|=1<<5;
    delay(1000);
	PTD->PTOR|=1<<5;

    PTD->PSOR|=(0b10000);
    PTD->PCOR|=(0b100);
    PTD->PCOR|=(0b1000000);

    delay(1000);
    PTD->PCOR|=(0b10000);

    /* Force the counter to be placed into memory. */
    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {
    	debugPrintFrame();
    }
}


//isr
void PORTC_PORTD_IRQHandler(void){
switch(PORTD->ISFR)
{
case(0b10000000):
	PORTD->PCR[7]|=PORT_PCR_ISF_MASK;
	uint8_t signalLevel =((PTD->PDIR)&(0b10000000) );
	if (signalLevel == 0) {
		printf("low\n");
	    pulseStartTime = time_elapsed;
	  } else {
	    pulseEndTime = time_elapsed;
	    printf("high\n");
	    processBit();
	  }
break;
default:
PORTD->ISFR|=0xFFFFFFFF;
break;
}
}

void TPM0_IRQHandler(void){
time_elapsed=time_elapsed+1;
TPM0->SC|=0x80;//clear TOF

}


void TPM1_IRQHandler(void){
//green led to debug or show its working, have to enable clock and mux first. not sure why not PTD->PTOR|=1<<5
TPM1->SC|=0x80;//clear TOF
}


