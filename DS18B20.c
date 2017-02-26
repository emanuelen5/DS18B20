// For pin definitions
#include <avr/io.h>
// For general one-wire functions
#include "one-wire.h"
#include "DS18B20.h"
// For memcpy
#include <string.h>

#define DS18B20_SELECT_DEVICE_CHECK(device) \
          {\
            uint8_t _ds_address[8];   \
            _ds_address[0] = DS18B20; \
            memcpy(&_ds_address[1], device->address, 6);   \
            _ds_address[W1_ADDR_BYTE_CRC] = \
              crc8(0x00, W1_CRC_POLYNOMIAL, _ds_address, 7); \
            if (wire1MatchROM(_ds_address)) \
              return -1; \
          }

/**
 * Initialize the memory area for the device and set the address.
 * Relies on user to find the address with the general one-wire library,
 * and then create objects depending on the returned device type ID.
 *
 * @param wire1    Pointer to the allocated memory for the device
 * @param address  0 if OK, otherwise error
 */
int8_t DS18B20_init(wire1_ds18b20_t *wire1, uint8_t *const address) {
  if (address[W1_ADDR_BYTE_DEV_TYPE] != DS18B20 || 
      crc8(0, W1_CRC_POLYNOMIAL, address, 7) != address[W1_ADDR_BYTE_CRC]) {
    return -1;
  } else {
    memcpy(wire1->address, &address[1], 6);
    wire1->status      |= BV(DS18B20_STATUS_ADDRESS_BIT);
    wire1->status_init |= BV(DS18B20_STATUS_ADDRESS_BIT);

    // Start by reading the scratchpad to make sure that all values are updated,
    // such as resolution.
    int8_t rdError;
    if ((rdError = DS18B20_readScratchpad(wire1))) {
      return rdError;
    }
    return 0;
  }
}

/**
 * Reads the status of the device if not already read.
 * @param  wire1          Pointer to the device to address
 * @param  parasitePower  Pointer to where to store the boolean result
 * @return                0 if OK; -1 if Match ROM failed
 */
int8_t DS18B20_usesParasitePower(wire1_ds18b20_t *wire1, 
                                 uint8_t *usesParasitePower) {
  if (wire1->status_init & BV(W1_STATUS_PARASITE_POWER_BIT)) {
    return wire1->status & BV(W1_STATUS_PARASITE_POWER_BIT);
  } else {
    DS18B20_SELECT_DEVICE_CHECK(wire1);
    wire1WriteByte(DS18B20_FUNC_PARASITE_POWER);
    uint8_t parasitePower = wire1ReadPowerSupply();
    wire1->status_init |= BV(W1_STATUS_PARASITE_POWER_BIT);
    if (parasitePower) {
      wire1->status |=  BV(W1_STATUS_PARASITE_POWER_BIT);
    } else {
      wire1->status &= ~BV(W1_STATUS_PARASITE_POWER_BIT);
    }
    return parasitePower;
  }
}

/**
 * Sets the alarm high temperature without updating the device.
 * @param  wire1  Pointer to the device to address
 * @param  tempH  The temperature to set it to, in whole degrees celsius
 */
void DS18B20_setAlarmTempH(wire1_ds18b20_t *wire1, int8_t tempH) {
  wire1->scratchPad[DS18B20_SCRATCHPAD_BYTE_ALARM_TH] = tempH;
}

/**
 * Sets the alarm low temperature without updating the device.
 * @param  wire1  Pointer to the device to address
 * @param  tempL  The temperature to set it to, in whole degrees celsius
 */
void DS18B20_setAlarmTempL(wire1_ds18b20_t *wire1, int8_t tempL) {
  wire1->scratchPad[DS18B20_SCRATCHPAD_BYTE_ALARM_TL] = tempL;
}

/**
 * Sets the temperature resolution without updating the device.
 * @param  wire1       Pointer to the device to address
 * @param  resolution  The resolution to use, in bits. Can be between 9 and 12.
 *                     Any value outside the range will be capped.
 */
