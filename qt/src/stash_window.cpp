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
#include <QToolBar>
#include <QToolButton>

#include <QApplication>

#include <QAction>
#include <QCloseEvent>
#include <QScrollArea>
#include <QSignalMapper>
#include <QSplitter>

#include <QTimer>

#include <QLocale>

#include <QFileDialog>
#include <QMessageBox>

#include <QVBoxLayout>

#include "../../core/scheduled_transaction.h"
#include "../../core/graph.h"

#include "document_index_view.h"
#include "transactions_view_widget.h"
#include "payees_view_widget.h"
#include "categories_view_widget.h"
#include "scheduled_transactions_view_widget.h"
#include "graphs_view_widget.h"

#include "dialogs/account_details_dialog.h"
#include "dialogs/make_transfer_dialog.h"
#include "dialogs/scheduled_transactions_due_dialog.h"
#include "dialogs/new_graph_dialog.h"

#include "settings/settings_window.h"

#include "ui_currency_formatter.h"

StashWindow::StashWindow() : QMainWindow(nullptr),
	m_pDeferredScheduledPopupTimer(nullptr),
	m_pCurrencyFormatter(nullptr)
{
	setupWindow();
}

StashWindow::~StashWindow()
{
	if (m_pCurrencyFormatter)
	{
		delete m_pCurrencyFormatter;
		m_pCurrencyFormatter = nullptr;
	}
}

