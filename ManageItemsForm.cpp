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
//  ManageItemsForm class source
//    The class that controls the models and views of the items editing form.

#include <QtGui>
#include <QtSql>

#include "Application.hpp"
#include "cashflow.hpp"
#include "ManageItemsForm.hpp"
#include "SqlTableModel.hpp"
#include "TableView.hpp"

using Cashflow::Application;

ManageItemsForm::ManageItemsForm(
    int id, QWidget *parent) : QDialog(parent) {

  categoryModel = new SqlTableModel(this);
  categoryModel->setTable("categoryMapView");
  categoryModel->setSort(CategoryMapView_CategoryName, Qt::AscendingOrder);
  categoryModel->setHeaderData(CategoryMapView_FlowName, Qt::Horizontal, tr("Flow"));
  categoryModel->setHeaderData(CategoryMapView_CategoryName, Qt::Horizontal, tr("Category"));
  categoryModel->setEditStrategy(QSqlTableModel::OnRowChange);
  categoryModel->select();

  categoryView = new TableView(this);
  categoryView->setModel(categoryModel);
  categoryView->setSelectionMode(QAbstractItemView::SingleSelection);
  categoryView->setSelectionBehavior(QAbstractItemView::SelectRows);
  categoryView->setEditTriggers(
    QAbstractItemView::DoubleClicked 
    | QAbstractItemView::EditKeyPressed 
    | QAbstractItemView::SelectedClicked);
  categoryView->setColumnHidden(CategoryMapView_FlowId, true);
  categoryView->setColumnHidden(CategoryMapView_CategoryId, true);
  categoryView->resizeColumnsToContents();
  categoryView->horizontalHeader()->setStretchLastSection(true);
  categoryView->verticalHeader()->setVisible(false);

  QVBoxLayout *categoryLayout = new QVBoxLayout;
  categoryLayout->addWidget(categoryView);

  categoryGroupBox = new QGroupBox(tr("&Categories"), this);
  categoryGroupBox->setLayout(categoryLayout);

  itemModel = new SqlTableModel(this);
  itemModel->setTable("itemMapView");
  itemModel->setSort(ItemMapView_ItemName, Qt::AscendingOrder);
  itemModel->setHeaderData(ItemMapView_FlowName, Qt::Horizontal, tr("Flow"));
  itemModel->setHeaderData(ItemMapView_CategoryName, Qt::Horizontal, tr("Category"));
  itemModel->setHeaderData(ItemMapView_ItemName, Qt::Horizontal, tr("Item"));
  itemModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
  itemModel->select();

  itemView = new TableView(this);
  itemView->setModel(itemModel);
  itemView->setSelectionMode(QAbstractItemView::SingleSelection);
  itemView->setSelectionBehavior(QAbstractItemView::SelectRows);
  itemView->setEditTriggers(
    QAbstractItemView::DoubleClicked 
    | QAbstractItemView::EditKeyPressed 
    | QAbstractItemView::SelectedClicked);
  itemView->setColumnHidden(ItemMapView_FlowId, true);
  itemView->setColumnHidden(ItemMapView_CategoryId, true);
  itemView->setColumnHidden(ItemMapView_ItemId, true);
  itemView->resizeColumnsToContents();
  itemView->horizontalHeader()->setStretchLastSection(true);
  itemView->verticalHeader()->setVisible(false);

  addItemButton = new QPushButton(tr("&Add Item"));
  deleteItemButton = new QPushButton(tr("&Delete Item"));
  itemButtonBox = new QDialogButtonBox;
  itemButtonBox->addButton(addItemButton, QDialogButtonBox::ActionRole);
  itemButtonBox->addButton(deleteItemButton, QDialogButtonBox::ActionRole);

  QVBoxLayout *itemLayout = new QVBoxLayout;
  itemLayout->addWidget(itemView);
  itemLayout->addWidget(itemButtonBox);

  itemGroupBox = new QGroupBox(tr("&Items"));
  itemGroupBox->setLayout(itemLayout);

  connect(
    addItemButton
    , SIGNAL(clicked())
    , this
    , SLOT(addItem()));

  connect(
    deleteItemButton
    , SIGNAL(clicked())
    , this
    , SLOT(deleteItem()));

  connect(
    itemModel
    , SIGNAL(dataSubmitted())
    , this
    , SIGNAL(mappingChanged()));

  closeButton = new QPushButton(tr("&Close"));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));

  mainItemButtonBox = new QDialogButtonBox;
  mainItemButtonBox->addButton(closeButton, QDialogButtonBox::AcceptRole);

  QVBoxLayout *categoryMapLayout = new QVBoxLayout;
  categoryMapLayout->addWidget(categoryGroupBox);

  mapButton = new QPushButton(tr("<-- &Map Item <--"));
  connect(mapButton, SIGNAL(clicked()), this, SLOT(mapItem()));

  QHBoxLayout *mappingLayout = new QHBoxLayout;
  mappingLayout->addLayout(categoryMapLayout);
  mappingLayout->addWidget(mapButton);
  mappingLayout->addWidget(itemGroupBox);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(mappingLayout);
  mainLayout->addWidget(mainItemButtonBox);
  setLayout(mainLayout);

  if (id != -1) {
    bool isRowFound = false;

    int row = 0;

    while (row < categoryModel->rowCount() && !isRowFound) {
      QSqlRecord record = categoryModel->record(row);
      if (record.value(CategoryMapView_CategoryId).toInt() == id) {
        isRowFound = true;

        categoryView->selectRow(row);
        break;
      }

      ++row;
    }
  }
  else {
    categoryView->selectRow(-1);
  }

  categoryView->setFocus();

  setWindowTitle(tr("Manage Mappings"));
}

