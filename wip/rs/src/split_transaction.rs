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

use crate::fixed::Fixed;

use crate::storage;
use crate::storage::SerialiseError;

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
    #[allow(dead_code)]
    pub fn new(description: &str, payee: &str, category: &str, amount: Fixed) -> SplitTransaction {
        SplitTransaction { description: description.to_string(),
                           payee: payee.to_string(),
                           category: category.to_string(),
                           amount }
    }

    pub fn load(&mut self, file: &std::fs::File, _file_version: u8) -> Result<(), SerialiseError> {
        self.description = storage::read_cstring(file)?;
        self.payee = storage::read_cstring(file)?;
        self.category = storage::read_cstring(file)?;
        self.amount.load(file)?;

        Ok(())
    }

    pub fn store(&self, file: &std::fs::File) -> Result<(), SerialiseError> {
        storage::write_cstring(file, &self.description)?;
        storage::write_cstring(file, &self.payee)?;
        storage::write_cstring(file, &self.category)?;
        self.amount.store(file)?;

        Ok(())
    }
}