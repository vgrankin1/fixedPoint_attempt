// fixed_point.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>

#include <climits>
#include <cstdint>

#include <string>


//https://habr.com/ru/post/131171/

union MyUnion
{
	unsigned int ival;
	float	fval;
};
/*A = (-1)**s * 2**(q-f) * M
* float: f = 127
*
*
*/

/*
*https://neerc.ifmo.ru/wiki/index.php?title=%D0%9F%D1%80%D0%B5%D0%B4%D1%81%D1%82%D0%B0%D0%B2%D0%BB%D0%B5%D0%BD%D0%B8%D0%B5_%D0%B2%D0%B5%D1%89%D0%B5%D1%81%D1%82%D0%B2%D0%B5%D0%BD%D0%BD%D1%8B%D1%85_%D1%87%D0%B8%D1%81%D0%B5%D0%BB
union {
	float    fl;
	uint32_t dw;
} f;
int s = ( f.dw >> 31 ) ? -1 : 1;   
int e = (f.dw >> 23) & 0xFF;     
int m = e ? (f.dw & 0x7FFFFF) | 0x800000 :	(f.dw & 0x7FFFFF) << 1;
*/
int get_signf(const float value)
{
	MyUnion m;
	m.fval = value;
	return (m.ival >> 31);
}
int get_qf(const float value)
{
	MyUnion m;
	m.fval = value;
	return (m.ival >> 23) & 0xff;
}
int get_mf(const float value)
{
	MyUnion m;
	m.fval = value;
	return m.ival & 0x7FFFFF;
}
bool is_zero(const float value)
{
	MyUnion m;
	m.fval = value;
	return m.ival == 0;
}
int get_inf(const float value)
{
	MyUnion m;
	m.fval = value;
	if (get_mf(value) != 0 || get_qf(value) != 255)
	{
		return 0;
	}
	if (get_signf(value))
		return -1;
	else
		return 1;
}
bool is_nan(const float value)
{
	MyUnion m;
	m.fval = value;
	if (get_qf(value) != 255 && get_mf(value) != 0x400000)
		return false;
	return true;
}

int str_to_fixed(const char *str, const uint8_t frac_bits)
{
	char fracstr[10];
	int intv = 0;
	int intvflag = 0;
	int fracv = 0;
	int p = 0;
	int f = 0;
	/*copy integer part of str
	*/
	for (char symb; (symb = str[p]) != 0; p++)
	{
		if(symb >= '0' && symb <= '9')
			intv = intv * 10 + str[p] - '0';
		else
		{
			if (symb == '.' || symb == ',')
				break;
			if (symb == '-' && p == 0)
				intvflag = 1;
			//error state
		}
	}
	if (intvflag == 1)
		intv = -intv;
	
	/*copy fractional part of str
	*/
	for (char symb; ; p++)
	{
		symb = str[p];
		if (symb == 0)
			break;
		fracstr[f++] = symb - '0';
	}
	
	for (int j = 0; j < frac_bits; j++)
	{
		int carry = 0;
		for (int i = f; i; i--)
		{
			fracstr[i - 1] = (fracstr[i - 1] << 1) + carry;
			carry = 0;
			if (fracstr[i - 1] > 9)
			{
				fracstr[i - 1] -= 10;
				carry = 1;
			}
		}
		fracv = (fracv << 1) | carry;
	}

	return (intv << frac_bits) | fracv;
}

int fixed_to_str(char *buffer, const int bufcnt, const int value, const uint8_t frac_bits)
{
	char fracstr[11];
	int intv = value >> frac_bits;
	int writen;

	const char *bins[] = {
		"5000000000", "2500000000", "1250000000", "0625000000", "0312500000", "0156250000", "0078125000", "0039062500", "0019531250", "0009765625",
		"0004882813", "0002441406", "0001220703", "0000610352", "0000305176", "0000000000" };
	
	for (int i = 0; i < 10; i++)
		fracstr[i] = 0;
	for (int i = frac_bits; i; i--)
	{
		if ( value & (1 << (i - 1) ) )
		{
			int carry = 0;
			int p = frac_bits - i > 15 ? 15 : frac_bits - i;
			for (int j = 10; j; j--)
			{
				
				fracstr[j - 1] += bins[p][j - 1] - '0' + carry;
				carry = 0;
				if (fracstr[j - 1] > 9)
				{
					fracstr[j - 1] -= 10;
					carry = 1;
				}
			}
			intv += carry;
		}
	}
	for (int i = 10, last_zero = 1; i; i--)
	{
		if (last_zero && i != 1)
		{
			if (fracstr[i - 1] != 0)	last_zero = 0;
			else						continue;
		}
		fracstr[i - 1] += '0';
	}
	fracstr[10] = 0;

	writen = snprintf(buffer, bufcnt, "%d.%s", intv, fracstr);
	return 0;
}

