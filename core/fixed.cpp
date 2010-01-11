/* Based on decimal.h/cpp by Taj Morton.
 */

#include "fixed.h"

#include <math.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>

fixed::fixed(const double value)
{
	m_precision = 2;
	SetFromDouble(value);
}

fixed::fixed(std::string value, CurrencyFormat format) : m_precision(2)
{
	SetFromString(value, format);
}

double fixed::ToDouble( ) const
{
	if (m_positive)
		return m_num / pow(10.0, m_precision);
	else
	{
		if (m_num != 0)
			return (m_num / pow(10.0, m_precision)) * -1;
		else
			return 0.0;
	}
}

fixed fixed::operator +( const fixed & rhs ) const
{
	fixed this_copy(*this);
	add(this_copy, rhs);
	return this_copy;
}

fixed & fixed::operator +=( const fixed & rhs )
{
	add(*this, rhs);
	return *this;
}

fixed fixed::operator -( const fixed & rhs ) const
{
	fixed this_copy(*this);
	subtract(this_copy, rhs);
	
	return this_copy;
}

fixed & fixed::operator -=( const fixed & rhs )
{
	subtract(*this, rhs);
	return *this;
}

fixed fixed::operator *( const fixed & rhs ) const
{
	fixed this_copy(*this);
	multiply(this_copy, rhs);
	
	return this_copy;
}

fixed & fixed::operator *=( const fixed & rhs )
{
	multiply(*this, rhs);
	return *this;
}

fixed fixed::operator /( const fixed & rhs ) const
{
	fixed this_copy(*this);
	divide(this_copy, rhs);
	
	return this_copy;
}

fixed & fixed::operator /=( const fixed & rhs )
{
	divide(*this, rhs);
	return *this;
}

bool fixed::IsPositive() const
{
	return m_positive;
}

bool fixed::IsZero() const
{
	return m_num == 0;
}

std::string fixed::ToString() const
{
	std::stringstream stream;
	stream << m_num;
	
	std::string str = stream.str();
	
	if ((str.length() != m_precision) && ((m_num / (unsigned long int)powl(10.0, m_precision)) == 0))
	{
		// this handles the case where a fixed has no integer part (like 0.00003)
		// In this case, m_num is stored as 3, and is converted the same way
		// add the leading zeros:
		
		str.insert(0, std::string(m_precision, '0'));
	}
	
	if (str.length() - m_precision == 0)
		str.insert(0, "0."); // we want a leading 0 if there is no integer part to this fixed
	else if (m_precision != 0)
		str.insert(str.length() - m_precision, ".");
	
	if (!m_positive && (str.length() - m_precision != 0)) // don't add a "-" if we're a 0 number!
		str.insert(0, "-");
	
	return str;
}

std::ostream & operator <<( std::ostream & os, const fixed & d)
{
	os << d.ToString();
	return os;
}

std::istream& operator >> (std::istream & stream, fixed & d)
{
	double value;
	
	stream >> value;
	
	d.SetFromDouble(value);
	
	return stream;
}

fixed::operator std::string( ) const
{
	return ToString();
}

void fixed::Load(std::fstream &stream, int version)
{
	// Strictly speaking, this isn't right, as m_num is an unsigned long,
	// but as it's very unlikely that an individual transaction is going to
	// have a value of over 21,474,836.47 (what this code can cope with), it's probably
	// worth doing for efficient storage
	
	int temp = 0;
	stream.read((char *) &temp, sizeof(int));
	
	m_num = abs(temp);
	
	if (temp > 0)
		m_positive = true;
	else
		m_positive = false;
}

void fixed::Store(std::fstream &stream)
{
	// Strictly speaking, this isn't right, as m_num is an unsigned long,
	// but as it's very unlikely that an individual transaction is going to
	// have a value of over 21,474,836.47 (what this code can cope with), it's probably
	// worth doing for efficient storage
	
	int temp = m_num;
	if (!m_positive)
		temp = -temp;
	
	stream.write((char *) &temp, sizeof(int));
}

bool fixed::operator >( const fixed & rhs ) const
{
	if (m_positive && !rhs.m_positive)
		return true;
	else if (!m_positive && rhs.m_positive)
		return false;
	else
	{
		fixed this_copy(*this), rhs_copy(rhs);
		
		if (m_positive && rhs.m_positive)
			return this_copy.m_num > rhs_copy.m_num;
		else
			return this_copy.m_num < rhs_copy.m_num;
	}
}

bool fixed::operator <( const fixed & rhs ) const
{
	if (m_positive && !rhs.m_positive)
		return false;
	else if (!m_positive && rhs.m_positive)
		return true;
	else
	{
		fixed this_copy(*this), rhs_copy(rhs);
		
		if (m_positive && rhs.m_positive)
			return this_copy.m_num < rhs_copy.m_num;
		else
			return this_copy.m_num > rhs_copy.m_num;
	}
}

