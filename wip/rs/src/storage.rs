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

use std::io::prelude::*;
use std::io;
use std::cmp;
use std::fmt;

use byteorder::{ReadBytesExt, WriteBytesExt};

#[derive(Debug)]
pub enum SerialiseError {
    CustomError(String),
    StdError(String),
    IOError(io::Error),
}

impl fmt::Display for SerialiseError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match *self {
            SerialiseError::CustomError(ref err) => write!(f, "Custom error: {}", err),
            SerialiseError::StdError(ref err) => write!(f, "Std error: {}", err),
            SerialiseError::IOError(ref err) => write!(f, "IO error: {}", err),
        }
    }
}

impl From<io::Error> for SerialiseError {
    fn from(error: io::Error) -> Self {
        SerialiseError::IOError(error)
    }
}

impl From<std::str::Utf8Error> for SerialiseError {
    fn from(error: std::str::Utf8Error) -> Self {
        SerialiseError::StdError(error.to_string())
    }
}

pub fn read_cstring(mut file: &std::fs::File) -> Result<String, SerialiseError> {
    let length = file.read_u8()?;
    let mut buf = [0u8; 255];

    let mut handle = file.take(length as u64);
    
    let read_size = handle.read(&mut buf);
    if read_size? != length as usize {
        return Err(SerialiseError::StdError("Error reading string".to_string()));
    }
    
    let final_string = std::str::from_utf8(&buf[..length as usize])?;

    return Ok(final_string.to_string());
}

pub fn write_cstring(mut file: &std::fs::File, string_val: &str) -> Result<(), SerialiseError> {
    // we clamp strings to 255 'chars'...
    // TODO: check for multi-byte chars first?!
    let length = cmp::min(255, string_val.len());

    // get slice to clamped portion
    let string_limit = &string_val[..length];
    let string_bytes = string_limit.as_bytes();

    file.write_u8(length as u8)?;

    file.write_all(string_bytes)?;

    Ok(())
}