/*
 Stash:  A Personal Finance app (Rust prototype port).
 Copyright (C) 2021 Peter Pearson
 
 Licensed under the Apache License, Version 2.0 (the "License");
 You may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 http://www.apache.org/licenses/LICENSE-2.0
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ---------
 */

#![allow(dead_code)]

use chrono::{NaiveDate, Datelike, Weekday};

use std::fmt;

use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};

use crate::storage::*;

#[derive(Copy, Clone, Debug)]
pub struct Date {

    internal_date:  NaiveDate,

    year:           u16,
    month:          u8,
    day:            u8,

    day_of_week:    u8,
}

impl Default for Date {
    fn default () -> Date {
        Date{internal_date: chrono::NaiveDate::from_ymd(0, 1, 1),
             year: 0, month: 0, day: 0, day_of_week: 0}
    }
}

impl fmt::Display for Date {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{:02}/{:02}/{}", self.day, self.month, self.year)
    }
}


impl Date {
    pub fn from_components(day: u32, month: u32, year: u32) -> Date {
        let new_date = Date {internal_date: chrono::NaiveDate::from_ymd(year as i32, month, day),
                                 year: year as u16, month: month as u8, day: day as u8, day_of_week: 0};
        new_date
    }

    pub fn now(&mut self) {
        let current_date = chrono::offset::Local::now();
        self.internal_date = chrono::NaiveDate::from_ymd(current_date.year(), current_date.month(), current_date.day());
        self.set_vars_from_chrono_time();
    }

    fn set_vars_from_chrono_time(&mut self) {
        self.year = self.internal_date.year() as u16;
        self.month = self.internal_date.month() as u8;
        self.day = self.internal_date.day() as u8;
        self.day_of_week = match self.internal_date.weekday() {
            Weekday::Mon => 1,
            Weekday::Tue => 2,
            Weekday::Wed => 3,
            Weekday::Thu => 4,
            Weekday::Fri => 5,
            Weekday::Sat => 6,
            Weekday::Sun => 0,
        }
    }

    fn set_chrono_time_from_vars(&mut self) {
        self.internal_date = chrono::NaiveDate::from_ymd(self.year as i32, self.month as u32, self.day as u32);
    }

    pub fn load(&mut self, mut file: &std::fs::File) -> Result<(), SerialiseError> {

        // new one:
        // Use more portable and space-efficient method:
        
        // day is stored in left-most (most significant in little-endian) 5 bits
        // month is stored in next 4 bits
        // year in right-most (least significant in little-endian) 16 bits
        
        // day shift 27 - mask: 0xF8000000
        // month shift 23 - mask: 0x7800000
        // year shift 0 - mask: 0xFFFF

        let packed_value = file.read_u32::<LittleEndian>()?;

        let day = (packed_value & 0xF8000000) >> 27;
        let month = (packed_value & 0x7800000) >> 23;
        let year = packed_value & 0xFFFF;

        self.day = day as u8;
        self.month = month as u8;
        self.year = year as u16;

        self.set_chrono_time_from_vars();

        Ok(())
    }

    pub fn store(&self, mut file: &std::fs::File) -> Result<(), SerialiseError> {
        // new one:
        // Use more portable and space-efficient method:
        
        // day is stored in left-most (most significant in little-endian) 5 bits
        // month is stored in next 4 bits
        // year in right-most (least significant in little-endian) 16 bits

        // day shift 27 - mask: 0xF8000000
        // month shift 23 - mask: 0x7800000
        // year shift 0 - mask: 0xFFFF

        let packed_date = (self.day as u32) << 27 | (self.month as u32) << 23 | self.year as u32;
        file.write_u32::<LittleEndian>(packed_date)?;

        Ok(())
    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn date_create1() {
        let date = Date::default();

        assert_eq!(date.year, 0);
        assert_eq!(date.month, 0);
        assert_eq!(date.day, 0);
    }

    #[test]
    fn date_create2() {
        let date = Date::from_components(15, 10, 2019);

        assert_eq!(date.year, 2019);
        assert_eq!(date.month, 10);
        assert_eq!(date.day, 15);
    }

}