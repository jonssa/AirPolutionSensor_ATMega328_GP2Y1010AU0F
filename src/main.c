/*
 *
 *  Created on: 2018-01-02
 *       Autor: Maciej Joñca
 *       main.c
 */

/*!
 * do³¹czanie bibliotek
 */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*!
 * do³¹czanie plików nag³ówkowych
 */
#include "LCD/lcd44780.h"
#include "1Wire/ds18x20.h"
#include "BUTTONS/lcd_pwm.h"
#include "GP2Y1010AU0F/GP2Y1010AU0F.h"

/*!
 * deklaracje funkcji
 */
void showmenu(void);			///< Poka¿ menu nadrzêdne
void clr_first(void);			///< Jesli by³a zmiana pozycji w menu wyczyœc ekran
void display_temp(void);		///< Wyœwietl temperaturê
void temp_meas(void);			///< Pomiar temperatury
void init_timer(void);			///< Inicjalizacja 8-bitowego timera

/*!
 * main - funkcja g³ówna
 */
int main(void)
{
	/*!
	 * funkcje inicjalizuj¹ce
	 */
	lcd_init();
	init_keys();
	init_pwm();
	init_timer();
	init_int();
	init_adc();

	/*!
	 * za³adowanie znaków do pamiêci CGRAM
	 */
	lcd_defchar(0x80, znak_happy);
	lcd_defchar(0x81, znak_neutral);
	lcd_defchar(0x82, znak_sad);
	lcd_defchar(0x83, znak_3);
	lcd_defchar(0x84, znak_termo);

	sei();							///< zezwolenie na przerwania

	/*!
	 *  pêtla nieskoñczona programu
	 */
	while(1)
	{
		/*!
		 * w zale¿noœci od trybu poka¿ wybór menu nadrzêdne lub jedn¹ z jej pozycji
		 */
		switch (upper_menu)
		{
			case 0: showmenu();		///< poka¿ menu nadrzêdne
			break;

			case 1:
			{
				switch(menu)		///< w zale¿noœci od pozycji w menu wykonaj dan¹ czynnoœc
				{
					case 0:			///< pierwszy przypadek
					{
						clr_first();					///< wyczyœc ekran
						avg_adc = 0;
						iter=0;

						while (iter < MEANDUSTITER)		///< 10 pomiarów do œredniej
						{
							PORTB &= ~SENSOR_LED;		///< stan niski na tranzystor diody IR czujnika (w³¹cz diodê)
							_delay_us(176);				///< poczekaj 176 us
							adc_value = pomiar();		///< wykonaj pomiar i przypisz jego wartoœc do zmiennej
							_delay_us(40);				///< poczekaj 40 us
							PORTB |= SENSOR_LED;		///< stan wysoki na tranzystor diody IR czujnika (wy³¹cz diodê)
							_delay_us(9680);			///< poczekaj 9.68 ms
							_delay_ms(80);				///< poczekaj 80 ms

							if(adc_value>20)				///< jeœli napiêcie >0 (wykonano poprawny pomiar)
							{
								iter++;					///< inkrementuj zmienn¹
								avg_adc += adc_value;	///< dodaj napiêcie chwilowej do policzenia œredniej
							}
						}
						voltage = avg_adc * 110UL * 462UL;   	///< przelicz na napiêcie

						voltage /= MEANDUSTITER;				///< policz œredni¹

						cz_d = voltage/10000000;				///< policz czêœc ca³kowit¹
						cz_u = (voltage / 100000) % 100;		///< policz czêœc u³amkow¹

						D= (( cz_d*100 +cz_u)*17143UL/10000UL)-45;	///< przelicz na zapylenie

						avg_dust = (avg_dust + D)/2;				///< zapylenie œrednie z dwóch ostatnich pomiarów

						D_max = MAX(D_max, D);						///< znajdŸ najwiêksz¹ wartoœc zapylenia
						D_min = MIN(D_min, D);						///< znajdŸ najmiejsz¹ wartoœc zapylenia

						if(min_flag) count_dust_min();				///< jeœli minê³a minuta przejdŸ do funkcji
						if(h_flag) count_dust_h();					///< jeœli minê³a godzina przejdŸ do funkcji

						if (subm == 0)								///< wybór submenu
						{
							lcd_locate(0,0);					///< kursor na pozycji 0,0
							lcd_str("PM 2.5  ");				///< wypisz na ekran napis
							lcd_int(D);						///< wypisz na ekran wartoœc zapylenia powietrza
							lcd_str(" ug/m");
							lcd_str("\x83");					///< wypisz na ekran zdefiniowany wczeœniej znak
							lcd_str("  ");

							lcd_locate(1,10);

							/*!
							 * w zale¿noœci od zapylenia wyœwietl minkê
							 */
							if (D>110) lcd_str("(!!!) ");
							else if (D >= 55 && D < 110) lcd_str(" (\x82)  ");
							else if (D >= 30 && D < 55) lcd_str(" (\x81)  ");
							else lcd_str(" (\x80)  ");
							temp_meas();						///< pomiar temperatury
						}
						else										///< druga opcja submenu
						{
							lcd_locate(0,0);
							lcd_str("Jakosc powietrza:");

							/*!
							 * w zale¿noœci od zapylenia wypisz odpowiedni napis
							 */
							lcd_locate(1,0);
							if (D<15) lcd_str("  Bardzo dobra!  ");
							else if (D >= 15 && D < 30) lcd_str("     Dobra     ");
							else if (D >= 30 && D < 55) lcd_str("  Umiarkowana  ");
							else if (D >= 55 && D < 110) lcd_str("      Zla      ");
							else lcd_str("   Bardzo zla!    ");
						}
					}break;

					case 1:											///< drugi przypadek
					{
						if (subm == 0)								///< submenu
						{
							clr_first();

							lcd_locate(0,0);
							lcd_str_P(PSTR("Maximum: "));
							lcd_int(D_max);							///< wypisz wartoœc maksymalna

							lcd_locate(1,0);
							lcd_str_P(PSTR("Minimum: "));
							lcd_int(D_min);							///< wypisz wartoœc minimalna
						}
						else
						{
							clr_first();

							count_avg();							///< wyswietl srednie godzinne zapylenie
							show_caqi();							///< wyswietl wsploczynnik CAQI
						}
					}break;

					case 2:											///< trzecia opcja menu
					{
						uint8_t tmp;								///< zmienna tymczasowa
						clr_first();

						tmp = OCR0A * 100 / 255;					///< przelicz wartoœc rejestru OCR0A na procenty

						lcd_locate(0,0);
						lcd_str_P(PSTR("Jasnosc wysw:"));

						lcd_locate(1,5);
						lcd_int(tmp);								///< wyœwietl jasnoœc wyœwietlacza w procentach
						lcd_str_P(PSTR(" "));

						zwiekszjasnosc();							///< funkcja zwieksz jasnosc
						zmniejszjasnosc();							///< funkcja zmniejsz jasnosc
					}break;
				}
			}break;
		}
	}
}

