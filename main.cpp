#include "mbed.h"
#include "TextLCD.h"
#include <list>

// Blinking rate in milliseconds
TextLCD lcd(p30, p29, p28, p27, p26, p25, TextLCD::LCD16x2);
AnalogIn SigIn(p17);
DigitalIn switchIn(p20);
AnalogOut SigOut(p18);
PwmOut PWM1(LED1);
SPI max72_spi(p5, NC, p7);
DigitalOut load(p8); //will provide the load signal
//DigitalIn LEDIN(p19);
DigitalOut beatLED(LED1);
Ticker reseter;
Ticker sampling;
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
int average_sample_size = 3;
int big_average_sample_size = 30;
list<float> inputs = {0.0}; //number in brackets sets number of singals to include in average
list<float> averages = {0.0};
list<float> sorted_averages = {0.0};
float av_max = 1;
float av_min = 0;
int numOfVals = 1;
float averaged = 0;
float stepSize = 0;
float minValue = 1;
float maxValue = 0;
float output = 0;
int digiOut = 0;
float heartRate = 0;
long period = 0;
int edges = 0;
long firstTime = 0;
int averages_size = 0;
bool filtered = false;
float PkPk = 0;


void FilterSignal() { //apply the first order filtering equation to the incoming analog signal
    current = alpha * signal + (1-alpha) * previous;
    previous = current;
}

void RollingAverage() {
    if(filtered) {
        numOfVals = inputs.size();
        while (numOfVals > average_sample_size) { //reduce list to sample size
            inputs.pop_back();
            numOfVals = inputs.size();
        }
        float sum = 0;
        for(float f: inputs) {
            sum += f;
        }
        averaged = sum / numOfVals;
        averages_size = averages.size();
        while (averages_size > big_average_sample_size) { //reduce list to sample size
            int back = averages.back();
            sorted_averages.remove(back);
            averages.pop_back();
            averages_size = averages.size();
        }
        filtered = false;
    }
}


void PKRst() {
    minValue = sorted_averages.back();
    maxValue = sorted_averages.front();
}

void BeatChecker(int digiOut){
    if(averaged <= minValue+(PkPk*0.2) && (averaged > averages.front()) && edges == 2) { //if rising edge detected and other edges already detected
        period = beatTime.elapsed_time().count() - firstTime;//calculate the period
        heartRate = (1.0/period) * pow(10,6) * 60; //compute the heart rate, converting from beats per microsecond to beats per minute
        edges = 0;//reset edges
        PKRst();//reset the peaks to account for shift or outliers
    }
    else if(averaged <= minValue+(PkPk*0.2) && (averaged > averages.front())) {//if an edge is detected and rising
        edges++;
        if (edges == 1) {
            firstTime = beatTime.elapsed_time().count();
        }
    }
    else if(averaged < averages.front() && averaged > minValue+(PkPk*0.8)) {//if an edge is detected and is falling
        edges++;
    }
    //check if pulse is outside of regular timing
    int relPkPk = sorted_averages.front() - sorted_averages.back();
    if(PkPk > relPkPk) {
        if(averaged <= minValue+(relPkPk*0.2) && (averaged > averages.front()) && edges == 2) { //if rising edge detected and other edges already detected
        period = beatTime.elapsed_time().count() - firstTime;//calculate the period
        heartRate = (1.0/period) * pow(10,6) * 60; //compute the heart rate, converting from beats per microsecond to beats per minute
        edges = 0;//reset edges
        PKRst();//reset the peaks to account for shift or outliers
        }
        else if(averaged <= minValue+(relPkPk*0.2) && (averaged > averages.front())) {//if an edge is detected and rising
            edges++;
            if (edges == 1) {
                firstTime = beatTime.elapsed_time().count();
            }
        }
        else if(averaged < averages.front() && averaged > minValue+(relPkPk*0.8)) {//if an edge is detected and is falling
            edges++;
        }
    }
    averages.push_front(averaged);
    sorted_averages.push_front(averaged);
    sorted_averages.sort();
}

void EightbyEightOutput(int digiout) {

}

int main()
{
    //reseter.attach(&PKRst, 10s);
    beatTime.start();
    while (true) {
        signal = SigIn.read();//take signal input
        current = alpha * signal + (1-alpha) * previous;//filter it
        previous = current;
        inputs.push_front(current);
        filtered = true;
        RollingAverage();
        if(averaged == 0) {}
        else {
            if (averaged < minValue) {minValue = averaged;}
            if (averaged > maxValue) {maxValue = averaged;}

            int digiOut = 0;
            PkPk = maxValue - minValue;

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

        //if(beatTime.elapsed_time().count()>= 10000000){PKRst();}
        if(switchIn == 1) {// if the switch is sending power, use the LCD
            PWM1 = 0;
            // lcd.printf("min  %.3f V\n", minValue*3.3);
            //lcd.printf("period %.3li us\n", period);
            //lcd.printf("(Da Dum)^2 G4\n");
            //lcd.printf("HR: %.0f BPM\n", heartRate);
            lcd.printf("averaged: %0.2f\n",averaged);
            lcd.printf("last: %0.2f\n",averages.front());
        }
        else {// if the switch is not using power, use the LED
            lcd.printf(" \n \n");
            PWM1.period_ms(period/1000.0);
            PWM1 = 0.5;
        }
        /*
        SigOut.write(averaged);
        lcd.printf("Average: %0.2f\n",(averaged));*/
        wait_us(70000);
    }
}

