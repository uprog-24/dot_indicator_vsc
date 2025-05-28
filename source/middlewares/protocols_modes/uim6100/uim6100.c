/**
 * @file uim6100.c
 */
#include "uim6100.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "tim.h"

#include <stdbool.h>

#define ARROW_MASK                                                             \
  0x03 ///< Маска для направления движения (0 и 1-й биты байта W3)
#define ARRIVAL_MASK 0b100 ///< Маска для бита прибытия (2-й бит байта W3)
#define ARRIVAL_VALUE 4 ///< Значение для бита прибытия в байте W3 (биты: 0100)
#define CODE_MESSAGE_W_1_MASK                                                  \
  0b00111111 ///< Маска для кода сообщения (для звуков)

#define CODE_FLOOR_W_2_MASK 0x3F ///< Маска для номера этажа

#define GONG_BUZZER_FREQ 1000 ///< Частота первого тона гонга
#define BUZZER_FREQ_CABIN_OVERLOAD                                             \
  3000 ///< Частота для тона бузера при перегрузе кабины VOICE_CABIN_OVERLOAD
#define BUZZER_FREQ_FIRE_DANGER                                                \
  BUZZER_FREQ_CABIN_OVERLOAD ///< Частота для тона бузера при пожарной опасности
                             ///< VOICE_FIRE_DANGER

#define SPECIAL_SYMBOLS_BUFF_SIZE 19 ///< Кол-во спец. символов

/**
 * Индексы байтов в сообщении UIM6100.
 */
typedef enum UIM_PACKET_BYTES {
  BYTE_CODE_OPERATION_0 = 0,
  BYTE_CODE_OPERATION_1,
  BYTE_W_0,
  BYTE_W_1,
  BYTE_W_2,
  BYTE_W_3
} uim6100_packet_bytes_t;

/**
 * Направления движения (UIM6100).
 */
typedef enum {
  UIM_6100_MOVE_UP = 2,
  UIM_6100_MOVE_DOWN = 1,
  UIM_6100_NO_MOVE = 0
} direction_uim_6100_t;

/**
 * Значения байта W1 (code message), без этажей.
 */
typedef enum CODE_MSG {
  VOICE_LIFT_IS_NOT_WORK = 55,
  VOICE_FIRE_DANGER = 56,
  VOICE_CABIN_OVERLOAD = 57,
  VOICE_DOORS_CLOSING = 58,
  VOICE_DOORS_OPENING = 59,
  SOUND_DOORS_CLOSING = 60,
  SOUND_DOORS_OPENING = 61,
  BUTTON_SOUND_SHORT = 62,
  MUTE_SOUND = 63
} code_msg_t;

/**
 * Значения байта W2 (коды этажей).
 */
typedef enum CODE_FLOOR {
  FLOOR_MINUS_1 = 41,
  FLOOR_MINUS_2 = 42,
  FLOOR_MINUS_3 = 43,
  FLOOR_MINUS_4 = 44,
  FLOOR_MINUS_5 = 45,
  FLOOR_MINUS_6 = 46,
  FLOOR_MINUS_7 = 47,
  FLOOR_MINUS_8 = 48,
  FLOOR_MINUS_9 = 49,
  RESERVE = 50,
  SEISMIC_DANGER = 51,
  LIFT_NOT_WORK = 52,
  TRANSFER_FIREFIGHTERS = 53,
  CODE_FLOOR_54 = 54,
  SERVICE = 55,
  EVACUATION = 56,
  FIRE_DANGER = 57,
  FAULT_IBP = 58,
  LOADING = 59
} code_floor_t;

