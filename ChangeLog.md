Version 1.0
-----------

* Qt port exists (for Linux), which is now feature-complete with older MacOS version
* Code refactoring to core storage files to improve consistency and portability/maintenance

Version 0.9
-----------

* Breaking file format change: older versions of Stash will not be able to open documents
  created with this new version.
* Dates are now stored in Stash's file format in a more compact and portable way
* Number of Split Transactions are only stored in a Transaction record if there are split
  transactions for that Transaction
* Dates and currencies are now displayed with a monospaced font, so numbers line up

Version 0.85
------------

* Updated build infrastructure to build on modern MacOS versions.
* Fixed several compile issues building on modern MacOS versions.
* Added new Transactions preference "New transactions are marked 'cleared' by default"
  which allows control over the default state of the cleared flag for new transactions.

Version 0.84
------------

* Updated code to build against OS X SDK for 10.6 and fixed a few compiler warnings
* Fixed bug which meant that creating a new document didn't clear the transactions view
* Fixed bug which meant that creating a new transaction after creating a new account in a
  new document wasn't possible without selecting the new account manually first

Version 0.83
------------

* A dot is now shown in the red title bar circle when the document has unsaved changes
* After importing QIF transactions, the document is now marked as unsaved
* Closing Stash's window now quits Stash
* Added Preference to enforce negative amounts for certain Transaction types
* Added Preference to enforce negative amounts for Split Transactions if the parent Transaction
  has a negative amount

Version 0.82
------------

* Fixed bug which ignored Cleared flag of transactions when importing QIF files
* Scheduled Transactions are now added to the Account in Cleared state
* Added right-click Add Selected Item menu to the Pie Chart graph
* Splitting an already split transaction now opens up the transaction and selects the last
  item ready for editing
* Date labels for Overview Chart are now drawn centred
* Changed the way Currency number interpretation is done to try to alleviate problems encountered
  using different input locales
* Improve compatibility with OS X 10.5 for certain machines

Version 0.81
------------

* Fixed bug which ignored option to set all imported transactions as cleared when importing OFX files
* Index Bar account amounts now have a drop shadow
* Area Chart X-axis labels are now drawn centred
* When clicking on a blank split transaction, the keyboard focus is automatically set to the Payee
  text field
* Tweaks to Pie Chart segment shading

Version 0.8
-----------

* Added Overview Chart type for graphs - basically shows a simple profit/loss blue/red bar pair
  for each month
* New Date Picker dropdown button icon
* Added simple (add and subtract only) maths expression parser to amount field - initial '=' symbol
  causes expression to be parsed
* Localised column names are now shown in the main transaction view column selector menu
* When the Scheduled Transaction window appears, the first item is now automatically selected
* Added Account Balances to the IndexBar on the left
* Fixed bug which meant that the Transfer window didn't always have an up-to-date list of Accounts
* Code optimisation for potential speed improvements
* Fixed memory leaks

Version 0.75
------------

* Italian localisation kindly done by Giovanni Medici
* Added ability to control which items are drawn in graphs
* Graph setting changes now take effect instantly
* Graph display type tab is now saved in the document
* Fixed issue with duplicate "Other" category for Area Charts
* Reconciled is now called Cleared
* Fixed bug which didn't re-enable Pie Chart viewing period segment control end buttons in
  certain situations
* Fixed bug which prevented keyboard focus from having effect after using the DatePickerDropdown
  window to pick a new date
* Tweaks to the sizing/positioning of the Area Chart axis/labels
* Dates on the x-axis of the Area Charts are now localised
* Can now manually add new Payees and Categories from their respective screens
* Fixed bug which meant that the balance of the new transaction item created was wrong if Make Transaction
  was used with the To account being the currently selected account
* New cleared checkmark images kindly done by Aleksandar Kovač
* Fixed bug which meant that split transactions could not be properly added to a transaction
  which had just been moved up or down

Version 0.7
-----------

* Added OFX Import and Export support
* Fixed bug which wouldn't refresh transactions view when changing viewed account if newly selected
  account had no transactions
* Added Transaction Types Check, Credit, Debit
* Changed all date-picker controllers to smaller textual steppers, and added DatePickerDropdown buttons
  next to them all
* Added new option to Make Transfer window, allowing generated transaction items to be automatically
  set as reconciled
* Added new Scheduled Transaction constraint: Last Working day of Month
* Added preference to control how selected Pie Chart segments are drawn (selected or popped out)

Version 0.65
------------

