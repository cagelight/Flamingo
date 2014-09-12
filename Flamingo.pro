CONFIG += c++11
CONFIG += Debug

QT_VERSION = 5
QT += core gui widgets

TARGET = Flamingo
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/qw_mainwindow.cpp \
    src/qa_core.cpp \
    src/qw_imgview.cpp

HEADERS  += \
    src/qw_mainwindow.hpp \
    src/qa_core.hpp \
    src/qw_imgview.hpp

Release:DESTDIR = release
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.rcc
Debug:UI_DIR = debug/.ui
