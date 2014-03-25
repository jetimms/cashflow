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

#include "MainForm.hpp"
#include "Application.hpp"
#include "cashflow.hpp"
#include "ManageCategoriesForm.hpp"
#include "ManageItemsForm.hpp"
#include "DecimalFieldItemDelegate.hpp"
#include "SqlTableModel.hpp"
#include "TableView.hpp"

#include "HeaderView.hpp"

using Cashflow::Application;
using Cashflow::Data;

static const QString applicationTitle = "Cashflow";
static const QString modifiedFileIndicator = "[*]";
static const QString titleFileSeperator = " - ";

MainForm::MainForm()
    : periodModel((SqlTableModel *)0)
    , flowModel((SqlTableModel *)0)
    , categoryModel((SqlTableModel *)0)
    , registerModel((SqlTableModel *)0)
    , unusedModel((QSqlRelationalTableModel *)0)
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
    , mappingChanged(false) {

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
  setDockOptions(QMainWindow::ForceTabbedDocks);
  setTabPosition(Qt::TopDockWidgetArea, QTabWidget::South);

  periodDockWidget = new QDockWidget(tr("&Period"));
  periodDockWidget->setWidget(periodPanel);
  periodDockWidget->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::TopDockWidgetArea, periodDockWidget);

  connect(
    periodDockWidget
    , SIGNAL(visibilityChanged(bool))
    , this
    , SLOT(focusOnPeriodDockWindow(bool)));

  flowDockWidget = new QDockWidget(tr("F&low"));
  flowDockWidget->setWidget(flowPanel);
  flowDockWidget->setTitleBarWidget(new QWidget());
  addDockWidget(Qt::TopDockWidgetArea, flowDockWidget);

  connect(
    flowDockWidget
    , SIGNAL(visibilityChanged(bool))
    , this
    , SLOT(focusOnFlowDockWindow(bool)));

  categoryDockWidget = new QDockWidget(tr("&Category"));
  categoryDockWidget->setWidget(categoryPanel);
  categoryDockWidget->setTitleBarWidget(new QWidget());
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
  newAction->setIcon(QIcon(":/images/add-item.png"));
  newAction->setShortcut(QKeySequence::New);
  newAction->setStatusTip(tr("Create a new cashflow file"));
  connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

  openAction = new QAction(tr("&Open..."), this);
//  openAction->setIcon(QIcon(":/images/open.png"));
  openAction->setShortcut(QKeySequence::Open);
  openAction->setStatusTip(tr("Open an existing cashflow file"));
  connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

  revertAction = new QAction(tr("&Revert"), this);
