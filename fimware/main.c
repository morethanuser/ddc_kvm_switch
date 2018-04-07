/****************************************************************************/
/* Author: hardware.coder@gmail.com                                         */
/* https://github.com/morethanuser/ddc_kvm_switch                           */
/* http://morethanuser.blogspot.com                                         */
/****************************************************************************/
#include "main.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

// lines to monior DDC
#define SDA_LEFT  PB2
#define SDA_RIGHT PB1
#define SCL       PB0

// IR led drivers
#define LEFT  PB3
#define RIGHT PB4

// ADC threshold
#define THRESHOLD 0x1F

// payloads
const uint8_t analog[] = {0x6E, 0x51, 0x84, 0x03, 0x60, 0x00, 0x01, 0xd9};
const uint8_t dvi[]    = {0x6E, 0x51, 0x84, 0x03, 0x60, 0x00, 0x03, 0xdb};
const uint8_t hdmi[]   = {0x6E, 0x51, 0x84, 0x03, 0x60, 0x00, 0x11, 0xc9};

// length of above commands
#define CMD_LENGTH 8


/****************************************************************************/
/* Function for bit-banging byte data on two I2C SDA lines at the same time.*/
/****************************************************************************/
void byte(uint8_t left, uint8_t right) {
   uint8_t i=7;

   H(DDRB, SDA_LEFT);
   H(DDRB, SDA_RIGHT);

   do {
      if (left & (1 << i)) {
         H(PORTB, SDA_LEFT);
      } else {
         L(PORTB, SDA_LEFT);
      }

      if (right & (1 << i)) {
         H(PORTB, SDA_RIGHT);
      } else {
         L(PORTB, SDA_RIGHT);
      }

      _delay_us(5);

      H(PORTB, SCL);
      _delay_us(150);

      L(PORTB, SCL);
      _delay_us(150);
   } while(i--);

   // let slaves pull down SDA
   L(DDRB, SDA_LEFT);
   L(DDRB, SDA_RIGHT);

   // wait for reply
   _delay_us(50);

   // last clk tick
   H(PORTB, SCL);
   _delay_us(150);
   L(PORTB, SCL);

   _delay_ms(1);
   _delay_us(200);
}


/****************************************************************************/
/* Function for sending commands to two monitors at same time.              */
/* One restriction is that length of left and rigth command buffer needs to */
/* be same length.                                                          */
/****************************************************************************/
void cmd(const uint8_t *left, const uint8_t *right, const uint8_t length) {
   uint8_t i = 0;

   H(DDRB, SDA_LEFT);
   H(DDRB, SDA_RIGHT);
   H(DDRB, SCL);

   H(PORTB, SDA_LEFT);
   H(PORTB, SDA_RIGHT);
   H(PORTB, SCL);

   _delay_ms(10);

   // start condition
   L(PORTB, SDA_LEFT);
   L(PORTB, SDA_RIGHT);
   _delay_us(100);

   L(PORTB, SCL);
   _delay_us(150);

   H(PORTB, SDA_LEFT);
   H(PORTB, SDA_RIGHT);

   _delay_ms(2);

   // strat clock data
   for(i=0; i < length; i++)
      byte(left[i], right[i]);

   H(DDRB, SDA_LEFT);
   H(DDRB, SDA_RIGHT);

   // stop condition
   _delay_us(300);
   L(PORTB, SDA_LEFT);
   L(PORTB, SDA_RIGHT);
   _delay_us(150);
   H(PORTB, SCL);
   _delay_us(150);
   H(PORTB, SDA_LEFT);
   H(PORTB, SDA_RIGHT);

   _delay_ms(10);
   L(DDRB, SDA_LEFT);
   L(DDRB, SDA_RIGHT);
   L(DDRB, SCL);
}


/****************************************************************************/
/* General function for getting ADC data.                                   */
/****************************************************************************/
uint16_t get() {

   uint16_t res1 = 0;
   uint16_t res2 = 0;
   uint16_t res3 = 0;

   H(ADCSRA, ADSC);
   while(IS(ADCSRA, ADSC));
   res1 = ADCW;

   H(ADCSRA, ADSC);
   while(IS(ADCSRA, ADSC));
   res2 = ADCW;

   H(ADCSRA, ADSC);
   while(IS(ADCSRA, ADSC));
   res3 = ADCW;

   return (res1 + res2 + res3) / 3;
}


/****************************************************************************/
/* Main with infinite loop.                                                 */
/****************************************************************************/
int main(void) {

   // ADC reference voltage is VCC
   L(ADMUX, REFS0);
   L(ADMUX, REFS1);

   L(ADMUX, ADLAR);
   H(ADCSRA, ADEN);
   H(ADCSRA, ADPS2);

   // setup IR drivers
   H(DDRB, LEFT);
   H(DDRB, RIGHT);

   // disable IR diodes for now
   H(PORTB, LEFT);
   H(PORTB, RIGHT);

   // for detecting IR diode reflections
   uint8_t left  = 0;
   uint8_t right = 0;

   // for saving prev state
   uint8_t left_prev  = 0;
   uint8_t right_prev = 0;

   // delay used to control relay by IR diodes power cycle length
   uint8_t delay = 1;

   uint16_t adc_val = 0;

   while(1) {

      // get base for adc change detection
      adc_val = get();

      // check if left IR reflection hit PIN diode
      L(PORTB, LEFT);
      _delay_us(500);
      if (abs(adc_val - get()) > THRESHOLD)
         left = 1;
      else
         left = 0;
      H(PORTB, LEFT);

      _delay_ms(2);

      // if delay is true, sleep more - feed less power to relay
      if(delay)
         _delay_ms(50);

      // check if right IR reflection hit PIN diode
      L(PORTB, RIGHT);
      _delay_us(500);
      if (abs(adc_val - get()) > THRESHOLD)
         right = 1;
      else
         right = 0;
      H(PORTB, RIGHT);

      if (left != left_prev)
         if (left_prev == 1 && right == 1) {
            // send command to monitors (switch to server displays)
            cmd(dvi, dvi, CMD_LENGTH);
            // disable relay power
            delay = 1;

            // additional delay, because previous loop was shorter
            // due to delay = 0
            _delay_ms(50);
         }

      if (right != right_prev)
         if (left == 1 && right_prev == 1) {
            // send command to monitors (switch to laptop displays)
            cmd(analog, hdmi, CMD_LENGTH);
            // enable relay power
            delay = 0;
         }

      left_prev  = left;
      right_prev = right;
   }

	return 0;
}
