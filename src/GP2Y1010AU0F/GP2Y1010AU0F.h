/*
 *
 *  Created on: 2018-01-02
 *       Autor: Maciej Jo�ca
 *       GP2Y1010AU0F.h
 */

#ifndef GP2Y1010AU0F_GP2Y1010AU0F_H_
#define GP2Y1010AU0F_GP2Y1010AU0F_H_

#define SENSOR_LED_DDR		DDRB
#define SENSOR_LED_PORT		PORTB
#define SENSOR_LED 			1<<PB0

#define ADC_CHANNEL			0

#define MAX(a,b) 			(	(a>b)?a:b	)
#define MIN(a,b) 			(	(a<b)?a:b	)

#define MEANDUSTITER 		10

/*!
 * deklaracje zmiennych
 */
uint64_t voltage, avg_dust, avg_adc;	///< napi�cie ,zapylenie oraz warto�c adc �rednia czujnika
uint32_t adc_value;							///< warto�c ADC podczas pomiaru napi�cia
uint16_t D, D_max;			///< zmienna zapylenia chwilowego, minimalnego oraz maksymalnego
extern  uint16_t D_min;
uint8_t iter;								///< zmienna iteracyjna do pomiaru warto�ci �redniej zapylenia
uint8_t cz_d, cz_u;							///< cz�c ca�kowita oraz u�amkowa wy�wietlanego zapylenia powietrza

uint64_t dust_5min[12];		///< tablica przechowuj�ca warto�ci zapylenia pobieranego co 5 min
uint64_t dust_h[24];		///< tablica przechowuj�ca warto�ci �rednie godzinne

volatile uint8_t s1_flag, min_flag, h_flag, first_flag;		///< flagi sygnalizuj�ce mini�cie sekundy, minuty, godziny oraz															///< flaga zmiany pozycji w menu do czyszczenia ekranu
volatile uint8_t s, min, h;									///< licznik sekund, minut oraz godzin
volatile uint8_t tryb, upper_menu, subm, menu;				///< zmienne do obs�ugi menu

/*!
 * deklaracje funkcji
 */
void init_adc(void);
void count_dust_min(void);		///< Wpisywanie pomiaru zapylenia do tablicy do 5 min
void count_dust_h(void);		///< Obliczanie �redniej godzinnej oraz przesuwanie tablicy o jedn� pozycj� co godzin�
void count_avg(void);			///< Wypisywanie �redniej na ekran w zale�no�ci od wybranej godziny (domy�lnie aktualna)
void show_caqi(void);			///< Przelicznik �redniej godzinowej na wsp�czynnik CAQI i jego wy�wietlenie
uint16_t pomiar(void);

#endif /* GP2Y1010AU0F_GP2Y1010AU0F_H_ */
