#include <Arduino.h>
#include <math.h>
#include <main.h>
// #include <Arduino_FreeRTOS.h>
// #include <MPU6050_tockn.h>
#include <dht.h>
#include <Wire.h>
#include <SPI.h>
#include "U8glib.h"
#include "basicMPU6050.h"
#include "imuFilter.h"


basicMPU6050<> imu;

imuFilter fusion;

vec3_t velocity = {0, 0, 0};  // 初始化速度为0
float deltaTime = 0.01;  // 假设你每10毫秒读取一次数据

// U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI 
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);	// Fast I2C / TWI 

const int buttonPin = 2;  // 按钮连接的引脚

// #define PI 3.141592654


// ========= hall functions ===========
unsigned long hall_dt = 0;
double hall_v;

void hall_speed() {

    hall_v = ((64*PI)/hall_dt)*3.6;   //km/h

    hall_dt = millis() - hall_dt;

}

// ========= imu functions ===========

void printVector( vec3_t r ) {
    Serial.print( r.x, 2 );
    Serial.print( "," );
    Serial.print( r.y, 2 );
    Serial.print( "," );
    Serial.print( r.z, 2 );
}

void printQuat( quat_t q ) {
    Serial.print( q.w );
    Serial.print( "," );
    printVector( q.v );
    }

// ========= oled functions ===========

void u8g_drawNumber(int x, int y, float number) {
  u8g.setPrintPos(x, y); // 设置输出位置
  u8g.print(number); // 打印数字
}

void draw(void) {
    // graphic commands to redraw the complete screen should be placed here  
    u8g.setFont(u8g_font_unifont);
    // u8g.setFont(u8g_font_osb21);
    // u8g.drawStr( 0, 10, "Hello World!");
    u8g.drawStr( 0, 12, "Vhall: ");
    u8g_drawNumber(50, 12, hall_v);
    u8g.drawStr( 95, 12, "km/h");
    u8g.drawStr( 0, 25, "Vx = ");
    u8g_drawNumber(50, 25, velocity.x);
    u8g.drawStr( 0, 40, "Vy = ");
    u8g_drawNumber(50, 40, velocity.y);
    u8g.drawStr( 0, 55, "Vz = ");
    u8g_drawNumber(50, 55, velocity.z);
    // u8g.drawStr( 0, 25, "Vx = ");
    // u8g_drawNumber(50, 25, imu.ax());
    // u8g.drawStr( 0, 40, "Vy = ");
    // u8g_drawNumber(50, 40, imu.ay());
    // u8g.drawStr( 0, 55, "Vz = ");
    // u8g_drawNumber(50, 55, imu.az());
}


// ========= arduino functions ===========

    void setup() {
    Serial.begin(115200);
        
    // Calibrate imu
    imu.setup();
    imu.setBias();
    
    // Initialize filter: 
    fusion.setup( imu.ax(), imu.ay(), imu.az() ); 


    // flip screen, if required
    // u8g.setRot180();
    
    // set SPI backup if required
    //u8g.setHardwareBackup(u8g_backup_avr_spi);

    // assign default color value
    if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
        u8g.setColorIndex(255);     // white
    }
    else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
        u8g.setColorIndex(3);         // max intensity
    }
    else if ( u8g.getMode() == U8G_MODE_BW ) {
        u8g.setColorIndex(1);         // pixel on
    }
    else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
        u8g.setHiColorByRGB(255,255,255);
    }
    
    pinMode(8, OUTPUT);

    pinMode(2, INPUT);

    attachInterrupt(digitalPinToInterrupt(buttonPin), hall_speed, RISING);
}

void loop() {  
    // Update filter:
    fusion.update( imu.gx(), imu.gy(), imu.gz(), imu.ax(), imu.ay(), imu.az() );    

    // Unit vectors of rectangular coodinates [Choose between GLOBAL_FRAME and LOCAL_FRAME]
    vec3_t x = fusion.getXaxis(GLOBAL_FRAME);
    vec3_t y = fusion.getYaxis(GLOBAL_FRAME);
    vec3_t z = fusion.getZaxis(GLOBAL_FRAME);

    const vec3_t VEC = {1, 1, 0};
    vec3_t v = fusion.projectVector(VEC, GLOBAL_FRAME);

    // Display vectors:
    Serial.print( " x = " );
    printVector( x );
    Serial.print( " | y = " );
    printVector( y );
    Serial.print( " | z = " );
    printVector( z );
    Serial.print( " | v = " );
    printVector( v );

    // Display quaternion  
    Serial.print( " | q = " );
    printQuat( fusion.getQuat() );  
    Serial.println();

    // 更新角速度（这已经是空间中的角速度）
    float omega_x = imu.gx();
    float omega_y = imu.gy();
    float omega_z = imu.gz();
    
    // 更新加速度
    float ax = imu.ax() - (0.53);
    float ay = imu.ay() - (- 0.52);
    float az = imu.az() - (- 2.00);
    
    // 从加速度计算速度
    velocity.x += ax * deltaTime;
    velocity.y += ay * deltaTime;
    velocity.z += az * deltaTime;

    // picture loop
    u8g.firstPage();  
    do {
        draw();
    } while( u8g.nextPage() );
    
    // rebuild the picture after some delay
    delay(10);

}