void fixed::SetFromDouble(const double value )
{
	double fractpart, intpart;
	
	fractpart = modf(fabs(value), &intpart);
	m_num = (long int)intpart * ( long int)pow(10.0, m_precision);
	
	// move fixed point over so now fractpart's `precision` digits
	// are on the int side
	fractpart *= pow(10.0, m_precision);
	
	// combine fracpart into (already shifted) m_intpart
	double roundCheck = modf(fractpart, &fractpart);
	m_num += (long int)fractpart;
	
	double nextDigit;
	roundCheck = modf(roundCheck * 10.0, &nextDigit);
	
//	if (m_round==Normal) {
		if ((int)nextDigit > 4)
			m_num++;
/*	}
	else if (m_round==AllUp) {
		if ((int)nextDigit!=0) {
			m_num++;
		}
	}
*/	
	if (value < 0.0)
		m_positive = false;
	else
		m_positive = true;
}

void fixed::SetFromString(std::string value, CurrencyFormat format)
{
	char delim = -1;
	if (format == UKFormat)
	{
		delim = ',';
	}
	else
	{
		delim = '.';
	}
	
	std::string::iterator it = value.begin();
	
	for (; it != value.end(); ++it)
	{
		if ((*it) == delim)
		{
			it = value.erase(it);
		}
	}
	
	double dValue = atof(value.c_str());
	SetFromDouble(dValue);
}

fixed & fixed::operator =( const double & rhs )
{
	SetFromDouble(rhs);
	return *this;
}

void fixed::add( fixed & dec, const fixed & value )
{
//	fixed value_copy(value);
	
	if (dec.m_positive == value.m_positive)
		dec.m_num += value.m_num;
	else
	{
		if (dec.m_num > value.m_num)
			dec.m_num -= value.m_num;
		else if (dec.m_num < value.m_num)
		{
			dec.m_num = value.m_num - dec.m_num;
			if (value.m_positive == true)
				dec.m_positive = true;
			else
				dec.m_positive = false;
		}
		else
		{
			dec.m_num = 0;
		}		
	}
}

void fixed::subtract( fixed & dec, const fixed & value )
{
//	fixed value_copy(value);
	
	if (dec.m_positive != value.m_positive)
	{
		if (value.m_positive && value.m_num > dec.m_num)
		{
			dec.m_positive = false;
			dec.m_num += value.m_num;
		}
		else
		{
			dec.m_num -= value.m_num;
		}
	}
	else if (!dec.m_positive && !value.m_positive)
	{
		if (dec.m_num > value.m_num)
		{
			dec.m_num -= value.m_num;
			dec.m_positive = false;
		}
		else if (dec.m_num < value.m_num)
		{
			dec.m_num = value.m_num - dec.m_num;
			dec.m_positive = true;
		}
		else
		{
			dec.m_num = 0;
		}
	}
	else
	{ // both are positive
		if (dec.m_num > value.m_num)
		{
			dec.m_num -= value.m_num;
		}
		else if (dec.m_num < value.m_num)
		{
			dec.m_num = value.m_num - dec.m_num;
			dec.m_positive = false;
		}
		else
		{
			dec.m_num = 0;
		}
	}
}

void fixed::multiply( fixed & dec, const fixed & value )
{
	dec.m_num *= value.m_num;
	
	// place the fixed point in the new position
	dec.m_precision = value.m_precision + dec.m_precision;
	
	if (dec.m_positive == value.m_positive)
		dec.m_positive = true;
	else
		dec.m_positive = false;
}

void fixed::divide( fixed & dec, const fixed & value )
{
	fixed value_copy(value);
		
	double div_result = (double)dec.m_num / (double)value_copy.m_num;
	
	double intpart, fractpart;
	fractpart = modf(div_result, &intpart);
	
	intpart *= pow(10.0, dec.m_precision); //make room for the fractpart
	fractpart *= pow(10.0, dec.m_precision); //move as much as we want over to the int side of the fixed point
	
	// combine fracpart into (already shifted) m_intpart
	dec.m_num = (long int)intpart + ( long int)fractpart;
	
	double roundCheck = modf(fractpart, &fractpart);
	
	double nextDigit;
	roundCheck = modf(roundCheck * 10.0, &nextDigit);
	
//	if (dec.m_round==Normal) {
		if ((int)nextDigit > 4)
			dec.m_num++;
//	}
/*	else if (dec.m_round==AllUp) {
		if ((int)nextDigit!=0) {
			dec.m_num++;
		}
	}
*/	
	if (dec.m_positive == value_copy.m_positive)
		dec.m_positive = true;
	else
		dec.m_positive = false;
}

bool fixed::operator ==( const fixed & rhs ) const
{
	if (m_positive != rhs.m_positive)
		return false;
	
	if (m_precision != rhs.m_precision)
	{
		fixed this_copy(*this), rhs_copy(rhs);
		
		return this_copy.m_num == rhs_copy.m_num;
	}
	else
		return m_num == rhs.m_num;
}

bool fixed::operator !=(const fixed & rhs) const
{
	if (m_positive != rhs.m_positive)
		return true;
	
	return m_num != rhs.m_num;
}
