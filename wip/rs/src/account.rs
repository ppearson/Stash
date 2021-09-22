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

use std::fmt;

use crate::transaction::Transaction;
use crate::fixed::Fixed;
use crate::storage;
use crate::storage::{SerialiseError};

use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};

#[derive(Clone, Copy, Debug)]
#[allow(dead_code)]
#[repr(u8)]
pub enum Type {
    Cash = 0,
    Checking = 1,
    Savings = 2,
    CreditCard = 3,
    Investment = 4,
    Asset = 5,
    Liability = 6,
    Other = 7,
}

#[derive(Clone, Debug)]
pub struct Account {
pub name:               String,
    institution:        String,
    number:             String,
    note:               String,

    type_:              Type,

pub transactions:       Vec<Transaction>,
}

impl Default for Account {
    fn default () -> Account {
        Account{name: "".to_string(), institution: "".to_string(), number: "".to_string(), note: "".to_string(),
                    type_: Type::Cash, transactions: Vec::with_capacity(0)}
    }
}

impl fmt::Display for Account {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Name: {}, Institution: {}, Number: {}, Transaction Count: {}", self.name, self.institution, self.number,
                    self.transactions.len())
    }
}

impl Account {
    pub fn new(name: &str, institution: &str, number: &str, acc_type: Type) -> Account {
        let mut new_account = Account::default();

        new_account.name = name.to_string();
        new_account.institution = institution.to_string();
        new_account.number = number.to_string();
        new_account.type_ = acc_type;

        return new_account;
    }

    pub fn load(&mut self, mut file: &std::fs::File, file_version: u8) -> Result<(), SerialiseError> {
        self.name = storage::read_cstring(&file)?;
        self.institution = storage::read_cstring(&file)?;
        self.number = storage::read_cstring(&file)?;
        self.note = storage::read_cstring(&file)?;

        let type_value = file.read_u8()?;
        self.type_ = unsafe { ::std::mem::transmute(type_value) };

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

        let type_value = self.type_ as u8;
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