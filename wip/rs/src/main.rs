
mod fixed;
mod date;
mod transaction;
mod account;
mod document;
mod storage;
mod split_transaction;
mod scheduled_transaction;
mod graph;

use std::env;

use fixed::Fixed;
use date::Date;
use transaction::Transaction;
use account::Account;
use document::Document;


fn list_accounts(document: &Document, show_balance: bool) {
    let max_account_length = document.accounts.iter().clone().map(|a| a.name.len()).max().unwrap();
    for (index, acc) in document.accounts.iter().enumerate() {
        if show_balance {
            println!("{}: {:mal$}   Bal: {}", index, acc.name, acc.get_balance(true).format(),
                                                mal = max_account_length);
        }
        else {
            println!("{}: {}", index, acc);
        }
    }

    if show_balance {
        let total_balance = document.get_balance(true).format();
        println!("Total balance: {}", total_balance);
    }
}

fn list_transactions(account: &Account, transaction_limit: i32) {
    let iter = if transaction_limit <= 0 {
        // without using something like the 'either' crate, we can't easily
        // conditionally initialise the iterator, as the types are different,
        // so do things a bit differently such that we always use the same
        // iterator type...
        account.transactions.iter().rev().take(account.transactions.len()).rev()
    }
    else {
        account.transactions.iter().rev().take(transaction_limit as usize).rev()
    };

    let max_payee_length = iter.clone().map(|t| t.payee.len()).max().unwrap();
    let max_desc_length = iter.clone().map(|t| t.description.len()).max().unwrap();

    for trans in iter {
        println!("{}  {:mpl$} {:mdl$} {}", trans.date, trans.payee, trans.description, trans.amount,
                                        mpl = max_payee_length, mdl = max_desc_length);
    }
}

fn list_transactions_with_balance(account: &Account, transaction_limit: i32) {
    let mut balance = Fixed::from(0.0);
    if transaction_limit > 0 {
        let initial_transactions = account.transactions.len() - transaction_limit as usize;
        // TODO: can we use sum() here?
        balance = account.transactions.iter().take(initial_transactions)
                                                   .fold(Fixed::from(0.0), |mut a, b| {
                                                    a += b.amount;
                                                    a
                                                });
    }
    let iter = if transaction_limit <= 0 {
        // without using something like the 'either' crate, we can't easily
        // conditionally initialise the iterator, as the types are different,
        // so do things a bit differently such that we always use the same
        // iterator type...
        account.transactions.iter().rev().take(account.transactions.len()).rev()
    }
    else {
        account.transactions.iter().rev().take(transaction_limit as usize).rev()
    };

    let max_payee_length = iter.clone().map(|t| t.payee.len()).max().unwrap();
    let max_desc_length = iter.clone().map(|t| t.description.len()).max().unwrap();
    let max_amount_length = iter.clone().map(|t| t.amount.num_full_digits()).max().unwrap() as usize;

    for trans in iter {
        balance += trans.amount;
        let str_amount = trans.amount.to_string();
        println!("{}  {:mpl$} {:mdl$} {:mal$} {}", trans.date, trans.payee, trans.description, str_amount,
                                        balance.format(),
                                        mpl = max_payee_length, mdl = max_desc_length, mal = max_amount_length);
    }
}

fn resave_document(document: &Document, save_filename: &String) -> bool {
    match document.store(save_filename) {
        Ok(()) => return true,
        Err(err) => {
            eprintln!("Error saving document: {}", err.to_string());
            return false;
        }
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() == 1 {
        let value: Fixed = Fixed::from(42.04);

        let final_val = value + Fixed::from(10.0);

        println!("Final val: {}", final_val);

        let date: Date = Date::create_from_components(5, 10, 2014);
        println!("Date: {}", date);

        let transaction: Transaction = Transaction::create("Food".to_string(), "Waitrose".to_string(), "Food".to_string(),
                                    Fixed::from(-42.0), Date::create_from_components(22, 04, 2018));
        println!("Transaction: {}", transaction);

        let account: Account = Account::create("Main".to_string(), "Barclays".to_string(), "012345".to_string(), account::Type::Cash);
        println!("Account: {}", account);

        return;
    }

    let path_arg = &args[1];

    let mut document = Document::create();
    let res = document.load(path_arg);
    if res.is_err() {
        eprintln!("Couldn't open document... {}", res.unwrap_err());
        return;
    }

    let command = &args[2];
    if command.eq("lista") {
        list_accounts(&document, false);
    }
    else if command.eq("listb") {
        list_accounts(&document, true);
    }
    else if command.eq("listt") {
        let mut account_index = 0;
        let mut transaction_limit = 30;
        if args.len() > 3 {
            account_index = args[3].parse::<i32>().unwrap();
            if args.len() > 4 {
                transaction_limit = args[4].parse::<i32>().unwrap();
            }
        }
        let account = &document.accounts[account_index as usize];
        list_transactions(&account, transaction_limit);
 //       list_transactions_with_balance(&account, transaction_limit);
    }
    else if command.eq("listtb") {
        let mut account_index = 0;
        let mut transaction_limit = 30;
        if args.len() > 3 {
            account_index = args[3].parse::<i32>().unwrap();
            if args.len() > 4 {
                transaction_limit = args[4].parse::<i32>().unwrap();
            }
        }
        let account = &document.accounts[account_index as usize];
        list_transactions_with_balance(&account, transaction_limit);
    }
    else if command.eq("resave") {
        if args.len() > 3 {
            let save_filename = &args[3];

            resave_document(&document, save_filename);
        }
    }
}

