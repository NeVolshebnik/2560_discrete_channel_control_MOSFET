#include <Arduino.h>
#include <SPI.h>
#include <vars.h>
#include <mcp_can.h>
#include <EEPROM.h>

#define without_polling_emergency_channels // запрет опроса неисправных каналов. Закомментировать эту строку для разрешения опроса неисправных каналов

uint8_t CAN_init();
uint8_t CAN_send_msg(uint16_t msg_id, uint8_t data[2] = {});
void CAN_get_cmd();
void change_bit_state(uint8_t channel_number, uint8_t bit_number, boolean value);
void power_circuit_alert_signal_processing(uint8_t channel_number, uint16_t signal_value);
void power_circuit_test();
void turn_off_all();
void in_cmd_decoder();
void check_signal_24v();
uint8_t get_cmd();

MCP_CAN CAN0(SS); // Set CS to pin SS

uint8_t CAN_init() // инициализация CAN шины
{
    uint8_t result;
    result = CAN0.begin(MCP_ANY, CAN_125KBPS, MCP_8MHZ);
    if (result == CAN_OK)
    {
        CAN0.init_Mask(0, 0x07FE);
        CAN0.init_Filt(0, 0x0001);
        CAN0.init_Mask(1, 0x07FC);
        CAN0.init_Filt(2, 0x0250);
        CAN0.setMode(MCP_NORMAL);
        pinMode(CAN0_INT, INPUT);
    }
    return result;
}

uint8_t CAN_send_msg(uint16_t msg_id, uint8_t data[2] = {}) // отправить данные в CAN
{
    uint8_t sendBuf[3] = {data[0], data[1], real_load_flag_val};
    // data[2] = real_load_flag_val;
    byte sndStat = CAN0.sendMsgBuf(msg_id, 0, 3, sendBuf);
    return sndStat;
}

