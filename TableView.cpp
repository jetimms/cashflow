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
//  TableView.cpp
//    This class modifies the QTableView class to help with the view's cell 
//    editor behavior.

#include <QtGui>
#include <QtSql>
#include <QDebug>
#include <QTableView>

#include "TableView.hpp"
#include "cashflow.hpp"

using Cashflow::TableView;

bool TableView::viewportEvent(QEvent *event) {
  resizeColumnsToContents();
  return QTableView::viewportEvent(event);
}

void TableView::keyPressEvent(QKeyEvent *event) {
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

void TableView::closeEditor(
    QWidget *editor,
    QAbstractItemDelegate::EndEditHint hint) {

  // if clicked out of editor
  if (hint == QAbstractItemDelegate::NoHint) {
    QTableView::closeEditor(
      editor, QAbstractItemDelegate::SubmitModelCache);
  }
  // if tabbed
  else if (
      hint == QAbstractItemDelegate::EditNextItem ||
      hint == QAbstractItemDelegate::EditPreviousItem) {

    QModelIndex editableIndex;
    if (hint == QAbstractItemDelegate::EditNextItem) {
      editableIndex = forwardEditableIndex(currentIndex());
    } else {
      editableIndex = backwardEditableIndex(currentIndex());
    }

    if (editableIndex == QModelIndex()) {
      // if tabbed off the end
      QTableView::closeEditor(
        editor, QAbstractItemDelegate::SubmitModelCache);
    } else {
      QTableView::closeEditor(
        editor, QAbstractItemDelegate::NoHint);

      setCurrentIndex(editableIndex);
      edit(editableIndex);
    }

  } else {
    QTableView::closeEditor(editor, hint);
  }
}

QModelIndex TableView::forwardEditableIndex(QModelIndex currentModelIndex) {
  return seekEditableIndex(currentModelIndex, 1);
}
  
QModelIndex TableView::backwardEditableIndex(QModelIndex currentModelIndex) {
  return seekEditableIndex(currentModelIndex, -1);
}

QModelIndex TableView::seekEditableIndex(
    QModelIndex currentModelIndex, int step) {
  QHeaderView *h = horizontalHeader();

  const int col = currentModelIndex.column();
  const int row = currentModelIndex.row();

  if (currentModelIndex != QModelIndex()) {
    // if any of the following are true, seek the next model index      
    bool seekNextIndex = false;

    // still on the original model index
    seekNextIndex = seekNextIndex || currentModelIndex == currentIndex();

    // current model index is hidden
    seekNextIndex = seekNextIndex || h->isSectionHidden(col);

    // current model index is not editable
    QAbstractItemModel *model = this->model();
    Qt::ItemFlags flags = 
      model->flags(currentModelIndex) & Qt::ItemIsEditable;

    seekNextIndex = seekNextIndex || (flags) != Qt::ItemIsEditable;

    if (seekNextIndex) {
      const int nextCol = col + step;
      QModelIndex nextModelIndex = currentModelIndex.sibling(row, nextCol);

      currentModelIndex = seekEditableIndex(nextModelIndex, step);
    }
  }
  
  return currentModelIndex;
}
