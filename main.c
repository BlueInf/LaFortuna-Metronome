#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
//#include "lcd.h"
#include <ruota.h>
#include "rios.h"
#include <util/delay.h>
#include <time.h>
#include "notes.h"
#include "image.h"

#define HEIGHT LCDWIDTH
#define WIDTH LCDHEIGHT

#define TITLE_FRAME 0

volatile FLAG = 1;

int screen_state;

void titleFrame();
void init(void);

int8_t enc_delta(void);
volatile int8_t delta;

int displayFrame(int state);

volatile flag = 0;
volatile counter = 0;

volatile timePassed = 0;

int displayFrame(int state)
{
    counter++;
    flag = 1;
    return state;
}

int timerTask(int state)
{
   
    OCR1A = 1500;
	OCR3A = OCR1A;
    timePassed += 5;
    if (FLAG == 1)
    {
        display_string_xy("Flag = 1", 30, 40);
    }
    if (FLAG == 0)
    {
        display_string_xy("Flag = 0", 30, 60);
    }
    return state;
}

int main(void)
{

    screen_state = TITLE_FRAME;

    init();
    sei();

    for (;;)
    {
    OCR1A = 5500;
	OCR3A = OCR1A;
        if (flag)
        {

            cli();
            flag = 0;
            sei();

            switch (screen_state)
            {
                OCR1A = 0;
	OCR3A = OCR1A;

            case TITLE_FRAME:
                titleFrame();
                break;

            default:
                titleFrame();
                break;
            }
        }
    }
}

/* Configure I/O Ports */
void init(void)
{

    /* 8MHz clock, no prescaling (DS, p. 48) */
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    /* use OC1A (RCH) and OC3A (LCH) pin as output */
	DDRB |= _BV(PB5);
	DDRC |= _BV(PC6);

        /* 
    * clear OC1A/OC3A on compare match 
    * set OC1A/OC3A at BOTTOM, non-inverting mode
    * Fast PWM, 8bit
    */
    TCCR1A |= _BV(COM1A1) | _BV(WGM10);
    TCCR3A |= _BV(COM3A1) | _BV(WGM30);
    
    /* 
    * Fast PWM, 8bit
    * Prescaler: clk/1 = 8MHz
    * PWM frequency = 8MHz / (255 + 1) = 31.25kHz
    */
    TCCR1B |= _BV(WGM12) | _BV(CS10);
    TCCR3B |= _BV(WGM32) | _BV(CS30);
    
    /* set initial duty cycle to zero */
    OCR1A = 0;
    OCR3A = 0;




    /** Initialize the  display */
    init_lcd();
    os_init_scheduler();
    os_init_ruota();
    os_add_task(timerTask, 50, 1);
    os_add_task(displayFrame, 25, 1);
}



// -----------------------------------------------------------------------------
// --------------------------------Frames---------------------------------------
// -----------------------------------------------------------------------------

volatile int firstTitle = 1;
volatile int firstButtonClicked = 0;
void titleFrame()
{

    if (!(get_switch_state(_BV(SWN)) || get_switch_state(_BV(SWE)) || get_switch_state(_BV(SWW)) || get_switch_state(_BV(SWS))) && firstTitle)
    {
        firstTitle = 0;
        clear_screen();
        display_string_xy("This is the metronome!!!!", WIDTH / 3 - 30, HEIGHT / 2);
        display_string_xy("Double tap to set the frequency", WIDTH / 3 - 30, HEIGHT / 2 + 8);
        display_string_xy("Made by Krasimir Marinov", WIDTH / 3 - 30, HEIGHT / 2 + 16);
    }
    else
    {
        if (get_switch_state(_BV(SWN)) || get_switch_state(_BV(SWE)) || get_switch_state(_BV(SWW)) || get_switch_state(_BV(SWS)) && (firstButtonClicked == 0))
        {
            firstButtonClicked = 1;
            FLAG = 0;
            display_string_xy("button clicked", WIDTH / 3 - 30, HEIGHT / 2 + 24);
            return;
        }
        else if (get_switch_state(_BV(SWN)) || get_switch_state(_BV(SWE)) || get_switch_state(_BV(SWW)) || get_switch_state(_BV(SWS)) && (firstButtonClicked == 1))
        {
            display_string_xy("Second button clicked", WIDTH / 3 - 30, HEIGHT / 2 + 32);
            FLAG = 0;
            firstButtonClicked = 1;
            return;
        }
    }
}

const uint16_t sinewave[] = //sine 256 values
    {
        0x80, 0x83, 0x86, 0x89, 0x8c, 0x8f, 0x92, 0x95, 0x98, 0x9c, 0x9f, 0xa2, 0xa5, 0xa8, 0xab, 0xae,
        0xb0, 0xb3, 0xb6, 0xb9, 0xbc, 0xbf, 0xc1, 0xc4, 0xc7, 0xc9, 0xcc, 0xce, 0xd1, 0xd3, 0xd5, 0xd8,
        0xda, 0xdc, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xed, 0xef, 0xf0, 0xf2, 0xf3, 0xf5,
        0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfc, 0xfd, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfd, 0xfc, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7,
        0xf6, 0xf5, 0xf3, 0xf2, 0xf0, 0xef, 0xed, 0xec, 0xea, 0xe8, 0xe6, 0xe4, 0xe2, 0xe0, 0xde, 0xdc,
        0xda, 0xd8, 0xd5, 0xd3, 0xd1, 0xce, 0xcc, 0xc9, 0xc7, 0xc4, 0xc1, 0xbf, 0xbc, 0xb9, 0xb6, 0xb3,
        0xb0, 0xae, 0xab, 0xa8, 0xa5, 0xa2, 0x9f, 0x9c, 0x98, 0x95, 0x92, 0x8f, 0x8c, 0x89, 0x86, 0x83,
        0x80, 0x7c, 0x79, 0x76, 0x73, 0x70, 0x6d, 0x6a, 0x67, 0x63, 0x60, 0x5d, 0x5a, 0x57, 0x54, 0x51,
        0x4f, 0x4c, 0x49, 0x46, 0x43, 0x40, 0x3e, 0x3b, 0x38, 0x36, 0x33, 0x31, 0x2e, 0x2c, 0x2a, 0x27,
        0x25, 0x23, 0x21, 0x1f, 0x1d, 0x1b, 0x19, 0x17, 0x15, 0x13, 0x12, 0x10, 0x0f, 0x0d, 0x0c, 0x0a,
        0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0c, 0x0d, 0x0f, 0x10, 0x12, 0x13, 0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f, 0x21, 0x23,
        0x25, 0x27, 0x2a, 0x2c, 0x2e, 0x31, 0x33, 0x36, 0x38, 0x3b, 0x3e, 0x40, 0x43, 0x46, 0x49, 0x4c,
        0x4f, 0x51, 0x54, 0x57, 0x5a, 0x5d, 0x60, 0x63, 0x67, 0x6a, 0x6d, 0x70, 0x73, 0x76, 0x79, 0x7c};