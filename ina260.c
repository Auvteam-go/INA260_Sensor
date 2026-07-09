#include "ina260.h"

static I2C_HandleTypeDef *s_hi2c = NULL;

/* LSB sizes per INA260 datasheet */
#define INA260_CURRENT_LSB   1.25e-3f  /* 1.25 mA  */
#define INA260_VOLTAGE_LSB   1.25e-3f  /* 1.25 mV  */
#define INA260_POWER_LSB     10.0e-3f  /* 10 mW    */

#define I2C_TIMEOUT_MS  50

static HAL_StatusTypeDef INA260_ReadReg16(uint8_t reg, int16_t *out)
{
    uint8_t buf[2];
    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(s_hi2c, INA260_I2C_ADDR, reg,
                                             I2C_MEMADD_SIZE_8BIT, buf, 2,
                                             I2C_TIMEOUT_MS);
    if (st != HAL_OK) return st;
    *out = (int16_t)((buf[0] << 8) | buf[1]);
    return HAL_OK;
}

HAL_StatusTypeDef INA260_Init(I2C_HandleTypeDef *hi2c)
{
    s_hi2c = hi2c;

    /* Confirm the device answers before relying on it */
    if (HAL_I2C_IsDeviceReady(s_hi2c, INA260_I2C_ADDR, 3, 100) != HAL_OK)
        return HAL_ERROR;

    /* Default config register (0x6127) already gives continuous
     * bus voltage + current conversion @ 1.1ms/1.1ms, 16 sample avg off.
     * Left at power-on default; write here only if custom averaging/
     * conversion time is required. */
    return HAL_OK;
}

HAL_StatusTypeDef INA260_Read(INA260_Data_t *data)
{
    int16_t raw_v, raw_i, raw_p;
    HAL_StatusTypeDef st;

    st = INA260_ReadReg16(INA260_REG_BUSVOLTAGE, &raw_v);
    if (st != HAL_OK) goto fail;

    st = INA260_ReadReg16(INA260_REG_CURRENT, &raw_i);
    if (st != HAL_OK) goto fail;

    st = INA260_ReadReg16(INA260_REG_POWER, &raw_p);
    if (st != HAL_OK) goto fail;

    data->voltage_V = (float)raw_v * INA260_VOLTAGE_LSB;
    data->current_A = (float)raw_i * INA260_CURRENT_LSB;
    data->power_W   = (float)(uint16_t)raw_p * INA260_POWER_LSB;
    data->valid     = 1;
    return HAL_OK;

fail:
    data->valid = 0;
    data->errorCount++;
    return st;
}
