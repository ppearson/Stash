use std::fmt;
use std::collections::BTreeSet;

use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};

use crate::account::Account;

use crate::storage;
use crate::storage::{SerialiseError};
use crate::fixed::Fixed;
use crate::scheduled_transaction::ScheduledTransaction;
use crate::graph::Graph;

#[allow(non_upper_case_globals)]
const kDOCUMENT_VERSION: u8 = 6;

#[derive(Clone, Debug)]
pub struct Document {

pub accounts:           Vec<Account>,

    payees:             BTreeSet<String>,
    categories:         BTreeSet<String>,

pub scheduled_trans:    Vec<ScheduledTransaction>,

    graphs:             Vec<Graph>

    // TODO:
}

impl Default for Document {
    fn default () -> Document {
        Document{accounts: Vec::with_capacity(0), payees: BTreeSet::new(), categories: BTreeSet::new(),
                scheduled_trans: Vec::with_capacity(0), graphs: Vec::with_capacity(0)}
    }
}

impl fmt::Display for Document {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Account Count: {}", self.accounts.len())
    }
}

impl Document {
    pub fn create() -> Document {
        let new_document = Document::default();
        return new_document;
    }

    pub fn load(&mut self, filename: &str) -> Result<(), SerialiseError> {
        let mut file = match std::fs::File::open(filename) {
            Ok(file) => file,
            Err(err) => return Err(SerialiseError::StdError(format!("Couldn't open file: '{}' - ", filename.to_string())
                                                             + &err.to_string())),
        };

        let file_id = file.read_u8()?;
        if file_id != 42 {
            // it's not a valid Stash file
            return Err(SerialiseError::CustomError("Unrecognised file format.".to_string()));
        }

        let file_version = file.read_u8()?;
        if file_version > kDOCUMENT_VERSION {
            // it's a document version from the future we don't know about...
            return Err(SerialiseError::CustomError("Future file format version.".to_string()));
        }

        self.accounts.clear();
        let account_count = file.read_u32::<LittleEndian>()?;

        for _i in 0..account_count {
            let mut temp_account = Account::default();
            temp_account.load(&file, file_version)?;

            self.accounts.push(temp_account);
        }

        self.scheduled_trans.clear();
        let scheduled_transaction_count = file.read_u32::<LittleEndian>()?;

        for _i in 0..scheduled_transaction_count {
            let mut temp_sched_trans = ScheduledTransaction::default();
            temp_sched_trans.load(&file, file_version)?;

            self.scheduled_trans.push(temp_sched_trans);
        }

        self.payees.clear();
        let payee_count = file.read_u32::<LittleEndian>()?;

        for _i in 0..payee_count {
            let string_payee = storage::read_cstring(&file)?;
            self.payees.insert(string_payee);
        }

        self.categories.clear();
        let category_count = file.read_u32::<LittleEndian>()?;

        for _i in 0..category_count {
            let string_category = storage::read_cstring(&file)?;
            self.categories.insert(string_category);
        }

        self.graphs.clear();
        if file_version > 1 {
            let graph_count = file.read_u32::<LittleEndian>()?;

            for _i in 0..graph_count {
                let mut new_graph = Graph::default();
                new_graph.load(&file, file_version)?;

                self.graphs.push(new_graph);
            }
        }

        Ok(())
    }

    pub fn store(&self, filename: &str) -> Result<(), SerialiseError> {
        let mut file = match std::fs::File::create(filename) {
            Ok(file) => file,
            Err(err) => return Err(SerialiseError::StdError("Couldn't create file: ".to_string() + &err.to_string())),
        };

        let file_id = 42u8;
        file.write_u8(file_id)?;

        file.write_u8(kDOCUMENT_VERSION)?;

        let account_count = self.accounts.len() as u32;
        file.write_u32::<LittleEndian>(account_count)?;

        for acc in &self.accounts {
            acc.store(&file)?;
        }

        // sched
        file.write_u32::<LittleEndian>(self.scheduled_trans.len() as u32)?;
        for sched_trans in &self.scheduled_trans {
            sched_trans.store(&file)?;
        }

        // payees
        file.write_u32::<LittleEndian>(self.payees.len() as u32)?;
        for payee_val in &self.payees {
            storage::write_cstring(&file, payee_val)?;
        }

        // categories
        file.write_u32::<LittleEndian>(self.categories.len() as u32)?;
        for category_val in &self.categories {
            storage::write_cstring(&file, category_val)?;
        }

        // graphs
        file.write_u32::<LittleEndian>(self.graphs.len() as u32)?;

        for graph in &self.graphs {
            graph.store(&file)?;
        }

        Ok(())
    }

    pub fn get_balance(&self, cleared_only: bool) -> Fixed {
        let mut final_balance = Fixed::default();

        for acc in &self.accounts {
            final_balance += acc.get_balance(cleared_only);
        }

        return final_balance;
    }
}
