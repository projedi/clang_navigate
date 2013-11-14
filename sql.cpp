#include "sql.h"

#include <iostream>
#include <sstream>

void insertRow(sqlite3 * db, SourceRange range, int id, std::string data, int type) {
   std::ostringstream query;
   sqlite3_stmt * stmt;
   int rc;

   query << "INSERT INTO items (id, file, row_b, col_b, row_e, col_e, type, data) VALUES " <<
   "(" << id << ","
       << "'" << range.filename << "' ,"
       << range.row_b << "," << range.col_b << ","
       << range.row_e << "," << range.col_e << ","
       << type << ","
       << "'" << data << "' )";

   //std::cerr << query.str() << std::endl;
   rc = sqlite3_prepare( db, query.str().c_str(), -1, &stmt, NULL );
   if (rc != SQLITE_OK) {
      std::cerr << "sqlite3_prepare[" << rc << "] " << sqlite3_errmsg(db) << " " << sqlite3_errcode(db) << std::endl << query.str() << std::endl;
      sqlite3_finalize( stmt );
      return;
   }
   rc = sqlite3_step( stmt );
   if (rc != SQLITE_DONE) {
      std::cerr << "sqlite3_step[" << rc << "] " << sqlite3_errmsg(db) << " " << sqlite3_errcode(db) << std::endl << query.str() << std::endl;
      sqlite3_finalize( stmt );
      return;
   }
   sqlite3_finalize( stmt );
}

int getDefinitionID(sqlite3 * db, SourceRange range) {
   std::ostringstream query;
   sqlite3_stmt * stmt;
   int rc;

   query << "SELECT id FROM items WHERE" <<
      " file = '" << range.filename << "'" <<
      " and row_b = " << range.row_b <<
      " and col_b = " << range.col_b <<
      " and row_e = " << range.row_e <<
      " and col_e = " << range.col_e <<
      " LIMIT 1";
   //std::cerr << query.str() << std::endl;

   rc = sqlite3_prepare( db, query.str().c_str(), -1, &stmt, NULL );
   if (rc != SQLITE_OK) {
      std::cerr << "sqlite3_prepare[" << rc << "] " << sqlite3_errmsg(db) << " " << sqlite3_errcode(db) << std::endl << query.str() << std::endl;
      sqlite3_finalize( stmt );
      return -1;
   }
   rc = sqlite3_step( stmt );
   if (rc == SQLITE_DONE) {
      return -1;
   }
   if (rc != SQLITE_ROW) {
      std::cerr << "sqlite3_step[" << rc << "] " << sqlite3_errmsg(db) << " " << sqlite3_errcode(db) << std::endl << query.str() << std::endl;
      sqlite3_finalize( stmt );
      return -1;
   }
   int id = sqlite3_column_int( stmt, 0 );
   sqlite3_finalize( stmt );
   return id;
}

int getNewDefinitionID(sqlite3 * db) {
   sqlite3_stmt * stmt;
   int rc;

   std::ostringstream query;
   query << "SELECT id FROM items ORDER BY id DESC LIMIT 1";
   //std::cerr << query.str() << std::endl;

   rc = sqlite3_prepare( db, query.str().c_str(), -1, &stmt, NULL );
   if (rc != SQLITE_OK) {
      std::cerr << "sqlite3_prepare[" << rc << "] " << sqlite3_errmsg(db) << " " << sqlite3_errcode(db) << std::endl << query.str() << std::endl;
      sqlite3_finalize( stmt );
      return -1;
   }
   rc = sqlite3_step( stmt );
   if (rc == SQLITE_DONE) {
      return 0;
   }
   if (rc != SQLITE_ROW) {
      std::cerr << "sqlite3_step[" << rc << "] " << sqlite3_errmsg(db) << " " << sqlite3_errcode(db) << std::endl << query.str() << std::endl;
      sqlite3_finalize( stmt );
      return -1;
   }
   int id = sqlite3_column_int( stmt, 0 );
   sqlite3_finalize( stmt );
   return id + 1;
}

void createTableIfNotExists(sqlite3 * db) {
   sqlite3_stmt * stmt;
   int rc;

   std::ostringstream query;
   query << "CREATE TABLE IF NOT EXISTS items (" <<
            " id INT NOT NULL," <<
            " file VARCHAR(255) NOT NULL," <<
            " row_b INT NOT NULL, col_b INT NOT NULL," <<
            " row_e INT NOT NULL, col_e INT NOT NULL," <<
            " type INT NOT NULL," <<
            " data TEXT NOT NULL );" <<
            " CREATE INDEX IF NOT EXISTS items_id_idx ON items (id);" <<
            " CREATE INDEX IF NOT EXISTS items_pos_idx ON items (file, row_b, col_b);";
   //std::cerr << query.str() << std::endl;

   rc = sqlite3_prepare( db, query.str().c_str(), -1, &stmt, NULL );
   if (rc != SQLITE_OK) {
      std::cerr << "sqlite3_prepare[" << rc << "] " << sqlite3_errmsg(db) << " " << sqlite3_errcode(db) << std::endl << query.str() << std::endl;
      sqlite3_finalize( stmt );
      return;
   }
   rc = sqlite3_step( stmt );
   if (rc != SQLITE_DONE) {
      std::cerr << "sqlite3_step[" << rc << "] " << sqlite3_errmsg(db) << " " << sqlite3_errcode(db) << std::endl << query.str() << std::endl;
      sqlite3_finalize( stmt );
      return;
   }
   sqlite3_finalize( stmt );
   return;
}

void dropFileIndex(sqlite3 * db, std::string filename) {
   sqlite3_stmt * stmt;
   int rc;

   std::ostringstream query;
   query << "DELETE FROM items WHERE file = '" << filename << "'";
   //std::cerr << query.str() << std::endl;

   rc = sqlite3_prepare( db, query.str().c_str(), -1, &stmt, NULL );
   if (rc != SQLITE_OK) {
      std::cerr << "sqlite3_prepare[" << rc << "] " << sqlite3_errmsg(db) << " " << sqlite3_errcode(db) << std::endl << query.str() << std::endl;
      sqlite3_finalize( stmt );
      return;
   }
   rc = sqlite3_step( stmt );
   if (rc != SQLITE_DONE) {
      std::cerr << "sqlite3_step[" << rc << "] " << sqlite3_errmsg(db) << " " << sqlite3_errcode(db) << std::endl << query.str() << std::endl;
      sqlite3_finalize( stmt );
      return;
   }
   sqlite3_finalize( stmt );
   return;
}
