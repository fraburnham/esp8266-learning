#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "DHT.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
#define BIT_RATE_9600            9600
#define DATA_PIN                 BIT5

os_event_t user_procTaskQueue[user_procTaskQueueLen];

static void user_procTask(os_event_t *events);
// lots of warns that other code doesn't give a shit that this is volatile
static volatile os_timer_t some_timer;

void some_timerfunc(void *arg) {
  // src/user_main.c:18:21: warning: initialization makes pointer from integer without a cast [enabled by default]
  uint8 *raw_data = (uint8*)read_dht(DATA_PIN);  // why am I warned to cast things w/ matching types?
  
  if(!valid_data(raw_data)) {
    os_printf("Data invalid! Bad read!\n");
  }

  // src/user_main.c:24:19: warning: initialization makes pointer from integer without a cast [enabled by default]
  DHTData *data = (DHTData*)parse_dht_data(raw_data);  // why am I warned to cast things w/ matching types?
  
  os_printf("Temp: %d\n", data->temperature);
  os_printf("RH: %d\n", data->humidity);
}

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
  return 0;
}

// Do I need this?
static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events)
{
    os_delay_us(10);
}

void ICACHE_FLASH_ATTR user_init() {
  uart_init(BIT_RATE_9600, BIT_RATE_9600);

  gpio_init();
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
  gpio_output_set(DATA_PIN, 0, DATA_PIN, 0);
  PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO5_U);

  // src/user_main.c:46:3: warning: passing argument 1 of 'ets_timer_disarm' discards 'volatile' qualifier from pointer target type [enabled by default]
  os_timer_disarm(&some_timer);
  os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);
  os_timer_arm(&some_timer, 5000, 1);

  // start junk task
  system_os_task(user_procTask, user_procTaskPrio, user_procTaskQueue, user_procTaskQueueLen);
}

// https://arduining.files.wordpress.com/2015/08/nodemcudevkit_v1-0_io.jpg?w=816&h=9999
