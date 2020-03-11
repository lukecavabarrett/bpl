#ifndef BPL_LIBS_DS_INCLUDE_DS_ENVIRONMENT_H_
#define BPL_LIBS_DS_INCLUDE_DS_ENVIRONMENT_H_

#include <cstdint>
#include <vector>
#include <db/db.h>

namespace bpl::ds {

class environment {
 public:
  struct node_t {
    //TODO: later, define sensible sizes for everything and pack well these structs

    uint32_t topology;//MSB is 0 -> father_id; MSB 1 -> 4/ rank id, 27/ min_id
    const uint32_t function_id;
    const std::vector<uint32_t> args_id;
    node_t(uint32_t pure_id,uint32_t delta); // pure constructor
    node_t(uint32_t delta,uint32_t pure_id,const std::vector<int>& original, const uint32_t fid);
    node_t(uint32_t pure_id,std::vector<uint32_t>&& original, const uint32_t fid);
    node_t();
  };
  typedef uint64_t topology_change; // 32/ node_id, 32/ previous topology value
  std::vector<node_t> nodes;
  std::vector<topology_change> history;
  inline void log_previous(uint32_t node_id,uint32_t previous);
  typedef uint64_t checkpoint_t; // 32/ nnodes, 32/ topology changes

  environment() = default;
  uint32_t get_boss_id(uint32_t);
  bool unify(uint32_t, uint32_t);
  checkpoint_t get_checkpoint();
  void restore_checkpoint(checkpoint_t);

  bool unify_with_clause(const std::vector<uint32_t>& lhs,const db::predicate::clause& cl);
  //IF THIS IS SUCCESSFUL, ADD RHS OF CLAUSE ON THE TOP OF THE STACK WITH IDs INCREASED BY ENV.SIZE BEFORE UNIFICATION
  uint32_t new_element_pure();
  uint32_t new_element_function(uint32_t fid,std::vector<uint32_t>&& args);
};

}

#endif //BPL_LIBS_DS_INCLUDE_DS_ENVIRONMENT_H_
