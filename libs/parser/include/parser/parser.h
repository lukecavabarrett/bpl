#ifndef BPL_LIBS_PARSER_INCLUDE_PARSE_PARSE_H_
#define BPL_LIBS_PARSER_INCLUDE_PARSE_PARSE_H_

#include <string>
#include <vector>
namespace bpl::parser{

namespace parsed {
struct expression {
  std::string name;
  std::vector<expression> args;
};

struct term{
  std::string pred;
  std::vector<expression> args;
};
struct clause{
  term lhs;
  std::vector<term> rhs;
};
}



namespace ast{

}

}

#endif //BPL_LIBS_PARSER_INCLUDE_PARSE_PARSE_H_
