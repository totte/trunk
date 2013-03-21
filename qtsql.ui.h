#include <qsqldatabase.h>
#include <qsqlrecord.h>
#include <qlistview.h>
#include <iostream>

#include <qsqldatabase.h>
#include <qsqlrecord.h>
#include <qsqlselectcursor.h>
#include <qlistview.h>
#include <qdatatable.h>
#include <qinputdialog.h>
#include "qtsql.h"

#include "images.h"


using namespace std;

void QtSql::setConfig( QMap <QString, QString> *conf) {

	QMap <QString, QString>::iterator it;

	for ( it=conf->begin(); it != conf->end(); it++ ) {
		configmap[it.key()] = it.data();
	}
}


void QtSql::addDB(QString cnctName) {
	if ( dbListView->findItem(cnctName, 0) == 0 ) {
		QListViewItem* root = new QListViewItem(dbListView, cnctName);
		root->setPixmap(0, QPixmap(database_xpm));
	}
}


void QtSql::addDBDetail(QListViewItem* dbnode) {

	QString cnctName = dbnode->text(0);
	QSqlDatabase* db = dbmap[cnctName];
	bool ok;

	if ( ! db->isOpen() ) {

		QString passwd = QInputDialog::getText( "QtSql", "Enter your password:", QLineEdit::Password, QString::null, &ok, this );

		if ( ! ok ) {
			return;
		}

		db->setPassword(passwd);

		if ( !db->open() ) {
			QMessageBox::information(this, "QtSql: Connection Failed.", db->lastError().text());
			return;
		}

		QString dbname = db->databaseName();	
		dbmap[cnctName] = db;

		QListViewItem* table;
		QListViewItem* view;

		QStringList tables = db->tables(QSql::Tables);
		QStringList views = db->tables(QSql::Views);

		for ( QStringList::Iterator it=tables.begin(); it != tables.end(); it++ ) {
			table = new QListViewItem(dbnode, *it);
			table->setPixmap(0, QPixmap(table_xpm));
		}

		// Open the item
		dbnode->setOpen(true);
		return;
	}
	return;
}


void QtSql::showTableInfo( QListViewItem* item) {

	if (item == NULL) {
		return;
	}
	
	if (item->depth() == 0) {
		if ( !dbmap[item->text(0)]->isOpen() ) {
			addDBDetail(item);
		}
		this->activeDBName = item->text(0);
		return;
	}

	QString connectionName = (item->parent())->text(0);
	QString driverName = dbmap[connectionName]->driverName();
	int tableTypeId;
	this->activeDBName = connectionName;

	// Clear the table
	QMemArray<int> qm(metaDataTable->numRows()-1);
	for (int i=0; i<qm.size(); i++) {
		qm[i]=i;
	}
	metaDataTable->removeRows(qm);
	
   QString tableName = item->text(0);
   QSqlRecordInfo tableInfo = dbmap[connectionName]->recordInfo(tableName);

   QSqlRecordInfo::iterator vit;
   
   metaDataTable->insertRows(1, tableInfo.size());
   int i=0;
   
   for (vit = tableInfo.begin(); vit != tableInfo.end(); vit++) {	   
	  tableTypeId = (*vit).typeID();
	  metaDataTable->setItem(i, 0, new QTableItem(metaDataTable, QTableItem::Never, (*vit).name()) );
	  metaDataTable->setItem(i, 1, new QTableItem(metaDataTable, QTableItem::Never, getTypeName(driverName, tableTypeId)) );
	  metaDataTable->setItem(i, 2, new QTableItem(metaDataTable, QTableItem::Never, QString::number((*vit).length())) );
	  metaDataTable->setItem(i, 3, new QTableItem(metaDataTable, QTableItem::Never, ((*vit).defaultValue()).toString() ) );

	  if ((*vit).isRequired()) {
		 metaDataTable->setItem(i, 4, new QTableItem(metaDataTable, QTableItem::Never, "N"));
	  } else {
		 metaDataTable->setItem(i, 4, new QTableItem(metaDataTable, QTableItem::Never, "Y"));
	  }
	  i++;
   }

	for (i=0; i<5; i++) {
		metaDataTable->adjustColumn(i);
	}

	contentsTable->setSqlCursor(new QSqlCursor(tableName, TRUE, dbmap[connectionName]), TRUE, TRUE);
	contentsTable->refresh(QDataTable::RefreshAll);
	resultsTabWidget->setCurrentPage(0);
}


