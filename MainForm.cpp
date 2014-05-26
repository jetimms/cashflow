//  Copyright 2014 Jason Eric Timms
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
//  MainForm class source
//    The class that controls the models and views of the main form.

#include <QtGui>
#include <QtSql>
#include <QDebug>

#include "cashflow.hpp"
#include "MainForm.hpp"

#include "Application.hpp"
#include "DecimalFieldItemDelegate.hpp"
#include "HeaderView.hpp"
#include "ManageCategoriesForm.hpp"
#include "ManageItemsForm.hpp"
#include "SqlTableModel.hpp"
#include "TableView.hpp"

using Cashflow::Application;
using Cashflow::Data;
using Cashflow::DecimalFieldItemDelegate;
using Cashflow::HeaderView;
using Cashflow::MainForm;
using Cashflow::ManageCategoriesForm;
using Cashflow::ManageItemsForm;
using Cashflow::SqlTableModel;
using Cashflow::TableView;

static const QString applicationTitle = "Cashflow";
static const QString modifiedFileIndicator = "[*]";
static const QString titleFileSeperator = " - ";

MainForm::MainForm()
    : periodModel((SqlTableModel *)0)
    , flowModel((SqlTableModel *)0)
    , categoryModel((SqlTableModel *)0)
    , registerModel((SqlTableModel *)0)
    , unusedModel((SqlTableModel *)0)
    , periodView((TableView *)0)
    , flowView((TableView *)0)
    , categoryView((TableView *)0)
    , registerView((TableView *)0)
    , unusedView((TableView *)0)
    , registerLabel((QLabel *)0)
    , unusedLabel((QLabel *)0)
    , periodPanel((QWidget *)0)
    , flowPanel((QWidget *)0)
    , categoryPanel((QWidget *)0)
    , registerPanel((QWidget *)0)
    , unusedPanel((QWidget *)0)
    , periodDockWidget((QDockWidget *)0)
    , flowDockWidget((QDockWidget *)0)
    , categoryDockWidget((QDockWidget *)0)
    , splitter((QSplitter *)0)
    , mainGroupBox((QGroupBox *)0)
    , fileMenu((QMenu *)0)
    , editMenu((QMenu *)0)
    , viewMenu((QMenu *)0)
    , helpMenu((QMenu *)0)
    , emptyGroupBox((QGroupBox *)0)
    , newButton((QPushButton *)0)
    , openButton((QPushButton *)0)
    , exitButton((QPushButton *)0)
    , emptyButtonBox((QDialogButtonBox *)0)
    , mappingChangedFlag(false) {

  createActions();
  setupEmpty();

  // if opened file, load recent list
  if (!qApp->savedDatabaseName().isEmpty()) {
    addCurrentFileToRecentList();
  } else {
    // fill the recent file list from prior session
    updateRecentFileActions();
  }

  // clear the SQL filters
  clearModelFilters();

  statusBar()->showMessage(tr("Ready"), 2000);

  // set initial title bar text
  displayDefaultTitle();

  periodViewHorizontalHeaderSortOrder = Qt::AscendingOrder;
  flowViewHorizontalHeaderSortOrder = Qt::AscendingOrder;
  categoryViewHorizontalHeaderSortOrder = Qt::AscendingOrder;
  registerViewHorizontalHeaderSortOrder = Qt::AscendingOrder;
  unusedViewHorizontalHeaderSortOrder = Qt::AscendingOrder;

  createEmptyToolBar();
  createFileToolBar();
  createEditToolBar();
  createViewToolBar();

  showEmptyToolBar();
}

void MainForm::setupEmpty() {
  createEmptyMenus();
  setCentralWidget(new QWidget());

  QSize initialSize = QSize(INITIAL_WIDTH_BLANK, INITIAL_HEIGHT_BLANK);
  resize(initialSize);

  createEmptyButtons();

  resize(initialSize);
}

void MainForm::setup() {
  readSettings();

  createMenus();
  createPanels();
  dockSummaryPanels();

  splitter = new QSplitter(Qt::Vertical);
  splitter->setFrameStyle(QFrame::StyledPanel);
  splitter->addWidget(registerPanel);
  splitter->addWidget(unusedPanel);

  mainLayout = new QVBoxLayout;
  mainLayout->addWidget(splitter);

  mainGroupBox = new QGroupBox(tr("Register of Items"), this);
  mainGroupBox->setLayout(mainLayout);
  setCentralWidget(mainGroupBox);

  // set up connections for updating the views on a data change
  connect(
    periodModel, SIGNAL(dataSubmitted())
    , this, SLOT(updateViewsAfterChange()));

  connect(
    registerModel, SIGNAL(dataSubmitted())
    , this, SLOT(updateViewsAfterChange()));

  // set up connections for updating the titlebar and undo log
  connect(
    periodModel, SIGNAL(dataSubmitted())
    , this, SLOT(showChangedOccured()));

  connect(
    registerModel, SIGNAL(dataSubmitted())
    , this, SLOT(showChangedOccured()));

  // allow sorting columns
  periodViewHorizontalHeader = periodView->horizontalHeader();
  periodViewHorizontalHeader->setClickable(true);
  connect(
    periodViewHorizontalHeader
    , SIGNAL(sectionClicked(int))
    , this
    , SLOT(periodViewHeaderClicked(int)));

  flowViewHorizontalHeader = flowView->horizontalHeader();
  flowViewHorizontalHeader->setClickable(true);
  connect(
    flowViewHorizontalHeader
    , SIGNAL(sectionClicked(int))
    , this
    , SLOT(flowViewHeaderClicked(int)));

  categoryViewHorizontalHeader = categoryView->horizontalHeader();
  categoryViewHorizontalHeader->setClickable(true);
  connect(
    categoryViewHorizontalHeader
    , SIGNAL(sectionClicked(int))
    , this
    , SLOT(categoryViewHeaderClicked(int)));

  registerViewHorizontalHeader = registerView->horizontalHeader();
  registerViewHorizontalHeader->setClickable(true);
  connect(
    registerViewHorizontalHeader
    , SIGNAL(sectionClicked(int))
    , this
    , SLOT(registerViewHeaderClicked(int)));

  unusedViewHorizontalHeader = unusedView->horizontalHeader();
  unusedViewHorizontalHeader->setClickable(true);
  connect(
    unusedViewHorizontalHeader
    , SIGNAL(sectionClicked(int))
    , this
    , SLOT(unusedViewHeaderClicked(int)));

  periodViewHorizontalHeader->setSortIndicatorShown(true);
  flowViewHorizontalHeader->setSortIndicatorShown(true);
  categoryViewHorizontalHeader->setSortIndicatorShown(true);
  registerViewHorizontalHeader->setSortIndicatorShown(true);
  unusedViewHorizontalHeader->setSortIndicatorShown(true);

  showFileToolBar();
}

void MainForm::showChangedOccured() {
  displayUnsavedTitle();
  logUndoRedoChange();
  qApp->setLogUndoRedoIndexToMax();
  undoAction->setEnabled(!qApp->logUndoRedoIndexAtZero());
  redoAction->setEnabled(!qApp->logUndoRedoIndexAtMax());
}

void MainForm::displayDefaultTitle() {
  setWindowTitle(QObject::tr(
    applicationTitle.toUtf8()
    + titleFileSeperator.toUtf8()
    + qApp->savedDatabaseName().toUtf8()));

  setWindowModified(false);
}

void MainForm::displayUnsavedTitle() {
  setWindowTitle(QObject::tr(
    applicationTitle.toUtf8()
    + titleFileSeperator.toUtf8()
    + qApp->savedDatabaseName().toUtf8()
    + modifiedFileIndicator.toUtf8()));

  setWindowModified(true);
}

void MainForm::createPanels() {
  createPeriodPanel();
  createFlowPanel();
  createCategoryPanel();
  createRegisterPanel();
  createUnusedPanel();
}

void MainForm::dockSummaryPanels() {
  setDockOptions(QMainWindow::AllowTabbedDocks);
  setTabPosition(Qt::TopDockWidgetArea, QTabWidget::West);

  periodDockWidget = new QDockWidget(tr("&Period"));
  periodDockWidget->setObjectName("periodDockWidget");
  periodDockWidget->setWidget(periodPanel);
  periodDockWidget->setTitleBarWidget(new QWidget());

  float minWidthRatio = 9.0f / 10.0f;
  float minHeightRatio = 1.0f / 4.0f;

  periodDockWidget->setMinimumSize(
    periodView->size().width() * minWidthRatio
    , periodView->size().height() * minHeightRatio);

  float maxWidthRatio = 9.0f / 10.0f;
  float maxHeightRatio = 6.0f / 10.0f;

  periodDockWidget->setMaximumSize(
    periodView->size().width() * maxWidthRatio
    , periodView->size().height() * maxHeightRatio);

  addDockWidget(Qt::TopDockWidgetArea, periodDockWidget);

  connect(
    periodDockWidget
    , SIGNAL(visibilityChanged(bool))
    , this
    , SLOT(focusOnPeriodDockWindow(bool)));

  flowDockWidget = new QDockWidget(tr("F&low"));
  flowDockWidget->setObjectName("flowDockWidget");
  flowDockWidget->setWidget(flowPanel);
  flowDockWidget->setTitleBarWidget(new QWidget());

  flowDockWidget->setMinimumSize(
    flowView->size().width() * minWidthRatio
    , flowView->size().height() * minHeightRatio);

  flowDockWidget->setMaximumSize(
    flowView->size().width() * maxWidthRatio
    , flowView->size().height() * maxHeightRatio);

  addDockWidget(Qt::TopDockWidgetArea, flowDockWidget);

  connect(
    flowDockWidget
    , SIGNAL(visibilityChanged(bool))
    , this
    , SLOT(focusOnFlowDockWindow(bool)));

  categoryDockWidget = new QDockWidget(tr("&Category"));
  categoryDockWidget->setObjectName("categoryDockWidget");
  categoryDockWidget->setWidget(categoryPanel);
  categoryDockWidget->setTitleBarWidget(new QWidget());

  categoryDockWidget->setMinimumSize(
    categoryView->size().width() * minWidthRatio
    , categoryView->size().height() * minHeightRatio);

  categoryDockWidget->setMaximumSize(
    categoryView->size().width() * maxWidthRatio
    , categoryView->size().height() * maxHeightRatio);

  addDockWidget(Qt::TopDockWidgetArea, categoryDockWidget);

  connect(
    categoryDockWidget
    , SIGNAL(visibilityChanged(bool))
    , this
    , SLOT(focusOnCategoryDockWindow(bool)));

  tabifyDockWidget(periodDockWidget, flowDockWidget);
  tabifyDockWidget(flowDockWidget, categoryDockWidget);

  periodDockWidget->raise();
}

