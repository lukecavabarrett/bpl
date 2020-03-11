#include <ds/environment.h>
#include <cassert>
#include <iostream>

namespace bpl::ds {

constexpr uint32_t BOSS_FLAG = uint32_t(1) << 31;
inline void environment::log_previous(uint32_t node_id, uint32_t previous) {
  history.push_back((static_cast<uint64_t>(node_id) << 32) | static_cast<uint64_t>(previous));
}
namespace {
struct decomposed_topology {
  int rank;
  uint32_t min_id;
};

decomposed_topology decompose_topology(uint32_t t) {
  return decomposed_topology{.rank=static_cast<int>(t >> 27), .min_id=static_cast<uint32_t>(t & 0x7ffffffu)};
}

uint32_t recompose_topology(decomposed_topology d) {
  return (static_cast<uint32_t>(d.rank) << 27) | (static_cast<uint32_t>(d.min_id) & 0x7ffffffu);
}

}

uint32_t environment::get_boss_id(uint32_t id) {
  uint64_t t = nodes[id].topology;
  if (t & BOSS_FLAG)return id;
  uint32_t b = get_boss_id(t);
  if (b != t) {
    log_previous(id, t);
    nodes[id].topology = t;
  }
  return t;
}



bool environment::unify(uint32_t x, uint32_t y) {
  x = get_boss_id(x);
  y = get_boss_id(y);
  if (x == y)return true;//already unified
  if (nodes[x].function_id == nodes[y].function_id) {
    if (nodes[x].function_id) {
      // unify two functions of the same kind
      bool ok = true;
      for (auto itx = nodes[x].args_id.cbegin(), ity = nodes[y].args_id.cbegin(); itx != nodes[x].args_id.cend(); ++itx, ++ity) {
        assert(ity != nodes[y].args_id.cend());
        ok &= unify(*itx, *ity);
        if (!ok)break;
      }
      return ok;
    } else {
      // unify two variables
      auto dx = decompose_topology(nodes[x].topology), dy = decompose_topology(nodes[y].topology);
      if (dx.rank < dy.rank) {
        std::swap(x, y);
        std::swap(dx, dy);
      }
      if (dx.rank == dy.rank && dx.rank < 0xff)dx.rank++;
      if (dy.min_id < dx.min_id)dx.min_id = dy.min_id;
      //y changed
      log_previous(y, nodes[y].topology);
      nodes[y].topology = x;
      //x can be changed
      uint32_t tx = recompose_topology(dx);
      if (tx != nodes[x].topology) {
        log_previous(x, nodes[x].topology);
        nodes[x].topology = tx;
      }
      return true;
    }
  } else {
    if (nodes[y].function_id) {
      if (nodes[x].function_id)return false;//cannot unify two different functions
      std::swap(x, y);
    }
    // unify variable y with function x
    //TODO: improve this union (rank not satisfied)
    auto dx = decompose_topology(nodes[x].topology), dy = decompose_topology(nodes[y].topology);
    if (dx.rank < dy.rank)dx.rank = dy.rank;
    else if (dx.rank == dy.rank && dx.rank < 0xff)dx.rank++;
    if (dy.min_id < dx.min_id)dx.min_id = dy.min_id;
    //y changed
    log_previous(y, nodes[y].topology);
    nodes[y].topology = x;
    //x can be changed
    uint32_t tx = recompose_topology(dx);
    if (tx != nodes[x].topology) {
      log_previous(x, nodes[x].topology);
      nodes[x].topology = tx;
    }
    return true;
  }
}

environment::checkpoint_t environment::get_checkpoint() {
  return (static_cast<uint64_t>(nodes.size()) << 32) | static_cast<uint64_t>(history.size());
}
void environment::restore_checkpoint(environment::checkpoint_t ck) {
  uint64_t history_sz = ck & 0xffffffff;
  while (history.size() != history_sz) {
    uint32_t id = history.back() >> 32, v = history.back() & 0xffffffff;
    history.pop_back();
    nodes[id].topology = v;
  }
  uint32_t nodes_sz = ck >> 32;
  nodes.resize(nodes_sz);
}
bool environment::unify_with_clause(const std::vector<uint32_t> &lhs, const db::predicate::clause &cl) {
  uint32_t delta = nodes.size();
  //1. add all the new variables
  nodes.reserve(nodes.size()+cl.items.size());
  for(const auto& item : cl.items){
    if(item.function_id)nodes.emplace_back(nodes.size()-delta,delta,item.args,item.function_id);
    else nodes.emplace_back(nodes.size()-delta,delta);
  }
  //2. proceed with unification
  bool ok = true;
  for(int i = 0;i<lhs.size();++i){
    ok&=unify(lhs[i],cl.lhs_args[i]+delta);
    if(!ok)break;
  }
  return ok;
}
uint32_t environment::new_element_pure() {
  int id = nodes.size();
  nodes.emplace_back(id,0);
  return id;
}
uint32_t environment::new_element_function(uint32_t fid, std::vector<uint32_t> &&args) {
  int id = nodes.size();
  nodes.emplace_back(id,std::move(args),fid);
  return id;
}

environment::node_t::node_t(uint32_t pure_id,uint32_t delta) : function_id(0), topology(BOSS_FLAG | (pure_id+delta)) {}

namespace {
std::vector<uint32_t> make_increased_vector( const std::vector<int> &original,uint32_t delta){
  std::vector<uint32_t> ret(original.begin(),original.end());
  for(auto&x:ret)x+=delta;
  return ret;
}

}

environment::node_t::node_t(uint32_t pure_id,uint32_t delta, const std::vector<int> &original, const uint32_t fid) : function_id(fid),topology(BOSS_FLAG | (pure_id+delta)),args_id(make_increased_vector(original,delta)) {}
environment::node_t::node_t(uint32_t pure_id, std::vector<uint32_t> &&original, const uint32_t fid) : function_id(fid),topology(BOSS_FLAG | (pure_id)),args_id(std::move(original)) {}
environment::node_t::node_t() : function_id(0) { std::cerr << "Cant call default contructor of a node" << std::endl; throw;}
}