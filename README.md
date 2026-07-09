# INA260 Sensor Driver - STM32F103C8T6

## 📋 Table of Contents
1. [Overview](#overview)
2. [Features](#features)
3. [Hardware Configuration](#hardware-configuration)
4. [API Reference](#api-reference)
5. [Usage Examples](#usage-examples)
6. [I2C Protocol Details](#i2c-protocol-details)
7. [Register Map](#register-map)
8. [Measurement Calculations](#measurement-calculations)
9. [Error Handling](#error-handling)
10. [Integration with FreeRTOS](#integration-with-freertos)
11. [Debugging & Troubleshooting](#debugging--troubleshooting)
12. [Performance Optimization](#performance-optimization)

---

## Overview

This is a **high-precision INA260 power monitor driver** for STM32F103C8T6 microcontrollers. The INA260 is a **precision digital current, voltage, and power monitor** with an I2C interface. This driver provides simple, reliable access to all measurement registers with automatic error handling.

### Key Features
- ✅ **Precision measurements**: Voltage, current, and power readings
- ✅ **High accuracy**: ±0.1% gain error, ±10mA current accuracy
- ✅ **I2C communication**: 100kHz/400kHz support
- ✅ **Error handling**: Automatic error detection and counting
- ✅ **Thread-safe**: Designed for RTOS environments
- ✅ **Configurable**: Easy to modify I2C address and timing
- ✅ **Low overhead**: Minimal CPU usage (~5ms per read)
- ✅ **Data validation**: Built-in validity flag

---

## Hardware Configuration

### Pin Configuration

| Parameter | Setting |
|-----------|---------|
| **I2C Port** | I2C1 (PB6/PB7) |
| **SCL Pin** | PB6 |
| **SDA Pin** | PB7 |
| **I2C Address** | 0x40 (7-bit) |
| **I2C Speed** | 100kHz (standard) |
| **Pull-up Resistors** | 4.7kΩ (required) |
| **Power Supply** | 3.3V |

### Connection Diagram

```
┌──────────────────┐              ┌─────────────────┐
│   STM32F103C8T6  │              │    INA260       │
├──────────────────┤              ├─────────────────┤
│                  │              │                 │
│   PB6 (SCL)      ├──────────────┼──► SCL          │
│   PB7 (SDA)      ├──────────────┼──► SDA          │
│   3.3V           ├──────────────┼──► VCC (3.3V)   │
│   GND            ├──────────────┼──► GND          │
│                  │              │                 │
│   External 4.7kΩ │              │                 │
│   Pull-ups on    │              │                 │
│   SCL and SDA    │              │                 │
│                  │              │                 │
└──────────────────┘              └─────────────────┘
```

### INA260 Address Configuration

| A1 Pin | A0 Pin | I2C Address (7-bit) | Write Address | Read Address |
|--------|--------|---------------------|---------------|--------------|
| GND    | GND    | 0x40                | 0x80          | 0x81         |
| GND    | VS     | 0x41                | 0x82          | 0x83         |
| GND    | SDA    | 0x42                | 0x84          | 0x85         |
| GND    | SCL    | 0x43                | 0x86          | 0x87         |
| VS     | GND    | 0x44                | 0x88          | 0x89         |
| VS     | VS     | 0x45                | 0x8A          | 0x8B         |
| VS     | SDA    | 0x46                | 0x8C          | 0x8D         |
| VS     | SCL    | 0x47                | 0x8E          | 0x8F         |

**Default (A0=GND, A1=GND):** 0x40

### Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **INA260** | Precision current/power monitor |
| **Pull-up Resistors** | 4.7kΩ (2x) |
| **Power Supply** | 3.3V |
| **Shunt Resistor** | Built-in (2mΩ) |

---

## API Reference

### Data Structures

#### INA260_Data_t
```c
typedef struct {
    float   voltage_V;     /* Bus voltage in Volts (0-36V) */
    float   current_A;     /* Current in Amperes (0-15A) */
    float   power_W;       /* Power in Watts (0-540W) */
    uint8_t valid;         /* 1 = valid data, 0 = error */
    uint32_t errorCount;   /* Running error counter */
} INA260_Data_t;
```

### Constants

```c
// I2C Address (Default)
#define INA260_I2C_ADDR   (0x40 << 1)  // 0x80 (8-bit)

// Register Addresses
#define INA260_REG_CONFIG      0x00
#define INA260_REG_CURRENT     0x01
#define INA260_REG_BUSVOLTAGE  0x02
#define INA260_REG_POWER       0x03
#define INA260_REG_MASK_ENABLE 0x06
#define INA260_REG_ALERT_LIMIT 0x07
#define INA260_REG_MFR_ID      0xFE
#define INA260_REG_DIE_ID      0xFF

// LSB Values
#define INA260_CURRENT_LSB   1.25e-3f  // 1.25mA per LSB
#define INA260_VOLTAGE_LSB   1.25e-3f  // 1.25mV per LSB
#define INA260_POWER_LSB     10.0e-3f  // 10mW per LSB
```

### Functions

#### INA260_Init()
```c
HAL_StatusTypeDef INA260_Init(I2C_HandleTypeDef *hi2c);
```

**Description:** Initializes the INA260 driver and verifies I2C communication.
**Parameters:**
- `hi2c`: Pointer to initialized I2C_HandleTypeDef
**Returns:**
- `HAL_OK`: Initialization successful
- `HAL_ERROR`: Device not found
**Note:** Must be called before any other INA260 functions.

**Example:**
```c
I2C_HandleTypeDef hi2c1;
// ... I2C initialization ...

if (INA260_Init(&hi2c1) != HAL_OK) {
    Error_Handler();  // INA260 not detected
}
```

---

#### INA260_Read()
```c
HAL_StatusTypeDef INA260_Read(INA260_Data_t *data);
```

**Description:** Reads voltage, current, and power from the INA260.
**Parameters:**
- `data`: Pointer to INA260_Data_t structure
**Returns:**
- `HAL_OK`: Read successful
- `HAL_TIMEOUT`: I2C communication timeout
- `HAL_ERROR`: I2C communication error
**Behavior:**
- Reads 3 registers sequentially
- Converts raw values to engineering units
- Sets valid=1 on success, valid=0 on failure
- Increments errorCount on failure

**Example:**
```c
INA260_Data_t ina_data;
if (INA260_Read(&ina_data) == HAL_OK) {
    printf("Voltage: %.2fV, Current: %.3fA, Power: %.2fW\n",
           ina_data.voltage_V,
           ina_data.current_A,
           ina_data.power_W);
} else {
    printf("Read failed! Errors: %lu\n", ina_data.errorCount);
}
```

---

## Usage Examples

### Basic Usage
```c
#include "ina260.h"

I2C_HandleTypeDef hi2c1;
INA260_Data_t ina_data;

int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    MX_I2C1_Init();  // Initialize I2C
    MX_USART1_UART_Init();  // For debug output
    
    if (INA260_Init(&hi2c1) == HAL_OK) {
        printf("INA260 initialized successfully\n");
    } else {
        printf("INA260 not found!\n");
        Error_Handler();
    }
    
    while (1) {
        if (INA260_Read(&ina_data) == HAL_OK) {
            printf("V: %.2fV, I: %.3fA, P: %.2fW, Seq: %lu\n",
                   ina_data.voltage_V,
                   ina_data.current_A,
                   ina_data.power_W,
                   ++seq);
        } else {
            printf("Read error! Count: %lu\n", ina_data.errorCount);
        }
        HAL_Delay(200);  // 200ms period
    }
}
```

### With FreeRTOS Task
```c
// Global data structure (protected by mutex)
INA260_Data_t g_inaData;
osMutexId_t g_inaMutex;

static void INA260_Task(void *argument) {
    INA260_Data_t local = {0};
    TickType_t lastWake = xTaskGetTickCount();
    
    for (;;) {
        // Read sensor (blocking, ~5ms)
        if (INA260_Read(&local) == HAL_OK) {
            // Protect shared data with mutex
            osMutexAcquire(g_inaMutex, osWaitForever);
            g_inaData.voltage_V = local.voltage_V;
            g_inaData.current_A = local.current_A;
            g_inaData.power_W = local.power_W;
            g_inaData.valid = local.valid;
            g_inaData.errorCount = local.errorCount;
            osMutexRelease(g_inaMutex);
        }
        
        // Maintain 200ms period
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
    }
}
```

### Power Monitoring with Alerts
```c
INA260_Data_t ina_data;
float current_limit = 2.5f;  // 2.5A limit

void CheckOvercurrent(void) {
    if (INA260_Read(&ina_data) == HAL_OK) {
        if (ina_data.current_A > current_limit) {
            printf("OVERCURRENT: %.3fA > %.1fA\n",
                   ina_data.current_A,
                   current_limit);
            // Take action: shutdown load, alert, etc.
            GPIOD->BSRR = GPIO_PIN_12;  // Turn off load
        }
    }
}
```

### Power Consumption Logging
```c
void LogPowerData(void) {
    INA260_Data_t ina_data;
    char log_entry[128];
    
    if (INA260_Read(&ina_data) == HAL_OK) {
        sprintf(log_entry, "%lu,%.2f,%.3f,%.2f\n",
                HAL_GetTick(),
                ina_data.voltage_V,
                ina_data.current_A,
                ina_data.power_W);
        // Write to SD card or send via UART
        send_to_uart(log_entry);
    }
}
```

### Multiple INA260 Sensors
```c
// Two INA260 sensors on same bus
INA260_Data_t ina1_data, ina2_data;

// INA260 #1: Address 0x40 (A0=GND, A1=GND)
// INA260 #2: Address 0x41 (A0=VS, A1=GND)

#define INA260_ADDR1 0x40
#define INA260_ADDR2 0x41

// Modify driver for different addresses
void ReadBothSensors(void) {
    // Read sensor 1
    if (INA260_Read_Addr(&ina1_data, INA260_ADDR1) == HAL_OK) {
        process_data1(ina1_data);
    }
    
    // Read sensor 2
    if (INA260_Read_Addr(&ina2_data, INA260_ADDR2) == HAL_OK) {
        process_data2(ina2_data);
    }
}
```

---

## I2C Protocol Details

### I2C Transaction Flow

#### 1. Read Register (16-bit)
```
┌─────────────────────────────────────────────────────────────────────────┐
│ Master (STM32)                  │ Slave (INA260)                       │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│ START                                                                   │
│   │                                                                     │
│   ├── Send I2C Address (0x80) + Write bit                             │
│   │                     │                                              │
│   │                     ├────── ACK                                   │
│   │                                                                   │
│   ├── Send Register Address (8-bit)                                  │
│   │                     │                                              │
│   │                     ├────── ACK                                   │
│   │                                                                   │
│   ├── STOP                                                             │
│                                                                         │
│   ├── START                                                            │
│   │                                                                     │
│   ├── Send I2C Address (0x81) + Read bit                             │
│   │                     │                                              │
│   │                     ├────── ACK                                   │
│   │                                                                   │
│   ├── Read Byte (MSB)                                                │
│   │                     ├────── Data                                  │
│   │   └── ACK (Master)                                               │
│   │                                                                   │
│   ├── Read Byte (LSB)                                                │
│   │                     ├────── Data                                  │
│   │   └── NACK (Master) - End of read                                │
│   │                                                                   │
│   ├── STOP                                                            │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

#### 2. Write Register (16-bit)
```
┌─────────────────────────────────────────────────────────────────────────┐
│ Master (STM32)                  │ Slave (INA260)                       │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│ START                                                                   │
│   │                                                                     │
│   ├── Send I2C Address (0x80) + Write bit                             │
│   │                     │                                              │
│   │                     ├────── ACK                                   │
│   │                                                                   │
│   ├── Send Register Address (8-bit)                                  │
│   │                     │                                              │
│   │                     ├────── ACK                                   │
│   │                                                                   │
│   ├── Send Data MSB                                                   │
│   │                     │                                              │
│   │                     ├────── ACK                                   │
│   │                                                                   │
│   ├── Send Data LSB                                                   │
│   │                     │                                              │
│   │                     ├────── ACK                                   │
│   │                                                                   │
│   ├── STOP                                                            │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### I2C Timing Parameters

| Parameter | Value | Description |
|-----------|-------|-------------|
| **Clock Speed** | 100kHz | Standard mode |
| **Transaction Time** | ~500µs | Per register read |
| **Total Read Time** | ~1.5ms | All 3 registers |
| **Timeout** | 50ms | Communication timeout |

---

## Register Map

### Configuration Register (0x00)
```
┌─────────────────────────────────────────────────────────────────────────┐
│ Bit 15-12 │ Bit 11 │ Bit 10-9 │ Bit 8-6 │ Bit 5 │ Bit 4-3 │ Bit 2-0  │
├───────────┼────────┼──────────┼─────────┼───────┼─────────┼──────────┤
│ MODE      │ AVG    │ VBUS CT  │ VSH CT  │ RST   │ OPER    │ CONV     │
└─────────────────────────────────────────────────────────────────────────┘

Default Value: 0x6127

MODE (Bits 15-12):
  0 = Power-down
  1 = Shunt voltage, single
  2 = Bus voltage, single
  3 = Shunt and bus, single
  4 = Power-down
  5 = Shunt voltage, continuous
  6 = Bus voltage, continuous
  7 = Shunt and bus, continuous  ← Default

AVG (Bit 11-9):
  000 = 1 sample
  001 = 4 samples
  010 = 16 samples
  011 = 64 samples
  100 = 128 samples
  101 = 256 samples
  110 = 512 samples
  111 = 1024 samples

VBUS CT (Bits 8-6):
  000 = 140µs
  001 = 204µs
  010 = 332µs
  011 = 588µs
  100 = 1.1ms  ← Default
  101 = 2.1ms
  110 = 4.1ms
  111 = 8.2ms

VSH CT (Bits 5-3):
  000 = 140µs
  001 = 204µs
  010 = 332µs
  011 = 588µs
  100 = 1.1ms  ← Default
  101 = 2.1ms
  110 = 4.1ms
  111 = 8.2ms

RST (Bit 3):
  0 = Normal operation
  1 = Device reset (self-clearing)

OPER (Bit 2-0):
  000 = Power-down
  001 = Shunt voltage, triggered
  010 = Bus voltage, triggered
  011 = Shunt and bus, triggered
  100 = Power-down
  101 = Shunt voltage, continuous
  110 = Bus voltage, continuous
  111 = Shunt and bus, continuous  ← Default
```

### Current Register (0x01)
```
┌─────────────────────────────────────────────────────────────────────────┐
│ Bit 15-0 │ Current reading (signed, 2's complement)                   │
└──────────┴─────────────────────────────────────────────────────────────┘

Resolution: 1.25mA/LSB (1.25e-3)
Current Range: 0-15A (typical)
Conversion: Current (A) = raw_value * 0.00125
```

### Bus Voltage Register (0x02)
```
┌─────────────────────────────────────────────────────────────────────────┐
│ Bit 15-0 │ Bus voltage reading (unsigned)                             │
└──────────┴─────────────────────────────────────────────────────────────┘

Resolution: 1.25mV/LSB (1.25e-3)
Voltage Range: 0-36V
Conversion: Voltage (V) = raw_value * 0.00125
```

### Power Register (0x03)
```
┌─────────────────────────────────────────────────────────────────────────┐
│ Bit 15-0 │ Power reading (unsigned)                                   │
└──────────┴─────────────────────────────────────────────────────────────┘

Resolution: 10mW/LSB (10.0e-3)
Power Range: 0-540W
Conversion: Power (W) = raw_value * 0.01
```

---

## Measurement Calculations

### Raw Value to Engineering Units

#### 1. Voltage Calculation
```c
int16_t raw_voltage;
float voltage_V;

// Read register
INA260_ReadReg16(INA260_REG_BUSVOLTAGE, &raw_voltage);

// Convert to voltage (V)
voltage_V = raw_voltage * INA260_VOLTAGE_LSB;  // 1.25mV per LSB

// Example:
// raw_voltage = 2640
// voltage_V = 2640 * 0.00125 = 3.30V
```

#### 2. Current Calculation
```c
int16_t raw_current;
float current_A;

// Read register
INA260_ReadReg16(INA260_REG_CURRENT, &raw_current);

// Convert to current (A)
current_A = raw_current * INA260_CURRENT_LSB;  // 1.25mA per LSB

// Example:
// raw_current = 400
// current_A = 400 * 0.00125 = 0.50A
```

#### 3. Power Calculation
```c
int16_t raw_power;
float power_W;

// Read register
INA260_ReadReg16(INA260_REG_POWER, &raw_power);

// Convert to power (W)
power_W = (uint16_t)raw_power * INA260_POWER_LSB;  // 10mW per LSB

// Example:
// raw_power = 165
// power_W = 165 * 0.01 = 1.65W
```

### Validation Tests

```c
// Test 1: Verify sensor detection
if (INA260_Init(&hi2c1) != HAL_OK) {
    printf("INA260 not detected! Check I2C wiring\n");
}

// Test 2: Verify valid readings
INA260_Data_t test;
if (INA260_Read(&test) == HAL_OK && test.valid) {
    // Check for reasonable values
    if (test.voltage_V > 0.1f && test.voltage_V < 36.0f) {
        printf("Valid voltage: %.2fV\n", test.voltage_V);
    }
}

// Test 3: Check ID registers
int16_t manufacturer_id;
INA260_ReadReg16(INA260_REG_MFR_ID, &manufacturer_id);
// Manufacturer ID should be 0x5449 (TI)

int16_t die_id;
INA260_ReadReg16(INA260_REG_DIE_ID, &die_id);
// Die ID should be 0x227E (for INA260)
```

---

## Error Handling

### Error Types

| Error Code | Cause | Recovery |
|------------|-------|----------|
| **HAL_OK** | Successful read | Continue normal operation |
| **HAL_ERROR** | I2C communication error | Check wiring, power cycle |
| **HAL_TIMEOUT** | I2C bus stuck | Reset I2C peripheral |
| **valid=0** | Sensor read failed | Retry, increment errorCount |

### Error Recovery Flow
```
INA260_Read()
    │
    ├── Read Voltage Register
    │       │
    │       ├── Success → Continue
    │       └── Failure → goto fail
    │
    ├── Read Current Register
    │       │
    │       ├── Success → Continue
    │       └── Failure → goto fail
    │
    ├── Read Power Register
    │       │
    │       ├── Success → Continue
    │       └── Failure → goto fail
    │
    ├── Success: valid=1, return HAL_OK
    │
    └── Fail: valid=0, errorCount++, return HAL_ERROR
```

### Error Counting

```c
// Error counter increments on every failure
data->errorCount++;

// Use for diagnostics
if (data->errorCount > 5) {
    printf("INA260: %lu consecutive errors\n", data->errorCount);
}

// Reset error count on successful read
if (INA260_Read(&ina_data) == HAL_OK) {
    if (ina_data.errorCount > 0) {
        printf("INA260 recovered (was %lu errors)\n", ina_data.errorCount);
    }
    ina_data.errorCount = 0;  // Reset after success
}
```

### I2C Recovery Procedures

```c
// 1. Re-initialize I2C
void Recovery_I2C_Init(void) {
    HAL_I2C_DeInit(&hi2c1);
    HAL_I2C_Init(&hi2c1);
    osDelay(10);
}

// 2. Software reset of INA260
void Recovery_Reset_INA260(void) {
    uint16_t config = 0x8000;  // Reset bit
    HAL_I2C_Mem_Write(&hi2c1, INA260_I2C_ADDR,
                      INA260_REG_CONFIG, I2C_MEMADD_SIZE_8BIT,
                      (uint8_t*)&config, 2, 100);
    osDelay(10);
}

// 3. Full recovery
void Recovery_Full(void) {
    Recovery_I2C_Init();
    Recovery_Reset_INA260();
    INA260_Init(&hi2c1);
}
```

---

## Integration with FreeRTOS

### Task Creation

```c
// INA260 Task
static osThreadId_t ina260TaskHandle;

void CreateINA260Task(void) {
    const osThreadAttr_t inaAttr = {
        .name = "INA260Task",
        .priority = osPriorityNormal,
        .stack_size = 256 * 4  // 1024 bytes
    };
    ina260TaskHandle = osThreadNew(INA260_Task, NULL, &inaAttr);
}

static void INA260_Task(void *argument) {
    INA260_Data_t local = {0};
    TickType_t lastWake = xTaskGetTickCount();
    
    for (;;) {
        if (INA260_Read(&local) == HAL_OK) {
            // Update shared data with mutex
            osMutexAcquire(g_inaMutex, osWaitForever);
            g_inaData = local;
            osMutexRelease(g_inaMutex);
        }
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
    }
}
```

### Mutex Protection

```c
// Shared data mutex
osMutexId_t g_inaMutex;

// In initialization
g_inaMutex = osMutexNew(NULL);

// In other tasks (reading data)
INA260_Data_t local;
osMutexAcquire(g_inaMutex, osWaitForever);
local = g_inaData;  // Copy shared data
osMutexRelease(g_inaMutex);

// Use local data
process_ina_data(local);
```

### Synchronization with Other Tasks

```c
// UARTTask reads INA260 data every 500ms
void UART_Task(void *argument) {
    INA260_Data_t local;
    
    for (;;) {
        // Use mutex to read shared data
        osMutexAcquire(g_inaMutex, osWaitForever);
        local = g_inaData;
        osMutexRelease(g_inaMutex);
        
        // Build packet with latest data
        build_telemetry_packet(&local);
        send_packet();
        
        osDelay(500);
    }
}
```

---

## Debugging & Troubleshooting

### Debug Output

```c
// Enable debug prints
#define INA260_DEBUG

#ifdef INA260_DEBUG
#define INA260_PRINT(fmt, ...) \
    printf("[INA260] " fmt, ##__VA_ARGS__)
#else
#define INA260_PRINT(fmt, ...)
#endif

// Usage in driver:
INA260_PRINT("Read attempt\n");
INA260_PRINT("Raw: V=%d, I=%d, P=%d\n", raw_v, raw_i, raw_p);
INA260_PRINT("V=%.2fV, I=%.3fA, P=%.2fW\n", voltage, current, power);
```

### I2C Debugging

```c
// Check I2C status
void Check_I2C_Status(void) {
    printf("I2C State: %d\n", hi2c1.State);
    printf("I2C Error Code: %d\n", hi2c1.ErrorCode);
}

// Force I2C reset
void Reset_I2C(void) {
    __HAL_RCC_I2C1_FORCE_RESET();
    __HAL_RCC_I2C1_RELEASE_RESET();
    HAL_I2C_Init(&hi2c1);
}
```

### Common Issues & Solutions

| Issue | Symptom | Solution |
|-------|---------|----------|
| **No I2C Response** | `HAL_ERROR` always | Check wiring, power, address, pull-ups |
| **Invalid Readings** | Values jump randomly | Check power supply, I2C noise |
| **I2C Bus Stuck** | `HAL_TIMEOUT` | Reset I2C, check SDA/SCL lines |
| **Intermittent Errors** | Random failures | Reduce I2C speed (100kHz) |
| **Zero Readings** | All values = 0 | Check shunt resistor, load connection |

### I2C Bus Troubleshooting

```c
// 1. Verify I2C device presence
HAL_StatusTypeDef Check_Device_Ready(void) {
    return HAL_I2C_IsDeviceReady(&hi2c1, INA260_I2C_ADDR, 3, 100);
}

// 2. Scan I2C bus for devices
void Scan_I2C_Bus(void) {
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 3, 10) == HAL_OK) {
            printf("Device found at 0x%02X\n", addr);
        }
    }
}

// 3. Check I2C pins
void Check_I2C_Pins(void) {
    printf("SCL: %d, SDA: %d\n",
           HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6),
           HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7));
}
```

---

## Performance Optimization

### 1. Reduce I2C Transactions
```c
// Single read of all registers (optimized)
typedef struct {
    int16_t voltage;
    int16_t current;
    int16_t power;
} INA260_AllRegisters_t;

HAL_StatusTypeDef INA260_ReadAll(INA260_AllRegisters_t *regs) {
    // Read all 3 registers in one transaction
    // ... implementation ...
}
```

### 2. Use DMA for I2C
```c
// Enable DMA for I2C transfers
// Reduces CPU usage significantly
// Configure in CubeMX:
// - I2C1 DMA Settings: TX and RX
// - DMA: Normal mode, Byte width
```

### 3. Reduce Conversion Time
```c
// Faster conversion (less averaging)
// Configuration Register value:
// 0x4127 = 1 sample, 1.1ms conversion
// 0x5127 = 4 samples, 1.1ms conversion
// 0x6127 = 16 samples, 1.1ms conversion (default)

uint16_t fast_config = 0x4127;  // Fastest mode
HAL_I2C_Mem_Write(&hi2c1, INA260_I2C_ADDR, INA260_REG_CONFIG,
                  I2C_MEMADD_SIZE_8BIT, (uint8_t*)&fast_config, 2, 100);
```

### 4. Batch Mode Reading
```c
// Read all registers in single I2C transaction
// Uses less bus overhead
// Implement using I2C sequential read
```

### 5. Compiler Optimizations
```makefile
# In Makefile
OPT = -Os        # Optimize for size
# or
OPT = -O2        # Optimize for speed

# Enable floating point optimizations
CFLAGS += -ffast-math -fno-math-errno
```

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2026-07-09 | Initial release |
| 1.0.1 | - | Added error counting |
| 1.1.0 | - | Added configuration options |
| 2.0.0 | - | Added DMA support |

---

## License

This driver is released under the MIT License.

```
MIT License

Copyright (c) 2026 Pralay2_stm2_sys2

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

---

## References

### Datasheets
- [INA260 Datasheet](https://www.ti.com/lit/ds/symlink/ina260.pdf)
- [STM32F103C8T6 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)

### Application Notes
- [INA260 Configurations](https://www.ti.com/lit/an/sboa262/sboa262.pdf)
- [I2C Communication](https://www.ti.com/lit/an/slva704/slva704.pdf)

---

## Contact & Support

For issues and support, please create an issue on the project repository.

**Last Updated:** 2026-07-09
**Version:** 1.0.0
**Status:** ✅ Production Ready