/*
if (EEPROM.read(real_load_flag_addr))
{
    delayMicroseconds(3000);
}
else
{
    //delay(50);
    delay((unsigned long)EEPROM.read(delay_for_lamp_load));

}
*/
void CAN_get_cmd() // получить команду из CAN
{
    if (!digitalRead(CAN0_INT))
    {
        CAN0.readMsgBuf(&rxId, &len, rxBuf);

        switch (rxId)
        {
        case on_channel_CAN_ID:
            change_bit_state(rxBuf[0] - 0x30, 0, true); // включение канала
            if (status_registers[rxBuf[0] - 0x30] == 1) // канал успешно включён
            {
                uint8_t data[2] = {rxBuf[0] - 0x30, status_registers[rxBuf[0] - 0x30]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            else // в канале имеется неисправность
            {
                uint8_t data[2] = {rxBuf[0] - 0x30, status_registers[rxBuf[0] - 0x30]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            /*
                        switch (rxBuf[0])
                        {
                        case ch0_on:
            #ifdef without_polling_emergency_channels
                            change_bit_state(0, 0, true); // включение канала
                            if (status_registers[0] == 1) // канал успешно включён
                            {
                                uint8_t data[2] = {0, status_registers[0]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
                            }
                            // Serial.println("Соленоид 1 вкл.");
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {0, status_registers[0]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #else
                            if (status_registers[0] < 2) // канал 0 исправен
                            {
                                change_bit_state(0, 0, true); // включение канала
                                if (status_registers[0] == 1) // канал успешно включён
                                {
                                    uint8_t data[2] = {0, status_registers[0]};
                                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                                }
                                // Serial.println("Соленоид 1 вкл.");
                            }
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {0, status_registers[0]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #endif
                            break;
                        case ch1_on:
            #ifdef without_polling_emergency_channels
                            change_bit_state(1, 0, true); // включение канала
                            if (status_registers[1] == 1) // канал успешно включён
                            {
                                uint8_t data[2] = {1, status_registers[1]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
                            }
                            // Serial.println("Соленоид 1 вкл.");
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {1, status_registers[1]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #else
                            if (status_registers[1] < 2) // канал 1 исправен
                            {
                                change_bit_state(1, 0, true); // включение канала
                                if (status_registers[1] == 1) // канал успешно включён
                                {
                                    uint8_t data[2] = {1, status_registers[1]};
                                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                                }
                                // Serial.println("Соленоид 2 вкл.");
                            }
                            else // в канале 1 имеется неисправность
                            {
                                uint8_t data[2] = {1, status_registers[1]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #endif
                            break;
                        case ch2_on:
            #ifdef without_polling_emergency_channels
                            change_bit_state(2, 0, true); // включение канала
                            if (status_registers[2] == 1) // канал успешно включён
                            {
                                uint8_t data[2] = {2, status_registers[2]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
                            }
                            // Serial.println("Соленоид 1 вкл.");
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {2, status_registers[2]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #else
                            if (status_registers[2] < 2) // канал 2 исправен
                            {
                                change_bit_state(2, 0, true); // включение канала
                                if (status_registers[2] == 1) // канал успешно включён
                                {
                                    uint8_t data[2] = {2, status_registers[2]};
                                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                                }
                                // Serial.println("Соленоид 3 вкл.");
                            }
                            else // в канале 2 имеется неисправность
                            {
                                uint8_t data[2] = {2, status_registers[2]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #endif
                            break;
                        case ch3_on:
            #ifdef without_polling_emergency_channels
                            change_bit_state(3, 0, true); // включение канала
                            if (status_registers[3] == 1) // канал успешно включён
                            {
                                uint8_t data[2] = {3, status_registers[3]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
                            }
                            // Serial.println("Соленоид 1 вкл.");
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {3, status_registers[3]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #else
                            if (status_registers[3] < 2) // канал 3 исправен
                            {
                                change_bit_state(3, 0, true); // включение канала
                                if (status_registers[3] == 1) // канал успешно включён
                                {
                                    uint8_t data[2] = {3, status_registers[3]};
                                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                                }
                                // Serial.println("Соленоид 4 вкл.");
                            }
                            else // в канале 3 имеется неисправность
                            {
                                uint8_t data[2] = {3, status_registers[3]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #endif
                            break;
                        case ch4_on:
            #ifdef without_polling_emergency_channels
                            change_bit_state(4, 0, true); // включение канала
                            if (status_registers[4] == 1) // канал успешно включён
                            {
                                uint8_t data[2] = {4, status_registers[4]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
                            }
                            // Serial.println("Соленоид 1 вкл.");
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {4, status_registers[4]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #else
                            if (status_registers[4] < 2) // канал 4 исправен
                            {
                                change_bit_state(4, 0, true); // включение канала
                                if (status_registers[4] == 1) // канал успешно включён
                                {
                                    uint8_t data[2] = {4, status_registers[4]};
                                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                                }
                                // Serial.println("Соленоид 5 вкл.");
                            }
                            else // в канале 4 имеется неисправность
                            {
                                uint8_t data[2] = {4, status_registers[4]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #endif
                            break;
                        case ch5_on:
            #ifdef without_polling_emergency_channels
                            change_bit_state(5, 0, true); // включение канала
                            if (status_registers[5] == 1) // канал успешно включён
                            {
                                uint8_t data[2] = {5, status_registers[5]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
                            }
                            // Serial.println("Соленоид 1 вкл.");
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {5, status_registers[5]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #else
                            if (status_registers[5] < 2) // канал 5 исправен
                            {
                                change_bit_state(5, 0, true); // включение канала
                                if (status_registers[5] == 1) // канал успешно включён
                                {
                                    uint8_t data[2] = {5, status_registers[5]};
                                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                                }
                                // Serial.println("Соленоид 6 вкл.");
                            }
                            else // в канале 5 имеется неисправность
                            {
                                uint8_t data[2] = {5, status_registers[5]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #endif
                            break;
                        case ch6_on:
            #ifdef without_polling_emergency_channels
                            change_bit_state(6, 0, true); // включение канала
                            if (status_registers[6] == 1) // канал успешно включён
                            {
                                uint8_t data[2] = {6, status_registers[6]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
                            }
                            // Serial.println("Соленоид 1 вкл.");
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {6, status_registers[6]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #else
                            if (status_registers[6] < 2) // канал 6 исправен
                            {
                                change_bit_state(6, 0, true); // включение канала
                                if (status_registers[6] == 1) // канал успешно включён
                                {
                                    uint8_t data[2] = {6, status_registers[6]};
                                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                                }
                                // Serial.println("Соленоид 7 вкл.");
                            }
                            else // в канале 6 имеется неисправность
                            {
                                uint8_t data[2] = {6, status_registers[6]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #endif
                            break;
                        case ch7_on:
            #ifdef without_polling_emergency_channels
                            change_bit_state(7, 0, true); // включение канала
                            if (status_registers[7] == 1) // канал успешно включён
                            {
                                uint8_t data[2] = {7, status_registers[7]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
                            }
                            // Serial.println("Соленоид 1 вкл.");
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {7, status_registers[7]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #else
                            if (status_registers[7] < 2) // канал 7 исправен
                            {
                                change_bit_state(7, 0, true); // включение канала
                                if (status_registers[7] == 1) // канал успешно включён
                                {
                                    uint8_t data[2] = {7, status_registers[7]};
                                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                                }
                                // Serial.println("Соленоид 8 вкл.");
                            }
                            else // в канале 7 имеется неисправность
                            {
                                uint8_t data[2] = {7, status_registers[7]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #endif
                            break;
                        case ch8_on:
            #ifdef without_polling_emergency_channels
                            change_bit_state(8, 0, true); // включение канала
                            if (status_registers[8] == 1) // канал успешно включён
                            {
                                uint8_t data[2] = {8, status_registers[8]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
                            }
                            // Serial.println("Соленоид 1 вкл.");
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {8, status_registers[8]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #else
                            if (status_registers[8] < 2) // канал 8 исправен
                            {
                                change_bit_state(8, 0, true); // включение канала
                                if (status_registers[8] == 1) // канал успешно включён
                                {
                                    uint8_t data[2] = {8, status_registers[8]};
                                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                                }
                                // Serial.println("Соленоид 9 вкл.");
                            }
                            else // в канале 8 имеется неисправность
                            {
                                uint8_t data[2] = {8, status_registers[8]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #endif
                            break;
                        case ch9_on:
            #ifdef without_polling_emergency_channels
                            change_bit_state(9, 0, true); // включение канала
                            if (status_registers[9] == 1) // канал успешно включён
                            {
                                uint8_t data[2] = {9, status_registers[9]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
                            }
                            // Serial.println("Соленоид 1 вкл.");
                            else // в канале 0 имеется неисправность
                            {
                                uint8_t data[2] = {9, status_registers[9]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #else
                            if (status_registers[9] < 2) // канал 9 исправен
                            {
                                change_bit_state(9, 0, true); // включение канала
                                if (status_registers[9] == 1) // канал успешно включён
                                {
                                    uint8_t data[2] = {9, status_registers[9]};
                                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                                }
                                // Serial.println("Соленоид 10 вкл.");
                            }
                            else // в канале 9 имеется неисправность
                            {
                                uint8_t data[2] = {9, status_registers[9]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
            #endif
                        }
                        */
            break;
        case off_channel_CAN_ID:
            change_bit_state(rxBuf[0] - 0x40, 0, false); // вЫключение канала
            if (status_registers[rxBuf[0] - 0x40] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {rxBuf[0] - 0x40, status_registers[rxBuf[0] - 0x40]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[rxBuf[0] - 0x40] > 1)
            {
                uint8_t data[2] = {rxBuf[0] - 0x40, status_registers[rxBuf[0] - 0x40]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            /*
                        switch (rxBuf[0])
                        {
                        case ch0_off:
                            change_bit_state(0, 0, false); // вЫключение канала
                            if (status_registers[0] == 0)  // канал успешно вЫключен
                            {
                                uint8_t data[2] = {0, status_registers[0]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
                            }
                            else if (status_registers[0] > 1)
                            {
                                uint8_t data[2] = {0, status_registers[0]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
                            // Serial.println("Соленоид 1 вЫкл.");
                            break;
                        case ch1_off:
                            change_bit_state(1, 0, false); // вЫключение канала
                            if (status_registers[1] == 0)  // канал успешно вЫключен
                            {
                                uint8_t data[2] = {1, status_registers[1]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
                            }
                            else if (status_registers[1] > 1)
                            {
                                uint8_t data[2] = {1, status_registers[1]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
                            // Serial.println("Соленоид 2 вЫкл.");
                            break;
                        case ch2_off:
                            change_bit_state(2, 0, false); // вЫключение канала
                            if (status_registers[2] == 0)  // канал успешно вЫключен
                            {
                                uint8_t data[2] = {2, status_registers[2]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
                            }
                            else if (status_registers[2] > 1)
                            {
                                uint8_t data[2] = {2, status_registers[2]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
                            // Serial.println("Соленоид 3 вЫкл.");
                            break;
                        case ch3_off:
                            change_bit_state(3, 0, false); // вЫключение канала
                            if (status_registers[3] == 0)  // канал успешно вЫключен
                            {
                                uint8_t data[2] = {3, status_registers[3]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
                            }
                            else if (status_registers[3] > 1)
                            {
                                uint8_t data[2] = {3, status_registers[3]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
                            // Serial.println("Соленоид 4 вЫкл.");
                            break;
                        case ch4_off:
                            change_bit_state(4, 0, false); // вЫключение канала
                            if (status_registers[4] == 0)  // канал успешно вЫключен
                            {
                                uint8_t data[2] = {4, status_registers[4]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
                            }
                            else if (status_registers[4] > 1)
                            {
                                uint8_t data[2] = {4, status_registers[4]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
                            // Serial.println("Соленоид 5 вЫкл.");
                            break;
                        case ch5_off:
                            change_bit_state(5, 0, false); // вЫключение канала
                            if (status_registers[5] == 0)  // канал успешно вЫключен
                            {
                                uint8_t data[2] = {5, status_registers[5]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
                            }
                            else if (status_registers[5] > 1)
                            {
                                uint8_t data[2] = {5, status_registers[5]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
                            // Serial.println("Соленоид 6 вЫкл.");
                            break;
                        case ch6_off:
                            change_bit_state(6, 0, false); // вЫключение канала
                            if (status_registers[6] == 0)  // канал успешно вЫключен
                            {
                                uint8_t data[2] = {6, status_registers[6]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
                            }
                            else if (status_registers[6] > 1)
                            {
                                uint8_t data[2] = {6, status_registers[6]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
                            // Serial.println("Соленоид 7 вЫкл.");
                            break;
                        case ch7_off:
                            change_bit_state(7, 0, false); // вЫключение канала
                            if (status_registers[7] == 0)  // канал успешно вЫключен
                            {
                                uint8_t data[2] = {7, status_registers[7]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
                            }
                            else if (status_registers[7] > 1)
                            {
                                uint8_t data[2] = {7, status_registers[7]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
                            // Serial.println("Соленоид 8 вЫкл.");
                            break;
                        case ch8_off:
                            change_bit_state(8, 0, false); // вЫключение канала
                            if (status_registers[8] == 0)  // канал успешно вЫключен
                            {
                                uint8_t data[2] = {8, status_registers[8]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
                            }
                            else if (status_registers[8] > 1)
                            {
                                uint8_t data[2] = {8, status_registers[8]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
                            // Serial.println("Соленоид 9 вЫкл.");
                            break;
                        case ch9_off:
                            change_bit_state(9, 0, false); // вЫключение канала
                            if (status_registers[9] == 0)  // канал успешно вЫключен
                            {
                                uint8_t data[2] = {9, status_registers[9]};
                                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
                            }
                            else if (status_registers[9] > 1)
                            {
                                uint8_t data[2] = {9, status_registers[9]};
                                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
                            }
                            // Serial.println("Соленоид 10 вЫкл.");
                            break;
                        }
                        */
            break;
        case (full_stop_CAN_ID || stop_all_discrete_ch_CAN_ID):
            turn_off_all();
            break;
        case get_status_channel_CAN_ID:
            if (rxBuf[0] < 10)
            {
                uint8_t data[2];
                data[0] = rxBuf[0];
                data[1] = status_registers[rxBuf[0]];
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            break;
        case write_in_EEPROM_CAN_ID:
            EEPROM.write(rxBuf[0], rxBuf[1]); // пишем в EEPROM полученные данные по полученному адресу
            real_load_flag_val = EEPROM.read(real_load_flag_addr);  // обновляем переменную с флагом
            uint8_t data[2];
            data[0] = rxBuf[0];
            data[1] = EEPROM.read(rxBuf[0]);
            CAN_send_msg(write_in_EEPROM_successfully_CAN_ID, data);
            break;
        }
        rxId = 0;
    }
}

void change_bit_state(uint8_t channel, uint8_t bit_number, boolean value) // изменение бита в регистре и состояния выходного пина
{

    bitWrite(status_registers[channel], bit_number, value == true ? 1 : 0);
    if (bit_number == 0) // если функция работает с битом вкл/вЫкл. сигнала на пине, то ...
    {
        digitalWrite(pins_out[channel], bitRead(status_registers[channel], bit_number));
        if (bitRead(status_registers[channel], bit_number)) // если канал только что был переведён во включённое состояние
        {
            delayMicroseconds(3000);
            int16_t signal_in_level = -1; // цифровое значение уровня сигнала оповещения о состоянии силовой цепи канала
            signal_in_level = analogRead(pins_in[channel]);
            // Serial.println(signal_in_level);
            if (signal_in_level >= 0) // измерение уровня сигнала оповещения произведено
            {
                power_circuit_alert_signal_processing(channel, signal_in_level);
            }
        }
    }
}

void power_circuit_alert_signal_processing(uint8_t channel_number, uint16_t signal_value) // обработка сигнала оповещения от силовой цепи канала
{
    if (!real_load_flag_val)  // если включён режим имитации нагрузки
            {
                // signal_value = signal_value > 350 ? 250 : signal_value;  // игнорируем КЗ в канале
                signal_value = 250;  // игнорируем любые аварии в канале
            }
    if (signal_value < 20) // обрыв в цепи нагрузки
    {
        change_bit_state(channel_number, 0, false); // вЫключение нагрузки
        change_bit_state(channel_number, 1, true);  // включение бита обрыва нагрузки
        change_bit_state(channel_number, 2, false); // вЫключение бита КЗ нагрузки
    }
    else if (signal_value > 350) // КЗ в цепи нагрузки
    {
        change_bit_state(channel_number, 0, false); // вЫключение нагрузки
        change_bit_state(channel_number, 1, false); // вЫключение бита обрыва нагрузки
        change_bit_state(channel_number, 2, true);  // включение бита КЗ нагрузки
    }
    else // рабочий режим нагрузки
    {
        change_bit_state(channel_number, 1, false); // вЫключение бита КЗ нагрузки
        change_bit_state(channel_number, 2, false); // вЫключение бита обрыва нагрузки
    }
}

void test_channel(uint8_t channel)
{
    int16_t signal_in_level;                   // значение уровня сигнала оповещения о состоянии силовой цепи канала
    if (bitRead(status_registers[channel], 0)) // нагрузка включена
    {
        signal_in_level = analogRead(pins_in[channel]);
        power_circuit_alert_signal_processing(channel, signal_in_level);
        if (status_registers[channel] > 1) // если в канале обнаружена авария
        {
            uint8_t send_buf[2] = {channel, status_registers[channel]};
            CAN_send_msg(alarm_CAN_ID, send_buf); // отправка аварийного сигнала в CAN шину
        }
    }
    else // нагрузка вЫключена
    {
        // if (timer <= millis_fix) // пришло время опроса неисправных и выключенных каналов
        if (!timer) // если это проверка канала при включении
        {
            change_bit_state(channel, 0, true); // включаем нагрузку
            // signal_in_level = analogRead(pins_in[channel]);
            change_bit_state(channel, 0, false); // вЫключаем нагрузку
            // power_circuit_alert_signal_processing(channel, signal_in_level);
        }
    }
}

void power_circuit_test() // тест цепей нагрузки всех каналов
{
    uint8_t sum = 1;
    for (uint8_t channel = 0; channel < sizeof(pins_in); channel++)
    {
        uint8_t send_buf[2] = {};
        test_channel(channel);

        // if (timer <= millis_fix) // пришло время отправки регистров состояния головному устройству
        if (!timer) // если идёт тестирование каналов при первом включении
        {
            // send_buf[0] = channel;
            // send_buf[1] = status_registers[channel];
            // if (!timer) // работает только при первом проходе, т.е. единожды при включении
            // {
            if (status_registers[channel])
            {
                send_buf[0] = channel;
                send_buf[1] = status_registers[channel];
                CAN_send_msg(disfunction_start_CAN_ID, send_buf); // отправка в CAN регистра состояния канала (отправляются только аварийные)
                sum = 0;
            }
            /*
            }
            else
            {
                CAN_send_msg(send_status_channel_CAN_ID, send_buf); // отправка в CAN регистра состояния канала
            }
            */
        }
    }
    if (!timer) // работает только при первом проходе, т.е. единожды при включении
    {
        /*
        uint16_t sum = 0;
        for (uint8_t channel = 0; channel < sizeof(pins_out); channel++)
        {
            sum += status_registers[channel];
        }
        */
        if (sum) // если все каналы исправны
        {
            uint8_t send_buf[2] = {0x00, 0x00};
            CAN_send_msg(test_OK_CAN_ID, send_buf);
        }
        // timer = millis() + polling_period_unsuccessful_channels;
        timer = 1;
    }
}

void turn_off_all() // выключить нагрузки всех каналов
{
    for (uint8_t i = 0; i < sizeof(pins_out); i++)
    {
        change_bit_state(i, 0, false);
    }
}

void in_cmd_decoder() // декодер команд
{
    /*
    if (rxId == on_channel_CAN_ID)
    {
        switch (rxBuf[0])
        {
        case ch0_on:
#ifdef without_polling_emergency_channels
            change_bit_state(0, 0, true); // включение канала
            if (status_registers[0] == 1) // канал успешно включён
            {
                uint8_t data[2] = {0, status_registers[0]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            // Serial.println("Соленоид 1 вкл.");
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {0, status_registers[0]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#else
            if (status_registers[0] < 2) // канал 0 исправен
            {
                change_bit_state(0, 0, true); // включение канала
                if (status_registers[0] == 1) // канал успешно включён
                {
                    uint8_t data[2] = {0, status_registers[0]};
                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                }
                // Serial.println("Соленоид 1 вкл.");
            }
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {0, status_registers[0]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#endif
            break;
        case ch1_on:
#ifdef without_polling_emergency_channels
            change_bit_state(1, 0, true); // включение канала
            if (status_registers[1] == 1) // канал успешно включён
            {
                uint8_t data[2] = {1, status_registers[1]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            // Serial.println("Соленоид 1 вкл.");
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {1, status_registers[1]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#else
            if (status_registers[1] < 2) // канал 1 исправен
            {
                change_bit_state(1, 0, true); // включение канала
                if (status_registers[1] == 1) // канал успешно включён
                {
                    uint8_t data[2] = {1, status_registers[1]};
                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                }
                // Serial.println("Соленоид 2 вкл.");
            }
            else // в канале 1 имеется неисправность
            {
                uint8_t data[2] = {1, status_registers[1]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#endif
            break;
        case ch2_on:
#ifdef without_polling_emergency_channels
            change_bit_state(2, 0, true); // включение канала
            if (status_registers[2] == 1) // канал успешно включён
            {
                uint8_t data[2] = {2, status_registers[2]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            // Serial.println("Соленоид 1 вкл.");
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {2, status_registers[2]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#else
            if (status_registers[2] < 2) // канал 2 исправен
            {
                change_bit_state(2, 0, true); // включение канала
                if (status_registers[2] == 1) // канал успешно включён
                {
                    uint8_t data[2] = {2, status_registers[2]};
                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                }
                // Serial.println("Соленоид 3 вкл.");
            }
            else // в канале 2 имеется неисправность
            {
                uint8_t data[2] = {2, status_registers[2]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#endif
            break;
        case ch3_on:
#ifdef without_polling_emergency_channels
            change_bit_state(3, 0, true); // включение канала
            if (status_registers[3] == 1) // канал успешно включён
            {
                uint8_t data[2] = {3, status_registers[3]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            // Serial.println("Соленоид 1 вкл.");
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {3, status_registers[3]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#else
            if (status_registers[3] < 2) // канал 3 исправен
            {
                change_bit_state(3, 0, true); // включение канала
                if (status_registers[3] == 1) // канал успешно включён
                {
                    uint8_t data[2] = {3, status_registers[3]};
                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                }
                // Serial.println("Соленоид 4 вкл.");
            }
            else // в канале 3 имеется неисправность
            {
                uint8_t data[2] = {3, status_registers[3]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#endif
            break;
        case ch4_on:
#ifdef without_polling_emergency_channels
            change_bit_state(4, 0, true); // включение канала
            if (status_registers[4] == 1) // канал успешно включён
            {
                uint8_t data[2] = {4, status_registers[4]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            // Serial.println("Соленоид 1 вкл.");
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {4, status_registers[4]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#else
            if (status_registers[4] < 2) // канал 4 исправен
            {
                change_bit_state(4, 0, true); // включение канала
                if (status_registers[4] == 1) // канал успешно включён
                {
                    uint8_t data[2] = {4, status_registers[4]};
                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                }
                // Serial.println("Соленоид 5 вкл.");
            }
            else // в канале 4 имеется неисправность
            {
                uint8_t data[2] = {4, status_registers[4]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#endif
            break;
        case ch5_on:
#ifdef without_polling_emergency_channels
            change_bit_state(5, 0, true); // включение канала
            if (status_registers[5] == 1) // канал успешно включён
            {
                uint8_t data[2] = {5, status_registers[5]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            // Serial.println("Соленоид 1 вкл.");
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {5, status_registers[5]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#else
            if (status_registers[5] < 2) // канал 5 исправен
            {
                change_bit_state(5, 0, true); // включение канала
                if (status_registers[5] == 1) // канал успешно включён
                {
                    uint8_t data[2] = {5, status_registers[5]};
                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                }
                // Serial.println("Соленоид 6 вкл.");
            }
            else // в канале 5 имеется неисправность
            {
                uint8_t data[2] = {5, status_registers[5]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#endif
            break;
        case ch6_on:
#ifdef without_polling_emergency_channels
            change_bit_state(6, 0, true); // включение канала
            if (status_registers[6] == 1) // канал успешно включён
            {
                uint8_t data[2] = {6, status_registers[6]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            // Serial.println("Соленоид 1 вкл.");
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {6, status_registers[6]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#else
            if (status_registers[6] < 2) // канал 6 исправен
            {
                change_bit_state(6, 0, true); // включение канала
                if (status_registers[6] == 1) // канал успешно включён
                {
                    uint8_t data[2] = {6, status_registers[6]};
                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                }
                // Serial.println("Соленоид 7 вкл.");
            }
            else // в канале 6 имеется неисправность
            {
                uint8_t data[2] = {6, status_registers[6]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#endif
            break;
        case ch7_on:
#ifdef without_polling_emergency_channels
            change_bit_state(7, 0, true); // включение канала
            if (status_registers[7] == 1) // канал успешно включён
            {
                uint8_t data[2] = {7, status_registers[7]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            // Serial.println("Соленоид 1 вкл.");
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {7, status_registers[7]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#else
            if (status_registers[7] < 2) // канал 7 исправен
            {
                change_bit_state(7, 0, true); // включение канала
                if (status_registers[7] == 1) // канал успешно включён
                {
                    uint8_t data[2] = {7, status_registers[7]};
                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                }
                // Serial.println("Соленоид 8 вкл.");
            }
            else // в канале 7 имеется неисправность
            {
                uint8_t data[2] = {7, status_registers[7]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#endif
            break;
        case ch8_on:
#ifdef without_polling_emergency_channels
            change_bit_state(8, 0, true); // включение канала
            if (status_registers[8] == 1) // канал успешно включён
            {
                uint8_t data[2] = {8, status_registers[8]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            // Serial.println("Соленоид 1 вкл.");
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {8, status_registers[8]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#else
            if (status_registers[8] < 2) // канал 8 исправен
            {
                change_bit_state(8, 0, true); // включение канала
                if (status_registers[8] == 1) // канал успешно включён
                {
                    uint8_t data[2] = {8, status_registers[8]};
                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                }
                // Serial.println("Соленоид 9 вкл.");
            }
            else // в канале 8 имеется неисправность
            {
                uint8_t data[2] = {8, status_registers[8]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#endif
            break;
        case ch9_on:
#ifdef without_polling_emergency_channels
            change_bit_state(9, 0, true); // включение канала
            if (status_registers[9] == 1) // канал успешно включён
            {
                uint8_t data[2] = {9, status_registers[9]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
            }
            // Serial.println("Соленоид 1 вкл.");
            else // в канале 0 имеется неисправность
            {
                uint8_t data[2] = {9, status_registers[9]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#else
            if (status_registers[9] < 2) // канал 9 исправен
            {
                change_bit_state(9, 0, true); // включение канала
                if (status_registers[9] == 1) // канал успешно включён
                {
                    uint8_t data[2] = {9, status_registers[9]};
                    CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения включения канала
                }
                // Serial.println("Соленоид 10 вкл.");
            }
            else // в канале 9 имеется неисправность
            {
                uint8_t data[2] = {9, status_registers[9]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
#endif
            break;
        default:
            Serial.println("Команда не распознана");
        }
    }
    else if (rxId == off_channel_CAN_ID)
    {
        switch (rxBuf[0])
        {
        case ch0_off:
            change_bit_state(0, 0, false); // вЫключение канала
            if (status_registers[0] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {0, status_registers[0]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[0] > 1)
            {
                uint8_t data[2] = {0, status_registers[0]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            // Serial.println("Соленоид 1 вЫкл.");
            break;
        case ch1_off:
            change_bit_state(1, 0, false); // вЫключение канала
            if (status_registers[1] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {1, status_registers[1]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[1] > 1)
            {
                uint8_t data[2] = {1, status_registers[1]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            // Serial.println("Соленоид 2 вЫкл.");
            break;
        case ch2_off:
            change_bit_state(2, 0, false); // вЫключение канала
            if (status_registers[2] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {2, status_registers[2]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[2] > 1)
            {
                uint8_t data[2] = {2, status_registers[2]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            // Serial.println("Соленоид 3 вЫкл.");
            break;
        case ch3_off:
            change_bit_state(3, 0, false); // вЫключение канала
            if (status_registers[3] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {3, status_registers[3]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[3] > 1)
            {
                uint8_t data[2] = {3, status_registers[3]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            // Serial.println("Соленоид 4 вЫкл.");
            break;
        case ch4_off:
            change_bit_state(4, 0, false); // вЫключение канала
            if (status_registers[4] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {4, status_registers[4]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[4] > 1)
            {
                uint8_t data[2] = {4, status_registers[4]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            // Serial.println("Соленоид 5 вЫкл.");
            break;
        case ch5_off:
            change_bit_state(5, 0, false); // вЫключение канала
            if (status_registers[5] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {5, status_registers[5]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[5] > 1)
            {
                uint8_t data[2] = {5, status_registers[5]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            // Serial.println("Соленоид 6 вЫкл.");
            break;
        case ch6_off:
            change_bit_state(6, 0, false); // вЫключение канала
            if (status_registers[6] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {6, status_registers[6]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[6] > 1)
            {
                uint8_t data[2] = {6, status_registers[6]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            // Serial.println("Соленоид 7 вЫкл.");
            break;
        case ch7_off:
            change_bit_state(7, 0, false); // вЫключение канала
            if (status_registers[7] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {7, status_registers[7]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[7] > 1)
            {
                uint8_t data[2] = {7, status_registers[7]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            // Serial.println("Соленоид 8 вЫкл.");
            break;
        case ch8_off:
            change_bit_state(8, 0, false); // вЫключение канала
            if (status_registers[8] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {8, status_registers[8]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[8] > 1)
            {
                uint8_t data[2] = {8, status_registers[8]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            // Serial.println("Соленоид 9 вЫкл.");
            break;
        case ch9_off:
            change_bit_state(9, 0, false); // вЫключение канала
            if (status_registers[9] == 0)  // канал успешно вЫключен
            {
                uint8_t data[2] = {9, status_registers[9]};
                CAN_send_msg(send_status_channel_CAN_ID, data); // отправка в CAN подтверждения вЫключения канала
            }
            else if (status_registers[9] > 1)
            {
                uint8_t data[2] = {9, status_registers[9]};
                CAN_send_msg(alarm_CAN_ID, data); // отправка аварийного сигнала в CAN шину
            }
            // Serial.println("Соленоид 10 вЫкл.");
            break;
        }
    }
    else if (rxId == full_stop_CAN_ID || rxId == stop_all_discrete_ch_CAN_ID)
    {
        turn_off_all();
    }
    else if (rxId == get_status_channel_CAN_ID)
    {
        if (rxBuf[0] < 10)
        {
            uint8_t data[2];
            data[0] = rxBuf[0];
            data[1] = status_registers[rxBuf[0]];
            CAN_send_msg(send_status_channel_CAN_ID, data); // отправка статуса канала в CAN
        }
    }
    else if (rxId == write_in_EEPROM_CAN_ID)
    {
        EEPROM.write(rxBuf[0], rxBuf[1]); // пишем в EEPROM полученные данные по полученному адресу
        uint8_t data[2];
        data[0] = rxBuf[0];
        data[1] = EEPROM.read(rxBuf[0]);
        CAN_send_msg(write_in_EEPROM_successfully_CAN_ID, data);
    }
    rxId = 0;
    */
}

void check_signal_24v() // проверка напряжения на сигнальном проводе 24V
{
    if (!digitalRead(pin_signal_24V))
    {
        turn_off_all();
    }
}

uint8_t get_cmd() // получение команды из serial порта
{
    if (Serial.available() > 0)
    {
        String string_from_serial = Serial.readString();
        if (string_from_serial.length() == 2)
        {
            // Serial.println(string_from_serial);
            return string_from_serial.toInt();
        }
        return 0;
    }
    return 0;
}
