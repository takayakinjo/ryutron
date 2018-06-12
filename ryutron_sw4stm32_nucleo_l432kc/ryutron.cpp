/* ryutron sub-system
 * Copyright (c) 2018 Takaya Kinjo
 *
 */
#include "mbed.h"
#include "ryutron.h"
#include "Adafruit_PWMServoDriver.h"

#include "FATFileSystem.h"
#include "HeapBlockDevice.h"
#include <stdio.h>
#include <errno.h>


#define DEFAULT_PWM 328         // 138-328-510 (630-1520-2359)
#define PWM_FREQ 50             // actual value is 52.89
#define PULSE_LENGTH 18907.1658 // 1/PWM_FREQ(52.89) unit:uSec

HeapBlockDevice bd(128 * 512, 512);
FATFileSystem fs("fs");
Serial pc(USBTX, USBRX); // tx, rx

Adafruit_PWMServoDriver pwm_driver(PA_10, PA_9);

AnalogIn adc0(PA_0);
AnalogIn adc1(PA_1);
AnalogIn adc2(PA_3);
AnalogIn adc3(PA_4);
AnalogIn adc4(PA_5);
AnalogIn adc5(PA_6);


//float pwm[16][3][2];

float pwm[16][3][2] = {{{-15, 573}, {0, 391}, {15, 240}}, {{-15, 252}, {0, 335}, {15, 410}}, {{0, 364}, {45, 263}, {90, 127}}, {{0, 257}, {45, 364}, {90, 460}}, 
                        {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, 
                        {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, 
                        {{0, 340}, {45, 231}, {90, 144}}, {{0, 324}, {45, 414}, {90, 540}}, {{-15, 246}, {0, 328}, {15, 409}}, {{-15, 357}, {0, 337}, {15, 303}}};
                        
/*
ID:0    -15.00 > 573    0.00 > 391      15.00 > 240 
ID:1    -15.00 > 252    0.00 > 335      15.00 > 410 
ID:2    0.00 > 364      45.00 > 263     90.00 > 127 
ID:3    0.00 > 257      45.00 > 364     90.00 > 460 
ID:4    0.00 > 0        0.00 > 0        0.00 > 0 
ID:5    0.00 > 0        0.00 > 0        0.00 > 0 
ID:6    0.00 > 0        0.00 > 0        0.00 > 0 
ID:7    0.00 > 0        0.00 > 0        0.00 > 0 
ID:8    0.00 > 0        0.00 > 0        0.00 > 0 
ID:9    0.00 > 0        0.00 > 0        0.00 > 0 
ID:10   0.00 > 0        0.00 > 0        0.00 > 0 
ID:11   0.00 > 0        0.00 > 0        0.00 > 0 
ID:12   0.00 > 340      45.00 > 231     90.00 > 144 
ID:13   0.00 > 324      45.00 > 414     90.00 > 540 
ID:14   -15.00 > 246    0.00 > 338      15.00 > 409 
ID:15   -15.00 > 357    0.00 > 337      15.00 > 303 
*/

void set_servo_angle(uint8_t id, float angle) {
    float angle1, angle2;
    float pulse, pulse1, pulse2;
    
    if (angle > pwm[id][2][0]) {
        return; // out of range
    } else if (angle >= pwm[id][1][0]) {
        angle1 = pwm[id][1][0];
        angle2 = pwm[id][2][0];
        pulse1 = pwm[id][1][1];
        pulse2 = pwm[id][2][1];
    } else if (angle >= pwm[id][0][0]) {
        angle1 = pwm[id][0][0];
        angle2 = pwm[id][1][0];
        pulse1 = pwm[id][0][1];
        pulse2 = pwm[id][1][1];
    } else {
        return; // out of range
    }
    if (angle1 == angle2) return; // invalid calibration angle
    pulse = (pulse1*(angle2-angle) + pulse2*(angle-angle1))/(angle2-angle1);        
    pwm_driver.setPWM(id, 0, (uint16_t)pulse);
}

void setServoPulse(uint8_t n, float pulse) {
    float pulselength = PULSE_LENGTH;
    pulse = 4096 * pulse / pulselength;
    pc.printf("DBG:%d\r\n", (int)pulse);
    pwm_driver.setPWM(n, 0, (uint16_t)pulse);
}
 
void initServoDriver() {
    pwm_driver.begin();
    pwm_driver.setPWMFreq(PWM_FREQ);  //This dosen't work well because of uncertain clock speed. Use setPrescale().
    //pwm_driver.setPrescale(64);    //This value is decided for 10ms interval.
    //pwm_driver.setPrescale(100);    // 60Hz
    pwm_driver.setI2Cfreq(400000); //400kHz
}

void init_pwm_table() {
    for (int i=0;i<16;i++ ) {
        for (int j=0;j<3;j++ ) {
            pwm[i][j][0] = 0.0;
            pwm[i][j][1] = DEFAULT_PWM;
        }
    }
}

void init() {

    //init_pwm_table();
    
    pwm_driver.i2c_probe(); // scan I2C 
    initServoDriver();
    for (int i=0;i<16;i++) {
        set_servo_angle(i, 0.0); // initial position
        wait(0.1);
    }
}             

void show_pwm_table() {
    for (int i=0;i<16;i++ ) {
        pc.printf("ID:%d\t", i);
        for (int j=0;j<3;j++ ) {
            pc.printf("%.2f > %d \t", pwm[i][j][0], (int)pwm[i][j][1]);
        }
        pc.printf("\n\r");
    }
}

void serial_flush(void) 
{ 
    char char1 = 0; 
    while (pc.readable())
        { char1 = pc.getc(); }
    return;
}
 
void sprash_message() {
    pc.printf("\n\r+++ Ryutron subsystem v 0.0.1\n\r");
}   
void echoback() {
    while(1) {
        pc.putc(pc.getc() + 1); // echo input back to terminal
    }
}
void show_status() {
    pc.printf("System status:\n\r");
    show_pwm_table();
}
void calib_pwm(int id, int n, float angle) {
    pwm[id][n][0] = angle;

   
    char c;
    bool terminate = false;
    pc.printf(" %d      \r", (int)pwm[id][n][1]); serial_flush();
    while(!terminate) {
        c = pc.getc();
        //pc.printf("DBG:%d\n\r", (int)c);
 
        switch(c) {
            case 44: // dec
                pwm[id][n][1]  -= (float)1.0;
                wait(0.05);
                break;
            case 46: // inc
                pwm[id][n][1] += (float)1.0;
                wait(0.05);
                break;
            case 0x0d: // quit
                terminate = true;
                break;
            default:
                break;
        }
        pc.printf(" %d      \r", (int)pwm[id][n][1]); serial_flush();
        pwm_driver.setPWM(id, 0, (uint16_t)pwm[id][n][1]);
    }

}

void set_servo() {
    int id = 0;
    float angle;
        
    pc.printf("Set servo angle\n\r");
    pc.printf("JointID: ");serial_flush(); 
    pc.scanf("%d", &id); pc.printf("%d\n\r", id);
    pc.printf("Angle: ");serial_flush(); 
    pc.scanf("%f", &angle); pc.printf("%f\n\r", angle);
    
    set_servo_angle(id, angle);
}

void joint_calibrator() {
    int id = 0;
    float angle[3];
    //char str[128];

    float swapAngle, swapPwm;
        
    pc.printf("Joint calibrator\n\r");
    pc.printf("JointID: ");serial_flush(); 
    pc.scanf("%d", &id); pc.printf("%d\n\r", id);
    for (int i=0;i<3;i++) {
        pc.printf("Assigned angle%d: ", i);serial_flush(); 
        pc.scanf("%f", &angle[i]); pc.printf("%f\n\r", angle[i]);
        calib_pwm(id, i, angle[i]);   
    }
    for (int j=0; j<2; j++) {
        for (int i=0; i<2; i++) {
            if (pwm[id][i][0] > pwm[id][i+1][0]) {
                swapAngle = pwm[id][i][0];
                swapPwm   = pwm[id][i][1];
                pwm[id][i][0] = pwm[id][i+1][0];
                pwm[id][i][1] = pwm[id][i+1][1];
                pwm[id][i+1][0] = swapAngle;
                pwm[id][i+1][1] = swapPwm;
            }
        }
    }
}   
void read_adc() {
    float AccX = adc0.read();
    float AccY = adc1.read();
    float AccZ = adc2.read();
    float GyroX = adc3.read();
    float GyroY = adc4.read();
    float DistS = adc5.read();

    pc.printf("Read ADC\r\n");
    pc.printf("Acc : %f, %f, %f\r\n", AccX, AccY, AccZ);
    pc.printf("Gyro: %f, %f\r\n", GyroX, GyroY);
    pc.printf("Dist: %f\r\n", DistS);
}
void interpretor() {
    char c;
    
    pc.printf("Maintenance Menu:\n\r");
    pc.printf(" s: show status\n\r");
    pc.printf(" c: joint calibrator\n\r");
    pc.printf(" a: get ADC value\r\n");
    pc.printf(" w: set angle\r\n");
    pc.printf(" q: quit\n\r");
    
    while(1) {
        pc.printf("> ");serial_flush();
        c = pc.getc();
        switch(c) {
            case 's':
                show_status();
                break;
            case 'c':
                joint_calibrator();
                break;
            case 'a':
                read_adc();
                break;
            case 'w':
                set_servo();
                break;
            case 'q':
                return;
                // break;
            default:
                pc.printf("\n\r", c);
                break;
        }
    }
}

void ryutron_system() {
    while(1) {
        pc.printf("System started(under construction)\n\r");
        wait(1);
    }
}

/*
ID:0    0.00 > 328      0.00 > 328      0.00 > 328 
ID:1    -15.00 > 252    0.00 > 335      15.00 > 410 
ID:2    0.00 > 364      45.00 > 263     90.00 > 127 
ID:3    0.00 > 257      45.00 > 364     90.00 > 460 
ID:4    0.00 > 328      0.00 > 328      0.00 > 328 
ID:5    0.00 > 328      0.00 > 328      0.00 > 328 
ID:6    0.00 > 328      0.00 > 328      0.00 > 328 
ID:7    0.00 > 328      0.00 > 328      0.00 > 328 
ID:8    0.00 > 328      0.00 > 328      0.00 > 328 
ID:9    0.00 > 328      0.00 > 328      0.00 > 328 
ID:10   0.00 > 328      0.00 > 328      0.00 > 328 
ID:11   0.00 > 328      0.00 > 328      0.00 > 328 
ID:12   0.00 > 328      0.00 > 328      0.00 > 328 
ID:13   0.00 > 328      0.00 > 328      0.00 > 328 
ID:14   0.00 > 328      0.00 > 328      0.00 > 328 
ID:15   0.00 > 328      0.00 > 328      0.00 > 328 
*/