#ifndef __BITMAP_H
#define __BITMAP_H

/*
 Программа для преобразования битмапа для горизонтального ДОТп в битмап для вертикального

			#include <stdio.h>
			#include <inttypes.h>

			void rotate (uint8_t * bitmap_7_ptr, uint8_t * bitmap_6_ptr, uint8_t start_bit)
			{
				int i, j;

                bitmap_6_ptr[0] = 0;

				for(int i = 0; i <= 5; ++i)
					for(int j = 0; j < 7; ++j)
						if(bitmap_7_ptr[j] & (1 << ( start_bit + i) ))
							bitmap_6_ptr[i+1] |= 1 << (7 - j);
			}

			void printBinary(uint8_t byte)
			{
				printf("0B");

				for(int i = 7; i >= 0; --i)
					if(byte & (1 << i))
						putchar('1');
					else
						putchar('0');

				putchar(',');
				putchar('\n');
			}

			int main()
			{
				uint8_t bitmap_7[7] =
				{
								0B00011100,			<< вставить битмап сюда
								0B00100010,			<< вставить битмап сюда
								0B00000010,			<< вставить битмап сюда
								0B00000100,			<< вставить битмап сюда
								0B00001000,			<< вставить битмап сюда
								0B00010000,			<< вставить битмап сюда
								0B00111110			<< вставить битмап сюда
				};

				uint8_t bitmap_5[5] = { 0 };
				rotate (bitmap_7, bitmap_5, 1);

				for(int i = 0; i < 5; ++i)
					printBinary(bitmap_5[i]);

				return 0;
			}
 */


// #define __ARROW_DOUBLE
	// доступные для вывода символы
	typedef enum {
		SYMBOL_DIGIT_ZERO = 0,
		SYMBOL_DIGIT_ONE,
		SYMBOL_DIGIT_TWO,
		SYMBOL_DIGIT_THREE,
		SYMBOL_DIGIT_FOUR,
		SYMBOL_DIGIT_FIVE,
		SYMBOL_DIGIT_SIX,
		SYMBOL_DIGIT_SEVEN,
		SYMBOL_DIGIT_EIGHT,
		SYMBOL_DIGIT_NINE,
		SYMBOL_ARROW_UP,
		SYMBOL_ARROW_DOWN,
		SYMBOL_ARROW_BOTH,
		SYMBOL_UNDERFLOOR,
		SYMBOL_MINUS_SIGN,
		SYMBOL_LETTER_F,
		SYMBOL_LETTER_K,
		SYMBOL_LETTER_G,
		SYMBOL_LETTER_EE,
		SYMBOL_LETTER_C,
		SYMBOL_LETTER_R,
		SYMBOL_LETTER_E,
		SYMBOL_ARROW_ANIMATION_UP_DYNAMIC,
		SYMBOL_ARROW_ANIMATION_DOWN_DYNAMIC,
		SYMBOL_UNDERSCORE,
		SYMBOL_LETTER_B,
		SYMBOL_LETTER_A,
		SYMBOL_ALL_ON,
		SYMBOL_CLEAN_BITMAP					// всегда последний!
	} symbol_table_t;

#endif
