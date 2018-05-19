#define PULSE_START 0
#define PULSE_END   1
#define DATA_LEN    41

typedef struct DHTData {
  uint8 temp;
  uint8 temp_frac;
  uint8 humidity;
  uint8 humidity_frac;
} DHTData;

extern DHTData *read_dht(int data_pin);