/// Буфер со спец. символами
static const code_location_symbols_t
    special_symbols_code_location[SPECIAL_SYMBOLS_BUFF_SIZE] = {
        {.code_location = RESERVE, .symbols = "b"},
        {.code_location = SEISMIC_DANGER, .symbols = "L"},
        {.code_location = LIFT_NOT_WORK, .symbols = "A"},
        {.code_location = TRANSFER_FIREFIGHTERS, .symbols = "P"},
        {.code_location = CODE_FLOOR_54, .symbols = "H"},
        {.code_location = SERVICE, .symbols = "C"},
        {.code_location = EVACUATION, .symbols = "E"},
        {.code_location = FIRE_DANGER, .symbols = "F"},
        {.code_location = FAULT_IBP, .symbols = "U"},
        {.code_location = LOADING, .symbols = "p"},
        {.code_location = FLOOR_MINUS_1, .symbols = "-1"},
        {.code_location = FLOOR_MINUS_2, .symbols = "-2"},
        {.code_location = FLOOR_MINUS_3, .symbols = "-3"},
        {.code_location = FLOOR_MINUS_4, .symbols = "-4"},
        {.code_location = FLOOR_MINUS_5, .symbols = "-5"},
        {.code_location = FLOOR_MINUS_6, .symbols = "-6"},
        {.code_location = FLOOR_MINUS_7, .symbols = "-7"},
        {.code_location = FLOOR_MINUS_8, .symbols = "-8"},
        {.code_location = FLOOR_MINUS_9, .symbols = "-9"},
};

/// Флаг для контроля состояния кнопки
static bool is_button_pressed = false;

/// Счетчик для подсчета кол-ва данных о нажатии кнопки приказа (order button is
/// pressed).
static uint8_t order_button_cnt = 0;

/// Счетчик для подсчета пропуска нажатия кнопки (order button is disable sound)
static uint8_t button_disable_cnt = 0;

/// Флаг для контроля перегруза кабины
static bool is_cabin_overload = false;

/// Флаг для контроля воспроизведения оповещения при пожарной опасности
static bool is_fire_danger = false;

extern uint8_t _bip_counter;

/** Содержит текущее и предыдущее состояние бита прибытия для управления гонгом
 * (фронт сигнала "Прибытие" (бит W[3].2) из 0 в 1).
 */
static uint8_t gong[2] = {
    0,
};

/**
 * @brief  Воспроизведение гонга в зависимости от битов направления в байте W3.
 * @note   1. Записываем бит прибытия, используя маску ARRIVAL_MASK, и
 *            направление, используя маску ARROW_MASK;
 * 		     2. По фронту сигнала "Прибытие" (бит W[3].2) из 0 в 1
 *            воспроизводим гонг play_gong в зависимости от направления.
 *            Проверяем предыдущее (gong[0]) и текущее состояние гонга
 *            (gong[1]).
 * @param  direction_byte_w_3: Байт W3.
 * @param  volume: Уровень громкости (buzzer.h).
 * @retval None
 */
static void setting_gong(uint8_t direction_byte_w_3, uint8_t volume) {
  direction_uim_6100_t direction = direction_byte_w_3 & ARROW_MASK;
  uint8_t arrival = direction_byte_w_3 & ARRIVAL_MASK;

  // Если сигнал из 0 меняется на 1, тогда детектируем прибытие на этаж
  gong[0] = (arrival == ARRIVAL_VALUE) != 0 ? 1 : 0;

  if (gong[0] && !gong[1]) {

    switch (direction) {
    case UIM_6100_MOVE_UP:
      play_gong(1, GONG_BUZZER_FREQ, volume, BIP_DURATION_GONG);
      break;
    case UIM_6100_MOVE_DOWN:
      play_gong(2, GONG_BUZZER_FREQ, volume, BIP_DURATION_GONG);
      break;
    case UIM_6100_NO_MOVE:
      play_gong(3, GONG_BUZZER_FREQ, volume, BIP_DURATION_GONG);
      break;
    default:
      __NOP();
      play_gong(3, GONG_BUZZER_FREQ, volume, BIP_DURATION_GONG);
      break;
    }
  }
  gong[1] = gong[0];
}

static uint8_t open_sound_edge[2] = {
    0,
};

static uint8_t close_sound_edge[2] = {
    0,
};

static uint8_t open_voice_edge[2] = {
    0,
};

