/*
 * test_Buzzer.c
 *
 *  Created on: Jan 21, 2025
 *      Author: me
 */

#include "main.h"
#include "test_Buzzer.h"
// #include "tim.h"

#include <stdint.h>

extern TIM_HandleTypeDef htim2;

// Ocatve_3 Frequencies (C3->B3)
uint32_t OctaveSmall[NOTES_NUMBER] = {131, 139, 147, 155, 165, 175,
                                      185, 196, 208, 220, 233, 247};

// Ocatve_4 Frequencies (C4->B4)
uint32_t Octave1[NOTES_NUMBER] = {262, 277, 294, 311, 330, 349,
                                  370, 392, 415, 440, 466, 494};

// Ocatve_5 Frequencies (C5->B5)
uint32_t Octave2[NOTES_NUMBER] = {523, 554, 587, 622, 659, 698,
                                  740, 784, 831, 880, 932, 988};

// Ocatve_6 Frequencies (C6->B6)
uint32_t Octave3[NOTES_NUMBER] = {1047, 1109, 1175, 1245, 1319, 1367,
                                  1480, 1568, 1661, 1760, 1865, 1976};

// Ocatve_7 Frequencies (C7->B7)
uint32_t Octave4[NOTES_NUMBER] = {2093, 2218, 2349, 2489, 2637, 2794,
                                  2960, 3136, 3322, 3520, 3729, 3951};

uint8_t NoteIndex = 0;

void Tone(uint32_t Frequency, uint32_t Duration);
void noTone();

void Test_BuzzerStart(void) { HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); }

#define ARRAY_SIZE 4
uint8_t Array[ARRAY_SIZE] = {0, 64, 128, 255};
void Test_BuzzerArray(void) {
  static uint8_t i = 0;
  TIM2->CCR2 = Array[i++];
  if (i == 4)
    i = 0;
  HAL_Delay(500);
}

void Test_BuzzerCounst(void) {
  static uint8_t i = 0;
  static uint8_t direction = 1;

  if (direction) {
    TIM2->CCR2 = i++;
  } else {
    TIM2->CCR2 = i--;
  }

  if ((i == 255 && direction) || (i == 255 && !direction))
    direction = !direction;
  HAL_Delay(1);
}

void Tone(uint32_t Frequency, uint32_t Duration) {
  TIM2->ARR = (1000000UL / Frequency) - 1; // Set The PWM Frequency
  TIM2->CCR2 = (TIM2->ARR >> 1);           // Set Duty Cycle 50%
  HAL_Delay(Duration);                     // Wait For The Tone Duration
}

#define NO_TONE_IS_100_PECVENT
//#define NO_TONE_IS_0_PECVENT

#ifdef NO_TONE_IS_0_PECVENT
static void noTone() {
  TIM2->CCR2 = 0; // Set Duty Cycle 0%
}
#endif

#ifdef NO_TONE_IS_100_PECVENT
void noTone() {
  TIM2->CCR2 = TIM2->ARR; // Set Duty Cycle 100%
}
#endif

void Test_BuzzerNotes(void) {

  // octave 3
  Tone(OctaveSmall[NOTE_C], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_C_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_D], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_D_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_E], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_F], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_F_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_G], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_G_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_A], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_A_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(OctaveSmall[NOTE_B], 500);
  noTone();
  HAL_Delay(100);

  // octave 4
  Tone(Octave1[NOTE_C], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_C_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_D], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_D_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_E], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_F], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_F_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_G], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_G_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_A], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_A_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave1[NOTE_B], 500);
  noTone();
  HAL_Delay(100);

  // octave 5
  Tone(Octave2[NOTE_C], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_C_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_D], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_D_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_E], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_F], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_F_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_G], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_G_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_A], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_A_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave2[NOTE_B], 500);
  noTone();
  HAL_Delay(100);

  // octave 6
  Tone(Octave3[NOTE_C], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_C_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_D], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_D_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_E], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_F], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_F_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_G], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_G_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_A], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_A_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave3[NOTE_B], 500);
  noTone();
  HAL_Delay(100);

  // octave 7
  Tone(Octave4[NOTE_C], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_C_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_D], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_D_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_E], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_F], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_F_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_G], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_G_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_A], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_A_diese], 500);
  noTone();
  HAL_Delay(100);

  Tone(Octave4[NOTE_B], 500);
  noTone();
  HAL_Delay(100);
}

