#ifndef __HUSKYLENS_HAL_H__
#define __HUSKYLENS_HAL_H__

#include "i2c.h"
#include "stdint.h"
#include "stdbool.h"

#define HUSKYLENS_I2C_ADDR  (0x32 << 1)

typedef struct {
    int16_t x;
    int16_t y;
} HuskyLensPosition;

bool HuskyLens_ReadXY(I2C_HandleTypeDef *hi2c, HuskyLensPosition *pos);

#endif // __HUSKYLENS_HAL_H__
