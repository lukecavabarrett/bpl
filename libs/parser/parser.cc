#include <parser/parser.h>
#include <cassert>

namespace bpl::parser {


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
  sv.remove_prefix(it);
  if (sv.empty()) return expr;
  assert(sv.back() == ')');
  assert(sv.front() == '(');
  sv.remove_prefix(1);
  sv.remove_suffix(1);
  sv=trim(sv);
  while(!sv.empty()) {
    it = find_free(sv, ',');
    expr.args.push_back(parse_expression(sv.substr(0,it)));
    sv.remove_prefix(it);
    if(sv.front()==',')sv.remove_prefix(1);
    else assert(sv.empty());
  }
  return expr;
}

term parse_term(std::string_view sv){
  expression e = parse_expression(sv);
  return term{.pred = e.name, .args=std::move(e.args)};
}

clause parse_clause(std::string_view sv){
  sv = trim(sv);
  assert(sv.back()=='.');
  sv.remove_suffix(1);
  int it = find_free(sv,':');
  clause c;
  if(it==sv.size()){
    //fact
    c.lhs = parse_term(sv);
  } else {
    //rule
    c.lhs = parse_term(sv.substr(0,it));
    sv.remove_prefix(it);
    assert(sv.front()==':');sv.remove_prefix(1);
    assert(sv.front()=='-');sv.remove_prefix(1);
    sv = trim(sv);
    while(!sv.empty()) {
      it = find_free(sv, ',');
      c.rhs.push_back(parse_term(sv.substr(0,it)));
      sv.remove_prefix(it);
      if(sv.front()==',')sv.remove_prefix(1);
      else assert(sv.empty());
    }
  }
  return c;
}



namespace ast {

}

bool expression::is_variable() const {
  return std::isupper(name.front());
}
}