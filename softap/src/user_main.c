#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"  // needed?
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "user_config.h"

#define BIT_RATE 9600

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

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

static volatile os_timer_t some_timer;

void ICACHE_FLASH_ATTR
some_timerfunc(void *arg) {
  os_printf("Alive!\n");
}

LOCAL void ICACHE_FLASH_ATTR
recv(void *arg, char *pdata, unsigned short len) {
  os_printf("Got some data!\n");

  struct espconn *conn = arg;

  os_printf("\nGot: %s\n", pdata);

  LOCAL char *body = "Hello world!";

  LOCAL char *status_line = "HTTP/1.1 200 OK";
  LOCAL char *server_line = "Server: WIP";
  LOCAL char *content_type = "Content-Type: text/text";
  LOCAL char head[256]; // probs needs protections, innit
  os_sprintf(head,
	     "%s\r\n%s\r\n%s\r\nContent-Length: %d\r\n",
	     status_line, server_line, content_type, os_strlen(body));

  LOCAL char response[512];
  os_sprintf(response,
	     "%s\r\n%s",
	     head, body);

  os_printf("Send status %d\n", espconn_send(conn, response, os_strlen(response)));
}

LOCAL void ICACHE_FLASH_ATTR
recon(void *arg, sint8 err) {
  os_printf("Got reconnect error\n");
}

LOCAL void ICACHE_FLASH_ATTR
discon(void *arg) {
  os_printf("Client disconnect\n");
}

LOCAL void ICACHE_FLASH_ATTR
webserver_listen(void *arg) {
  os_printf("Got connection on port 80!\n");

  // the example registers some failure callbacks and a data callback
  struct espconn *conn = arg;

  espconn_regist_recvcb(conn, &recv);
  espconn_regist_reconcb(conn, &recon);
  espconn_regist_disconcb(conn, &discon);
}

void ICACHE_FLASH_ATTR
post_init_setup() {
  //enable softap
  if(!wifi_set_opmode(SOFTAP_MODE)) {
    os_printf("Failed to set wifi opmode!!\n");
  }
  
  // setup softap config
  struct softap_config config = {
    .authmode = AUTH_WPA2_PSK,
    .password = "youshouldchangethis",
    .ssid = "Skytemp 00",
    .ssid_len = 10,
    .channel = 2,
    .max_connection = 1
  };
  
  if(!wifi_softap_set_config(&config)) {
    os_printf("Failed to set softap config!\n");
  }

  // setup as softap and see what data the callback gets after `espconn_accept`...
  LOCAL esp_tcp http_connection = {
    .local_port = 80,
  };
  
  LOCAL struct espconn esp_connection = {
    .type = ESPCONN_TCP,
    .proto.tcp = &http_connection,
    .state = ESPCONN_NONE
  };

  if(espconn_regist_connectcb(&esp_connection, &webserver_listen) != 0) {
    os_printf("Failed to register connect callback!\n");
  }
  
  if(espconn_accept(&esp_connection) != 0) {
    os_printf("Failed to setup espconn!\n");
  }
}

void ICACHE_FLASH_ATTR
user_init() {
  // do some delay (maybe) and output the current station mode
  // then try to adjust it and what have you
  // eventually try to connect to the house AP
  uart_init(BIT_RATE, BIT_RATE);

  //Disarm timer
  os_timer_disarm(&some_timer);

  //Setup timer
  os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);

  //Arm the timer
  //&some_timer is the pointer
  //1000 is the fire time in ms
  //0 for once and 1 for repeating
  os_timer_arm(&some_timer, 20000, 1);

  system_init_done_cb(&post_init_setup);
}
