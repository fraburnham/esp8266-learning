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
  // src/user_main.c:24:19: warning: initialization makes pointer from integer without a cast [enabled by default]
  DHTData *data = (DHTData*)read_dht(DATA_PIN);  // why am I warned to cast things w/ matching types?
  
  os_printf("Temp: %d.%d\n", data->temp, data->temp_frac);
  os_printf("RH: %d.%d\n", data->humidity, data->humidity_frac);
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
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
  gpio_output_set(BIT4, 0, BIT4, 0);
  //PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO5_U); // the pullup is built into my little device thing

  // src/user_main.c:46:3: warning: passing argument 1 of 'ets_timer_disarm' discards 'volatile' qualifier from pointer target type [enabled by default]
  os_timer_disarm(&some_timer);
  os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);
  os_timer_arm(&some_timer, 5000, 1);

  // start junk task
  system_os_task(user_procTask, user_procTaskPrio, user_procTaskQueue, user_procTaskQueueLen);
}

// https://arduining.files.wordpress.com/2015/08/nodemcudevkit_v1-0_io.jpg?w=816&h=9999
