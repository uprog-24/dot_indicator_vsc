/**
 * @file button.c
 */
#include "button.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "tim.h"

#include <stdbool.h>

#define SETTINGS_MODE_VOLUME                                                   \
  "V0L" ///< String that displayed in settings_mode_t = VOL
#define SETTINGS_MODE_ID "ID" ///< String that displayed in settings_mode_t = ID
#define SETTINGS_MODE_SFT                                                      \
  "SHF" ///< String that displayed in mode ID when shift for floor location
        ///< (Alpaca)
#define SETTINGS_MODE_ESC                                                      \
  "ESC" ///< String that displayed in settings_mode_t = ESC

#define LEVEL_VOLUME_0 "L0" ///< String that displayed when level_volume = 0
#define LEVEL_VOLUME_1 "L1" ///< String that displayed when level_volume = 1
#define LEVEL_VOLUME_2 "L2" ///< String that displayed when level_volume = 2
#define LEVEL_VOLUME_3 "L3" ///< String that displayed when level_volume = 3

#define SHIFT_P_FLOOR "pFL"
#define SHIFT_MINUS_FLOOR "-FL"

/**
 * Stores modes of settings
 */
typedef enum { ID = 0, LEVEL_VOLUME, ESC } settings_mode_t;

/// Flag to control first btn1 click
volatile bool is_first_btn_clicked = true;

/// Counter for pressing of BUTTON_1
volatile uint8_t btn_1_set_mode_counter = 0;

/// Flag to control BUTTON_1 state
volatile bool is_button_1_pressed = false;

/// Flag to control BUTTON_2 state
volatile bool is_button_2_pressed = false;

/// Counter for pressing of BUTTON_2
volatile uint8_t btn_2_set_value_counter = 0;

/// Flag to control bip for level_volume_0
static bool is_level_volume_0_displayed = false;

/// Flag to control bip for level_volume_1
static bool is_level_volume_1_displayed = false;

/// Flag to control bip for level_volume_2
static bool is_level_volume_2_displayed = false;

/// Flag to control bip for level_volume_3
static bool is_level_volume_3_displayed = false;

static void reset_volume_flags() {
  is_level_volume_0_displayed = false;
  is_level_volume_1_displayed = false;
  is_level_volume_2_displayed = false;
  is_level_volume_3_displayed = false;
}

/**
 * @brief  Handle Interrupt by EXTI line, setting the state when button is
 *         pressed.
 *         1. BUTTON_1_Pin and BUTTON_2_Pin are used for MENU;
 *         2. If (GPIO_Pin == DATA_Pin) then read the pin state until the bit
 *            changes from 1 to 0, set is_start_bit_received;
 *         3. Start TIM3 with timing for first data bit and set disable IRQ.
 * @param  GPIO_Pin: Button's pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  /* Current matrix state: MATRIX_STATE_INIT, MATRIX_STATE_START,
   MATRIX_STATE_WORKING, MATRIX_STATE_MENU */
  extern matrix_state_t matrix_state;

  /// Counter for elapsed time in seconds between pressing of buttons
  extern uint32_t time_since_last_press_sec;

  if (GPIO_Pin == BUTTON_1_Pin) {
    is_button_1_pressed = true;
    matrix_state = MATRIX_STATE_MENU;
    btn_1_set_mode_counter++;
    is_interface_connected = false;
  }

  if (GPIO_Pin == BUTTON_2_Pin) {
    is_button_2_pressed = true;

    if (btn_1_set_mode_counter == 0) {
      btn_2_set_value_counter = 0;
    } else {
      btn_2_set_value_counter++;
    }
  }

  if (GPIO_Pin == BUTTON_1_Pin || GPIO_Pin == BUTTON_2_Pin) {
    time_since_last_press_sec = 0;
  }

#if PROTOCOL_UKL
  /// Flag to control is start bit is received (state DATA_Pin from 1 to 0)
  extern bool is_start_bit_received;

  /// Buffer with timings for reading data bits
  extern const uint16_t ukl_timings[];

  if (GPIO_Pin == DATA_Pin) {
    if (matrix_state == MATRIX_STATE_WORKING) {
      if (!is_start_bit_received) {
        if (HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin) == GPIO_PIN_RESET) {
          is_start_bit_received = true;
          TIM3_Start(PRESCALER_FOR_US, ukl_timings[0]);
          HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
        }
      }
    }
  }

#endif
}

/**
 * @brief  Start TIM4 for counting 20 seconds between click buttons when
 *         matrix_state = MATRIX_STATE_MENU
 * @param  None
 * @retval None
 */
void start_timer_menu() { TIM4_Start(); }

/**
 * @brief  Stop TIM4 when matrix_state = MATRIX_STATE_MENU
 * @param  None
 * @retval None
 */
void stop_timer_menu() { TIM4_Stop(); }

/// Store settings mode
static settings_mode_t btn_1_settings_mode = ID;

/// Current (temporary) level of volume
static uint8_t level_volume = 1;

