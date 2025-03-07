#include "mbed.h"
#include "TextLCD.h"

// Blinking rate in milliseconds
#define WAIT_TIME    500us
TextLCD lcd(p30, p29, p28, p27, p26, p25, TextLCD::LCD16x2);
AnalogIn SigIn(p17);
AnalogIn switchIn(p20);
AnalogOut SigOut(p18);
PwmOut PWM1(LED1);
SPI max72_spi(p5, NC, p7);
DigitalOut load(p8); //will provide the load signal
//DigitalIn LEDIN(p19);
//DigitalOut beatLED(LED1);

Timer beatTime;//timer to track time between heartbeats
//TextLCD lcd(REGSEL, ENABLE, MSB1, MSB2, MSB3, MSB4), TextLCD::LCD16x2);
//register select p26, LCD pin4
// Enable p25, LCD pin 6
// MSB4 p21, LCD pin 14 (Most Significant Bit 4)
// MSB3 p22, LCD pin 13
// MSB2 p23, LCD pin 12
// MSB1 P24, LCD pin 11
int c; //A variable to count with, do not use count, it is something else entirely
float signal = SigIn.read();
float previous = 0;
float current = 0;
float alpha = 0.75;
float inputs[5]; //number in brackets sets number of singals to include in average
int ValsStored = 0;
int numOfVals = 10;
float averaged = 0;
float stepSize = 0;
float minValue = 1;
float maxValue = 0;
float output = 0;
int digiOut = 0;
float heartRate = 0;
long period = 0;
int edges = 0;

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


void PKRst() {
    minValue = 1;
    maxValue = 0;
}

void LEDPulse() {
    PWM1.period_ms(period/1000.0);
}

void BeatChecker(int digiOut){
    if((digiOut == 0) && edges==3) { //if at a troph and there has already been a troph and peak, record a beat and calculate the new heartrate
        beatTime.stop();//stop the beat timer
        period = beatTime.elapsed_time().count();
        heartRate = (1.0/period) * pow(10,6) * 60; //compute the heart rate, converting from beats per microsecond to beats per minute
        edges = 0;
        PKRst();//reset the peaks to account for shift or outliers
        beatTime.start();//restart the beat timer
    }
    else if(digiOut <= 1 || digiOut >= 6) {//if an edge is detected
        edges++;
    }
    
}

void EightbyEightOutput(int digiout) {

}

int main()
{

    beatTime.start();
    while (true) {
        PWM1 = 1;
        PWM1.period_ms(1000);
        //beatLED = LEDIN;
        signal = SigIn.read();
    
        FilterSignal();
        //RollingAverage();
        averaged = current;

        if(averaged == 0) {}
        else {
            if (averaged < minValue) {minValue = averaged;}
            if (averaged > maxValue) {maxValue = averaged;}

            int digiOut = 0;
            float PkPk = maxValue - minValue;

            if (averaged < minValue+(PkPk*1/8)){
                output=minValue;
                digiOut = 0;
            }
            else if (averaged > minValue+(PkPk*1/8) && averaged <= minValue+(PkPk*2/8)){
                output=minValue+(PkPk*1/7);
                digiOut = 1;
            }
            else if (averaged > minValue+(PkPk*2/8) && averaged <= minValue+(PkPk*3/8)){
                output=minValue+(PkPk*2/7);
                digiOut = 2;
            }
            else if (averaged > minValue+(PkPk*3/8) && averaged <= minValue+(PkPk*4/8)){
                output=minValue+(PkPk*3/7);
                digiOut = 3;
            }
            else if (averaged > minValue+(PkPk*4/8) && averaged <= minValue+(PkPk*5/8)){
                output=minValue+(PkPk*4/7);
                digiOut = 4;
            }
            else if (averaged > minValue+(PkPk*5/8) && averaged <= minValue+(PkPk*6/8)){
                output=minValue+(PkPk*5/7);
                digiOut = 5;
            }
            else if (averaged > minValue+(PkPk*6/8) && averaged <= minValue+(PkPk*7/8)){
                output=minValue+(PkPk*6/7);
                digiOut = 6;
            }
            else if (averaged > minValue+(PkPk*7/8)){
                output=minValue+(PkPk);
                digiOut = 7;
            }
            SigOut.write(output);
            EightbyEightOutput(digiOut);
            BeatChecker(digiOut);
        }

        if(beatTime.elapsed_time().count()>= 10000000){PKRst();}
        // lcd.printf("min  %.3f V\n", minValue*3.3);
         lcd.printf("period %.3li V\n", period);
        //lcd.printf("(Da Dum)^2 G4\n");
        lcd.printf("HR: %.0f BPM\n", heartRate);
    }
}

