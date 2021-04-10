Stash
-----

Stash is a fast and simple personal finance native application.

Its core data-storage and serialisation files are written in C++, and there is an
existing (but somewhat old) native Mac OS interface written in Objective-C++ / Cocoa,
as well as a much more recent, Qt interface/port which now exists (for Linux).

Features
--------

* Easy-to-use interface to view and edit transactions
* Localised Dates and Currency formatting
* Multiple Accounts
* Split Transactions
* OFX Import/Export (1.0 and 2.0 versions)
* QIF Import/Export
* Scheduled Transactions
* Pie Chart Graphs
* Area Chart Graphs over time
* Overview Chart Graphs

Licence
-------

Stash is licensed under the GPL version 2.0. For more information, see the included
Licence.txt file.

Downloading
-----------

Stash is hosted on GitHub. Its project page is here:
https://github.com/ppearson/Stash

If you want to download the source code, you can do the following:

git clone git://github.com/ppearson/Stash

Implementations
---------------

There are two full implementations of Stash now, in different subdirectories of the main checkout. Both implementations
are native wrappers around the core C++ storage files:

* mac/: the original OS X / MacOS interface wrapper, written in Objective C++ against native Cocoa bindings, which was written in 2009-2010.
* qt/: a much newer (written in 2020-2021) Qt (4/5) interface wrapper, predominantly for native Linux support.

The Qt port is now feature-complete to the original MacOS version, and it's likely any future
development I do will be against the Qt version.

There's also a *very* incomplete and WIP CLI version of Stash which is only really for testing file format
and core infrastructure experiments, as I don't really see the use in having a fully-featured command line
interface to Stash (I much prefer using a GUI), however it could be worthwhile adding export functionality
in the future to this version, however I have no need for that currently, and otherwise this CLI is very
unlikely to be worked on by me.

Helping
-------

Any help with Stash is more than welcome. As mentioned above, Stash's core is written in
C++, with the Mac Cocoa interface written in Objective-C++, and a newer Qt interface written
in C++ (for Linux). The list of features still to be implemented in Stash that I hope to get to eventually is:

* Encryption of data files
* Import Rules
* CSV import/export
* Reports
* Budgets
* Multiple currencies

