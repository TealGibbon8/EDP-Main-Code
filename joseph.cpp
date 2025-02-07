#include "mbed.h"
#include "TextLCD.h"
#include <iostream>
#include <vector>

AnalogIn Ain(p15);
AnalogOut Aout(p18);
TextLCD lcd(p26, p25, p24, p23, p22, p21, TextLCD::LCD16x2);
float minValue = 1.0;
float maxValue = 0.0;
std::vector<float> samples(5, 0.0);
std::vector<float> Asamples(20, 0.0);
Ticker TimerInt;


void sample() {
    float value = Ain.read(); // Read the analog value (0.0 to 1.0)

    samples.insert(samples.begin(), value);

    // Remove the last object from the vector
    samples.pop_back();
}




float averaging(){

    float sum = 0.0;

    // Iterate through the vector and add each element to the sum
    for (float value : samples) {
        sum += value;
    }
    float Average = sum/Asamples.size();
    return Average;
}

float smoothing(){
    float a = 0.75;
    float lastValue = 0.0;
    float smoothValue = a * samples[0] + (1 - a) * lastValue;
    lastValue = smoothValue;
    return smoothValue;
}

// float digitalise(){
//     int digitalSignal

//     return digitalSignal
// }

void resetPk() {
    minValue = 1.0;
    maxValue = 0;  
}


int main() {
    //sampler.attach(sample, 10ms); // Sample every 1 millisecond (adjust as needed)
    TimerInt.attach(&resetPk, 1s); // setup ticker to call flip at 1Hz

    while (true) {
        float Areal = Ain.read()*3.3;

        sample();
        //float finalWave = smoothing() - averaging() + 0.5;
        float finalWave = smoothing();

        if (finalWave < minValue) {minValue = finalWave;}
        if (finalWave > maxValue) {maxValue = finalWave;}

        ThisThread::sleep_for(2ms);

        float PkPk = maxValue - minValue;
        int digiOut = 0;
        float digiwaveOut;
        if (finalWave < minValue+(PkPk*1/8)){
            digiwaveOut=minValue;
            digiOut = 0;
        }
        else if (finalWave > minValue+(PkPk*1/8) && finalWave <= minValue+(PkPk*2/8)){
            digiwaveOut=minValue+(PkPk*1/7);
            digiOut = 1;
        }
        else if (finalWave > minValue+(PkPk*2/8) && finalWave <= minValue+(PkPk*3/8)){
            digiwaveOut=minValue+(PkPk*2/7);
            digiOut = 2;
        }
        else if (finalWave > minValue+(PkPk*3/8) && finalWave <= minValue+(PkPk*4/8)){
            digiwaveOut=minValue+(PkPk*3/7);
            digiOut = 3;
        }
        else if (finalWave > minValue+(PkPk*4/8) && finalWave <= minValue+(PkPk*5/8)){
            digiwaveOut=minValue+(PkPk*4/7);
            digiOut = 4;
        }
        else if (finalWave > minValue+(PkPk*5/8) && finalWave <= minValue+(PkPk*6/8)){
            digiwaveOut=minValue+(PkPk*5/7);
            digiOut = 5;
        }
        else if (finalWave > minValue+(PkPk*6/8) && finalWave <= minValue+(PkPk*7/8)){
            digiwaveOut=minValue+(PkPk*6/7);
            digiOut = 6;
        }
        else if (finalWave > minValue+(PkPk*7/8)){
            digiwaveOut=minValue+(PkPk);
            digiOut = 7;
        }
        Aout = digiwaveOut;


        lcd.printf("HB v=%.2f V\n", digiwaveOut); //prints prints input voltage
    }
}