void StashWindow::closeEvent(QCloseEvent* event)
{
	if (shouldDiscardCurrentDocument())
	{
		saveSettings();
		
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void StashWindow::setWindowModifiedAndRebuildIndex(bool rebuildDocIndex)
{
	setWindowModified(true);
	
	if (rebuildDocIndex)
	{
		m_pIndexView->rebuildFromDocument();
	}
}

void StashWindow::setupWindow()
{
	setTabShape(QTabWidget::Rounded);
	
	setUnifiedTitleAndToolBarOnMac(true);
	
	setWindowIcon(QIcon(":/stash/images/main_icon_0.png"));
	
	loadSettings();
		
	m_pMainContainerWidget = new QWidget(this);
	m_pMainLayout = new QVBoxLayout(m_pMainContainerWidget);
	m_pMainLayout->setMargin(0);
	m_pMainLayout->setContentsMargins(0, 0, 0, 0);
	m_pMainLayout->setSpacing(0);

	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(1);
	sizePolicy.setVerticalStretch(1);
	setSizePolicy(sizePolicy);
	
	m_pMainContainerWidget->setSizePolicy(sizePolicy);
	
	setCentralWidget(m_pMainContainerWidget);
	
	setupMenu();
	setupToolbar();
	
	configureFormatters();
	
	m_pIndexSplitter = new QSplitter(Qt::Horizontal, m_pMainContainerWidget);
	
	m_pMainLayout->addWidget(m_pIndexSplitter);
	
	m_pIndexView = new DocumentIndexView(m_documentController.getDocument(), m_pIndexSplitter, this);
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
	connect(m_pIndexView, SIGNAL(documentChangedFromIndex()), this, SLOT(documentChangedFromIndex()));
	connect(m_pIndexView, SIGNAL(deselectAnyAccount()), this, SLOT(deselectAnyAccount()));
	
	m_pTransactionsViewWidget = new TransactionsViewWidget(this, this); // bit odd, but prevents need to cast to get both...
	
	m_pPayeesViewWidget = new PayeesViewWidget(this, this);
	
	m_pCategoriesViewWidget = new CategoriesViewWidget(this, this);
	
	m_pScheduledTransactionsViewWidget = new ScheduledTransactionsViewWidget(m_documentController.getDocument(), this, this);
	
	m_pGraphsViewWidget = new GraphsViewWidget(m_documentController.getDocument(), this, this);
	
	m_pDeferredScheduledPopupTimer = new QTimer(this);
	connect(m_pDeferredScheduledPopupTimer, SIGNAL(timeout()), this, SLOT(deferredScheduledTransactionsTimeout()));
	
	m_pIndexSplitter->addWidget(m_pIndexView);
	m_pIndexSplitter->addWidget(m_pTransactionsViewWidget);
	m_pIndexSplitter->addWidget(m_pPayeesViewWidget);
	m_pIndexSplitter->addWidget(m_pCategoriesViewWidget);
	m_pIndexSplitter->addWidget(m_pScheduledTransactionsViewWidget);
	m_pIndexSplitter->addWidget(m_pGraphsViewWidget);
	
	m_pIndexSplitter->setHandleWidth(2);
	m_pIndexSplitter->setCollapsible(0, false);
	m_pIndexSplitter->setCollapsible(1, false);
	m_pIndexSplitter->setCollapsible(2, false);
	m_pIndexSplitter->setCollapsible(3, false);
	m_pIndexSplitter->setCollapsible(4, false);
	m_pIndexSplitter->setCollapsible(5, false);
	
	QList<int> mainSizes;
	mainSizes.push_back(150);
	mainSizes.push_back(500);
	mainSizes.push_back(500);
	mainSizes.push_back(500);
	mainSizes.push_back(500);
	mainSizes.push_back(500);
	
	m_pIndexSplitter->setSizes(mainSizes);
	
	m_pIndexSplitter->setStretchFactor(0, 0);
	m_pIndexSplitter->setStretchFactor(0, 2);
	
	m_pPayeesViewWidget->hide();
	m_pCategoriesViewWidget->hide();
	m_pScheduledTransactionsViewWidget->hide();
	
	m_pIndexView->resize(100, 50);
	// just to have the items resized correctly initially with a blank document...
	m_pIndexView->rebuildFromDocument();
	
	positionWindow();
	
	m_pTransactionsViewWidget->setViewDurationType(eTransViewShowRecent);
	
	setCurrentFile("");
	
	bool openLastFileOnStartup = m_settings.getBool("global/open_most_recent_file_startup", false);
	if (openLastFileOnStartup && !m_recentFiles.empty())
	{
		loadDocument(m_recentFiles[0]);
	}
}

void StashWindow::setupMenu()
{
	m_pMenuBar = new QMenuBar(this);
	m_pMenuBar->setObjectName(QString::fromUtf8("menuBar"));
		
	QMenu* menuFile = new QMenu("&File", m_pMenuBar);
	menuFile->setObjectName(QString::fromUtf8("menuFile"));	
	
	QAction* pFileNewAction = new QAction(this);
	QAction* pFileOpenAction = new QAction(this);
	QAction* pFileSaveAction = new QAction(this);
	QAction* pFileSaveAsAction = new QAction(this);
	
	QAction* pFileImportOFXFileAction = new QAction(this);
	QAction* pFileImportQIFFileAction = new QAction(this);
	
	QAction* pFileExportOFXFileAction = new QAction(this);
	QAction* pFileExportQIFFileAction = new QAction(this);
	
	pFileNewAction->setText(QApplication::translate("StashWindow", "File &New", 0));
	
	pFileOpenAction->setText(QApplication::translate("StashWindow", "File &Open...", 0));
	pFileOpenAction->setShortcut(QApplication::translate("StashWindow", "Ctrl+O", 0));
	
	pFileSaveAction->setText(QApplication::translate("StashWindow", "&Save", 0));
	pFileSaveAction->setShortcut(QApplication::translate("StashWindow", "Ctrl+S", 0));
	
	pFileSaveAsAction->setText(QApplication::translate("StashWindow", "Save &As...", 0));
	pFileSaveAsAction->setShortcut(QApplication::translate("StashWindow", "Ctrl+Shift+S", 0));
	
	pFileImportOFXFileAction->setText(QApplication::translate("StashWindow", "Import &OFX...", 0));
	pFileImportQIFFileAction->setText(QApplication::translate("StashWindow", "Import &QIF...", 0));
	
	pFileExportOFXFileAction->setText(QApplication::translate("StashWindow", "Export OFX...", 0));
	pFileExportQIFFileAction->setText(QApplication::translate("StashWindow", "Export QIF...", 0));
	
	// disable them for the moment until they're implemented...
	pFileImportOFXFileAction->setEnabled(false);
	pFileImportQIFFileAction->setEnabled(false);
	pFileExportOFXFileAction->setEnabled(false);
	pFileExportQIFFileAction->setEnabled(false);
	
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
	
	QAction* pFileExit = new QAction(this);
	pFileExit->setText("Exit");
	
	connect(pFileExit, SIGNAL(triggered()), this, SLOT(close()));
	
	menuFile->addSeparator();
	menuFile->addAction(pFileExit);
	
	QMenu* menuEdit = new QMenu("&Edit", m_pMenuBar);
	menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
	
	QMenu* menuView = new QMenu("&View", m_pMenuBar);
	menuView->setObjectName(QString::fromUtf8("menuView"));
	
	m_pViewMenuShowToolbarAction = new QAction(this);
	m_pViewMenuShowRecentTransactions = new QAction(this);
	m_pViewMenuShowThisYearTransactions = new QAction(this);
	m_pViewMenuShowAllTransactions = new QAction(this);
	
	m_pViewMenuShowToolbarAction->setText(QApplication::translate("StashWindow", "Show &Toolbar", 0));
	m_pViewMenuShowToolbarAction->setCheckable(true);
	
	m_pViewMenuShowRecentTransactions->setText(QApplication::translate("StashWindow", "&Recent Transactions", 0));
	m_pViewMenuShowRecentTransactions->setCheckable(true);
	m_pViewMenuShowThisYearTransactions->setText(QApplication::translate("StashWindow", "This &Year Transactions", 0));
	m_pViewMenuShowThisYearTransactions->setCheckable(true);
	m_pViewMenuShowAllTransactions->setText(QApplication::translate("StashWindow", "&All Transactions", 0));
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
	
	QMenu* menuInsert = new QMenu("&Insert", m_pMenuBar);
	menuInsert->setObjectName(QString::fromUtf8("menuInsert"));
	
	QAction* pInsertAccount = new QAction(this);
	QAction* pInsertGraph = new QAction(this);
	
	pInsertAccount->setText(QApplication::translate("StashWindow", "New &Account...", 0));
	pInsertGraph->setText(QApplication::translate("StashWindow", "New &Graph", 0));
	
	connect(pInsertAccount, SIGNAL(triggered()), this, SLOT(insertAccount()));
	connect(pInsertGraph, SIGNAL(triggered()), this, SLOT(insertGraph()));
	
	menuInsert->addAction(pInsertAccount);
	menuInsert->addAction(pInsertGraph);
	
	QMenu* menuTransaction = new QMenu("&Transaction", m_pMenuBar);
	menuTransaction->setObjectName(QString::fromUtf8("menuTransaction"));
	
	m_pTransactionAddNewTransaction = new QAction(this);
	m_pTransactionDeleteTransaction = new QAction(this);
	m_pTransactionSplitTransaction = new QAction(this);
	
	m_pTransactionMoveUp = new QAction(this);
	m_pTransactionMoveDown = new QAction(this);
	m_pTransactionMakeTransfer = new QAction(this);
	
	m_pTransactionAddNewTransaction->setText(QApplication::translate("StashWindow", "New Transaction", 0));
	m_pTransactionAddNewTransaction->setShortcut(QApplication::translate("StashWindow", "Ctrl+N", 0));
	m_pTransactionDeleteTransaction->setText(QApplication::translate("StashWindow", "Delete Transaction", 0));
//	m_pTransactionDeleteTransaction->setShortcut(QApplication::translate("StashWindow", "Del", 0));
	m_pTransactionSplitTransaction->setText(QApplication::translate("StashWindow", "Split Transaction", 0));
	m_pTransactionSplitTransaction->setShortcut(QApplication::translate("StashWindow", "Ctrl+L", 0));
	
	m_pTransactionMoveUp->setText(QApplication::translate("StashWindow", "Move Up", 0));
	m_pTransactionMoveUp->setShortcut(QApplication::translate("StashWindow", "Ctrl+Shift+Up", 0));
	m_pTransactionMoveDown->setText(QApplication::translate("StashWindow", "Move Down", 0));
	m_pTransactionMoveDown->setShortcut(QApplication::translate("StashWindow", "Ctrl+Shift+Down", 0));
	
	m_pTransactionMakeTransfer->setText(QApplication::translate("StashWindow", "Make Transfer...", 0));
	m_pTransactionMakeTransfer->setShortcut(QApplication::translate("StashWindow", "Ctrl+T", 0));
	
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
	
	QMenu* menuTools = new QMenu("T&ools", m_pMenuBar);
	menuTools->setObjectName(QString::fromUtf8("menuTools"));
	
	QAction* pToolsSettings = new QAction(this);
	pToolsSettings->setText("Settings...");
	
	connect(pToolsSettings, SIGNAL(triggered()), this, SLOT(toolsSettings()));
	
	menuTools->addAction(pToolsSettings);
	
	m_pMenuBar->addMenu(menuFile);
	m_pMenuBar->addMenu(menuEdit);
	m_pMenuBar->addMenu(menuView);
	m_pMenuBar->addMenu(menuInsert);
	m_pMenuBar->addMenu(menuTransaction);
	m_pMenuBar->addMenu(menuTools);
	
	setMenuBar(m_pMenuBar);
	
	updateRecentFileActions();
}

void StashWindow::setupToolbar()
{
	// Toolbar has duplicate actions of the Menu, so as to have different text/icons...
	
	m_pToolBar = new QToolBar(this);
	m_pToolBar->setObjectName(QString::fromUtf8("mainToolbar"));
	m_pToolBar->setMovable(false);
	m_pToolBar->setIconSize(QSize(24, 24));
	m_pToolBar->setFloatable(false);
	m_pToolBar->setFocusPolicy(Qt::NoFocus);
	
	QAction* pToolbarNewAccount = new QAction(QIcon(":/stash/images/add_account.png"), "", this);
	pToolbarNewAccount->setToolTip("Create a new Account");
	
	m_pToolbarShowRecentTransactions = new QAction("Recent", this);
	m_pToolbarShowRecentTransactions->setCheckable(true);
	m_pToolbarShowRecentTransactions->setToolTip("Show Recent Transactions");
	m_pToolbarShowThisYearTransactions = new QAction("This Year", this);
	m_pToolbarShowThisYearTransactions->setCheckable(true);
	m_pToolbarShowThisYearTransactions->setToolTip("Show Transactions from This Year");
	m_pToolbarShowAllTransactions = new QAction("All", this);
	m_pToolbarShowAllTransactions->setCheckable(true);
	m_pToolbarShowAllTransactions->setToolTip("Show All Transactions");
	
	m_pToolbarShowRecentTransactions->setChecked(true);
	
	m_pToolbarMakeTransfer = new QAction(QIcon(":/stash/images/make_transfer.png"), "", this);
	m_pToolbarMakeTransfer->setToolTip("Make Transfer transactions between accounts");
	
	
	connect(pToolbarNewAccount, SIGNAL(triggered()), this, SLOT(insertAccount()));
	
	connect(m_pToolbarShowRecentTransactions, SIGNAL(triggered()), this, SLOT(viewShowRecentTransactions()));
	connect(m_pToolbarShowThisYearTransactions, SIGNAL(triggered()), this, SLOT(viewShowThisYearTransactions()));
	connect(m_pToolbarShowAllTransactions, SIGNAL(triggered()), this, SLOT(viewShowAllTransactions()));

	connect(m_pToolbarMakeTransfer, SIGNAL(triggered()), this, SLOT(transactionMakeTransfer()));
	
	m_pToolBar->addAction(pToolbarNewAccount);
	m_pToolBar->addSeparator();
	m_pToolBar->addAction(m_pToolbarShowRecentTransactions);
	m_pToolBar->addAction(m_pToolbarShowThisYearTransactions);
	m_pToolBar->addAction(m_pToolbarShowAllTransactions);
	m_pToolBar->addSeparator();
	m_pToolBar->addAction(m_pToolbarMakeTransfer);

	m_pMainLayout->addWidget(m_pToolBar);
}

void StashWindow::positionWindow()
{
	QSettings& settings = m_settings.getInternal();
	if (settings.contains("windows/main_window"))
	{
		// On Linux with X11, this doesn't really work that well, and the window walks/drifts
		// up (and sometimes left) each time, but it's better than nothing for the moment, and at least the size
		// is roughly maintained...
		// Note: on Linux the above is the case in Qt 4.8, but seems fixed in later 5.x versions...
		// TODO: store the size and pos ourselves?
		restoreGeometry(settings.value("windows/main_window").toByteArray());
	}
	else
	{
		resize(900, 575);
	}
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
		// this will automatically display the transactions list for the account as well...
		m_pIndexView->selectItem(eDocIndex_Account, 0);
		
		// use a timer so that the main window can fully finish drawing for the situation where
		// the previous document is opened automatically on startup and a scheduled transaction
		// dialog needs to be shown.
		m_pDeferredScheduledPopupTimer->start(80);
	}
	
	return true;
}

bool StashWindow::saveCurrentDocument()
{
	if (m_currentFile.isEmpty())
	{
		fileSaveAs();
		return (!m_currentFile.isEmpty());
	}
		
	bool makeBackup = m_settings.getBool("global/make_backup_file_when_saving", true);
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
		QMessageBox::information(this, "Save Error", "There was an error attempting to save the current Stash document...");
		return false;
	}
	
	m_documentController.getDocument().Store(fileStream);
	
	fileStream.close();	
	
	// to update the window title and remove the modified flag
	setCurrentFile(m_currentFile);
	
	return true;
}

