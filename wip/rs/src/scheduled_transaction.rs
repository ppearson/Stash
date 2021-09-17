use crate::fixed::Fixed;
use crate::date::Date;
use crate::transaction;

use crate::storage;
use crate::storage::{SerialiseError};

use byteorder::{ReadBytesExt, WriteBytesExt};

use std::fmt;

#[derive(Clone, Copy, Debug)]
#[allow(dead_code)]
pub enum Frequency {
    Weekly,
    TwoWeeks,
    FourWeeks,
    Monthly,
    TwoMonths,
    Quarterly,
    Annually,
}

#[derive(Clone, Copy, Debug)]
#[allow(dead_code)]
pub enum Constraint {
    ExactDate,
    ExactOrNextWorkingDay,
    LastWorkingDayOfMonth,
}

#[derive(Clone, Debug)]
pub struct ScheduledTransaction {
    account_index:        u32,
    enabled:              bool,
    payee:                String,
    amount:               Fixed,
    category:             String,
    description:          String,

    frequency:            Frequency,
    next_date:            Date,
    type_:                transaction::Type,

    constraint:           Constraint
}

impl Default for ScheduledTransaction {
    fn default () -> ScheduledTransaction {
        ScheduledTransaction{account_index: 0, enabled: true, description: "".to_string(), payee: "".to_string(), category: "".to_string(),
                    amount: Fixed::from(0.0),
                    frequency: Frequency::Weekly, next_date: Date::default(), type_: transaction::Type::None, constraint: Constraint::ExactDate}
    }
}

impl fmt::Display for ScheduledTransaction {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Desc: {}, Payee: {}, Category: {}, Amount: {}", self.description, self.payee, self.category, self.amount)
    }
}

impl ScheduledTransaction {

    pub fn load(&mut self, mut file: &std::fs::File, file_version: u8) -> Result<(), SerialiseError> {
        // obviously not great in general, but it's what the C++ version is doing, so...
        self.account_index = file.read_u8()? as u32;

        if file_version == 0 {
            self.enabled = true;
        }
        else {
            let bitset_value = file.read_u8()?;

            // BitSet crate is apparently no longer maintained, so might as well do it ourselves...
            self.enabled = bitset_value & (1 << 0) != 0;
        }

        self.payee = storage::read_cstring(&file)?;
        self.amount.load(&file)?;
        self.category = storage::read_cstring(&file)?;
        self.description = storage::read_cstring(&file)?;

        let frequency_value = file.read_u8()?;
        self.frequency = unsafe { ::std::mem::transmute(frequency_value) };

        self.next_date.load(&file)?;

        let transaction_type_value = file.read_u8()?;
        self.type_ = unsafe { ::std::mem::transmute(transaction_type_value) };

        let constraint_value = file.read_u8()?;
        self.constraint = unsafe { ::std::mem::transmute(constraint_value) };

        Ok(())
    }

    pub fn store(&self, mut file: &std::fs::File) -> Result<(), SerialiseError> {
        file.write_u8(self.account_index as u8)?;

        let mut bitset = 0u8;
        if self.enabled {
            bitset |= 1 << 0;
        }
        file.write_u8(bitset)?;

        storage::write_cstring(file, &self.payee)?;
        self.amount.store(&file)?;
        storage::write_cstring(file, &self.category)?;
        storage::write_cstring(file, &self.description)?;

        file.write_u8(self.frequency as u8)?;

        self.next_date.store(&file)?;

        file.write_u8(self.type_ as u8)?;
        file.write_u8(self.constraint as u8)?;

        Ok(())
    }
}