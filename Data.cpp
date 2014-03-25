//  Copyright 2014 Jason Eric Timms
// 
//  source code licence
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
//  Data class source
//    This class modifies to the handle the database creation and operations.

#include <QtGui>
#include <QtSql>
#include <QDebug>

#include "Data.hpp"
#include "cashflow.hpp"

using Cashflow::Data;

const QString fileTemplate = "cashflow.db";

Data::Data()
    : dataModified(false) {
  bool isRunningOkay = true;

  isRunningOkay = createNewDatabaseFile();

  if (isRunningOkay) {
    QSqlQuery query;
    query.exec(
      "PRAGMA foreign_keys=ON;");
  }
}

bool Data::newDatabase() {
  // set the to-be-saved database file name to an empty string for now
  clearSavedDatabaseName();

  createDatabaseStructure();
  prepopulatePermanentData();
  prepopulateMappableData();

  setDataModified(false);

  return clearEditableData();
}

QString Data::connectionName() {
  QSqlDatabase db = QSqlDatabase::database();
  return db.connectionName();
}

QString Data::internalDatabaseName() {
  QSqlDatabase db = QSqlDatabase::database();
  return db.databaseName();
}

QString Data::savedDatabaseName() {
  return savedFileName;
}

void Data::clearSavedDatabaseName() {
  // set the to-be-saved database file name to an empty string for now
  savedFileName = "";
}

bool Data::createNewDatabaseFile() {
  bool isRunningOkay = true;

  // create the new working file object
  workingDatabaseFile.reset(new QTemporaryFile());

  // name the working file
  workingDatabaseFile->setFileTemplate(
    QDir::homePath() + QDir::toNativeSeparators("/") + fileTemplate);

  // create the working file
  workingDatabaseFile->open();
  workingDatabaseFile->close();

  // create default database connection
  QSqlDatabase db;
  if (QSqlDatabase::connectionNames().isEmpty()) {
	  db = QSqlDatabase::addDatabase("QSQLITE");

  	if (!db.isValid()) {
  		QMessageBox::warning(
  			(QWidget *)0
  			, QObject::tr("Error Type=")
  				+ db.lastError().type()
  				+ " "
  				+ QObject::tr("Could not open new database.")
  			, db.lastError().text());

  		isRunningOkay = false;
  	}
  }

	if (isRunningOkay) {
		// set the working file to be the database file
		db.setDatabaseName(workingDatabaseFile->fileName());

		// open the database
		if (!db.open()) {
			QMessageBox::warning(
				0
				, QObject::tr("Error: Could not create new database.")
				, db.lastError().text());

			isRunningOkay = false;
		}
	}

	if (isRunningOkay && !db.isValid()) {
		QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ db.lastError().type()
				+ " "
				+ QObject::tr("Could not open new database.")
			, db.lastError().text());

		isRunningOkay = false;
	}

  return isRunningOkay;
}

