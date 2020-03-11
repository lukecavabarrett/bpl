add_executable(bpl bpl-console.cc)
target_link_libraries(bpl parser db query)

add_executable(sample sample.cc)
target_link_libraries(sample parser db query)