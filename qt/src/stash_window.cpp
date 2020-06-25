/*
 * Stash:  A Personal Finance app (Qt UI).
 * Copyright (C) 2020 Peter Pearson
 * You can view the complete license in the Licence.txt file in the root
 * of the source tree.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "stash_window.h"

#include <QMenuBar>

#include <QtGui/QAction>
#include <QtGui/QApplication>

#include <QCloseEvent>
#include <QScrollArea>
#include <QSignalMapper>
#include <QSplitter>

#include <QFileDialog>
#include <QMessageBox>

#include <QVBoxLayout>

#include "document_index_view.h"
#include "transactions_view_widget.h"

#include "dialogs/account_details_dialog.h"

StashWindow::StashWindow() : QMainWindow(nullptr)
{
	setupWindow();
}

StashWindow::~StashWindow()
{
	
}

void StashWindow::closeEvent(QCloseEvent* event)
{
	if (shouldDiscardCurrentDocument())
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void StashWindow::setupWindow()
{
	resize(900, 575);
	
    setTabShape(QTabWidget::Rounded);
	
    setUnifiedTitleAndToolBarOnMac(true);
		
	m_pMainContainerWidget = new QWidget(this);
	m_pMainLayout = new QVBoxLayout(m_pMainContainerWidget);
	m_pMainLayout->setMargin(0);
	m_pMainLayout->setContentsMargins(0, 0, 0, 0);

	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(1);
	sizePolicy.setVerticalStretch(1);
	setSizePolicy(sizePolicy);
	
	m_pMainContainerWidget->setSizePolicy(sizePolicy);
	
	setCentralWidget(m_pMainContainerWidget);
	
	setupMenu();
	setupToolbar();
	
	m_pIndexSplitter = new QSplitter(Qt::Horizontal, m_pMainContainerWidget);
	
	m_pMainLayout->addWidget(m_pIndexSplitter);
	
	m_pIndexView = new DocumentIndexView(m_documentController.getDocument(), m_pIndexSplitter);
	m_pIndexView->setMinimumWidth(80);
	m_pIndexView->setMaximumWidth(250);
	m_pIndexView->setMinimumHeight(200);
	// Qt seems to ignore the 'Preferred' policy here (docs say it should use sizeHint() as initial
	// preferred width, while still allowing optional contraction and expansion, which is what we want),
	// and uses the MaximumWidth value as the initial width instead, which *isn't* what we want.
	// So we'll use 'Fixed' instead for the moment, as it's the least bad option (although it still sets
	// the initial width to the MaxiumWidth value, but at least it doesn't automatically expand further).
	m_pIndexView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	
	connect(m_pIndexView, SIGNAL(indexSelectionHasChanged(DocumentIndexType,int)), this, SLOT(docIndexSelectionHasChanged(DocumentIndexType,int)));
	
	m_pTransactionsViewWidget = new TransactionsViewWidget(this, this); // bit odd, but prevents need to cast to get both...
	
	m_pIndexSplitter->addWidget(m_pIndexView);
	m_pIndexSplitter->addWidget(m_pTransactionsViewWidget);
	m_pIndexSplitter->setHandleWidth(2);
	m_pIndexSplitter->setCollapsible(0, false);
	m_pIndexSplitter->setCollapsible(1, false);
	
	QList<int> mainSizes;
	mainSizes.push_back(150);
	mainSizes.push_back(500);
	
	m_pIndexSplitter->setSizes(mainSizes);
	
	m_pIndexSplitter->setStretchFactor(0, 0);
	m_pIndexSplitter->setStretchFactor(0, 2);
	
	m_pIndexView->resize(100, 50);
	
	m_pTransactionsViewWidget->setViewDurationType(eTransViewShowRecent);
	
	setCurrentFile("");
}

void StashWindow::setupMenu()
{
	m_pMenuBar = new QMenuBar(this);
    m_pMenuBar->setObjectName(QString::fromUtf8("menuBar"));
	
	
    QMenu* menuFile = new QMenu("File", m_pMenuBar);
    menuFile->setObjectName(QString::fromUtf8("menuFile"));	
	
	QAction* pFileNewAction = new QAction(this);
	QAction* pFileOpenAction = new QAction(this);
	QAction* pFileSaveAction = new QAction(this);
	QAction* pFileSaveAsAction = new QAction(this);
	
	QAction* pFileImportOFXFileAction = new QAction(this);
	QAction* pFileImportQIFFileAction = new QAction(this);
	
	QAction* pFileExportOFXFileAction = new QAction(this);
	QAction* pFileExportQIFFileAction = new QAction(this);
	
	pFileNewAction->setText(QApplication::translate("StashWindow", "File New", 0, QApplication::UnicodeUTF8));
	
	pFileOpenAction->setText(QApplication::translate("StashWindow", "File Open...", 0, QApplication::UnicodeUTF8));
    pFileOpenAction->setShortcut(QApplication::translate("StashWindow", "Ctrl+O", 0, QApplication::UnicodeUTF8));
	
	pFileSaveAction->setText(QApplication::translate("StashWindow", "Save", 0, QApplication::UnicodeUTF8));
    pFileSaveAction->setShortcut(QApplication::translate("StashWindow", "Ctrl+S", 0, QApplication::UnicodeUTF8));
	
	pFileSaveAsAction->setText(QApplication::translate("StashWindow", "Save As...", 0, QApplication::UnicodeUTF8));
    pFileSaveAsAction->setShortcut(QApplication::translate("StashWindow", "Ctrl+Shift+S", 0, QApplication::UnicodeUTF8));
	
	pFileImportOFXFileAction->setText(QApplication::translate("StashWindow", "Import OFX...", 0, QApplication::UnicodeUTF8));
	pFileImportQIFFileAction->setText(QApplication::translate("StashWindow", "Import QIF...", 0, QApplication::UnicodeUTF8));
	
	pFileExportOFXFileAction->setText(QApplication::translate("StashWindow", "Export OFX...", 0, QApplication::UnicodeUTF8));
	pFileExportQIFFileAction->setText(QApplication::translate("StashWindow", "Export QIF...", 0, QApplication::UnicodeUTF8));
	
	menuFile->addAction(pFileNewAction);
	menuFile->addSeparator();
	menuFile->addAction(pFileOpenAction);
	menuFile->addSeparator();
	menuFile->addAction(pFileSaveAction);
	menuFile->addAction(pFileSaveAsAction);
	menuFile->addSeparator();
	
	menuFile->addAction(pFileImportOFXFileAction);
	menuFile->addAction(pFileImportQIFFileAction);
	menuFile->addSeparator();
	menuFile->addAction(pFileExportOFXFileAction);
	menuFile->addAction(pFileExportQIFFileAction);
	
	connect(pFileNewAction, SIGNAL(triggered()), this, SLOT(fileNew()));
	connect(pFileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));
	connect(pFileSaveAction, SIGNAL(triggered()), this, SLOT(fileSave()));
	connect(pFileSaveAsAction, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
	
	connect(pFileImportOFXFileAction, SIGNAL(triggered()), this, SLOT(fileImportOFXFile()));
	connect(pFileImportQIFFileAction, SIGNAL(triggered()), this, SLOT(fileImportQIFFile()));
	connect(pFileExportOFXFileAction, SIGNAL(triggered()), this, SLOT(fileExportOFXFile()));
	connect(pFileExportQIFFileAction, SIGNAL(triggered()), this, SLOT(fileExportQIFFile()));
	
	menuFile->addSeparator();
	
	for (int i = 0; i < MaxRecentFiles; i++)
	{
		m_recentFileActions[i] = new QAction(this);
		menuFile->addAction(m_recentFileActions[i]);
	}

	for (int i = 0; i < MaxRecentFiles; ++i)
	{
		m_recentFileActions[i]->setVisible(false);
		connect(m_recentFileActions[i], SIGNAL(triggered()), this, SLOT(fileOpenRecentFile()));
	}
	
    QMenu* menuEdit = new QMenu("Edit", m_pMenuBar);
    menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
	
	QMenu* menuView = new QMenu("View", m_pMenuBar);
    menuView->setObjectName(QString::fromUtf8("menuView"));
	
	m_pViewMenuShowToolbarAction = new QAction(this);
	m_pViewMenuShowRecentTransactions = new QAction(this);
	m_pViewMenuShowThisYearTransactions = new QAction(this);
	m_pViewMenuShowAllTransactions = new QAction(this);
	
	m_pViewMenuShowToolbarAction->setText(QApplication::translate("StashWindow", "Show Toolbar", 0, QApplication::UnicodeUTF8));
	m_pViewMenuShowToolbarAction->setCheckable(true);
	
	m_pViewMenuShowRecentTransactions->setText(QApplication::translate("StashWindow", "Recent Transactions", 0, QApplication::UnicodeUTF8));
	m_pViewMenuShowRecentTransactions->setCheckable(true);
	m_pViewMenuShowThisYearTransactions->setText(QApplication::translate("StashWindow", "This Year Transactions", 0, QApplication::UnicodeUTF8));
	m_pViewMenuShowThisYearTransactions->setCheckable(true);
	m_pViewMenuShowAllTransactions->setText(QApplication::translate("StashWindow", "All Transactions", 0, QApplication::UnicodeUTF8));
	m_pViewMenuShowAllTransactions->setCheckable(true);
	
	menuView->addAction(m_pViewMenuShowToolbarAction);
	menuView->addSeparator();
	menuView->addAction(m_pViewMenuShowRecentTransactions);
	menuView->addAction(m_pViewMenuShowThisYearTransactions);
	menuView->addAction(m_pViewMenuShowAllTransactions);
	
	m_pViewMenuShowToolbarAction->setChecked(true);
	connect(m_pViewMenuShowToolbarAction, SIGNAL(toggled(bool)), this, SLOT(viewShowToolbar(bool)));
	
	m_pViewMenuShowRecentTransactions->setChecked(true);
	connect(m_pViewMenuShowRecentTransactions, SIGNAL(triggered()), this, SLOT(viewShowRecentTransactions()));
	connect(m_pViewMenuShowThisYearTransactions, SIGNAL(triggered()), this, SLOT(viewShowThisYearTransactions()));
	connect(m_pViewMenuShowAllTransactions, SIGNAL(triggered()), this, SLOT(viewShowAllTransactions()));
	
	QMenu* menuInsert = new QMenu("Insert", m_pMenuBar);
    menuInsert->setObjectName(QString::fromUtf8("menuInsert"));
	
	QAction* pInsertAccount = new QAction(this);
	QAction* pInsertGraph = new QAction(this);
	
	pInsertAccount->setText(QApplication::translate("StashWindow", "New Account", 0, QApplication::UnicodeUTF8));
	pInsertGraph->setText(QApplication::translate("StashWindow", "New Graph", 0, QApplication::UnicodeUTF8));
	
	connect(pInsertAccount, SIGNAL(triggered()), this, SLOT(insertAccount()));
	
	menuInsert->addAction(pInsertAccount);
	menuInsert->addAction(pInsertGraph);
	
	QMenu* menuTransaction = new QMenu("Transaction", m_pMenuBar);
	menuTransaction->setObjectName(QString::fromUtf8("menuTransaction"));
	
	m_pTransactionAddNewTransaction = new QAction(this);
	m_pTransactionDeleteTransaction = new QAction(this);
	m_pTransactionSplitTransaction = new QAction(this);
	
	m_pTransactionMoveUp = new QAction(this);
	m_pTransactionMoveDown = new QAction(this);
	m_pTransactionMakeTransfer = new QAction(this);
	
	m_pTransactionAddNewTransaction->setText(QApplication::translate("StashWindow", "New Transaction", 0, QApplication::UnicodeUTF8));
    m_pTransactionAddNewTransaction->setShortcut(QApplication::translate("StashWindow", "Ctrl+N", 0, QApplication::UnicodeUTF8));
	m_pTransactionDeleteTransaction->setText(QApplication::translate("StashWindow", "Delete Transaction", 0, QApplication::UnicodeUTF8));
//	m_pTransactionDeleteTransaction->setShortcut(QApplication::translate("StashWindow", "Del", 0, QApplication::UnicodeUTF8));
	m_pTransactionSplitTransaction->setText(QApplication::translate("StashWindow", "Split Transaction", 0, QApplication::UnicodeUTF8));
	m_pTransactionSplitTransaction->setShortcut(QApplication::translate("StashWindow", "Ctrl+L", 0, QApplication::UnicodeUTF8));
	
	m_pTransactionMoveUp->setText(QApplication::translate("StashWindow", "Move Up", 0, QApplication::UnicodeUTF8));
	m_pTransactionMoveDown->setText(QApplication::translate("StashWindow", "Move Down", 0, QApplication::UnicodeUTF8));
	
	m_pTransactionMakeTransfer->setText(QApplication::translate("StashWindow", "Make Transfer...", 0, QApplication::UnicodeUTF8));
	m_pTransactionMakeTransfer->setShortcut(QApplication::translate("StashWindow", "Ctrl+T", 0, QApplication::UnicodeUTF8));
	
	connect(m_pTransactionAddNewTransaction, SIGNAL(triggered()), this, SLOT(transactionAddNewTransaction()));
	connect(m_pTransactionDeleteTransaction, SIGNAL(triggered()), this, SLOT(transactionDeleteTransaction()));
	connect(m_pTransactionSplitTransaction, SIGNAL(triggered()), this, SLOT(transactionSplitTransaction()));
	
	connect(m_pTransactionMoveUp, SIGNAL(triggered()), this, SLOT(transactionMoveUp()));
	connect(m_pTransactionMoveDown, SIGNAL(triggered()), this, SLOT(transactionMoveDown()));
	connect(m_pTransactionMakeTransfer, SIGNAL(triggered()), this, SLOT(transactionMakeTransfer()));
	
	menuTransaction->addAction(m_pTransactionAddNewTransaction);
	menuTransaction->addAction(m_pTransactionDeleteTransaction);
	menuTransaction->addSeparator();
	menuTransaction->addAction(m_pTransactionSplitTransaction);
	menuTransaction->addSeparator();
	menuTransaction->addAction(m_pTransactionMoveUp);
	menuTransaction->addAction(m_pTransactionMoveDown);
	menuTransaction->addSeparator();
	menuTransaction->addAction(m_pTransactionMakeTransfer);
	
	m_pMenuBar->addMenu(menuFile);
	m_pMenuBar->addMenu(menuEdit);
	m_pMenuBar->addMenu(menuView);
	m_pMenuBar->addMenu(menuInsert);
	m_pMenuBar->addMenu(menuTransaction);
	
	setMenuBar(m_pMenuBar);
	
	updateRecentFileActions();
}

void StashWindow::setupToolbar()
{
	
}

bool StashWindow::loadDocument(const QString& fileName)
{
	std::fstream fileStream(fileName.toStdString().c_str(), std::ios::in | std::ios::binary);
	if (!fileStream)
	{
		return false;
	}

	// hacky, but...
	// this will be set to true if Load() returns false due to a future file format
	// version we don't know about...
	bool isFutureFileVersion = false;

	if (!m_documentController.getDocument().Load(fileStream, isFutureFileVersion))
	{
		fileStream.close();

		QMessageBox::information(this, "Open Error", "Unrecognised stash file format version.\nIt's likely that the file format of the Stash document you are trying to open is from a newer version of Stash.");

		return false;
	}

	fileStream.close();
	
	setCurrentFile(fileName);
	
	// update views
	
	m_pIndexView->rebuildFromDocument();
	
	if (m_documentController.getDocument().getAccountCount() > 0)
	{
		Account* pFirstAccount = &m_documentController.getDocument().getAccount(0);
		
		m_pTransactionsViewWidget->setAccount(pFirstAccount);
		
		m_pTransactionsViewWidget->rebuildFromAccount();
	}
	
	return true;
}

// menu actions
void StashWindow::fileNew()
{
	if (!shouldDiscardCurrentDocument())
		return;
	
	m_documentController.getDocument().clear();
	
	setCurrentFile("");
	
	m_pIndexView->rebuildFromDocument();
	
	m_pTransactionsViewWidget->setAccount(nullptr);
		
	m_pTransactionsViewWidget->rebuildFromAccount();
}

void StashWindow::fileOpen()
{
	if (!shouldDiscardCurrentDocument())
		return;
	
	QFileDialog dialog(this);

	dialog.setWindowTitle("Open File");
	dialog.setNameFilter(tr("Stash documents (*.stash)"));
	dialog.setFileMode(QFileDialog::ExistingFiles);
//	dialog.setDirectory(initialDir);
	dialog.setViewMode(QFileDialog::Detail);

	if (!dialog.exec())
		 return;

	QString fileName = dialog.selectedFiles()[0];
	loadDocument(fileName);
}

void StashWindow::fileSave()
{
	if (m_currentFile.isEmpty())
	{
		fileSaveAs();
		return;
	}
	
	// don't run yet.
	return;
	
	// TODO: hook up to settings...
	bool makeBackup = false;
	
	if (makeBackup)
	{
		QString strPathBackup = m_currentFile;
		strPathBackup += ".bak";
		
		// TODO: do we want to rename, or copy? copy() would be safer?
		rename(m_currentFile.toStdString().c_str(), strPathBackup.toStdString().c_str());
	}
	
	std::fstream fileStream(m_currentFile.toStdString().c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!fileStream)
	{
		return;
	}
	
	m_documentController.getDocument().Store(fileStream);
	
	fileStream.close();	
	
	// to update the window title and remove the modified flag
	setCurrentFile(m_currentFile);
}

void StashWindow::fileSaveAs()
{
	QFileDialog dialog(this, tr("Save File"));
	dialog.setNameFilter(tr("Stash documents (*.stash)"));
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setViewMode(QFileDialog::Detail);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDefaultSuffix(tr("stash"));
	dialog.setConfirmOverwrite(true);

	if (!dialog.exec())
		 return;

	QString fileName = dialog.selectedFiles()[0];

	std::string path = fileName.toStdString();

	if (path.empty())
		return;
	
	std::fstream fileStream(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!fileStream)
	{
		return;
	}
	
	if (!m_documentController.getDocument().Store(fileStream))
	{
		QMessageBox::information(this, "Save Error", "There was an error saving the document.");
	}
	else
	{
		setCurrentFile(fileName);
	}
	
	fileStream.close();
}

void StashWindow::fileImportOFXFile()
{
	
}

void StashWindow::fileImportQIFFile()
{
	
}

void StashWindow::fileExportOFXFile()
{
	
}

void StashWindow::fileExportQIFFile()
{
	
}

void StashWindow::fileOpenRecentFile()
{
	if (!shouldDiscardCurrentDocument())
		return;
	
	QAction* action = qobject_cast<QAction*>(sender());
	if (action)
		loadDocument(action->data().toString());
}

void StashWindow::viewShowToolbar(bool visible)
{
	
}

void StashWindow::viewShowRecentTransactions()
{
	m_pTransactionsViewWidget->setViewDurationType(eTransViewShowRecent);
	m_pViewMenuShowRecentTransactions->setChecked(true);
	
	m_pViewMenuShowThisYearTransactions->setChecked(false);
	m_pViewMenuShowAllTransactions->setChecked(false);
}

void StashWindow::viewShowThisYearTransactions()
{
	m_pTransactionsViewWidget->setViewDurationType(eTransViewShowThisYear);
	m_pViewMenuShowThisYearTransactions->setChecked(true);
	
	m_pViewMenuShowRecentTransactions->setChecked(false);
	m_pViewMenuShowAllTransactions->setChecked(false);
}

void StashWindow::viewShowAllTransactions()
{
	m_pTransactionsViewWidget->setViewDurationType(eTransViewShowAll);
	m_pViewMenuShowAllTransactions->setChecked(true);
	
	m_pViewMenuShowRecentTransactions->setChecked(false);
	m_pViewMenuShowThisYearTransactions->setChecked(false);
}

void StashWindow::insertAccount()
{
	AccountDetailsDialog accountDetails(this, true);
	if (accountDetails.exec() != QDialog::Accepted)
		return;
	
	// otherwise, create a new Account
	
	Account newAccount;
	newAccount.setName(accountDetails.getAccountName());	
	newAccount.setInstitution(accountDetails.getAccountInstitution());
	newAccount.setNumber(accountDetails.getAccountNumber());
	newAccount.setNote(accountDetails.getAccountNote());
	newAccount.setType(accountDetails.getAccountType());
	
	float fVal = 0.0f;
	
	std::string startingBalance = accountDetails.getAccountStartingBalance();
	if (!startingBalance.empty())
	{
		sscanf(startingBalance.c_str(), "%f", &fVal);
	}
	
	Date todaysDate;
	todaysDate.Now();
	
	Transaction startingTransaction("Starting Balance", "", "", fixed(fVal), todaysDate);
	startingTransaction.setCleared(true);
	newAccount.addTransaction(startingTransaction);
	
	m_documentController.getDocument().addAccount(newAccount);
	
	m_pIndexView->rebuildFromDocument();
	
	if (m_documentController.getDocument().getAccountCount() > 0)
	{
		Account* pFirstAccount = &m_documentController.getDocument().getAccount(0);
		
		m_pTransactionsViewWidget->setAccount(pFirstAccount);
		
		m_pTransactionsViewWidget->rebuildFromAccount();
	}
}

void StashWindow::insertGraph()
{
	
}

void StashWindow::transactionAddNewTransaction()
{
	m_pTransactionsViewWidget->addNewTransaction();
}

void StashWindow::transactionDeleteTransaction()
{
	m_pTransactionsViewWidget->deleteSelectedTransaction();
}

void StashWindow::transactionSplitTransaction()
{
	m_pTransactionsViewWidget->splitCurrentTransaction();	
}

void StashWindow::transactionMoveUp()
{
	m_pTransactionsViewWidget->moveSelectedTransactionUp();
}

void StashWindow::transactionMoveDown()
{
	m_pTransactionsViewWidget->moveSelectedTransactionDown();
}

void StashWindow::transactionMakeTransfer()
{
	
}

void StashWindow::docIndexSelectionHasChanged(DocumentIndexType type, int index)
{
	if (type == eDocIndex_Account && index >= 0 && index < m_documentController.getDocument().getAccountCount())
	{
		Account* pAccount = &m_documentController.getDocument().getAccount(index);
		
		m_pTransactionsViewWidget->setAccount(pAccount);
		
		m_pTransactionsViewWidget->rebuildFromAccount();
	}
}

void StashWindow::updateRecentFileActions()
{
	QMutableStringListIterator it(m_recentFiles);
	while (it.hasNext())
	{
		if (!QFile::exists(it.next()))
			it.remove();
	}

	for (int i = 0; i < MaxRecentFiles; i++)
	{
		if (i < m_recentFiles.count())
		{
			QString text = tr("%1. %2").arg(QString::number(i + 1), QFileInfo(m_recentFiles[i]).fileName());
			m_recentFileActions[i]->setText(text);
			m_recentFileActions[i]->setData(m_recentFiles[i]);
			m_recentFileActions[i]->setVisible(true);
		}
		else
		{
			m_recentFileActions[i]->setVisible(false);
		}
	}
}

void StashWindow::setCurrentFile(const QString& fileName)
{
	m_currentFile = fileName;
	setWindowModified(false);
	QString shownDocName = "Untitled";
	if (!m_currentFile.isEmpty())
	{
		shownDocName = QFileInfo(m_currentFile).fileName();
		m_recentFiles.removeAll(m_currentFile);
		m_recentFiles.prepend(m_currentFile);
		updateRecentFileActions();

//		QSettings& settings = Settings::instance().getInternal();
//		settings.setValue("recentFiles", m_recentFiles);
	}
	setWindowTitle(tr("%1[*] - %2").arg(shownDocName).arg(tr("Stash")));
}

bool StashWindow::shouldDiscardCurrentDocument()
{
	if (!isWindowModified())
		return true;

//	int ret = QMessageBox::warning(this, tr("Stash"), tr("The current document has been modified.\nDo you want to save your changes?"),
//					QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape);
	int ret = QMessageBox::warning(this, tr("Stash"), tr("The current document has been modified.\nDo you want to save your changes?"),
					QString("Yes"), QString("No"), QString("Cancel"),
					0, 2);

	if (ret == QMessageBox::Yes)
	{
		// TODO:  ? ...
		fileSave();
		return true;
	}
	else if (ret == QMessageBox::Cancel)
	{
		return false;
	}

	return true;
}
