add_executable(test_triekv triekv.c)
target_link_libraries(test_triekv stdutil)

add_executable(test_block blocks.c)
target_link_libraries(test_block stdutil)