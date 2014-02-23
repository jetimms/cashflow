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
//  ManageCategoriesForm class source
//    The class that controls the models and views of the categories editing form.

#include <QtGui>
#include <QtSql>

#include "Application.hpp"
#include "cashflow.hpp"
#include "ManageCategoriesForm.hpp"
#include "SqlTableModel.hpp"
#include "TableView.hpp"

using Cashflow::Application;

ManageCategoriesForm::ManageCategoriesForm(
    int id, QWidget *parent) : QDialog(parent) {

  inCategoryModel = new SqlTableModel(this);
  inCategoryModel->setTable("inCategoryMapView");
  inCategoryModel->setSort(CategoryMapView_CategoryName, Qt::AscendingOrder);
  inCategoryModel->setHeaderData(
    CategoryMapView_CategoryName, Qt::Horizontal, tr("Category"));
  inCategoryModel->setEditStrategy(QSqlTableModel::OnRowChange);
  inCategoryModel->select();

  inCategoryView = new TableView(this);
  inCategoryView->setModel(inCategoryModel);
  inCategoryView->setSelectionMode(QAbstractItemView::SingleSelection);
  inCategoryView->setSelectionBehavior(QAbstractItemView::SelectRows);
  inCategoryView->setEditTriggers(
    QAbstractItemView::DoubleClicked 
    | QAbstractItemView::EditKeyPressed 
    | QAbstractItemView::SelectedClicked);
  inCategoryView->setColumnHidden(CategoryMapView_CategoryId, true);
  inCategoryView->setColumnHidden(CategoryMapView_FlowId, true);
  inCategoryView->setColumnHidden(CategoryMapView_FlowName, true);
  inCategoryView->horizontalHeader()->setStretchLastSection(true);
  inCategoryView->verticalHeader()->setVisible(false);

  addInButton = new QPushButton(tr("&Add In Category"));
  deleteInButton = new QPushButton(tr("&Delete In Category"));
  inButtonBox = new QDialogButtonBox;
  inButtonBox->addButton(addInButton, QDialogButtonBox::ActionRole);
  inButtonBox->addButton(deleteInButton, QDialogButtonBox::ActionRole);

  QVBoxLayout *inLayout = new QVBoxLayout;
  inLayout->addWidget(inCategoryView);
  inLayout->addWidget(inButtonBox);

  inGroupBox = new QGroupBox(tr("&In Flow Categories"), this);
  inGroupBox->setLayout(inLayout);

  connect(
    addInButton
    , SIGNAL(clicked())
    , this
    , SLOT(addInCategory()));

  connect(
    deleteInButton
    , SIGNAL(clicked())
    , this
    , SLOT(deleteInCategory()));

  outCategoryModel = new SqlTableModel(this);
  outCategoryModel->setTable("outCategoryMapView");
  outCategoryModel->setSort(CategoryMapView_CategoryName, Qt::AscendingOrder);
  outCategoryModel->setHeaderData(
    CategoryMapView_CategoryName, Qt::Horizontal, tr("Category"));
  outCategoryModel->setEditStrategy(QSqlTableModel::OnRowChange);
  outCategoryModel->select();

  outCategoryView = new TableView(this);
  outCategoryView->setModel(outCategoryModel);
//  outCategoryView->setItemDelegate(new QSqlRelationalDelegate(this));
  outCategoryView->setSelectionMode(QAbstractItemView::SingleSelection);
  outCategoryView->setSelectionBehavior(QAbstractItemView::SelectRows);
  outCategoryView->setEditTriggers(
    QAbstractItemView::DoubleClicked 
    | QAbstractItemView::EditKeyPressed 
    | QAbstractItemView::SelectedClicked);
  outCategoryView->setColumnHidden(CategoryMapView_CategoryId, true);
  outCategoryView->setColumnHidden(CategoryMapView_FlowId, true);
  outCategoryView->setColumnHidden(CategoryMapView_FlowName, true);
  outCategoryView->horizontalHeader()->setStretchLastSection(true);
  outCategoryView->verticalHeader()->setVisible(false);

  addOutButton = new QPushButton(tr("&Add Out Category"));
  deleteOutButton = new QPushButton(tr("&Delete Out Category"));
  outButtonBox = new QDialogButtonBox;
  outButtonBox->addButton(addOutButton, QDialogButtonBox::ActionRole);
  outButtonBox->addButton(deleteOutButton, QDialogButtonBox::ActionRole);

  QVBoxLayout *outLayout = new QVBoxLayout;
  outLayout->addWidget(outCategoryView);
  outLayout->addWidget(outButtonBox);

  outGroupBox = new QGroupBox(tr("&Out Flow Categories"), this);
  outGroupBox->setLayout(outLayout);

  connect(
    addOutButton
    , SIGNAL(clicked())
    , this
    , SLOT(addOutCategory()));

  connect(
    deleteOutButton
    , SIGNAL(clicked())
    , this
    , SLOT(deleteOutCategory()));

  connect(
    inCategoryModel
    , SIGNAL(dataSubmitted())
    , this
    , SIGNAL(mappingChanged()));

  connect(
    outCategoryModel
    , SIGNAL(dataSubmitted())
    , this
    , SIGNAL(mappingChanged()));

  closeButton = new QPushButton(tr("&Close"));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));

  mainButtonBox = new QDialogButtonBox;
  mainButtonBox->addButton(closeButton, QDialogButtonBox::AcceptRole);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(inGroupBox);
  mainLayout->addWidget(outGroupBox);
  mainLayout->addWidget(mainButtonBox);
  setLayout(mainLayout);

  if (id != -1) {
    bool isRowFound = false;

    int inRow = 0;

    while (inRow < inCategoryModel->rowCount() && !isRowFound) {
      QSqlRecord record = inCategoryModel->record(inRow);
      if (record.value(CategoryMapView_CategoryId).toInt() == id) {
        isRowFound = true;

        inCategoryView->selectRow(inRow);
        break;
      }

      ++inRow;
    }

    int outRow = 0;

    while (outRow < outCategoryModel->rowCount() && !isRowFound) {
      QSqlRecord record = outCategoryModel->record(outRow);
      if (record.value(CategoryMapView_CategoryId).toInt() == id) {
        isRowFound = true;

        outCategoryView->selectRow(outRow);
        break;
      }

      ++outRow;
    }
  }
  else {
    inCategoryView->selectRow(-1);
  }

  inCategoryView->setFocus();

  setWindowTitle(tr("Manage Categories"));
}