void MainForm::clearModelFilters() {
  periodModelFilter = "";
  flowModelFilter = "";
  categoryModelFilter = "";
  registerModelFilter = "";

  periodModelFilterLabel = "";
  flowModelFilterLabel = "";
  categoryModelFilterLabel = "";
  registerModelFilterLabel = "";
}

void MainForm::createActions() {
  createFileActions();
  createEditActions();
  createViewActions();
  createHelpActions();
}

void MainForm::createFileActions() {
  newAction = new QAction(tr("&New"), this);
  newAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 11/6.png"));
  newAction->setShortcut(QKeySequence::New);
  newAction->setStatusTip(tr("Create a new cashflow file"));
  connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

  openAction = new QAction(tr("&Open..."), this);
  openAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 11/3.png"));
  openAction->setShortcut(QKeySequence::Open);
  openAction->setStatusTip(tr("Open an existing cashflow file"));
  connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

  revertAction = new QAction(tr("&Revert"), this);
  revertAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 10/14.png"));
  revertAction->setStatusTip(tr("Revert to the last save of the current cashflow file"));
  connect(revertAction, SIGNAL(triggered()), this, SLOT(revertToSave()));

  closeAction = new QAction(tr("&Close"), this);
  closeAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 11/1.png"));
  closeAction->setShortcut(QKeySequence::Close);
  closeAction->setStatusTip(tr("Close the current cashflow file"));
  connect(closeAction, SIGNAL(triggered()), this, SLOT(closeFile()));

  saveAction = new QAction(tr("&Save"), this);
  saveAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 10/7.png"));
  saveAction->setShortcut(QKeySequence::Save);
  saveAction->setStatusTip(tr("Save the current cashflow file"));
  connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

  saveAsAction = new QAction(tr("Save &As..."), this);
  saveAsAction->setIcon(QIcon(imagePath + "/save-file-as.png"));
  saveAsAction->setShortcut(QKeySequence::SaveAs);
  saveAsAction->setStatusTip(tr("Save the current cashflow file under a new name and load the new file"));
  connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

  backupAsAction = new QAction(tr("&Backup As..."), this);
  backupAsAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 10/3.png"));
  backupAsAction->setStatusTip(tr("Save the current cashflow file under a new name and keep the original file loaded"));
  connect(backupAsAction, SIGNAL(triggered()), this, SLOT(backupAs()));

  manageCategoriesAction = new QAction(tr("Manage &Categories..."), this);
  manageCategoriesAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 5/7.png"));
  manageCategoriesAction->setShortcut(tr("Ctrl+Alt+Shift+C"));
  manageCategoriesAction->setStatusTip(
    tr("Manage the mapping of the categories"));
  connect(
    manageCategoriesAction
    , SIGNAL(triggered())
    , this
    , SLOT(manageCategories()));

  manageItemsAction = new QAction(tr("Manage &Items..."), this);
  manageItemsAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 5/8.png"));
  manageItemsAction->setShortcut(tr("Ctrl+Alt+Shift+I"));
  manageItemsAction->setStatusTip(
    tr("Manage the mapping of the items"));
  connect(
    manageItemsAction
    , SIGNAL(triggered())
    , this
    , SLOT(manageItems()));

  propertiesAction = new QAction(tr("P&roperties..."), this);
  propertiesAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 4/2.png"));
  propertiesAction->setStatusTip(tr("Give some info on the current file"));
  connect(propertiesAction, SIGNAL(triggered()), this, SLOT(properties()));

  // recently opened files action
  for (int i = 0; i < MaxRecentFiles; ++i) {
    recentFileActions[i] = new QAction(this);
    recentFileActions[i]->setVisible(false);
    connect(
      recentFileActions[i]
      , SIGNAL(triggered())
      , this
      , SLOT(openRecentFile()));
  }

  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcut(tr("Ctrl+Q"));
  exitAction->setStatusTip(tr("Exit the application"));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));
}

void MainForm::createEditActions() {
  undoAction = new QAction(tr("&Undo"), this);
  undoAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 8/4.png"));
  undoAction->setShortcut(QKeySequence::Undo);
  undoAction->setStatusTip(
    tr("Undo the data changes"));

  connect(
    undoAction
    , SIGNAL(triggered())
    , this
    , SLOT(undo()));

  redoAction = new QAction(tr("&Redo"), this);
  redoAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 8/5.png"));
  redoAction->setShortcut(QKeySequence::Redo);
  redoAction->setStatusTip(
    tr("Redo the data changes"));

  connect(
    redoAction
    , SIGNAL(triggered())
    , this
    , SLOT(redo()));

  addPeriodAction = new QAction(tr("&Add Period"), this);
  addPeriodAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 8/11.png"));
  addPeriodAction->setShortcut(tr("Ctrl+A"));
  addPeriodAction->setStatusTip(
    tr("Add a period to the budget"));
  connect(
    addPeriodAction
    , SIGNAL(triggered())
    , this
    , SLOT(addPeriod()));

  clonePeriodAction = new QAction(tr("&Clone Period"), this);
  clonePeriodAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 8/9.png"));
  clonePeriodAction->setStatusTip(
    tr("Clone the current period"));
  connect(
    clonePeriodAction
    , SIGNAL(triggered())
    , this
    , SLOT(clonePeriod()));

  deletePeriodAction = new QAction(tr("&Delete Period"), this);
  deletePeriodAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 8/12.png"));
  deletePeriodAction->setShortcut(tr("Ctrl+D"));
  deletePeriodAction->setStatusTip(
    tr("Remove a period from the budget"));
  connect(
    deletePeriodAction
    , SIGNAL(triggered())
    , this
    , SLOT(deletePeriod()));

  registerItemAction = new QAction(tr("&Register Item"), this);
  registerItemAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 8/6.png"));
  registerItemAction->setShortcut(tr("Ctrl+R"));
  registerItemAction->setStatusTip(
    tr("Register an unused item"));
  connect(
    registerItemAction
    , SIGNAL(triggered())
    , this
    , SLOT(registerItem()));

  registerAllItemsAction = new QAction(tr("&Register All Items"), this);
//  registerAllItemsAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 8/6.png"));
  registerAllItemsAction->setShortcut(tr("Ctrl+Shift+R"));
  registerAllItemsAction->setStatusTip(
    tr("Register all unused items"));
  connect(
    registerAllItemsAction
    , SIGNAL(triggered())
    , this
    , SLOT(registerAllUnregisteredItems()));

  unregisterItemAction = new QAction(tr("&Unregister Item"), this);
  unregisterItemAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 8/7.png"));
  unregisterItemAction->setShortcut(tr("Ctrl+U"));
  unregisterItemAction->setStatusTip(
    tr("Remove a registered item"));
  connect(
    unregisterItemAction
    , SIGNAL(triggered())
    , this
    , SLOT(unregisterItem()));
    
  unregisterAllItemsAction = new QAction(tr("&Unregister All Items"), this);
//  unregisterAllItemsAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 8/6.png"));
  unregisterAllItemsAction->setShortcut(tr("Ctrl+Shift+U"));
  unregisterAllItemsAction->setStatusTip(
    tr("Register all unused items"));
  connect(
    unregisterAllItemsAction
    , SIGNAL(triggered())
    , this
    , SLOT(unregisterAllRegisteredItems()));
}

void MainForm::createViewActions() {
  toggleShowPeriodAction =
    new QAction(tr("Show/Hide Period Panel"), this);
  toggleShowPeriodAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 9/5.png"));
  toggleShowPeriodAction->setShortcut(tr("Ctrl+Alt+P"));
  toggleShowPeriodAction->setStatusTip(tr("Show or hide the period panel"));
  connect(
    toggleShowPeriodAction
    , SIGNAL(triggered())
    , this
    , SLOT(toggleShowPeriodPanel()));

  toggleShowFlowAction = new QAction(tr("Show/Hide Flow Panel"), this);
  toggleShowFlowAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 9/11.png"));
  toggleShowFlowAction->setShortcut(tr("Ctrl+Alt+F"));
  toggleShowFlowAction->setStatusTip(tr("Show or hide the flow panel"));
  connect(
    toggleShowFlowAction
    , SIGNAL(triggered())
    , this
    , SLOT(toggleShowFlowPanel()));

  toggleShowCategoryAction = new QAction(tr("Show/Hide Category Panel"), this);
  toggleShowCategoryAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 9/10.png"));
  toggleShowCategoryAction->setShortcut(tr("Ctrl+Alt+C"));
  toggleShowCategoryAction->setStatusTip(tr("Show or hide the category panel"));
  connect(
    toggleShowCategoryAction
    , SIGNAL(triggered())
    , this
    , SLOT(toggleShowCategoryPanel()));

  toggleShowUnusedAction =
    new QAction(tr("Show/Hide Unregistered Item Panel"), this);
  toggleShowUnusedAction->setIcon(QIcon(imagePathSmashing_gemicons + "/row 9/9.png"));
  toggleShowUnusedAction->setShortcut(tr("Ctrl+Alt+U"));
  toggleShowUnusedAction->setStatusTip(
    tr("Show or hide the unregistered item panel"));
  connect(
    toggleShowUnusedAction
    , SIGNAL(triggered())
    , this
    , SLOT(toggleShowUnusedPanel()));
}

