#include "mbed.h"
#include "TextLCD.h"

// Blinking rate in milliseconds
#define BLINKING_RATE     500ms
TextLCD lcd(p26, p25, p24, p23, p22, p21, TextLCD::LCD16x2);
//TextLCD lcd(REGSEL, ENABLE, MSB1, MSB2, MSB3, MSB4), TextLCD::LCD16x2);
//register select p26, LCD pin4
// Enable p25, LCD pin 6
// MSB4 p21, LCD pin 14 (Most Significant Bit 4)
// MSB3 p22, LCD pin 13
// MSB2 p23, LCD pin 12
// MSB1 P24, LCD pin 11
int c; //A variable to count with, do not use count, it is something else entirely

int main()
{
    // Initialise the digital pin LED1, LED2, LED3, LED4 as an output
    //They are connected to LPC1678 pins P1.18, P1.20, P1.21 and P1.23 no external
    //https://os.mbed.com/handbook/PwmOut#implementation-details will have PWM conflicts
    // DigitalOut led1(LED1);
    // DigitalOut led2(LED2);
    // DigitalOut led3(LED3);
    // DigitalOut led4(LED4);
    // c = 0;

    while (true) {
        /*
        led1 = !led1; // debug lights, change when changing code to show the code has updated.
        led2 = !led2;
        led3 = !led3;
        led4 = !led4;
        lcd.printf("Hello  %d \n", c); //prints hello and a number first line of display
        ++c; //increases the number
        lcd.printf("Hello  %d \n", c); // prints to the second line
        ThisThread::sleep_for(BLINKING_RATE); //waits for a time, possible to interupt?
        */
    }
}