void Data::createDatabaseStructure() {
  QProgressDialog progress(
    QObject::tr("Cashflow")
    , QString()
    , 0
    , 35);

	bool isRunningOkay = true;

  progress.setLabelText(QObject::tr("Create database structure ..."));
  int progressCounter = 0;

  if (isRunningOkay) {
    isRunningOkay &= dropPeriodTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= createPeriodTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= dropFlowTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= createFlowTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= dropCategoryTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= createCategoryTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= dropItemTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= createItemTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= dropRegisterTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= createRegisterTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= dropLogUndoRedoTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    isRunningOkay &= createLogUndoRedoTable();
  	progress.setValue(++progressCounter);
    qApp->processEvents();
  }

  if (isRunningOkay) {
    QSqlQuery query;
    query.exec(
      "create view periodMetricsView as\n"
      "  select\n"
      "    per.id as periodId\n"
      "    , per.name as periodName\n"
      "    , round(coalesce(\n"
      "        sum(\n"
      "          case\n"
      "            when flowName = 'In' then\n"
      "              reg.budget\n"
      "            else\n"
      "              -reg.budget\n"
      "          end)\n"
      "        , 0), 2) as budgetBalance\n"
      "    , round(coalesce(\n"
      "        sum(\n"
      "          case\n"
      "            when flowName = 'In' then\n"
      "              reg.actual\n"
      "            else\n"
      "              -reg.actual\n"
      "          end)\n"
      "        , 0), 2) as actualBalance\n"
      "    , round(coalesce(\n"
      "        sum(\n"
      "          case\n"
      "            when flowName = 'In' then\n"
      "              reg.budget\n"
      "            else\n"
      "              -reg.budget\n"
      "          end)\n"
      "        - sum(\n"
      "          case\n"
      "            when flowName = 'In' then\n"
      "              reg.actual\n"
      "            else\n"
      "              -reg.actual\n"
      "          end)\n"
      "        , 0), 2) as differenceBalance\n"
      "  from\n"
      "    period per\n"
      "    left outer join\n"
      "      (select\n"
      "        reg.periodId\n"
      "        , flo.name as flowName\n"
      "        , reg.budget\n"
      "        , reg.actual\n"
      "      from\n"
      "        register reg\n"
      "        join item ite\n"
      "          on ite.id = reg.itemId\n"
      "        join category cat\n"
      "          on cat.id = ite.categoryId\n"
      "        join flow flo\n"
      "          on flo.id = cat.flowId) reg\n"
      "      on reg.periodId = per.id\n"
      "  group by\n"
      "    per.id\n"
      "    , per.name;\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create view flowMetricsView as\n"
      "  select\n"
      "    reg.periodId\n"
      "    , flo.id as flowId\n"
      "    , per.name as periodName\n"
      "    , flo.name as flowName\n"
      "    , round(sum(coalesce(reg.budget, 0)), 2) as budget\n"
      "    , round(sum(coalesce(reg.actual, 0)), 2) as actual\n"
      "    , round(sum(coalesce(reg.budget, 0) - coalesce(reg.actual, 0)), 2) as difference\n"
      "  from\n"
      "    period per\n"
      "    cross join flow flo\n"
      "    join category cat\n"
      "      on cat.flowId = flo.id\n"
      "    join item ite\n"
      "      on ite.categoryId = cat.id\n"
      "    join register reg\n"
      "      on reg.periodId = per.id\n"
      "      and reg.itemId = ite.id\n"
      "  group by\n"
      "    reg.periodId\n"
      "    , flo.id\n"
      "    , per.name\n"
      "    , flo.name\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create view categoryMetricsView as\n"
      "  select\n"
      "    reg.periodId\n"
      "    , cat.flowId\n"
      "    , cat.id as categoryId\n"
      "    , per.name as periodName\n"
      "    , flo.name as flowName\n"
      "    , cat.name as categoryName\n"
      "    , round(sum(coalesce(reg.budget, 0)), 2) as budget\n"
      "    , round(sum(coalesce(reg.actual, 0)), 2) as actual\n"
      "    , round(sum(coalesce(reg.budget, 0) - coalesce(reg.actual, 0)), 2) as difference\n"
      "  from\n"
      "    period per\n"
      "    cross join flow flo\n"
      "    join category cat\n"
      "      on cat.flowId = flo.id\n"
      "    join item ite\n"
      "      on ite.categoryId = cat.id\n"
      "    join register reg\n"
      "      on reg.periodId = per.id\n"
      "      and reg.itemId = ite.id\n"
      "  group by\n"
      "    reg.periodId\n"
      "    , cat.flowId\n"
      "    , cat.id\n"
      "    , per.name\n"
      "    , flo.name\n"
      "    , cat.name\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create view registerMetricsView as\n"
      "  select\n"
      "    reg.id as registerId\n"
      "    , reg.periodId\n"
      "    , cat.flowId\n"
      "    , ite.categoryId\n"
      "    , ite.id as itemId\n"
      "    , per.name as periodName\n"
      "    , flo.name as flowName\n"
      "    , cat.name as categoryName\n"
      "    , ite.name as itemName\n"
      "    , reg.note as note\n"
      "    , round(sum(coalesce(reg.budget, 0)), 2) as budget\n"
      "    , round(sum(coalesce(reg.actual, 0)), 2) as actual\n"
      "    , round(sum(coalesce(reg.budget, 0) - coalesce(reg.actual, 0)), 2) as difference\n"
      "  from\n"
      "    period per\n"
      "    cross join flow flo\n"
      "    join category cat\n"
      "      on cat.flowId = flo.id\n"
      "    join item ite\n"
      "      on ite.categoryId = cat.id\n"
      "    join register reg\n"
      "      on reg.periodId = per.id\n"
      "      and reg.itemId = ite.id\n"
      "  group by\n"
      "    reg.id\n"
      "    , reg.periodId\n"
      "    , cat.flowId\n"
      "    , ite.categoryId\n"
      "    , ite.id\n"
      "    , per.name\n"
      "    , flo.name\n"
      "    , cat.name\n"
      "    , ite.name\n"
      "    , reg.note\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create view unusedMetricsView as\n"
      "  select\n"
      "    per.id as periodId\n"
      "    , ite.id as itemId\n"
      "    , per.name as periodName\n"
      "    , flo.name as flowName\n"
      "    , cat.name as categoryName\n"
      "    , ite.name as itemName\n"
      "  from\n"
      "    period per\n"
      "    cross join flow flo\n"
      "    join category cat\n"
      "      on cat.flowId = flo.id\n"
      "    join item ite\n"
      "      on ite.categoryId = cat.id\n"
      "    left outer join register reg\n"
      "      on reg.periodId = per.id\n"
      "      and reg.itemId = ite.id\n"
      "  group by\n"
      "    per.id\n"
      "    , ite.id\n"
      "    , per.name\n"
      "    , flo.name\n"
      "    , cat.name\n"
      "    , ite.name\n"
      "    , reg.periodId\n"
      "  having\n"
      "    reg.periodId is null\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create view categoryMapView as\n"
      "  select\n"
      "    cat.flowId\n"
      "    , flo.name as flowName\n"
      "    , cat.id as categoryId\n"
      "    , cat.name as categoryName\n"
      "  from\n"
      "    flow flo\n"
      "    join category cat\n"
      "      on cat.flowId = flo.id\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create view itemMapView as\n"
      "  select\n"
      "    cat.flowId\n"
      "    , flo.name as flowName\n"
      "    , cat.id as categoryId\n"
      "    , cat.name as categoryName\n"
      "    , ite.id as itemId\n"
      "    , ite.name as itemName\n"
      "  from\n"
      "    flow flo\n"
      "    join category cat\n"
      "      on cat.flowId = flo.id\n"
      "    join item ite\n"
      "      on ite.categoryId = cat.id\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create view inCategoryMapView as\n"
      "  select\n"
      "    cat.flowId\n"
      "    , flo.name as flowName\n"
      "    , cat.id as categoryId\n"
      "    , cat.name as categoryName\n"
      "  from\n"
      "    flow flo\n"
      "    join category cat\n"
      "      on cat.flowId = flo.id\n"
      "  where\n"
      "    flo.name = 'In'\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create view outCategoryMapView as\n"
      "  select\n"
      "    cat.flowId\n"
      "    , flo.name as flowName\n"
      "    , cat.id as categoryId\n"
      "    , cat.name as categoryName\n"
      "  from\n"
      "    flow flo\n"
      "    join category cat\n"
      "      on cat.flowId = flo.id\n"
      "  where\n"
      "    flo.name = 'Out'\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create view inItemMapView as\n"
      "  select\n"
      "    cat.flowId\n"
      "    , flo.name as flowName\n"
      "    , cat.id as categoryId\n"
      "    , cat.name as categoryName\n"
      "    , ite.id as itemId\n"
      "    , ite.name as itemName\n"
      "  from\n"
      "    flow flo\n"
      "    join category cat\n"
      "      on cat.flowId = flo.id\n"
      "    join item ite\n"
      "      on ite.categoryId = cat.id\n"
      "  where\n"
      "    flo.name = 'In'\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create view outItemMapView as\n"
      "  select\n"
      "    cat.flowId\n"
      "    , flo.name as flowName\n"
      "    , cat.id as categoryId\n"
      "    , cat.name as categoryName\n"
      "    , ite.id as itemId\n"
      "    , ite.name as itemName\n"
      "  from\n"
      "    flow flo\n"
      "    join category cat\n"
      "      on cat.flowId = flo.id\n"
      "    join item ite\n"
      "      on ite.categoryId = cat.id\n"
      "  where\n"
      "    flo.name = 'Out'\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger periodMetricsViewTrigger_InsteadOfUpdate\n"
      "  instead of\n"
      "  update on periodMetricsView\n"
      "  for each row\n"
      "  begin\n"
      "    update period\n"
      "    set\n"
      "      name = new.periodName\n"
      "    where\n"
      "      id = old.periodId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'update period\n' ||"
      "      'set\n' ||"
      "      '  name = ' || quote(old.periodName) || '\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(old.periodId) || '\n'\n"
      "      , 'update period\n' ||"
      "        'set\n' ||"
      "        '  name = ' || quote(new.periodName) || '\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.periodId) || '\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger periodMetricsViewTrigger_InsteadOfInsert\n"
      "  instead of\n"
      "  insert on periodMetricsView\n"
      "  for each row\n"
      "  begin\n"
      "    insert into period(\n"
      "      id\n"
      "      , name)\n"
      "    values(\n"
      "      new.periodId\n"
      "      , new.periodName);\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'delete\n' ||"
      "      'from\n' ||"
      "      '  period\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(new.periodId) || '\n'\n"
      "      , 'insert into period(\n' ||"
      "        '  id\n' ||"
      "        '  , name)\n' ||"
      "        'values(\n' ||"
      "        '  ' || quote(new.periodId) || '\n' ||"
      "        '  , ' || quote(new.periodName) || ')\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger periodMetricsViewTrigger_InsteadOfDelete\n"
      "  instead of\n"
      "  delete on periodMetricsView\n"
      "  for each row\n"
      "  begin\n"
      "    delete\n"
      "    from\n"
      "      period\n"
      "    where\n"
      "      id = old.periodId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'insert into period(\n' ||"
      "      '  id\n' ||"
      "      '  , name)\n' ||"
      "      'values(\n' ||"
      "      '  ' || quote(old.periodId) || '\n' ||"
      "      '  , ' || quote(old.periodName) || ')\n'\n"
      "      , 'delete\n' ||"
      "        'from\n' ||"
      "        '  period\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.periodId) || '\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger flowMetricsViewTrigger_InsteadOfUpdate\n"
      "  instead of\n"
      "  update on flowMetricsView\n"
      "  for each row\n"
      "  begin\n"
      "    select *\n"
      "    from\n"
      "      flowMetricsView;\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger categoryMetricsViewTrigger_InsteadOfUpdate\n"
      "  instead of\n"
      "  update on categoryMetricsView\n"
      "  for each row\n"
      "  begin\n"
      "    select *\n"
      "    from\n"
      "      categoryMetricsView;\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger registerMetricsViewTrigger_InsteadOfUpdate\n"
      "  instead of\n"
      "  update on registerMetricsView\n"
      "  for each row\n"
      "  begin\n"
      "    update register\n"
      "    set\n"
      "      budget = new.budget\n"
      "      , actual = new.actual\n"
      "      , note = new.note\n"
      "    where\n"
      "      id = old.registerId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'update register\n' ||"
      "      'set\n' ||"
      "      '  budget = ' || quote(old.budget) || '\n' ||"
      "      '  , actual = ' || quote(old.actual) || '\n' ||"
      "      '  , note = ' || quote(old.note) || '\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(old.registerId) || '\n'\n"
      "      , 'update register\n' ||"
      "        'set\n' ||"
      "        '  budget = ' || quote(new.budget) || '\n' ||"
      "        '  , actual = ' || quote(new.actual) || '\n' ||"
      "        '  , note = ' || quote(new.note) || '\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.registerId) || '\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger registerMetricsViewTrigger_InsteadOfInsert\n"
      "  instead of\n"
      "  insert on registerMetricsView\n"
      "  for each row\n"
      "  begin\n"
      "    insert into register(\n"
      "      id\n"
      "      , periodId\n"
      "      , itemId\n"
      "      , budget\n"
      "      , actual\n"
      "      , note)\n"
      "    values(\n"
      "      new.registerId\n"
      "      , new.periodId\n"
      "      , new.itemId\n"
      "      , new.budget\n"
      "      , new.actual\n"
      "      , new.note);\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'delete\n' ||"
      "      'from\n' ||"
      "      '  register\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(new.registerId) || '\n'\n"
      "      , 'insert into register(\n' ||"
      "        '  id\n' ||"
      "        '  , periodId\n' ||"
      "        '  , itemId\n' ||"
      "        '  , budget\n' ||"
      "        '  , actual\n' ||"
      "        '  , note)\n' ||"
      "        'values(\n' ||"
      "        '  ' || quote(new.registerId) || '\n' ||"
      "        '  , ' || quote(new.periodId) || '\n' ||"
      "        '  , ' || quote(new.itemId) || '\n' ||"
      "        '  , ' || quote(new.budget) || '\n' ||"
      "        '  , ' || quote(new.actual) || '\n' ||"
      "        '  , '''')\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger registerMetricsViewTrigger_InsteadOfDelete\n"
      "  instead of\n"
      "  delete on registerMetricsView\n"
      "  for each row\n"
      "  begin\n"
      "    delete\n"
      "    from\n"
      "      register\n"
      "    where\n"
      "      id = old.registerId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'insert into register(\n' ||"
      "      '  id\n' ||"
      "      '  , periodId\n' ||"
      "      '  , itemId\n' ||"
      "      '  , budget\n' ||"
      "      '  , actual\n' ||"
      "      '  , note)\n' ||"
      "      'values(\n' ||"
      "      '  ' || quote(old.registerId) || '\n' ||"
      "      '  , ' || quote(old.periodId) || '\n' ||"
      "      '  , ' || quote(old.itemId) || '\n' ||"
      "      '  , ' || quote(old.budget) || '\n' ||"
      "      '  , ' || quote(old.actual) || '\n' ||"
      "      '  , ' || quote(old.note) || ')\n'\n"
      "      , 'delete\n' ||"
      "        'from\n' ||"
      "        '  register\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.registerId) || '\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger categoryMapViewTrigger_InsteadOfUpdate\n"
      "  instead of\n"
      "  update on categoryMapView\n"
      "  for each row\n"
      "  begin\n"
      "    update category\n"
      "    set\n"
      "      name = new.categoryName\n"
      "      , flowId = new.flowId\n"
      "    where\n"
      "      id = old.categoryId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'update category\n' ||"
      "      'set\n' ||"
      "      '  name = ' || quote(old.categoryName) || '\n' ||"
      "      '  , flowId = ' || quote(old.flowId) || '\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(old.categoryId) || '\n'\n"
      "      , 'update category\n' ||"
      "        'set\n' ||"
      "        '  name = ' || quote(new.categoryName) || '\n' ||"
      "        '  , flowId = ' || quote(new.flowId) || '\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.categoryId) || '\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger categoryMapViewTrigger_InsteadOfInsert\n"
      "  instead of\n"
      "  insert on categoryMapView\n"
      "  for each row\n"
      "  begin\n"
      "    insert into category(\n"
      "      id\n"
      "      , name\n"
      "      , flowId)\n"
      "    values(\n"
      "      new.categoryId\n"
      "      , new.categoryName\n"
      "      , new.flowId);\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'delete\n' ||"
      "      'from\n' ||"
      "      '  category\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(new.categoryId) || '\n'\n"
      "      , 'insert into category(\n' ||"
      "        '  id\n' ||"
      "        '  , name\n' ||"
      "        '  , flowId)\n' ||"
      "        'values(\n' ||"
      "        '  ' || quote(new.categoryId) || '\n' ||"
      "        '  , ' || quote(new.categoryName) || '\n' ||"
      "        '  , ' || quote(new.flowId) || ')\n';\n"
      "  end\n");

  	progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger categoryMapViewTrigger_InsteadOfDelete\n"
      "  instead of\n"
      "  delete on categoryMapView\n"
      "  for each row\n"
      "  begin\n"
      "    delete\n"
      "    from\n"
      "      category\n"
      "    where\n"
      "      id = old.categoryId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'insert into category(\n' ||"
      "      '  id\n' ||"
      "      '  , name\n' ||"
      "      '  , flowId)\n' ||"
      "      'values(\n' ||"
      "      '  ' || quote(old.categoryId) || '\n' ||"
      "      '  , ' || quote(old.categoryName) || '\n' ||"
      "      '  , ' || quote(old.flowId) || ')\n'\n"
      "      , 'delete\n' ||"
      "        'from\n' ||"
      "        '  category\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.categoryId) || '\n';\n"
      "  end\n");

  	progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger inCategoryMapViewTrigger_InsteadOfUpdate\n"
      "  instead of\n"
      "  update on inCategoryMapView\n"
      "  for each row\n"
      "  begin\n"
      "    update category\n"
      "    set\n"
      "      name = new.categoryName\n"
      "      , flowId = new.flowId\n"
      "    where\n"
      "      id = old.categoryId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'update category\n' ||"
      "      'set\n' ||"
      "      '  name = ' || quote(old.categoryName) || '\n' ||"
      "      '  , flowId = ' || quote(old.flowId) || '\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(old.categoryId) || '\n'\n"
      "      , 'update category\n' ||"
      "        'set\n' ||"
      "        '  name = ' || quote(new.categoryName) || '\n' ||"
      "        '  , flowId = ' || quote(new.flowId) || '\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.categoryId) || '\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger inCategoryMapViewTrigger_InsteadOfInsert\n"
      "  instead of\n"
      "  insert on inCategoryMapView\n"
      "  for each row\n"
      "  begin\n"
      "    insert into category(\n"
      "      id\n"
      "      , name\n"
      "      , flowId)\n"
      "    values(\n"
      "      new.categoryId\n"
      "      , new.categoryName\n"
      "      , new.flowId);\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'delete\n' ||"
      "      'from\n' ||"
      "      '  category\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(new.categoryId) || '\n'\n"
      "      , 'insert into category(\n' ||"
      "        '  id\n' ||"
      "        '  , name\n' ||"
      "        '  , flowId)\n' ||"
      "        'values(\n' ||"
      "        '  ' || quote(new.categoryId) || '\n' ||"
      "        '  , ' || quote(new.categoryName) || '\n' ||"
      "        '  , ' || quote(new.flowId) || ')\n';\n"
      "  end\n");

  	progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger inCategoryMapViewTrigger_InsteadOfDelete\n"
      "  instead of\n"
      "  delete on inCategoryMapView\n"
      "  for each row\n"
      "  begin\n"
      "    delete\n"
      "    from\n"
      "      category\n"
      "    where\n"
      "      id = old.categoryId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'insert into category(\n' ||"
      "      '  id\n' ||"
      "      '  , name\n' ||"
      "      '  , flowId)\n' ||"
      "      'values(\n' ||"
      "      '  ' || quote(old.categoryId) || '\n' ||"
      "      '  , ' || quote(old.categoryName) || '\n' ||"
      "      '  , ' || quote(old.flowId) || ')\n'\n"
      "      , 'delete\n' ||"
      "        'from\n' ||"
      "        '  category\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.categoryId) || '\n';\n"
      "  end\n");

  	progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger outCategoryMapViewTrigger_InsteadOfUpdate\n"
      "  instead of\n"
      "  update on outCategoryMapView\n"
      "  for each row\n"
      "  begin\n"
      "    update category\n"
      "    set\n"
      "      name = new.categoryName\n"
      "      , flowId = new.flowId\n"
      "    where\n"
      "      id = old.categoryId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'update category\n' ||"
      "      'set\n' ||"
      "      '  name = ' || quote(old.categoryName) || '\n' ||"
      "      '  , flowId = ' || quote(old.flowId) || '\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(old.categoryId) || '\n'\n"
      "      , 'update category\n' ||"
      "        'set\n' ||"
      "        '  name = ' || quote(new.categoryName) || '\n' ||"
      "        '  , flowId = ' || quote(new.flowId) || '\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.categoryId) || '\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger outCategoryMapViewTrigger_InsteadOfInsert\n"
      "  instead of\n"
      "  insert on outCategoryMapView\n"
      "  for each row\n"
      "  begin\n"
      "    insert into category(\n"
      "      id\n"
      "      , name\n"
      "      , flowId)\n"
      "    values(\n"
      "      new.categoryId\n"
      "      , new.categoryName\n"
      "      , new.flowId);\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'delete\n' ||"
      "      'from\n' ||"
      "      '  category\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(new.categoryId) || '\n'\n"
      "      , 'insert into category(\n' ||"
      "        '  id\n' ||"
      "        '  , name\n' ||"
      "        '  , flowId)\n' ||"
      "        'values(\n' ||"
      "        '  ' || quote(new.categoryId) || '\n' ||"
      "        '  , ' || quote(new.categoryName) || '\n' ||"
      "        '  , ' || quote(new.flowId) || ')\n';\n"
      "  end\n");

  	progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger outCategoryMapViewTrigger_InsteadOfDelete\n"
      "  instead of\n"
      "  delete on outCategoryMapView\n"
      "  for each row\n"
      "  begin\n"
      "    delete\n"
      "    from\n"
      "      category\n"
      "    where\n"
      "      id = old.categoryId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'insert into category(\n' ||"
      "      '  id\n' ||"
      "      '  , name\n' ||"
      "      '  , flowId)\n' ||"
      "      'values(\n' ||"
      "      '  ' || quote(old.categoryId) || '\n' ||"
      "      '  , ' || quote(old.categoryName) || '\n' ||"
      "      '  , ' || quote(old.flowId) || ')\n'\n"
      "      , 'delete\n' ||"
      "        'from\n' ||"
      "        '  category\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.categoryId) || '\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger itemMapViewTrigger_InsteadOfUpdate\n"
      "  instead of\n"
      "  update on itemMapView\n"
      "  for each row\n"
      "  begin\n"
      "    update item\n"
      "    set\n"
      "      name = new.itemName\n"
      "      , categoryId = new.categoryId\n"
      "    where\n"
      "      id = old.itemId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'update item\n' ||"
      "      'set\n' ||"
      "      '  name = ' || quote(old.itemName) || '\n' ||"
      "      '  , categoryId = ' || quote(old.categoryId) || '\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(old.itemId) || '\n'\n"
      "      , 'update item\n' ||"
      "        'set\n' ||"
      "        '  name = ' || quote(new.itemName) || '\n' ||"
      "        '  , categoryId = ' || quote(new.categoryId) || '\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.itemId) || '\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger itemMapViewTrigger_InsteadOfInsert\n"
      "  instead of\n"
      "  insert on itemMapView\n"
      "  for each row\n"
      "  begin\n"
      "    insert into item(\n"
      "      id\n"
      "      , name\n"
      "      , categoryId)\n"
      "    values(\n"
      "      new.itemId\n"
      "      , new.itemName\n"
      "      , new.categoryId);\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'delete\n' ||"
      "      'from\n' ||"
      "      '  item\n' ||"
      "      'where\n' ||"
      "      '  id = ' || quote(new.itemId) || '\n'\n"
      "      , 'insert into item(\n' ||"
      "        '  id\n' ||"
      "        '  , name\n' ||"
      "        '  , categoryId)\n' ||"
      "        'values(\n' ||"
      "        '  ' || quote(new.itemId) || '\n' ||"
      "        '  , ' || quote(new.itemName) || '\n' ||"
      "        '  , ' || quote(new.categoryId) || ')\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    query.exec(
      "create trigger itemMapViewTrigger_InsteadOfDelete\n"
      "  instead of\n"
      "  delete on itemMapView\n"
      "  for each row\n"
      "  begin\n"
      "    delete\n"
      "    from\n"
      "      item\n"
      "    where\n"
      "      id = old.itemId;\n"
      "    insert into logUndoRedo(\n"
      "      undoCommand\n"
      "      , redoCommand)\n"
      "    select\n"
      "      'insert into item(\n' ||"
      "      '  id\n' ||"
      "      '  , name\n' ||"
      "      '  , categoryId)\n' ||"
      "      'values(\n' ||"
      "      '  ' || quote(old.itemId) || '\n' ||"
      "      '  , ' || quote(old.itemName) || '\n' ||"
      "      '  , ' || quote(old.categoryId) || ')\n'\n"
      "      , 'delete\n' ||"
      "        'from\n' ||"
      "        '  item\n' ||"
      "        'where\n' ||"
      "        '  id = ' || quote(old.itemId) || '\n';\n"
      "  end\n");

    progress.setValue(++progressCounter);
    qApp->processEvents();

    progress.setValue(progress.maximum());
    qApp->processEvents();
  }
}

