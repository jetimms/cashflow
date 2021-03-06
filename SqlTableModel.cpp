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
//  SqlTableModel.cpp
//  Modifies to the QSqlTableModel class to help with field alignment and row
//  removal in the cashflow program.

#include <QtGui>
#include <QtSql>
#include <QDebug>

#include "cashflow.hpp"
#include "SqlTableModel.hpp"

using Cashflow::SqlTableModel;

SqlTableModel::SqlTableModel(
  QObject *parent, QSqlDatabase db)
  : QSqlTableModel(parent, db)
{
  // intentionally empty function
}

QVariant SqlTableModel::data(const QModelIndex &index, int role) const {
  QString fieldName =
    headerData(index.column(), Qt::Horizontal, Qt::DisplayRole).toString();

  // if the alignment role, right-align for numbers
  if (role == Qt::TextAlignmentRole) {
    if (fieldName == "Budget"
        || fieldName == "Actual"
        || fieldName == "Difference"
        || fieldName == "Budget Balance"
        || fieldName == "Actual Balance"
        || fieldName == "Difference Balance") {
      return (int)(Qt::AlignRight | Qt::AlignVCenter);
    }
  }
  else if (role == Qt::BackgroundRole) {
    if (
      tableName() != "periodMetricsView"
      && tableName() != "registerMetricsView"
      && tableName() != "inCategoryMapView"
      && tableName() != "outCategoryMapView"
      && tableName() != "categoryMapView"
      && tableName() != "itemMapView")
    {
      return QVariant(QColor(224, 224, 224));
    }
    else if (tableName() == "periodMetricsView") {
      if (fieldName != "Period") {
        return QVariant(QColor(224, 224, 224));
      }
    }
    else if (tableName() == "registerMetricsView") {
      if (
        fieldName != "Note"
        && fieldName != "Budget"
        && fieldName != "Actual")
      {
        return QVariant(QColor(224, 224, 224));
      }
    }
    else if (tableName() == "inCategoryMapView") {
      if (fieldName != "Category") {
        return QVariant(QColor(224, 224, 224));
      }
    }
    else if (tableName() == "outCategoryMapView") {
      if (fieldName != "Category") {
        return QVariant(QColor(224, 224, 224));
      }
    }
    else if (tableName() == "categoryMapView") {
      if (fieldName != "Category") {
        return QVariant(QColor(224, 224, 224));
      }
    }
    else if (tableName() == "itemMapView") {
      if (fieldName != "Item") {
        return QVariant(QColor(224, 224, 224));
      }
    }
  }
  else if (role == Qt::ForegroundRole) {
    if (
      tableName() != "periodMetricsView"
      && tableName() != "registerMetricsView"
      && tableName() != "inCategoryMapView"
      && tableName() != "outCategoryMapView"
      && tableName() != "categoryMapView"
      && tableName() != "itemMapView")
    {
      return QVariant(QColor(42, 42, 42));
    }
    else if (tableName() == "periodMetricsView") {
      if (fieldName != "Period") {
        return QVariant(QColor(42, 42, 42));
      }
    }
    else if (tableName() == "registerMetricsView") {
      if (
        fieldName != "Note"
        && fieldName != "Budget"
        && fieldName != "Actual")
      {
        return QVariant(QColor(42, 42, 42));
      }
    }
    else if (tableName() == "inCategoryMapView") {
      if (fieldName != "Category") {
        return QVariant(QColor(42, 42, 42));
      }
    }
    else if (tableName() == "outCategoryMapView") {
      if (fieldName != "Category") {
        return QVariant(QColor(42, 42, 42));
      }
    }
    else if (tableName() == "categoryMapView") {
      if (fieldName != "Category") {
        return QVariant(QColor(42, 42, 42));
      }
    }
    else if (tableName() == "itemMapView") {
      if (fieldName != "Item") {
        return QVariant(QColor(42, 42, 42));
      }
    }
  }

  return QSqlTableModel::data(index, role);
}

QString SqlTableModel::selectStatement() const {
  return QSqlTableModel::selectStatement();
}

bool SqlTableModel::submit() {
  bool isRunningOkay = QSqlTableModel::submit();

  emit dataSubmitted();

  return isRunningOkay;
}

bool SqlTableModel::removeRow(int row, const QModelIndex &parent) {
  bool isRunningOkay = true;

  isRunningOkay = QSqlTableModel::removeRow(row, parent);

  emit dataSubmitted();

  return isRunningOkay;
}

Qt::ItemFlags SqlTableModel::flags(const QModelIndex & index) const {

  Qt::ItemFlags defaultFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  Qt::ItemFlags editFlags = defaultFlags | Qt::ItemIsEditable;
  
  Qt::ItemFlags decideFlags = defaultFlags;

  if (tableName() == "periodMetricsView") {
    if (index.column() == fieldIndex("PeriodName")) {
      decideFlags = editFlags;
    }
  }
  else if (tableName() == "registerMetricsView") {
    if (index.column() == fieldIndex("Note")
      || index.column() == fieldIndex("Budget")
      || index.column() == fieldIndex("Actual"))
    {
      decideFlags = editFlags;
    }
  }
  else if (tableName() == "inCategoryMapView") {
    if (index.column() == fieldIndex("CategoryName")) {
      decideFlags = editFlags;
    }
  }
  else if (tableName() == "outCategoryMapView") {
    if (index.column() == fieldIndex("CategoryName")) {
      decideFlags = editFlags;
    }
  }
  else if (tableName() == "categoryMapView") {
    if (index.column() == fieldIndex("CategoryName")) {
      decideFlags = editFlags;
    }
  }
  else if (tableName() == "itemMapView") {
    if (index.column() == fieldIndex("ItemName")) {
      decideFlags = editFlags;
    }
  }

  Qt::ItemFlags returnFlags = decideFlags;

  return returnFlags;
}
