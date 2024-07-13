#define real_load_flag_addr 0  // адрес флага, который указывает подключен ли нагрузкой реальный соленоид (1) или лампа накаливания (0)
#define delay_for_lamp_load 1  // адрес хранения задержки опроса состояния силовой цепи канала, если нагрузкой является лампа накаливания
//#define open_circuit_limit 2  // предельное значение показаний АЦП, ниже которого обрыв цепи
//#define limit_value_short_circuit 3   // младший байт предельное значение показаний АЦП, свыше которого короткое замыкание
//#define limit_value_short_circuit 4   // старший байт предельное значение показаний АЦП, свыше которого короткое замыкание

// #define test_ok 0x0010 // первоначальное тестирование закончено
#define ch0_on 0x30  // включение канала 0
#define ch0_off 0x40 // вЫключение канала 0
#define ch1_on 0x31  // включение канала 1
#define ch1_off 0x41 // вЫключение канала 1
#define ch2_on 0x32  // включение канала 2
#define ch2_off 0x42 // вЫключение канала 2
#define ch3_on 0x33  // включение канала 3
#define ch3_off 0x43 // вЫключение канала 3
#define ch4_on 0x34  // включение канала 4
#define ch4_off 0x44 // вЫключение канала 4
#define ch5_on 0x35  // включение канала 5
#define ch5_off 0x45 // вЫключение канала 5
#define ch6_on 0x36  // включение канала 6
#define ch6_off 0x46 // вЫключение канала 6
#define ch7_on 0x37  // включение канала 7
#define ch7_off 0x47 // вЫключение канала 7
#define ch8_on 0x38  // включение канала 8
#define ch8_off 0x48 // вЫключение канала 8
#define ch9_on 0x39  // включение канала 9
#define ch9_off 0x49 // вЫключение канала 9

#define full_stop_CAN_ID 0x01                               // ID команды полной остановки системы
#define disfunction_start_CAN_ID 0x90                       // ID сообщения об аварии в канале при стартовом тестировании
#define alarm_CAN_ID 0x91                                   // ID аварийных сообщений
#define stop_all_discrete_ch_CAN_ID 0x94                    // ID выключения всех каналов управления дискретных распределителей
#define test_OK_CAN_ID 0x0254                               // ID сообщения test_OK
#define send_status_channel_CAN_ID 0x0255                   // ID сообщений отправки регистра статуса канала
#define on_channel_CAN_ID 0x0250                            // ID сообщений команд включения каналов
#define off_channel_CAN_ID 0x0251                           // ID сообщений команд вЫключения каналов
#define get_status_channel_CAN_ID 0x0252                    // ID сообщений запроса регистра статуса канала
#define write_in_EEPROM_CAN_ID 0x0258                       // ID сообщений для записи в EEPROM (формат: CAN_ID, адрес ячейки, значение ячейки)
#define write_in_EEPROM_successfully_CAN_ID 0x0259          // ID сообщений для чтения из EEPROM (формат: CAN_ID, адрес ячейки, значение ячейки)

#define polling_period_unsuccessful_channels 10000 // периодичность опроса каналов с установленными аварийными битами

#define pin_signal_24V 3
#define CAN0_INT 2

uint8_t real_load_flag_val;  // флаг включения режима иммитации нагрузки
uint8_t status_registers[10];
unsigned long timer = 0;
// unsigned long timer_lamp_load;
unsigned long millis_fix = 0;
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[3];

const uint8_t pins_in[10] = {
    54, // вход контроля исправности цепи нагрузки канала 0
    55, // вход контроля исправности цепи нагрузки канала 1
    56, // вход контроля исправности цепи нагрузки канала 2
    57, // вход контроля исправности цепи нагрузки канала 3
    58, // вход контроля исправности цепи нагрузки канала 4
    59, // вход контроля исправности цепи нагрузки канала 5
    60, // вход контроля исправности цепи нагрузки канала 6
    61, // вход контроля исправности цепи нагрузки канала 7
    62, // вход контроля исправности цепи нагрузки канала 8
    63, // вход контроля исправности цепи нагрузки канала 9
};
const uint8_t pins_out[10] = {
    22, // сигнал управления нагрузкой канала 0 (соленоид 1)
    23, // сигнал управления нагрузкой канала 1 (соленоид 2)
    24, // сигнал управления нагрузкой канала 2 (соленоид 3)
    25, // сигнал управления нагрузкой канала 3 (соленоид 4)
    26, // сигнал управления нагрузкой канала 4 (соленоид 5)
    27, // сигнал управления нагрузкой канала 5 (соленоид 6)
    28, // сигнал управления нагрузкой канала 6 (соленоид 7)
    29, // сигнал управления нагрузкой канала 7 (соленоид 8)
    20, // сигнал управления нагрузкой канала 8 (соленоид 9)
    21, // сигнал управления нагрузкой канала 9 (соленоид 10)
};
#define solenoid_1 pins_out[0]
#define solenoid_2 pins_out[1]
#define solenoid_3 pins_out[2]
#define solenoid_4 pins_out[3]
#define solenoid_5 pins_out[4]
#define solenoid_6 pins_out[5]
#define solenoid_7 pins_out[6]
#define solenoid_8 pins_out[7]
#define solenoid_9 pins_out[8]
#define solenoid_10 pins_out[9]