static uint8_t close_voice_edge[2] = {
    0,
};
static void set_door_sound(uint8_t code_msg_byte_w_1) {
  // uint8_t open = code_msg_byte_w_1 & VOICE_DOORS_OPENING;
  //  & SOUND_DOORS_OPENING
  // uint8_t open = code_msg_byte_w_1 & 0x3F;

  // Если сигнал из 0 меняется на 1, тогда детектируем прибытие на этаж
  // sound open
  open_sound_edge[0] =
      // (open & VOICE_DOORS_OPENING == VOICE_DOORS_OPENING) != 0 ? 1 : 0;
      ((code_msg_byte_w_1 & 0x3F) == SOUND_DOORS_OPENING) ? 1 : 0;

  if (open_sound_edge[0] && !open_sound_edge[1]) {

    play_gong(1, GONG_BUZZER_FREQ, matrix_settings.volume, BIP_DURATION_DOORS);
  }
  open_sound_edge[1] = open_sound_edge[0];

  // sound close
  close_sound_edge[0] =
      // (open & VOICE_DOORS_OPENING == VOICE_DOORS_OPENING) != 0 ? 1 : 0;
      ((code_msg_byte_w_1 & 0x3F) == SOUND_DOORS_CLOSING) ? 1 : 0;

  if (close_sound_edge[0] && !close_sound_edge[1]) {

    play_gong(2, GONG_BUZZER_FREQ, matrix_settings.volume, BIP_DURATION_DOORS);
  }
  close_sound_edge[1] = close_sound_edge[0];

  // voice open
  open_voice_edge[0] =
      // (open & VOICE_DOORS_OPENING == VOICE_DOORS_OPENING) != 0 ? 1 : 0;
      ((code_msg_byte_w_1 & 0x3F) == VOICE_DOORS_OPENING) ? 1 : 0;

  if (open_voice_edge[0] && !open_voice_edge[1]) {

    play_gong(1, GONG_BUZZER_FREQ, matrix_settings.volume, BIP_DURATION_DOORS);
  }
  open_voice_edge[1] = open_voice_edge[0];

  // voice close
  close_voice_edge[0] =
      // (open & VOICE_DOORS_OPENING == VOICE_DOORS_OPENING) != 0 ? 1 : 0;
      ((code_msg_byte_w_1 & 0x3F) == VOICE_DOORS_CLOSING) ? 1 : 0;

  if (close_voice_edge[0] && !close_voice_edge[1]) {

    play_gong(2, GONG_BUZZER_FREQ, matrix_settings.volume, BIP_DURATION_DOORS);
  }
  close_voice_edge[1] = close_voice_edge[0];
}

static uint8_t btn_call_sound_edge[2] = {
    0,
};
static void set_btn_call_sound(uint8_t code_msg_byte_w_1) {
  // uint8_t open = code_msg_byte_w_1 & VOICE_DOORS_OPENING;
  //  & SOUND_DOORS_OPENING
  // uint8_t open = code_msg_byte_w_1 & 0x3F;

  // Если сигнал из 0 меняется на 1, тогда детектируем прибытие на этаж
  // sound open
  btn_call_sound_edge[0] =
      // (open & VOICE_DOORS_OPENING == VOICE_DOORS_OPENING) != 0 ? 1 : 0;
      ((code_msg_byte_w_1 & 0x3F) == BUTTON_SOUND_SHORT) ? 1 : 0;

  if (btn_call_sound_edge[0] && !btn_call_sound_edge[1]) {
    // is_call_btn = true;
    play_gong(1, GONG_BUZZER_FREQ, matrix_settings.volume,
              BIP_DURATION_CALL_BTN);
  }
  btn_call_sound_edge[1] = btn_call_sound_edge[0];
}
#if 0
static void set_btn_call_sound(uint8_t code_msg_byte_w_1) {
  /* Нажатие кнопки вызова */
  if ((code_msg_byte_w_1 & CODE_MESSAGE_W_1_MASK) == BUTTON_SOUND_SHORT) {
    // if ((code_msg_byte_w_1 & BUTTON_SOUND_SHORT) == BUTTON_SOUND_SHORT) {

    if (button_disable_cnt == 0) {
      if (matrix_settings.volume != VOLUME_0) {
        is_button_pressed = true;
        play_gong(1, 1000, matrix_settings.volume, BIP_DURATION_CALL_BTN);
      }
    }

    /** Не воспроизводить последующие нажатия, если кнопка уже нажата;
     */
    if (is_button_pressed) {
      button_disable_cnt++;
      if (button_disable_cnt == 3) {
        button_disable_cnt = 0;
        is_button_pressed = false;
      }
    }
  }
}
#endif