void ManageCategoriesForm::done(int result) {
  inCategoryModel->submitAll();
  outCategoryModel->submitAll();
  QDialog::done(result);
}

void ManageCategoriesForm::addInCategory() {
  addCategory(
    inCategoryView
    , inCategoryModel
    , qApp->getInFlowId()
    , CategoryMapView_FlowId
    , CategoryMapView_CategoryId
    , CategoryMapView_CategoryName);
}

void ManageCategoriesForm::addOutCategory() {
  addCategory(
    outCategoryView
    , outCategoryModel
    , qApp->getOutFlowId()
    , CategoryMapView_FlowId
    , CategoryMapView_CategoryId
    , CategoryMapView_CategoryName);
}

void ManageCategoriesForm::addCategory(
    TableView *categoryView
    , SqlTableModel *categoryModel
    , QString flowId
    , quint8 FlowIdColumn
    , quint8 CategoryIdColumn
    , quint8 CategoryNameColumn) {
  bool isRunningOkay = true;

	// get a new row
  int row = categoryModel->rowCount();

  if (row == -1) {
		QString atLine = ATLINE;
		QMessageBox::critical(
      this
      , tr("Cannot Add Category")
      , tr("The category view row is invalid.")
				+ "\n" + atLine
      , QMessageBox::Ok);

    isRunningOkay = false;
  }

  if (isRunningOkay) {
    categoryModel->insertRow(row);

    // apply the flow id primary key value
    QModelIndex categoryModelFlowId =
      categoryModel->index(row, FlowIdColumn);
    categoryModel->setData(categoryModelFlowId, flowId, Qt::EditRole);

    // get a new category id
    QModelIndex categoryModelCategoryId =
      categoryModelFlowId.sibling(row, CategoryIdColumn);
    QString categoryId = qApp->getNewCategoryId();
    categoryModel->setData(categoryModelCategoryId, categoryId, Qt::EditRole);

    // move to the inserted row
    QModelIndex categoryModelCategoryName =
      categoryModelFlowId.sibling(row, CategoryNameColumn);

  	categoryView->setFocus();
  	categoryView->setCurrentIndex(categoryModelCategoryName);
  }
}

void ManageCategoriesForm::deleteInCategory() {
  deleteCategory(
    inCategoryView
    , inCategoryModel
    , CategoryMapView_CategoryId
    , CategoryMapView_CategoryName);
}

void ManageCategoriesForm::deleteOutCategory() {
  deleteCategory(
    outCategoryView
    , outCategoryModel
    , CategoryMapView_CategoryId
    , CategoryMapView_CategoryName);
}

void ManageCategoriesForm::deleteCategory(
    TableView *categoryView
    , SqlTableModel *categoryModel
    , quint8 CategoryIdColumn
    , quint8 CategoryNameColumn) {
  bool isRunningOkay = true;

	// get the category's row
	int row = -1;
	QModelIndex categoryViewIndex = categoryView->currentIndex();
	if (categoryViewIndex.isValid()) {
		row = categoryViewIndex.row();
	}

	// get the category name and id
	QSqlRecord categoryRecord = categoryModel->record(row);
	QString categoryName = categoryRecord.value(CategoryNameColumn).toString();
	QString categoryId = 
	  categoryRecord.value(CategoryIdColumn).toString();

	// verify the category is not in use
	if (qApp->categoryHasItems(categoryId)) {
	  QString atLine = ATLINE;
		QMessageBox::critical(
			this
			, tr("Cannot Delete Category")
			, tr("The category %1 is in use. It cannot be deleted.")
				.arg(categoryName)
				+ "\n" + atLine
			, QMessageBox::Ok);

		isRunningOkay = false;
	}

  // remove the row
	if (isRunningOkay) {
  	if (!categoryModel->removeRow(row)) {
  	  QString atLine = ATLINE;
  		QMessageBox::critical(
  			this
  			, tr("Cannot Delete Category")
  			, tr("Cannot remove row from the model.")
  				+ "\n" + atLine
  			, QMessageBox::Ok);

  		isRunningOkay = false;
  	}
  }

  // move focus to the next row
	if (isRunningOkay) {
    int rowCount = categoryModel->rowCount();
    int nextRow = row < rowCount ? row : rowCount - 1;
    nextRow = nextRow >= 0 ? nextRow : 0;

    QModelIndex categoryModelCategoryName =
      categoryViewIndex.sibling(nextRow, CategoryNameColumn);

  	categoryView->setFocus();
  	categoryView->setCurrentIndex(categoryModelCategoryName);
	}
}
