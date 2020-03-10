#include <parser/parser.h>
#include <cassert>

namespace bpl::parser {

namespace parsed {

inline std::string_view trim(std::string_view sv) {
  while (!sv.empty() && sv.front() <= 32)sv.remove_prefix(1);
  while (!sv.empty() && sv.back() <= 32)sv.remove_suffix(1);
  return sv;
}

int find_free(std::string_view &sv, char c) {
  int p = 0;
  for (int i = 0; i < sv.size(); ++i) {
    if (p == 0 && sv[i] == c)return i;
    if (sv[i] == '(')++p;
    if (sv[i] == ')')--p;
  }
  return sv.size();
}

expression parse_expression(std::string_view sv) {
  sv = trim(sv);
  int it = find_free(sv, '(');
  expression expr{.name=std::string(trim(sv.substr(0, it)))};
  if (sv.empty()) return expr;
  sv.remove_prefix(it);
  assert(sv.back() == ')');
  assert(sv.front() == '(');
  sv.remove_prefix(1);
  sv.remove_suffix(1);
  sv=trim(sv);
  while(!sv.empty()) {
    it = find_free(sv, ',');
    expr.args.push_back(parse_expression(sv.substr(0,it)));
    sv.remove_suffix(it);
    if(sv.front()==',')sv.remove_suffix(1);
    else assert(sv.empty());
  }
}

}

namespace ast {

}

}