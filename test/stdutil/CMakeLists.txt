option(ENABLE_LOG "Enable logging" ON)

add_executable(test_triekv triekv.c)
target_link_libraries(test_triekv stdutil)

add_executable(test_block blocks.c)
target_link_libraries(test_block stdutil)

add_executable(test_box box.c)
target_link_libraries(test_box stdutil)

if(ENABLE_LOG)
    target_compile_definitions(test_block PRIVATE ENABLE_LOG)
    target_compile_definitions(test_triekv PRIVATE ENABLE_LOG)
    target_compile_definitions(test_box PRIVATE ENABLE_LOG)
endif()