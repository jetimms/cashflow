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
//  TableView class definition
//    This class modifies the QTableView class to help with the view's cell 
//    editor behavior.

#ifndef _TABLEVIEW_HPP_
  #define _TABLEVIEW_HPP_

  #include <QTableView>

  namespace Cashflow {
    class TableView : public QTableView {
    public:
      TableView(QWidget *parent = (QWidget *)0)
          : QTableView(parent) {
      }
  
    protected:
      bool viewportEvent(QEvent *event);
      void keyPressEvent(QKeyEvent *event);

    protected slots:
      void closeEditor(
        QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
  
    private:
      QModelIndex forwardEditableIndex(QModelIndex currentModelIndex);
      QModelIndex backwardEditableIndex(QModelIndex currentModelIndex);
      QModelIndex seekEditableIndex(QModelIndex currentModelIndex, int step);
    };
  }
#endif // _TABLEVIEW_HPP_
