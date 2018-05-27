#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "user_config.h"

#define BIT_RATE_9600 9600

// Set the 5th sector from the end of the flash to store the RF_CAL parameter.
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set (void) {
  enum flash_size_map size_map = system_get_flash_size_map();
  uint32 rf_cal_sec = 0;

  switch (size_map) {
  case FLASH_SIZE_4M_MAP_256_256:
    rf_cal_sec = 128 - 5;
    break;
    
  case FLASH_SIZE_8M_MAP_512_512:
    rf_cal_sec = 256 - 5;
    break;
    
  case FLASH_SIZE_16M_MAP_512_512:
  case FLASH_SIZE_16M_MAP_1024_1024:
    rf_cal_sec = 512 - 5;
    break;
    
  case FLASH_SIZE_32M_MAP_512_512:
  case FLASH_SIZE_32M_MAP_1024_1024:
    rf_cal_sec = 512 - 5;
    break;
    
  case FLASH_SIZE_64M_MAP_1024_1024:
    rf_cal_sec = 2048 - 5;
    break;
    
  case FLASH_SIZE_128M_MAP_1024_1024:
    rf_cal_sec = 4096 - 5;
    break;
    
  default:
    rf_cal_sec = 0;
    break;
  }
  
  return	rf_cal_sec;
}

/*
typedef void (* wifi_promiscuous_cb_t)(uint8 *buf, uint16 len);

void wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb);
*/
void ICACHE_FLASH_ATTR
packet_callback(uint8 *buf, uint16 len) {
  // do I need to malloc space here? What is the format of the buffer?
  os_printf("%d, %d\n", buf, len);
  return;
}

void ICACHE_FLASH_ATTR
post_init () {
  wifi_station_disconnect();
  
  os_delay_us(65535);
  os_delay_us(65535);
  os_delay_us(65535);
  os_delay_us(65535);
  os_delay_us(65535);
  os_delay_us(65535);
  os_delay_us(65535);
  os_delay_us(65535);
  os_delay_us(65535);
  os_delay_us(65535);
  os_delay_us(65535);

  wifi_set_promiscuous_rx_cb(packet_callback);
  
  wifi_promiscuous_enable(1);
  os_printf("Enabled promiscuous mode\n");
  
  if(wifi_set_channel(6)) {
    os_printf("Listening on chan 6\n");
  } else {
    os_printf("Failed to set channel");
  }
}

void ICACHE_FLASH_ATTR
user_init () {
  uart_init(BIT_RATE_9600, BIT_RATE_9600);

  wifi_set_opmode(STATION_MODE);
  
  system_init_done_cb(post_init);  
}
