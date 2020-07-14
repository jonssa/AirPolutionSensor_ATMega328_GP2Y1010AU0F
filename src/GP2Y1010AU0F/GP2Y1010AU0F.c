/*
 *
 *  Created on: 2018-01-02
 *       Autor: Maciej Jońca
 *       GP2Y1010AU0F.c
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "GP2Y1010AU0F.h"
#include "../LCD/lcd44780.h"


uint16_t D_min = 1000;

/*!
 * init_adc - funkcja inicjalizująca przetwornik ADC
 */
void init_adc()
{
	ADCSRA |= (1<<ADEN);							///< włącz ADC
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1);				///< preskaler = 64
	ADMUX |= (1<<REFS1) | (1<<REFS0);				///< ustaw napięcie odniesienia na 1.1V

	SENSOR_LED_DDR |= SENSOR_LED;					///< Ustaw pin do którego jest podłączona dioda czujnika jako wyjście
}

/*!
 * count_dust_min - funkcja która co 5 minut do tablicy wpisuje wartośc zapylenia powietrza
 */
void count_dust_min(void)
{
	static uint8_t iter_5min = 0;				///< zmienna statyczna do odwoływania się do odpowiednich elementów tablicy

	if((min % 5) == 0 )							///< jeśli modulo 5 z liczby minut wynosi 0 (jest wielokrotnośc 5 minut)
	{
		dust_5min[iter_5min] = avg_dust;
		if (iter_5min < 12) iter_5min ++;		///< jeśli wartośc zmiennej jest mniejsza niż 11 zwiększ ją o 1
		else iter_5min = 0;						///< w przeciwnym wypadku wyzeruj
	}
	min_flag = 0;							///< ustaw flagę minut na 0
}

/*!
 * count_dust_h - ta funkcja ma za zadanie wpisywac do wektora średnią godzinną zapylenia powietrza
 */
void count_dust_h(void)
{
	uint8_t i,j;													///< zmienne lokalne

	for(j=23; j>0; j--) dust_h[j] = dust_h[j-1];					///< przesuwanie wektora o jedną pozycję
	for(i=0; i<12; i++) dust_h[0] = (dust_h[0] + dust_5min[i]);		///< dodawanie wartosci z pomiaru 5 minutowego do 1 elementu tablicy

	dust_h[0] /= 12;												///< policz średnią dla ostatniej godziny
	D_max = 0;
	D_min = 1000;
	h_flag = 0;														///< flaga godzin na 0
}

/*!
 * count_avg - funkcja wyświetlająca wartośc średnią godzinną na wyświetlacz
 */
void count_avg(void)
{
	lcd_locate(0,0);
	lcd_str("Avg -");
	lcd_int(tryb+1);
	lcd_str("h ");


	lcd_int(dust_h[tryb]);			///< wyświetl wartośc zapylenia godzinnego w zależności od wybranej godziny
	lcd_str(" mg/m");
	lcd_str("\x83");
	lcd_str("     ");

}

/*!
 * show_caqi - przeliczanie godzinnej wartości zapylenia na współczynnik CAQI
 */
void show_caqi(void)
{
	uint8_t tmp;						///< zmienna lokalna
	tmp = dust_h[tryb];					///< przypisanie wartości do zmiennej

	lcd_locate(1,0);
	if (tmp<15) lcd_str("  Bardzo dobra!  ");
	else if (tmp >= 15 && tmp < 30) lcd_str("     Dobra     ");
	else if (tmp >= 30 && tmp < 55) lcd_str("  Umiarkowana  ");
	else if (tmp >= 55 && tmp < 110) lcd_str("      Zla      ");
	else lcd_str("   Bardzo zla!    ");
}

/*!
 * pomiar - funkcja dokonująca pomiaru na wybranym kanale ADC_CHANNEL
 */
inline uint16_t pomiar()
{
	ADMUX = (ADMUX & 0b11111000) | ADC_CHANNEL;		///< maskowanie 3 młodszych bitów aby załączyc pomiar na wybranym kanale
	ADCSRA |= (1<<ADSC);							///< rozpocznij pomiar

	while(ADCSRA & (1<<ADSC));						///< gdy zakończy się pomiar bit ADSC w rejestrze ADCSRA jest zerowany
													///< jeśli jest 1 kontynuuj pomiar
	return ADCW;									///< zwróc zawartośc rejestru ADCW (połączone rejestry ADCL oraz ADCH)
}
