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
//    This class modifies to the QTableView class to help with column resizing
//    in the cashflow program.

#ifndef _TABLEVIEW_HPP_
  #define _TABLEVIEW_HPP_

  #include <QTableView>

  class TableView : public QTableView {
  public:
    TableView(QWidget *parent = (QWidget *)0) 
        : QTableView(parent) {
    }

  protected:
    bool viewportEvent(QEvent *event) {
      resizeColumnsToContents();
      return QTableView::viewportEvent(event);
    }

  private:
    void keyPressEvent(QKeyEvent *event) {
      switch (event->key()) {
      case Qt::Key_Space:
        // pass through
      case Qt::Key_Select:
        // pass through
      case Qt::Key_F2:
        // pass through
      case Qt::Key_Enter:
        // pass through
      case Qt::Key_Return:
        // pass through
      case Qt::Key_Insert:
        if (!edit(currentIndex(), EditKeyPressed, event))
          event->ignore();
        break;
      default:
        QAbstractItemView::keyPressEvent(event);
        break;
      }
    }
  };

#endif // _TABLEVIEW_HPP_
