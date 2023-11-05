#include <Arduino.h>

#include <math.h>
// #include <Arduino_FreeRTOS.h>
// #include <MPU6050_tockn.h>
#include <dht.h>
#include <Wire.h>
// #include <SPI.h>
#include "U8glib.h"
#include "basicMPU6050.h"
// #include "imuFilter.h"

#include <main.h>

// 这里的时间秒数超过30会失效，原因不明
#define V_TO_ZERO_TIME_SEC 6 // 6秒速度不更新则速度置0
#define BACK_HOME_TIME_SEC 25 // 30秒按钮无动作回1画面
#define SLEEPH_TIME_SEC 32 // 60秒无动作则睡眠

basicMPU6050<> imu; // 创建MPU6050六轴加速度计对象

// imuFilter fusion;

// vec3_t velocity = {0, 0, 0};  // 初始化速度为0
float deltaTime = 1;  // 假设你每10毫秒读取一次数据

// U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI 
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);	// Fast I2C / TWI 

const int hallPin = 2;  // 霍尔传感器引脚
const int ledPin = 13;  // LED引脚
const int buttonPin = 3;  // 按钮引脚
const int dhtPin = 5;  // DHT11温湿度传感器引脚
// #define PI 3.141592654

dht DHT;    // 创建DHT11温湿度传感器对象

// ========= hall functions ===========
unsigned long hall_dt = 0 , hall_it = 0 , touch_it = 0;
double hall_v = 0 , hall_v_temp = 0 , hall_v_MAX = 0;
float hall_mile = 0;

void cal_MAX_v()    // 计算最大速度
{
    if (hall_v > hall_v_MAX)
    {
        hall_v_MAX = hall_v;
    }
    Serial.println(hall_v_MAX);
}

void hall_speed(void) {   // 霍尔速度计算函数
    Serial.println("void hall_speed()");

    hall_v_temp = ((64*0.01*PI)/(hall_dt*0.001))*3.5;   //km/h

    if (hall_v_temp >= 0 && hall_v_temp <= 260) // 限制速度范围，滤除不可能的速度杂波
    {
        hall_v = hall_v_temp;
    }
    else Serial.println("hallVtemp : error");

    cal_MAX_v();    //  计算最大速度
    Serial.println(hall_v);
}

void hall_odo(void) {   // 里程累加函数
    Serial.println("void hall_odo()");
    hall_mile = hall_mile + 0.7*PI;
}

// ========= imu functions ===========

// void printVector( vec3_t r ) {
//     Serial.print( r.x, 2 );
//     Serial.print( "," );
//     Serial.print( r.y, 2 );
//     Serial.print( "," );
//     Serial.print( r.z, 2 );
// }

// void printQuat( quat_t q ) {
//     Serial.print( q.w );
//     Serial.print( "," );
//     printVector( q.v );
//     }

// ========= oled functions ===========

void u8g_drawNumber(int x, int y, float number) { // oled绘制数字函数
  u8g.setPrintPos(x, y); // 设置输出位置
  u8g.print(number); // 打印数字
}

int button_touch_flag = 0;  // 按钮触摸标志，被按钮中断函数改变，被绘图函数读取
bool power_on_flag = 1; //  开机启动用flag，显示开机画面一次后失效

