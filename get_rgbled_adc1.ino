#define SKETCH_DESC "TEST get board name, GPIO_NUM of RGBLED and ADC1"
#define VERSION "20250629"
#include "M5Unified.h"
#include "EspEasyLED.h"
#include <string.h>
static constexpr const m5::board_t  board_index[] = {
  m5::board_t::board_M5AtomLite,
  m5::board_t::board_M5AtomS3Lite,
  m5::board_t::board_M5StampC3U,
  m5::board_t::board_M5StampPico,
  m5::board_t::board_unknown,
};

String board_name[] = {
  "AtomLite",
  "AtomS3Lite",
  "StampC3U",
  "StampPico",
  "unknown",
};

static constexpr const uint8_t pin_table[][2] = {  // RGBLED, ADC1
  // GPIO_NUM_nn may cause compile error when compiling for C3U
  {27, 32}, // M5AtomLite
  {35, 4},  // M5AtomS3Lite
  {2, 4},   // M5StampC3U
  {27, 32}, // M5StampPico
  {99, 99}, // for unknown board
};

String get_board_name(m5::board_t board) {
  int i;
  for (i = 0; i < sizeof(board_index); i++) {
    if (board_index[i] == board) break;
  }
  return board_name[i];
}

gpio_num_t get_gpio_num_rgbled(m5::board_t board) {
  int i;
  for (i = 0; i < sizeof(board_index); i++) {
    if (board_index[i] == board) break;
  }
  return (gpio_num_t) pin_table[i][0];
}

gpio_num_t get_gpio_num_adc1(m5::board_t board) {
  int i;
  for (i = 0; i < sizeof(board_index); i++) {
    if (board_index[i] == board) break;
  }
  return (gpio_num_t) pin_table[i][1];
}


void setup() {
  m5::board_t board;
  String board_name;
  gpio_num_t gpio_num_rgbled;
  gpio_num_t gpio_num_adc1;

  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  delay(2000);
  Serial.println(SKETCH_DESC);
  Serial.println(VERSION);

  board = M5.getBoard();
  board_name = get_board_name(board);
  gpio_num_rgbled = get_gpio_num_rgbled(board);
  gpio_num_adc1 = get_gpio_num_adc1(board);
  Serial.printf("Board %s, RGBLED GPIO:%d, ADC1 GPIO:%d\n",\
    board_name, gpio_num_rgbled, gpio_num_adc1);
}

void loop() {
}

