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
    : QSqlTableModel(parent, db) {
  // intentionally empty function
}

QVariant SqlTableModel::data(const QModelIndex &index, int role) const {
  // if the alignment role, right-align for numbers
  if (role == Qt::TextAlignmentRole) {
    QString fieldName =
      headerData(index.column(), Qt::Horizontal, Qt::DisplayRole).toString();

    if (fieldName == "Budget"
        || fieldName == "Actual"
        || fieldName == "Difference"
        || fieldName == "Budget Balance"
        || fieldName == "Actual Balance"
        || fieldName == "Difference Balance") {
      return (int)(Qt::AlignRight | Qt::AlignVCenter);
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
