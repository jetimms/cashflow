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
//  DecimalFieldItemDelegate class definition
//    This class modifies to the QStyledItemDelegate class to help with 
//    displaying the currency amount fields to two decimal places.

#ifndef _DECIMALFIELDITEMDELEGATE_HPP_
  #define _DECIMALFIELDITEMDELEGATE_HPP_

  #include <QStyledItemDelegate>
  
  class DecimalFieldItemDelegate : public QStyledItemDelegate {
  public:
    DecimalFieldItemDelegate(QObject *parent = 0) 
      : QStyledItemDelegate(parent) {}
  
    QString displayText(const QVariant &value, const QLocale &) const {
      QString str = QString::number(value.toDouble(), 'f', 2);
      return str;
    }
  };
  
#endif // _DECIMALFIELDITEMDELEGATE_HPP_
