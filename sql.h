#pragma once

#include <string>

class sqlite3;

const int DEFINITION_TYPE = 0;
const int DECLARATION_TYPE = 1;
const int USAGE_TYPE = 2;

struct SourceRange {
   std::string filename;
   unsigned row_b, col_b;
   unsigned row_e, col_e;
};

void insertRow(sqlite3 * db, SourceRange range, int id, std::string data, int type);

int getDefinitionID(sqlite3 * db, SourceRange range);

int getNewDefinitionID(sqlite3 * db);

void createTableIfNotExists(sqlite3 * db);

void dropFileIndex(sqlite3 * db, std::string filename);

void dropBase(sqlite3 * db);