bool Data::dropPeriodTable() {
	bool isRunningOkay = true;

	QSqlQuery query;
	query.prepare("drop table if exists period");
	query.exec();

	if (isRunningOkay
			&& !query.isActive()) {
		QString message = "Invalid drop of period table.";
		QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ query.lastError().type()
				+ " "
				+ QObject::tr(message.toUtf8())
			, ATLINE + ":" + query.lastError().text());

		isRunningOkay = false;
	}

  return isRunningOkay;
}

bool Data::createPeriodTable() {
	bool isRunningOkay = true;

	if (isRunningOkay) {
  	QSqlQuery query;
  	query.exec(
      "create table period(\n"
      "  id uuid primary key not null\n"
      "  , name varchar(40) not null)\n");

    if (!query.isActive()) {
  		QString message = "Invalid create of period table.";
  		QMessageBox::warning(
  			(QWidget *)0
  			, QObject::tr("Error Type=")
  				+ query.lastError().type()
  				+ " "
  				+ QObject::tr(message.toUtf8())
  			, ATLINE + ":" + query.lastError().text());
  
  		isRunningOkay = false;
  	}
  }

  return isRunningOkay;
}

bool Data::dropFlowTable() {
	bool isRunningOkay = true;

	QSqlQuery query;
	query.prepare("drop table if exists flow");
	query.exec();

	if (isRunningOkay
			&& !query.isActive()) {
		QString message = "Invalid drop of flow table.";
		QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ query.lastError().type()
				+ " "
				+ QObject::tr(message.toUtf8())
			, ATLINE + ":" + query.lastError().text());

		isRunningOkay = false;
	}

  return isRunningOkay;
}

