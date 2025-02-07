#include "mbed.h"
#include "TextLCD.h"

// Blinking rate in milliseconds
#define BLINKING_RATE     1ms
TextLCD lcd(p26, p25, p24, p23, p22, p21, TextLCD::LCD16x2);
AnalogIn SigIn(p17);
AnalogOut SigOut(p18);
Ticker TimerInt;
//TextLCD lcd(REGSEL, ENABLE, MSB1, MSB2, MSB3, MSB4), TextLCD::LCD16x2);
//register select p26, LCD pin4
// Enable p25, LCD pin 6
// MSB4 p21, LCD pin 14 (Most Significant Bit 4)
// MSB3 p22, LCD pin 13
// MSB2 p23, LCD pin 12
// MSB1 P24, LCD pin 11
int c; //A variable to count with, do not use count, it is something else entirely
float signal = SigIn.read();
float previous, current;
float alpha = 0.2;
float inputs[10]; //number in brackets sets number of singals to include in average
int ValsStored;
int numOfVals = 10;
float averaged;
float stepSize = 0;
float mini = 1;
float maxi = 0;
float output;

void FilterSignal() { //apply the first order filtering equation to the incoming analog signal
    current = alpha * signal + (1-alpha) * previous;
    previous = current;
}

void RollingAverage() {
    numOfVals = sizeof(inputs);
    inputs[ValsStored] = current;
    ValsStored ++;
    if (ValsStored >= numOfVals-1) { //if required number of signals are recorded
        float sum = 0;
        for(int i = 0; i < numOfVals; i++) {
            sum += inputs[i];
        }
        averaged = sum / numOfVals;
        ValsStored = 0;
    }
}

void printer() {
    lcd.printf("input  %.3f V\n", signal*3.3);
    lcd.printf("output %.3f V\n", averaged*3.3);
}

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
    //TimerInt.attach(&printer, 5000us); // setup ticker to call flip at 50uS

    while (true) {
        signal = SigIn.read();
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
        /*
        maxi = 0;
        mini = 1;
        if (signal > maxi) {
            maxi = signal;
        }
        if (signal < mini) {
            mini = signal;
        }
        stepSize = (maxi-mini)/8;

        FilterSignal();
        RollingAverage();
        if(averaged != 0) {
            output = averaged * stepSize;
        }
        else {output = 0;}
        */
        SigOut.write(signal);
        // lcd.printf("input  %.3f V\n", signal*3.3);
        // lcd.printf("output %.3f V\n", output*3.3);
        //lcd.printf("Da Dum Da Dum\nGroup 4\n");
        ThisThread::sleep_for(100ms);
    }
}

