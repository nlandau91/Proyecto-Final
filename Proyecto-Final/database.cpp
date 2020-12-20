#include "database.h"

database::database()
{
    const QString DRIVER("QSQLITE");
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
         QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);
    }
}
