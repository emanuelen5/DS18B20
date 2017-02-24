// For pin definitions
#include <avr/io.h>
// For general one-wire functions
#include "one-wire.h"
#include "DS18B20.h"
// For memcpy
#include <string.h>

#define DS18B20_FUNC_COPY_SCRATCHPAD      0x48
#define DS18B20_FUNC_READ_SCRATCHPAD      0xBE
#define DS18B20_FUNC_WRITE_SCRATCHPAD     0x4E
#define DS18B20_FUNC_START_CONV           0x44
#define DS18B20_FUNC_PARASITE_POWER       0xB4
#define DS18B20_FUNC_RECALL_E2            0xB8

#define DS18B20_SELECT_DEVICE_CHECK  if (wire1MatchROM(wire1->address)) {return -1;}

/**
 * Initialize the memory area for the device and set the address.
 * Relies on user to find the address with the general one-wire library,
 * and then create objects depending on the returned device type ID.
 *
 * @param wire1    Pointer to the allocated memory for the device
 * @param address  0 if OK, otherwise error
 */
int8_t DS18B20_init(wire1_ds18b20_t *wire1, uint8_t *const address) {
  if (address[0] != DS18B20 || crc8(0, W1_CRC_POLYNOMIAL, address, 7) != address[7]) {
    return -1;
  } else {
    // TODO: Check that the address is valid by accessing it?
    memcpy(&address[1], wire1->address, 6);
    wire1->status |= BV(DS18B20_STATUS_ADDRESS_BIT);
    wire1->status_init |= BV(DS18B20_STATUS_ADDRESS_BIT);
    return 0;
  }
}

/**
 * Reads the status of the device if not already read
 * @param  wire1  Pointer to the allocated memory for the device
 * @return        1 if it uses parasite power, 0 if not, -1 if error
 */
int8_t DS18B20_usesParasitePower(wire1_ds18b20_t *wire1) {
  if (wire1->status_init & BV(W1_STATUS_PARASITE_POWER_BIT)) {
    return wire1->status & BV(W1_STATUS_PARASITE_POWER_BIT);
  } else {
    DS18B20_SELECT_DEVICE_CHECK;
	wire1WriteByte(DS18B20_FUNC_PARASITE_POWER);
    uint8_t parasitePower = wire1ReadPowerSuppy();
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
 * Selects the one-wire device and then sets the resolution for it. Returns
 * status to show if the access was successful.
 *
 * @param  wire1       Pointer to the device to address
 * @param  resolution  The resolution to use (in bits). Can be between
 *                     9 and 12. Any value outside the range will be capped. * @return             0 if the access was successful, nonzero otherwise.
 */
int8_t DS18B20_setResolution(wire1_ds18b20_t *wire1, uint8_t resolution) {
  if (resolution > 12) {
    resolution = 12;
  } else if (resolution < 9) {
    resolution = 9;
  }
  resolution = resolution - 9;
  DS18B20_SELECT_DEVICE_CHECK;
  //wire1WriteByte();
  return 0;
}

/**
 * Reads the scratchpad (8 byte) of a device.
 * 
 * Verifies the CRC on read and then checks that they coincide.
 * @param wire1  Pointer to the device to address
 * @return       0 if OK; -1 if Match ROM failed, 1 if CRC failed
 */
int8_t DS18B20_readScratchpad(wire1_ds18b20_t *const wire1) {
  DS18B20_SELECT_DEVICE_CHECK;
  wire1WriteByte(0xBE);
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
 * Starts a temperature conversion for a thermometer
 * 
 * @param wire1  Pointer to the device to address
 * @return       0 if OK; -1 if Match ROM failed
 */
int8_t DS18B20_convertTemperature(wire1_ds18b20_t *const wire1) {
	DS18B20_SELECT_DEVICE_CHECK;
	wire1WriteByte(DS18B20_FUNC_START_CONV);
	// Wait until temperature conversion is complete (could wait with this until next 1-wire command is fired)
	while (wire1ReadBit());
	return 0;
}

int8_t DS18B20_readTemperature(wire1_ds18b20_t *const wire1) {
	DS18B20_SELECT_DEVICE_CHECK;
	return 0;
}