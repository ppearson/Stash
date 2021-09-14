use std::fmt;

use crate::transaction::Transaction;
use crate::fixed::Fixed;
use crate::storage;
use crate::storage::{SerialiseError};

use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};

#[derive(Clone,Copy, Debug)]
#[allow(dead_code)]
pub enum Type {
    Cash,
    Checking,
    Savings,
    CreditCard,
    Investment,
    Asset,
    Liability,
    Other,
}

#[derive(Clone, Debug)]
pub struct Account {
pub name:               String,
    institution:        String,
    number:             String,
    note:               String,

    mtype:              Type,

pub transactions:       Vec<Transaction>,
}

impl Default for Account {
    fn default () -> Account {
        Account{name: "".to_string(), institution: "".to_string(), number: "".to_string(), note: "".to_string(),
                    mtype: Type::Cash, transactions: Vec::with_capacity(0)}
    }
}

impl fmt::Display for Account {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Name: {}, Institution: {}, Number: {}, Transaction Count: {}", self.name, self.institution, self.number,
                    self.transactions.len())
    }
}

impl Account {
    pub fn create(name: String, institution: String, number: String, mtype: Type) -> Account {
        let mut new_account = Account::default();

        new_account.name = name.clone();
        new_account.institution = institution.clone();
        new_account.number = number.clone();
        new_account.mtype = mtype;

        return new_account;
    }

    pub fn load(&mut self, mut file: &std::fs::File, file_version: u8) -> Result<(), SerialiseError> {
        self.name = storage::read_cstring(&file)?;
        self.institution = storage::read_cstring(&file)?;
        self.number = storage::read_cstring(&file)?;
        self.note = storage::read_cstring(&file)?;

        let type_value = file.read_u8()?;
        self.mtype = unsafe { ::std::mem::transmute(type_value) };

        let num_transactions = file.read_u32::<LittleEndian>()?;

        self.transactions.clear();
        self.transactions.reserve(num_transactions as usize);

        for _i in 0..num_transactions {
            let mut transaction = Transaction::default();
            transaction.load(&file, file_version)?;

            self.transactions.push(transaction);
        }

        Ok(())
    }

    pub fn store(&self, mut file: &std::fs::File) -> Result<(), SerialiseError> {
        storage::write_cstring(file, &self.name)?;
        storage::write_cstring(file, &self.institution)?;
        storage::write_cstring(file, &self.number)?;
        storage::write_cstring(file, &self.note)?;

        let type_value = self.mtype as u8;
        file.write_u8(type_value)?;

        file.write_u32::<LittleEndian>(self.transactions.len() as u32)?;

        for trans in &self.transactions {
            trans.store(file)?;
        }

        Ok(())
    }

    pub fn get_balance(&self, cleared_only: bool) -> Fixed {
        let mut final_balance = Fixed::default();

        if cleared_only {
            for trans in &self.transactions {
                if trans.cleared {
                    final_balance += trans.amount;
                }
            }
        }
        else {
            for trans in &self.transactions {
                final_balance += trans.amount;
            }
        }


        return final_balance;
    }
}