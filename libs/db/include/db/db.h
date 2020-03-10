#ifndef BPL_LIBS_DB_INCLUDE_DB_DB_H_
#define BPL_LIBS_DB_INCLUDE_DB_DB_H_

#include <parser/parser.h>
#include <unordered_map>

namespace bpl {

class db {

 public:
  struct symbol_descr {
    enum symbol_type { S_FUNCTION, S_PREDICATE };
    symbol_type type;
    int id;
  };

  std::unordered_map<std::string, symbol_descr> symbol_table;

  //function ids start from 1
  struct function {
    std::string name;
    int degree;
  };

  std::vector<function> functions;
  int function_id(std::string_view name,int degree);

  struct predicate {
    std::string name;
    int degree;
    struct clause{
      struct item{
        int function_id; //references functions; 0 means pure variable
        std::vector<int> args; //references items
      };
      std::vector<item> items;
      std::vector<int> lhs_args; //each one references items
      struct rhs_item{
        int predicate_id;
        std::vector<int> args; //reference items
      };
      std::vector<rhs_item> rhs;
    };
    std::vector<clause> clauses;
  };
  int make_expr_id(std::vector<db::predicate::clause::item> &items,std::unordered_map<std::string,int>& vartable,const parser::expression& expr);

  std::vector<predicate> predicates;
  int predicate_id(std::string_view name,int degree);

  explicit db();
  void insert_clause(const parser::clause&);

};

}
#endif //BPL_LIBS_DB_INCLUDE_DB_DB_H_