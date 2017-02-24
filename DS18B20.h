
typedef struct {
  /** Address of the device. Byte 1 from type, byte 7 from CRC */
  uint8_t address[6];
  /**
   * A bit field specifying the cached status of the device
   * Bits represent boolean values:
   *   [7:2] reserved
   *   [1] Parasize power
   *   [0] Initialized
   */
  uint8_t status, status_init;
  uint8_t scratchPad[8];
  uint8_t TL, TH;
} wire1_ds18b20_t;

#define DS18B20_STATUS_RESOLUTION_BIT_1     6
#define DS18B20_STATUS_RESOLUTION_BIT_0     5
#define DS18B20_STATUS_CONV_READY_BIT       2
#define DS18B20_STATUS_PARASITE_POWER_BIT   1
#define DS18B20_STATUS_ADDRESS_BIT          0

int8_t  DS18B20_initDevice(wire1_ds18b20_t *wire1);
int8_t  DS18B20_usesParasitePower(wire1_ds18b20_t *wire1);
int8_t  DS18B20_setResolution(wire1_ds18b20_t *wire1, uint8_t resolution);
int8_t  DS18B20_readScratchpad(wire1_ds18b20_t *const wire1);
int8_t  DS18B20_convertTemperature(wire1_ds18b20_t *const wire1);