void ManageItemsForm::done(int result) {
  itemModel->submitAll();
  QDialog::done(result);
}

void ManageItemsForm::addItem() {
  addItem(
    itemView
    , itemModel
    , ItemMapView_ItemId
    , ItemMapView_ItemName);
}

void ManageItemsForm::addItem(
    TableView *itemView
    , SqlTableModel *itemModel
    , quint8 ItemIdColumn
    , quint8 ItemNameColumn) {
  bool isRunningOkay = true;

	// get a new row
	int row = itemModel->rowCount();

	if (row == -1) {
		QString atLine = ATLINE;
		QMessageBox::critical(
			this
			, tr("Cannot Add Item")
			, tr("The item view row is invalid.")
				+ "\n" + atLine
			, QMessageBox::Ok);

		isRunningOkay = false;
	}

	QModelIndex categoryViewIndex;

	if (isRunningOkay) {
		categoryViewIndex = categoryView->currentIndex();
		if (categoryViewIndex.row() == -1) {
			QString atLine = ATLINE;
			QMessageBox::critical(
				this
				, tr("Cannot Add Item")
				, tr("The category to map the new item too was not selected.\n"
					"Please select a category to add a new item.")
					+ "\n" + atLine
				, QMessageBox::Ok);

			isRunningOkay = false;
		}
	}

	QString itemName = "";

  if (isRunningOkay) {
    itemModel->insertRow(row);

		// apply the currently selected category id to the item
		QModelIndex itemModelCategoryId =
			itemModel->index(row, ItemMapView_CategoryId);
		QSqlRecord categoryRecord =
      categoryModel->record(categoryViewIndex.row());
    QString categoryId = categoryRecord.value("categoryId").toString();
		itemModel->setData(itemModelCategoryId, categoryId, Qt::EditRole);

    // apply the item id primary key value
		QModelIndex itemModelItemId =
			itemModelCategoryId.sibling(row, ItemIdColumn);
    QString itemId = qApp->getNewItemId();

    if (!itemModel->setData(itemModelItemId, itemId, Qt::EditRole)) {
      QString atLine = ATLINE;
      QMessageBox::critical(
        this
        , tr("Cannot Add Item")
        , tr("The new item %1 was not added.")
          .arg(itemName)
          + "\n" + atLine
        , QMessageBox::Ok);

      isRunningOkay = false;
    }

    // move to the inserted row
    QModelIndex itemModelItemName =
      itemModelItemId.sibling(row, ItemNameColumn);

    itemView->setFocus();
    itemView->setCurrentIndex(itemModelItemName);
  }
}

void ManageItemsForm::deleteItem() {
  deleteItem(
    itemView
    , itemModel
    , ItemMapView_ItemId
    , ItemMapView_ItemName);
}

