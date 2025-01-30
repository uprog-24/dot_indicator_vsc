/*
 * test_Buzzer.h
 *
 *  Created on: Jan 21, 2025
 *      Author: me
 */

#ifndef TESTS_TEST_BUZZER_H_
#define TESTS_TEST_BUZZER_H_

enum notes_t {
  NOTE_C,
  NOTE_C_diese,
  NOTE_D,
  NOTE_D_diese,
  NOTE_E,
  NOTE_F,
  NOTE_F_diese,
  NOTE_G,
  NOTE_G_diese,
  NOTE_A,
  NOTE_A_diese,
  NOTE_B,
  NOTES_NUMBER
};

#define NOTE_E_flat NOTE_D_diese
#define NOTE_A_flat NOTE_G_diese

enum octaves_t {
  OCTAVE_SMALL_C3_B3,
  OCRAVE_1_C4_B4,
  OCRAVE_2_C5_B5,
  OCRAVE_3_C6_B6,
  OCRAVE_4_C7_B7,
  OCRAVES_NUMBER
};

void MX_TIM2_Init_1uS(void);
void Test_BuzzerStart(void);
void Test_BuzzerArray(void);
void Test_BuzzerCounst(void);
void Test_BuzzerNotes(void);
void Test_CCR_Gdiese(void);
void Test_For_Elise(void);
void Test_Cosmic_music(void);
void Test_Sorcerers_doll(void);
void Test_Forester(void);

void Tone(uint32_t Frequency, uint32_t Duration);

#endif /* TESTS_TEST_BUZZER_H_ */
