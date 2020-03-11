#include <parser/parser.h>
#include <db/db.h>
#include <query/query.h>
#include <iostream>
using namespace bpl;

void geqtest(){
  db storage;
  storage.insert_clause(parser::parse_clause("geq(X,X)."));
  storage.insert_clause(parser::parse_clause("geq(succ(X),Y) :- geq(X,Y)."));
  //bugs:
  query::execute_query(storage,parser::parse_term_list("geq(X,0)."));
  //query::execute_query(storage,parser::parse_term_list("geq(X,0)."));

  //unifying (X,X) with (X,a) doesn't produce the desired result
}

void eqtest(){
  db storage;
  storage.insert_clause(parser::parse_clause("eq(X,X)."));
  //bugs:
  query::execute_query(storage,parser::parse_term_list("eq(X,f(g))."));
  //query::execute_query(storage,parser::parse_term_list("geq(X,0)."));

  //unifying (X,X) with (X,a) doesn't produce the desired result
}

void occurscheck_test(){
  db storage;
  storage.insert_clause(parser::parse_clause("eq(X,X)."));
  //bugs:
  query::execute_query(storage,parser::parse_term_list("eq(X,f(X))."));

}

void print_test(){
  db storage;
  storage.insert_clause(parser::parse_clause("eq(A,A)."));
  query::execute_query(storage,parser::parse_term_list("eq(X,X)."));
}

void test2(){
  db storage;
  storage.insert_clause(parser::parse_clause("eq(A,A)."));
  query::execute_query(storage,parser::parse_term_list("eq(X,Y), eq(X,1)."));
}


int main(){
  db storage;
  storage.insert_clause(parser::parse_clause("eq(A,A)."));
  query::execute_query(storage,parser::parse_term_list("eq([1,2,3],X)."));
}