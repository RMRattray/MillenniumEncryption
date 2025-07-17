#include <iostream>
#include <sqlite3.h>

int main(int argc, char **argv)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("SCRAPE.db", &db);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
    }
    std::cout << "This application is the server for the Millennium Encryption system!" << std::endl;
    return 0;
}