void MainForm::createHelpActions() {
  aboutAction = new QAction(tr("&About"), this);
  aboutAction->setStatusTip(tr("Show the application's About box"));
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAction = new QAction(tr("About &Qt"), this);
  aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
  connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainForm::createMenus() {
  revertAction->setEnabled(false);
  closeAction->setEnabled(true);
  saveAction->setEnabled(true);
  saveAsAction->setEnabled(true);
  backupAsAction->setEnabled(true);
  propertiesAction->setEnabled(true);
  manageCategoriesAction->setEnabled(true);
  manageItemsAction->setEnabled(true);

  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newAction);
  fileMenu->addAction(openAction);
  fileMenu->addAction(revertAction);
  fileMenu->addAction(closeAction);
  fileMenu->addSeparator();
  fileMenu->addAction(saveAction);
  fileMenu->addAction(saveAsAction);
  fileMenu->addAction(backupAsAction);
  fileMenu->addSeparator();
  fileMenu->addAction(propertiesAction);
  fileMenu->addSeparator();
  fileMenu->addAction(manageCategoriesAction);
  fileMenu->addAction(manageItemsAction);

  // add a seperator and save the pointer to hide it if no recently opened files
  seperatorAction = fileMenu->addSeparator();

  // add the recently opened files
  for (int i = 0; i < MaxRecentFiles; ++i) {
    fileMenu->addAction(recentFileActions[i]);
  }

  // add a seperator
  fileMenu->addSeparator();
  fileMenu->addAction(exitAction);

  editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(undoAction);
  editMenu->addAction(redoAction);
  editMenu->addSeparator();
  editMenu->addAction(addPeriodAction);
  editMenu->addAction(clonePeriodAction);
  editMenu->addAction(deletePeriodAction);
  editMenu->addSeparator();
  editMenu->addAction(registerItemAction);
  editMenu->addAction(registerAllItemsAction);
  editMenu->addAction(unregisterItemAction);
  editMenu->addAction(unregisterAllItemsAction);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(toggleShowPeriodAction);
  viewMenu->addAction(toggleShowFlowAction);
  viewMenu->addAction(toggleShowCategoryAction);
  viewMenu->addAction(toggleShowUnusedAction);

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAction);
  helpMenu->addAction(aboutQtAction);
}

void MainForm::createEmptyMenus() {
  revertAction->setEnabled(false);
  closeAction->setEnabled(false);
  saveAction->setEnabled(false);
  saveAsAction->setEnabled(false);
  backupAsAction->setEnabled(false);
  propertiesAction->setEnabled(false);
  manageCategoriesAction->setEnabled(false);
  manageItemsAction->setEnabled(false);
  manageItemsAction->setEnabled(false);
  manageItemsAction->setEnabled(false);

  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newAction);
  fileMenu->addAction(openAction);
  fileMenu->addAction(revertAction);
  fileMenu->addAction(closeAction);
  fileMenu->addSeparator();
  fileMenu->addAction(saveAction);
  fileMenu->addAction(saveAsAction);
  fileMenu->addAction(backupAsAction);
  fileMenu->addSeparator();
  fileMenu->addAction(propertiesAction);
  fileMenu->addSeparator();
  fileMenu->addAction(manageCategoriesAction);
  fileMenu->addAction(manageItemsAction);

  // add a seperator and save the pointer to hide it if no recently opened files
  seperatorAction = fileMenu->addSeparator();

  // add the recently opened files
  for (int i = 0; i < MaxRecentFiles; ++i) {
    if (recentFileActions[i] != (QAction *)0) {
      fileMenu->addAction(recentFileActions[i]);
    }
  }

  // add a menu seperator
  fileMenu->addSeparator();
  fileMenu->addAction(exitAction);

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAction);
  helpMenu->addAction(aboutQtAction);
}

void MainForm::createEmptyButtons() {
  newButton = new QPushButton(tr("New File"));
  newButton->setObjectName("newButton");
  newButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  openButton = new QPushButton(tr("Open File"));
  openButton->setObjectName("openButton");
  openButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  exitButton = new QPushButton(tr("Exit Program"));
  exitButton->setObjectName("exitButton");
  exitButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  emptyButtonBox = new QDialogButtonBox(Qt::Vertical, (QWidget *)0);
  emptyButtonBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  emptyButtonBox->addButton(newButton, QDialogButtonBox::ActionRole);
  emptyButtonBox->addButton(openButton, QDialogButtonBox::ActionRole);
  emptyButtonBox->addButton(exitButton, QDialogButtonBox::ActionRole);

  QVBoxLayout *emptyLayout = new QVBoxLayout;
  emptyLayout->addWidget(emptyButtonBox);

  connect(
    newButton
    , SIGNAL(clicked())
    , this
    , SLOT(newFile()));

  connect(
    openButton
    , SIGNAL(clicked())
    , this
    , SLOT(open()));

  connect(
    exitButton
    , SIGNAL(clicked())
    , this
    , SLOT(exitApplication()));

  emptyGroupBox = new QGroupBox(this);
  emptyGroupBox->setLayout(emptyLayout);

  setCentralWidget(emptyGroupBox);
}

void MainForm::newFile() {
  if (okToContinue()) {
    if (qApp->newFile()) {
      deleteFileFormObjects();
      setup();
      showFileToolBar();
      updateViewsAfterChange();
      periodView->setFocus();
      displayDefaultTitle();

      revertAction->setEnabled(false);
      undoAction->setEnabled(false);
      redoAction->setEnabled(false);

      // add a period for the new file
      addPeriod("initial period name");
    }
  }
}

void MainForm::open(QString fileName) {
  if (okToContinue()) {
    if (qApp->open(fileName)) {
      deleteFileFormObjects();
      setup();
      showFileToolBar();
      addCurrentFileToRecentList();
      updateViewsAfterChange();
      periodView->setFocus();
      displayDefaultTitle();

      revertAction->setEnabled(true);
      undoAction->setEnabled(!qApp->logUndoRedoIndexAtZero());
      redoAction->setEnabled(!qApp->logUndoRedoIndexAtMax());
    }
  }
}

void MainForm::revertToSave() {
  open(qApp->savedDatabaseName());
}

void MainForm::closeFile() {
  if (okToContinue()) {
    qApp->clearSavedDatabaseName();
    deleteFileFormObjects();
    setupEmpty();
    showEmptyToolBar();
    displayDefaultTitle();

    revertAction->setEnabled(false);
    undoAction->setEnabled(false);
    redoAction->setEnabled(false);
  }
}

void MainForm::openRecentFile() {
  bool isRunningOkay = true;

  QAction *action = qobject_cast<QAction *>(sender());
  QString fileName = action->data().toString();

  if (action == (QAction *)0) {
    isRunningOkay = false;
  }

  if (isRunningOkay) {
    open(fileName);
  }
}

void MainForm::openRecentFileByIndex(int index) {
  bool isRunningOkay = true;

  QAction *action = recentFileActions[index];
  QString fileName = action->data().toString();

  if (action == (QAction *)0) {
    isRunningOkay = false;
  }

  if (isRunningOkay) {
    open(fileName);
  }
}

void MainForm::deleteFileFormObjects() {
  if (periodDockWidget) {
    periodDockWidget->setParent((QDockWidget *)0);
    delete periodDockWidget;
    periodDockWidget = (QDockWidget *)0;
  }

  if (flowDockWidget) {
    flowDockWidget->setParent((QDockWidget *)0);
    delete flowDockWidget;
    flowDockWidget = (QDockWidget *)0;
  }

  if (categoryDockWidget) {
    categoryDockWidget->setParent((QDockWidget *)0);
    delete categoryDockWidget;
    categoryDockWidget = (QDockWidget *)0;
  }

  if (mainGroupBox) {
    setCentralWidget(new QWidget());
    mainGroupBox->setParent((QGroupBox *)0);
    delete mainGroupBox;
    mainGroupBox = (QGroupBox *)0;
  }

  menuBar()->clear();
}

void MainForm::save() {
  qApp->save();
  addCurrentFileToRecentList();
  displayDefaultTitle();

  revertAction->setEnabled(true);
}

void MainForm::saveAs() {
  qApp->saveAs();
  addCurrentFileToRecentList();
  displayDefaultTitle();

  revertAction->setEnabled(true);
}

void MainForm::addCurrentFileToRecentList() {
  if (qApp->addCurrentFileToRecentList()) {
    updateRecentFileActions();
  }
}

void MainForm::backupAs() {
  qApp->backupAs();
}

void MainForm::properties() {
  QMessageBox::information(
    this
    , QObject::tr("Data Properties")
    , QObject::tr("Connection: ")
      + qApp->connectionName()
      + QObject::tr("\nWorking Database: ")
      + qApp->internalDatabaseName()
      + QObject::tr("\nSaved Database: ")
      + qApp->savedDatabaseName());
}