/*!
 * showmenu - funkcja odpowiadaj¹ca za wyœwietlanie odpowiedniej pozycji w menu
 * 			  w zale¿noœci od zmiennej tryb - s¹ 3 pozycje menu
 */
void showmenu(void)
{
	clr_first();

	lcd_locate(0,5);
	lcd_str_P(PSTR("MENU"));

	lcd_locate(1,4);
	if ((tryb%3) == 0)
	{
		lcd_str_P(PSTR("Pomiar"));
	}
	else if ((tryb%3) == 1)
	{
		lcd_str_P(PSTR("Pamiec"));
	}
	else
	{
		lcd_str_P(PSTR("Opcje "));
	}
}

/*!
 * clr_first - funkcja odpowiadaj¹ca za czyszczenie ekranu po wykonanej operacji zmiany menu
 */
void clr_first(void)
{
	if(first_flag==1)		///< jeœli flaga wynosi 0 - wyzeruj ekran
	{
		lcd_cls();
		first_flag=0;		///< po wykonanej operacji ustaw flagê na 1
	}
}

/*!
 * display_temp - s³u¿y do wyœwietlania aktualnej temperatury na wyœwietlaczu LCD
 */
void display_temp(void)
{
	lcd_locate(1,0);
	lcd_str("\x84");
	if(subzero) lcd_str("-");		///< jeœli subzero==1 wyœwietla znak minus
	else lcd_str(" ");
	lcd_int(cel);					///< wyœwietl czêœc ca³kowit¹ temperatury
	lcd_str(".");
	lcd_int(cel_fract_bits); 		///< wyœwietl czêœc dziesiêtn¹ temperatury
	lcd_write_data(0xDF);			///< wyœwietl znak o w indeksie górnym
	lcd_str("C");
}

/*!
 * init_timer - funkcja inicjalizuj¹ca timer, ustawia odpowiednie wartoœci w rejestrach
 */
void init_timer(void)
{
	TCCR2A |= (1<<WGM21);							///< tryb CTC
	TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20);		///< preskaler = 1024
	OCR2A = 77;										///< rejestr przepe³nienia = 77
	TIMSK2 |= (1<<OCIE2A);							///< zezwolenie na przerwanie CompareMatch
}

/*!
 * temp_meas - funkcja dokonuj¹ca pomiaru temperatury za pomoc¹ interfejsu 1wire - wykorzystuje pliki z folderu 1Wire
 */
