/*
 *
 *  Created on: 2018-01-02
 *       Autor: Maciej Jo�ca
 *       main.c
 */

/*!
 * do��czanie bibliotek
 */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*!
 * do��czanie plik�w nag��wkowych
 */
#include "LCD/lcd44780.h"
#include "1Wire/ds18x20.h"
#include "BUTTONS/lcd_pwm.h"
#include "GP2Y1010AU0F/GP2Y1010AU0F.h"

/*!
 * deklaracje funkcji
 */
void showmenu(void);			///< Poka� menu nadrz�dne
void clr_first(void);			///< Jesli by�a zmiana pozycji w menu wyczy�c ekran
void display_temp(void);		///< Wy�wietl temperatur�
void temp_meas(void);			///< Pomiar temperatury
void init_timer(void);			///< Inicjalizacja 8-bitowego timera

/*!
 * main - funkcja g��wna
 */
int main(void)
{
	/*!
	 * funkcje inicjalizuj�ce
	 */
	lcd_init();
	init_keys();
	init_pwm();
	init_timer();
	init_int();
	init_adc();

	/*!
	 * za�adowanie znak�w do pami�ci CGRAM
	 */
	lcd_defchar(0x80, znak_happy);
	lcd_defchar(0x81, znak_neutral);
	lcd_defchar(0x82, znak_sad);
	lcd_defchar(0x83, znak_3);
	lcd_defchar(0x84, znak_termo);

	sei();							///< zezwolenie na przerwania

	/*!
	 *  p�tla niesko�czona programu
	 */
	while(1)
	{
		/*!
		 * w zale�no�ci od trybu poka� wyb�r menu nadrz�dne lub jedn� z jej pozycji
		 */
		switch (upper_menu)
		{
			case 0: showmenu();		///< poka� menu nadrz�dne
			break;

			case 1:
			{
				switch(menu)		///< w zale�no�ci od pozycji w menu wykonaj dan� czynno�c
				{
					case 0:			///< pierwszy przypadek
					{
						clr_first();					///< wyczy�c ekran
						avg_adc = 0;
						iter=0;

						while (iter < MEANDUSTITER)		///< 10 pomiar�w do �redniej
						{
							PORTB &= ~SENSOR_LED;		///< stan niski na tranzystor diody IR czujnika (w��cz diod�)
							_delay_us(176);				///< poczekaj 176 us
							adc_value = pomiar();		///< wykonaj pomiar i przypisz jego warto�c do zmiennej
							_delay_us(40);				///< poczekaj 40 us
							PORTB |= SENSOR_LED;		///< stan wysoki na tranzystor diody IR czujnika (wy��cz diod�)
							_delay_us(9680);			///< poczekaj 9.68 ms
							_delay_ms(80);				///< poczekaj 80 ms

							if(adc_value>20)				///< je�li napi�cie >0 (wykonano poprawny pomiar)
							{
								iter++;					///< inkrementuj zmienn�
								avg_adc += adc_value;	///< dodaj napi�cie chwilowej do policzenia �redniej
							}
						}
						voltage = avg_adc * 110UL * 462UL;   	///< przelicz na napi�cie

						voltage /= MEANDUSTITER;				///< policz �redni�

						cz_d = voltage/10000000;				///< policz cz�c ca�kowit�
						cz_u = (voltage / 100000) % 100;		///< policz cz�c u�amkow�

						D= (( cz_d*100 +cz_u)*17143UL/10000UL)-45;	///< przelicz na zapylenie

						avg_dust = (avg_dust + D)/2;				///< zapylenie �rednie z dw�ch ostatnich pomiar�w

						D_max = MAX(D_max, D);						///< znajd� najwi�ksz� warto�c zapylenia
						D_min = MIN(D_min, D);						///< znajd� najmiejsz� warto�c zapylenia

						if(min_flag) count_dust_min();				///< je�li min�a minuta przejd� do funkcji
						if(h_flag) count_dust_h();					///< je�li min�a godzina przejd� do funkcji

						if (subm == 0)								///< wyb�r submenu
						{
							lcd_locate(0,0);					///< kursor na pozycji 0,0
							lcd_str("PM 2.5  ");				///< wypisz na ekran napis
							lcd_int(D);						///< wypisz na ekran warto�c zapylenia powietrza
							lcd_str(" ug/m");
							lcd_str("\x83");					///< wypisz na ekran zdefiniowany wcze�niej znak
							lcd_str("  ");

							lcd_locate(1,10);

							/*!
							 * w zale�no�ci od zapylenia wy�wietl mink�
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
							 * w zale�no�ci od zapylenia wypisz odpowiedni napis
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
							lcd_int(D_max);							///< wypisz warto�c maksymalna

							lcd_locate(1,0);
							lcd_str_P(PSTR("Minimum: "));
							lcd_int(D_min);							///< wypisz warto�c minimalna
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

						tmp = OCR0A * 100 / 255;					///< przelicz warto�c rejestru OCR0A na procenty

						lcd_locate(0,0);
						lcd_str_P(PSTR("Jasnosc wysw:"));

						lcd_locate(1,5);
						lcd_int(tmp);								///< wy�wietl jasno�c wy�wietlacza w procentach
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
 * showmenu - funkcja odpowiadaj�ca za wy�wietlanie odpowiedniej pozycji w menu
 * 			  w zale�no�ci od zmiennej tryb - s� 3 pozycje menu
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
 * clr_first - funkcja odpowiadaj�ca za czyszczenie ekranu po wykonanej operacji zmiany menu
 */
void clr_first(void)
{
	if(first_flag==1)		///< je�li flaga wynosi 0 - wyzeruj ekran
	{
		lcd_cls();
		first_flag=0;		///< po wykonanej operacji ustaw flag� na 1
	}
}

/*!
 * display_temp - s�u�y do wy�wietlania aktualnej temperatury na wy�wietlaczu LCD
 */
void display_temp(void)
{
	lcd_locate(1,0);
	lcd_str("\x84");
	if(subzero) lcd_str("-");		///< je�li subzero==1 wy�wietla znak minus
	else lcd_str(" ");
	lcd_int(cel);					///< wy�wietl cz�c ca�kowit� temperatury
	lcd_str(".");
	lcd_int(cel_fract_bits); 		///< wy�wietl cz�c dziesi�tn� temperatury
	lcd_write_data(0xDF);			///< wy�wietl znak o w indeksie g�rnym
	lcd_str("C");
}

/*!
 * init_timer - funkcja inicjalizuj�ca timer, ustawia odpowiednie warto�ci w rejestrach
 */
void init_timer(void)
{
	TCCR2A |= (1<<WGM21);							///< tryb CTC
	TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20);		///< preskaler = 1024
	OCR2A = 77;										///< rejestr przepe�nienia = 77
	TIMSK2 |= (1<<OCIE2A);							///< zezwolenie na przerwanie CompareMatch
}

/*!
 * temp_meas - funkcja dokonuj�ca pomiaru temperatury za pomoc� interfejsu 1wire - wykorzystuje pliki z folderu 1Wire
 */
void temp_meas(void)
{
	if(s1_flag) 					///< sprawdzanie flagi sekund
	{
		if( 0 == (s%3) )			///< modulo 3 z liczby sekund - w 1 sekundzie dokonaj sprawdzenia czujnik�w na magistrali
		{
			uint8_t *cl=(uint8_t*)gSensorIDs;								///< pobierz wska�nik do tablicy adres�w czujnik�w
			for( uint8_t i=0; i<MAXSENSORS*OW_ROMCODE_SIZE; i++) *cl++ = 0; ///< kasuj ca�� tablic�
			czujniki_cnt = search_sensors();								///< wykryj ile jest czujnik�w na magistrali
		}

		if( 1 == (s%3) ) DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL ); 	///< w 2 sekundzie rozpocznij pomiar

		if( 2 == (s%3) )			///< w 3 sekundzie je�li pomiar jest poprawny wy�wietl temperatur� - je�li nie wy�wietl napis ERROR
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
		s1_flag=1;					///< ustaw flag� sekundy
		s++;						///< zwi�ksz licznik sekund

		if(s>2)					///< gdy minie 59 sekund
		{
			s=0; 					///< wyzeruj licznik sekund
			min++;					///< zwi�ksz licznik minut
			min_flag = 1;			///< ustaw flag� minuty

			if(min>59)				///< je�li minie 59 minut
			{
				h++;				///< zwi�ksz licznik godzin
				h_flag = 1;			///< wyzeruj flag� godzin
				min = 0;			///< wyzeruj licznik minut
				if (h > 24) h = 0;	///< je�li min� 24 h wyzeruj licznik godzin
			}
		}
		cnt=0;						///< wyzeruj licznik milisekund
	}
}

/*!
 * procedura obs�ugi przerwania na liniach PCINT[23:16],
 * w tej funkcji obs�uguje przyciski BUT1 oraz BUT2 pod��czone do portu D
 */
ISR( PCINT2_vect )
{
	if( !(PIND & BUT2 ) )			///< je�li przycisk BUT2 zosta� wci�ni�ty
	{
		_delay_ms(20);
		if( !(PIND & BUT2 ) )		///< programowe zniesienie zjawiska drgania styk�w
		{
		tryb += 1;					///< zwi�ksz zmienn� tryb odpowiadaj�ca za wyb�r menu
		if(tryb == 24) tryb = 0;	///< zmienna wynosi a� 24 poniewa� u�ywana jest tak�e do wype�niania wektora godzinnego zapylenia
		}
	}

	if( !(PIND & BUT1 ) )			///< je�li przycisk BUT1 zosta� wci�ni�ty
	{
		_delay_ms(20);
		if( !(PIND & BUT1 ) )		///< programowe zniesienie zjawiska drgania styk�w
		{
		tryb -= 1;					///< zmniejsz zmienn� tryb odpowiadaj�ca za wyb�r menu
		if(tryb > 24) tryb = 23;
		}
	}
}

/*!
 * procedura obs�ugi przerwania na liniach PCINT[7:0],
 * w tej funkcji obs�uguje przyciski KEY_INT oraz KEY_INT1 pod��czone do portu B
 */
ISR( PCINT0_vect )
{
	first_flag = 1;					///< ustaw flag� czyszczenia ekranu

	if( !(PINB & KEY_INT ) )		///< je�li przycisk KEY_INT zosta� wci�ni�ty
	{
		_delay_ms(20);				///< programowe zniesienie zjawiska drgania styk�w
		if( !(PINB & KEY_INT ) )
		{
			if(upper_menu == 1)		///< zmienna upper_menu
			{
				upper_menu = 0;
				tryb = 0;			///< ustawienie zmiennej tryb na 0 blokuje zmian� warto�ci zmiennych za pomoc� przycisk�w BUT1 i BUT2
			}
			else
				upper_menu = 1;
			menu = (tryb % 3);		///< po naci�ni�ciu przypisz reszt� z dzielenia przez 3 zmiennej tryb zmiennej menu
			subm = 0;					///< ustaw submenu na 0 (domy�lne)
		}
	}

	if( !(PINB & KEY_INT1 ) )		///< je�li przycisk KEY_INT zosta� wci�ni�ty
	{
		_delay_ms(20);
		if( !(PINB & KEY_INT1 ) )	///< programowe zniesienie zjawiska drgania styk�w
		{
			if(subm == 1)
				subm = 0;		///< zmiana submenu
			else
				subm += 1;
			tryb = 0;				///< ustawienie zmiennej tryb na 0 blokuje zmian� warto�ci zmiennych za pomoc� przycisk�w BUT1 i BUT2
		}
	}
}
