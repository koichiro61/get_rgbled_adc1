#define SKETCH_DESC "TEST get board name, GPIO_NUM of RGBLED and ADC1, active/inactive timer, and blinker_timer"
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

EspEasyLED* rgbled;

struct interval_timer_param {
  uint16_t interval;      // median interval (sec)
  uint16_t fluc_interval; // flluction of inverval (sec) 
  uint16_t inactive_dur;  // duration of inactive time (sec)
  uint16_t fluc_inactive;  // fluctuation of inactive duration (sec) 
} timer_param;

struct blinker_timer_param {
  uint16_t dur_on;  // in millisec
  uint16_t dur_off; // in millisec
} blinker_param;

volatile bool operation_active = true;
volatile bool blinker_on = true;

void blinker_timer(void *arg) {
  blinker_timer_param* timer_param = (blinker_timer_param*) arg;
  uint16_t dur_on = timer_param->dur_on;
  uint16_t dur_off = timer_param->dur_off;

  uint32_t start_time_msec;

  while(1) {
    blinker_on = true;
    start_time_msec = millis();
    while (millis() < start_time_msec + dur_on) {
      vTaskDelay(10);
    }
    blinker_on = false;
    start_time_msec = millis();
    while (millis() < start_time_msec + dur_off) {
      vTaskDelay(10);
    }
  }
}

uint16_t get_random_number(uint16_t median, uint16_t fluctuation) {
  return random(max(0, median - fluctuation), median + fluctuation);
}

void interval_timer(void *arg) {
  interval_timer_param* timer_param = (interval_timer_param*)arg;
  uint16_t median_interval = timer_param->interval;  // all parameters in sec
  uint16_t fluc_interval = timer_param->fluc_interval;
  uint16_t median_inactive_dur = timer_param->inactive_dur;
  uint16_t fluc_inactive_dur = timer_param->fluc_inactive;

  uint32_t start_time_msec;

  while(1) {
    uint16_t temp_interval = get_random_number(median_interval, fluc_interval);
    uint16_t temp_inactive_dur = get_random_number(median_inactive_dur, fluc_inactive_dur);

    start_time_msec = millis();
    operation_active = true;
    while (millis() < start_time_msec + temp_interval * 1000) {
      vTaskDelay(1000);
    }
    // now, interval period expired and move to inactive mode
    start_time_msec = millis();
    operation_active = false;
    while (millis() < start_time_msec + temp_inactive_dur * 1000) {
      vTaskDelay(1000);
    }
  }
}

void setup() {
  m5::board_t board;
  String board_name;
  gpio_num_t gpio_num_rgbled, gpio_num_adc1;

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

  randomSeed(analogRead(gpio_num_adc1));

  timer_param = {10, 3, 5, 2};
  xTaskCreatePinnedToCore(interval_timer, "interval_timer", 4096,\
    (void*)&timer_param, 1, NULL, tskNO_AFFINITY);

  blinker_param = {250, 250};
  xTaskCreatePinnedToCore(blinker_timer, "blinker_timer", 4096,\
    (void*)&blinker_param, 1, NULL, tskNO_AFFINITY);

  pinMode(gpio_num_rgbled, OUTPUT);
  rgbled = new EspEasyLED(gpio_num_rgbled,1,20);
}

void loop() {
  if (operation_active) {
    rgbled->showColor(EspEasyLEDColor::BLUE);
  } else {
    if (blinker_on) {
      rgbled->showColor(EspEasyLEDColor::BLUE);
    } else {
      rgbled->showColor(EspEasyLEDColor::BLACK);
    }
  }  
  delay(10);
}