bool StashWindow::addScheduledTransactionAsTransaction(unsigned int schedTransactionIndex)
{
	Date today;
	
	ScheduledTransaction& schedTrans = m_documentController.getDocument().getScheduledTransaction(schedTransactionIndex);
	
	Transaction newTransaction(schedTrans.getDescription(), schedTrans.getPayee(), schedTrans.getCategory(), schedTrans.getAmount(), today);
	
	newTransaction.setType(schedTrans.getType());
	newTransaction.setCleared(true);
	
	Account& account = m_documentController.getDocument().getAccount(schedTrans.getAccount());
	account.addTransaction(newTransaction);
	
	schedTrans.AdvanceNextDate();
	
	m_pIndexView->rebuildFromDocument();
	
	m_pTransactionsViewWidget->rebuildFromAccount();
	
	m_pTransactionsViewWidget->scrollToLastTransaction(true); // we want to enforce scrolling to the very latest
	
	setWindowModifiedAndRebuildIndex(true);
	
	return true;
}

bool StashWindow::skipScheduledTransaction(unsigned int schedTransactionIndex)
{
	ScheduledTransaction& schedTrans = m_documentController.getDocument().getScheduledTransaction(schedTransactionIndex);
	schedTrans.AdvanceNextDate();
	
	setWindowModified(true);
	
	return true;
}

