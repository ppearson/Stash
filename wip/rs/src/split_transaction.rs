use crate::fixed::Fixed;

use crate::storage;
use crate::storage::{SerialiseError};

use std::fmt;

#[derive(Clone, Debug)]
pub struct SplitTransaction {
    description:        String,
    payee:              String,
    category:           String,

    amount:             Fixed,
}

impl Default for SplitTransaction {
    fn default () -> SplitTransaction {
        SplitTransaction{description: "".to_string(), payee: "".to_string(), category: "".to_string(),
                    amount: Fixed::from(0.0)}
    }
}

impl fmt::Display for SplitTransaction {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Desc: {}, Payee: {}, Category: {}, Amount: {}", self.description, self.payee, self.category, self.amount)
    }
}

impl SplitTransaction {
    pub fn create(description: String, payee: String, category: String, amount: Fixed) -> SplitTransaction {
        let mut new_split_transaction = SplitTransaction::default();

        new_split_transaction.description = description.clone();
        new_split_transaction.payee = payee.clone();
        new_split_transaction.category = category.clone();
        new_split_transaction.amount = amount;

        return new_split_transaction;
    }

    pub fn load(&mut self, file: &std::fs::File, _file_version: u8) -> Result<(), SerialiseError> {
        self.description = storage::read_cstring(&file)?;
        self.payee = storage::read_cstring(&file)?;
        self.category = storage::read_cstring(&file)?;
        self.amount.load(&file)?;

        Ok(())
    }

    pub fn store(&self, file: &std::fs::File) -> Result<(), SerialiseError> {
        storage::write_cstring(file, &self.description)?;
        storage::write_cstring(file, &self.payee)?;
        storage::write_cstring(file, &self.category)?;
        self.amount.store(&file)?;

        Ok(())
    }
}