#![allow(dead_code)]

//use std::ops;
use std::cmp;
use std::fmt;

use crate::storage::*;

use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};

const PRECISION: u32 = 2;

#[derive(Copy, Clone, Debug)]
pub struct Fixed {
    m_num:      u64,
    m_positive: bool
}

impl Default for Fixed {
    fn default () -> Fixed {
        Fixed{m_num: 0, m_positive: true}
    }
}

impl From<f64> for Fixed {
    fn from(val: f64) -> Self {
        let mut num = Fixed::default();
        num.set_from_f64(val);

        return num;
    }
}

impl fmt::Display for Fixed {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let f64_val = self.to_f64();
        write!(f, "{:.2}", f64_val)
    }
}

/*
impl Clone for Fixed {
    fn clone(&self) -> Fixed {
        Fixed new_copy { m_num : self.m_num, m_positive : self.m_positive };
    }
}
*/

impl PartialEq for Fixed {
    fn eq(&self, other: &Self) -> bool {
        self.m_positive == other.m_positive &&
        self.m_num == other.m_num
    }

    fn ne(&self, other: &Self) -> bool {
        self.m_positive != other.m_positive ||
        self.m_num != other.m_num
    }
}

impl PartialOrd for Fixed {
    fn partial_cmp(&self, other: &Self) -> Option<cmp::Ordering> {
        // TODO: can we use match?
        if self.m_positive == other.m_positive &&
           self.m_num == other.m_num {
                return Some(cmp::Ordering::Equal);
           }
        else {
            let is_less_than;
            if self.m_positive != other.m_positive {
                is_less_than = !other.m_positive && self.m_positive;
            }
            else {
                is_less_than = other.m_num < self.m_num;
            }
            
            match is_less_than {
                true => Some(cmp::Ordering::Less),
                false => Some(cmp::Ordering::Greater)
            }
        }
    }

    fn lt(&self, other: &Self) -> bool {
        if self.m_positive && !other.m_positive {
            return false;
        }
        else if !self.m_positive && other.m_positive {
            return true;
        }
        else {
            if self.m_positive && other.m_positive {
                return self.m_num < other.m_num;
            }
            else {
                return self.m_num > other.m_num;
            }
        }
    }

    fn gt(&self, other: &Self) -> bool {
        if self.m_positive && !other.m_positive {
            return true;
        }
        else if !self.m_positive && other.m_positive {
            return false;
        }
        else {
            if self.m_positive && other.m_positive {
                return self.m_num > other.m_num;
            }
            else {
                return self.m_num < other.m_num;
            }
        }
    }
}

impl std::ops::Add for Fixed {
    type Output = Fixed;

    fn add(self, other: Fixed) -> Fixed {
        let mut copy = self.clone();
        copy.add_impl(other);
        return copy;       
    }
}

impl std::ops::Sub for Fixed {
    type Output = Fixed;

    fn sub(self, other: Fixed) -> Fixed {
        let mut copy = self.clone();
        copy.subtract_impl(other);
        return copy;       
    }
}

impl std::ops::AddAssign for Fixed {
    fn add_assign(&mut self, other: Fixed) {
        self.add_impl(other);
    }
}

impl std::ops::SubAssign for Fixed {
    fn sub_assign(&mut self, other: Fixed) {
        self.subtract_impl(other);
    }
}

impl Fixed {

    fn add_impl(&mut self, val: Fixed) {
        if self.m_positive == val.m_positive {
            self.m_num += val.m_num;
        }
        else {
            if self.m_num > val.m_num {
                self.m_num -= val.m_num;
            }
            else if self.m_num < val.m_num {
                self.m_num = val.m_num - self.m_num;
                if val.m_positive == true {
                    self.m_positive = true;
                }
                else {
                    self.m_positive = false;
                }
            }
            else {
                self.m_num = 0;
            }
        }
    }

    fn subtract_impl(&mut self, val: Fixed) {
        if self.m_positive != val.m_positive {
            // TODO: there's a bug here that's triggering some of the unit tests
            if val.m_positive && val.m_num > self.m_num {
                self.m_positive = false;
                self.m_num += val.m_num;
            }
            else {
                self.m_num -= val.m_num;
            }
        }
        else if !self.m_positive && !val.m_positive {
            // TODO: there's a bug here that's triggering some of the unit tests
            if self.m_num > val.m_num {
                self.m_num -= val.m_num;
                self.m_positive = false;
            }
            else if self.m_num < val.m_num {
                self.m_positive = true;
            }
            else {
                self.m_num = 0;
            }
        }
        else {
            // both are positive
            if self.m_num > val.m_num {
                self.m_num -= val.m_num;
            }
            else if self.m_num < val.m_num {
                self.m_num = val.m_num - self.m_num;
                self.m_positive = false;
            }
            else {
                self.m_num = 0;
            }
        }
    }

