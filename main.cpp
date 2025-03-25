#include "mbed.h"
#include "TextLCD.h"
#include <chrono>
#include <list>

#define max7219_reg_noop         0x00
#define max7219_reg_digit0       0x01
#define max7219_reg_digit1       0x02
#define max7219_reg_digit2       0x03
#define max7219_reg_digit3       0x04
#define max7219_reg_digit4       0x05
#define max7219_reg_digit5       0x06
#define max7219_reg_digit6       0x07
#define max7219_reg_digit7       0x08
#define max7219_reg_decodeMode   0x09
#define max7219_reg_intensity    0x0a
#define max7219_reg_scanLimit    0x0b
#define max7219_reg_shutdown     0x0c
#define max7219_reg_displayTest  0x0f

#define LOW 0
#define HIGH 1

// Blinking rate in milliseconds
TextLCD lcd(p30, p29, p28, p27, p26, p25, TextLCD::LCD16x2);
AnalogIn SigIn(p17);
DigitalIn switchIn(p20);
AnalogOut SigOut(p18);
SPI max72_spi(p5, NC, p7);
DigitalOut load(p8); //will provide the load signal
DigitalOut beatLED(LED1);
Ticker reseter;
Timer beatTime;//timer to track time between heartbeats
Timer LEDTimer;
//TextLCD lcd(REGSEL, ENABLE, MSB1, MSB2, MSB3, MSB4), TextLCD::LCD16x2);
//register select p26, LCD pin4
// Enable p25, LCD pin 6
// MSB4 p21, LCD pin 14 (Most Significant Bit 4)
// MSB3 p22, LCD pin 13
// MSB2 p23, LCD pin 12
// MSB1 P24, LCD pin 11
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
float previous_HR = 0;
int period = 0;
int edges = 0;
int firstTime = 0;
int averages_size = 0;
bool filtered = false;
float PkPk = 0;
int LEDTime = 1000;
int lastLEDTime = 0;
float minV = 1;
float maxV = 0;
bool risingEdge = false;
bool fallingEdge = false;
int sampling_time = 50000;

char values[8] = {0x01, 0x02, 0x4, 0x08, 0x10, 0x20, 0x40, 0x80};
char displayOutput[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
char defaultOutput[8] = {0x02, 0x08, 0x40, 0x80, 0x20, 0x08, 0x20, 0x02};

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
    minValue = minV;
    maxValue = maxV;
}

void BeatChecker(int digiOut){
    if((averaged <= minValue+(PkPk*0.2)) && edges == 2 && risingEdge) { //if rising edge detected and other edges already detected
        period = std::chrono::duration_cast<std::chrono::milliseconds>(beatTime.elapsed_time()).count() - firstTime;//calculate the period
        previous_HR = heartRate;
        heartRate = 60000.0/period; //compute the heart rate, converting from beats per microsecond to beats per minute
        if((heartRate >= previous_HR * 1.25 && heartRate >=60) || heartRate <= previous_HR * 0.75) {
            heartRate = previous_HR;
        }
        edges = 0;//reset edges
        risingEdge = false;
        fallingEdge = false;
        beatTime.reset();
    }
    else if((averaged <= minValue+(PkPk*0.2)) && !risingEdge) {//if an edge is detected and rising
        edges++;
        risingEdge = true;
        if (edges == 1) {
            firstTime = std::chrono::duration_cast<std::chrono::milliseconds>(beatTime.elapsed_time()).count();
        }
    }
    else if(averaged > minValue+(PkPk*0.8) && !fallingEdge) {//if an edge is detected and is falling
        edges++;
        fallingEdge = true;
    }
    averages.push_front(averaged);
    sorted_averages.push_front(averaged);
    sorted_averages.sort();
    minV = sorted_averages.back();
    maxV = sorted_averages.front();
}

void write_to_max( int reg, int col)
{
    load = LOW;            // begin
    max72_spi.write(reg);  // specify register
    max72_spi.write(col);  // put data
    load = HIGH;           // make sure data is loaded (on rising edge of LOAD/CS)
}

//writes 8 bytes to the display  
void pattern_to_display(char *testdata){
    int cdata; 
    for(int idx = 0; idx <= 7; idx++) {
        cdata = testdata[idx]; 
        write_to_max(idx+1,cdata);
    }
} 
 

void setup_dot_matrix ()
{
    // initiation of the max 7219
    // SPI setup: 8 bits, mode 0
    max72_spi.format(8, 0);
     
  
  
       max72_spi.frequency(100000); //down to 100khx easier to scope ;-)
      

    write_to_max(max7219_reg_scanLimit, 0x07);
    write_to_max(max7219_reg_decodeMode, 0x00);  // using an led matrix (not digits)
    write_to_max(max7219_reg_shutdown, 0x01);    // not in shutdown mode
    write_to_max(max7219_reg_displayTest, 0x00); // no display test
    for (int e=1; e<=8; e++) {    // empty registers, turn all LEDs off
        write_to_max(e,0);
    }
   // maxAll(max7219_reg_intensity, 0x0f & 0x0f);    // the first 0x0f is the value you can set
     write_to_max(max7219_reg_intensity,  0x08);     
 
}

void clear(){
     for (int e=1; e<=8; e++) {    // empty registers, turn all LEDs off
        write_to_max(e,0);
    }
}

void EightbyEightOutput(int digiout) {
    char value = values[digiout];
    char newOutput[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    newOutput[0] = value;
    for(int i = 1; i <= 7; i++) {
        newOutput[i] = displayOutput[i-1];
    }
    for(int i = 0; i < 8; i++) {
        displayOutput[i] = newOutput[i];
    }
    pattern_to_display(displayOutput);
}

int main()
{
    setup_dot_matrix();
    reseter.attach(&PKRst, 2s);
    LEDTimer.start();
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
            BeatChecker(digiOut);
            if(heartRate > 0) {
                EightbyEightOutput(digiOut);
            }
            else {pattern_to_display(defaultOutput);}
        }

        if(switchIn == 1) {// if the switch is sending power, use the LCD
            beatLED = 0;
            
            lcd.printf("(Da Dum)^2 G4\n");
            if(heartRate < 40) {
                lcd.printf("Finding HR\n");
            }
            else {
                lcd.printf("HR: %.0f BPM\n", heartRate);
            }
        }
        else {// if the switch is not using power, use the LED
            lcd.printf("               \n                \n");
            // lcd.printf("(Da Dum)^2 G4\n");
            // lcd.printf("period %.3i ms\n", period);
            LEDTime = period/2;
            if(LEDTimer.elapsed_time().count() >= LEDTime) {
                beatLED = !beatLED;
                LEDTimer.reset();
            }
        }
        /*if(period > 100) {
            sampling_time = period/8*1000;
        }*/
        wait_us(sampling_time);
    }
}