void draw(void) {   //绘图函数，通过button_touch_flag选择绘制的画面
    if (power_on_flag == 0) {
        if (button_touch_flag == 1)  // 开机画面
        {
            // // graphic commands to redraw the complete screen should be placed here  
            // u8g.setFont(u8g_font_unifont);
            // // u8g.setFont(u8g_font_osb21);
            // // u8g.drawStr( 0, 10, "Hello World!");
            // u8g.drawStr( 0, 12, "Vh : ");
            // u8g_drawNumber(40, 12, hall_v);
            // u8g.drawStr( 95, 12, "km/h");

            // u8g.drawStr( 0, 25, "Ax = ");
            // u8g_drawNumber(40, 25, imu.ax() );
            // u8g.drawStr( 0, 40, "Ay = ");
            // u8g_drawNumber(40, 40, imu.ay() );
            // u8g.drawStr( 0, 55, "Az = ");
            // u8g_drawNumber(40, 55, imu.ax() );
            // // u8g.drawStr( 0, 25, "Vx = ");
            // // u8g_drawNumber(50, 25, imu.ax());
            // // u8g.drawStr( 0, 40, "Vy = ");
            // // u8g_drawNumber(50, 40, imu.ay());
            // // u8g.drawStr( 0, 55, "Vz = ");
            // // u8g_drawNumber(50, 55, imu.az());

                // graphic commands to redraw the complete screen should be placed here  
            // u8g.setFont(u8g_font_unifont);
            u8g.setFont(u8g_font_osb26);
            // u8g.drawStr( 0, 10, "Hello World!");
            // u8g.drawStr( 0, 12, "Vh : ");
            u8g_drawNumber(5, 30, hall_v);

            // u8g.setFont(u8g_font_unifont);
            u8g.drawStr( 50, 60, "km/h");

            // u8g.drawStr( 0, 25, "Ax = ");
            // u8g_drawNumber(40, 25, imu.ax() );
            // u8g.drawStr( 0, 40, "Ay = ");
            // u8g_drawNumber(40, 40, imu.ay() );
            // u8g.drawStr( 0, 55, "Az = ");
            // u8g_drawNumber(40, 55, imu.ax() );
        }
        else if (button_touch_flag == 2)    // 温度画面
        {
            u8g.setFont(u8g_font_osb26);
            u8g_drawNumber(5, 30, DHT.temperature);
            u8g.drawStr( 85, 60, "*C");
        }
        else if (button_touch_flag == 3)    // 湿度画面
        {
            u8g.setFont(u8g_font_osb26);
            u8g_drawNumber(5, 30, DHT.humidity);
            u8g.drawStr( 95, 60, "%");
        }
        else if (button_touch_flag == 4)    // 里程画面
        {
            // 高于1000m自动转单位km显示
            u8g.setFont(u8g_font_osb26);
            if (hall_mile <= 999.99)
            {
                u8g_drawNumber(5, 30, hall_mile);
                u8g.drawStr( 95, 60, "m");
            }
            else
            {
                u8g_drawNumber(5, 30, hall_mile / 1000);
                u8g.drawStr( 85, 60, "km");
            }

        }
        else if (button_touch_flag == 5)    // 最大速度画面
        {
            u8g.setFont(u8g_font_osb26);
            u8g_drawNumber(5, 30, hall_v_MAX);
            u8g.drawStr( 10, 60, "V max");
        }
        else if (button_touch_flag == 6)    // 秒表画面
        {
            u8g.setFont(u8g_font_osb26);
            u8g_drawNumber(5, 30, (millis()*0.001));    // 秒
            u8g.drawStr( 70, 60, "sec");
        }
        else if (button_touch_flag == 7)    // 加速计画面
        {
            u8g.setFont(u8g_font_unifont);
            u8g.drawStr( 0, 25, "Ax = ");
            u8g_drawNumber(40, 25, (-1)*imu.ax() );
            u8g.drawStr( 0, 40, "Ay = ");
            u8g_drawNumber(40, 40, imu.ay() );
            u8g.drawStr( 0, 55, "Az = ");
            u8g_drawNumber(40, 55, (-1)*imu.az() );
        }
        else Serial.println("void draw Nothing");   // 空画面
    }
    else {  // 开机动画
        u8g.setFont(u8g_font_osb26);
        u8g.drawStr( 10, 10, "J master");
        u8g.drawStr( 5, 60, "StopWatch");
        // delay (1500);
        delay(10);
        power_on_flag = 0;
    }

}

// ========= arduino functions ===========


void button_touch(void) {   // 按钮触摸中断函数，改变button_touch_flag
    Serial.println("void button_touch()");
    delay(10);
    if(digitalRead(buttonPin) == HIGH)
    {
        // delay(500);
        while(digitalRead(buttonPin) == HIGH){
        }
        button_touch_flag++;
        if (button_touch_flag > 7)  // 高于菜单个数则flag置零
        {
            button_touch_flag = 0;
        }
    }
    // else if(digitalRead(buttonPin) == LOW)
    // {
    //     while(digitalRead(buttonPin) == LOW){
    //     }
    //     button_touch_flag--;
    //     if (button_touch_flag <= 0)  // 高于菜单个数则flag置零
    //     {
    //         button_touch_flag = 0;
    //     }
    // }
    Serial.println(button_touch_flag);

    touch_it = millis();    //  执行按钮中断退出的时间
}

