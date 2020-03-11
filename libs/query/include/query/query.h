#ifndef BPL_LIBS_QUERY_INCLUDE_QUERY_QUERY_H_
#define BPL_LIBS_QUERY_INCLUDE_QUERY_QUERY_H_
#include <db/db.h>


namespace bpl::query{

void execute_query(db& storage,std::vector<parser::term> query_goal);

}

#endif //BPL_LIBS_QUERY_INCLUDE_QUERY_QUERY_H_
