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
//  Application class source
//    The class that houses and communicates between the data and main form objects.

#include <QApplication>
#include <QtGui>
#include "Application.hpp"
#include "MainForm.hpp"

using Cashflow::Application;

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
      , savedLogUndoRedoIndex(0) {
  QCoreApplication::setApplicationName("cashflow");

  readSettings();
  resetForm();

  // initialize undo log index
  setLogUndoRedoIndexToMax();
}

Application::~Application() {
  writeSettings();
}

bool Application::notify(QObject *receiver, QEvent *event) {
  try {
    return QApplication::notify(receiver, event);
  }
  catch(std::exception& e) {
    qDebug() << "Exception thrown:" << e.what();
  }

  return false;
}

bool Application::newFile() {
  logUndoRedoClear();
  setLogUndoRedoIndexToZero();

  return data.newDatabase();
}

bool Application::open(QString fileName) {
  bool isRunningOkay = true;

  isRunningOkay = data.connectToDatabase(fileName);

	if (isRunningOkay) {
    setLogUndoRedoIndexToMax();
		savedLogUndoRedoIndex = logUndoRedoIndex;
	}

  return isRunningOkay;
}

bool Application::save() {
  bool isRunningOkay = data.save();

	if (isRunningOkay) {
		savedLogUndoRedoIndex = logUndoRedoIndex;
	}

  return isRunningOkay;
}

bool Application::saveAs() {
  bool isRunningOkay = data.saveAs();

	if (isRunningOkay) {
		savedLogUndoRedoIndex = logUndoRedoIndex;
	}

  return isRunningOkay;
}

bool Application::backupAs() {
  bool isRunningOkay = data.backupAs();

  savedLogUndoRedoIndex = logUndoRedoIndex;

  return isRunningOkay;
}

bool Application::undo() {
  bool isRunningOkay = true;

  if (logUndoRedoIndexAtZero()) {
    isRunningOkay = false;
  }

  if (isRunningOkay) {
    isRunningOkay = data.undo(logUndoRedoIndex);
  }

  if (isRunningOkay) {
    --logUndoRedoIndex;
  }

  return isRunningOkay;
}

bool Application::redo() {
  bool isRunningOkay = true;

  if (logUndoRedoIndexAtMax()) {
    isRunningOkay = false;
  }

  if (isRunningOkay) {
    ++logUndoRedoIndex;
    isRunningOkay = data.redo(logUndoRedoIndex);
  }

  if (!isRunningOkay) {
    --logUndoRedoIndex;
  }

  return isRunningOkay;
}

bool Application::categoryHasItems(QString categoryId) {
  return data.categoryHasItems(categoryId);
}

bool Application::itemInRegister(QString itemId) {
  return data.itemInRegister(itemId);
}

bool Application::addCurrentFileToRecentList() {
  bool isRunningOkay = true;

  QString savedFileName = data.savedDatabaseName();

  isRunningOkay = !savedFileName.isEmpty();

  if (isRunningOkay) {
    recentFiles.removeAll(savedFileName);
    recentFiles.prepend(savedFileName);
  }

  return isRunningOkay;
}

QString Application::connectionName() {
  return data.connectionName();
}

QString Application::internalDatabaseName() {
  return data.internalDatabaseName();
}

QString Application::savedDatabaseName() {
  return data.savedDatabaseName();
}

QString Application::getInFlowId() const {
  return data.getInFlowId();
}

QString Application::getOutFlowId() const {
  return data.getOutFlowId();
}

QString Application::getNewPeriodId() const {
  return data.getNewPrimaryKeyId();
}

QString Application::getNewCategoryId() const {
  return data.getNewPrimaryKeyId();
}

QString Application::getNewItemId() const {
  return data.getNewPrimaryKeyId();
}

QString Application::getNewRegisterId() const {
  return data.getNewPrimaryKeyId();
}

QStringList Application::getRecentFiles() const {
  return recentFiles;
}

void Application::resetForm() {
  form.reset(new MainForm());
	form->resize(500, 600);
	form->setWindowIcon(QIcon(":/images/icon.png"));
	form->show();
}

void Application::writeSettings() const {
  QSettings settings;
  settings.beginGroup("Application");
  settings.setValue("recentFiles", recentFiles);
  settings.endGroup();
}

void Application::readSettings() {
  QSettings settings;
  settings.beginGroup("Application");
  recentFiles = settings.value("recentFiles").toStringList();
  settings.endGroup();
}

void Application::clearSavedDatabaseName() {
  data.clearSavedDatabaseName();
}

bool Application::logUndoRedoIndexAtZero() {
  return logUndoRedoIndex == 0;
}

bool Application::logUndoRedoIndexAtMax() {
  return logUndoRedoIndex == data.logUndoRedoCount();
}

bool Application::logUndoRedoIndexAtSaved() {
  return logUndoRedoIndex == savedLogUndoRedoIndex;
}

void Application::setLogUndoRedoIndexToZero() {
  logUndoRedoIndex = 0;
}

void Application::setLogUndoRedoIndexToMax() {
  logUndoRedoIndex = data.logUndoRedoCount();
}

void Application::incrementLogUndoRedoIndex() {
  ++logUndoRedoIndex;
}

bool Application::logUndoRedoChange() {
  bool isRunningOkay = true;

	// remove all redos current one
  isRunningOkay =
    data.deleteFromLogUndoRedo(
  		logUndoRedoIndex + 1, data.logUndoRedoCount() - 1);

  return isRunningOkay;
}

bool Application::logUndoRedoClear() {
  bool isRunningOkay = true;

	// remove all redos current one
  isRunningOkay =
    data.deleteFromLogUndoRedo(1, data.logUndoRedoCount());

  return isRunningOkay;
}

bool Application::getDataModified() const {
  return data.getDataModified();
}

void Application::setDataModified(bool isDataModified) {
  data.setDataModified(isDataModified);
}

void Application::clonePeriodAs(QString sourcePeriodId, QString periodId) {
  data.clonePeriodAs(sourcePeriodId, periodId);
}