//  revertAction->setIcon(QIcon(":/images/revert.png"));
  revertAction->setStatusTip(tr("Revert to the last save of the current cashflow file"));
  connect(revertAction, SIGNAL(triggered()), this, SLOT(revertToSave()));

  closeAction = new QAction(tr("&Close"), this);
  closeAction->setShortcut(QKeySequence::Close);
  closeAction->setStatusTip(tr("Close the current cashflow file"));
  connect(closeAction, SIGNAL(triggered()), this, SLOT(closeFile()));

  saveAction = new QAction(tr("&Save"), this);
  saveAction->setIcon(QIcon(":/images/save.png"));
  saveAction->setShortcut(QKeySequence::Save);
  saveAction->setStatusTip(tr("Save the current cashflow file"));
  connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

  saveAsAction = new QAction(tr("Save &As..."), this);
  saveAsAction->setShortcut(QKeySequence::SaveAs);
  saveAsAction->setStatusTip(tr("Save the current cashflow file under a new name and load the new file"));
  connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

  backupAsAction = new QAction(tr("&Backup As..."), this);
  backupAsAction->setStatusTip(tr("Save the current cashflow file under a new name and keep the original file loaded"));
  connect(backupAsAction, SIGNAL(triggered()), this, SLOT(backupAs()));

  manageCategoriesAction = new QAction(tr("Manage &Categories..."), this);
  manageCategoriesAction->setShortcut(tr("Ctrl+Alt+Shift+C"));
  manageCategoriesAction->setStatusTip(
    tr("Manage the mapping of the categories"));
  connect(
    manageCategoriesAction
    , SIGNAL(triggered())
    , this
    , SLOT(manageCategories()));

  manageItemsAction = new QAction(tr("Manage &Items..."), this);
  manageItemsAction->setShortcut(tr("Ctrl+Alt+Shift+I"));
  manageItemsAction->setStatusTip(
    tr("Manage the mapping of the items"));
  connect(
    manageItemsAction
    , SIGNAL(triggered())
    , this
    , SLOT(manageItems()));

  propertiesAction = new QAction(tr("P&roperties..."), this);
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
  undoAction->setShortcut(QKeySequence::Undo);
  undoAction->setStatusTip(
    tr("Undo the data changes"));

  connect(
    undoAction
    , SIGNAL(triggered())
    , this
    , SLOT(undo()));

  redoAction = new QAction(tr("&Redo"), this);
  redoAction->setShortcut(QKeySequence::Redo);
  redoAction->setStatusTip(
    tr("Redo the data changes"));

  connect(
    redoAction
    , SIGNAL(triggered())
    , this
    , SLOT(redo()));

  addPeriodAction = new QAction(tr("&Add Period"), this);
  addPeriodAction->setShortcut(tr("Ctrl+A"));
  addPeriodAction->setStatusTip(
    tr("Add a period to the budget"));
  connect(
    addPeriodAction
    , SIGNAL(triggered())
    , this
    , SLOT(addPeriod()));

  clonePeriodAction = new QAction(tr("&Clone Period"), this);
  clonePeriodAction->setStatusTip(
    tr("Clone the current period"));
  connect(
    clonePeriodAction
    , SIGNAL(triggered())
    , this
    , SLOT(clonePeriod()));

  deletePeriodAction = new QAction(tr("&Delete Period"), this);
  deletePeriodAction->setShortcut(tr("Ctrl+D"));
  deletePeriodAction->setStatusTip(
    tr("Remove a period from the budget"));
  connect(
    deletePeriodAction
    , SIGNAL(triggered())
    , this
    , SLOT(deletePeriod()));

  registerItemAction = new QAction(tr("&Register Item"), this);
  registerItemAction->setShortcut(tr("Ctrl+R"));
  registerItemAction->setStatusTip(
    tr("Register an unused item"));
  connect(
    registerItemAction
    , SIGNAL(triggered())
    , this
    , SLOT(registerItem()));

  unregisterItemAction = new QAction(tr("&Unregister Item"), this);
  unregisterItemAction->setShortcut(tr("Ctrl+U"));
  unregisterItemAction->setStatusTip(
    tr("Remove a registered item"));
  connect(
    unregisterItemAction
    , SIGNAL(triggered())
    , this
    , SLOT(unregisterItem()));
}

void MainForm::createViewActions() {
  toggleShowPeriodAction =
    new QAction(tr("Show/Hide Period Panel"), this);
  toggleShowPeriodAction->setShortcut(tr("Ctrl+Alt+P"));
  toggleShowPeriodAction->setStatusTip(tr("Show or hide the period panel"));
  connect(
    toggleShowPeriodAction
    , SIGNAL(triggered())
    , this
    , SLOT(toggleShowPeriodPanel()));

  toggleShowFlowAction = new QAction(tr("Show/Hide Flow Panel"), this);
  toggleShowFlowAction->setShortcut(tr("Ctrl+Alt+F"));
  toggleShowFlowAction->setStatusTip(tr("Show or hide the flow panel"));
  connect(
    toggleShowFlowAction
    , SIGNAL(triggered())
    , this
    , SLOT(toggleShowFlowPanel()));

  toggleShowCategoryAction = new QAction(tr("Show/Hide Category Panel"), this);
  toggleShowCategoryAction->setShortcut(tr("Ctrl+Alt+C"));
  toggleShowCategoryAction->setStatusTip(tr("Show or hide the category panel"));
  connect(
    toggleShowCategoryAction
    , SIGNAL(triggered())
    , this
    , SLOT(toggleShowCategoryPanel()));

  toggleShowUnusedAction =
    new QAction(tr("Show/Hide Unregistered Item Panel"), this);
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
  editMenu->addAction(unregisterItemAction);

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
  newButton = new QPushButton(tr("New"));
  newButton->setObjectName("newButton");
  newButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  openButton = new QPushButton(tr("Open"));
  openButton->setObjectName("openButton");
  openButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  exitButton = new QPushButton(tr("Exit"));
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
      updateViewsAfterChange();
      periodView->setFocus();
      displayDefaultTitle();

      revertAction->setEnabled(false);
      undoAction->setEnabled(false);
      redoAction->setEnabled(false);
    }
  }
}