void QtSql::about()
{
    QMessageBox::information(this, "QtSql",
			     tr("QtSql is a basic database navigator.\n"
				"It supports all database types supported by the underlying Qt implementation.\n"
				"This should include at least MySQL, PostgreSql and ODBC3.\n\n"
				"Developed by Phil Bradley\n"
				"Released: 3rd January 2005.\n")
			     );
}


void QtSql::newConnection()
{
	ConnectionTool *ac = new ConnectionTool(this);
	ac->setMode(0);
	ac->addDrivers();
	if (ac->exec() == QDialog::Accepted) {
		rescan();
	}
}


void QtSql::quit()
{
	int ret = QMessageBox::question(this, "QtSql: Confirm Exit", tr("Are you sure you wish to exit QtSql?"), QMessageBox::Yes, QMessageBox::No);

	if (ret == QMessageBox::Yes) {
		QMap <QString, QSqlDatabase*>::iterator it;
		for (it = dbmap.begin(); it != dbmap.end(); it++) {
			(*it)->close();
		}

		// Save history
		saveHistory();

		// Save dimensions

		exit(0);
	}
}


void QtSql::execute()
{    
	resultsTabWidget->setCurrentPage(2);

	QString qry = (queryEditor->text()).stripWhiteSpace();
	QString checkQry = qry.lower();

	if (activeDBName == "") {
		QMessageBox::information(this, "QtSql: SQL Error.", "Please select a connection.");
		return;
	}

	if (qry == "") {
	    QMessageBox::information(this, "QtSql: SQL Error.", "Please enter a query.");
	    return;
	}
	
	QSqlCursor *qryCrsr = new QSqlSelectCursor(qry, dbmap[activeDBName]);	
	resultsTable->setSqlCursor(qryCrsr, TRUE, TRUE);

	// Nasty hack, the cursor shouldn't even be able to execute non-selects but
	// it seems to. If the qry is an update/insert it will get re-executed when
	// the table refreshes. This is bad. If the qry consists of "select ..." 
	// then assume it's just a select and refresh the table
	if (checkQry.startsWith("select")) {
		resultsTable->refresh(QDataTable::RefreshAll);
	} else {
		// The datatable will automatically handle errors with a popup. If we are not 
		// using it, we need to explicitly warn the user.
		QSqlError err = qryCrsr->lastError();
		if (err.type() != QSqlError::None) {
			err.showMessage();
		}
	}

	if ( (historyList->findItem(qry) == 0)  && (qry != "") ){
		historyList->insertItem(qry);
	}
}


void QtSql::setSQLText()
{
    queryEditor->setText(historyList->currentText());
}


