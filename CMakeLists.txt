PROJECT(trunk)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.10.2)
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtSql REQUIRED)
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${QT_QTSQL_INCLUDE_DIR})

# Source files
SET(SOURCES main.cpp browser.cpp connectionwidget.cpp qsqlconnectiondialog.cpp)

# Header files
SET(HEADERS browser.h connectionwidget.h qsqlconnectiondialog.h)

# UI files
SET(FORMS browserwidget.ui qsqlconnectiondialog.ui)

# MOC-wrapper
QT4_WRAP_CPP(HEADERS_MOC ${HEADERS})

# UIC-wrapper
QT4_WRAP_UI(FORMS_HEADERS ${FORMS})

# The final steps...
ADD_EXECUTABLE(trunk ${SOURCES} ${HEADERS_MOC} ${FORMS_HEADERS})
TARGET_LINK_LIBRARIES(trunk ${QT_LIBRARIES})

# ...and the installer!
INSTALL(TARGETS trunk DESTINATION bin)
