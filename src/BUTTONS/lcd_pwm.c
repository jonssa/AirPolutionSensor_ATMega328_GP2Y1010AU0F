/*
 *
 *  Created on: 2018-01-02
 *       Autor: Maciej Jo�ca
 *       lcd_pwm.c
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lcd_pwm.h"

/*!
 * init_keys - funkcja inicjalizuj�ca przyciski oraz anod� wy�wietlacza
 */
void init_keys(void)
{
	DDRD |= LCD_A; 					///< LCD_A jako wyj�cie
	DDRD &= ~ (BUT1 | BUT2); 		///< przyciski BUT1 i BUT2 jako wej�cia
	PORTD |= BUT1 | BUT2; 			///< podci�ganie przycisk�w BUT1 i BUT2 do VCC

	DDRB &= ~(KEY_INT | KEY_INT1);	///< przyciski KEY_INT i KEY_INT1 jako wej�cia
	PORTB |= KEY_INT | KEY_INT1;	///< podci�ganie przycisk�w KEY_INT i KEY_INT1 do VCC
}

/*!
 * init_pwm - funkcja inicjalizujaca - PWM zainicjalizowany na porcie PD6 ---> LCD
 */
void init_pwm(void)
{
	TCCR0A |= (1<<WGM01) | (1<<WGM00); 		///< tryb fast PWM
	TCCR0A |= (1<<COM0A1);					///< wyzeruj rejestr OC0A po osi�gni�ciu zadanej warto�ci
	TCCR0B |= (1<<CS00); 					///< preskaler = 1
	OCR0A = 128; 							///< inicjalizacja wy�wietlacza ze wsp�czynnikiem wype�nienia 50 %
}

/*!
 * init_int - funkcja inicjalizujaca przerwania PCINT
 */
void init_int(void)
{
	PCICR |= (1<<PCIE0) | (1<<PCIE2);		///< zezwolenie na przerwania liniach PCINT[7:0] oraz PCINT[23:16]
	PCMSK0 |= KEY_INT | KEY_INT1;			///< wpisanie do rejestru PCMSK0 odpowiadaj�cego za linie PCINT[7:0]
											///< na zezwolenie na przerwania na przyciskach KEY_INT oraz KEY_INT1
	PCMSK2 |= BUT1 | BUT2;					///< wpisanie do rejestru PCMSK2 odpowiadaj�cego za linie PCINT[23:16]
											///< na zezwolenie na przerwania na przyciskach BUT1 oraz BUT2
}

/*!
 * zmniejszjasnosc - funkcja odpowiadajaca za zmniejszanie jasnosci wyswietlacza
 */
void zmniejszjasnosc(void)
{

	in1 = (PIND & BUT1);										///< sprawdz stan linii z przyciskiem BUT2 (in1 - stan tera�niejszy)

	if(in0 & (~in1))											///< je�eli wyst�pi�o zbocze narastaj�ce
	{
		if(OCR0A >= 10 && OCR0A <= 255)		OCR0A -= 5;			///< je�li OCR0A jest w podanym przedziale to zmniejsz rejestr o 5
	}

	if((~PIND) & BUT1)											///< je�eli przycisk jest nadal przyci�ni�ty
	{
		zlicz += 1;												///< zwi�ksz zmienn� zlicz o 1
		if ((zlicz >300) && (zlicz % 30 == 1))					///< je�eli warunki s� spe�nione
		{
			if(OCR0A >= 10 && OCR0A <= 255) 	OCR0A -= 5;		///< je�li OCR0A jest w podanym przedziale to zmniejsz rejestr o 5
		}

	}

	if((~in0) & in1) zlicz = 0;									///< je�li przycisk zosta� zwolniony wyzeruj zmienn� zlicz
	in0 = in1;													///< przypisz zmiennej in0 zmienn� in1 (in0 - stan poprzedni)
}

/*!
 * zwiekszjasnosc - funkcja odpowiadajaca za zwiekszanie jasnosci wyswietlacza
 * rozumowanie poni�szej funkcji jest takie same jak dla funkcji zwiekszjasnosc()
 */
void zwiekszjasnosc(void)
{
	in3 = (PIND & BUT2);
	if(in2 & (~in3))
	{
		if(OCR0A >= 0 && OCR0A <=245)	OCR0A += 5;
	}

	if((~PIND) & BUT2)
	{
		zlicz += 1;
		if ((zlicz >300) && (zlicz % 30 == 1))
		{
			if(OCR0A >= 0 && OCR0A <=245)	OCR0A += 5;
		}
	}

	if((~in2) & in3) zlicz = 0;
	in2 = in3;
}