    // emulate libc modf()
    fn modf(val: f64) -> (f64, f64) {
        let val_abs = val.abs();
        let fract_part = val_abs % 1.0;
        let int_part = (val_abs / 1.0).floor();

        return (fract_part, int_part);
    }

    fn set_from_f64(&mut self, val: f64) {
        let (fract_part, int_part) = Fixed::modf(val);

        self.m_num = int_part as u64 * 10u64.pow(PRECISION);

        // move Fixed point over so now fractpart's `precision` digits
	    // are on the int side
        let fract_part = fract_part * 10.0f64.powf(PRECISION as f64);

        // combine fracpart into (already shifted) intpart
        let (round_check, fract_part) = Fixed::modf(fract_part);
        self.m_num += fract_part as u64;

        let (_fract_temp, next_digit) = Fixed::modf(round_check * 10.0);
        if next_digit as u64 > 4 {
            self.m_num += 1;
        }

        self.m_positive = !(val < 0.0);
    }

    fn to_f64(self) -> f64 {
        if self.m_positive {
            return (self.m_num as f64) / 10.0f64.powf(PRECISION as f64);
        }
        else {
            if self.m_num != 0 {
                return (self.m_num as f64) / 10.0f64.powf(PRECISION as f64) * -1.0;
            }
            else {
                // TODO: do we even need this else clause?
                return 0.0;
            }
        }
    }

    fn to_f64_abs(self) -> f64 {
        return (self.m_num as f64) / 10.0f64.powf(PRECISION as f64);
    }

    fn set_positive(&mut self) {
        self.m_positive = true;
    }

    fn set_negative(&mut self) {
        self.m_positive = false;
    }

    #[allow(dead_code)]
    fn is_zero(&self) -> bool {
        return true;
    }

    fn is_positive(&self) -> bool {
        return self.m_positive;
    }

    pub fn num_digits(&self) -> u32 {
        // Note: this is different from the C++ version on purpose,
        //       as Rust doesn't support self modification (\=) returning
        //       the value.
        let mut count = 0;
        let mut temp = self.m_num;

        // Note: this is different to the C++ version, as Rust doesn't
        //       support while temp /= 10 { ...
        while temp > 0 {
            temp /= 10;
            count += 1;
        }

        // remove fractional digits
        if count > 2 {
            // it's at least one whole number (i.e. 1.00)
            count -= 2;
        }
        else if count == 2 {
            // only subtract one, so we account for leading 0
		    count -= 1;
        }

        return count;
    }

    pub fn num_full_digits(&self) -> u32 {
        let mut count = 1;
        let mut temp = self.m_num;

        while temp > 0 {
            temp /= 10;
            count += 1;
        }

        if !self.m_positive {
            count += 1;
        }

        return count;
    }

    pub fn format(&self) -> String {
        let number_string = format!("{:.2}", self.to_f64());
        let mut num_base_digits = self.num_digits();

        let mut out = String::new();

        let mut add_sep_next = false;
        for chr in number_string.chars() {
            if add_sep_next {
                out.push(',');
            }
/*
            if num_base_digits > 3 {
                num_base_digits -= 1;
                add_sep_next = num_base_digits % 3 == 0;
            }
            else {
                add_sep_next = false;
            }
*/

            add_sep_next = num_base_digits > 3 && { num_base_digits -= 1; num_base_digits % 3 == 0 };

            out.push(chr);
        }
/*
        // TODO: can we improve this performance-wise? Iterate forwards?
        let a = number_string.chars().rev().enumerate();
        for (idx, val) in a {
            if val != '.' {
                if idx != 0 && idx != 3 && idx % 3 == 0 {
                    out.insert(0, ',');
                }
            }
            
            out.insert(0, val);
        }
*/
        return out;
    }

    pub fn load(&mut self, mut file: &std::fs::File) -> Result<(), SerialiseError> {
        // Strictly speaking, this isn't right, as m_num is a u64,
        // but as it's very unlikely that an individual transaction is going to
        // have a value of over 21,474,836.47 (what this code can cope with), it's probably
        // worth doing for efficient storage

        let temp = file.read_i32::<LittleEndian>()?;

        self.m_num = temp.abs() as u64;

        self.m_positive = temp > 0;

        Ok(())
    }

    pub fn store(&self, mut file: &std::fs::File) -> Result<(), SerialiseError> {
        // Strictly speaking, this isn't right, as m_num is a u64,
        // but as it's very unlikely that an individual transaction is going to
        // have a value of over 21,474,836.47 (what this code can cope with), it's probably
        // worth doing for efficient storage

        let mut temp = self.m_num as i32;
        if !self.m_positive {
            temp = -temp;
        }
        file.write_i32::<LittleEndian>(temp)?;

        Ok(())
    }

}



#[cfg(test)]
mod tests {
    use super::*;
    use assert_approx_eq::assert_approx_eq;