void QtSql::initialise() 
{
	QDomDocument doc( "mydocument" );
	QString configFileName = QString(".qtsql.xml");
	QString historyFileName = QString(".qt_history");

	this->homeDir = QString(getenv("HOME"));
	this->fqConfigFileName = homeDir + "/" + configFileName;
	this->fqHistoryFileName = homeDir + "/" + historyFileName;

	if (!QFile::exists(fqConfigFileName)) {
		QMessageBox::warning(this, tr("Configuration File Not Found"), tr("Configuration file %1 not found.\nA new config file will be created from a template.").arg(fqConfigFileName));

		if (! createSkeleton(fqConfigFileName)) {
			exit(2);
		}
	}

	QFile conf(fqConfigFileName);

	if ( !conf.open(IO_ReadOnly) ) {
		QMessageBox::critical(this, tr("Open failed"), tr("Warning: Error opening config file: %1").arg(conf.errorString()) );
		return;
	}

	QString errmsg;
	int errline;
	int errcol;

	if ( !doc.setContent(&conf, true, &errmsg, &errline, &errcol) ) {
		conf.close();
		cerr << "Error parsing config file at line: " << errline << ", col " << errcol << ": Msg: " << errmsg << endl;

		QString msgPrompt = QString("Error parsing config file\nMsg: %1").arg(errmsg);
		QMessageBox::information(this, "QtSql: Initialising", msgPrompt);
		return;
	}
	conf.close();

	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	QString cnctName;
	QString dbname;
	QString dbtype;
	QString dbuser; 
	QString dbpassword;
	QString dbhost;

	QString paramName;
	QString paramValue;

	while( !n.isNull() ) {
		QDomElement e = n.toElement(); // try to convert the node to an element.
		if( !e.isNull() ) {
			if ("connection" == e.tagName()) {
				cnctName = (e.attributeNode("name")).value();
				dbname = (e.attributeNode("dbname")).value();
				dbtype = (e.attributeNode("type")).value();
				dbuser = (e.attributeNode("username")).value();
				dbpassword = (e.attributeNode("password")).value();
				dbhost = (e.attributeNode("hostname")).value();

				// We might be rescanning, don't want to connect twice
				if (dbmap[cnctName] == NULL) {
					dbmap[cnctName] = QSqlDatabase::addDatabase(dbtype, dbname);
					dbmap[cnctName]->setDatabaseName( dbname );
					dbmap[cnctName]->setUserName( dbuser );
					dbmap[cnctName]->setPassword( dbpassword );
					dbmap[cnctName]->setHostName( dbhost );
				}
			}

			if ("param" == e.tagName()) {
				paramName = (e.attributeNode("name")).value();
				paramValue = (e.attributeNode("value")).value();

				if (configmap[paramName] == NULL) {
					configmap[paramName] = paramValue;
				}
			}

		} else {
			cout << "Warning: Null entry" << endl;
		}
		n = n.nextSibling();
	}

	QMap <QString, QSqlDatabase*>::iterator it;
	for (it = dbmap.begin(); it != dbmap.end(); it++) {
		addDB(it.key());
	}

	readHistory();
}


void QtSql::rescan()
{
    initialise();
}


void QtSql::editConnection()
{
	QListViewItem *target = dbListView->selectedItem();
	QString cname = target->text(0);
	ConnectionTool *ac = new ConnectionTool(this);

	ac->setMode(1); // Should be ConnectionTool::UPDATE
	ac->addDrivers();
	ac->populate( cname );

	if (ac->exec() == QDialog::Accepted) {
		(dbmap[cname])->close();
		dbmap.remove(cname);		// It will be added back in the rescan
		//dbListView->takeItem(target);
		//delete(target);
		rescan();
		target->setOpen(false);
	}
}


void QtSql::dbListView_contextMenuRequested( QListViewItem * item, const QPoint & location, int state)
{
	if (item == NULL) {
		return;
	}

	if (item->depth() > 0) {
		return;
	}

	QPopupMenu *contextMenu = new QPopupMenu(this);
	contextMenu->insertItem("Edit", this, SLOT(editConnection()));
	contextMenu->insertItem("Delete", this, SLOT(deleteConnection()));
	contextMenu->insertItem("Refresh", this, SLOT(refreshConnection()));
	contextMenu->popup(location);
}


void QtSql::deleteConnection()
{
	QListViewItem *target = dbListView->selectedItem();
	QString cname = target->text(0);

	int ret = QMessageBox::question(this, "QtSql: Confirm Delete", tr("Are you sure you wish to delete connection " + cname + " ?"), QMessageBox::Yes, QMessageBox::No);

	if (ret == QMessageBox::Yes) {
		ConnectionTool *ac = new ConnectionTool(this);
		ac->remove(cname);
		dbListView->takeItem(target);
		delete(target);
		dbmap.remove(cname);
	}
}

void QtSql::refreshConnection()
{
	QListViewItem *item = dbListView->selectedItem();
	QString cnctName = item->text(0);
	QSqlDatabase* db = dbmap[cnctName];

	if ( db->isOpen() ) {
		db->close();
	}

	QListViewItem *subItem = item->firstChild();
	QListViewItem *nextSubItem;
	
	while (subItem) {
		nextSubItem = subItem->nextSibling();
		item->takeItem(subItem);
		subItem = nextSubItem;
	}

	addDBDetail(item);
}