bool Data::createFlowTable() {
	bool isRunningOkay = true;

	if (isRunningOkay) {
  	QSqlQuery query;
    query.exec(
      "create table flow(\n"
      "  id uuid primary key\n"
      "  , name varchar(40) not null)\n");

    if (!query.isActive()) {
  		QString message = "Invalid create of flow table.";
  		QMessageBox::warning(
  			(QWidget *)0
  			, QObject::tr("Error Type=")
  				+ query.lastError().type()
  				+ " "
  				+ QObject::tr(message.toUtf8())
  			, ATLINE + ":" + query.lastError().text());
  
  		isRunningOkay = false;
  	}
  }

  return isRunningOkay;
}

bool Data::dropCategoryTable() {
	bool isRunningOkay = true;

	QSqlQuery query;
	query.prepare("drop table if exists category");
	query.exec();

	if (isRunningOkay
			&& !query.isActive()) {
		QString message = "Invalid drop of category table.";
		QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ query.lastError().type()
				+ " "
				+ QObject::tr(message.toUtf8())
			, ATLINE + ":" + query.lastError().text());

		isRunningOkay = false;
	}

  return isRunningOkay;
}

bool Data::createCategoryTable() {
	bool isRunningOkay = true;

	if (isRunningOkay) {
  	QSqlQuery query;
    query.exec(
      "create table category(\n"
      "  id uuid primary key\n"
      "  , name varchar(40) not null\n"
      "  , flowId uuid not null\n"
      "  , foreign key (flowId) references flow(id)\n"
      "    on delete cascade)\n");

    if (!query.isActive()) {
  		QString message = "Invalid create of category table.";
  		QMessageBox::warning(
  			(QWidget *)0
  			, QObject::tr("Error Type=")
  				+ query.lastError().type()
  				+ " "
  				+ QObject::tr(message.toUtf8())
  			, ATLINE + ":" + query.lastError().text());
  
  		isRunningOkay = false;
  	}
  }

  return isRunningOkay;
}