void Test_CCR_Gdiese(void) {
  uint8_t percentPWM = 50;
  TIM2->ARR = (1000000UL / Octave4[NOTE_G_diese]) - 1; // Set The PWM Frequency

#if 0
    while (percentPWM <= 100) {
      TIM2->CCR2 = ((TIM2->ARR / 100) * percentPWM++);
      HAL_Delay(50);
    }
#endif

  TIM2->CCR2 = ((TIM2->ARR / 100) * 75);

  HAL_Delay(2000);
  TIM2->CCR2 = ((TIM2->ARR / 100) * 50);
  HAL_Delay(2000);
}

void MX_TIM2_Init_1kHz(void) {

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 280;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 256 - 1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 128 - 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);
}

void MX_TIM2_Init_1uS(void) {

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 64 - 1; // freq tim = 1 000 000
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);
}

struct note_and_duration_t {
  enum notes_t note;
  enum octaves_t octave;
  uint8_t duration;
};

/* ───────────────────────────────────────────────────────────────────────────────────
 */
// Für Elise - Very Easy Piano tutorial - Beginner
struct note_and_duration_t For_Elise[] = {
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E_flat, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E_flat, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 4},

    {.note = NOTE_C, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 4},

    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_A_flat, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 4},

    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},

    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E_flat, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E_flat, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 4},

    {.note = NOTE_C, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 4},

    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 4},

    /*---------------------------------------------------------------*/

    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E_flat, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E_flat, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 4},

    {.note = NOTE_C, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 4},

    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_A_flat, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 4},

    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},

    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E_flat, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E_flat, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 4},

    {.note = NOTE_C, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 4},

    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_B, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 4},

    /*---------------------------------------------------------------*/
};

static uint16_t For_Elise_length(void) {
  uint16_t length = 0;
  length = sizeof(For_Elise) / sizeof(struct note_and_duration_t);
  return length;
}

void Test_For_Elise(void) {
  uint16_t length = For_Elise_length();
  uint16_t note_dutation = 250;
  uint16_t counter = 0;

  while (counter < length) {
    switch (For_Elise[counter].octave) {
    case OCTAVE_SMALL_C3_B3:
      Tone(OctaveSmall[For_Elise[counter].note],
           note_dutation * For_Elise[counter].duration);
      break;

    case OCRAVE_1_C4_B4:
      Tone(Octave1[For_Elise[counter].note],
           note_dutation * For_Elise[counter].duration);
      break;

    case OCRAVE_2_C5_B5:
      Tone(Octave2[For_Elise[counter].note],
           note_dutation * For_Elise[counter].duration);
      break;

    case OCRAVE_3_C6_B6:
      Tone(Octave3[For_Elise[counter].note],
           note_dutation * For_Elise[counter].duration);
      break;

    case OCRAVE_4_C7_B7:
      Tone(Octave4[For_Elise[counter].note],
           note_dutation * For_Elise[counter].duration);
      break;
    default:
      break;
    }
    counter++;
  }
  noTone();
  HAL_Delay(note_dutation);
}

/* ───────────────────────────────────────────────────────────────────────────────────
 */

struct note_and_duration_t Cosmic_music[] = {
    {.note = NOTE_D, .octave = OCRAVE_2_C5_B5, .duration = 4},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_2_C5_B5, .duration = 2},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 4},

    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_2_C5_B5, .duration = 1},

    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_2_C5_B5, .duration = 1},

    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_2_C5_B5, .duration = 1},

    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_2_C5_B5, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 2},

    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 8},
};

static uint16_t Cosmic_music_length(void) {
  uint16_t length = 0;
  length = sizeof(Cosmic_music) / sizeof(struct note_and_duration_t);
  return length;
}

/* Эдуард Артемьев - Поход */
void Test_Cosmic_music(void) {
  uint16_t length = Cosmic_music_length();
  uint16_t note_dutation = 200;
  uint16_t counter = 0;

  while (counter < length) {
    switch (Cosmic_music[counter].octave) {
    case OCTAVE_SMALL_C3_B3:
      Tone(OctaveSmall[Cosmic_music[counter].note],
           note_dutation * Cosmic_music[counter].duration);
      break;

    case OCRAVE_1_C4_B4:
      Tone(Octave1[Cosmic_music[counter].note],
           note_dutation * Cosmic_music[counter].duration);
      break;

    case OCRAVE_2_C5_B5:
      Tone(Octave2[Cosmic_music[counter].note],
           note_dutation * Cosmic_music[counter].duration);
      break;

    case OCRAVE_3_C6_B6:
      Tone(Octave3[Cosmic_music[counter].note],
           note_dutation * Cosmic_music[counter].duration);
      break;

    case OCRAVE_4_C7_B7:
      Tone(Octave4[Cosmic_music[counter].note],
           note_dutation * Cosmic_music[counter].duration);
      break;
    default:
      noTone();
      HAL_Delay(Cosmic_music[counter].duration);
      break;
    }
    counter++;
  }
  noTone();
  HAL_Delay(note_dutation);
}

