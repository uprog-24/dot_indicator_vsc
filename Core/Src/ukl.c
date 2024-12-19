/**
 * @file ukl.c
 */
#include "ukl.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "tim.h"

#define PACKET_SIZE 13               ///< Number of data bits in received packet
#define CODE_FLOOR_MASK 0x3F         ///< Mask for 0-5 bits
#define DIRECTION_MASK 0xC0          ///< Mask for 6,7 bits
#define CONTROL_BITS_MASK 0x1FC0     ///< Mask for 6-12 bits
#define SPECIAL_SYMBOLS_BUFF_SIZE 7  ///< Number of special symbols
#define DELAY_MS_DATA_RECEIVE \
  200 - 1  ///< Delay after receiving 13 bytes before next reading pin
#define GONG_BUZZER_FREQ 3000  ///< Frequency of bip for ARRIVAL gong
#define BUZZER_FREQ_CABIN_OVERLOAD \
  5000  ///< Frequency of bip for VOICE_CABIN_OVERLOAD
#define BUZZER_FREQ_FIRE_DANGER \
  BUZZER_FREQ_CABIN_OVERLOAD  ///< Frequency of bip for FIRE_DANGER
#define FILTER_BUFF_SIZE \
  5  ///< Size of buffer with received data (width of filter)

/**
 * Stores values of code location
 */
typedef enum CODE_LOCATION_UKL {
  LOCATION_P = 57,
  LOCATION_P1 = 58,
  LOCATION_P2 = 59,
  LOCATION_MINUS_1 = 60,
  LOCATION_MINUS_2 = 61,
  LOCATION_MINUS_3 = 62,
  LOCATION_MINUS_4 = 63,
} code_location_ukl_t;

/**
 * Stores values of control bits
 */
typedef enum CONTROL_BITS_STATES {
  CABIN_OVERLOAD = 0x100,
  GONG_ARRIVAL = 0x200,
  SIGNAL_PRESS_ORDER_BUTTON = 0x400,
  TRANSPORTATION_BEDRIDDEN_PATIENTS = 0x800,
  FARE_DANGER = 0x1000
} control_bits_states_t;

/**
 * Stores values of direction of the movement (UKL)
 */
typedef enum {
  UKL_MOVE_UP = 128,
  UKL_MOVE_DOWN = 64,
  UKL_NO_MOVE = 0
} direction_ukl_t;

/// Buffer with code location and it's symbols
static const code_location_symbols_t
    special_symbols_code_location[SPECIAL_SYMBOLS_BUFF_SIZE] = {
        {.code_location = LOCATION_P2, .symbols = "p2"},
        {.code_location = LOCATION_P1, .symbols = "p1"},
        {.code_location = LOCATION_P, .symbols = "p"},
        {.code_location = LOCATION_MINUS_1, .symbols = "-1"},
        {.code_location = LOCATION_MINUS_2, .symbols = "-2"},
        {.code_location = LOCATION_MINUS_3, .symbols = "-3"},
        {.code_location = LOCATION_MINUS_4, .symbols = "-4"}};

/// Flag to control if cabin is overloaded
static bool is_cabin_overload_sound = false;

/// Counter for number received data (cabin is overloaded)
static uint8_t cabin_overload_cnt = 0;

/// Flag to control if gong is playing
static volatile bool is_gong_play = false;

/// Flag to control pressing order button
static bool is_press_order_button = false;

/// Counter for number received data (order button is pressed)
static uint8_t order_button_cnt = 0;

/// Counter for number received data (order button is disable sound)
static uint8_t button_disable_cnt = 0;

/// Counter for number received data (fire danger)
static uint8_t fire_danger_cnt = 0;

/// Flag to control fire danger
static bool is_fire_danger_sound = false;

/// Counter for number received data (fire danger is disable sound)
static uint8_t fire_disable_cnt = 0;

/**
 * @brief  Setting sound of buzzer.
 * @note   Checking state of bits (1/0) in order to turn on buzzer sound
 * @param  matrix_string: Pointer to the output matrix_string with symbols
 * @param  control_bits:  Received control bits
 * @retval None
 */