bool Data::dropItemTable() {
	bool isRunningOkay = true;

	QSqlQuery query;
	query.prepare("drop table if exists item");
	query.exec();

	if (isRunningOkay
			&& !query.isActive()) {
		QString message = "Invalid drop of item table.";
		QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ query.lastError().type()
				+ " "
				+ QObject::tr(message.toUtf8())
			, ATLINE + ":" + query.lastError().text());

		isRunningOkay = false;
	}

  return isRunningOkay;
}

bool Data::createItemTable() {
	bool isRunningOkay = true;

	if (isRunningOkay) {
  	QSqlQuery query;
    query.exec(
      "create table item(\n"
      "  id uuid primary key\n"
      "  , name varchar(40) not null\n"
      "  , categoryId uuid not null\n"
      "  , foreign key (categoryId) references category(id)\n"
      "    on delete cascade)\n");

    if (!query.isActive()) {
  		QString message = "Invalid create of item table.";
  		QMessageBox::warning(
  			(QWidget *)0
  			, QObject::tr("Error Type=")
  				+ query.lastError().type()
  				+ " "
  				+ QObject::tr(message.toUtf8())
  			, ATLINE + ":" + query.lastError().text());
  
  		isRunningOkay = false;
  	}
  }

  return isRunningOkay;
}

bool Data::dropRegisterTable() {
	bool isRunningOkay = true;

	QSqlQuery query;
	query.prepare("drop table if exists register");
	query.exec();

	if (isRunningOkay
			&& !query.isActive()) {
		QString message = "Invalid drop of register table.";
		QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ query.lastError().type()
				+ " "
				+ QObject::tr(message.toUtf8())
			, ATLINE + ":" + query.lastError().text());

		isRunningOkay = false;
	}

  return isRunningOkay;
}

bool Data::createRegisterTable() {
	bool isRunningOkay = true;

	if (isRunningOkay) {
  	QSqlQuery query;
    query.exec(
      "create table register(\n"
      "  id uuid primary key\n"
      "  , periodId uuid not null\n"
      "  , itemId uuid not null\n"
      "  , budget real not null\n"
      "  , actual real not null\n"
      "  , note text not null default ''\n"
      "  , foreign key (periodId) references period(id)\n"
      "    on delete cascade\n"
      "  , foreign key (itemId) references item(id)\n"
      "    on delete cascade)\n");

    if (!query.isActive()) {
  		QString message = "Invalid create of register table.";
  		QMessageBox::warning(
  			(QWidget *)0
  			, QObject::tr("Error Type=")
  				+ query.lastError().type()
  				+ " "
  				+ QObject::tr(message.toUtf8())
  			, ATLINE + ":" + query.lastError().text());
  
  		isRunningOkay = false;
  	}
  }

  return isRunningOkay;
}

bool Data::dropLogUndoRedoTable() {
	bool isRunningOkay = true;

	QSqlQuery query;
	query.prepare("drop table if exists logUndoRedo");
	query.exec();

	if (isRunningOkay
			&& !query.isActive()) {
		QString message = "Invalid drop of logUndoRedo table.";
		QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ query.lastError().type()
				+ " "
				+ QObject::tr(message.toUtf8())
			, ATLINE + ":" + query.lastError().text());

		isRunningOkay = false;
	}

  return isRunningOkay;
}

bool Data::createLogUndoRedoTable() {
	bool isRunningOkay = true;

	if (isRunningOkay) {
  	QSqlQuery query;
    query.exec(
      "create table logUndoRedo(\n"
      "  id integer primary key\n"
      "  , undoCommand text not null\n"
      "  , redoCommand text not null)\n");

    if (!query.isActive()) {
  		QString message = "Invalid create of logUndoRedo table.";
  		QMessageBox::warning(
  			(QWidget *)0
  			, QObject::tr("Error Type=")
  				+ query.lastError().type()
  				+ " "
  				+ QObject::tr(message.toUtf8())
  			, ATLINE + ":" + query.lastError().text());
  
  		isRunningOkay = false;
  	}
  }

  return isRunningOkay;
}

void Data::prepopulateFlowTable(
     QProgressDialog &progress
    , int progressCounter) {

  // in flow
  inFlowId = getNewPrimaryKeyId();

  QString inInsertText =
    "insert into flow(\n"
    "  id\n"
    "  , name)\n"
    "values(\n"
    "  '" + inFlowId + "'\n"
    "  , 'In')\n";

  QSqlQuery query;
  query.exec(inInsertText);

  progress.setValue(++progressCounter);
  qApp->processEvents();

  // out flow
  outFlowId = getNewPrimaryKeyId();

  QString outInsertText =
    "insert into flow(\n"
    "  id\n"
    "  , name)\n"
    "values(\n"
    "  '" + outFlowId + "'\n"
    "  , 'Out')\n";

  query.exec(outInsertText);

  progress.setValue(++progressCounter);
  qApp->processEvents();
}

void Data::prepopulateCategoryTable(
     QProgressDialog &progress
    , int progressCounter) {

  // initialize default in flow (income) categories
  QStringList inFlowCategories;
  inFlowCategories
    << "Gift" << "Property Sales" << "Rebate" << "Refund" << "Reimbursement"
    << "Salary" << "Savings";

  foreach(QString inFlowCategory, inFlowCategories) {
    QString insertText =
      QString(
        "insert into category(\n"
        "  id\n"
        "  , name\n"
        "  , flowId)\n"
        "values(\n"
        "  '%1'\n"
        "  , '%2'\n"
        "  , '%3')\n")
        .arg(QUuid::createUuid().toString())
        .arg(inFlowCategory)
        .arg(inFlowId);

    QSqlQuery query(insertText);
  }

  progress.setValue(++progressCounter);
  qApp->processEvents();

  // initialize default out flow (expense) categories
  QStringList outFlowCategories;
  outFlowCategories
    << "Business" << "Charitable" << "Clothing" << "Debt" << "Food" << "Health"
    << "Housing" << "Insurance" << "Personal" << "Recreation" << "Savings"
    << "Taxes" << "Transportation" << "Travel" << "Utilities";

  foreach(QString outFlowCategory, outFlowCategories) {
    QString insertText =
      QString(
        "insert into category(\n"
        "  id\n"
        "  , name\n"
        "  , flowId)\n"
        "values(\n"
        "  '%1'\n"
        "  , '%2'\n"
        "  , '%3')\n")
        .arg(QUuid::createUuid().toString())
        .arg(outFlowCategory)
        .arg(outFlowId);

    QSqlQuery query(insertText);
  }

  progress.setValue(++progressCounter);
  qApp->processEvents();
}

