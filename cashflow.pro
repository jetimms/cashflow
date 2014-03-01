# Copyright 2014 Jason Eric Timms
# 
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
# cashflow.pro
#   Qt 4 project file

CONFIG += uitools

release: {
  CONFIG -= console
  DESTDIR = build/release
}

debug: {
  CONFIG += console
  DESTDIR = build/debug
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

TEMPLATE = app
TARGET = cashflow
# LIBS += .
DEPENDPATH += .
INCLUDEPATH += .

QT += sql

#FORMS += .
HEADERS = \
  Application.hpp \
  cashflow.hpp \
  Data.hpp \
  DecimalFieldItemDelegate.hpp \
  HeaderView.hpp \
  ManageCategoriesForm.hpp \
  ManageItemsForm.hpp \
  MainForm.hpp \
  SqlTableModel.hpp \
  TableView.hpp
SOURCES = \
  Application.cpp \
  Data.cpp \
  ManageCategoriesForm.cpp \
  ManageItemsForm.cpp \
  MainForm.cpp \
  SqlTableModel.cpp \
  main.cpp
RESOURCES = \
  cashflow.qrc

win32:RC_FILE += cashflow.rc
