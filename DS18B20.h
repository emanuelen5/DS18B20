
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
} wire1_ds18b20_t;

#define DS18B20_STATUS_RESOLUTION_BIT_1     6
#define DS18B20_STATUS_RESOLUTION_BIT_0     5
#define DS18B20_STATUS_CONV_STARTED_BIT     2
#define DS18B20_STATUS_PARASITE_POWER_BIT   1
#define DS18B20_STATUS_ADDRESS_BIT          0

#define DS18B20_SCRATCHPAD_BYTE_TEMP_LSB   0
#define DS18B20_SCRATCHPAD_BYTE_TEMP_MSB   1
#define DS18B20_SCRATCHPAD_BYTE_ALARM_TH   2
#define DS18B20_SCRATCHPAD_BYTE_ALARM_TL   3
#define DS18B20_SCRATCHPAD_BYTE_CONF       4

#define DS18B20_FUNC_COPY_SCRATCHPAD      0x48
#define DS18B20_FUNC_READ_SCRATCHPAD      0xBE
#define DS18B20_FUNC_WRITE_SCRATCHPAD     0x4E
#define DS18B20_FUNC_START_CONV           0x44
#define DS18B20_FUNC_PARASITE_POWER       0xB4
#define DS18B20_FUNC_RECALL_E2            0xB8

int8_t DS18B20_init(wire1_ds18b20_t *wire1, uint8_t *const address);
int8_t  DS18B20_usesParasitePower(wire1_ds18b20_t *wire1,
                                  uint8_t *usesParasitePower);
void    DS18B20_setAlarmTempH(wire1_ds18b20_t *wire1, int8_t tempH);
void    DS18B20_setAlarmTempL(wire1_ds18b20_t *wire1, int8_t tempL);
uint8_t DS18B20_getResolution(wire1_ds18b20_t *const wire1);
void    DS18B20_setResolution(wire1_ds18b20_t *wire1, uint8_t resolution);
int8_t  DS18B20_readScratchpad(wire1_ds18b20_t *const wire1);
int8_t  DS18B20_writeScratchpad(wire1_ds18b20_t *const wire1);
int8_t  DS18B20_convertTemperature(wire1_ds18b20_t *const wire1);
int8_t  DS18B20_readTemperature(wire1_ds18b20_t *const wire1, 
                                uint16_t *temp);