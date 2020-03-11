#include <parser/parser.h>
#include <db/db.h>
#include <query/query.h>
#include <iostream>
using namespace bpl;
int main(){


  constexpr std::string_view dbmode_prompt = "|: ", querymode_prompt = "?- ";

  bool dbmode = true;
  db storage;

  for (std::string line; std::cout<<(dbmode ?dbmode_prompt : querymode_prompt), std::getline(std::cin, line);) {
    std::string_view cmd = parser::trim(line);
    if(cmd.empty())continue;
    if(cmd.front()=='\\'){
      cmd.remove_prefix(1);
      switch (cmd.front()){
        case'q':exit(0);
        case 's':dbmode^=1;
          break;
      }
    } else {
      if(dbmode){
        storage.insert_clause(parser::parse_clause(cmd));
      } else {
        //querymode
        query::execute_query(storage,parser::parse_term_list(cmd));
      }
    }
  }


}