#include <Arduino.h>
#include <SPI.h>
#include "functions.h"

#define without_polling_emergency_channels // запрет опроса неисправных каналов. Закомментировать эту строку для разрешения опроса неисправных каналов

// byte byte_from_SPI;

void setup()
{
  real_load_flag_val = EEPROM.read(real_load_flag_addr);
  // real_load_flag_val = 255;
  Serial.begin(9600);
  // Serial.println("START");
  SPI.begin();
  if (CAN_init()) // инициализация CAN шины
  {
    Serial.println("Не удалось инициализировать шину CAN");
    // Далее обработка ошибки инициализации
  }

  for (uint8_t i = 0; i < sizeof(pins_out); i++)
  { // инициализация вЫходов
    pinMode(pins_out[i], OUTPUT);
  }

  for (uint8_t i = 0; i < sizeof(pins_in); i++)
  { // инициализация входов
    pinMode(pins_in[i], INPUT);
  }

  power_circuit_test(); // тест силовой цепи всех каналов
}

void loop()
{
  check_signal_24v();
  CAN_get_cmd();
  // in_cmd_decoder();
  power_circuit_test(); // тест силовой цепи всех каналов
  
}
