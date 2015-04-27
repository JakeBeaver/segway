#include <hFramework.h>
#include <stdio.h>
#include "lego.h"
#include "lego-touch.h"
#include "Hitechnic_Accel.h"
#include "Hitechnic_Gyro.h"
#include <math.h>
#include <Lego_Ultrasonic.h>
using namespace hFramework;


tLegoSensor touch(hSens5);
hSensors::Lego_Ultrasonic ultra(hSens4.getSoftwareI2C());

hSensors::Hitechnic_Accel accel(hSens1);
hSensors::Hitechnic_Gyro gyro(hSens2);

class csensors
{
    public:
    bool calib = false;
    int gauge_distance;
    int distance;
    float temp;
    float abs_ang=0;
    float angle_gyro=0;
    float angle_accel=0;
    float resultant_angle=0;

    float offset_gyro=1720;
    float offset_accel=-9;

    float multi_accel=180/3.141592;
    float multi_gyro =88.5/40;

    float dt=1;
    float recalib = 0.01;

    float absolute_angle_accel()
    {
        float x,y,z;
        accel.read(x,y,z);
        if (x!=0 and y!=0)
        {
            abs_ang=atan(x/z);
        }
        return abs_ang;
    }
    float angle_accel_function()
    {
        angle_accel=absolute_angle_accel()-offset_accel;
        return angle_accel;
    }

    void calculate()
    {
        if (not calib){
            angle_gyro = (gyro.read()-offset_gyro)* dt/1000 * multi_gyro;
            angle_accel=(absolute_angle_accel()-offset_accel)* -multi_accel;
            //resultant_angle += angle_gyro;
            resultant_angle = ((resultant_angle + angle_gyro) * (1-recalib) +  angle_accel * recalib);
            //printf("acc.angle: %f gyr.angle: %f, %d, resultant: %f\n",angle_accel, angle_gyro, gyro.read(), resultant_angle);
            printf("acc.angle: %f gyr.angle: %f, dist: %d\n",angle_accel, resultant_angle, distance);
            sys.delay_us(dt);

        }
    }
    void calibrate()
    {
        resultant_angle = 0;
        offset_accel=absolute_angle_accel();
        printf("offset: %f\n",offset_accel);
    }
};
csensors sensors;

void angle_calculator()
{
    bool go_class = false;

    while(not go_class)
    {
        sensors.calculate();
    }
}

void encoder()
{
    for(;;)
    {
        LED2.toggle();
        sys.delay_ms(100);
    }
}

//do zrobienia <<------------------------------------------------do zrobienia
void distance_measurement(){
    int temp;
    for (;;){
        temp=ultra.readDist();
        if (temp>0 & temp<100) sensors.distance = temp;
    }
}
void hMain(void)
{
    bool calib;
    sys.setLogDev(&Serial);
    sys.taskCreate(encoder);
    sys.taskCreate(distance_measurement);
    sys.taskCreate(angle_calculator);
    sensors.calibrate();

    for (;;)
    {
        sensors.calib = not TSreadState(touch);

        if (sensors.calib)
        {
            sensors.calibrate();
            LED1.on();
            LED3.on();
        }
        else {
            LED1.off();
            LED3.off();

        }

        hMot1.setPower(sensors.resultant_angle*-200);
        hMot2.setPower(sensors.resultant_angle*-200);
    }
}
