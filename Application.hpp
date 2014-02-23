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
//  Application class definition
//    The class that houses and communicates between the data and main form objects.

#ifndef _CASHFLOW_APPLICATION_HPP_
  #define _CASHFLOW_APPLICATION_HPP_
  
  #include <QApplication>
  #include <QString>
  #include <QScopedPointer>

  #include "Data.hpp"
  #include "MainForm.hpp"

  #if defined(qApp)
    #undef qApp
  #endif
  #define qApp (static_cast<Application*>(QCoreApplication::instance()))

  namespace Cashflow {
    class Application : public QApplication {
    public:
      Application(int &argc, char **argv);
      ~Application();
  
      bool newFile();
      bool open(QString filename = QString());
      bool save();
      bool saveAs();
      bool backupAs();
      bool undo();
      bool redo();
      bool categoryHasItems(QString categoryId);
      bool itemInRegister(QString itemId);
      bool addCurrentFileToRecentList();

      QString connectionName();
      QString internalDatabaseName();
      QString savedDatabaseName();

      QString getInFlowId() const;
      QString getOutFlowId() const;

      QString getNewPeriodId() const;
      QString getNewCategoryId() const;
      QString getNewItemId() const;
      QString getNewRegisterId() const;

      QStringList getRecentFiles() const;

      void clearSavedDatabaseName();

      bool logUndoRedoIndexAtZero();
      bool logUndoRedoIndexAtMax();
      bool logUndoRedoIndexAtSaved();

      void setLogUndoRedoIndexToZero();
      void setLogUndoRedoIndexToMax();
      void incrementLogUndoRedoIndex();
  
      bool logUndoRedoChange();
      bool logUndoRedoClear();

      bool getDataModified() const;
      void setDataModified(bool isDataModified);

      void clonePeriodAs(QString sourcePeriodId, QString periodId);

    private:
      virtual bool notify(QObject *receiver, QEvent *event);
      void resetForm();
      void writeSettings() const;
      void readSettings();

      Cashflow::Data data;
      QScopedPointer<MainForm> form;
      QStringList recentFiles;
      quint16 logUndoRedoIndex;
      quint16 savedLogUndoRedoIndex;
    };
  }
  
#endif // _CASHFLOW_APPLICATION_HPP_