void MainForm::exitApplication() {
  if (okToContinue()) {
    close();
  }
}

bool MainForm::okToContinue() {
  bool returnValue = true;

  if (isWindowModified()) {
    int r =
      QMessageBox::warning(
        this
        , tr("Cashflow")
        , tr(
          "The document has been modified.\n"
          "Do you want to save your changes?")
        , QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (r == QMessageBox::Yes) {
      qApp->save();
    }
    else if (r == QMessageBox::Cancel) {
      returnValue = false;
    }
  }

  return returnValue;
}

void MainForm::updateRecentFileActions() {
  QStringList recentFiles = qApp->getRecentFiles();
  QMutableStringListIterator iter(recentFiles);

  while (iter.hasNext()) {
    if (!QFile::exists(iter.next())) {
      iter.remove();
    }
  }

  for (int j = 0; j < MaxRecentFiles; ++j) {
    if (j < recentFiles.count()) {
      QString text = tr("&%1 %2").arg(j + 1).arg(recentFiles[j]);

      recentFileActions[j]->setText(text);
      recentFileActions[j]->setData(recentFiles[j]);
      recentFileActions[j]->setVisible(true);
    }
    else {
      recentFileActions[j]->setVisible(false);
    }
  }

  seperatorAction->setVisible(!recentFiles.isEmpty());
}

void MainForm::undo() {
  bool isRunningOkay = true;

  isRunningOkay = qApp->undo();

  undoAction->setEnabled(!qApp->logUndoRedoIndexAtZero());
  redoAction->setEnabled(!qApp->logUndoRedoIndexAtMax());

  if (isRunningOkay) {
    updateViewsAfterChange();

    // if unmodified before, set the display to show changes have been made
    if (qApp->logUndoRedoIndexAtSaved() == true) {
      displayDefaultTitle();
    } else {
      displayUnsavedTitle();
    }
  }
}

void MainForm::redo() {
  bool isRunningOkay = true;

  isRunningOkay = qApp->redo();

  undoAction->setEnabled(!qApp->logUndoRedoIndexAtZero());
  redoAction->setEnabled(!qApp->logUndoRedoIndexAtMax());

  if (isRunningOkay) {
    updateViewsAfterChange();

    // if unmodified before, set the display to show changes have been made
    if (qApp->logUndoRedoIndexAtSaved() == true) {
      displayDefaultTitle();
    } else {
      displayUnsavedTitle();
    }
  }
}

void MainForm::addPeriod(QString periodName) {
  bool isRunningOkay = true;

  // go to the last row in the period view and insert a row after
  int row = periodModel->rowCount();
  periodModel->insertRow(row);

  QModelIndex periodNameIndex = periodModel->index(row, PeriodMetricsView_PeriodName);
  if (!periodNameIndex.isValid()) {
    QMessageBox::warning(
      (QWidget *)0
      , QObject::tr("Error: Row not valid.")
      , QObject::tr("The period row is not valid."));

    isRunningOkay = false;
  }

  if (isRunningOkay) {
    periodView->setCurrentIndex(periodNameIndex);

    // give the period id field a new id value
    QString periodId = qApp->getNewPeriodId();

    QModelIndex periodIdIndex =
      periodNameIndex.sibling(row, PeriodMetricsView_PeriodId);

    periodModel->setData(periodIdIndex, periodId, Qt::EditRole);

    if (periodName != "") {
      periodModel->setData(periodNameIndex, periodName, Qt::EditRole);
    }

    // not sure why, but these following two methods allow the above name to
    // go in and create the period with it

    // setFocus will trigger a creation of the row
    periodView->setFocus();

    // open the editor so the user can give the new period a name
    periodView->edit(periodNameIndex);
  }
}

void MainForm::clonePeriod() {
  bool isRunningOkay = true;

  // get the source period's id
  QModelIndex sourcePeriodViewCurrent = periodView->currentIndex();

  QSqlRecord sourcePeriodRecord =
    periodModel->record(sourcePeriodViewCurrent.row());
  QString sourcePeriodId = sourcePeriodRecord.value("periodId").toString();

  // insert a period after the source period
  int row = sourcePeriodViewCurrent.row() + 1;

  periodModel->insertRow(row);

  // edit new period name
  QModelIndex periodNameIndex =
    periodModel->index(row, PeriodMetricsView_PeriodName);

  if (!periodNameIndex.isValid()) {
    QMessageBox::warning(
      (QWidget *)0
      , QObject::tr("Error: Row not valid.")
      , QObject::tr("The clone period row is not valid."));

    isRunningOkay = false;
  }

  QString periodId = "";

  if (isRunningOkay) {
    periodView->setCurrentIndex(periodNameIndex);

    // give the period id field a new id value
    periodId = qApp->getNewPeriodId();

    QModelIndex periodIdIndex =
      periodNameIndex.sibling(row, PeriodMetricsView_PeriodId);
    periodModel->setData(periodIdIndex, periodId, Qt::EditRole);

    // give a similar name to the source period
    QSqlRecord sourcePeriodRecord =
      periodModel->record(sourcePeriodViewCurrent.row());
    QString periodName =
      "Clone of " + sourcePeriodRecord.value("periodName").toString();
    periodModel->setData(periodNameIndex, periodName, Qt::EditRole);

    // submit the changes
    //   this will force the edit to submit, so it will need to be renamed
    if (!periodModel->submit()) {
      QString messageText =
        "Database Error: " + periodModel->lastError().databaseText() + "\n"
        + "Driver Error: " + periodModel->lastError().driverText() + "\n";

      if (periodModel->lastError().type() == QSqlError::StatementError) {
        messageText +=
          "Error type=" + QString::number(periodModel->lastError().type())
          + "\n"
          + "QSqlError::StatementError\n"
          + "Statement=" + periodModel->selectStatement();
      }
      else {
        messageText +=
          "Error type=" + QString::number(periodModel->lastError().type());
      }

      QMessageBox::warning(
        (QWidget *)0
        , QObject::tr("Error: Could not add row to register.")
        , messageText);

      isRunningOkay = false;
    }
  }

  if (isRunningOkay) {
    // clone the data
    qApp->clonePeriodAs(sourcePeriodId, periodId);

    updateViewsAfterChange();

    // setFocus back to the period view again
    periodView->setFocus();
  }
}

void MainForm::deletePeriod() {
  bool isRunningOkay = true;

  QModelIndex index = periodView->currentIndex();
  if (!index.isValid()) {
    QMessageBox::warning(
      (QWidget *)0
      , QObject::tr("Error: Row not valid.")
      , QObject::tr("The period row is not valid."));

    isRunningOkay = false;
  }

  if (isRunningOkay
      && !periodModel->removeRow(index.row())) {
    QMessageBox::warning(
      (QWidget *)0
      , periodModel->lastError().type()
        + " "
        + QObject::tr("Error: Could not remove row from period.")
      , periodModel->lastError().text());

    isRunningOkay = false;
  }

  if (isRunningOkay) {
    periodView->setFocus();
  }
}

void MainForm::registerItem() {
  bool isRunningOkay = true;

  if (periodModel->rowCount() == 0) {
    isRunningOkay = false;
  }

  if (unusedModel->rowCount() == 0) {
    isRunningOkay = false;
  }

  if (periodView->currentIndex() == QModelIndex()) {
    isRunningOkay = false;
  }

  QModelIndex unusedModelItemName = QModelIndex();
  int newRegisterRow = -1;

  if (isRunningOkay) {
    // go to the last row in the item view and insert a row after
    newRegisterRow = registerModel->rowCount();

    if (!registerModel->insertRow(newRegisterRow)) {
      QString messageText =
        "Database Error: " + registerModel->lastError().databaseText() + "\n"
        + "Driver Error: " + registerModel->lastError().driverText() + "\n";

      if (registerModel->lastError().type() == QSqlError::StatementError) {
        messageText +=
          "Error type=" + QString::number(registerModel->lastError().type())
          + "\n"
          + "QSqlError::StatementError\n"
          + "Statement=" + registerModel->selectStatement();
      }
      else {
        messageText +=
          "Error type=" + QString::number(registerModel->lastError().type());
      }

      QMessageBox::warning(
        (QWidget *)0
        , QObject::tr("Error: Could not insert an empty row into register.")
        , messageText);

      isRunningOkay = false;
    }
  }

  if (isRunningOkay) {
    // give the register id field the a new value
    QModelIndex registerModelRegisterId =
      registerModel->index(newRegisterRow, RegisterMetricsView_RegisterId);

    QString registerId = qApp->getNewRegisterId();

    registerModel->setData(registerModelRegisterId, registerId, Qt::EditRole);

    // give the period field the current period value
    QModelIndex periodViewCurrent = periodView->currentIndex();

    QSqlRecord periodRecord = periodModel->record(periodViewCurrent.row());
    QString periodId = periodRecord.value("periodId").toString();

    QModelIndex registerModelPeriodId =
      registerModelRegisterId.sibling(
        newRegisterRow
        , RegisterMetricsView_PeriodId);

    registerModel->setData(registerModelPeriodId, periodId, Qt::EditRole);

    // give the item id field the current unused item value
    QModelIndex unusedViewCurrent = unusedView->currentIndex();

    // if no cell or row has been selected in the unregistered item view
    if (unusedViewCurrent == QModelIndex()) {
      // select first unused item
      unusedViewCurrent = unusedView->indexAt(QPoint(0, 0));
    }

    // save the row in the unused view for later
    int newUnusedViewRow = unusedViewCurrent.row();

    if (newUnusedViewRow >= unusedModel->rowCount() - 1) {
      --newUnusedViewRow;
    }

    unusedModelItemName =
      unusedViewCurrent.sibling(newUnusedViewRow, UnusedMetricsView_ItemName);

    QModelIndex registerModelItemId =
      registerModelRegisterId.sibling(
        newRegisterRow
        , RegisterMetricsView_ItemId);

    QSqlRecord unusedRecord = unusedModel->record(unusedViewCurrent.row());
    QString itemId = unusedRecord.value("itemId").toString();

    registerModel->setData(registerModelItemId, itemId, Qt::EditRole);

    // give the budget and actual values of zero
    QModelIndex budgetFieldIndex =
      registerModelRegisterId.sibling(
        newRegisterRow
        , RegisterMetricsView_Budget);
    registerModel->setData(budgetFieldIndex, 0, Qt::EditRole);

    QModelIndex actualFieldIndex =
      registerModelRegisterId.sibling(
        newRegisterRow
        , RegisterMetricsView_Actual);
    registerModel->setData(actualFieldIndex, 0, Qt::EditRole);

    // place a empty string in the note
    QModelIndex noteFieldIndex =
      registerModelRegisterId.sibling(
        newRegisterRow
        , RegisterMetricsView_Note);
    registerModel->setData(noteFieldIndex, QString(""), Qt::EditRole);

    // set the register view index back to item name
    QModelIndex registerModelItemName =
      registerModelRegisterId.sibling(
        newRegisterRow
        , RegisterMetricsView_ItemName);
    registerView->setCurrentIndex(registerModelItemName);

    if (!registerModel->submit()) {
      QString messageText =
        "Database Error: " + registerModel->lastError().databaseText() + "\n"
        + "Driver Error: " + registerModel->lastError().driverText() + "\n";

      if (registerModel->lastError().type() == QSqlError::StatementError) {
        messageText +=
          "Error type=" + QString::number(registerModel->lastError().type())
          + "\n"
          + "QSqlError::StatementError\n"
          + "Statement=" + registerModel->selectStatement();
      }
      else {
        messageText +=
          "Error type=" + QString::number(registerModel->lastError().type());
      }

      QMessageBox::warning(
        (QWidget *)0
        , QObject::tr("Error: Could not add row to register.")
        , messageText);

      isRunningOkay = false;
    }
  }

  if (isRunningOkay) {
    updateViewsAfterChange();

    // set the focus back to the unused item view
    unusedView->setFocus();
    unusedView->setCurrentIndex(unusedModelItemName);
  }
}

void MainForm::unregisterItem(int itemRow) {
  bool isRunningOkay = true;

  if (periodModel->rowCount() == 0) {
    isRunningOkay = false;
  }

  if (registerModel->rowCount() == 0) {
    isRunningOkay = false;
  }

  if (periodView->currentIndex() == QModelIndex()) {
    isRunningOkay = false;
  }

  QModelIndex index = QModelIndex();
  
  if (isRunningOkay) {
    index = registerView->currentIndex();
    
    if (!index.isValid()) {
      // select first unused item
      index = registerView->indexAt(QPoint(0, 0));
    }
  }

  int row = (itemRow == -1) ? index.row() : itemRow;

  // check for unregistering changed values
  if (isRunningOkay) {
    // get register record from the model at given row
    QSqlRecord registerRecord = registerModel->record(row);
    QString itemName = registerRecord.value("itemName").toString();

    // get the period field the current period value
    QModelIndex periodViewIndex = periodView->currentIndex();
    QSqlRecord periodRecord = periodModel->record(periodViewIndex.row());
    QString periodName = periodRecord.value("periodName").toString();

    // show warning if record has been changed
    double budget = registerRecord.value("budget").toDouble();
    double actual = registerRecord.value("actual").toDouble();
    QString note = registerRecord.value("note").toString();

    if (
      abs(budget) >= 0.01 
      || abs(actual) >= 0.01
      || note != "")
    {
      if (unregisterChangedChoice == QMessageBox::NoToAll) {
        isRunningOkay = false;
      }
      else if (unregisterChangedChoice != QMessageBox::YesToAll) {
        unregisterChangedChoice =
          QMessageBox::warning(
            this
            , tr("Unregister Entry")
            , tr("Unregister %1 for Period %2? It contains changed values.")
              .arg(itemName)
              .arg(periodName)
            , QMessageBox::Yes 
              | QMessageBox::No 
              | QMessageBox::YesToAll
              | QMessageBox::NoToAll);
      }
          
      if (unregisterChangedChoice == QMessageBox::No
        || unregisterChangedChoice == QMessageBox::NoToAll)
      {
        isRunningOkay = false;
      }

      if (unregisterChangedChoice == QMessageBox::No
        || unregisterChangedChoice == QMessageBox::Yes)
      {
        unregisterChangedChoice = QMessageBox::NoButton;
      }
    }
  }


  if (isRunningOkay
      && !registerModel->removeRow(row)) {
    QMessageBox::warning(
      (QWidget *)0
      , registerModel->lastError().type()
        + " "
        + QObject::tr("Error: Could not remove row from register.")
      , registerModel->lastError().text());

    isRunningOkay = false;
  }

  if (isRunningOkay) {
    // save the row in the unused view for later
    if (row >= registerModel->rowCount()) {
      --row;
    }

    QModelIndex priorFieldIndex =
      registerModel->index(row, RegisterMetricsView_ItemName);

    registerView->setFocus();
    registerView->setCurrentIndex(priorFieldIndex);
  }
}

void MainForm::manageCategories() {
  int categoryId = -1;

  QModelIndex index = categoryView->currentIndex();
  if (index.isValid()) {
    QSqlRecord record = categoryModel->record(index.row());
    categoryId = record.value(CategoryMetricsView_CategoryId).toInt();
  }

  ManageCategoriesForm form(categoryId, this);

  connect(
    &form, SIGNAL(mappingChanged())
    , this, SLOT(setMappingChanged()));

  form.exec();

  disconnect(
    &form, SIGNAL(mappingChanged())
    , this, SLOT(setMappingChanged()));

  if (getMappingChanged()) {
    showChangedOccured();
    resetMappingChanged();
  }

  updateViews();
}

void MainForm::manageItems() {
  int itemId = -1;
  QModelIndex index = registerView->currentIndex();
  if (index.isValid()) {
    QSqlRecord record = registerModel->record(index.row());
    itemId = record.value(RegisterMetricsView_ItemId).toInt();
  }

  ManageItemsForm form(itemId, this);

  connect(
    &form, SIGNAL(mappingChanged())
    , this, SLOT(setMappingChanged()));

//  form.exec();
  form.show();

  disconnect(
    &form, SIGNAL(mappingChanged())
    , this, SLOT(setMappingChanged()));

  if (getMappingChanged()) {
    showChangedOccured();
    resetMappingChanged();
  }

  updateViews();
}

void MainForm::createPeriodPanel() {
  periodModel = new SqlTableModel(this);
  periodModel->setTable("periodMetricsView");
  periodModel->setSort(PeriodMetricsView_PeriodName, Qt::AscendingOrder);
  periodModel->setHeaderData(
    PeriodMetricsView_PeriodName, Qt::Horizontal, tr("Period"));
  periodModel->setHeaderData(
    PeriodMetricsView_BudgetBalance, Qt::Horizontal, tr("Budget Balance"));
  periodModel->setHeaderData(
    PeriodMetricsView_ActualBalance, Qt::Horizontal, tr("Actual Balance"));
  periodModel->setHeaderData(
    PeriodMetricsView_DifferenceBalance
    , Qt::Horizontal
    , tr("Difference Balance"));
  periodModel->setHeaderData(
    PeriodMetricsView_DifferenceBalance
    , Qt::Horizontal
    , tr("Difference Balance"));
  periodModel->setEditStrategy(QSqlTableModel::OnRowChange);
  periodModel->select();

  periodView = new TableView(this);
  periodView->setModel(periodModel);
  periodView->setItemDelegate(new QSqlRelationalDelegate(this));
//  periodView->setHorizontalHeader(new HeaderView(Qt::Horizontal, this));

  // set delegates for numeric columns
  periodView->setItemDelegateForColumn(
    PeriodMetricsView_BudgetBalance, new DecimalFieldItemDelegate(this));
  periodView->setItemDelegateForColumn(
    PeriodMetricsView_ActualBalance, new DecimalFieldItemDelegate(this));
  periodView->setItemDelegateForColumn(
    PeriodMetricsView_DifferenceBalance, new DecimalFieldItemDelegate(this));
  periodView->setSelectionMode(QAbstractItemView::SingleSelection);
  periodView->setSelectionBehavior(QAbstractItemView::SelectRows);
  periodView->setEditTriggers(
    QAbstractItemView::DoubleClicked
    | QAbstractItemView::EditKeyPressed
    | QAbstractItemView::SelectedClicked);
  periodView->setColumnHidden(PeriodMetricsView_PeriodId, true);
  periodView->verticalHeader()->setVisible(false);
  periodView->horizontalHeader()->setStretchLastSection(true);

  connect(
    periodView->selectionModel()
    , SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &))
    , this
    , SLOT(updatePeriodView()));

  periodLayout = new QVBoxLayout;
  periodLayout->addWidget(periodView);

  periodPanel = new QWidget;
  periodPanel->setLayout(periodLayout);
}

