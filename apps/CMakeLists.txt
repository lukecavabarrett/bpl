add_executable(bpl-console bpl-console.cc)
target_link_libraries(bpl-console parser db query)

add_executable(sample sample.cc)
target_link_libraries(sample parser db query)