void QtSql::saveHistory()
{

	int cnt = historyList->numRows(); 
	int maxHistory = ((configmap["maxHistory"]) != NULL) ?  (configmap["maxHistory"]).toInt() : 100;
	int maxCnt = (cnt > maxHistory) ? maxHistory : cnt;

	QFile conf(fqHistoryFileName);
	
	if ( !conf.open(IO_WriteOnly|IO_Truncate) ) {
		QMessageBox::critical(this, tr("Open failed"), tr("Error opening history file: %1").arg(conf.errorString()) );
		return;
	}

	QTextStream stream( &conf );
	QString* item;

	for (int i=maxCnt-1; i>=0; i--) {
		item = new QString(historyList->text(i));	
		stream << item->replace("\n", " ") << endl;
	} 

	conf.close();
}

bool QtSql::createSkeleton(QString fileName) 
{
	const char* qtSQLSkeletonConfig = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?><config> <param name=\"maxHistory\" value=\"100\"/> </config>";

	QFile conf(fileName);
	
	if ( !conf.open(IO_WriteOnly|IO_Truncate) ) {
		QMessageBox::critical(this, tr("Open failed"), tr("Error opening config file: %1").arg(conf.errorString()) );
		return(false);
	}

	QTextStream stream( &conf );
	stream << qtSQLSkeletonConfig << endl;
	conf.close();
	return(true);
}

void QtSql::readHistory()
{
	QFile history(fqHistoryFileName);

	if (!QFile::exists(fqHistoryFileName)) {
		return;
	}

	if ( !history.open(IO_ReadOnly) ) {
		QMessageBox::warning(this, tr("Open failed"), tr("Warning: Could not open history file: %1").arg(history.errorString()) );
		return;
	}

	historyList->clear();

	QTextStream stream( &history );
	QString line;

	while ( !stream.atEnd() ) {
		line = stream.readLine(); // line of text excluding '\n'
		historyList->insertItem(line);
	}
	history.close();
}


void QtSql::getSQLFileName()
{
    QFileDialog* fd = new QFileDialog( this, "file dialog", TRUE );
    fd->setMode( QFileDialog::ExistingFile );
    fd->setFilter( "All Files (*.*)" );
    fd->addFilter( "SQL Files (*.sql)" );
    
    QString fileName;
    
    if ( fd->exec() == QDialog::Accepted ) {
	fileName = fd->selectedFile();
    }
    
    cout << "Reading From:  " << fileName << endl;
    
    QFile savefile(fileName);

    if (!QFile::exists(fileName)) {
	return;
    }

    if ( !savefile.open(IO_ReadOnly) ) {
	QMessageBox::warning(this, tr("Open failed"), tr("Warning: Could not open file: %1").arg(savefile.errorString()) );
	return;
    }
  
    QTextStream stream( &savefile );
    QString line;

    while ( !stream.atEnd() ) {
	line.append(stream.readLine()).append("\n");
    }
    savefile.close();
    queryEditor->clear();
    queryEditor->setText(line);
}

void QtSql::saveResult()
{
    QString fileName = QFileDialog::getSaveFileName(
                    getenv("HOME"),
                    "CSV (*.csv)",
                    this,
                    "save file dialog"
                    "Choose a filename to save under" );
   
    QFile savefile(fileName);
    
    if ( !savefile.open(IO_WriteOnly|IO_Truncate) ) {
	QMessageBox::warning(this, tr("Open failed"), tr("Warning: Could not open file: %1 for writing").arg(savefile.errorString()) );
	return;
    }
        
    QTextStream stream( &savefile );
	
    int row=0;
    int col=0;
    QString line;
    
    for (row=0; row<resultsTable->numRows(); row++) {
	line.truncate(0);
	for (col=0; col<resultsTable->numCols(); col++) {
	    line.append(resultsTable->text(row, col));
	    line.append("\t");
	}
	stream << line << endl;	
    }
    
    savefile.close();
    QMessageBox::information(this, "QtSql: Successfully Saved Results.", "Successfully Saved Results");
}


