/**
  * $Id: typemap.cpp,v 1.1 2004/03/03 10:56:06 philb Exp $
  */

#include "typemap.h"

QString getMysqlTypeName(int tabletype) {
	switch (tabletype) {
		case 0:
			return(QString("decimal"));
		case 4:
			return(QString("float"));
		case 3:
			return(QString("integer"));
		case 7:
			return(QString("timestamp"));
		case 10:
			return(QString("date"));
		case 12:
			return(QString("datetime"));
		case 252:
			return(QString("text"));
		case 253:
			return(QString("varchar"));
		case 254:
			return(QString("char"));
		default:
			return(QString("Unknown Type: ").append(QString::number(tabletype)));
	}
}

QString getPsqlTypeName(int tabletype) {
	switch (tabletype) {
		case 20:
			return(QString("bigint"));
		case 23:
			return(QString("integer"));
		case 25:
			return(QString("text"));
		case 701:
			return(QString("double"));
		case 790:
			return(QString("money"));
		case 1042:
			return(QString("char"));
		case 1043:
			return(QString("varchar"));
		case 1082:
			return(QString("date"));
		case 1114:
			return(QString("timestamp without timezone"));
		case 1184:
			return(QString("timestamp with timezone"));
		case 1700:
			return(QString("numeric"));
		default:
			return(QString("Unknown Type: ").append(QString::number(tabletype)));
	}
}

QString getTypeName(QString dbtype, int tabletype) {
	if (dbtype == "QMYSQL3") {
			return(getMysqlTypeName(tabletype));
	}

	if (dbtype == "QPSQL7") {
		return(getPsqlTypeName(tabletype));
	}

	return(QString("Unknown Type: ").append(QString::number(tabletype)));
}