void Data::prepopulateItemTable(
     QProgressDialog &progress
    , int progressCounter) {
  QSqlQuery query;

  // initialize default items
  insertItem("In", "Gift", "Anniversary Gift");
  insertItem("In", "Gift", "Birthday Gift");
  insertItem("In", "Gift", "Christmas Gift");
  insertItem("In", "Gift", "Easter Gift");
  insertItem("In", "Gift", "Fathers Day Gift");
  insertItem("In", "Gift", "Mothers Day Gift");
  insertItem("In", "Gift", "Holiday Gift");
  insertItem("In", "Gift", "Congratulations Gift");
  insertItem("In", "Property Sales", "Misc. Property Resale");
  insertItem("In", "Rebate", "Misc. Rebate");
  insertItem("In", "Refund", "Misc. Refund");
  insertItem("In", "Reimbursement", "Misc. Reimbursement");
  insertItem("In", "Salary", "Misc. Gross Pay");
  insertItem("In", "Salary", "Misc. Take Home Pay");
  insertItem("In", "Savings", "Appliance Fund");
  insertItem("In", "Savings", "Elective Surgery Fund");
  insertItem("In", "Savings", "Emergency Fund");
  insertItem("In", "Savings", "Furniture Fund");
  insertItem("In", "Savings", "Gift Fund");
  insertItem("In", "Savings", "Home Insurance Fund");
  insertItem("In", "Savings", "Home Repair Fund");
  insertItem("In", "Savings", "New Home Fund");
  insertItem("In", "Savings", "Next Month's Rent Fund");
  insertItem("In", "Savings", "Retirement Fund");
  insertItem("In", "Savings", "Spending Surplus Fund");
  insertItem("In", "Savings", "Vacation Fund");
  insertItem("In", "Savings", "Vehicle Insurance Fund");
  insertItem("In", "Savings", "Vehicle Maintenance Fund");
  insertItem("In", "Savings", "Vehicle Purchase Fund");
  insertItem("In", "Savings", "Vet Fund");
  insertItem("Out", "Food", "Dining Out");
  insertItem("Out", "Food", "Food Bill");
  insertItem("Out", "Food", "Grocery");
  insertItem("Out", "Utilities", "Electricity");
  insertItem("Out", "Utilities", "Gas");
  insertItem("Out", "Utilities", "Internet");
  insertItem("Out", "Utilities", "Phone");
  insertItem("Out", "Utilities", "TV");
  insertItem("Out", "Utilities", "Water");
  insertItem("Out", "Clothing", "Clothes");
  insertItem("Out", "Clothing", "Laundry And Tailoring");
  insertItem("Out", "Clothing", "Cleaning");
  insertItem("Out", "Housing", "Furniture");
  insertItem("Out", "Housing", "Kitchenware");
  insertItem("Out", "Housing", "Mortgage");
  insertItem("Out", "Housing", "Real-Estate Taxes");
  insertItem("Out", "Housing", "Rent");
  insertItem("Out", "Housing", "Renters Insurance");
  insertItem("Out", "Housing", "Repairs And Maintenance");
  insertItem("Out", "Housing", "Safe Deposit Box");
  insertItem("Out", "Housing", "Storage");
  insertItem("Out", "Health", "Allergist");
  insertItem("Out", "Health", "Chiropractor");
  insertItem("Out", "Health", "Dentist");
  insertItem("Out", "Health", "Doctor");
  insertItem("Out", "Health", "Eye Care");
  insertItem("Out", "Health", "Hospital");
  insertItem("Out", "Health", "Medicine");
  insertItem("Out", "Health", "Spa");
  insertItem("Out", "Transportation", "Auto Parts");
  insertItem("Out", "Transportation", "Bus Fair");
  insertItem("Out", "Transportation", "Car Wash");
  insertItem("Out", "Transportation", "Gas And Oil");
  insertItem("Out", "Transportation", "License And Registration");
  insertItem("Out", "Transportation", "Repairs And Tires");
  insertItem("Out", "Transportation", "Vehicle History");
  insertItem("Out", "Taxes", "Federal Income Tax");
  insertItem("Out", "Taxes", "Federal Medicare Tax");
  insertItem("Out", "Taxes", "Federal Oasdi Social Security");
  insertItem("Out", "Taxes", "Income Tax Preparation");
  insertItem("Out", "Taxes", "Property Tax");
  insertItem("Out", "Taxes", "State Income Tax");
  insertItem("Out", "Taxes", "Vehicle Tax");
  insertItem("Out", "Debt", "Misc");
  insertItem("Out", "Insurance", "Car Insurance");
  insertItem("Out", "Insurance", "Dental Insurance");
  insertItem("Out", "Insurance", "Disability Insurance");
  insertItem("Out", "Insurance", "Homeowners Insurance");
  insertItem("Out", "Insurance", "Medical Insurance");
  insertItem("Out", "Insurance", "Renters Insurance");
  insertItem("Out", "Insurance", "Term Life Insurance");
  insertItem("Out", "Insurance", "Travel Insurance");
  insertItem("Out", "Insurance", "Vision Insurance");
  insertItem("Out", "Savings", "401K");
  insertItem("Out", "Savings", "Auto Maintenance Fund");
  insertItem("Out", "Savings", "Car Insurance Payment Fund");
  insertItem("Out", "Savings", "Education");
  insertItem("Out", "Savings", "Elective Surgery");
  insertItem("Out", "Savings", "Emergency Fund");
  insertItem("Out", "Savings", "Flex Spending Account");
  insertItem("Out", "Savings", "Furniture Fund");
  insertItem("Out", "Savings", "Gift Fund");
  insertItem("Out", "Savings", "New Home Fund Standby");
  insertItem("Out", "Savings", "New Home Fund");
  insertItem("Out", "Savings", "Next Month's Rent");
  insertItem("Out", "Savings", "Retirement Fund Standby");
  insertItem("Out", "Savings", "Rothira");
  insertItem("Out", "Savings", "Spending Surplus");
  insertItem("Out", "Savings", "Used Vehicle Fund");
  insertItem("Out", "Savings", "Vacation");
  insertItem("Out", "Savings", "Vet Fund");
  insertItem("Out", "Charitible", "Donations Or Tithe");
  insertItem("Out", "Charitible", "Gifts");
  insertItem("Out", "Personal", "Cash");
  insertItem("Out", "Personal", "Convention");
  insertItem("Out", "Personal", "Firearm");
  insertItem("Out", "Personal", "Education");
  insertItem("Out", "Personal", "Gifts");
  insertItem("Out", "Personal", "Hair Care");
  insertItem("Out", "Personal", "Organization Dues");
  insertItem("Out", "Personal", "Pet Care");
  insertItem("Out", "Personal", "Sales");
  insertItem("Out", "Personal", "Shipping");
  insertItem("Out", "Personal", "Stationary");
  insertItem("Out", "Personal", "Subscriptions");
  insertItem("Out", "Personal", "Toiletries");
  insertItem("Out", "Personal", "Veterinarian");
  insertItem("Out", "Business", "Repairs And Maintenance");
  insertItem("Out", "Business", "Shipping");
  insertItem("Out", "Recreation", "Blow");
  insertItem("Out", "Recreation", "Books");
  insertItem("Out", "Recreation", "Computer");
  insertItem("Out", "Recreation", "Electronics");
  insertItem("Out", "Recreation", "Entertainment");
  insertItem("Out", "Recreation", "Games");
  insertItem("Out", "Recreation", "Movies");
  insertItem("Out", "Recreation", "Sports");
  insertItem("Out", "Travel", "Cleaning");
  insertItem("Out", "Travel", "Dining");
  insertItem("Out", "Travel", "Transportation");
  insertItem("Out", "Travel", "Groceries");
  insertItem("Out", "Travel", "Entertainment");
  insertItem("Out", "Travel", "Lodging");
  insertItem("Out", "Travel", "Tips");

  progress.setValue(++progressCounter);
  qApp->processEvents();
}

void Data::insertItem(
    QString flowName, QString categoryName, QString itemName) {
  QString insertText(QString(
    "insert into item(\n"
    "  id\n"
    "  , name\n"
    "  , categoryId)\n"
    "select\n"
    "  '%1'\n"
    "  , '%2'\n"
    "  , cat.id\n"
    "from\n"
    "  flow flo\n"
    "  join category cat\n"
    "    on cat.flowId = flo.id\n"
    "where\n"
    "  flo.name = '%3'\n"
    "  and cat.name = '%4'\n")
    .arg(QUuid::createUuid().toString())
    .arg(itemName)
    .arg(flowName)
    .arg(categoryName));

  QSqlQuery query(insertText);
}

//void Data::deleteItem(
//    QString flowName, QString categoryName, QString itemName) {
//  QString insertText(QString(
//    "delete ite(\n"
//    "from\n"
//    "  flow flo\n"
//    "  join category cat\n"
//    "    on cat.flowId = flo.id\n"
//    "  join item ite\n"
//    "    on ite.categoryId = cat.id\n"
//    "where\n"
//    "  flo.name = '%1'\n"
//    "  and cat.name = '%2'\n"
//    "  and ite.name = '%3'\n")
//    .arg(flowName)
//    .arg(categoryName)
//    .arg(itemName));
//
//  QSqlQuery query(insertText);
//}

void Data::prepopulatePermanentData() {
  QProgressDialog progress(QObject::tr("Cashflow"), QString(), 0, 2);

  progress.setLabelText(QObject::tr("Pre-populate permanent data ..."));
  int progressCounter = 0;

  prepopulateFlowTable(progress, progressCounter);

  progress.setValue(progress.maximum());
  qApp->processEvents();
}

void Data::prepopulateMappableData() {
  QProgressDialog progress(QObject::tr("Cashflow"), QString(), 0, 5);

  progress.setLabelText(QObject::tr("Pre-populate mappable data ..."));
  int progressCounter = 0;

  prepopulateCategoryTable(progress, progressCounter);
  prepopulateItemTable(progress, progressCounter);

  progress.setValue(progress.maximum());
  qApp->processEvents();
}

