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
//  ManageCategoriesForm class definition
//    The class that controls the models and views of the categories editing form.

#ifndef _MANAGECATEGORIESFORM_HPP_
  #define _MANAGECATEGORIESFORM_HPP_

  #include <QDialog>

  class QDialogButtonBox;
  class QGroupBox;
  class QLabel;
  class QPushButton;
  class SqlTableModel;
  class TableView;
  
  enum {
    // table column enums
    Category_Id = 0
    , Category_Name = 1
    , Category_FlowId = 2

    // metrics view column enums
    , CategoryMetricsView_PeriodId = 0
    , CategoryMetricsView_FlowId = 1
    , CategoryMetricsView_CategoryId = 2
    , CategoryMetricsView_PeriodName = 3
    , CategoryMetricsView_FlowName = 4
    , CategoryMetricsView_CategoryName = 5
    , CategoryMetricsView_Budget = 6
    , CategoryMetricsView_Actual = 7
    , CategoryMetricsView_Difference = 8
  };

  enum {
    CategoryMapView_FlowId = 0
    , CategoryMapView_FlowName
    , CategoryMapView_CategoryId
    , CategoryMapView_CategoryName
  };

  class ManageCategoriesForm : public QDialog {
    Q_OBJECT

  public:
    ManageCategoriesForm(int id, QWidget *parent = (QWidget *)0);

  public slots:
    void done(int result);

  private slots:
    void addInCategory();
    void deleteInCategory();

    void addOutCategory();
    void deleteOutCategory();

  private:
    void addCategory(
        TableView *categoryView
        , SqlTableModel *categoryModel
        , QString flowId
        , quint8 FlowIdColumn
        , quint8 CategoryIdColumn
        , quint8 CategoryNameColumn);

    void deleteCategory(
      TableView *categoryView
      , SqlTableModel *categoryModel
      , quint8 CategoryMapIdColumn
      , quint8 CategoryMapNameColumn);

    SqlTableModel *inCategoryModel;
    TableView *inCategoryView;
    QGroupBox *inGroupBox;
    QPushButton *addInButton;
    QPushButton *deleteInButton;
    QDialogButtonBox *inButtonBox;

    SqlTableModel *outCategoryModel;
    TableView *outCategoryView;
    QGroupBox *outGroupBox;
    QPushButton *addOutButton;
    QPushButton *deleteOutButton;
    QDialogButtonBox *outButtonBox;

    QPushButton *closeButton;
    QDialogButtonBox *mainButtonBox;

  signals:
    void mappingChanged();
  };

#endif //_MANAGECATEGORIESFORM_HPP_
