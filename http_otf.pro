#-------------------------------------------------
#
# Project created by QtCreator 2014-06-17T21:23:19
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = http_otf
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    cloud_http.cpp \
    cloud_helper.cpp \
    cloud_mime.cpp \
    cloud_auth.cpp

HEADERS += \
    cloud_http.h \
    cloud_mime.h \
    circular_buffer.h \
    cloud_assoc_cont.h