std::string fixed_to_string(const int value, const uint8_t frac_bits)
{
	char buffer[255];
	fixed_to_str(buffer, 255, value, frac_bits);
	return std::string(buffer);
}
int main()
{

	float fl_val = 0.;//std::numeric_limits<float>::max();
	fl_val = -0. / fl_val;
	MyUnion m;
	m.fval = fl_val;

	std::cout << "float:\t" << fl_val;

	
	
	std::cout << "\nint\t" << m.ival;

	std::cout << "\nsign:\t" << get_signf(fl_val);
	std::cout << "\nq:\t" << get_qf(fl_val) << "\tp = q-f: " << get_qf(fl_val) - 127;
	std::cout << "\nm:\t" << get_mf(fl_val);
	std::cout << "\ninf: " << get_inf(fl_val);
	std::cout << "\nnan: " << is_nan(fl_val);

	std::cout << std::endl;
	int a5 = str_to_fixed("-5.7", 4);
	int a4 = str_to_fixed("5.7", 4);
	int b4 = str_to_fixed("1.3", 4);
	std::cout << "\nfixed a4: " << a4 << "\tfixed b4: " << b4 << "\tfixed a4+b4: " << a4 + b4;
	std::cout << "\tfixed a4*b4: " << ((a4 << 1) * (b4 << 1) >> 2);

	char strbuff[256];
	fixed_to_str(strbuff, 256, b4, 4);
	std::cout << "\n\nfrom fixed: " << strbuff;

	{
		int32_t a = 0x1000L;    // q15: a = 0.125
		int32_t b = 0x20000L;   // q20: b = 0.125
		int32_t c = 0;          // q25
		c = (a << 5) + b;       // q20: (a * 2 ^ (20 - 15) + b); c = 0x40000L (0.25 в q20)
		c <<= 5;

		std::cout << "\n\nexample\n" << "a: " << a << "=" << fixed_to_string(a, 15);
		std::cout << "\nb: " << b << "=" << fixed_to_string(b, 20);
		std::cout << "\nc=a+b=" << c << "=" << fixed_to_string(c, 25);
	}

	{
		int32_t a = 0x8000L;    // q16: a = 0.5
		int32_t b = 0x100000L;  // q21: b = 0.5
		int32_t c = 0xC0000L;   // q20: c = 0.75
		int64_t d;              // Временная переменная с увеличенным числом разрядов, чтобы хватило на результат.
		d = (int64_t)a * (int64_t)b; // q37 = q16 * q21; d = 0x800000000L (0.25 in q37)
		d >>= 17;               // q37 / 2 ^ 17 = q20
		c += (int32_t)d;        // q20: c = 0x100000 (1 in q20)
		std::cout << "\n\nexample\n" << "a: " << a << "=" << fixed_to_string(a, 16);
		std::cout << "\nb: " << b << "=" << fixed_to_string(b, 21);
		std::cout << "\nd=a*b=" << d << "=" << fixed_to_string(d, 20);
	}
	{
			//a / b = n1 * 2** - q1 / (n2 * 2 - q2) = n1 / n2 * 2 - (q1 — q2).
			/*Сомножитель 2 - (q1 — q2) означает, что при выполнении деления экспонента автоматически уменьшается.Если не принять меры, часть значащих разрядов отбрасывается автоматически.
			Способ коррекции очевиден — необходимо заранее увеличить разрядность делителя настолько, чтобы в результате деления получить желаемое количество значащих бит :
		a / b = n1 * 2 - q1 * 2q3 / (n2 * 2 - q2) = n1 / n2 * 2 - (q1 — q2 + q3).
			Таким образом, экспонента частного увеличена на q3 разряда.
			*/
		int32_t a = 0x4000L;    // q15: a = 0.5
		int32_t b = 0x80000L;  // q20: b = 0.5
		int32_t c = 0;          // q25
		int64_t d;              // Временная переменная с увеличенным числом разрядов.
		d = (int64_t)a << 30;  // q45: d = 0x200000000000; (0.5 in q45)
		c = (int32_t)(d / (int64_t)b);  // q25: c = 0x2000000; (1 in q25)

		/*Очевидно, что при превышении числом разрядности 32 бита, проблему уже не решить так просто.Тем не менее, для простых инженерных расчетов 32 - битных чисел обычно более, чем достаточно.
			Есть один простой способ значительно сократить потерю точности при делении — предварительное нормирование делимого.Нормирование — фактически максимальный сдвиг мантиссы влево, при котором не происходит отбрасывания значащих битов.Определить, на сколько можно сдвинуть число, можно путем подсчета ведущих нулей в делимом, для чего существуют специальные алгоритмы(или даже аппаратные инструкции процессора).
			После деления частное следует сдвинуть вправо на такое же количество бит для восстановления экспоненты.
			Вышеприведенный фрагмент кода при этом может выглядеть таким образом :
			*/
#if 0
		int32_t a = 0x4000L;    // q15: a = 0.5
		int32_t b = 0x80000L;  // q20: b = 0.5
		int32_t c = 0;          // q25
		int norm_shift = norm(a); // Вычисление нормирующего сдвига. norm_shift = 16
		c = ((a << norm_shift) / b); // q(-5): c = 0x800 (1*2^norm in q(-5))
		c <<= (30 - norm);      // q25: c = 0x2000000; (1 in q25)
#endif
	}
	return 0; 
}

