#include <avr/io.h>
#include "one-wire.h"
#include "DS18B20.h"
#include <string.h>

/**
 * Initialize the memory area for the device
 * @param wire1  Pointer to the allocated memory for the device
 */
void initDevice(wire1_ds18b20_t *wire1) {
  wire1->status_init = 0;
}

/**
 * Set the address for a one-wire device
 * @param wire1    Pointer to the allocated memory for the device
 * @param address  0 if OK, otherwise error
 */
void DS18B20_setAddress(wire1_ds18b20_t *wire1, uint8_t *const address) {
  wire1->status_init |= BV(DS18B20_STATUS_ADDRESS_BIT);
  if (address[0] == DS18B20 && crc8(0, W1_CRC_POLYNOMIAL, address, 7) == address[7]) {
    // TODO: Check that the address is valid by accessing it?
    memcpy(&address[1], wire1->address, 6);
    wire1->status |= BV(DS18B20_STATUS_ADDRESS_BIT);
  } else {
    return -1;
  }
}

/**
 * Reads the status of the device if not already read
 * @param  wire1  Pointer to the allocated memory for the device
 * @return        1 if it uses parasite power, 0 if not, -1 if error
 */
uint8_t usesParasitePower(wire1_ds18b20_t *wire1) {
  if (wire1->status_init & BV(W1_STATUS_PARASITE_POWER_BIT)) {
    return wire1->status & BV(W1_STATUS_PARASITE_POWER_BIT);
  } else {
    wire1MatchROM(wire1->address);
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
 * Selects the one-wire device and then sets the resolution for it. Returns
 * status to show if the access was successful.
 *
 * @param  wire1       Pointer to the device to address
 * @param  resolution  The resolution to use (in bits). Can be between
 *                     9 and 12. Any value outside the range will be capped.
 * @return             0 if the access was successful, nonzero otherwise.
 */
uint8_t setResolution(wire1_ds18b20_t *wire1, uint8_t resolution) {
  if (resolution > 12) {
    resolution = 12;
  } else if (resolution < 9) {
    resolution = 9;
  }
  resolution = resolution - 9;
  wire1MatchROM(wire1->address);
  //wire1WriteByte();
}