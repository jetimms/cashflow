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
//  HeaderView class definition
//    This class modifies to the QHeaderView class to help with header field 
//    orientation and header combo box control in cashflow program.

#ifndef _HEADERVIEW_HPP_
  #define _HEADERVIEW_HPP_

  #include <QHeaderView>

  class HeaderView : public QHeaderView {
  public:
    HeaderView(Qt::Orientation orientation, QWidget *parent = (QWidget *)0)
        : QHeaderView(orientation, parent) {
    }

  protected:
    void paintEvent(QPaintEvent *event) {
//      QStylePainter painter(this);
//      QStyleOptionComboBox opt;
//      initStyleOption(&opt);
//      painter.drawComplexControl(QStyle::CC_ComboBox, opt);
//
//      QVariant itemData = this->itemData(this->currentIndex(), Qt::DisplayRole);
//
//      if (!itemData.isNull() && qVariantCanConvert<QPen>(itemData)) {
//        style()->drawItemText(
//          &painter
//          , this->rect()
//          , Qt::AlignLeft
//          , (this->palette())
//          , true
//          , itemData.toString());
//      }

      return QHeaderView::paintEvent(event);
    }
  };

#endif // _HEADERVIEW_HPP_

