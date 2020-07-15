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

#ifndef STASH_WINDOW_H
#define STASH_WINDOW_H

#include <QMainWindow>

#include "settings_state.h"
#include "document_controller.h"

#include "view_common.h"

class QScrollArea;
class QSignalMapper;
class QWidget;
class QVBoxLayout;
class QSplitter;
class QTimer;

class DocumentIndexView;

class TransactionsViewWidget;
class ScheduledTransactionsViewWidget;

class StashWindow : public QMainWindow
{
	Q_OBJECT
public:
	StashWindow();
	virtual ~StashWindow();
	
	virtual void closeEvent(QCloseEvent* event);
	
	void setWindowModifiedAndRebuildIndex(bool rebuildDocIndex);
	
	// this isn't ideal, but it needs to be passed somehow, and I'm less happy with a singleton...
	const SettingsState& getSettingsState() const
	{
		return m_settings;
	}
	
protected:
	
	void setupWindow();
	
	void setupMenu();
	void setupToolbar();
	
	void positionWindow();
	
	bool loadDocument(const QString& fileName);
	bool saveCurrentDocument();
	
	friend class ScheduledTransactionsDueDialog;
	
	// I'm not amazingly happy about this, but it's simplest...
	bool addScheduledTransactionAsTransaction(unsigned int schedTransactionIndex);
	bool skipScheduledTransaction(unsigned int schedTransactionIndex);
	
	void loadSettings();
	void saveSettings();
	
public slots:
	
	// menu slots
	void fileNew();
	void fileOpen();
	void fileSave();
	void fileSaveAs();
	
	void fileImportOFXFile();
	void fileImportQIFFile();
	
	void fileExportOFXFile();
	void fileExportQIFFile();
	
	void fileOpenRecentFile();
	
	void viewShowToolbar(bool visible);
	
	void viewShowRecentTransactions();
	void viewShowThisYearTransactions();
	void viewShowAllTransactions();
	
	void insertAccount();
	void insertGraph();
	
	void transactionAddNewTransaction();
	void transactionDeleteTransaction();
	void transactionSplitTransaction();
	void transactionMoveUp();
	void transactionMoveDown();
	void transactionMakeTransfer();
	
	void toolsSettings();
	
	void docIndexSelectionHasChanged(DocumentIndexType type, int index);
	void documentChangedFromIndex(); // trigger document modify from index (for account deletion)
	void deselectAnyAccount(); // for deselecting/hiding transaction when the last account is deleted.
	
	void deferredScheduledTransactionsTimeout();
	
protected:
	void updateRecentFileActions();
	void setCurrentFile(const QString& fileName);
	bool shouldDiscardCurrentDocument();
	
	void calculateDueScheduledTransactionAndDisplayDialog();
	
	
protected:
	DocumentController		m_documentController;
	
protected:
	// widget stuff
	QMenuBar*				m_pMenuBar;
	QToolBar*				m_pToolBar;
	
	// Menu actions
	QAction*				m_pViewMenuShowToolbarAction;
	QAction*				m_pViewMenuShowRecentTransactions;
	QAction*				m_pViewMenuShowThisYearTransactions;
	QAction*				m_pViewMenuShowAllTransactions;
	
	QAction*				m_pTransactionAddNewTransaction;
	QAction*				m_pTransactionDeleteTransaction;
	QAction*				m_pTransactionSplitTransaction;
	QAction*				m_pTransactionMoveUp;
	QAction*				m_pTransactionMoveDown;
	QAction*				m_pTransactionMakeTransfer;
	
	// Toolbar actions (separate as they have different text, icons, at least for the Show ones).
	// TODO: some of them could be shared...
	QAction*				m_pToolbarShowRecentTransactions;
	QAction*				m_pToolbarShowThisYearTransactions;
	QAction*				m_pToolbarShowAllTransactions;
	QAction*				m_pToolbarMakeTransfer;
	
	QVBoxLayout*			m_pMainLayout;
	QWidget*				m_pMainContainerWidget; // for everything (including index)
	
	QSplitter*				m_pIndexSplitter;
	DocumentIndexView*		m_pIndexView;
	
	
	
	TransactionsViewWidget*				m_pTransactionsViewWidget;
	ScheduledTransactionsViewWidget*	m_pScheduledTransactionsViewWidget;
	
	QTimer*					m_pDeferredScheduledPopupTimer;
	
	
	SettingsState				m_settings;
	
	// other state
	QString						m_currentFile;
	
	QStringList					m_recentFiles;
	enum { MaxRecentFiles = 5 };
	QAction*					m_recentFileActions[MaxRecentFiles];
};

#endif // STASH_WINDOW_H
