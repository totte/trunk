/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


using namespace std;

enum mode {CREATE=0, UPDATE=1, DELETE=2};

QDomDocument* ConnectionTool::readConfig() {

	QDomDocument* doc = new QDomDocument( "config" );
	QString homeDir(getenv("HOME"));
	QFile conf(homeDir + "/.qtsql.xml");

	if ( !conf.open(IO_ReadOnly) ) {
		QMessageBox::critical(this, tr("Open failed"), tr("Could not open file '" + conf.name() + "' for reading: %1").arg(conf.errorString()) );
		return(NULL);
	}

	QString xmlErrMsg;
	QString msgPrompt;
	int line;
	int col;

	if ( !doc->setContent(&conf, true, &xmlErrMsg, &line, &col) ) {
		conf.close();
		msgPrompt = QString("Error parsing config file\nMsg: %3").arg(xmlErrMsg);
		QMessageBox::information(this, "QtSql: Connection Tool", msgPrompt);
		return(NULL);
	}
	conf.close();

	return(doc);
}

void ConnectionTool::writeConfig(QDomDocument* doc) {
	QString homeDir(getenv("HOME"));
	QFile conf(homeDir + "/.qtsql.xml");
   
	if ( !conf.open(IO_WriteOnly | IO_Truncate) ) {
		QMessageBox::critical(this, tr("Open failed"), tr("Could not open file '" + conf.name() + "' for writing: %1").arg(conf.errorString()) );
		return;
	}
    
	QTextStream stream( &conf );
	stream << doc->toString();
	conf.close(); 
cout << "done." << endl;
}

void ConnectionTool::populate(QString cname) {
	QDomDocument* config = readConfig();
	QDomElement docElem = config->documentElement();
	QDomElement e;
	QString cnctName;
	bool found;

	found = getConnectionByName( cname, config, &e );

	if ( ! found ) {
		QMessageBox::information(this, "QtSql: Connection Tool", "Error: Could not locate connection '" + cnctName + "'");
		return;
	}

	originalConnectionName = e.attributeNode("name").value();
	editCnctName->setText(e.attributeNode("name").value());
	editCnctName->setReadOnly(true);

	editDBName->setText(e.attributeNode("dbname").value());
	editUserName->setText(e.attributeNode("username").value());
	editHostName->setText(e.attributeNode("hostname").value());
	//comboCnctType->currentText();

}

void ConnectionTool::saveConnection() {
	QDomDocument* config = readConfig();
	QDomElement target;

	if (config == NULL) {
		QMessageBox::information(this, "QtSql: Connection Tool", "Saving The Connection Failed.");
		return;
	}

	QDomElement root = config->documentElement();

	QString cnctName = (editCnctName->text()).stripWhiteSpace();
	QString dbname = (editDBName->text()).stripWhiteSpace();
	QString dbusername = (editUserName->text()).stripWhiteSpace();
	QString dbhost = (editHostName->text()).stripWhiteSpace();
	QString dbtype = comboCnctType->currentText();

	if ( (cnctName == "") || (dbname == "") || (dbhost == "") || (dbusername == "") ) {
		QMessageBox::information(this, "QtSql: Connection Tool", "Please Supply a Connection Name, DB name, Hostname and Username.");
		return;
	}

	QDomElement elem = config->createElement( "connection" );
	elem.setAttribute( "name", cnctName );
	elem.setAttribute( "dbname", dbname );
	elem.setAttribute( "username", dbusername );
	elem.setAttribute( "type", dbtype );
	elem.setAttribute( "hostname", dbhost );

	// Should be using enum
	switch(dlg_mode) {
		case 0:
			// Create a new connection
			root.appendChild( elem );
			break;
		case 1:
			if (getConnectionByName( originalConnectionName, config, &target )) {
				root.replaceChild( elem, target );
			}
			break;
		default:
			return;
	}

	writeConfig(config); 
	this->accept();
}

void ConnectionTool::testConnection() {
	QString cnctName = (editCnctName->text()).stripWhiteSpace();
	QString dbname = (editDBName->text()).stripWhiteSpace();
	QString dbhost = (editHostName->text()).stripWhiteSpace();
	QString dbusername = (editUserName->text()).stripWhiteSpace();
	QString dbtype = comboCnctType->currentText();

	if ( (dbname == "") || (dbhost == "") || (dbusername == "") ) {
		QMessageBox::information(this, "QtSql: Connection Tool", "Please Supply a DB name, Hostname and Username.");
		return;
	}
	
	QSqlDatabase* db = QSqlDatabase::addDatabase(dbtype, dbname);
	db->setDatabaseName( dbname );
	db->setUserName( dbusername );
	db->setHostName( dbhost );

	if ( db->open() ) {
		QMessageBox::information(this, "QtSql: Connection Successful",  "Connection Successful");
	} else {
		QMessageBox::information(this, "QtSql: Connection Failed.", db->lastError().text());
	}
			
}


void ConnectionTool::addDrivers() {
    QStringList list = QSqlDatabase::drivers();
    comboCnctType->insertStringList(list);
}

int ConnectionTool::getMode() {
    return(dlg_mode);
}


void ConnectionTool::setMode( int mode ) {
    dlg_mode = mode;
}

void ConnectionTool::remove( QString cname ) {
	QDomDocument* config = readConfig();
	QDomElement docElem = config->documentElement();
	QDomElement e;
	QString cnctName;
	bool found;

	found = getConnectionByName( cname, config, &e );

	if (found) {
		docElem.removeChild(e);
	} else {
		QMessageBox::information(this, "QtSql: Connection Tool", "Error: Could not locate connection '" + cname + "'");
		return;
	}
	
	writeConfig(config); 
}


bool ConnectionTool::getConnectionByName( QString name, QDomDocument* doc, QDomElement* e ) {
	QDomElement docElem = doc->documentElement();
	QDomNode n = docElem.firstChild();
	QString cnctName;
	bool found = false;

	// Search for the node by name
	while( !n.isNull() ) {
		*e = n.toElement();
		if (!e->isNull()) {
			if (e->tagName() == "connection") {
				cnctName = (e->attributeNode("name")).value();
				if (cnctName == name) {
					found = true;
					break;
				}
			}
		}
		n = n.nextSibling();
	}

	return(found);
}