void ManageItemsForm::deleteItem(
    TableView *itemView
    , SqlTableModel *itemModel
    , quint8 ItemIdColumn
    , quint8 ItemNameColumn) {
  bool isRunningOkay = true;

  // get the category's row
  int row = -1;
  QModelIndex itemViewIndex = itemView->currentIndex();
  if (itemViewIndex.isValid()) {
    row = itemViewIndex.row();
  }

  // get the item name and id
  QSqlRecord itemRecord = itemModel->record(row);
  QString itemName = itemRecord.value(ItemNameColumn).toString();
  QString itemId = itemRecord.value(ItemIdColumn).toString();

  // verify the item is not in use
  if (qApp->itemInRegister(itemId)) {
    QString atLine = ATLINE;
    QMessageBox::critical(
      this
      , tr("Cannot Delete Item")
      , tr("The item %1 is in use. It cannot be deleted.")
        .arg(itemName)
        + "\n" + atLine
      , QMessageBox::Ok);

    isRunningOkay = false;
  }

  // remove the row
  if (isRunningOkay) {
    bool rowRemoved = itemModel->removeRow(row);
    if (!rowRemoved) {
      QString atLine = ATLINE;
      QMessageBox::critical(
        this
        , tr("Cannot Delete Item")
        , tr("Cannot remove row from the model.")
          + "\n" + atLine
        , QMessageBox::Ok);

      isRunningOkay = false;
    }
  }

  // move focus to the next row
  if (isRunningOkay) {
    int rowCount = itemModel->rowCount();
    int nextRow = row < rowCount ? row : rowCount - 1;
    nextRow = nextRow >= 0 ? nextRow : 0;

    QModelIndex itemModelItemName =
      itemViewIndex.sibling(nextRow, ItemNameColumn);

    itemView->setFocus();
    itemView->setCurrentIndex(itemModelItemName);
  }

	if (isRunningOkay) {
		itemModel->submitAll();
		itemModel->submit();
		updateViewsAfterChange();
	}
}

void ManageItemsForm::mapItem() {
  bool isRunningOkay = true;
  
  // get currently selected in category id
  QModelIndex categoryViewIndex = categoryView->currentIndex();
  if (!categoryViewIndex.isValid()) {
    QString atLine = ATLINE;
    QMessageBox::critical(
      this
      , tr("Cannot Map Item to a Category")
      , tr("The nothing was selected in the in flow category view.")
        + "\n" + atLine
      , QMessageBox::Ok);

    isRunningOkay = false;
  }

  if (isRunningOkay) {
    QSqlRecord categoryRecord =
      categoryModel->record(categoryViewIndex.row());
    QString categoryId = categoryRecord.value("categoryId").toString();
    QString categoryName = categoryRecord.value("categoryName").toString();

    // get field for item's category id
    QModelIndex itemViewIndex = itemView->currentIndex();
    if (!itemViewIndex.isValid()) {
      QString atLine = ATLINE;
      QMessageBox::critical(
        this
        , tr("Cannot Map Item to a Category")
        , tr("The nothing was selected in the in flow item view.")
          + "\n" + atLine
        , QMessageBox::Ok);

      isRunningOkay = false;
    }

    if (isRunningOkay) {
      QModelIndex itemModelCategoryId =
        itemModel->index(itemViewIndex.row(), ItemMapView_CategoryId);
      QSqlRecord itemRecord = itemModel->record(itemViewIndex.row());
      QString itemName = itemRecord.value("itemName").toString();

      // set the new category
      if (!itemModel->setData(
          itemModelCategoryId, categoryId, Qt::EditRole)) {
        QString atLine = ATLINE;
        QMessageBox::critical(
          this
          , tr("Cannot Map Item to a Category")
          , tr("The item %1 was not mapped to the category %2.")
            .arg(itemName)
            .arg(categoryName)
            + "\n" + atLine
          , QMessageBox::Ok);

        isRunningOkay = false;
      }
    }
  }

  if (isRunningOkay) {
		itemModel->submitAll();
		itemModel->submit();
    updateViewsAfterChange();
  }
}

void ManageItemsForm::updateViewsAfterChange() {
  QModelIndex categoryViewIndex = categoryView->currentIndex();
  QModelIndex itemViewIndex = itemView->currentIndex();

  if (!categoryModel->select()) {
    QMessageBox::warning(
      (QWidget *)0
      , QObject::tr("Error: Selecting the category model failed.")
      , QObject::tr("There was an error with selecting the category model."));
  }

  categoryView->setCurrentIndex(categoryViewIndex);

  if (!itemModel->select()) {
    QMessageBox::warning(
      (QWidget *)0
      , QObject::tr("Error: Selecting the item model failed.")
      , QObject::tr("There was an error with selecting the item model."));
  }

  itemView->setCurrentIndex(itemViewIndex);
}