/* ───────────────────────────────────────────────────────────────────────────────────
 */

struct note_and_duration_t Sorcerers_doll[] = {
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 1},

    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},

    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 1},

    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},

    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 1},

    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 1},

    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},

    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 1},

    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},

    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
};

static uint16_t Sorcerers_doll_length(void) {
  uint16_t length = 0;
  length = sizeof(Sorcerers_doll) / sizeof(struct note_and_duration_t);
  return length;
}

void Test_Sorcerers_doll(void) {
  uint16_t length = Sorcerers_doll_length();
  uint16_t note_dutation = 250;
  uint16_t counter = 0;

  while (counter < length) {
    switch (Sorcerers_doll[counter].octave) {
    case OCTAVE_SMALL_C3_B3:
      Tone(OctaveSmall[Sorcerers_doll[counter].note],
           note_dutation * Sorcerers_doll[counter].duration);
      break;

    case OCRAVE_1_C4_B4:
      Tone(Octave1[Sorcerers_doll[counter].note],
           note_dutation * Sorcerers_doll[counter].duration);
      break;

    case OCRAVE_2_C5_B5:
      Tone(Octave2[Sorcerers_doll[counter].note],
           note_dutation * Sorcerers_doll[counter].duration);
      break;

    case OCRAVE_3_C6_B6:
      Tone(Octave3[Sorcerers_doll[counter].note],
           note_dutation * Sorcerers_doll[counter].duration);
      break;

    case OCRAVE_4_C7_B7:
      Tone(Octave4[Sorcerers_doll[counter].note],
           note_dutation * Sorcerers_doll[counter].duration);
      break;
    default:
      noTone();
      HAL_Delay(Sorcerers_doll[counter].duration);
      break;
    }
    noTone();
    HAL_Delay(note_dutation / 10);
    counter++;
  }
  noTone();
  HAL_Delay(note_dutation);
}

/* ───────────────────────────────────────────────────────────────────────────────────
 */

struct note_and_duration_t Forester[] = {
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},

    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},

    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 2},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 2},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 2},

    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 2},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 2},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 2},

    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 2},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 2},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},

    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 2},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 2},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},

    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},

    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 2},

    /*---------------------------------------------------------------*/
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 3},

    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},

    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 3},

    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},

    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 3},

    {.note = NOTE_C, .octave = OCRAVE_2_C5_B5, .duration = 1},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_A_diese, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_A, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},

    {.note = NOTE_C, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTE_C, .octave = OCRAVE_1_C4_B4, .duration = 3},

    {.note = NOTE_C, .octave = OCRAVE_1_C4_B4, .duration = 1},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_E, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_F, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},
    {.note = NOTE_G, .octave = OCRAVE_1_C4_B4, .duration = 3},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 1},

    {.note = NOTE_D, .octave = OCRAVE_1_C4_B4, .duration = 2},
    {.note = NOTES_NUMBER, .octave = OCRAVES_NUMBER, .duration = 2},

};

static uint16_t Forester_length(void) {
  uint16_t length = 0;
  length = sizeof(Forester) / sizeof(struct note_and_duration_t);
  return length;
}

void Test_Forester(void) {
  uint16_t length = Forester_length();
  uint16_t note_dutation = 100;
  uint16_t counter = 0;

  while (counter < length) {
    switch (Forester[counter].octave) {
    case OCTAVE_SMALL_C3_B3:
      Tone(OctaveSmall[Forester[counter].note],
           note_dutation * Forester[counter].duration);
      break;

    case OCRAVE_1_C4_B4:
      Tone(Octave1[Forester[counter].note],
           note_dutation * Forester[counter].duration);
      break;

    case OCRAVE_2_C5_B5:
      Tone(Octave2[Forester[counter].note],
           note_dutation * Forester[counter].duration);
      break;

    case OCRAVE_3_C6_B6:
      Tone(Octave3[Forester[counter].note],
           note_dutation * Forester[counter].duration);
      break;

    case OCRAVE_4_C7_B7:
      Tone(Octave4[Forester[counter].note],
           note_dutation * Forester[counter].duration);
      break;
    default:
      noTone();
      HAL_Delay(Forester[counter].duration);
      break;
    }
    noTone();
    HAL_Delay(note_dutation / 10);
    counter++;
  }
  noTone();
  HAL_Delay(note_dutation);
}
