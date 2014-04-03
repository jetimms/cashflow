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
//  SqlTableModel class definition
//    This class modifies the QSqlTableModel class to help with field 
//    alignment and row removal in the cashflow program.

#ifndef _SQLTABLEMODEL_HPP_
  #define _SQLTABLEMODEL_HPP_

  #include <QModelIndex>
  #include <QSqlTableModel>

  namespace Cashflow {
    class SqlTableModel : public QSqlTableModel {
  		Q_OBJECT
    public:
      SqlTableModel(
        QObject *parent = (QObject *)0
        , QSqlDatabase db = QSqlDatabase());
  
      QVariant data(
        const QModelIndex &index
        , int role = Qt::DisplayRole) const;
  
      bool removeRow(int row, const QModelIndex &parent = QModelIndex());
      QString selectStatement() const;
  
    public slots:
      bool submit();
    
    signals:
      void dataSubmitted();
    };
  }
#endif // _SQLTABLEMODEL_HPP_
