/**
  * $Id: typemap.h,v 1.1 2004/03/03 10:56:09 philb Exp $
  */

#include <qstring.h>
#include <qsqldatabase.h>

QString getMysqlTypeName(int tabletype);
QString getPsqlTypeName(int tabletype);
QString getTypeName(QString dbtype, int tabletype);
