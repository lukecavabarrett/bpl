#include <query/query.h>
#include <ds/environment.h>
#include <cassert>
#include <algorithm>
#include <iostream>

namespace bpl::query {

namespace {

void fill_vartable(std::unordered_map<std::string, uint32_t> &vartable, const parser::expression &expr, ds::environment &env) {
  if (expr.is_variable()) {
    auto it = vartable.find(expr.name);
    if (it != vartable.end())return;
    uint32_t id = env.new_element_pure();
    vartable[expr.name] = id;
  } else for (const parser::expression &sexpr : expr.args)fill_vartable(vartable, sexpr, env);
}

uint32_t make_insert_expr_id(std::unordered_map<std::string, uint32_t> &vartable, const parser::expression &expr, bpl::db &storage, ds::environment &env) {
  if (expr.is_variable())return vartable[expr.name];
  //function
  std::vector<uint32_t> children;
  for (const parser::expression &sexpr : expr.args)make_insert_expr_id(vartable, sexpr, storage, env);
  uint32_t fid = storage.function_id(expr.name,expr.args.size());
  uint32_t id = env.new_element_function(fid, std::move(children));
  return id;
}

}

struct term {
  uint32_t predicate_id;
  std::vector<uint32_t> args; //reference ufds world
  term() = default;
  term(const db::predicate::clause::rhs_item& it,uint32_t delta) : predicate_id(it.predicate_id) {
    args.reserve(it.args.size());
    for(const auto&x:it.args)args.push_back(delta+x);
  }
};

void print_follow(int i,int lt,const bpl::db &storage,ds::environment& env, const std::vector<std::string>& vars){
  const ds::environment::node_t& n = env.nodes[env.get_boss_id(i)];
  uint32_t min_id = n.topology & 0x7ffffffu;
  if(n.function_id==0 || min_id<lt){
    //pure or already described
    if(min_id<vars.size())std::cout<<vars[min_id];
    else std::cout<<"_"<<min_id;
    return;
  }
  //if(i==lt)--lt; //decrease lt from second iteration onwards
  std::cout << storage.functions[n.function_id].name;
  if(n.args_id.empty())return;
  std::cout << '(';
  for(int j = 0;j<n.args_id.size();++j){
    print_follow(n.args_id[j],lt,storage,env,vars);
    std::cout << (j==(n.args_id.size()-1) ? ')' : ',');
  }
}

void describe_solution(const bpl::db &storage,ds::environment& env, const std::vector<std::string>& vars){
  bool anypr = false;
  for(uint32_t i = 0; i < vars.size();i++){
    std::cout << vars[i] << " = ";
    print_follow(i,i,storage,env,vars);
    std::cout << std::endl;
    anypr=true;
  }
  if(!anypr)std::cout << "true."<<std::endl;
}

void rec_solve(const bpl::db &storage,std::vector<term>& goal_list,ds::environment& env, const std::vector<std::string>& vars){
  if(goal_list.empty()){
    std::cerr << "found solution" << std::endl;
    describe_solution(storage,env,vars);
    return;
  }
  term pr = std::move(goal_list.back()); goal_list.pop_back();
  auto ck = env.get_checkpoint();
  for(const db::predicate::clause& cl : storage.predicates[pr.predicate_id].clauses){
    uint32_t delta = env.nodes.size();
    if(env.unify_with_clause(pr.args,cl)){
      for(auto it = cl.rhs.crbegin();it!=cl.rhs.crend();++it)goal_list.emplace_back(*it, delta);
      auto n = cl.rhs.size();

      rec_solve(storage,goal_list,env,vars);

      goal_list.resize(goal_list.size()-n);
    }
    env.restore_checkpoint(ck);
  }
  goal_list.emplace_back(std::move(pr));
}

void execute_query(bpl::db &storage, std::vector<parser::term> query_goal) {
//generate env, and starting stack
// start similar to db::make_expr_id, but make variables have smallest ids
  std::unordered_map<std::string, uint32_t> vartable; // reference ufds world
  ds::environment env;
  for (const parser::term &term : query_goal)for (const parser::expression &expr : term.args)fill_vartable(vartable, expr, env);
  std::vector<std::string> vars(vartable.size());
  for(const auto& [s,i] : vartable)vars[i]=s;
  std::vector<term> goal_list;
  for (const parser::term &pterm : query_goal) {
    goal_list.emplace_back();
    term& t = goal_list.back();
    assert(storage.symbol_table.find(pterm.pred) != storage.symbol_table.end());
    assert(storage.symbol_table.find(pterm.pred)->second.type == db::symbol_descr::S_PREDICATE);
    t.predicate_id = storage.symbol_table.find(pterm.pred)->second.id;
    assert(storage.predicates[t.predicate_id].degree == pterm.args.size());
    for (const parser::expression &expr : pterm.args)t.args.push_back(make_insert_expr_id(vartable, expr, storage, env));
  }
  std::reverse(goal_list.begin(),goal_list.end());
  rec_solve(storage,goal_list,env,vars);
}
}