void StashWindow::loadSettings()
{
	QSettings& settings = m_settings.getInternal();
	m_recentFiles = settings.value("recentFiles").toStringList();
}

void StashWindow::saveSettings()
{
	QSettings& settings = m_settings.getInternal();
	
	settings.setValue("windows/main_window", saveGeometry());
	settings.setValue("recentFiles", m_recentFiles);
}

void StashWindow::configureFormatters()
{
	if (m_pCurrencyFormatter)
	{
		delete m_pCurrencyFormatter;
		m_pCurrencyFormatter = nullptr;
	}
	
	int displayCurrencySourceTypeValue = m_settings.getInt("global/display_currency_source_type", 0);
	
	if (displayCurrencySourceTypeValue == 0)
	{
		// based of locale, but use our own formatters for certain things if we have them
		QLocale locale;
		QLocale::Country country = locale.country();
		if (country == QLocale::NewZealand || country == QLocale::Australia)
		{
			m_pCurrencyFormatter = new UICurrForm_DollarNegSymbolPrefix();
		}
		else if (country == QLocale::UnitedKingdom)
		{
			m_pCurrencyFormatter = new UICurrForm_PoundSterling();
		}
		else if (country == QLocale::UnitedStates)
		{
			m_pCurrencyFormatter = new UICurrForm_DollarNegParenthesis();
		}
		else
		{
			m_pCurrencyFormatter = new UICurrForm_QLocale();
		}
	}
/*	else if (displayCurrencySourceTypeValue == 1)
	{
		// from document, not implemented yet
	}
*/	else
	{
		// use manual override type
		int manualOverrideType = m_settings.getInt("global/display_currency_manual_override", 0);
		if (manualOverrideType == 0)
		{
			m_pCurrencyFormatter = new UICurrForm_DollarNegSymbolPrefix();
		}
		else if (manualOverrideType == 1)
		{
			m_pCurrencyFormatter = new UICurrForm_PoundSterling();
		}
		else if (manualOverrideType == 2)
		{
			m_pCurrencyFormatter = new UICurrForm_DollarNegParenthesis();
		}
		else
		{
			m_pCurrencyFormatter = new UICurrForm_QLocale();
		}
	}

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

	const QString& fileName = dialog.selectedFiles()[0];
	loadDocument(fileName);
}

