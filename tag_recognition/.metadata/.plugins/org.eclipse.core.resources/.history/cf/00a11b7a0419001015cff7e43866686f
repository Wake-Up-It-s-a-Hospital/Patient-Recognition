/*
 * HuskyLens_HAL.c
 *
 *  Created on: Apr 12, 2025
 *      Author: Luke
 */

#include "HuskyLens_HAL.h"

#define CMD_REQUEST_BLOCKS 0x21

static uint8_t tx_buf[] = {0x55, 0xAA, 0x11, 0x00, CMD_REQUEST_BLOCKS};

bool HuskyLens_ReadXY(I2C_HandleTypeDef *hi2c, HuskyLensPosition *pos) {
    uint8_t rx_buf[30];

    if (HAL_I2C_Master_Transmit(hi2c, HUSKYLENS_I2C_ADDR, tx_buf, sizeof(tx_buf), 100) != HAL_OK)
        return false;

    if (HAL_I2C_Master_Receive(hi2c, HUSKYLENS_I2C_ADDR, rx_buf, sizeof(rx_buf), 100) != HAL_OK)
        return false;

    if (!(rx_buf[0] == 0x55 && rx_buf[1] == 0xAA))
        return false;

    pos->x = rx_buf[8]  | (rx_buf[9]  << 8);
    pos->y = rx_buf[10] | (rx_buf[11] << 8);

    return true;
}

