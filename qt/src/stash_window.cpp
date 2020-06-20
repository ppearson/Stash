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

void StashWindow::setupWindow()
{
	resize(836, 575);
	
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
	m_pIndexView->setMinimumWidth(200);
	m_pIndexView->setMaximumWidth(300);
	m_pIndexView->setMinimumHeight(300);
	m_pIndexView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	
	connect(m_pIndexView, SIGNAL(indexSelectionHasChanged(DocumentIndexType,int)), this, SLOT(docIndexSelectionHasChanged(DocumentIndexType,int)));
	
	m_pTransactionsViewWidget = new TransactionsViewWidget(this);
	
	m_pIndexSplitter->addWidget(m_pIndexView);
	m_pIndexSplitter->addWidget(m_pTransactionsViewWidget);
	m_pIndexSplitter->setHandleWidth(2);
	m_pIndexSplitter->setCollapsible(0, false);
	m_pIndexSplitter->setCollapsible(1, false);
	
	QList<int> mainSizes;
	mainSizes.push_back(200);
	mainSizes.push_back(500);
	
	m_pIndexSplitter->setSizes(mainSizes);
	
	m_pIndexSplitter->setStretchFactor(0, 0);
	m_pIndexSplitter->setStretchFactor(0, 2);
	
	m_pTransactionsViewWidget->setViewDurationType(eTransViewShowRecent);
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
	
	pFileNewAction->setText(QApplication::translate("StashWindow", "File New", 0, QApplication::UnicodeUTF8));
	
	pFileOpenAction->setText(QApplication::translate("StashWindow", "File Open...", 0, QApplication::UnicodeUTF8));
    pFileOpenAction->setShortcut(QApplication::translate("StashWindow", "Ctrl+O", 0, QApplication::UnicodeUTF8));
	
	pFileSaveAction->setText(QApplication::translate("StashWindow", "Save", 0, QApplication::UnicodeUTF8));
    pFileSaveAction->setShortcut(QApplication::translate("StashWindow", "Ctrl+S", 0, QApplication::UnicodeUTF8));
	
	pFileSaveAsAction->setText(QApplication::translate("StashWindow", "Save As...", 0, QApplication::UnicodeUTF8));
    pFileSaveAsAction->setShortcut(QApplication::translate("StashWindow", "Ctrl+Shift+S", 0, QApplication::UnicodeUTF8));
	
	menuFile->addAction(pFileNewAction);
	menuFile->addSeparator();
	menuFile->addAction(pFileOpenAction);
	menuFile->addSeparator();
	menuFile->addAction(pFileSaveAction);
	menuFile->addAction(pFileSaveAsAction);
	menuFile->addSeparator();
	
	connect(pFileNewAction, SIGNAL(triggered()), this, SLOT(fileNew()));
	connect(pFileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));
	connect(pFileSaveAction, SIGNAL(triggered()), this, SLOT(fileSave()));
	connect(pFileSaveAsAction, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
	
	
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
	m_pTransactionSplitTransaction = new QAction(this);
	
	m_pTransactionAddNewTransaction->setText(QApplication::translate("StashWindow", "New Transaction", 0, QApplication::UnicodeUTF8));
    m_pTransactionAddNewTransaction->setShortcut(QApplication::translate("StashWindow", "Ctrl+N", 0, QApplication::UnicodeUTF8));
	m_pTransactionSplitTransaction->setText(QApplication::translate("StashWindow", "Split Transaction", 0, QApplication::UnicodeUTF8));
	
	connect(m_pTransactionAddNewTransaction, SIGNAL(triggered()), this, SLOT(transactionAddNewTransaction()));
	connect(m_pTransactionSplitTransaction, SIGNAL(triggered()), this, SLOT(transactionSplitTransaction()));
	
	menuTransaction->addAction(m_pTransactionAddNewTransaction);
	menuTransaction->addAction(m_pTransactionSplitTransaction);
	
	m_pMenuBar->addMenu(menuFile);
	m_pMenuBar->addMenu(menuEdit);
	m_pMenuBar->addMenu(menuView);
	m_pMenuBar->addMenu(menuInsert);
	m_pMenuBar->addMenu(menuTransaction);
	
	setMenuBar(m_pMenuBar);
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

		fprintf(stderr, "Error: Unrecognised stash file format version.\nIt's likely that the file format of the Stash document you are trying to open is from a newer version of Stash.\n");

		return false;
	}

	fileStream.close();
	
	m_currentFilePath = fileName.toStdString();
	
	setWindowTitle(QString("Stash - ") + fileName);
	
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
	m_documentController.getDocument().clear();
	
	m_currentFilePath = "";
	
	setWindowTitle(QString("Stash"));
	
	m_pIndexView->rebuildFromDocument();
	
	m_pTransactionsViewWidget->setAccount(nullptr);
		
	m_pTransactionsViewWidget->rebuildFromAccount();
}

void StashWindow::fileOpen()
{
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
	if (m_currentFilePath.empty())
	{
		fileSaveAs();
		return;
	}
	
	// don't run yet.
	return;
	
	// TODO: hook up to settings...
	bool makeBackup = true;
	
	if (makeBackup)
	{
		std::string strPathBackup = m_currentFilePath;
		strPathBackup += ".bak";
		
		rename(m_currentFilePath.c_str(), strPathBackup.c_str());
	}
	
	std::fstream fileStream(m_currentFilePath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!fileStream)
	{
		return;
	}
	
	m_documentController.getDocument().Store(fileStream);
	
	fileStream.close();	
}

void StashWindow::fileSaveAs()
{
	
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

void StashWindow::transactionSplitTransaction()
{
	m_pTransactionsViewWidget->splitCurrentTransaction();	
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