* Fixed issue which meant that sometimes two adjacent Pie Chart slices were the same colour
* When adding a new Graph, the new item in the IndexBar automatically starts editing
* Due Scheduled Transactions window now shows Description of Scheduled transactions
* Fixed bug which didn't set the document's status as modified if a due Scheduled Transaction
  had been skipped
* Added support for toggling Transaction View table columns (right-click the header)
* Added optional Transaction View table column "Type"
* Transaction View table now remembers the order and visibility of columns between sessions
* Fixed issue entering negative currency amounts with a "-" prefix for US locale settings

Version 0.6
-----------

* New main application icons kindly created by Aleksandar Kovač
* Speed improvements - opening a document is now around 7x faster and the analysis of data
  in order to show a graph is now 17x faster
* Added a Make Transfer feature, which allows reciprocal Transactions to be easily created for doing a
  transfer from one account to another
* Added the ability to have constraints for Scheduled Transactions. Currently the only one available is
  to force the next calculated date to be moved to the following Monday if it would fall on a weekend
* Fixed bug which would show any due Scheduled Transactions twice if a Stash document was opened by
  double-clicking it in the Finder
* Fixed bug which didn't draw the left-hand border of an Area Chart plot item correctly if it was
  selected
* Fixed bug which sometimes caused a crash when creating a new graph if no transactions existing on the
  current date
* Fixed bug which sometimes caused a crash when pressing backspace (delete) key when the Transactions
  View wasn't active
* Changed Preferences Window to have multiple panes, allowing more categorised options:
* Added Pie Chart preferences, allowing the controlling of grouping smaller items into another item
  and the order in which the segments should be sorted
* Added Area Chart preferences, allowing the controlling of grouping smaller items into another item
* Added preferences allowing the Amounts and/or Balances of transactions to be shown in red if they
  are negative
* Added preference allowing the duration of the Recent Transaction view to be altered

Version 0.51
------------

* Fixed crash when using non-ASCII characters for text items

Version 0.5
-----------

* Added segment control to the toolbar, allowing easy switching of what transactions to show
* Added support for Area Chart graphs, plotting items over time
* Added segment control to the Pie Chart view, allowing common timeframes to be easily selected and 
  decremented/incremented
* Graphs are now updated on-the-fly, based on changes to their settings
* Multiple transactions can now be deleted at once
* Expanded transactions (showing their split items) can now be moved up/down
* When importing QIF files, there is now an option to mark all imported transactions as reconciled
* Fix issue with running under OS X 10.5.

Version 0.4
-----------

* If a Bar Chart graph segment title is going to fall off the edge of the window, move
  it back so it fits in correctly
* Stash now saves the last size/position of the main window, and restores that size/position the
  next time it loads
* Accounts and Graphs can now be renamed by selecting Rename from the right-click context menu for
  them in the Index Bar
* Stash now remembers the widths of the columns for the Transactions view between sessions
* When drawing Pie Chart graphs, and segment item with no title gets placed under "Other"
* Pie Chart graph segments can now be selected, and the cumulative amount of these segment(s) is
  now shown above the total amount
* When changing the amount of a transaction, the balance for that item, as well as all the transactions
  after it are now updated accordingly
* When removing or moving a transaction up/down, the balance for affected transactions and the
  transactions following it are now updated
* Fix bug which imported split transaction items wrongly from QIF files
* Added Toolbar to main window with Add Account and Add Graph buttons
* Splitter Views now save their sizes, so index bar should remember its size between sessions

Version 0.3
-----------

* Added Graphs, allowing Pie Charts to be displayed of Expense/Deposit Categories/Payees
* Fixed issue with adding a due scheduled transaction in certain situations
* When adding new transactions, the transaction list now scrolls down so that the new item is visible
* Added option to preferences to automatically scroll to the latest transaction when viewing an account
* Accounts and Graphs can be renamed by double-clicking the item in the IndexBar on the left
* Improve QIF file import slightly

Version 0.2
-----------

* Views now have smaller text, so more rows can fit into the same screen height
* Changed last 100 transactions view to Last 30 days, which shows all transactions from the last
  30 days
* Rearranged controls in edit views, so they take up less space and re-arrange based on main window
  size
* New transactions now get added with the date selected on the date picker control
* Added more info to the About window
* Added right-click context menu for Accounts in the Index Bar, allowing details of account to be
  viewed/edited and the account to be deleted
* Scheduled Transactions can now be enabled or disabled
* Added preferences window
* Added option to allow the most recent document to be automatically opened on startup
* Added option to make backup files when saving

Version 0.1
-----------

* Initial Version
