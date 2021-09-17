use crate::fixed::Fixed;
use crate::date::Date;

use crate::storage;
use crate::storage::{SerialiseError};

use crate::split_transaction::SplitTransaction;

use byteorder::{ReadBytesExt, WriteBytesExt};

use std::fmt;

#[derive(Clone, Copy, Debug)]
#[allow(dead_code)]
#[repr(u8)]
pub enum Type {
    None = 0,
    Deposit = 1,
    Withdrawal = 2,
    Transfer = 3,
    StandingOrder = 4,
    DirectDebit = 5,
    PointOfSale = 6,
    Charge = 7,
    ATM = 8,
    Cheque = 9,
    Credit = 10,
    Debit = 11
}

#[derive(Clone, Debug)]
pub struct Transaction {
    splits:             Vec<SplitTransaction>,

    category:           String,
    pub description:    String,
    pub payee:          String,

    pub amount:         Fixed,
    pub date:           Date,

    type_:              Type,

    pub cleared:        bool,
    flagged:            bool, // Not used...
    reconciled:         bool,

    has_fit_id:         bool,

    split:              bool
}

impl Default for Transaction {
    fn default () -> Transaction {
        Transaction{splits: Vec::with_capacity(0), category: "".to_string(), description: "".to_string(), payee: "".to_string(),
                    amount: Fixed::from(0.0),
                    date: Date::default(),
                    type_: Type::None,
                    cleared: false, flagged: false, reconciled: false, has_fit_id: false, split: false}
    }
}

impl fmt::Display for Transaction {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Date: {}, Payee: {}, Category: {}, Amount: {}", self.date, self.payee, self.category, self.amount)
    }
}

impl Transaction {
    pub fn new(description: &str, payee: &str, category: &str, amount: Fixed, date: Date) -> Transaction {
        let mut new_transaction = Transaction::default();

        new_transaction.description = description.to_string();
        new_transaction.payee = payee.to_string();
        new_transaction.category = category.to_string();
        new_transaction.amount = amount;
        new_transaction.date = date;

        return new_transaction;
    }

    pub fn load(&mut self, mut file: &std::fs::File, file_version: u8) -> Result<(), SerialiseError> {
        self.date.load(&file)?;
        self.description = storage::read_cstring(&file)?;
        self.payee = storage::read_cstring(&file)?;
        self.category = storage::read_cstring(&file)?;
        self.amount.load(&file)?;

        let type_value = file.read_u8()?;
        self.type_ = unsafe { ::std::mem::transmute(type_value) };

        let bitset_value = file.read_u8()?;

        // BitSet crate is apparently no longer maintained, so might as well do it ourselves...
        self.cleared = bitset_value & (1 << 0) != 0;
        self.split = bitset_value & (1 << 1) != 0;

        // old document versions always saved the number of split transactions,
        // even if there weren't any...
        if file_version < 6 || self.split {
            let num_splits = file.read_u8()?;

            for _i in 0..num_splits {
                let mut split_trans = SplitTransaction::default();
                split_trans.load(&file, file_version)?;

                self.splits.push(split_trans);
            }
        }

        Ok(())
    }

    pub fn store(&self, mut file: &std::fs::File) -> Result<(), SerialiseError> {
        self.date.store(&file)?;

        storage::write_cstring(file, &self.description)?;
        storage::write_cstring(file, &self.payee)?;
        storage::write_cstring(file, &self.category)?;

        self.amount.store(&file)?;

        let type_value = self.type_ as u8;
        file.write_u8(type_value)?;

        let mut bitset_value = 0u8;
        if self.cleared {
            bitset_value |= 1 << 0;
        }
        if self.split {
            bitset_value |= 1 << 1;
        }

        file.write_u8(bitset_value)?;

        if self.split {
            let num_splits = self.splits.len() as u8;
            file.write_u8(num_splits)?;

            for split in &self.splits {
                split.store(&file)?;
            }
        }

        Ok(())
    }
}
