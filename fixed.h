/* Fixed point decimal class datatype
 */

/* Based on decimal.h/cpp by Taj Morton.
 */

#ifndef FIXED_H
#define FIXED_H

#include <string>

class fixed
{
public:
	fixed(const double value = 0.00);
	~fixed() { };
	
	fixed & operator=(const double & rhs);
	
	fixed operator+(const fixed & rhs) const;
	fixed & operator+=(const fixed & rhs);
	
	fixed operator-(const fixed & rhs) const;
	fixed & operator-=(const fixed & rhs);
	
	fixed operator*(const fixed & rhs) const;
	fixed & operator*=(const fixed & rhs);
	
	fixed operator/(const fixed & rhs) const;
	fixed & operator/=(const fixed & rhs);
	
	bool operator>(const fixed & rhs) const;	
	bool operator<(const fixed & rhs) const;	
	bool operator==(const fixed & rhs) const;
	bool operator!=(const fixed & rhs) const;
	
	bool IsPositive() const;
	
	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream);
	
	friend std::ostream& operator<< (std::ostream & os, const fixed & d);
	friend std::istream& operator>> (std::istream & stream, fixed & d);
	
	double ToDouble() const;
	operator std::string() const;
	
private:
	bool m_positive;
	
	unsigned short int m_precision;
	unsigned long int m_num;
	
	std::string ToString() const;
	
	void SetFromDouble(const double value = 0.00);
	
	static void add(fixed & dec, const fixed & value);
	static void subtract(fixed & dec, const fixed & value);
	static void multiply(fixed & dec, const fixed & value);
	static void divide(fixed & dec, const fixed & value);
};

#endif