#include "ets_sys.h"
#include "gpio.h"
#include "osapi.h"
#include "os_type.h"
#include "DHT.h"

uint8 *read_dht(int data_pin) { // pin is probs the wrong type
  static uint8 data[5];  // 40bits of data from dht

  // stop interrupts
  ets_intr_lock();

  // set pin for output & hold high
  gpio_output_set(data_pin, 0, data_pin, 0);
  // keep high for 250ms (give or take)
  os_delay_us(250000);  // this could be part of the issue...
  
  // hold 20ms of low line
  gpio_output_set(0, data_pin, data_pin, 0);
  os_delay_us(20000);

  // keep high for 40us (now we gettin' serious)
  gpio_output_set(data_pin, 0, data_pin, 0);
  os_delay_us(40);
  
  // drop the line & switch to input
  gpio_output_set(0, data_pin, data_pin, 0);
  gpio_output_set(0, 0, 0, data_pin);
  // adafruit sets the pullup here, do I need to?

  // input & wait 10us
  os_delay_us(10);

  // expect data-ready of 80us low then 80us high (approx, probably take anything over 50us...)

  // begin storing data
  // read 8 bit chunks (is there a stop/split bit?)
  // each bit is 50us pulse followed by a 70us (1) pulse or 30us (0) pulse
  // compare the length to the 50us pulse to determine which bit you have

  // restore pin to output & pullup high

  ets_intr_unlock();
  
  return data;
}

bool valid_data(uint8 *data) {
  uint8 parity = 0;

  for(int i=0; i<4; i++) {
    parity += data[i];
  }
  
  return (parity == data[4]);
}

DHTData *parse_dht_data(uint8 *data) {
  static DHTData parsed_data;

  parsed_data.humidity = (data[0] << 8) + data[1];
  parsed_data.temperature = (data[2] << 8) + data[3];
    
  return &parsed_data;
}