void MainForm::createFlowPanel() {
  flowModel = new SqlTableModel(this);
  flowModel->setTable("flowMetricsView");
  flowModel->setSort(FlowMetricsView_FlowName, Qt::AscendingOrder);

  flowModel->setHeaderData(
    FlowMetricsView_PeriodName, Qt::Horizontal, tr("Period"));
  flowModel->setHeaderData(
    FlowMetricsView_FlowName, Qt::Horizontal, tr("Flow"));
  flowModel->setHeaderData(
    FlowMetricsView_Budget, Qt::Horizontal, tr("Budget"));
  flowModel->setHeaderData(
    FlowMetricsView_Actual, Qt::Horizontal, tr("Actual"));
  flowModel->setHeaderData(
    FlowMetricsView_Difference, Qt::Horizontal, tr("Difference"));
  flowModel->setEditStrategy(QSqlTableModel::OnRowChange);
  flowModel->select();

  flowView = new TableView(this);
  flowView->setModel(flowModel);
  flowView->setItemDelegate(new QSqlRelationalDelegate(this));

  // set delegates for numeric columns
  flowView->setItemDelegateForColumn(
    FlowMetricsView_Budget, new DecimalFieldItemDelegate(this));
  flowView->setItemDelegateForColumn(
    FlowMetricsView_Actual, new DecimalFieldItemDelegate(this));
  flowView->setItemDelegateForColumn(
    FlowMetricsView_Difference, new DecimalFieldItemDelegate(this));
  flowView->setSelectionMode(QAbstractItemView::SingleSelection);
  flowView->setSelectionBehavior(QAbstractItemView::SelectRows);
  flowView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  flowView->setColumnHidden(FlowMetricsView_PeriodId, true);
  flowView->setColumnHidden(FlowMetricsView_FlowId, true);
  flowView->horizontalHeader()->setStretchLastSection(true);
  flowView->verticalHeader()->setVisible(false);

  connect(
    flowView->selectionModel()
    , SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &))
    , this
    , SLOT(updateFlowView()));

  flowLayout = new QVBoxLayout;
  flowLayout->addWidget(flowView);

  flowPanel = new QWidget;
  flowPanel->setLayout(flowLayout);
}

