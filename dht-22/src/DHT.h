typedef struct DHTData {
  int16_t temperature;
  int16_t humidity;  // not sure if these will be signed or not, if the size is small enough it shouldn't matter (right?)
}DHTData;

uint8 *read_dht(int data_pin);
bool valid_data(uint8 *data);
DHTData *parse_dht_data(uint8 *data);
