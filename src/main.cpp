#include <Arduino.h>
#include <math.h>
#include <main.h>
#include <Arduino_FreeRTOS.h>
#include <MPU6050_tockn.h>
#include <dht.h>
#include <Wire.h>
#include <SPI.h>
#include "U8glib.h"


#define circumference_wheel(r) (2 * M_PI *(r))
float c_wheel = circumference_wheel(40);

dht DHT;
#define DHT11_PIN 5

#define hall_pin 2
#define touchButton_pin 3
#define LED_BUILTIN 13

MPU6050 mpu6050(Wire);

float speed = 0;

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/


void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}


MPU6050_data_t MPU6050_data_temp;
MPU6050_data_t MPU6050_data_static;
void mpu6050_updater(MPU6050_data_t *ptr)
{
  mpu6050.update();
  ptr->AccX = mpu6050.getAccX();
  ptr->AccY = mpu6050.getAccY();
  ptr->AccZ = mpu6050.getAccZ();
  ptr->GyroX = mpu6050.getGyroX();
  ptr->GyroY = mpu6050.getGyroY();
  ptr->GyroZ = mpu6050.getGyroZ();
  ptr->AccAngleX = mpu6050.getAccAngleX();
  ptr->AccAngleY = mpu6050.getAccAngleY();
  //ptr->AccAngleZ = mpu6050.getAccAngleZ();
  ptr->GyroAngleX = mpu6050.getGyroAngleX();
  ptr->GyroAngleY = mpu6050.getGyroAngleY();
  ptr->GyroAngleZ = mpu6050.getGyroAngleZ();
  ptr->AngleX = mpu6050.getAngleX();
  ptr->AngleY = mpu6050.getAngleY();
  ptr->AngleZ = mpu6050.getAngleZ();
}
void mpu6050_updater_offset(MPU6050_data_t *ptr)
{
  mpu6050.update();
  ptr->AccX = mpu6050.getAccX() - MPU6050_data_static.AccX;
  ptr->AccY = mpu6050.getAccY() - MPU6050_data_static.AccY;
  ptr->AccZ = mpu6050.getAccZ() - MPU6050_data_static.AccZ;
  ptr->GyroX = mpu6050.getGyroX() - MPU6050_data_static.GyroX;
  ptr->GyroY = mpu6050.getGyroY() - MPU6050_data_static.GyroY;
  ptr->GyroZ = mpu6050.getGyroZ() - MPU6050_data_static.GyroZ;
  ptr->AccAngleX = mpu6050.getAccAngleX() - MPU6050_data_static.AccAngleX;
  ptr->AccAngleY = mpu6050.getAccAngleY() - MPU6050_data_static.AccAngleY;
  //ptr->AccAngleZ = mpu6050.getAccAngleZ() - MPU6050_data_static.AccAngleZ;
  ptr->GyroAngleX = mpu6050.getGyroAngleX() - MPU6050_data_static.GyroAngleX;
  ptr->GyroAngleY = mpu6050.getGyroAngleY() - MPU6050_data_static.GyroAngleY;
  ptr->GyroAngleZ = mpu6050.getGyroAngleZ() - MPU6050_data_static.GyroAngleZ;
  ptr->AngleX = mpu6050.getAngleX() - MPU6050_data_static.AngleX;
  ptr->AngleY = mpu6050.getAngleY() - MPU6050_data_static.AngleY;
  ptr->AngleZ = mpu6050.getAngleZ() - MPU6050_data_static.AngleZ;

}


// This is a task.
void TaskMPU6050(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  long timer = 0;

  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  for (;;)
  {
    mpu6050.update();

    if(speed > -1 || speed < 1){
          mpu6050_updater(&MPU6050_data_static);
        }
        mpu6050_updater_offset(&MPU6050_data_temp);

        if(millis() - timer > 100){
          
          Serial.println("=======================================================");
          Serial.print("temp : ");Serial.println(mpu6050.getTemp());
          Serial.print("accX : ");Serial.print(mpu6050.getAccX());
          Serial.print("\taccY : ");Serial.print(mpu6050.getAccY());
          Serial.print("\taccZ : ");Serial.println(mpu6050.getAccZ());
        
          Serial.print("gyroX : ");Serial.print(mpu6050.getGyroX());
          Serial.print("\tgyroY : ");Serial.print(mpu6050.getGyroY());
          Serial.print("\tgyroZ : ");Serial.println(mpu6050.getGyroZ());
        
          Serial.print("accAngleX : ");Serial.print(mpu6050.getAccAngleX());
          Serial.print("\taccAngleY : ");Serial.println(mpu6050.getAccAngleY());
        
          Serial.print("gyroAngleX : ");Serial.print(mpu6050.getGyroAngleX());
          Serial.print("\tgyroAngleY : ");Serial.print(mpu6050.getGyroAngleY());
          Serial.print("\tgyroAngleZ : ");Serial.println(mpu6050.getGyroAngleZ());
          
          Serial.print("angleX : ");Serial.print(mpu6050.getAngleX());
          Serial.print("\tangleY : ");Serial.print(mpu6050.getAngleY());
          Serial.print("\tangleZ : ");Serial.println(mpu6050.getAngleZ());
          Serial.println("=======================================================\n");

          timer = millis();
          
        }

    vTaskDelay( 100 / portTICK_PERIOD_MS );
  }
}


void TaskHALL(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  unsigned long startTime;
  startTime = millis();
  for (; ;)
  {
  if (digitalRead(hall_pin) == 1){
    unsigned long currentTime = millis(); // 获取当前时间
    unsigned long elapsedTime = currentTime - startTime; // 计算已经运行的时间
  // 使用elapsedTime变量进行后续操作
    speed = c_wheel / elapsedTime;
if (MPU6050_data_temp.AccX < 0 ){
      speed = -speed;
    }
    startTime = millis(); // 记录程序开始时间
  }
    
    vTaskDelay( 100 / portTICK_PERIOD_MS );
  }
}


void TaskDHT(void *pvParameters)
{
  (void) pvParameters;

  for (; ;){
    DHT.read11(DHT11_PIN);

    Serial.print(DHT.humidity, 1);
    Serial.print(",\t");
    Serial.println(DHT.temperature, 1);
  
    vTaskDelay( 100 / portTICK_PERIOD_MS );
  }
}


U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI 
void draw(void) {
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  u8g.drawStr( 0, 22, "Hello World!");
  u8g.drawStr( 0, 42,   "MPU6050");
  u8g.drawStr( 0, 62, "DHT11");
  u8g.drawStr( 0, 82, "MPU6050");
  u8g.drawStr( 0, 102, "DHT11");

}

void TaskDISPLAY(void *pvParameters)
{
  (void) pvParameters;

  u8g.begin();
  // flip screen, if required
  u8g.setRot180();
  
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

  for (; ;){
  picture loop
  u8g.firstPage();  
  do {
    draw();
    vTaskDelay(50);
  } while( u8g.nextPage() );
  draw();
  
  vTaskDelay( 100 / portTICK_PERIOD_MS );
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();

    // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  "Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskMPU6050
    ,  "MPU6050"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  xTaskCreate(
    TaskHALL
    ,  "HALL"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

      xTaskCreate(
    TaskDHT
    ,  "DHT"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

    xTaskCreate(
    TaskDISPLAY
    ,  "DISPLAY"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.

}

void loop() {
  // put your main code here, to run repeatedly:

  // Empty. Things are done in Tasks.
}
