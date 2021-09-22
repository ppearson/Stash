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

use crate::date::Date;

use crate::storage;
use crate::storage::{SerialiseError};

use std::collections::BTreeSet;

use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};

use std::fmt;

#[derive(Clone, Copy, Debug)]
#[allow(dead_code)]
pub enum Type {
    ExpenseCategories,
    ExpensePayees,
    DepositCategories,
    DepositPayees
}

#[derive(Clone, Copy, Debug)]
#[allow(dead_code)]
pub enum DateType {
    DateWeek,
    DateMonth,
    DateYear,
    DateCustom
}

#[derive(Clone, Copy, Debug)]
#[allow(dead_code)]
pub enum ItemsType {
    AllItems,
    AllItemsExceptSpecified,
    OnlySpecified
}

#[derive(Clone, Copy, Debug)]
#[allow(dead_code)]
pub enum ViewType {
    Total,
    OverTime,
    Overview
}

#[derive(Clone, Debug)]
pub struct Graph {
    name:               String,
    account_index:      u32,
    view_type:          ViewType,
    start_date:         Date,
    end_date:           Date,
    gtype:              Type,
    ignore_transfers:   bool,
    date_type:          DateType,
    items_type:         ItemsType,

    items:              BTreeSet<String>
}

impl Default for Graph {
    fn default () -> Graph {
        Graph{name: "".to_string(), account_index: 0, view_type: ViewType::Total, start_date: Date::default(), end_date: Date::default(),
                gtype: Type::ExpenseCategories, ignore_transfers: false, date_type: DateType::DateCustom, items_type: ItemsType::AllItems,
                items: BTreeSet::new()}
    }
}

impl fmt::Display for Graph {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Name: {}", self.name)
    }
}

impl Graph {
    pub fn load(&mut self, mut file: &std::fs::File, file_version: u8) -> Result<(), SerialiseError> {
        // obviously not great in general, but it's what the C++ version is doing, so...
        self.account_index = file.read_u8()? as u32;

        self.name = storage::read_cstring(&file)?;

        self.start_date.load(&file)?;
        self.end_date.load(&file)?;

        let type_value = file.read_u8()?;
        self.gtype = unsafe { ::std::mem::transmute(type_value) };

        let bitset_value = file.read_u8()?;

        // BitSet crate is apparently no longer maintained, so might as well do it ourselves...
        self.ignore_transfers = bitset_value & (1 << 0) != 0;

        if file_version > 2 {
            let date_type_value = file.read_u8()?;
            self.date_type = unsafe { ::std::mem::transmute(date_type_value) };
        }
        
        if file_version > 4 {
            let items_type_value = file.read_u8()?;
            self.items_type = unsafe { ::std::mem::transmute(items_type_value) };

            let num_items = file.read_u32::<LittleEndian>()?;
            self.items.clear();
            for _i in 0..num_items {
                let string_item = storage::read_cstring(&file)?;
                self.items.insert(string_item);
            }

            let view_type_value = file.read_u8()?;
            self.view_type = unsafe { ::std::mem::transmute(view_type_value) };
        }
        
        Ok(())
    }

    pub fn store(&self, mut file: &std::fs::File) -> Result<(), SerialiseError> {
        file.write_u8(self.account_index as u8)?;

        storage::write_cstring(&file, &self.name)?;

        self.start_date.store(&file)?;
        self.end_date.store(&file)?;

        file.write_u8(self.gtype as u8)?;

        let mut bitset = 0u8;
        if self.ignore_transfers {
            bitset |= 1 << 0;
        }
        file.write_u8(bitset)?;

        file.write_u8(self.date_type as u8)?;
        file.write_u8(self.items_type as u8)?;

        file.write_u32::<LittleEndian>(self.items.len() as u32)?;
        for item in &self.items {
            storage::write_cstring(&file, &item)?;
        }

        file.write_u8(self.view_type as u8)?;

        Ok(())
    }
}