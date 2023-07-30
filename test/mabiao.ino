/*
Name:       MPU6050_TestDemo.ino
Created:  2020/3/18 星期三 20:17:09
Author:     SilenceJerui
*/
/*    接线方式： 
//MPU6050--UNO   
//VCC--5V   
//GND--GND   
//SCL--A5  
//SDA--A4
//AD0-- （AD0引脚为地址选择引脚，悬空或者接GND，硬件地址为0x68，接VCC/高电平硬件地址为0x69）   
//INT-- （中断引脚，如果用到可以接中端口，此处不接）
*/
#include <Wire.h>
// Define User Types below here or use a .h file
//
#define ACCEL_CONFIG 1
#define GYRO_CONFIG 1
#define G 9.8
float val_seven[7];
float velocity = 0.0; // 当前速度
float p_zero_a_x = 0.0;
// Define Function Prototypes that use User Types below here or use a .h file
//
// Define Functions below here or use other.ino or cpp files
//向0x6B写入一个字节的数据0，启动器件
void MPU_START(void)
{
   Wire.beginTransmission(0x68); //开启MPU6050的传输
   Wire.write(0x6B); //指定寄存器地址
   Wire.write(0); //写入一个字节的数
   Wire.endTransmission(true); //结束传输，true表示释放总线
}
//配置角速度倍率
void GYRO_CONFIG_SET(int f)
{   
   Wire.beginTransmission(0x68); //开启MPU-6050的传
   Wire.write(0x1B); //角速度倍率寄存器的地址
   Wire.requestFrom(0x68, 1, true); //先读出原配置
   unsigned char acc_conf = Wire.read();
   acc_conf = ((acc_conf & 0xE7) | (f << 3));
   Wire.write(acc_conf);
   Wire.endTransmission(true); //结束传输，true表示释放总线
}
//配置加速度倍率
void ACCEL_CONFIG_SET(int f)
{
   Wire.beginTransmission(0x68); //开启MPU-6050的传输
   Wire.write(0x1C); //加速度倍率寄存器的地址
   Wire.requestFrom(0x68, 1, true); //先读出原配置
   unsigned char acc_conf = Wire.read();
   acc_conf = ((acc_conf & 0xE7) | (f << 3));
   Wire.write(acc_conf);
   Wire.endTransmission(true); //结束传输，true表示释放总线
}
//获取MPU数据
void Get_Value(void)
{
   //获取各个轴分量数据
   Wire.beginTransmission(0x68); //开启MPU-6050的传输
   Wire.write(0x3B);
   Wire.requestFrom(0x68, 7 * 2, true);
   Wire.endTransmission(true);
   for (long i = 0; i < 7; ++i) 
   {
      val_seven[i] = Wire.read() << 8 | Wire.read();
   }
   //数据换算
   val_seven[0] = (float)(val_seven[0] / 32768 * (2 ^ ACCEL_CONFIG) *
G);//acc_x
   val_seven[1] = (float)(val_seven[1] / 32768 * (2 ^ ACCEL_CONFIG) * G);//acc_y
   val_seven[2] = (float)(val_seven[2] / 32768 * (2 ^ ACCEL_CONFIG) *
G);//acc_z
   val_seven[3] = (float)(val_seven[3] / 340 + 36.53);//acc_z
   val_seven[4] = (float)(val_seven[4] / 32768 * (2 ^ GYRO_CONFIG) *
250);//gyr_x
   val_seven[5] = (float)(val_seven[5] / 32768 * (2 ^ GYRO_CONFIG) *
250);//gyr_y
   val_seven[6] = (float)(val_seven[6] / 32768 * (2 ^ GYRO_CONFIG) *
250);//gyr_z
}
// The setup() function runs once each time the micro-controller starts
void setup()
{
   Serial.begin(115200);
   Wire.begin();//以主机模式开启总线
   MPU_START();//启动MPU6050   
   GYRO_CONFIG_SET(0);//配置器件角速度倍率
   ACCEL_CONFIG_SET(0);//配置器件加速度倍率

   Get_Value();
   p_zero_a_x = val_seven[0];
}
// Add the main program code into the continuous loop() function
void loop()
{
   Get_Value();
   Serial.print("acc_x:");
   Serial.print('\t');
   Serial.println(val_seven[0] - p_zero_a_x*2);
  //  Serial.print("acc_y:");
  //  Serial.print('\t');
  //  Serial.println(val_seven[1]);
  //  Serial.print("acc_z:");
  //  Serial.print('\t');
  //  Serial.println(val_seven[2]);
  //  Serial.print("gyr_x:");
  //  Serial.print('\t');
  //  Serial.println(val_seven[4]);
  //  Serial.print("gyr_y:");
  //  Serial.print('\t');
  //  Serial.println(val_seven[5]);
  //  Serial.print("gyr_z:");
  //  Serial.print('\t');
  //  Serial.println(val_seven[6]);
  //  Serial.print("TEMP:");
  //  Serial.print('\t');
  //  Serial.println(val_seven[3]);
  //  delay(500);
    // 计算速度
  velocity += (val_seven[0] - p_zero_a_x*2) * 0.01; // 假设采样时间间隔为0.01秒

  // 打印当前速度
  Serial.print("Velocity: ");
  Serial.print('\t');
  Serial.println(velocity);

  delay(10);
}
