#include "ets_sys.h"
#include "gpio.h"
#include "osapi.h"
#include "os_type.h"
#include "DHT.h"

uint32 bit_pulses[DATA_LEN][2], ref_pulses[DATA_LEN][2];
int pulse_offset;

void
handle_rising_edge_interrupt (uint32 now) {
  bit_pulses[pulse_offset][PULSE_START] = now;
  ref_pulses[pulse_offset][PULSE_END] = now;

  pulse_offset++;
}

void
handle_falling_edge_interrupt (uint32 now) {
  ref_pulses[pulse_offset][PULSE_START] = now;
   
  if (pulse_offset > 0) {
    bit_pulses[pulse_offset-1][PULSE_END] = now;
  }
  
  if (pulse_offset > (DATA_LEN - 1)) {
    gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_DISABLE);
  }
}

void
pulse_interrupt_handler () { // this should get the pin to listen to as an argument and expects to be set for both edges
  uint32 now = system_get_time();
  
  ets_intr_lock();
  uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS); // read interrupt
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status); // clear interrupt flag

  if (gpio_status & BIT5) {  // needs to be passable!
    if (GPIO_INPUT_GET(5)) {
      handle_rising_edge_interrupt(now);
    } else {
      handle_falling_edge_interrupt(now);
    }
  }

  ets_intr_unlock();
}

uint8
wire_data_to_byte (uint32 bit_pulses[8][2], uint32 ref_pulses[8][2]) {
  uint8 byte = 0;

  for (int i=0; i<8; i++) {
    uint32 ref_pulse = ref_pulses[i][PULSE_END] - ref_pulses[i][PULSE_START];
    uint32 bit_pulse = bit_pulses[i][PULSE_END] - bit_pulses[i][PULSE_START];

    byte += ((bit_pulse > ref_pulse) ? 1 : 0) << (8 - (i + 1));
  }

  return byte;
}

void
send_start_signal (int data_pin) {
  gpio_output_set(0, data_pin, data_pin, 0);
  os_delay_us(20000);

  gpio_output_set(0, 0, 0, data_pin);
}

bool
valid_data (uint8 *data) {
  uint8 parity = 0;

  if (data == NULL) {
    return false;
  }

  for (int i=0; i<4; i++) {
    parity += data[i];
  }
  
  return (parity == data[4]);
}

DHTData *
parse_dht_data () {
  static DHTData parsed_data;
  uint8 raw_data[5] = {0};

  for (int i=1; i<40; i=i+8){
    raw_data[((i - 1) / 8)] = wire_data_to_byte((bit_pulses + i), (ref_pulses + i));
  }

  if (!valid_data(raw_data)) {
    return NULL;
  }

  parsed_data.humidity      = raw_data[0];
  parsed_data.humidity_frac = raw_data[1];
  parsed_data.temp          = raw_data[2];
  parsed_data.temp_frac     = raw_data[3];
    
  return &parsed_data;
}

DHTData *
read_dht(int data_pin) {
  static uint8 raw_data[5] = {0};
  pulse_offset = 0;
  memset(bit_pulses, 0, sizeof(bit_pulses));
  memset(ref_pulses, 0, sizeof(ref_pulses));
    
  send_start_signal(data_pin);

  ETS_GPIO_INTR_ATTACH(pulse_interrupt_handler, NULL);
  gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_ANYEDGE);
  ETS_GPIO_INTR_ENABLE();

  // delay for read to complete (what would be a better way?)
  os_delay_us(20000);

  return parse_dht_data();
}
