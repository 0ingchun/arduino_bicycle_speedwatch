#pragma once

typedef struct MPU6050_data_s
{
    float AccX;
    float AccY;
    float AccZ;

    float GyroX;
    float GyroY;
    float GyroZ;

    float AccAngleX;
    float AccAngleY;
    //float AccAngleZ;

    float GyroAngleX;
    float GyroAngleY;
    float GyroAngleZ;

    float AngleX;
    float AngleY;
    float AngleZ;

} MPU6050_data_t;