bool Data::clearEditableData() {
  bool isRunningOkay = true;

  QSqlQuery query;
  query.exec(
    "delete\n"
    "from\n"
    "  period\n");

  if (isRunningOkay
      && query.lastError().isValid()) {
    QString message = "Invalid clear of period records.";
    QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ query.lastError().type()
				+ " "
				+ QObject::tr(message.toUtf8())
      , ATLINE + ":" + query.lastError().text());

    isRunningOkay = false;
  }

  if (isRunningOkay) {
    query.exec(
      "delete\n"
      "from\n"
      "  register\n");
  }

  if (isRunningOkay
      && query.lastError().isValid()) {
    QString message = "Invalid clear of register records.";
    QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ query.lastError().type()
				+ " "
				+ QObject::tr(message.toUtf8())
      , ATLINE + ":" + query.lastError().text());

    isRunningOkay = false;
  }

  return isRunningOkay;
}


void Data::fillPeriodWithAllItems(QString period) {
  QSqlQuery query;

  // add all items for each period
  query.exec(
    "insert into register(\n"
    "  periodId\n"
    "  , itemId\n"
    "  , budget\n"
    "  , actual\n"
    "  , note)\n"
    "select\n"
    "  per.id\n"
    "  , ite.id\n"
    "  , 0.00\n"
    "  , 0.00\n"
    "  , ''\n"
    "from\n"
    "  period per\n"
    "  cross join item ite\n"
    "where\n"
    "  per.periodId = '" + period + "';\n");

  if (!query.isValid()) {
    QString message = "Invalid query.";
    QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ query.lastError().type()
				+ " "
				+ QObject::tr(message.toUtf8())
      , ATLINE + ":" + query.lastError().text());
  }
}

void Data::fillAllPeriodsWithAllItems() {
  QSqlQuery query;

  // add all items for each period
  query.exec(
    "insert into register(\n"
    "  periodId\n"
    "  , itemId\n"
    "  , budget\n"
    "  , actual)\n"
    "select\n"
    "  per.id\n"
    "  , ite.id\n"
    "  , 0.00\n"
    "  , 0.00\n"
    "from\n"
    "  period per\n"
    "  cross join item ite;\n");

  if (!query.isValid()) {
    QString message = "Invalid query.";
    QMessageBox::warning(
			(QWidget *)0
			, QObject::tr("Error Type=")
				+ query.lastError().type()
				+ " "
				+ QObject::tr(message.toUtf8())
      , ATLINE + ":" + query.lastError().text());
  }
}

bool Data::connectToDatabase(QString fileName) {
  bool isRunningOkay = true;

  if (fileName.isEmpty()) {
    // prompt for the file name to open
		fileName =
      QFileDialog::getOpenFileName(
        (QWidget *)0
  			, tr("Connect")
        , savedFileName
        , tr("SQLite Database files (*.db *.dat *.cashflow)"));

    if (fileName.isEmpty()) {
      isRunningOkay = false;
    }
  }

  if (isRunningOkay) {
    isRunningOkay = openFile(fileName);
  }

  return isRunningOkay;
}

bool Data::openFile(QString openFileName) {
  bool isRunningOkay = true;

  QString connectionName;

  QSqlDatabase db = QSqlDatabase::database();

  // open a block here to allow the removal of the default connection
  {
    if (!db.isValid()) {
      QMessageBox::warning(
  			(QWidget *)0
  			, QObject::tr("Error Type=")
  				+ db.lastError().type()
  				+ " "
  				+ QObject::tr("Invalid database.")
        , db.lastError().text());

      isRunningOkay = false;
    }

    connectionName = db.connectionName();

  	// close the current database connection
    if (isRunningOkay) {
  		db.close();

  		if (!db.isValid()) {
  			QMessageBox::warning(
  				(QWidget *)0
  				, QObject::tr("Error Type=")
  					+ db.lastError().type()
  					+ " "
  					+ QObject::tr("Could not close the current database.")
  				, db.lastError().text());

  			isRunningOkay = false;
  		}
  	}
  }

	// close the current database connection
//	QSqlDatabase::removeDatabase(connectionName);

  // copy the opened file over the working one
  QString workingDatabaseFileName = "";

	if (isRunningOkay) {
    // grab the file name
    workingDatabaseFileName = workingDatabaseFile->fileName();

		// release the current file and grab a new one, temporarily
    workingDatabaseFile.reset(new QTemporaryFile());

    // copy the open file over the working file
    if (!QFile::copy(openFileName, workingDatabaseFileName)) {
      QMessageBox::warning(
				(QWidget *)0
				, QObject::tr("Error: File not opened.")
				, QObject::tr("The file could not be opened."));

      isRunningOkay = false;
    }
  }

  // open a new database connection
	if (isRunningOkay) {
//		db = QSqlDatabase::addDatabase("QSQLITE");
//
//		if (!db.isValid()) {
//			QMessageBox::warning(
//				(QWidget *)0
//				, QObject::tr("Error Type=")
//					+ db.lastError().type()
//					+ " "
//					+ QObject::tr("Could not create new database connection.")
//				, db.lastError().text());
//
//			isRunningOkay = false;
//		}
  }

	if (isRunningOkay) {
  	// rename the file to what it was
  	workingDatabaseFile->setFileName(workingDatabaseFileName);

  	// reopen the working database
		db.setDatabaseName(workingDatabaseFile->fileName());

		if (!db.open()) {
			QMessageBox::warning(
				(QWidget *)0
				, QObject::tr("Could not open existing database.")
				, QObject::tr("Error Type=")
					+ db.lastError().type()
					+ " "
				  + db.lastError().text());

			isRunningOkay = false;
		}
  }

  if (isRunningOkay) {
    // set the inFlowId value to the loaded one
    QSqlQuery query(
      "select\n"
      "  id\n"
      "from\n"
      "  flow\n"
      "where\n"
      "  name = 'In'\n");

    if (query.next()) {
      inFlowId = query.value(0).toString();
    } else {
			QMessageBox::warning(
				(QWidget *)0
				, QObject::tr("Could not get loaded In flow id.")
				, QObject::tr("The In flow id from the opened file is missing."));

			isRunningOkay = false;
		}
  }

  if (isRunningOkay) {
    // set the outFlowId value to the loaded one
    QSqlQuery query(
      "select\n"
      "  id\n"
      "from\n"
      "  flow\n"
      "where\n"
      "  name = 'Out'\n");

    if (query.next()) {
      outFlowId = query.value(0).toString();
    } else {
			QMessageBox::warning(
				(QWidget *)0
				, QObject::tr("Could not get loaded Out flow id.")
				, QObject::tr("The Out flow id from the opened file is missing."));

			isRunningOkay = false;
		}
  }

  if (isRunningOkay) {
    // set the current file name to the opened one
    savedFileName = openFileName;

    setDataModified(false);
  }

  return isRunningOkay;
}

bool Data::save() {
  bool isRunningOkay = true;

  if (savedFileName.isEmpty()) {
    // perform a save-as operation if still using the working file
    isRunningOkay = saveAs();
  } else {
    // save the current file
    isRunningOkay = saveFile(savedFileName);
  }

  return isRunningOkay;
}

bool Data::saveAs() {
  bool isRunningOkay = true;

  // prompt for the new file name
  QString newFileName =
    QFileDialog::getSaveFileName(
      (QWidget *)0
      , tr("Save As")
      , QDir::homePath() + QDir::toNativeSeparators("/untitled.cashflow")
      , tr("SQLite Database files (*.db *.dat *.cashflow)"));

  isRunningOkay = saveFile(newFileName);

  if (isRunningOkay) {
    // if the saved work, store the last saved file name
    savedFileName = newFileName;
  }

  return isRunningOkay;
}

bool Data::backupAs() {
  bool isRunningOkay = true;

  // prompt for the new file name
  QString newFileName =
    QFileDialog::getSaveFileName(
      (QWidget *)0
      , tr("Clone As")
      , QDir::homePath() + QDir::toNativeSeparators("/untitled.cashflow")
      , tr("SQLite Database files (*.db *.dat *.cashflow)"));

  // attempt a file save operation
  isRunningOkay = saveFile(newFileName);

  return isRunningOkay;
}