void MainForm::createCategoryPanel() {
  categoryModel = new SqlTableModel(this);
  categoryModel->setTable("categoryMetricsView");

  categoryModel->setSort(CategoryMetricsView_CategoryName, Qt::AscendingOrder);
  categoryModel->setHeaderData(
    CategoryMetricsView_PeriodName, Qt::Horizontal, tr("Period"));
  categoryModel->setHeaderData(
    CategoryMetricsView_FlowName, Qt::Horizontal, tr("Flow"));
  categoryModel->setHeaderData(
    CategoryMetricsView_CategoryName, Qt::Horizontal, tr("Category"));
  categoryModel->setHeaderData(
    CategoryMetricsView_Budget, Qt::Horizontal, tr("Budget"));
  categoryModel->setHeaderData(
    CategoryMetricsView_Actual, Qt::Horizontal, tr("Actual"));
  categoryModel->setHeaderData(
    CategoryMetricsView_Difference, Qt::Horizontal, tr("Difference"));
  categoryModel->setEditStrategy(QSqlTableModel::OnRowChange);
  categoryModel->select();

  categoryView = new TableView(this);
  categoryView->setModel(categoryModel);
  categoryView->setItemDelegate(new QSqlRelationalDelegate(this));

  // set delegates for numeric columns
  categoryView->setItemDelegateForColumn(
    CategoryMetricsView_Budget, new DecimalFieldItemDelegate(this));
  categoryView->setItemDelegateForColumn(
    CategoryMetricsView_Actual, new DecimalFieldItemDelegate(this));
  categoryView->setItemDelegateForColumn(
    CategoryMetricsView_Difference, new DecimalFieldItemDelegate(this));
  categoryView->setSelectionMode(QAbstractItemView::SingleSelection);
  categoryView->setSelectionBehavior(QAbstractItemView::SelectRows);
  categoryView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  categoryView->setColumnHidden(CategoryMetricsView_PeriodId, true);
  categoryView->setColumnHidden(CategoryMetricsView_FlowId, true);
  categoryView->setColumnHidden(CategoryMetricsView_CategoryId, true);
  categoryView->horizontalHeader()->setStretchLastSection(true);
  categoryView->verticalHeader()->setVisible(false);

  connect(
    categoryView->selectionModel()
    , SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &))
    , this
    , SLOT(updateCategoryView()));

  categoryLayout = new QVBoxLayout;
  categoryLayout->addWidget(categoryView);

  categoryPanel = new QWidget;
  categoryPanel->setLayout(categoryLayout);
}

void MainForm::createRegisterPanel() {
  registerModel = new SqlTableModel(this);
  registerModel->setTable("registerMetricsView");

  registerModel->setSort(RegisterMetricsView_ItemName, Qt::AscendingOrder);

  registerModel->setHeaderData(
    RegisterMetricsView_PeriodName, Qt::Horizontal, tr("Period"));
  registerModel->setHeaderData(
    RegisterMetricsView_FlowName, Qt::Horizontal, tr("Flow"));
  registerModel->setHeaderData(
    RegisterMetricsView_CategoryName, Qt::Horizontal, tr("Category"));
  registerModel->setHeaderData(
    RegisterMetricsView_ItemName, Qt::Horizontal, tr("Item"));
  registerModel->setHeaderData(
    RegisterMetricsView_Note, Qt::Horizontal, tr("Note"));
  registerModel->setHeaderData(
    RegisterMetricsView_Budget, Qt::Horizontal, tr("Budget"));
  registerModel->setHeaderData(
    RegisterMetricsView_Actual, Qt::Horizontal, tr("Actual"));
  registerModel->setHeaderData(
    RegisterMetricsView_Difference, Qt::Horizontal, tr("Difference"));
  registerModel->setEditStrategy(QSqlTableModel::OnRowChange);
  registerModel->select();

  connect(
    registerModel
    , SIGNAL(beforeUpdate(int, QSqlRecord &))
    , this
    , SLOT(validateRegisterModelMetrics(int, QSqlRecord &)));

  registerView = new TableView(this);
  registerView->setModel(registerModel);
  registerView->setItemDelegate(new QSqlRelationalDelegate(this));

  // set delegates for numeric columns
  registerView->setItemDelegateForColumn(
    RegisterMetricsView_Budget, new DecimalFieldItemDelegate(this));
  registerView->setItemDelegateForColumn(
    RegisterMetricsView_Actual, new DecimalFieldItemDelegate(this));
  registerView->setItemDelegateForColumn(
    RegisterMetricsView_Difference, new DecimalFieldItemDelegate(this));
  registerView->setSelectionMode(QAbstractItemView::SingleSelection);
  registerView->setSelectionBehavior(QAbstractItemView::SelectRows);
  registerView->setEditTriggers(
    QAbstractItemView::DoubleClicked
    | QAbstractItemView::EditKeyPressed
    | QAbstractItemView::SelectedClicked);
  registerView->setColumnHidden(RegisterMetricsView_RegisterId, true);
  registerView->setColumnHidden(RegisterMetricsView_PeriodId, true);
  registerView->setColumnHidden(RegisterMetricsView_FlowId, true);
  registerView->setColumnHidden(RegisterMetricsView_CategoryId, true);
  registerView->setColumnHidden(RegisterMetricsView_ItemId, true);
  registerView->horizontalHeader()->setStretchLastSection(true);
  registerView->verticalHeader()->setVisible(false);

  registerLabel = new QLabel(tr("&Register"));
  registerLabel->setBuddy(registerView);

  registerLayout = new QVBoxLayout;
  registerLayout->addWidget(registerLabel);
  registerLayout->addWidget(registerView);

  registerPanel = new QWidget;
  registerPanel->setLayout(registerLayout);
}

void MainForm::createUnusedPanel() {
  unusedModel = new SqlTableModel(this);
  unusedModel->setTable("unusedMetricsView");
  unusedModel->setSort(
    UnusedMetricsView_ItemName, Qt::AscendingOrder);
  unusedModel->setHeaderData(
    UnusedMetricsView_PeriodName, Qt::Horizontal, tr("Period"));
  unusedModel->setHeaderData(
    UnusedMetricsView_FlowName, Qt::Horizontal, tr("Flow"));
  unusedModel->setHeaderData(
    UnusedMetricsView_CategoryName, Qt::Horizontal, tr("Category"));
  unusedModel->setHeaderData(
    UnusedMetricsView_ItemName, Qt::Horizontal, tr("Item"));
  unusedModel->setEditStrategy(QSqlTableModel::OnRowChange);
  unusedModel->select();

  unusedView = new TableView(this);
  unusedView->setModel(unusedModel);
  unusedView->setItemDelegate(new QSqlRelationalDelegate(this));

  unusedView->setSelectionMode(QAbstractItemView::SingleSelection);
  unusedView->setSelectionBehavior(QAbstractItemView::SelectRows);
  unusedView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  unusedView->setColumnHidden(UnusedMetricsView_PeriodId, true);
  unusedView->setColumnHidden(UnusedMetricsView_ItemId, true);
  unusedView->horizontalHeader()->setStretchLastSection(true);
  unusedView->verticalHeader()->setVisible(false);

  unusedLabel = new QLabel(tr("&Unregistered Items"));
  unusedLabel->setBuddy(unusedView);

  unusedLayout = new QVBoxLayout;
  unusedLayout->addWidget(unusedLabel);
  unusedLayout->addWidget(unusedView);

  unusedPanel = new QWidget;
  unusedPanel->setLayout(unusedLayout);
}

void MainForm::updateViews() {
  updatePeriodView();
  updateFlowView();
  updateCategoryView();
  updateRegisterView();
}

