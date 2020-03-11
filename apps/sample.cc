#include <parser/parser.h>
#include <db/db.h>
#include <query/query.h>
#include <iostream>
using namespace bpl;
int main(){

  db storage;
  storage.insert_clause(parser::parse_clause("true."));
  query::execute_query(storage,parser::parse_term_list("true."));
}