static void setting_sound_ukl(char *matrix_string,
                              control_bits_states_t control_bits) {
  if (control_bits == (control_bits_states_t)UKL_MOVE_UP ||
      control_bits == (control_bits_states_t)UKL_MOVE_DOWN ||
      control_bits == (control_bits_states_t)UKL_NO_MOVE) {
    stop_buzzer_sound();
  }

  if ((control_bits & TRANSPORTATION_BEDRIDDEN_PATIENTS) ==
      TRANSPORTATION_BEDRIDDEN_PATIENTS) {
    matrix_string[DIRECTION] = '+';
  }

  if ((control_bits & SIGNAL_PRESS_ORDER_BUTTON) == SIGNAL_PRESS_ORDER_BUTTON) {
    order_button_cnt++;
    if (order_button_cnt > 5U) {  // 1150 ms
      stop_buzzer_sound();
      order_button_cnt = 0;
#if 1
      play_gong(1, GONG_BUZZER_FREQ, matrix_settings.volume);
#endif
      is_press_order_button = true;
    }

    if (is_press_order_button) {
      button_disable_cnt++;
      if (button_disable_cnt == 3) {  // 690 ms, STOP
        button_disable_cnt = 0;
        is_press_order_button = false;
      }
    }

  } else {
    order_button_cnt = 0;
    button_disable_cnt = 0;
    is_press_order_button = false;
  }

  if ((control_bits & GONG_ARRIVAL) == GONG_ARRIVAL) {
    if (!is_gong_play) {
      stop_buzzer_sound();
#if 1
      play_gong(3, GONG_BUZZER_FREQ, matrix_settings.volume);
#endif
      is_gong_play = true;
    }
  } else {
    is_gong_play = false;
  }

  if ((control_bits & CABIN_OVERLOAD) == CABIN_OVERLOAD) {  // ?
    cabin_overload_cnt++;
    if (cabin_overload_cnt > 5U) {  // 1150 ms
      stop_buzzer_sound();
      cabin_overload_cnt = 0;
#if 1
      TIM2_Start_bip(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
#endif
      is_cabin_overload_sound = true;
    }

  } else {
    cabin_overload_cnt = 0;
    is_cabin_overload_sound = false;
  }

  if ((control_bits & FARE_DANGER) == FARE_DANGER) {
    matrix_string[MSB] = 'F';
    matrix_string[LSB] = 'c';
    fire_danger_cnt++;
    if (fire_danger_cnt > 5U) {  // 1150 ms, START
      stop_buzzer_sound();
      fire_danger_cnt = 0;
#if 1
      TIM2_Start_bip(BUZZER_FREQ_FIRE_DANGER, VOLUME_3);
#endif
      is_fire_danger_sound = true;
    }

    if (is_fire_danger_sound) {
      fire_disable_cnt++;
      if (fire_disable_cnt == 3) {  // 690 ms, STOP
        fire_disable_cnt = 0;
        is_fire_danger_sound = false;
        stop_buzzer_sound();
      }
    }

  } else {
    is_fire_danger_sound = false;
    fire_disable_cnt = 0;
    fire_danger_cnt = 0;
  }
}

/// Structure for data that will be displayed on matrix
static drawing_data_t drawing_data = {0, 0};

/**
 * @brief  Transform UKL values of direction to common directionType that
 *         defined in drawing.h
 * @param  direction: Value from enum direction_ukl_t:
 *                    UKL_MOVE_UP/UKL_MOVE_DOWN/UKL_NO_MOVE
 * @retval None
 */
static void transform_direction_to_common(direction_ukl_t direction) {
  switch (direction) {
    case UKL_MOVE_UP:
      drawing_data.direction = DIRECTION_UP;
      break;
    case UKL_MOVE_DOWN:
      drawing_data.direction = DIRECTION_DOWN;
      break;
    case UKL_NO_MOVE:
      drawing_data.direction = NO_DIRECTION;
      break;

    default:
      drawing_data.direction = NO_DIRECTION;
      break;
  }
}

/// Flag to control is data filtered before displaying it on matrix
static bool is_data_filtered = false;

/// Number of received data
static uint8_t number_received_data = 0;

/// Current index of the element in filter_buff
static uint8_t current_index_buff = 0;

/**
 * Stores the parameters for filtering received data that will be displayed on
 * matrix: floor, direction, control_bits and counter that save number of
 * repetitions of the data
 */
typedef struct {
  uint8_t floor;
  direction_ukl_t direction;
  control_bits_states_t control_bits;
  uint8_t counter;
} floor_counter_t;

/// Buffer that store received data and its repetitions
static floor_counter_t filter_buff[FILTER_BUFF_SIZE];

/// Copy of received_data_ukl (13 bits received by UKL)
static volatile uint16_t received_data_ukl_copy = 1;

/**
 * @brief  Setting structure with type floor_counter_t
 * @param  filter_struct: Pointer to the structure with type floor_counter_t
 * @param  floor:         Received floor
 * @param  direction:     Received direction
 * @param  control_bits:  Received control_bits
 * @param  counter:       Save number of repetitions of the data
 * @retval None
 */
static void set_filter_structure(floor_counter_t *filter_struct, uint8_t floor,
                                 direction_ukl_t direction,
                                 control_bits_states_t control_bits,
                                 uint8_t counter) {
  filter_struct->floor = floor;
  filter_struct->direction = direction;
  filter_struct->control_bits = control_bits;
  filter_struct->counter = counter;
}

/**
 * @brief  Sorting filter_buff in descending order using the bubble method.
 * @note   filter_buff[0].counter has maximum value and will be displayed on
 *         matrix
 * @param  filter_buff: Pointer to the buffer with received data by UKL
 * @param  buff_size:   Size of filter_buff
 * @retval None
 */
static void sort_bubble(floor_counter_t *filter_buff, uint8_t buff_size) {
  for (uint8_t i = 0; i < buff_size - 1; i++) {
    for (uint8_t k = 0; k < buff_size - i - 1; k++) {
      if (filter_buff[k].counter < filter_buff[k + 1].counter) {
        floor_counter_t temp = filter_buff[k];
        filter_buff[k] = filter_buff[k + 1];
        filter_buff[k + 1] = temp;
      }
    }
  }
}

/**
 * @brief  Save received data in filter_buff in order to display data that has
 *         maximum number of repetitions (filter_buff[i].counter).
 * @note   1. Initialize buffer with structures;
 *            get floor, direction, control_bits from received data
 *            (received_data_ukl_copy);
 *         2. Increase filter_buff[i].counter if filter_buff
 *            already has current received data, if current received data is
 *            not filter_buff then add it with filter_buff[i].counter = 1;
 *         3. If counter number_received_data == FILTER_BUFF_SIZE then call
 *            sort_bubble, filter_buff[0].counter has maximum value and will be
 *            displayed on matrix
 * @param  None
 * @retval None
 */
static void filter_data() {
  if (current_index_buff == 0) {
    for (uint8_t i = 0; i < FILTER_BUFF_SIZE; i++) {
      set_filter_structure(&filter_buff[i], 0, 0, 0, 0);
    }
    current_index_buff = 0;
  }

  uint8_t floor = received_data_ukl_copy & CODE_FLOOR_MASK;
  direction_ukl_t direction = received_data_ukl_copy & DIRECTION_MASK;
  control_bits_states_t control_bits =
      received_data_ukl_copy & CONTROL_BITS_MASK;

  bool is_data_found = false;
  number_received_data++;

  for (uint8_t i = 0; i < current_index_buff; i++) {
    if (filter_buff[i].floor == floor &&
        filter_buff[i].direction == direction &&
        filter_buff[i].control_bits == control_bits) {
      filter_buff[i].counter++;
      is_data_found = true;
      break;
    }
  }

  if (!is_data_found && current_index_buff < FILTER_BUFF_SIZE - 1) {
    set_filter_structure(&filter_buff[current_index_buff], floor, direction,
                         control_bits, 1);
    current_index_buff++;
  }

  if (number_received_data == FILTER_BUFF_SIZE) {
    sort_bubble(filter_buff, FILTER_BUFF_SIZE);

    is_data_filtered = true;
    number_received_data = 0;
    current_index_buff = 0;
  }
}

/// Received control bits
static control_bits_states_t control_bits;

/// String that will be displayed on matrix
static char matrix_string[3];

/// Flag to control is data completed
volatile bool is_read_data_completed = false;

/**
 * @brief  Process data using UKL protocol
 * @note   1. Filter data;
 *         2. If is_data_filtered then set drawing_data structure, setting
 *            symbols and sound;
 *         3. Display matrix_string while next data is not received and
 *            interface is connected.
 * @param  None
 * @retval None
 */
static void process_data_ukl() {
#if 1

  filter_data();

  if (is_data_filtered) {
    is_data_filtered = false;

    transform_direction_to_common(filter_buff[0].direction);
    drawing_data.floor = filter_buff[0].floor;
    control_bits = filter_buff[0].control_bits;

    setting_symbols(matrix_string, &drawing_data, MAX_POSITIVE_NUMBER_LOCATION,
                    special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);

    if (matrix_settings.volume != VOLUME_0) {
    	setting_sound_ukl(matrix_string, control_bits);
    }

  }

#endif

#if 0
  transform_direction_to_common(received_data_ukl_copy & DIRECTION_MASK);
  drawing_data.floor = received_data_ukl_copy & CODE_FLOOR_MASK;
  control_bits = received_data_ukl_copy & CONTROL_BITS_MASK;

  if (drawing_data.floor != 63) {
    setting_symbols(matrix_string, &drawing_data, MAX_POSITIVE_NUMBER_LOCATION,
                    special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);
    setting_sound_ukl(matrix_string, control_bits);
  }
#endif

  while (is_read_data_completed == false && is_interface_connected) {
    draw_string_on_matrix(matrix_string);
  }
}

/**
 * @brief  Process data received by DATA_Pin.
 * @note   If transmitted data by UKL protocol is received then process data
 * @param  None
 * @retval None
 */
void process_data_pin() {
  if (is_read_data_completed) {
    is_read_data_completed = false;

    process_data_ukl();
  }
}

/// Variable for 13 bits received by UKL
static volatile uint16_t received_data_ukl = 1;

/// Index of received data bit
static volatile uint8_t bit_index = 0;

/// Flag to control is start bit is received (state DATA_Pin from 1 to 0)
volatile bool is_start_bit_received = false;

/// Buffer with timings for reading data bits
const uint16_t ukl_timings[PACKET_SIZE] = {3100, 1850, 1900, 1900, 1900,
                                           1900, 1900, 2050, 4100, 1650,
                                           1900, 2150, 1900};

/// Value of the current received bit
volatile uint8_t bit = 1;

/// Delay after receiving 13 bytes before next reading pin
static const uint16_t delay_ms_data_receive = DELAY_MS_DATA_RECEIVE;

/// Number of data bits in received packet
static const uint8_t packet_size = PACKET_SIZE;

/// Maximum index of received data
static const uint8_t max_index_packet = PACKET_SIZE - 1;

/**
 * @brief  Read data bit when time from ukl_timings[PACKET_SIZE] is elapsed.
 * @note   Function is called in HAL_TIM_PeriodElapsedCallback
 *         1. Read DATA_Pin, add bit to received_data_ukl by bit_index, stop
 *            TIM3 and start it for next bit;
 *         2. Check received_data_ukl_copy (not all 1), counters for interface
 *            connection;
 *         3. Start TIM3 for finished delay in 200 ms;
 *         4. If bit_index == packet_size (13 bits) then reset states of
 *            variables. Set enable IRQ for reading DATA_Pin.
 * @param  None
 * @retval None
 */
void read_data_bit() {
  bit = HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin);

  if (bit_index == max_index_packet) {
    bit_index++;
  }

  // read 0 - 11 bits
  if (bit_index < max_index_packet) {
    received_data_ukl |= (bit << bit_index);

    // ukl_timings for next bit
    bit_index++;
    TIM3_Stop();
    TIM3_Start(PRESCALER_FOR_US, ukl_timings[bit_index]);
  } else if (!is_read_data_completed) {
    // read last bit (12 bit)
    //  if (bit_index == max_index_packet) {
    received_data_ukl |= (bit << bit_index);
    received_data_ukl_copy = received_data_ukl;

    // filter "+-4"
    if ((received_data_ukl_copy & 0xFFF) != 0xFFF) {
      is_read_data_completed = true;
    }

    TIM3_Stop();

    alive_cnt[0] = (alive_cnt[0] < UINT32_MAX) ? alive_cnt[0] + 1 : 0;
    is_interface_connected = true;

    TIM3_Start(PRESCALER_FOR_MS, delay_ms_data_receive);
  }

  // delay in 200 ms is elapsed
  if (bit_index == packet_size) {
    bit_index = 0;
    is_start_bit_received = false;
    TIM3_Stop();
    received_data_ukl = 0;
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  }
}

/**
 * @brief  Stop protocol, reset variables when matrix_state = MATRIX_STATE_MENU
 * @param  None
 * @retval None
 */
void stop_ukl_before_menu_mode() {
  TIM3_Stop();
  is_start_bit_received = false;
  is_read_data_completed = false;
  bit_index = 0;
  received_data_ukl = 0;
}
