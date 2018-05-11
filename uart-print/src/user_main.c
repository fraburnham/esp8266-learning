#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"

// this stuff should be auto generated if I'm always gonna need it
#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
#define BIT_RATE_9600            9600

os_event_t    user_procTaskQueue[user_procTaskQueueLen];

// why do the examples have the procTask defined before it appears?

uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void) {
  return 0;
}

// so everything needs to be a task?
// an "os" task (though this is the non-os sdk...)

static void ICACHE_FLASH_ATTR heller(os_event_t *events) {
  os_printf("Hello world!");
  os_delay_us(3000);
  system_os_post(user_procTaskPrio, 0, 0);  // loop by sending a message to self
}

void ICACHE_FLASH_ATTR user_init() {
  uart_init(BIT_RATE_9600, BIT_RATE_9600);

  system_os_task(heller, user_procTaskPrio, user_procTaskQueue, user_procTaskQueueLen);
  system_os_post(user_procTaskPrio, 0, 0);
}