void MainForm::updatePeriodView() {
  QModelIndex index = periodView->currentIndex();

  if (index.isValid()) {
    QSqlRecord record = periodModel->record(index.row());
    QString periodId = record.value("periodId").toString();

    periodModelFilter = QString("periodId = '%1'").arg(periodId);
    periodModelFilterLabel =
      tr("Period %1").arg(record.value("PeriodName").toString());

    flowModelFilter = "";
    flowModelFilterLabel = "";

    categoryModelFilter = "";
    categoryModelFilterLabel = "";

    registerModelFilter = "";
    registerModelFilterLabel = "";

    setFlowRestriction();
    setCategoryRestriction();
    setRegisterRestriction();
    setUnusedRestriction();
  }

  periodView->update();
}

void MainForm::updateFlowView() {
  QModelIndex index = flowView->currentIndex();

  if (index.isValid()) {
    QSqlRecord record = flowModel->record(index.row());
    QString flowId = record.value("flowId").toString();

    flowModelFilter = QString("flowId = '%1'").arg(flowId);
    flowModelFilterLabel =
      tr("Flow %1").arg(record.value("FlowName").toString());

    categoryModelFilter = "";
    categoryModelFilterLabel = "";

    registerModelFilter = "";
    registerModelFilterLabel = "";

    setCategoryRestriction();
    setRegisterRestriction();
  }

  flowView->update();
}

void MainForm::updateCategoryView() {
  QModelIndex index = categoryView->currentIndex();

  if (index.isValid()) {
    QSqlRecord record = categoryModel->record(index.row());
    QString categoryId = record.value("categoryId").toString();

    categoryModelFilter = QString("categoryId = '%1'").arg(categoryId);
    categoryModelFilterLabel =
      tr("Category %1").arg(record.value("CategoryName").toString());

    registerModelFilter = "";
    registerModelFilterLabel = "";

    setRegisterRestriction();
  }

  categoryView->update();
}

void MainForm::updateRegisterView() {
  setUnusedRestriction();

  registerView->update();
}

void MainForm::setFlowRestriction() {
  QString modelFilter = (periodModelFilter != "" ? periodModelFilter : "");
  flowModel->setFilter(modelFilter);

  QString modelFilterLabel =
    (periodModelFilterLabel != "" ? tr(" for ") + periodModelFilterLabel : "");

  flowView->horizontalHeader()->setVisible(flowModel->rowCount() > 0);
}

void MainForm::setCategoryRestriction() {
  QString modelFilter = (periodModelFilter != "" ? periodModelFilter : "");
  modelFilter +=
    (flowModelFilter != ""
    ? (modelFilter != "" ? " and " : "")
      + flowModelFilter
    : "");
  categoryModel->setFilter(modelFilter);

  QString modelFilterLabel =
    (periodModelFilterLabel != "" ? tr(" for ") + periodModelFilterLabel : "");
  modelFilterLabel +=
    (flowModelFilterLabel != ""
    ? (modelFilterLabel != "" ? " and " : "")
      + flowModelFilterLabel
    : "");

  categoryView->horizontalHeader()->setVisible(categoryModel->rowCount() > 0);
}

void MainForm::setRegisterRestriction() {
  QString modelFilter = (periodModelFilter != "" ? periodModelFilter : "");
  modelFilter +=
    (flowModelFilter != ""
    ? (modelFilter != "" ? " and " : "")
      + flowModelFilter
    : "");
  modelFilter +=
    (categoryModelFilter != ""
    ? (modelFilter != "" ? " and " : "")
      + categoryModelFilter
    : "");
  registerModel->setFilter(modelFilter);

  QString modelFilterLabel =
    (periodModelFilterLabel != "" ? tr(" for ") + periodModelFilterLabel : "");
  modelFilterLabel +=
    (flowModelFilterLabel != ""
    ? (modelFilterLabel != "" ? " and " : "")
      + flowModelFilterLabel
    : "");
  modelFilterLabel +=
    (categoryModelFilterLabel != ""
    ? (modelFilterLabel != "" ? " and " : "")
      + categoryModelFilterLabel
    : "");
  registerLabel->setText(tr("&Registered Items") + modelFilterLabel);

  registerView->horizontalHeader()->setVisible(registerModel->rowCount() > 0);
}

void MainForm::setUnusedRestriction() {
  QString modelFilter = (periodModelFilter != "" ? periodModelFilter : "");
  unusedModel->setFilter(modelFilter);

  QString modelFilterLabel =
    (periodModelFilterLabel != "" ? tr(" for ") + periodModelFilterLabel : "");
  unusedLabel->setText(tr("&Unregisted Items")  + modelFilterLabel);

  unusedView->horizontalHeader()->setVisible(unusedModel->rowCount() > 0);
}

void MainForm::about() {
  QString aboutText =
    ABOUT_NAME_AND_VERSION
    + ABOUT_COPYRIGHT_TEXT
    + ABOUT_APP_DESCRIPTION;

  QFile file("./LICENSE.TXT");
  QStringList strings;

  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&file);
    while (!in.atEnd()) {
      strings += in.readLine().split(";");
    }
  }

  aboutText +=
    "<p>" + strings.join("</p><p>") + "</p>";

  QMessageBox::about(
    this
    , tr("About cashflow")
    , tr(aboutText.toUtf8()));
}

void MainForm::toggleShowPeriodPanel() {
  bool visible = periodDockWidget->isVisible();

  if (visible) {
    periodDockWidget->hide();
  } else {
    periodDockWidget->show();
  }
}

void MainForm::toggleShowFlowPanel() {
  bool visible = flowDockWidget->isVisible();

  if (visible) {
    flowDockWidget->hide();
  } else {
    flowDockWidget->show();
  }
}

void MainForm::toggleShowCategoryPanel() {
  bool visible = categoryDockWidget->isVisible();

  if (visible) {
    categoryDockWidget->hide();
  } else {
    categoryDockWidget->show();
  }
}

void MainForm::toggleShowUnusedPanel() {
  bool visible = unusedPanel->isVisible();

  if (visible) {
    unusedPanel->hide();
  } else {
    unusedPanel->show();
  }
}

void MainForm::focusOnPeriodDockWindow(bool visible) {
  if (visible) {
    periodDockWidget->raise();
    periodView->setFocus();
  }
}

void MainForm::focusOnFlowDockWindow(bool visible) {
  if (visible) {
    flowDockWidget->raise();
    flowView->setFocus();
  }
}

void MainForm::focusOnCategoryDockWindow(bool visible) {
  if (visible) {
    categoryDockWidget->raise();
    categoryView->setFocus();
  }
}

void MainForm::updateViewsAfterChange() {
  QModelIndex periodViewIndex = periodView->currentIndex();
  QModelIndex flowViewIndex = flowView->currentIndex();
  QModelIndex categoryViewIndex = categoryView->currentIndex();

  if (!periodModel->select()) {
    QMessageBox::warning(
      (QWidget *)0
      , QObject::tr("Error: Selecting the period model failed.")
      , QObject::tr("There was an error with selecting the period model."));
  }

  periodView->setCurrentIndex(periodViewIndex);

  if (!flowModel->select()) {
    QMessageBox::warning(
      (QWidget *)0
      , QObject::tr("Error: Selecting the flow model failed.")
      , QObject::tr("There was an error with selecting the flow model."));
  }

  flowView->setCurrentIndex(flowViewIndex);

  if (!categoryModel->select()) {
    QMessageBox::warning(
      (QWidget *)0
      , QObject::tr("Error: Selecting the category model failed.")
      , QObject::tr("There was an error with selecting the category model."));
  }

  categoryView->setCurrentIndex(categoryViewIndex);
}

void MainForm::logUndoRedoChange() {
  qApp->logUndoRedoChange();
}

void MainForm::periodViewHeaderClicked(int logicalIndex) {
  if (periodViewHorizontalHeaderSortOrder == Qt::AscendingOrder) {
    periodViewHorizontalHeaderSortOrder = Qt::DescendingOrder;
  } else {
    periodViewHorizontalHeaderSortOrder = Qt::AscendingOrder;
  }

  periodViewHorizontalHeader->setSortIndicator(
    logicalIndex
    , periodViewHorizontalHeaderSortOrder);
  periodView->sortByColumn(logicalIndex);
}

void MainForm::flowViewHeaderClicked(int logicalIndex) {
  if (flowViewHorizontalHeaderSortOrder == Qt::AscendingOrder) {
    flowViewHorizontalHeaderSortOrder = Qt::DescendingOrder;
  } else {
    flowViewHorizontalHeaderSortOrder = Qt::AscendingOrder;
  }

  flowViewHorizontalHeader->setSortIndicator(
    logicalIndex
    , flowViewHorizontalHeaderSortOrder);
  flowView->sortByColumn(logicalIndex);
}

void MainForm::categoryViewHeaderClicked(int logicalIndex) {
  if (categoryViewHorizontalHeaderSortOrder == Qt::AscendingOrder) {
    categoryViewHorizontalHeaderSortOrder = Qt::DescendingOrder;
  } else {
    categoryViewHorizontalHeaderSortOrder = Qt::AscendingOrder;
  }

  categoryViewHorizontalHeader->setSortIndicator(
    logicalIndex
    , categoryViewHorizontalHeaderSortOrder);
  categoryView->sortByColumn(logicalIndex);
}

void MainForm::registerViewHeaderClicked(int logicalIndex) {
  if (registerViewHorizontalHeaderSortOrder == Qt::AscendingOrder) {
    registerViewHorizontalHeaderSortOrder = Qt::DescendingOrder;
  } else {
    registerViewHorizontalHeaderSortOrder = Qt::AscendingOrder;
  }

  registerViewHorizontalHeader->setSortIndicator(
    logicalIndex
    , registerViewHorizontalHeaderSortOrder);
  registerView->sortByColumn(logicalIndex);
}

