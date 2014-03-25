//  Copyright 2014 Jason Eric Timms
// 
//  source code license
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
//  database license for save files
//    This cashflow database (in its implemented form and all embedded DDL, DML 
//    and SQL, therein) is made available under the Open Database License: 
//    http://opendatacommons.org/licenses/odbl/1.0/. Any rights in individual 
//    contents of the database are licensed under the Database Contents 
//    License: http://opendatacommons.org/licenses/dbcl/1.0/
//
//  Data class definition
//    This class modifies to the handle the database creation and operations.

#ifndef _CASHFLOW_DATA_HPP_
  #define _CASHFLOW_DATA_HPP_
  
  #include <QFile>
  #include <QProgressDialog>
  #include <QString>
  #include <QTemporaryFile>
  #include <QScopedPointer>
  #include <QSqlDatabase>

  namespace Cashflow {
    class Data : public QObject {
    public:
      Data();
  
      bool newDatabase();
      bool connectToDatabase(QString fileName = QString());
      bool save();
      bool saveAs();
      bool backupAs();
      bool undo(quint16 index) const;
      bool redo(quint16 index) const;
      bool categoryHasItems(QString categoryId);
      bool itemInRegister(QString itemId);

      void clearRegister();
      void clearPeriods();
      void cleanDatabase();
      void clearSavedDatabaseName();

      void fillPeriodWithAllItems(QString);
      void fillAllPeriodsWithAllItems();
      void createFakeData();
      
      QString connectionName();
      QString internalDatabaseName();
      QString savedDatabaseName();

      QString getInFlowId() const;
      QString getOutFlowId() const;

      QString getNewPrimaryKeyId() const;

      quint16 logUndoRedoCount() const;
			quint16 nThLogUndoRedoId(quint16 index) const;

      bool deleteFromLogUndoRedo(quint16 firstIndex, quint16 endIndex);

      bool getDataModified() const;
      void setDataModified(bool isDataModified);

      void clonePeriodAs(QString sourcePeriodId, QString periodId);

    private:
      bool createNewDatabaseFile();
      void createDatabaseStructure();

      bool dropPeriodTable();
      bool createPeriodTable();

      bool dropFlowTable();
      bool createFlowTable();

      bool dropCategoryTable();
      bool createCategoryTable();

      bool dropItemTable();
      bool createItemTable();

      bool createRegisterTable();
      bool dropRegisterTable();

      bool createLogUndoRedoTable();
      bool dropLogUndoRedoTable();

      void prepopulatePermanentData();
      void prepopulateMappableData();
      bool clearEditableData();

      void prepopulateFlowTable(QProgressDialog &, int);
      void prepopulateCategoryTable(QProgressDialog &, int);
      void prepopulateItemTable(QProgressDialog &, int);
      void insertItem(
        QString flowName, QString categoryName, QString itemName);

      bool saveFile(QString);
      bool openFile(QString);
      
      QString uniqueSuffix();
      
      QString savedFileName;
      QScopedPointer<QTemporaryFile> workingDatabaseFile;

      QString inFlowId;
      QString outFlowId;
      
      bool dataModified;
    };
  }
#endif // _CASHFLOW_DATA_HPP_

