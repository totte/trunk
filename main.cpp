/**
  * $Id: main.cpp,v 1.1 2004/03/03 10:55:59 philb Exp philb $
  */

using namespace std;
#include <iostream>

#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqlrecord.h>
#include <qlistview.h>
#include <qpixmap.h>
#include <qtable.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qxml.h>
#include <qdom.h>

#include "qtsql.h"



//QMap <int, QString> typemap();


int main( int argc, char ** argv ) {

	QApplication app( argc, argv );

	QtSql w;
	w.initialise();
	w.show();

	app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	app.connect(w.dbListView, SIGNAL(doubleClicked(QListViewItem*)), &w, SLOT(showTableInfo(QListViewItem*)) );
	app.connect(w.dbListView, SIGNAL(clicked(QListViewItem*)), &w, SLOT(showTableInfo(QListViewItem*)) );
	app.connect(w.dbListView, SIGNAL(returnPressed(QListViewItem*)), &w, SLOT(showTableInfo(QListViewItem*)) );
	//app.connect(w.dbListView, SIGNAL(clicked(QListViewItem*)), &w, SLOT(showDBInfo(QListViewItem*)) );

	return(app.exec());
}
