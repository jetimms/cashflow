//	Copyright	2014 Jason Eric	Timms
//
//		Licensed under the Apache	License, Version 2.0 (the	"License");
//		you	may	not	use	this file	except in	compliance with	the	License.
//		You	may	obtain a copy	of the License at
//
//			http://www.apache.org/licenses/LICENSE-2.0
//
//		Unless required	by applicable	law	or agreed	to in	writing, software
//		distributed	under	the	License	is distributed on	an "AS IS" BASIS,
//		WITHOUT	WARRANTIES OR	CONDITIONS OF	ANY	KIND,	either express or	implied.
//		See	the	License	for	the	specific language	governing	permissions	and
//		limitations	under	the	License.
//
//	MainForm class definition
//		The	class	that controls	the	models and views of	the	main form.

#ifndef	_MAINFORM_HPP_
	#define	_MAINFORM_HPP_

	#include <QMainWindow>
	#include <QMessageBox>
	#include <QSqlRelationalTableModel>
	#include <QTableView>

	#include "SqlTableModel.hpp"

	class	QAction;
	class QComboBox;
	class	QDialogButtonBox;
	class	QGroupBox;
	class	QHBoxLayout;
	class	QLabel;
	class	QMenu;
	class	QModelIndex;
	class	QPushButton;
	class	QShortcut;
	class	QSplitter;
	class	QSqlTableModel;
	class QToolBar;
	class	QVBoxLayout;

  namespace Cashflow {
  	enum {
  		// table column	enums
  		Period_Id	=	0
  		,	Period_Name	=	1

  		,	Flow_Id	=	0
  		,	Flow_Name	=	1
  		// metrics view	column enums
  		,	PeriodMetricsView_PeriodId = 0
  		,	PeriodMetricsView_PeriodName = 1
  		,	PeriodMetricsView_BudgetBalance	=	2
  		,	PeriodMetricsView_ActualBalance	=	3
  		,	PeriodMetricsView_DifferenceBalance	=	4

  		,	FlowMetricsView_PeriodId = 0
  		,	FlowMetricsView_FlowId = 1
  		,	FlowMetricsView_PeriodName = 2
  		,	FlowMetricsView_FlowName = 3
  		,	FlowMetricsView_Budget = 4
  		,	FlowMetricsView_Actual = 5
  		,	FlowMetricsView_Difference = 6
  	};

  	enum {
  		MaxRecentFiles = 5
  	};

  	class	MainForm : public	QMainWindow	{
  		Q_OBJECT

  	public:
  		MainForm();

  	private	slots:
  		void setup();
  		void setupEmpty();

  		void newFile();
  		void open(QString	fileName = QString());
  		void revertToSave();
  		void closeFile();
  		void save();
  		void saveAs();
  		void backupAs();
  		void openRecentFile();
  		void openRecentFileByIndex(int index);
  		void properties();
  		void exitApplication();

  		void manageCategories();
  		void manageItems();

  		void undo();
  		void redo();

  		void addPeriod(QString periodName = "");
  		void clonePeriod();
  		void deletePeriod();

  		void registerItem();
  		void unregisterItem(int itemRow = -1);

  		void toggleShowPeriodPanel();
  		void toggleShowFlowPanel();
  		void toggleShowCategoryPanel();
  		void toggleShowUnusedPanel();

  		void about();

  		void focusOnPeriodDockWindow(bool	visible);
  		void focusOnFlowDockWindow(bool	visible);
  		void focusOnCategoryDockWindow(bool	visible);

  		void updateViews();
  		void updatePeriodView();
  		void updateFlowView();
  		void updateCategoryView();

  		void updateViewsAfterChange();

  		void showChangedOccured();
  		void displayDefaultTitle();
  		void displayUnsavedTitle();

  		void logUndoRedoChange();

  		void periodViewHeaderClicked(int);
  		void flowViewHeaderClicked(int);
  		void categoryViewHeaderClicked(int);
  		void registerViewHeaderClicked(int);
  		void unusedViewHeaderClicked(int);

  		void setMappingChanged();

      void registerAllUnregisteredItems();
      void unregisterAllRegisteredItems();

      void validateRegisterModelMetrics(int, QSqlRecord &);

  	private:
  		bool okToContinue();
  		void addCurrentFileToRecentList();
  		void getRecentFiles();

  		void createPanels();
  		void createPeriodPanel();
  		void createFlowPanel();
  		void createCategoryPanel();
  		void createRegisterPanel();
  		void createUnusedPanel();

  		void dockSummaryPanels();
  		void clearModelFilters();

  		void createActions();
  		void createFileActions();
  		void createEditActions();
  		void createViewActions();
  		void createHelpActions();

  		void createMenus();
  		void createEmptyMenus();
  		void createEmptyButtons();

  		void deleteFileFormObjects();

  		void setFlowRestriction();
  		void setCategoryRestriction();
  		void setRegisterRestriction();
  		void setUnusedRestriction();

  		void updateRegisterView();
  		void updateRecentFileActions();

  		void resetMappingChanged();
  		bool getMappingChanged() const;

  		void writeSettings() const;
  		void readSettings();

  		void closeEvent(QCloseEvent	*event);

      void createEmptyToolBar();
      void createFileToolBar();
      void createEditToolBar();
      void createViewToolBar();

      void showEmptyToolBar();
      void showFileToolBar();

  		SqlTableModel	*periodModel;
  		SqlTableModel	*flowModel;
  		SqlTableModel	*categoryModel;
  		SqlTableModel	*registerModel;
  		SqlTableModel	*unusedModel;

  		QTableView *periodView;
  		QTableView *flowView;
  		QTableView *categoryView;
  		QTableView *registerView;
  		QTableView *unusedView;

  		QHeaderView	*periodViewHorizontalHeader;
  		QHeaderView	*flowViewHorizontalHeader;
  		QHeaderView	*categoryViewHorizontalHeader;
  		QHeaderView	*registerViewHorizontalHeader;
  		QHeaderView	*unusedViewHorizontalHeader;

  		Qt::SortOrder	periodViewHorizontalHeaderSortOrder;
  		Qt::SortOrder	flowViewHorizontalHeaderSortOrder;
  		Qt::SortOrder	categoryViewHorizontalHeaderSortOrder;
  		Qt::SortOrder	registerViewHorizontalHeaderSortOrder;
  		Qt::SortOrder	unusedViewHorizontalHeaderSortOrder;

  		QLabel *registerLabel;
  		QLabel *unusedLabel;

  		QWidget	*periodPanel;
  		QWidget	*flowPanel;
  		QWidget	*categoryPanel;
  		QWidget	*registerPanel;
  		QWidget	*unusedPanel;

  		QVBoxLayout	*periodLayout;
  		QVBoxLayout	*flowLayout;
  		QVBoxLayout	*categoryLayout;
  		QVBoxLayout	*registerLayout;
  		QVBoxLayout	*unusedLayout;

  		QString	periodModelFilter;
  		QString	flowModelFilter;
  		QString	categoryModelFilter;
  		QString	registerModelFilter;

  		QString	periodModelFilterLabel;
  		QString	flowModelFilterLabel;
  		QString	categoryModelFilterLabel;
  		QString	registerModelFilterLabel;

  		QDockWidget	*periodDockWidget;
  		QDockWidget	*flowDockWidget;
  		QDockWidget	*categoryDockWidget;

  		QSplitter	*splitter;
  		QVBoxLayout	*mainLayout;
  		QGroupBox	*mainGroupBox;

  		QShortcut	*periodDockWidgetShortcut;

  		QAction	*newAction;
  		QAction	*openAction;
  		QAction	*revertAction;
  		QAction	*closeAction;
  		QAction	*saveAction;
  		QAction	*saveAsAction;
  		QAction	*backupAsAction;
  		QAction	*manageCategoriesAction;
  		QAction	*manageItemsAction;
  		QAction	*recentFileActions[MaxRecentFiles];
  		QAction	*seperatorAction;

  		QAction	*propertiesAction;

  		QAction	*undoAction;
  		QAction	*redoAction;
  		QAction	*addPeriodAction;
  		QAction	*clonePeriodAction;
  		QAction	*deletePeriodAction;
  		QAction	*registerItemAction;
  		QAction	*registerAllItemsAction;
  		QAction	*unregisterItemAction;
  		QAction	*unregisterAllItemsAction;

  		QAction	*toggleShowPeriodAction;
  		QAction	*toggleShowFlowAction;
  		QAction	*toggleShowCategoryAction;
  		QAction	*toggleShowUnusedAction;

  		QAction	*exitAction;
  		QAction	*aboutAction;
  		QAction	*aboutQtAction;

  		QMenu	*fileMenu;
  		QMenu	*editMenu;
  		QMenu	*viewMenu;
  		QMenu	*helpMenu;

  		QGroupBox	*emptyGroupBox;
  		QPushButton	*newButton;
  		QPushButton	*openButton;
  		QPushButton	*exitButton;
  		QDialogButtonBox *emptyButtonBox;

  		bool mappingChangedFlag;

      QToolBar *emptyToolBar;
      QToolBar *fileToolBar;
      QToolBar *editToolBar;
      QToolBar *viewToolBar;

      QComboBox *recentFilesComboBox;
      
      QMessageBox::StandardButtons unregisterChangedChoices;
      enum QMessageBox::StandardButton unregisterChangedChoice;
  	};
  }
#endif //	_MAINFORM_HPP_
