#ifndef BPL_LIBS_PARSER_INCLUDE_PARSE_PARSE_H_
#define BPL_LIBS_PARSER_INCLUDE_PARSE_PARSE_H_

#include <string>
#include <vector>
namespace bpl::parser{
std::string_view trim(std::string_view);

struct expression {
  std::string name;
  std::vector<expression> args;
  bool is_variable() const;
};

struct term{
  std::string pred;
  std::vector<expression> args;
};
struct clause{
  term lhs;
  std::vector<term> rhs;
};


expression parse_expression(std::string_view sv);
term parse_term(std::string_view sv);
std::vector<term> parse_term_list(std::string_view sv);
clause parse_clause(std::string_view sv);

}

#endif //BPL_LIBS_PARSER_INCLUDE_PARSE_PARSE_H_