    #[test]
    fn modf_1() {
        let (fractional, int) = Fixed::modf(13.42);

//        relative_eq!(Fixed::modf(13.42), (0.42, 13.0));
        assert_approx_eq!(fractional, 0.42);
        assert_approx_eq!(int, 13.0);
    }

    #[test]
    fn modf_2() {
        let (fractional, int) = Fixed::modf(72.001);
        
        assert_approx_eq!(fractional, 0.001);
        assert_approx_eq!(int, 72.0);
    }

    #[test]
    fn modf_3() {
        let (fractional, int) = Fixed::modf(64.000);
        
        assert_approx_eq!(fractional, 0.000);
        assert_approx_eq!(int, 64.0);
    }


    #[test]
    fn general01() {
        let start = 42.42f64;
        let fixed_val: Fixed = Fixed::from(start);

        let final_val = fixed_val.to_f64();

        assert_approx_eq!(start, final_val);
    }

    #[test]
    fn ops_add1() {
        let fixed_val: Fixed = Fixed::from(40.42);

        let fixed_val = fixed_val + Fixed::from(10.0);

        assert_approx_eq!(fixed_val.to_f64(), 50.42);
    }

    #[test]
    fn ops_add2() {
        let fixed_val: Fixed = Fixed::from(40.42);

        let fixed_val = fixed_val + Fixed::from(-10.0);

        assert_approx_eq!(fixed_val.to_f64(), 30.42);
    }

    #[test]
    fn ops_add_assign1() {
        let mut fixed_val: Fixed = Fixed::from(40.42);

        fixed_val += Fixed::from(10.0);

        assert_approx_eq!(fixed_val.to_f64(), 50.42);
    }

    #[test]
    fn ops_add_assign2() {
        let mut fixed_val: Fixed = Fixed::from(40.42);

        fixed_val += Fixed::from(-10.0);

        assert_approx_eq!(fixed_val.to_f64(), 30.42);
    }

    #[test]
    fn ops_add_assign3() {
        let mut fixed_val: Fixed = Fixed::from(-40.42);

        fixed_val += Fixed::from(10.0);

        assert_approx_eq!(fixed_val.to_f64(), -30.42);
    }

    //

    #[test]
    fn ops_sub1() {
        let fixed_val: Fixed = Fixed::from(40.42);

        let fixed_val = fixed_val - Fixed::from(10.0);

        assert_approx_eq!(fixed_val.to_f64(), 30.42);
    }

    #[test]
    fn ops_sub2() {
        let fixed_val: Fixed = Fixed::from(-40.42);

        // TODO: fix the method... (might be broken in C++ as well)
        let fixed_val = fixed_val - Fixed::from(10.0);

        assert_approx_eq!(fixed_val.to_f64(), -30.42);
    }

    #[test]
    fn ops_sub_assign1() {
        let mut fixed_val: Fixed = Fixed::from(40.42);

        fixed_val -= Fixed::from(10.0);

        assert_approx_eq!(fixed_val.to_f64(), 30.42);
    }

    #[test]
    fn ops_sub_assign2() {
        let mut fixed_val: Fixed = Fixed::from(40.42);

        // TODO: fix this - might be broken in C++ as well...
        fixed_val -= Fixed::from(-10.0);

        assert_approx_eq!(fixed_val.to_f64(), 30.42);
    }

    #[test]
    fn ops_sub_assign3() {
        let mut fixed_val: Fixed = Fixed::from(-40.42);

        fixed_val -= Fixed::from(10.0);

        assert_approx_eq!(fixed_val.to_f64(), -30.42);
    }


    // comparisons
    #[test]
    fn cmp_equals1() {
        let fixed_val = Fixed::from(42.01);

        assert_eq!(fixed_val, Fixed::from(42.01));
    }

    #[test]
    fn cmp_equals2() {
        let fixed_val = Fixed::from(-42.01);

        assert_eq!(fixed_val, Fixed::from(-42.01));
    }

    #[test]
    fn cmp_equals3() {
        let fixed_val = Fixed::from(-42.01);

        assert_ne!(fixed_val, Fixed::from(42.01));
    }

    //

    #[test]
    fn cmp_lessthan1() {
        let fixed_val = Fixed::from(-42.01);
        assert_eq!(fixed_val < Fixed::from(42.01), true);
    }

    #[test]
    fn cmp_lessthan2() {
        let fixed_val = Fixed::from(42.01);
        assert_eq!(fixed_val < Fixed::from(43.01), true);
    }

    #[test]
    fn cmp_greaterthan1() {
        let fixed_val = Fixed::from(-42.01);
        assert_eq!(fixed_val > Fixed::from(42.01), false);
    }

    #[test]
    fn cmp_greaterthan2() {
        let fixed_val = Fixed::from(42.01);
        assert_eq!(fixed_val > Fixed::from(43.01), false);
    }

}