/// Current (temporary) id for interface CAN/UART
static uint8_t id = 1;

/// Selected level of volume
static uint8_t selected_level_volume;

/// Selected id for interface CAN/UART
static uint8_t selected_id;

/// Structure for data that will be displayed on matrix
static drawing_data_t drawing_data = {0, 0};

uint8_t shift_mode = 1;

/**
 * @brief  Handle pressing BUTTON_1 and BUTTON_2.
 * @note   When BUTTON_1 is pressed 1st time - matrix_state = MATRIX_STATE_MENU,
 *         BUTTON_1 allows to select settings_mode_t: ID, VOLUME, ESCAPE
 *         BUTTON_2 allows to select value for ID, VOLUME
 * @param  None
 * @retval None
 */
void press_button() {
  /// Current menu state: MENU_STATE_OPEN, MENU_STATE_WORKING, MENU_STATE_CLOSE
  extern menu_state_t menu_state;

#if !DEMO_MODE && !TEST_MODE

  if (is_first_btn_clicked) {
    reset_volume_flags();

    is_first_btn_clicked = false;

    btn_2_set_value_counter = 0;

    bool is_id_from_flash_valid = matrix_settings.addr_id >= ADDR_ID_MIN &&
                                  matrix_settings.addr_id <= ADDR_ID_LIMIT;

    id = is_id_from_flash_valid ? matrix_settings.addr_id : MAIN_CABIN_ID;

    if (matrix_settings.volume != VOLUME_0 &&
        matrix_settings.volume != VOLUME_1 &&
        matrix_settings.volume != VOLUME_2 &&
        matrix_settings.volume != VOLUME_3) {
      level_volume = 1;
    } else {
      switch (matrix_settings.volume) {
      case VOLUME_0:
        level_volume = 0;
        break;
      case VOLUME_1:
        level_volume = 1;
        break;
      case VOLUME_2:
        level_volume = 2;
        break;
      case VOLUME_3:
        level_volume = 3;
        break;
      }
    }

    selected_level_volume = level_volume;
    selected_id = id;
  }

  if (is_button_1_pressed) {
    is_button_1_pressed = false;

    switch (btn_1_set_mode_counter) {
    case 1:

      while (btn_1_set_mode_counter == 1 && btn_2_set_value_counter == 0) {
        draw_string_on_matrix(SETTINGS_MODE_VOLUME);
        btn_1_settings_mode = LEVEL_VOLUME;
      }

      break;
    case 2:

      while (btn_1_set_mode_counter == 2 && btn_2_set_value_counter == 0) {
#if PROTOCOL_UIM_6100 || PROTOCOL_UEL || PROTOCOL_UKL
        draw_string_on_matrix(SETTINGS_MODE_ID);
#elif PROTOCOL_ALPACA
        draw_string_on_matrix(SETTINGS_MODE_SFT);
#endif

        btn_1_settings_mode = ID;
      }

      // state after selection of value for LEVEL_VOLUME
      while (btn_1_settings_mode == LEVEL_VOLUME &&
             btn_1_set_mode_counter == 2 && btn_2_set_value_counter == 1) {
        draw_string_on_matrix(SETTINGS_MODE_VOLUME);
      }

      // return to choose value LEVEL_VOLUME
      if (btn_2_set_value_counter == 2) {
        btn_1_set_mode_counter = 1;
        btn_2_set_value_counter = 1;
        level_volume = selected_level_volume;

        reset_volume_flags();
      }

      break;

    case 3:

      // return to choose mode LEVEL_VOLUME
      if (btn_1_settings_mode == LEVEL_VOLUME) {
        btn_2_set_value_counter = 0;
        level_volume = selected_level_volume;

        reset_volume_flags();
      }

      while (btn_1_set_mode_counter == 3 && btn_2_set_value_counter == 0) {
        draw_string_on_matrix(SETTINGS_MODE_ESC);
        btn_1_settings_mode = ESC;
      }

      if (btn_2_set_value_counter == 0) {
        btn_1_set_mode_counter = 1;
      } else {
        // state after selection of value for ID
        while (btn_1_settings_mode == ID && btn_1_set_mode_counter == 3 &&
               btn_2_set_value_counter == 1) {
          draw_string_on_matrix(SETTINGS_MODE_ID);
          // draw_string_on_matrix("-1");
        }

        // return to choose mode ID
        if (btn_1_set_mode_counter == 4) {
          btn_1_set_mode_counter = 1;
          btn_2_set_value_counter = 0;
          id = selected_id;
        }

        // return to choose value ID
        if (btn_2_set_value_counter == 2) {
          btn_1_set_mode_counter = 2;
          btn_2_set_value_counter = 1;
          id = selected_id;
        }
      }

      break;
    }
  }

  if (is_button_2_pressed) {
    is_button_2_pressed = false;

    switch (btn_2_set_value_counter) {
    case 1:

      if (btn_1_set_mode_counter == 0) {
        btn_2_set_value_counter = 0;
      } else {
        switch (btn_1_settings_mode) {
        case LEVEL_VOLUME:
          selected_level_volume = level_volume;
          while (btn_1_settings_mode == LEVEL_VOLUME &&
                 btn_2_set_value_counter == 1 && btn_1_set_mode_counter == 1) {
            if (level_volume == 0) {
              draw_string_on_matrix(LEVEL_VOLUME_0);
              play_bip_for_menu(&is_level_volume_0_displayed, VOLUME_0);

              is_level_volume_3_displayed = false;
            }

            if (level_volume == 1) {
              draw_string_on_matrix(LEVEL_VOLUME_1);
              play_bip_for_menu(&is_level_volume_1_displayed, VOLUME_1);

              is_level_volume_0_displayed = false;
            }
            if (level_volume == 2) {
              draw_string_on_matrix(LEVEL_VOLUME_2);
              play_bip_for_menu(&is_level_volume_2_displayed, VOLUME_2);

              is_level_volume_1_displayed = false;
            }
            if (level_volume == 3) {
              draw_string_on_matrix(LEVEL_VOLUME_3);
              play_bip_for_menu(&is_level_volume_3_displayed, VOLUME_3);

              is_level_volume_2_displayed = false;
            }
          }

          level_volume++;
          if (level_volume > VOLUME_LEVEL_LIMIT) {
            level_volume = 0;
          }
          break;

        case ID:
#if PROTOCOL_UIM_6100 || PROTOCOL_UEL || PROTOCOL_UKL

          drawing_data.floor = id;
          setting_symbols(matrix_string, &drawing_data, ADDR_ID_LIMIT, NULL, 0);

          selected_id = id;
          while (btn_1_settings_mode == ID && btn_2_set_value_counter == 1 &&
                 btn_1_set_mode_counter == 2) {
            draw_string_on_matrix(matrix_string);
          }

          id++;
          if (id > ADDR_ID_LIMIT) {
            id = ADDR_ID_MIN;
          }
#elif PROTOCOL_ALPACA

          if (id == 0) {
            matrix_string[DIRECTION] = 'c';
            matrix_string[MSB] = '0';
            matrix_string[LSB] = 'c';
          } else if (id >= 1 && id <= 10) {
            matrix_string[DIRECTION] = 'c';
            matrix_string[MSB] = 'p';
            matrix_string[LSB] = convert_int_to_char(id);

            if (id == 10) {
              matrix_string[DIRECTION] = 'p';
              matrix_string[MSB] = convert_int_to_char(id / 10);
              matrix_string[LSB] = convert_int_to_char(id % 10);
              ;
            }
          } else if (id >= 11 && id <= 73) {

            if (id >= 11 && id <= 19) {
              matrix_string[DIRECTION] = '-';
              matrix_string[MSB] = convert_int_to_char(id - 10);
              matrix_string[LSB] = 'c';
            }

            if (id >= 20) {
              matrix_string[DIRECTION] = '-';
              matrix_string[MSB] = convert_int_to_char((id - 10) / 10);
              matrix_string[LSB] = convert_int_to_char((id - 10) % 10);
            }
          }

          selected_id = id;
          while (btn_1_settings_mode == ID && btn_2_set_value_counter == 1 &&
                 btn_1_set_mode_counter == 2) {
            draw_string_on_matrix(matrix_string);
          }

          id++;
          if (id > ADDR_ID_LIMIT) {
            id = ADDR_ID_MIN;
          }
#if 0
          while (btn_1_settings_mode == ID && btn_2_set_value_counter == 1 &&
                 btn_1_set_mode_counter == 2) {

            switch (shift_mode) {
            case 1:
              draw_string_on_matrix(SHIFT_P_FLOOR);
              break;

            case 2:
              draw_string_on_matrix(SHIFT_MINUS_FLOOR);
              break;

            default:
              break;
            }
          }

          shift_mode++;

          if (shift_mode == 3) {
            shift_mode = 1;
          }
#endif

#endif
          break;

        case ESC:

          switch (selected_level_volume) {
          case 0:
            update_structure(&matrix_settings, VOLUME_0, selected_id);
            break;
          case 1:
            update_structure(&matrix_settings, VOLUME_1, selected_id);
            break;
          case 2:
            update_structure(&matrix_settings, VOLUME_2, selected_id);
            break;
          case 3:
            update_structure(&matrix_settings, VOLUME_3, selected_id);
            break;
          }

          btn_1_set_mode_counter = 0;
          btn_2_set_value_counter = 0;
          is_first_btn_clicked = true;
          matrix_string[DIRECTION] = 'c';
          matrix_string[MSB] = 'c';
          matrix_string[LSB] = 'c';
          menu_state = MENU_STATE_CLOSE;
          break;
        }

        if (btn_1_settings_mode != ESC) {
          btn_2_set_value_counter = 1;
        }

        break;
      }
    }
  }

#endif
}
