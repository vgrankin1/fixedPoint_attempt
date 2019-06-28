// ConsoleApplication4.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
typedef uint32 fx16q16_t;
typedef uint32 fx5q27_t;
#define fx16q16_make(a)  ((fx16q16_t)((a)<<16))
#define fx5q27_make(a)  ((fx5q27_t)((a)<<27))
Для обратного преобразования, как не странно, мы сдвигаем наше число обратно.
#define fx16q16_uint16(a)((uint16)((a) >> 16))
#define fx5q27_uint8(a)  ((uint8)((a)>>27))


Можно преобразовать и в число с плавающей запятой:
#define fx16q16_float(a) ((a) / (float)(1LL<<16))


Для преобразование числа из одного положения фиксированной запятой в другое, его нужно сдвинуть на разницу в количестве дробный бит:

#define fx16q16_to_fx5q27(a) ((fx5q27_t)a<<11)
ложение и вычитание числе с фиксированной запятой точно такое-же, как и у целых чисел. Можно даже не определять макросы для этого

fx16q16_make(5)+fx16q16_make(8)==fx16q16_make(13)
fx16q16_make(18)-fx16q16_make(4)==fx16q16_make(14)


А вот для умножения и деления нам понадобится контейнер в два раза большего размера для хранения промежуточного результата, ведь при умножении 32 бит на 32 бита может получиться 64 бита:

#define fx16q16_mul(a,b) ((fx16q16_t)(((int64_t)(a) * (b)) >> 16))
#define fx16q16_div(a,b) ((fx16q16_t)(((int64_t)(a) << 16) / (b)))


Кстати, если вам нужно разделить на 10, или любую другую константу, можно заставить препроцессор посчитать 1/10, а потом — умножать на это число. Такой финт ушами позволяет сильно ускориться на процессорах без аппаратного делителя. К примеру:

fx16q16_t result = fx16q16_mul(number_to_divide_by_10, fx16q16_make(1.0f/10.0f))


Если вам нужно умножить или разделить на целое число, то можно делать эту операцию прямо на месте, без никаких сдвигов и преобразований

fx16q16_t mul_by_5 = fx_number * 5
fx16q16_t div_by_5 = fx_number / 5


На счет умножения и деления чисел с фиксированной точкой. 64-х разрядное умножение и особенно деление может быть очень медленным. Особенно на 8-ми и 16-ти битниках. А умножение и деление можно реализовать и без использования типов с большей разрядностью. Например умножение q16:
uint32_t mult_q16(uint32_t a, uint32_t b)
{
uint32_t inta = a >> 16;
uint32_t intb = b >> 16;
uint32_t fracta = a & 0x0000ffff;
uint32_t fractb = b & 0x0000ffff;
uint32_t result = (inta * intb << 16) + (inta * fractb + fracta * intb) + (fracta * fractb >> 16);
return result;
}

Нужно 4 умножения 16х16=>32 бита.
Деление q16:
uint32_t div_q16(uint32_t a, uint32_t b)
{
uint32_t r = a >> 16, q = a << 16;
uint_fast8_t carry = 0;
for (uint_fast8_t i = 0; i < 32; i++)
{
if(r & 0x80000000)
carry = 1;
else
carry = 0;
r <<= 1;
if (q & 0x80000000)
r |= 1;
q <<= 1;
if (r >= b || carry)
{
q = q | 1;
r -= b;
}
}
return q;
}

Хоть оно и побитовое, но работает в разы бsстрее, чем х64 деление на Cortex-m3. Может быть можно сделать еще быстрее используя аппаратное деление 32/32=>32 и алгоритм Кнута.

*/


unsigned str2fix(const char *str, const int frac_bits)
{
	long long decimal = 0, frac, frac_dec;
	int length;
	char buffer[256];
	length = 0;//Длина строки
	for (int i = 0, sym; ; i++)/*разбор строки,*/
	{
		if ((sym = str[i]) == 0)
		{
			buffer[length] = 0;
			break;
		}
		if (sym == '.')
		{
			buffer[length] = 0;
			decimal = strtol(buffer, 0, 10);
			length = 0;
		}
		else
		{
			buffer[length++] = str[i];
			if (length > 255)
				return -1;
		}
	}
	if (length > frac_bits)
		length = frac_bits;
	for (; length < frac_bits; buffer[length++] = '0') {};
	buffer[length] = 0;
	frac_dec = strtoull( buffer, 0, 10);
	frac = 0;
	long long coef = 1, coefdec, rescoef;

	for (int i = 0; i < frac_bits; i++)
	{
		
		frac = frac << 1;
		coefdec = 1;
		for (int j = 0; j < length - 1 - i; j++)
		{
			coefdec *= 10;
		}
		coef *= 5;
		rescoef = coef*coefdec;
		if (frac_dec >= rescoef)
		{
			frac_dec -= rescoef;
			frac = frac | 1;
		}
	}
	if (decimal < 0)
		frac = -frac;
	return (decimal << frac_bits) + frac;
}

int fix2str(char *str_out, const unsigned str_size, const int frac_bits, const unsigned int value)
{
	int decimal, frac, length = 0;
	long long frac_dec, coef = 1, coefdec;
	char buffer[256];
	frac_dec = 0;//дробное число переведенное в десятичное и домноженное на 10^n
	if (value & (1 << 31))
	{
		int us_value = -int(value);
		decimal = us_value >> frac_bits;
		frac = us_value - (decimal << frac_bits);
		str_out[0] = '-';
		str_out[1] = 0;
		length++;
	}
	else 
	{
		decimal = value >> frac_bits;
		frac = value - (decimal << frac_bits);
		str_out[0] = 0;
	}
	itoa(decimal, buffer, 10);
	if ((length += strlen(buffer)) >= (int)str_size)
		return -1;
	strcat(str_out, buffer);
	for (int i = 0; i < frac_bits && frac != 0; i++)
	{
		/*
		Вычисление коэффициентов степени 2 дробной части, вида 2^-1, 2^-2, 2^-3
		*/
		coefdec = 1;
		for (int j = 0; j < frac_bits - 1 - i; j++)
		{
			coefdec *= 10;
		}
		coef *= 5;
		if (frac & (1 << (frac_bits - 1 - i)))
			frac_dec += coef*coefdec;
	}
	sprintf(buffer, "%lld", frac_dec);
	str_out[length++] = '.';
	frac = strlen(buffer);
	for (; frac < frac_bits; str_out[length++] = '0', frac++) {}
	str_out[length] = 0;
	if ((length += strlen(buffer)) >= (int)str_size)
		return -1;
	strcat(str_out, buffer);
	return length;
}

unsigned fixadd(const unsigned a, const unsigned fbitsa, const unsigned b, const unsigned fbitsb)
{
	return 0;
}

void fixmul(const unsigned a, const unsigned b, const unsigned frac_bits, unsigned *lc, unsigned *hc)
{
	unsigned long long res = unsigned long long(a)*b >> frac_bits;
	*lc = res;
	if (hc)
		*hc = res >> 32;
}

int main(int argc, char* argv[])
{
	char buffer[256];
	unsigned val1, val2, valc;
	val1 = str2fix("2.0", 18);
	val2 = str2fix("2.2", 18);
	//valc = val1 + val2;
	fixmul(val1, val2, 18, &valc, 0);
	fix2str(buffer, 256, 18, valc);
	return 0;
}

