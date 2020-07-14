/*
 *
 *  Created on: 2018-01-02
 *       Autor: Maciej Joñca
 *       lcd_pwm.h
 */

#ifndef BUTTONS_LCD_PWM_H_
#define BUTTONS_LCD_PWM_H_

/*!
 * przypisanie przycisków i anody wyœwietlacza do portów
 */
#define KEY_INT		(1<<PB7)
#define KEY_INT1	(1<<PB6)

#define BUT1 		(1<<PD5)
#define BUT2 		(1<<PD7)

#define LCD_A 		(1<<PD6)

uint8_t in0, in1, in2, in3;		///< zmienne zbocz
uint16_t zlicz;					///< zmienna inkrementacji

void init_keys(void);			///< Inicjalizacja przycisków
void init_pwm(void);			///< Inicjalizacja PWM
void init_int(void);			///< Inicjalizacja przerwañ
void zwiekszjasnosc(void);		///< Zwiêksz jasnoœc
void zmniejszjasnosc(void);		///< Zmniejsz jasnoœc

#endif /* BUTTONS_LCD_PWM_H_ */
