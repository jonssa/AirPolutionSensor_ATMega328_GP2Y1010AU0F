/*
 *
 *  Created on: 2018-01-02
 *       Autor: Maciej Jo�ca
 *       lcd_pwm.h
 */

#ifndef BUTTONS_LCD_PWM_H_
#define BUTTONS_LCD_PWM_H_

/*!
 * przypisanie przycisk�w i anody wy�wietlacza do port�w
 */
#define KEY_INT		(1<<PB7)
#define KEY_INT1	(1<<PB6)

#define BUT1 		(1<<PD5)
#define BUT2 		(1<<PD7)

#define LCD_A 		(1<<PD6)

uint8_t in0, in1, in2, in3;		///< zmienne zbocz
uint16_t zlicz;					///< zmienna inkrementacji

void init_keys(void);			///< Inicjalizacja przycisk�w
void init_pwm(void);			///< Inicjalizacja PWM
void init_int(void);			///< Inicjalizacja przerwa�
void zwiekszjasnosc(void);		///< Zwi�ksz jasno�c
void zmniejszjasnosc(void);		///< Zmniejsz jasno�c

#endif /* BUTTONS_LCD_PWM_H_ */