void MainForm::unusedViewHeaderClicked(int logicalIndex) {
  if (unusedViewHorizontalHeaderSortOrder == Qt::AscendingOrder) {
    unusedViewHorizontalHeaderSortOrder = Qt::DescendingOrder;
  } else {
    unusedViewHorizontalHeaderSortOrder = Qt::AscendingOrder;
  }

  unusedViewHorizontalHeader->setSortIndicator(
    logicalIndex
    , unusedViewHorizontalHeaderSortOrder);
  unusedView->sortByColumn(logicalIndex);
}

void MainForm::setMappingChanged() {
  mappingChangedFlag = true;
}

void MainForm::resetMappingChanged() {
  mappingChangedFlag = false;
}

bool MainForm::getMappingChanged() const {
  return mappingChangedFlag;
}

void MainForm::writeSettings() const {
  if (closeAction->isEnabled() == true) {
    QSettings settings("cashflow", "cashflow");

    settings.beginGroup("MainForm");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.endGroup();
  }
}

void MainForm::readSettings() {
  QSettings settings("cashflow", "cashflow");

  settings.beginGroup("MainForm");

  QSize initialSize = QSize(INITIAL_WIDTH, INITIAL_HEIGHT);
  resize(settings.value("size", initialSize).toSize());

  QPoint initialPosition = QPoint(INITIAL_WINDOW_X, INITIAL_WINDOW_Y);
  move(settings.value("pos", initialPosition).toPoint());

  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());

  settings.endGroup();
}

void MainForm::closeEvent(QCloseEvent *event) {
  writeSettings();
  event->accept();
}

void MainForm::createEmptyToolBar() {
  emptyToolBar = addToolBar(tr("Empty Toolbar"));
  emptyToolBar->setObjectName(tr("emptyToolBar"));
  emptyToolBar->setAllowedAreas(Qt::TopToolBarArea);
  emptyToolBar->setMovable(false);

  emptyToolBar->addAction(newAction);
  emptyToolBar->addAction(openAction);

  recentFilesComboBox = new QComboBox;
  QStringList recentFiles = qApp->getRecentFiles();

  for (int i = 0; i < recentFiles.count(); ++i) {
    QString text = recentFiles[i];
    recentFilesComboBox->addItem(text);
  }

  connect(
    recentFilesComboBox
    , SIGNAL(currentIndexChanged(int))
    , this
    , SLOT(openRecentFileByIndex(int)));

  emptyToolBar->addWidget(recentFilesComboBox);
}

void MainForm::createFileToolBar() {
  fileToolBar = addToolBar(tr("File Toolbar"));
  fileToolBar->setObjectName(tr("fileToolBar"));
  fileToolBar->setAllowedAreas(Qt::TopToolBarArea);
  fileToolBar->setMovable(false);

  fileToolBar->addAction(newAction);
  fileToolBar->addAction(openAction);
  fileToolBar->addAction(revertAction);
  fileToolBar->addAction(closeAction);
  fileToolBar->addSeparator();
  fileToolBar->addAction(saveAction);
  fileToolBar->addAction(saveAsAction);
  fileToolBar->addAction(backupAsAction);
  fileToolBar->addSeparator();
  fileToolBar->addAction(propertiesAction);
  fileToolBar->addSeparator();
  fileToolBar->addAction(manageCategoriesAction);
  fileToolBar->addAction(manageItemsAction);
  fileToolBar->addSeparator();

  recentFilesComboBox = new QComboBox();
  QStringList recentFiles = qApp->getRecentFiles();

  for (int i = 0; i < recentFiles.count(); ++i) {
    QString text = recentFiles[i];
    recentFilesComboBox->addItem(text);
  }

  connect(
    recentFilesComboBox
    , SIGNAL(currentIndexChanged(int))
    , this
    , SLOT(openRecentFileByIndex(int)));

  fileToolBar->addWidget(recentFilesComboBox);
}

void MainForm::createEditToolBar() {
  addToolBarBreak();

  editToolBar = addToolBar(tr("Edit Toolbar"));
  editToolBar->setObjectName(tr("editToolBar"));
  editToolBar->setAllowedAreas(Qt::TopToolBarArea);
  editToolBar->setMovable(false);

  editToolBar->addAction(undoAction);
  editToolBar->addAction(redoAction);
  editToolBar->addSeparator();
  editToolBar->addAction(addPeriodAction);
  editToolBar->addAction(clonePeriodAction);
  editToolBar->addAction(deletePeriodAction);
  editToolBar->addSeparator();
  editToolBar->addAction(registerItemAction);
  editToolBar->addAction(unregisterItemAction);
}

void MainForm::createViewToolBar() {
  addToolBarBreak();

  viewToolBar = addToolBar(tr("View Toolbar"));
  viewToolBar->setObjectName(tr("viewToolBar"));
  viewToolBar->setAllowedAreas(Qt::TopToolBarArea);
  viewToolBar->setMovable(false);

  viewToolBar->addAction(toggleShowPeriodAction);
  viewToolBar->addAction(toggleShowFlowAction);
  viewToolBar->addAction(toggleShowCategoryAction);
  viewToolBar->addAction(toggleShowUnusedAction);
}

void MainForm::showEmptyToolBar() {
  emptyToolBar->setVisible(true);
  fileToolBar->setVisible(false);
  editToolBar->setVisible(false);
  viewToolBar->setVisible(false);
}

void MainForm::showFileToolBar() {
  emptyToolBar->setVisible(false);
  fileToolBar->setVisible(true);
  editToolBar->setVisible(true);
  viewToolBar->setVisible(true);
}

void MainForm::registerAllUnregisteredItems() {
  QModelIndex modelIndex = periodView->currentIndex();

  if (modelIndex != QModelIndex()) {
    QModelIndex periodIdIndex =
      periodModel->index(modelIndex.row(), PeriodMetricsView_PeriodId);
  
    if (!periodIdIndex.isValid()) {
      QMessageBox::warning(
        (QWidget *)0
        , QObject::tr("Error: Row not valid.")
        , QObject::tr("The period row is not valid."));
    } else {
      periodView->setFocus();
  
      quint32 totalUnusedItems = unusedModel->rowCount();
  
      QProgressDialog progress(
        "Registering All Items...", "Cancel", 0, totalUnusedItems, this);
  
      progress.setWindowModality(Qt::WindowModal);
  
      quint32 itemsLeftToConsider = unusedModel->rowCount();
  
      while (itemsLeftToConsider > 0) {
        progress.setValue(totalUnusedItems - unusedModel->rowCount());
  
        if (progress.wasCanceled())
          break;
  
        registerItem();

        itemsLeftToConsider--;
      }
  
      progress.setValue(registerModel->rowCount());

      unusedPanel->hide();

      registerView->setFocus();
    }
  }
}

void MainForm::unregisterAllRegisteredItems() {
  QModelIndex modelIndex = periodView->currentIndex();

  if (modelIndex != QModelIndex()) {
    QModelIndex periodIdIndex =
      periodModel->index(modelIndex.row(), PeriodMetricsView_PeriodId);
  
    if (!periodIdIndex.isValid()) {
      QMessageBox::warning(
        (QWidget *)0
        , QObject::tr("Error: Row not valid.")
        , QObject::tr("The period row is not valid."));
    } else {
      unusedPanel->show();

      periodView->setFocus();
  
      int totalRegisteredItems = registerModel->rowCount();
  
      QProgressDialog progress(
        "Unregistering All Items...", "Cancel", 0, totalRegisteredItems, this);
  
      progress.setWindowModality(Qt::WindowModal);

      unregisterChangedChoice = QMessageBox::NoButton;

      for (
        int itemIndex = 0;
        itemIndex < totalRegisteredItems;
        ++itemIndex)
      {
        progress.setValue(itemIndex + 1);
  
        if (progress.wasCanceled())
          break;
  
        unregisterItem(
          itemIndex - totalRegisteredItems + registerModel->rowCount());
      }

      unregisterChangedChoice = QMessageBox::NoButton;

      progress.setValue(totalRegisteredItems);

      unusedView->setFocus();
    }
  }
}

void MainForm::validateRegisterModelMetrics(
  int row
  , QSqlRecord & registerRecord)
{
  double budget = registerRecord.value("budget").toDouble();
  double actual = registerRecord.value("actual").toDouble();

  // if the budget value wasn't updated, grab it from the record
  if (!registerRecord.isGenerated("budget")) {
    QSqlRecord registerRecord = registerModel->record(row);
    budget = registerRecord.value("budget").toDouble();
  }

  // if the actual value wasn't updated, grab it from the record
  if (!registerRecord.isGenerated("actual")) {
    QSqlRecord registerRecord = registerModel->record(row);
    actual = registerRecord.value("actual").toDouble();
  }

  QString rules = 
    QObject::tr(
    "The budget and actual values are positive (or zero) and the actual value "
    "cannot exceed budget value, ie. you must budget for what you actually "
    "spend."
    "\n\n"
    "The value(s) will be corrected for you.");

  bool correctionNeeded = false;

  if (budget < 0) {
    correctionNeeded = true;
    
    budget = 0;
    registerRecord.setValue("budget", budget);
     registerRecord.setGenerated("budget", true);
  }

  if (actual < 0) {
    correctionNeeded = true;

    actual = 0;
    registerRecord.setValue("actual", actual);
    registerRecord.setGenerated("actual", true);
  }
  
  if (budget < actual) {
    correctionNeeded = true;

    budget = actual;
    registerRecord.setValue("budget", budget);
    registerRecord.setGenerated("budget", true);
  }

  if (correctionNeeded) {
    QMessageBox::information(this, QObject::tr("Validation Rules"), rules);
  }
}