void DS18B20_setResolution(wire1_ds18b20_t *wire1, uint8_t resolution) {
  if (resolution > 12) {
    resolution = 12;
  } else if (resolution < 9) {
    resolution = 9;
  }
  resolution = resolution - 9;
  wire1->status &= ~(BV(DS18B20_STATUS_RESOLUTION_BIT_1) |
                     BV(DS18B20_STATUS_RESOLUTION_BIT_0));
  wire1->status |= resolution << DS18B20_STATUS_RESOLUTION_BIT_0;
}

/**
 * Mask out the resolution bits and return the calculated resolution.
 * @param  wire1  Pointer to the device to address
 * @return        The resolution of the device, in bits (between 9-12)
 */
inline uint8_t DS18B20_getResolution(wire1_ds18b20_t *const wire1) {
  uint8_t tmp = wire1->status & (BV(DS18B20_STATUS_RESOLUTION_BIT_1) |
                                 BV(DS18B20_STATUS_RESOLUTION_BIT_0));
  return (tmp >> DS18B20_STATUS_RESOLUTION_BIT_0) + 9;
}

/**
 * Reads the scratchpad (8 byte) of a device.
 * Verifies the CRC on read and then checks that they coincide.
 * @param wire1  Pointer to the device to address
 * @return       0 if OK; -1 if Match ROM failed, 1 if CRC failed
 */
int8_t DS18B20_readScratchpad(wire1_ds18b20_t *const wire1) {
  DS18B20_SELECT_DEVICE_CHECK(wire1);
  wire1WriteByte(DS18B20_FUNC_READ_SCRATCHPAD);
  for (int i = 0; i < 8; i++) {
    wire1->scratchPad[i] = wire1ReadByte();
  }
  uint8_t ownCRC = crc8(0, W1_CRC_POLYNOMIAL,
  wire1->scratchPad, 8);

  // Read CRC in 9th byte and compare
  if (ownCRC == wire1ReadByte()) {
    return 0;
  } else {
    return 1;
  }
}

/**
 * Writes to the scratchpad of the device (alarm high/low temperature and
 * resolution)
 *
 * @param wire1  Pointer to the device to address
 * @return       0 if OK; -1 if Match ROM failed
 */
int8_t DS18B20_writeScratchpad(wire1_ds18b20_t *const wire1) {
  DS18B20_SELECT_DEVICE_CHECK(wire1);
  wire1WriteByte(DS18B20_FUNC_WRITE_SCRATCHPAD);
  for (int i  = DS18B20_SCRATCHPAD_BYTE_ALARM_TH;
           i <= DS18B20_SCRATCHPAD_BYTE_CONF; i++) {
    wire1WriteByte(wire1->scratchPad[i]);
  }
  return 0;
}

/**
 * Starts a temperature conversion for a thermometer
 * @param wire1  Pointer to the device to address
 * @return       0 if OK; -1 if Match ROM failed
 */
int8_t DS18B20_convertTemperature(wire1_ds18b20_t *const wire1) {
  DS18B20_SELECT_DEVICE_CHECK(wire1);
  wire1WriteByte(DS18B20_FUNC_START_CONV);
  // Update status to force a re-read the temperature
  wire1->status |= BV(DS18B20_STATUS_CONV_STARTED_BIT);

  /**
   * Temperature conversion time = 93.75 ms * 2^(resolution - 9) ~= 
   * 95 us * 2^(resolution + 1) = 97.28 ms
   */
  wire1SetupPoll4Idle(BV(DS18B20_getResolution(wire1) + 1));
  return 0;
}

/**
 * Reads the temperature from the thermometer if a new conversion has been
 * requested. Otherwise, it just returns the last read temperature.
 * 
 * @param  wire1  Pointer to the device to address
 * @return        0 if OK; -1 if Match ROM failed
 */
int8_t DS18B20_readTemperature(wire1_ds18b20_t *const wire1, 
  uint16_t *temperature) {
  int8_t rdError;
  if (wire1->status & BV(DS18B20_STATUS_CONV_STARTED_BIT) && 
      (rdError = DS18B20_readScratchpad(wire1))) {
    return rdError;
  }
  *temperature = wire1->scratchPad[DS18B20_SCRATCHPAD_BYTE_TEMP_LSB];
  return 0;
}