void MainForm::open(QString fileName) {
  if (okToContinue()) {
    if (qApp->open(fileName)) {
      deleteFileFormObjects();
      setup();
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

void MainForm::addPeriod() {
  bool isRunningOkay = true;

  // go to the last row in the period view and insert a row after
  int row = periodModel->rowCount();
  periodModel->insertRow(row);

  // edit new period name
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
    periodView->edit(periodNameIndex);
    periodView->setFocus();

    // give the period id field a new id value
    QString periodId = qApp->getNewPeriodId();

    QModelIndex periodIdIndex =
      periodNameIndex.sibling(row, PeriodMetricsView_PeriodId);
    periodModel->setData(periodIdIndex, periodId, Qt::EditRole);
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

  if (isRunningOkay) {
    periodView->setCurrentIndex(periodNameIndex);
    periodView->edit(periodNameIndex);
    periodView->setFocus();

    // give the period id field a new id value
    QString periodId = qApp->getNewPeriodId();

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

    if (isRunningOkay) {
      // clone the data
      qApp->clonePeriodAs(sourcePeriodId, periodId);

      updateViewsAfterChange();
    }
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

  if (unusedModel->rowCount() == 0) {
    isRunningOkay = false;
  }

  QModelIndex unusedModelItemName;
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

  updateViewsAfterChange();

  // set the focus back to the unused item view
  unusedView->setFocus();
  unusedView->setCurrentIndex(unusedModelItemName);
}

void MainForm::unregisterItem() {
  bool isRunningOkay = true;

  QModelIndex index = registerView->currentIndex();
  if (!index.isValid()) {
    QMessageBox::warning(
      (QWidget *)0
      , QObject::tr("Error: Row not valid.")
      , QObject::tr("The register row is not valid."));

    isRunningOkay = false;
  }

  int row;

  if (isRunningOkay) {
    row = index.row();

    if (!index.isValid()) {
      QMessageBox::warning(
        (QWidget *)0
        , QObject::tr("Error: Index not valid.")
        , QObject::tr("The register view index is not valid."));

      isRunningOkay = false;
    }
  }

  if (isRunningOkay) {
    // get register record from the model at given row
    QSqlRecord registerRecord = registerModel->record(row);
    QString itemName = registerRecord.value("itemName").toString();

    // get the period field the current period value
    QModelIndex periodViewIndex = periodView->currentIndex();
    QSqlRecord periodRecord = periodModel->record(periodViewIndex.row());
    QString periodName = periodRecord.value("periodName").toString();

    // show warning if budget or actual values are non-zero (or > 0.01)
    double budget = registerRecord.value("budget").toDouble();
    double actual = registerRecord.value("actual").toDouble();

    if (abs(budget) >= 0.01 || abs(actual) >= 0.01) {
      int r =
        QMessageBox::warning(
          this
          , tr("Unregister Entry")
          , tr("Unregister %1 for Period %2? It contains non-zero values.")
            .arg(itemName)
            .arg(periodName)
          , QMessageBox::Yes | QMessageBox::No);

      if (r == QMessageBox::No) {
        isRunningOkay = false;
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
  periodView->setHorizontalHeader(new HeaderView(Qt::Horizontal, this));

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
  unusedModel = new QSqlRelationalTableModel(this);
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
//  flowLabel->setText(tr("F&low") + modelFilterLabel);

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
  mappingChanged = true;
}

void MainForm::resetMappingChanged() {
  mappingChanged = false;
}

bool MainForm::getMappingChanged() const {
  return mappingChanged;
}

void MainForm::writeSettings() const {
  if (closeAction->isEnabled() == true) {
    QSettings settings("cashflow", "cashflow");
  
    settings.beginGroup("MainForm");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
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

  settings.endGroup();
}

void MainForm::closeEvent(QCloseEvent *event) {
  writeSettings();
  event->accept();
}
