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
//  ManageItemsForm class definition
//    The class that controls the models and views of the items editing form.

#ifndef _MANAGEITEMSFORM_HPP_
  #define _MANAGEITEMSFORM_HPP_

  #include <QDialog>
  #include "ManageCategoriesForm.hpp"

  class QDialogButtonBox;
  class QGroupBox;
  class QLabel;
  class QPushButton;
  class QSqlRecord;
  class SqlTableModel;
  class TableView;

  enum {
    // table column enums
    Item_Id = 0
    , Item_Name = 1
    , Item_CategoryId = 2

    , Register_Id = 0
    , Register_PeriodId = 1
    , Register_ItemId = 2
    , Register_Budget = 3
    , Register_Actual = 4

    // metrics view column enums
    , RegisterMetricsView_RegisterId = 0
    , RegisterMetricsView_PeriodId = 1
    , RegisterMetricsView_FlowId = 2
    , RegisterMetricsView_CategoryId = 3
    , RegisterMetricsView_ItemId = 4
    , RegisterMetricsView_PeriodName = 5
    , RegisterMetricsView_FlowName = 6
    , RegisterMetricsView_CategoryName = 7
    , RegisterMetricsView_ItemName = 8
    , RegisterMetricsView_Note = 9
    , RegisterMetricsView_Budget = 10
    , RegisterMetricsView_Actual = 11
    , RegisterMetricsView_Difference = 12

    , UnusedMetricsView_PeriodId = 0
    , UnusedMetricsView_ItemId = 1
    , UnusedMetricsView_PeriodName = 2
    , UnusedMetricsView_FlowName = 3
    , UnusedMetricsView_CategoryName = 4
    , UnusedMetricsView_ItemName = 5
  };
  
  // map view column enums
  enum {
    ItemMapView_FlowId = 0
    , ItemMapView_FlowName
    , ItemMapView_CategoryId
    , ItemMapView_CategoryName
    , ItemMapView_ItemId
    , ItemMapView_ItemName
  };

  class ManageItemsForm : public QDialog {
    Q_OBJECT

  public:
    ManageItemsForm(int id, QWidget *parent = (QWidget *)0);

  public slots:
    void done(int result);

  private slots:
    void addItem();
    void deleteItem();
    void mapItem();
    void updateViewsAfterChange();
  
  private:
    void addItem(
      TableView *itemView
      , SqlTableModel *itemModel
      , quint8 ItemIdColumn
      , quint8 ItemNameColumn);

    void deleteItem(
      TableView *itemView
      , SqlTableModel *itemModel
      , quint8 ItemIdColumn
      , quint8 ItemNameColumn);

    SqlTableModel *categoryModel;
    TableView *categoryView;
    QGroupBox *categoryGroupBox;
    QPushButton *addCategoryButton;
    QPushButton *deleteCategoryButton;
    QDialogButtonBox *categoryButtonBox;

    SqlTableModel *itemModel;
    TableView *itemView;
    QGroupBox *itemGroupBox;
    QPushButton *addItemButton;
    QPushButton *deleteItemButton;
    QDialogButtonBox *itemButtonBox;

    QPushButton *mapButton;
    QPushButton *closeButton;
    QDialogButtonBox *mainItemButtonBox;

  signals:
    void mappingChanged();
  };

#endif //_MANAGEITEMSFORM_HPP_
