#include <db/db.h>
#include <cassert>
#include <iostream>

namespace bpl{


db::db() {
  functions.push_back({.name="__pure_placeholder"});
}

int db::function_id(std::string_view name, int degree) {
  auto it = symbol_table.find(std::string(name));
  if(it!=symbol_table.end()){
    //already existing
    assert(it->second.type==symbol_descr::S_FUNCTION);//avoid redeclaration
    int fid = it->second.id;
    assert(functions[fid].degree==degree);//cannot redeclare with different degree
    return fid;
  }
  int fid = functions.size();
  functions.push_back({.name=std::string(name),.degree=degree});
  if(degree)std::cerr<<"introduced function "<<name<<"/"<<degree<<"."<<std::endl;
  else std::cerr<<"introduced constant "<<name<<"."<<std::endl;
  symbol_table[std::string(name)]=symbol_descr{.type=symbol_descr::S_FUNCTION,.id=fid};
  return fid;
}

int db::predicate_id(std::string_view name, int degree) {
  auto it = symbol_table.find(std::string(name));
  if(it!=symbol_table.end()){
    //already existing
    assert(it->second.type==symbol_descr::S_PREDICATE);//avoid redeclaration
    int pid = it->second.id;
    assert(predicates[pid].degree==degree);//cannot redeclare with different degree
    return pid;
  }
  int pid = predicates.size();
  predicates.push_back({.name=std::string(name),.degree=degree});
  std::cerr<<"introduced predicate "<<name;if(degree)std::cerr<<"/"<<degree;std::cerr<<"."<<std::endl;
  symbol_table[std::string(name)]=symbol_descr{.type=symbol_descr::S_PREDICATE,.id=pid};
  return pid;
}

typedef db::predicate::clause::item clitem;
int db::make_expr_id(std::vector<clitem> &items,std::unordered_map<std::string,int>& vartable,const parser::expression& expr){
  if(expr.is_variable()){
    //lookup
    assert(expr.args.empty());// variables are not functions
    if(vartable.find(expr.name)==vartable.end()){
      //make new
      int vid = items.size();
      vartable[expr.name]=vid;
      items.push_back({.function_id=0});
      return vid;
    } else {
      //use linked
      return vartable.find(expr.name)->second;
    }
  } else {
    //expr is function
    std::vector<int> children_ids;
    for(const parser::expression& child : expr.args)children_ids.push_back(make_expr_id(items,vartable,child));
    int fid = function_id(expr.name,expr.args.size());
    int eid = items.size();
    items.push_back({.function_id=fid,.args=std::move(children_ids)});
    return eid;
  }
}

void db::insert_clause(const parser::clause& c) {
  int declarer_pid = predicate_id(c.lhs.pred,c.lhs.args.size());
  predicates[declarer_pid].clauses.emplace_back();
  predicate::clause& cl = predicates[declarer_pid].clauses.back();
  std::unordered_map<std::string,int> variables_table; //link to items
  //fill cl.lhs
  for(const parser::expression& expr : c.lhs.args)cl.lhs_args.push_back(make_expr_id(cl.items,variables_table,expr));
  //fill cl.rhs
  for(const parser::term& term : c.rhs){
    cl.rhs.emplace_back();
    predicate::clause::rhs_item& rhs_item = cl.rhs.back();
    rhs_item.predicate_id = predicate_id(term.pred,term.args.size());
    for(const parser::expression& expr : term.args)rhs_item.args.push_back(make_expr_id(cl.items,variables_table,expr));
  }
}

}

