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

struct RxControl {
 signed rssi:8; // signal intensity of packet
 unsigned rate:4;
 unsigned is_group:1;
 unsigned:1;
 unsigned sig_mode:2; // 0:is not 11n packet; non-0:is 11npacket;
 unsigned legacy_length:12; // if not 11n packet, shows length of packet.
 unsigned damatch0:1;
 unsigned damatch1:1;
 unsigned bssidmatch0:1;
 unsigned bssidmatch1:1;
 unsigned MCS:7; // if is 11n packet, shows the modulation
 // and code used (range from 0 to 76)
 unsigned CWB:1; // if is 11n packet, shows if is HT40 packet or not
 unsigned HT_length:16;// if is 11n packet, shows length of packet.
 unsigned Smoothing:1;
 unsigned Not_Sounding:1;
 unsigned:1;
 unsigned Aggregation:1;
 unsigned STBC:2;
 unsigned FEC_CODING:1; // if is 11n packet, shows if is LDPC packet or not.
 unsigned SGI:1;
 unsigned rxend_state:8;
 unsigned ampdu_cnt:8;
 unsigned channel:4; //which channel this packet in.
 unsigned:12;
};

typedef struct LenSeq{
 u16 len; // length of packet
 u16 seq; // serial number of packet, the high 12bits are serial number,
 // low 4 bits are Fragment number (usually be 0)
 u8 addr3[6]; // the third address in packet
}LenSeq;

typedef struct sniffer_buf{
 struct RxControl rx_ctrl;
 u8 buf[36]; // head of ieee80211 packet
 u16 cnt; // number count of packet
 struct LenSeq lenseq[1]; //length of packet
}sniffer_buf;

void ICACHE_FLASH_ATTR
packet_callback(uint8 *buf, uint16 len) {
  // do I need to malloc space here? What is the format of the buffer?
  // https://www.espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf
  // pg 106/chap 14
  // os_printf("%d, %d\n", buf, len);
  //  for(int i=0; i<

  //wifi_promiscuous_enable(0);

  if(len == sizeof(sniffer_buf)) {
    sniffer_buf * sb = (sniffer_buf *)buf;  // all needed info for deauth frame should be here
    // management frames look like 00 00 (version type)
    // 10 00 (subtype: beacon)
    
    /* if(sb->buf[] == 0) { */
    /*   os_printf("Management frame!\n"); */
    /*   // bssid is 14 bytes into the buffer (I think...) */
    /*   // BUT it is in the sniffer_buf2! */
    /*   // double callback? */
    /*   wifi_set_promiscuous_rx_cb(management_frame_callback); */
    /* } */

    /* //    if((count % 20) == 0){ */
    /* for(int i=0; i<6; i++) { */
    /*   os_printf("%02x", sb->lenseq[0].addr3[i]); */
    /* } */

    /* os_printf(","); */
      
    for(int i=0; i<36; i++) {
    	os_printf("%02x", (u8)sb->buf[i]);
    }
    os_printf("\n");
  }
    

  //wifi_promiscuous_enable(1);
  return;
}

void ICACHE_FLASH_ATTR
post_init () {
  wifi_station_disconnect();
  
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