bool Data::saveFile(QString saveFileName) {
  bool isRunningOkay = true;

  // clean up old, unused space in database
  cleanDatabase();

  // create fail-safe backup in case saveFileName exists
  QString failSafeFileName = saveFileName + "." + uniqueSuffix();

  // copy temporary backup if saveFileName exists, in case of failure
  if (isRunningOkay
      && QFile::exists(saveFileName)
      && !QFile::copy(saveFileName, failSafeFileName)) {
    isRunningOkay = false;
  }

  // remove the current file so that copy will save over
  if (isRunningOkay
      && QFile::exists(saveFileName)
      && !QFile::remove(saveFileName)) {
    isRunningOkay = false;
  }

  // copy the working database file as the save file name
  if (isRunningOkay
      && !QFile::exists(saveFileName)
      && workingDatabaseFile->exists()
      && !workingDatabaseFile->copy(saveFileName)) {
    isRunningOkay = false;
  }

  // delete fail-safe backup if it and save file name exists
  if (isRunningOkay
      && QFile::exists(saveFileName)
      && QFile::exists(failSafeFileName)
      && !QFile::remove(failSafeFileName)) {
    isRunningOkay = false;
  }

  if (!isRunningOkay) {
    isRunningOkay = false;
  }

  if (isRunningOkay) {
    setDataModified(false);
  }

  return isRunningOkay;
}

void Data::cleanDatabase() {
  QSqlQuery query;

  // clean up the database
  query.exec("vacuum;\n");

  // the following query validation step always returns false for vacuum
//  if (!query.isValid()) {
//    QString message = "Invalid query.";
//    QMessageBox::warning(
//			(QWidget *)0
//			, QObject::tr("Error Type=")
//				+ query.lastError().type()
//				+ " "
//				+ QObject::tr(message.toUtf8())
//      , ATLINE + ":" + query.lastError().text());
//  }
}

QString Data::uniqueSuffix() {
  QString dateTime = QString::number(QDateTime::currentMSecsSinceEpoch());

  dateTime.replace(":", "_");
  dateTime.replace("+", "p");

  return dateTime;
}

bool Data::categoryHasItems(QString categoryId) {
  QSqlQuery query(QString(
    "select\n"
    "  count(*)\n"
    "from\n"
    "  item\n"
    "where\n"
    "  categoryId = %1")
      .arg(categoryId));

  int itemCount = 0;

  if (query.next()) {
    itemCount = query.value(0).toInt();
  }

  return itemCount > 0;
}

bool Data::itemInRegister(QString itemId) {
  QSqlQuery query(QString(
    "select\n"
    "  count(*)\n"
    "from\n"
    "  register\n"
    "where\n"
    "  itemId = %1")
      .arg(itemId));

  int registeredItemCount = 0;

  if (query.next()) {
    registeredItemCount = query.value(0).toInt();
  }

  return registeredItemCount > 0;
}

QString Data::getInFlowId() const {
  return inFlowId;
}

QString Data::getOutFlowId() const {
  return outFlowId;
}

QString Data::getNewPrimaryKeyId() const {
  return QUuid::createUuid().toString();
}

quint16 Data::logUndoRedoCount() const {
  QSqlQuery query(QString(
    "select\n"
    "  count(*)\n"
    "from\n"
    "  logUndoRedo\n"));

  int count = 0;

  if (query.next()) {
    count = query.value(0).toInt();
  }

  return count;
}

bool Data::undo(quint16 index) const {
  bool isRunningOkay = true;

  // get the undo SQL
  QSqlQuery query(QString(
    "select\n"
    "  undoCommand\n"
    "from\n"
    "  logUndoRedo\n"
    "where\n"
    "  id = %1\n")
		.arg(nThLogUndoRedoId(index)));

  QString undo = "";

  if (query.next()) {
    undo = query.value(0).toString();
  } else {
    isRunningOkay = false;
  }

  // run the undo SQL
  if (isRunningOkay) {
    QStringList undoStringList = undo.split('\n');
		undo = undoStringList.join(" ");

		QSqlQuery undoQuery;
		undoQuery.prepare(undo);

    if (!undoQuery.exec()) {
      isRunningOkay = false;
    }
  }

  return isRunningOkay;
}

bool Data::redo(quint16 index) const {
  bool isRunningOkay = true;

  // get the redo SQL
  QSqlQuery query(QString(
    "select\n"
    "  redoCommand\n"
    "from\n"
    "  logUndoRedo\n"
    "where\n"
    "  id = %1\n")
		.arg(nThLogUndoRedoId(index)));

  QString redo = "";

  if (query.next()) {
    redo = query.value(0).toString();
  } else {
    isRunningOkay = false;
  }

  // run the redo SQL
  if (isRunningOkay) {
    QStringList redoStringList = redo.split('\n');
		redo = redoStringList.join(" ");

		QSqlQuery redoQuery;
		redoQuery.prepare(redo);

		if (!redoQuery.exec()) {
      isRunningOkay = false;
    }
  }

  return isRunningOkay;
}

bool Data::deleteFromLogUndoRedo(
    quint16 firstIndex, quint16 endIndex) {
  bool isRunningOkay = true;

	if (firstIndex == 0
			|| firstIndex > endIndex
      || endIndex > logUndoRedoCount()) {
    isRunningOkay = false;
  }

  if (isRunningOkay) {
    // get the redo SQL
    QSqlQuery query(QString(
      "delete\n"
      "from\n"
      "  logUndoRedo\n"
      "where\n"
      "  id between %1 and %2\n")
      .arg(nThLogUndoRedoId(firstIndex))
      .arg(nThLogUndoRedoId(endIndex)));

    if (!query.exec()) {
      isRunningOkay = false;
    }
  }

  return isRunningOkay;
}

quint16 Data::nThLogUndoRedoId(quint16 index) const {
  bool isRunningOkay = true;

	if (index == 0
			|| index > logUndoRedoCount()) {
    isRunningOkay = false;
  }

	quint16 id = 0;

  if (isRunningOkay) {
    QSqlQuery query(QString(
      "select\n"
      "  id\n"
      "from\n"
      "  logUndoRedo\n"
      "order by\n"
      "  id\n"));

		if (query.exec() && query.isSelect()
				&& query.seek(index - 1) && query.isValid()) {
			id = query.value(0).toInt();
    } else {
			QMessageBox::warning(
				(QWidget *)0
				, QObject::tr("Could not get undo log id.")
				, QObject::tr("The undo log id is missing."));

			isRunningOkay = false;
		}
  }

  return id;
}

bool Data::getDataModified() const {
  return dataModified;
}

void Data::setDataModified(bool isDataModified) {
  dataModified = isDataModified;
}

void Data::clonePeriodAs(QString sourcePeriodId, QString periodId) {
  bool isRunningOkay = true;

	if (sourcePeriodId.isEmpty()
			|| periodId.isEmpty()) {
    isRunningOkay = false;
  }

	// get the source register ids
  QString registerSelectStatement =
    "select distinct\n"
		"  id\n"
    "from\n"
    "  register\n"
    "where\n"
    "  periodId = '%1'\n";

  QSqlQuery registerQuery(registerSelectStatement.arg(sourcePeriodId));
  QStringList sourceRegisterIds;

	while (registerQuery.next()) {
    sourceRegisterIds += registerQuery.value(0).toString();
  }

  if (sourceRegisterIds.count() == 0) {
    isRunningOkay = false;
  }

  if (isRunningOkay) {
    foreach(QString sourceRegisterId, sourceRegisterIds) {
      // insert into the period all of the source registers and values
			QString queryString =
				"insert into register(\n"
				"  id\n"
				"  , periodId\n"
				"  , itemId\n"
				"  , budget\n"
				"  , actual\n"
				"  , note)\n"
				"select\n"
				"  '%1'\n"
				"  , '%2'\n"
				"  , rmv.itemId\n"
				"  , rmv.budget\n"
				"  , rmv.actual\n"
				"  , rmv.note\n"
				"from\n"
				"  registerMetricsView rmv\n"
				"where\n"
				"  registerId = '%3'\n";

			queryString =
				queryString
					.arg(getNewPrimaryKeyId())
					.arg(periodId)
					.arg(sourceRegisterId);

			QSqlQuery query(queryString);

      if (!query.exec()) {
        isRunningOkay = false;
      }
    }
  }
}