/**
 * @brief  Обработка code message (байт W1), включение/выключение бузера.
 * @param  code_msg_byte_w_1: Байт W1.
 * @retval None
 */
static void process_code_msg(uint8_t code_msg_byte_w_1) {

  /* Если гонг отработал и не перегруз/пожар, то воспроизводим нажатие кнопки
   * вызова, иначе не воспроизводим нажатие
   */
  if (_bip_counter == 0 && !is_cabin_overload && !is_fire_danger) {
    set_btn_call_sound(code_msg_byte_w_1);
  }

  /* Перегруз кабины: если не Пожар, воспроизводим */
  if (!is_fire_danger) {

    if ((code_msg_byte_w_1 & CODE_MESSAGE_W_1_MASK) == VOICE_CABIN_OVERLOAD) {

      if (matrix_settings.volume != VOLUME_0) {
        is_cabin_overload = true;
        TIM2_Start_bip(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
      }
      matrix_string[DIRECTION] = 'c';
      matrix_string[MSB] = 'K';
      matrix_string[LSB] = 'g';

    }
    // Следующие полученные данные по CAN
    else if (is_cabin_overload) {
      TIM2_Stop_bip();
      is_cabin_overload = false;
    }
  }

  /* Пожарная опасность */
  if ((code_msg_byte_w_1 & CODE_MESSAGE_W_1_MASK) == VOICE_FIRE_DANGER) {

    if (matrix_settings.volume != VOLUME_0) {
      is_fire_danger = true;
      TIM2_Start_bip(BUZZER_FREQ_FIRE_DANGER, VOLUME_3);
    }
  }
  // Следующие полученные данные по CAN
  else if (is_fire_danger) {
    TIM2_Stop_bip();
    is_fire_danger = false;
  }
}

/**
 * @brief  Обработка звуковых сигналов: нажатие кнопки приказа, гонг,
 * перегруз, пожарная опасность.
 * @param  code_msg_byte_w_1: Байт W1.
 * @retval None
 */
static void setting_sound_uim(msg_t *msg) {

  uint8_t code_msg_byte_w_1 = msg->w1;

  /* Если гонг отработал и не перегруз/пожар, то воспроизводим нажатие кнопки
   * вызова, иначе не воспроизводим нажатие
   */
  if (_bip_counter == 0 && !is_cabin_overload && !is_fire_danger) {
    set_btn_call_sound(code_msg_byte_w_1);
  }

  /* Гонг прибытия */
  if (!is_fire_danger) {
    if (matrix_settings.volume != VOLUME_0) {
      setting_gong(msg->w3, matrix_settings.volume);
    }
  }

  /* Перегруз кабины: если не Пожар, воспроизводим */
  if (!is_fire_danger) {
    if ((code_msg_byte_w_1 & CODE_MESSAGE_W_1_MASK) == VOICE_CABIN_OVERLOAD) {

      if (matrix_settings.volume != VOLUME_0) {
        is_cabin_overload = true;
        TIM2_Start_bip(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
      }

      matrix_string[DIRECTION] = 'c';
      matrix_string[MSB] = 'K';
      matrix_string[LSB] = 'g';
    }
    // Следующие полученные данные по CAN
    else if (is_cabin_overload) {
      TIM2_Stop_bip();
      is_cabin_overload = false;
    }
  }

  /* Пожарная опасность */
  if ((code_msg_byte_w_1 & CODE_MESSAGE_W_1_MASK) == VOICE_FIRE_DANGER) {
    is_fire_danger = true;

    if (matrix_settings.volume != VOLUME_0) {
      TIM2_Start_bip(BUZZER_FREQ_FIRE_DANGER, VOLUME_3);
    }

  }
  // Следующие полученные данные по CAN
  else if (is_fire_danger) {
    TIM2_Stop_bip();
    is_fire_danger = false;
  }
}

/// Структура с данными для отображения (direction, floor).
static drawing_data_t drawing_data = {0, 0};

/**
 * @brief  Преобразование значений направления движения UIM6100 в общий тип
 *         направления, который определен в файле drawing.h.
 * @param  direction: Значение из enum direction_uim_6100_t:
 *                    UIM_6100_MOVE_UP/UIM_6100_MOVE_DOWN/UIM_6100_NO_MOVE.
 * @retval None
 */
static void transform_direction_to_common(direction_uim_6100_t direction) {
  switch (direction) {
  case UIM_6100_MOVE_UP:
    drawing_data.direction = DIRECTION_UP;
    break;
  case UIM_6100_MOVE_DOWN:
    drawing_data.direction = DIRECTION_DOWN;
    break;
  case UIM_6100_NO_MOVE:
    drawing_data.direction = NO_DIRECTION;
    break;

  default:
    drawing_data.direction = NO_DIRECTION;
    break;
  }
}

/**
 * @brief  Обработка данных по протоколу UIM6100 (ШК6000).
 * @note   1. Установка структуры drawing_data, обработка code message,
 *            воспроизведение гонга и отображение символов;
 *         2. Отображение matrix_string пока следующие данные не получены и
 *            интерфейс CAN подключен.
 * @param  msg: Указатель на структуру полученных данных.
 * @retval None
 */

bool is_call_btn = false;
void process_data_uim(msg_t *msg) {
  /// Флаг для контроля полученных данных по CAN.
  extern volatile bool is_data_received;

  uint8_t code_msg = msg->w1;
  drawing_data.floor = msg->w2 & CODE_FLOOR_W_2_MASK;

  transform_direction_to_common(msg->w3 & ARROW_MASK);

  setting_symbols(matrix_string, &drawing_data, MAX_POSITIVE_NUMBER_FLOOR,
                  special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);

  // Кабинный индикатор
  if (matrix_settings.addr_id == MAIN_CABIN_ID) {
#if 0
    /* Проверено на испытаниях */
    if (matrix_settings.volume != VOLUME_0) {
      setting_gong(msg->w3, matrix_settings.volume);
    }
    process_code_msg(code_msg);
#else
    /* Функция для обработки ВСЕХ звукрвых оповещений была составлена после
     * испытаний, не проверена */
    setting_sound_uim(msg);
#endif
  } else {
    // Этажный индикатор

    /* Если гонг отработал, то воспроизводим нажатие кнопки вызова, иначе не
     * воспроизводим нажатие
     */
    if (_bip_counter == 0) {
      // set_btn_call_sound(msg->w1 & CODE_MESSAGE_W_1_MASK);
      if (matrix_settings.volume != VOLUME_0) {
        set_btn_call_sound(msg->w1);
      }
    }

    // if (_bip_counter == 0) {
    //   set_open_door_sound(code_msg);
    // }

    // Гонг
    if (matrix_settings.addr_id == drawing_data.floor ||
        matrix_settings.addr_id == 47) {
      if (matrix_settings.volume != VOLUME_0) {

        if (_bip_counter == 0) {
          // set_open_door_sound(code_msg);
          set_door_sound(msg->w1);
        }

        setting_gong(msg->w3, matrix_settings.volume);
      }
    }
  }

  /*
   * Пока новые 6 байт данных не получены и CAN подключен, отображаем текущую
   * matrix_string
   */
  while (is_data_received == false && is_interface_connected == true) {
    draw_string_on_matrix(matrix_string);
  }
}