// boll daiji_flag = 0;
void hall_touch(void) { // 霍尔传感器中断函数，触发后操作霍尔速度和最大霍尔速度
    Serial.println("void hall_touch()");
    
    hall_dt = millis() - hall_it;   //  计算霍尔传感器中断间隔时间
    Serial.println(hall_dt);

    hall_speed();
    hall_odo();
    if (button_touch_flag == 0) button_touch_flag = 1;
    
    hall_it = millis(); // 执行霍尔中断退出的时间
}

// ========= arduino functions ===========

    void setup() {
    Serial.begin(115200);
    delay(20);

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
    
    //清空屏幕内容？？怎么写
    // u8g.clearBuffer();


    // Calibrate imu
    imu.setup();
    imu.setBias();
    
    // Initialize filter: 
    // fusion.setup( imu.ax(), imu.ay(), imu.az() );

    //霍尔传感器与中断函数初始化
    pinMode(2, INPUT);
    attachInterrupt(digitalPinToInterrupt(hallPin), hall_touch, RISING);

    //触摸按钮中断函数初始化
    pinMode(3, INPUT);
    attachInterrupt(digitalPinToInterrupt(buttonPin), button_touch, RISING);
}

void loop() {  
    // Update filter:
    // fusion.update( imu.gx(), imu.gy(), imu.gz(), imu.ax(), imu.ay(), imu.az() );    

    // // Unit vectors of rectangular coodinates [Choose between GLOBAL_FRAME and LOCAL_FRAME]
    // vec3_t x = fusion.getXaxis(GLOBAL_FRAME);
    // vec3_t y = fusion.getYaxis(GLOBAL_FRAME);
    // vec3_t z = fusion.getZaxis(GLOBAL_FRAME);

    // const vec3_t VEC = {1, 1, 0};
    // vec3_t v = fusion.projectVector(VEC, GLOBAL_FRAME);

    // // Display vectors:
    // Serial.print( " x = " );
    // printVector( x );
    // Serial.print( " | y = " );
    // printVector( y );
    // Serial.print( " | z = " );
    // printVector( z );
    // Serial.print( " | v = " );
    // printVector( v );

    // // Display quaternion  
    // Serial.print( " | q = " );
    // printQuat( fusion.getQuat() );  
    // Serial.println();

    // 更新角速度（这已经是空间中的角速度）
    // float omega_x = imu.gx();
    // float omega_y = imu.gy();
    // float omega_z = imu.gz();
    
    // // 更新加速度
    // float ax = imu.ax() - (0.53);
    // float ay = imu.ay() - (- 0.52);
    // float az = imu.az() - (- 2.00);
    
    // 从加速度计算速度
    // velocity.x += ax * deltaTime;
    // velocity.y += ay * deltaTime;
    // velocity.z += az * deltaTime;

    //////////////////////////////////

    DHT.read11(dhtPin); // 更新并读取温湿度

    // Serial.print(DHT.humidity, 1);
    // Serial.print(",\t");
    // Serial.println(DHT.temperature, 1);

    //////////////////////////////////

    if ((millis() - hall_it) >= (V_TO_ZERO_TIME_SEC*1000)) {   // 自行车速度为0超时速度置零
        hall_v = 0.00;
    }

    if ((millis() - touch_it) >= (BACK_HOME_TIME_SEC*1000) && (millis() - hall_it) <= (BACK_HOME_TIME_SEC*1000)) {   // 骑行中，按钮超时未触发返回画面1
        button_touch_flag = 1;
    }

    // 打印间隔
    Serial.println(millis() - hall_it);
    Serial.println(millis() - touch_it);

    if ((millis() - hall_it) >= (SLEEPH_TIME_SEC*1000) && (millis() - touch_it) >= (SLEEPH_TIME_SEC*1000)) {   // 超时未启动自行车或未触发按钮画面flag置零（黑屏画面）
        button_touch_flag = 0;
    }

    // picture loop
    u8g.firstPage();  // oled绘图
    do {
        draw();
    } while( u8g.nextPage() );
    

    // Serial.println(millis());

    // rebuild the picture after some delay
    delay(deltaTime);   // 轮询间隔时间，防死机

}