void temp_meas(void)
{
	if(s1_flag) 					///< sprawdzanie flagi sekund
	{
		if( 0 == (s%3) )			///< modulo 3 z liczby sekund - w 1 sekundzie dokonaj sprawdzenia czujników na magistrali
		{
			uint8_t *cl=(uint8_t*)gSensorIDs;								///< pobierz wskaŸnik do tablicy adresów czujników
			for( uint8_t i=0; i<MAXSENSORS*OW_ROMCODE_SIZE; i++) *cl++ = 0; ///< kasuj ca³¹ tablicê
			czujniki_cnt = search_sensors();								///< wykryj ile jest czujników na magistrali
		}

		if( 1 == (s%3) ) DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL ); 	///< w 2 sekundzie rozpocznij pomiar

		if( 2 == (s%3) )			///< w 3 sekundzie jeœli pomiar jest poprawny wyœwietl temperaturê - jeœli nie wyœwietl napis ERROR
		{
			if( DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel, &cel_fract_bits) ) display_temp();
			else
			{
				lcd_locate(1,0);
				lcd_str(" error ");
			}
		}
		s1_flag=0;					///< flaga sekund = 0
	}
}


ISR(TIMER2_COMPA_vect)
{
	static uint8_t cnt=0;			///< statyczna zmienna cnt do odliczania setnych milisekund

	if(++cnt>99)  					///< gdy minie 990 milisekund
	{
		s1_flag=1;					///< ustaw flagê sekundy
		s++;						///< zwiêksz licznik sekund

		if(s>2)					///< gdy minie 59 sekund
		{
			s=0; 					///< wyzeruj licznik sekund
			min++;					///< zwiêksz licznik minut
			min_flag = 1;			///< ustaw flagê minuty

			if(min>59)				///< jeœli minie 59 minut
			{
				h++;				///< zwiêksz licznik godzin
				h_flag = 1;			///< wyzeruj flagê godzin
				min = 0;			///< wyzeruj licznik minut
				if (h > 24) h = 0;	///< jeœli min¹ 24 h wyzeruj licznik godzin
			}
		}
		cnt=0;						///< wyzeruj licznik milisekund
	}
}

/*!
 * procedura obs³ugi przerwania na liniach PCINT[23:16],
 * w tej funkcji obs³uguje przyciski BUT1 oraz BUT2 pod³¹czone do portu D
 */
ISR( PCINT2_vect )
{
	if( !(PIND & BUT2 ) )			///< jeœli przycisk BUT2 zosta³ wciœniêty
	{
		_delay_ms(20);
		if( !(PIND & BUT2 ) )		///< programowe zniesienie zjawiska drgania styków
		{
		tryb += 1;					///< zwiêksz zmienn¹ tryb odpowiadaj¹ca za wybór menu
		if(tryb == 24) tryb = 0;	///< zmienna wynosi a¿ 24 poniewa¿ u¿ywana jest tak¿e do wype³niania wektora godzinnego zapylenia
		}
	}

	if( !(PIND & BUT1 ) )			///< jeœli przycisk BUT1 zosta³ wciœniêty
	{
		_delay_ms(20);
		if( !(PIND & BUT1 ) )		///< programowe zniesienie zjawiska drgania styków
		{
		tryb -= 1;					///< zmniejsz zmienn¹ tryb odpowiadaj¹ca za wybór menu
		if(tryb > 24) tryb = 23;
		}
	}
}

/*!
 * procedura obs³ugi przerwania na liniach PCINT[7:0],
 * w tej funkcji obs³uguje przyciski KEY_INT oraz KEY_INT1 pod³¹czone do portu B
 */
ISR( PCINT0_vect )
{
	first_flag = 1;					///< ustaw flagê czyszczenia ekranu

	if( !(PINB & KEY_INT ) )		///< jeœli przycisk KEY_INT zosta³ wciœniêty
	{
		_delay_ms(20);				///< programowe zniesienie zjawiska drgania styków
		if( !(PINB & KEY_INT ) )
		{
			if(upper_menu == 1)		///< zmienna upper_menu
			{
				upper_menu = 0;
				tryb = 0;			///< ustawienie zmiennej tryb na 0 blokuje zmianê wartoœci zmiennych za pomoc¹ przycisków BUT1 i BUT2
			}
			else
				upper_menu = 1;
			menu = (tryb % 3);		///< po naciœniêciu przypisz resztê z dzielenia przez 3 zmiennej tryb zmiennej menu
			subm = 0;					///< ustaw submenu na 0 (domyœlne)
		}
	}

	if( !(PINB & KEY_INT1 ) )		///< jeœli przycisk KEY_INT zosta³ wciœniêty
	{
		_delay_ms(20);
		if( !(PINB & KEY_INT1 ) )	///< programowe zniesienie zjawiska drgania styków
		{
			if(subm == 1)
				subm = 0;		///< zmiana submenu
			else
				subm += 1;
			tryb = 0;				///< ustawienie zmiennej tryb na 0 blokuje zmianê wartoœci zmiennych za pomoc¹ przycisków BUT1 i BUT2
		}
	}
}
