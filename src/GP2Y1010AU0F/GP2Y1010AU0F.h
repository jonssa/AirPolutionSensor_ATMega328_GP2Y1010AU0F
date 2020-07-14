/*
 *
 *  Created on: 2018-01-02
 *       Autor: Maciej Joñca
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
uint64_t voltage, avg_dust, avg_adc;	///< napiêcie ,zapylenie oraz wartoœc adc œrednia czujnika
uint32_t adc_value;							///< wartoœc ADC podczas pomiaru napiêcia
uint16_t D, D_max;			///< zmienna zapylenia chwilowego, minimalnego oraz maksymalnego
extern  uint16_t D_min;
uint8_t iter;								///< zmienna iteracyjna do pomiaru wartoœci œredniej zapylenia
uint8_t cz_d, cz_u;							///< czêœc ca³kowita oraz u³amkowa wyœwietlanego zapylenia powietrza

uint64_t dust_5min[12];		///< tablica przechowuj¹ca wartoœci zapylenia pobieranego co 5 min
uint64_t dust_h[24];		///< tablica przechowuj¹ca wartoœci œrednie godzinne

volatile uint8_t s1_flag, min_flag, h_flag, first_flag;		///< flagi sygnalizuj¹ce miniêcie sekundy, minuty, godziny oraz															///< flaga zmiany pozycji w menu do czyszczenia ekranu
volatile uint8_t s, min, h;									///< licznik sekund, minut oraz godzin
volatile uint8_t tryb, upper_menu, subm, menu;				///< zmienne do obs³ugi menu

/*!
 * deklaracje funkcji
 */
void init_adc(void);
void count_dust_min(void);		///< Wpisywanie pomiaru zapylenia do tablicy do 5 min
void count_dust_h(void);		///< Obliczanie œredniej godzinnej oraz przesuwanie tablicy o jedn¹ pozycjê co godzinê
void count_avg(void);			///< Wypisywanie œredniej na ekran w zale¿noœci od wybranej godziny (domyœlnie aktualna)
void show_caqi(void);			///< Przelicznik œredniej godzinowej na wspó³czynnik CAQI i jego wyœwietlenie
uint16_t pomiar(void);

#endif /* GP2Y1010AU0F_GP2Y1010AU0F_H_ */
