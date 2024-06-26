file(GLOB HEADERS "*.hpp"
                  "*.h")

add_library(eosio
            eosiolib.cpp
            crypto.cpp
            base64.cpp
            ${HEADERS})

add_library(eosio_malloc
            malloc.cpp
            ${HEADERS})

add_library(eosio_dsm
            simple_malloc.cpp
            ${HEADERS})

add_library(eosio_cmem
            memory.cpp
            ${HEADERS})


set_target_properties(eosio_malloc PROPERTIES LINKER_LANGUAGE C)

target_include_directories(eosio PUBLIC
                                 ${CMAKE_SOURCE_DIR}/libc/cdt-musl/include
                                 ${CMAKE_SOURCE_DIR}/libc/cdt-musl/src/internal
                                 ${CMAKE_SOURCE_DIR}/libc/cdt-musl/src/crypt
                                 ${CMAKE_SOURCE_DIR}/libc/cdt-musl/arch/eos
                                 ${CMAKE_SOURCE_DIR}/libc++/cdt-libcxx/include
                                 ${CMAKE_SOURCE_DIR})

target_link_libraries( eosio c c++ )
add_custom_command( TARGET eosio POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:eosio> ${BASE_BINARY_DIR}/lib )
add_custom_command( TARGET eosio_malloc POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:eosio_malloc> ${BASE_BINARY_DIR}/lib )
add_custom_command( TARGET eosio_dsm POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:eosio_dsm> ${BASE_BINARY_DIR}/lib )
add_custom_command( TARGET eosio_cmem POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:eosio_cmem> ${BASE_BINARY_DIR}/lib )

if (ENABLE_NATIVE_COMPILER)
   add_native_library(native_eosio
                      eosiolib.cpp
                      crypto.cpp
                      malloc.cpp
                      base64.cpp
                      ${HEADERS})

   add_dependencies( native_eosio eosio )
   add_custom_command( TARGET native_eosio POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:native_eosio> ${BASE_BINARY_DIR}/lib )
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../eosiolib DESTINATION ${BASE_BINARY_DIR}/include FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
