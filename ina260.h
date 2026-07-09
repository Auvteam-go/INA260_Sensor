#ifndef INA260_H
#define INA260_H

#include "main.h"
#include <stdint.h>

/* 7-bit I2C address (A0/A1 tied GND -> 0x40). Adjust if your breakout's
 * address pins are strapped differently. */
#define INA260_I2C_ADDR   (0x40 << 1)

#define INA260_REG_CONFIG      0x00
#define INA260_REG_CURRENT     0x01
#define INA260_REG_BUSVOLTAGE  0x02
#define INA260_REG_POWER       0x03
#define INA260_REG_MASK_ENABLE 0x06
#define INA260_REG_ALERT_LIMIT 0x07
#define INA260_REG_MFR_ID      0xFE
#define INA260_REG_DIE_ID      0xFF

typedef struct {
    float   voltage_V;
    float   current_A;
    float   power_W;
    uint8_t valid;
    uint32_t errorCount;
} INA260_Data_t;

/* Must be called once at startup with the I2C handle to use. */
HAL_StatusTypeDef INA260_Init(I2C_HandleTypeDef *hi2c);

/* Blocking read of all three measurement registers. */
HAL_StatusTypeDef INA260_Read(INA260_Data_t *data);

#endif /* INA260_H */