void StashWindow::fileSave()
{
	saveCurrentDocument();
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

	const QString& fileName = dialog.selectedFiles()[0];

	std::string path = fileName.toStdString();

	if (path.empty())
		return;
	
	std::fstream fileStream(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!fileStream)
	{
		QMessageBox::information(this, "Save Error", "There was an error saving the document.");
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
	if (visible)
	{
		m_pToolBar->show();
	}
	else
	{
		m_pToolBar->hide();
	}
}

// TODO: these three could be condensed a bit - helper function that all three call with an enum?
void StashWindow::viewShowRecentTransactions()
{
	m_pTransactionsViewWidget->setViewDurationType(eTransViewShowRecent);
	m_pViewMenuShowRecentTransactions->setChecked(true);
	m_pToolbarShowRecentTransactions->setChecked(true);
	
	m_pViewMenuShowThisYearTransactions->setChecked(false);
	m_pViewMenuShowAllTransactions->setChecked(false);
	m_pToolbarShowThisYearTransactions->setChecked(false);
	m_pToolbarShowAllTransactions->setChecked(false);
}

void StashWindow::viewShowThisYearTransactions()
{
	m_pTransactionsViewWidget->setViewDurationType(eTransViewShowThisYear);
	m_pViewMenuShowThisYearTransactions->setChecked(true);
	m_pToolbarShowThisYearTransactions->setChecked(true);
	
	m_pViewMenuShowRecentTransactions->setChecked(false);
	m_pViewMenuShowAllTransactions->setChecked(false);
	m_pToolbarShowRecentTransactions->setChecked(false);
	m_pToolbarShowAllTransactions->setChecked(false);
}

void StashWindow::viewShowAllTransactions()
{
	m_pTransactionsViewWidget->setViewDurationType(eTransViewShowAll);
	m_pViewMenuShowAllTransactions->setChecked(true);
	m_pToolbarShowAllTransactions->setChecked(true);
	
	m_pViewMenuShowRecentTransactions->setChecked(false);
	m_pViewMenuShowThisYearTransactions->setChecked(false);
	m_pToolbarShowRecentTransactions->setChecked(false);
	m_pToolbarShowThisYearTransactions->setChecked(false);
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
	
	if (m_documentController.getDocument().getAccountCount() == 1)
	{
		// select the first account...
		m_pIndexView->selectItem(eDocIndex_Account, 0, true);
	}
	
	setWindowModified(true);
}

void StashWindow::insertGraph()
{
	if (m_documentController.getDocument().getAccountCount() < 1)
	{
		QMessageBox::information(this, "Error", "At least one Account must exist before Graphs can be created.");
		return;
	}
	
	NewGraphDialog newGraphDialog(m_documentController.getDocument(), this);
	if (newGraphDialog.exec() != QDialog::Accepted)
		return;
	
	Graph newGraph;
	newGraph.setName(newGraphDialog.getName());
	newGraph.setAccount(newGraphDialog.getAccountIndex());
	
	newGraph.setIgnoreTransfers(newGraphDialog.getIgnoreTransfers());
	
	newGraph.setStartDate(newGraphDialog.getStartDate());
	newGraph.setEndDate(newGraphDialog.getEndDate());
	
	m_documentController.getDocument().addGraph(newGraph);
	
	m_pIndexView->rebuildFromDocument();
	
	setWindowModified(true);
}

void StashWindow::transactionAddNewTransaction()
{
	m_pTransactionsViewWidget->addNewTransaction();
	
	setWindowModified(true);
}

void StashWindow::transactionDeleteTransaction()
{
	m_pTransactionsViewWidget->deleteSelectedTransaction();
	
	setWindowModified(true);
}

void StashWindow::transactionSplitTransaction()
{
	m_pTransactionsViewWidget->splitCurrentTransaction();
	
	setWindowModified(true);
}

void StashWindow::transactionMoveUp()
{
	m_pTransactionsViewWidget->moveSelectedTransactionUp();
	
	setWindowModified(true);
}

void StashWindow::transactionMoveDown()
{
	m_pTransactionsViewWidget->moveSelectedTransactionDown();
	
	setWindowModified(true);
}

void StashWindow::transactionMakeTransfer()
{
	if (m_documentController.getDocument().getAccountCount() <= 1)
		return;
	
	MakeTransferDialog makeTransferDlg(m_documentController.getDocument(), this);
	
	if (makeTransferDlg.exec() != QDialog::Accepted)
		return;
	
	// create two transactions to represent the transfer between accounts.
	
	Account& fromAccount = m_documentController.getDocument().getAccount(makeTransferDlg.getFromAccountIndex());
	Account& toAccount = m_documentController.getDocument().getAccount(makeTransferDlg.getToAccountIndex());
	
	fixed transferAmount = makeTransferDlg.getAmount();
	fixed transferAmountNegative = transferAmount;
	transferAmountNegative.setNegative();
	
	Transaction fromTransaction(makeTransferDlg.getDescription(), toAccount.getName(), makeTransferDlg.getCategory(),
								transferAmountNegative, makeTransferDlg.getDate());
	
	Transaction toTransaction(makeTransferDlg.getDescription(), fromAccount.getName(), makeTransferDlg.getCategory(),
								transferAmount, makeTransferDlg.getDate());
	
	fromTransaction.setType(Transaction::Transfer);
	toTransaction.setType(Transaction::Transfer);
	
	if (makeTransferDlg.getMarkTransactionsAsCleared())
	{
		fromTransaction.setCleared(true);
		toTransaction.setCleared(true);
	}
	
	fromAccount.addTransaction(fromTransaction);
	toAccount.addTransaction(toTransaction);
	
	setWindowModified(true);
	
	m_pIndexView->rebuildFromDocument();
	
	m_pTransactionsViewWidget->rebuildFromAccount();
}

void StashWindow::toolsSettings()
{
	QSettings& settings = m_settings.getInternal();

	SettingsWindow window(settings);

	if (window.exec() == QDialog::Accepted)
	{
		// refresh stuff that might depend on settings.
		configureFormatters();
		
		m_pIndexView->rebuildFromDocument();
		
		m_pTransactionsViewWidget->rebuildFromAccount();
		m_pScheduledTransactionsViewWidget->rebuildFromDocument();
	}
}

void StashWindow::docIndexSelectionHasChanged(DocumentIndexType type, int index)
{
	if (type == eDocIndex_Account && index >= 0 && index < m_documentController.getDocument().getAccountCount())
	{
		Account* pAccount = &m_documentController.getDocument().getAccount(index);
		
		m_pTransactionsViewWidget->setAccount(pAccount);
		
		m_pTransactionsViewWidget->rebuildFromAccount();
		
		m_pPayeesViewWidget->hide();
		m_pCategoriesViewWidget->hide();
		m_pScheduledTransactionsViewWidget->hide();
		m_pGraphsViewWidget->hide();
		
		m_pTransactionsViewWidget->show();		
		
		// for the moment hard-code, might want option?
		m_pTransactionsViewWidget->scrollToLastTransaction();
	}
	else if (type == eDocIndex_ManagePayees)
	{
		m_pTransactionsViewWidget->hide();
		m_pScheduledTransactionsViewWidget->hide();
		
		m_pPayeesViewWidget->show();
		m_pCategoriesViewWidget->hide();
		m_pGraphsViewWidget->hide();
		m_pPayeesViewWidget->updatePayeesFromDocument();
	}
	else if (type == eDocIndex_ManageCategories)
	{
		m_pTransactionsViewWidget->hide();
		m_pScheduledTransactionsViewWidget->hide();
		
		m_pPayeesViewWidget->hide();
		m_pGraphsViewWidget->hide();
		
		m_pCategoriesViewWidget->show();
		m_pCategoriesViewWidget->updateCategoriesFromDocument();
	}
	else if (type == eDocIndex_ManageScheduledTransactions)
	{
		m_pTransactionsViewWidget->hide();
		m_pPayeesViewWidget->hide();
		m_pCategoriesViewWidget->hide();
		m_pGraphsViewWidget->hide();
		m_pScheduledTransactionsViewWidget->show();
		
		m_pScheduledTransactionsViewWidget->rebuildFromDocument();
	}
	else if (type == eDocIndex_Graph && index >= 0 && index < m_documentController.getDocument().getGraphCount())
	{
		Graph* pGraph = &m_documentController.getDocument().getGraph(index);
		
		m_pTransactionsViewWidget->hide();
		m_pPayeesViewWidget->hide();
		m_pCategoriesViewWidget->hide();
		m_pScheduledTransactionsViewWidget->hide();
		
		m_pGraphsViewWidget->show();
		
		m_pScheduledTransactionsViewWidget->rebuildFromDocument();
		
		m_pGraphsViewWidget->setGraph(pGraph);
		
		m_pGraphsViewWidget->rebuildFromGraph();
	}
}

void StashWindow::documentChangedFromIndex()
{
	setWindowModified(true);
}

void StashWindow::deselectAnyAccount()
{
	// show invalid account
	m_pTransactionsViewWidget->setAccount(nullptr);
	
	m_pTransactionsViewWidget->rebuildFromAccount();
}

void StashWindow::deferredScheduledTransactionsTimeout()
{
	m_pDeferredScheduledPopupTimer->stop();
	calculateDueScheduledTransactionAndDisplayDialog();
}

void StashWindow::updateRecentFileActions()
{
	QMutableStringListIterator it(m_recentFiles);
	while (it.hasNext())
	{
		// TODO: do we actually want to do this in all cases? It might be on a network drive that's been unmounted
		//       or something...
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
		// TODO: we can't just call this as-is, as it has early-outs without a return value...
		fileSave();
		return true;
	}
	else if (ret == 2) // cancel button
	{
		return false;
	}

	return true;
}

// Note: this is triggered from a timer event
void StashWindow::calculateDueScheduledTransactionAndDisplayDialog()
{
	DueSchedTransactions dueTransactions;
	
	Date today;
	
	unsigned int schedTransIndex = 0;
	std::vector<ScheduledTransaction>::const_iterator it = m_documentController.getDocument().SchedTransBegin();
	for (; it != m_documentController.getDocument().SchedTransEnd(); ++it, schedTransIndex++)
	{
		if (it->isEnabled() && it->getNextDate() <= today)
		{
			int accountIndex = it->getAccount();
			
			// TODO: this isn't really *that* much of a guarentee, ideally we'd have something like a UUID
			//       connecting the two...
			if (accountIndex >= 0 && accountIndex < m_documentController.getDocument().getAccountCount())
			{
				// Note: schedTransIndex is the index into the overall total list of scheduled transactions
				DueSchedTransactions::DueSchedTrans newTrans(schedTransIndex, it->getPayee(), it->getDescription());
				
				// TODO: again, replace this with something better...
				newTrans.amount = m_pCurrencyFormatter->formatCurrencyAmount(it->getAmount());
				newTrans.date = it->getNextDate().FormattedDate(Date::UK);
				
				const Account& account = m_documentController.getDocument().getAccount(accountIndex);
				newTrans.account = account.getName();
				dueTransactions.transactions.emplace_back(newTrans);
			}
		}
	}
	
	if (!dueTransactions.transactions.empty())
	{
		ScheduledTransactionsDueDialog* pScheduledTransactionsDlg = new ScheduledTransactionsDueDialog(this, dueTransactions);
		
		// non-modal, and will delete itself when closed.
		pScheduledTransactionsDlg->show();
	}
}
