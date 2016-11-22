#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

add_definitions(${COMPILE_WARNING_FLAGS} ${COMPILE_PLATFORM_FLAGS})

if (ENABLE_VALGRIND AND VALGRIND_EXE)
  set(memcheck-cmd ${VALGRIND_EXE} --error-exitcode=1 --quiet
                   --leak-check=full --trace-children=yes)
endif ()

macro (pn_add_c_test test file)
  add_executable (${test} ${file})
  target_link_libraries (${test} qpid-proton)
  if (BUILD_WITH_CXX)
    set_source_files_properties (${file} PROPERTIES LANGUAGE CXX)
  endif (BUILD_WITH_CXX)
  if (CMAKE_SYSTEM_NAME STREQUAL Windows)
    add_test (NAME ${test}
              COMMAND ${env_py}
                "PATH=$<TARGET_FILE_DIR:qpid-proton>"
                $<TARGET_FILE:${test}>)
  else ()
    add_test (${test} ${memcheck-cmd} ${CMAKE_CURRENT_BINARY_DIR}/${test})
  endif ()
endmacro(pn_add_c_test)

link_directories(${LINK_DIRECTORIES} /home/jdanek/Work)

add_executable (fuzz fuzz_detect.c ../core/autodetect.c)
target_link_libraries (fuzz qpid-proton Fuzzer m stdc++)

find_package(Libuv)

add_executable (fuzz_proactor_recv fuzz_proactor_recv.c ../../../examples/c/proactor/libuv_proactor.c)
target_link_libraries (fuzz_proactor_recv qpid-proton ${Libuv_LIBRARIES} Fuzzer m stdc++)
set_target_properties(fuzz_proactor_recv PROPERTIES
      COMPILE_DEFINITIONS  "PN_PROACTOR_INCLUDE=\"libuv_proactor.h\"")

# for alf-fuzz, when compilling with normal clang
add_executable (afl_fuzz_proactor_recv fuzz_proactor_recv.c ../../../examples/c/proactor/libuv_proactor.c afl_driver.cpp ./afl-llvm-rt.o.o)

# again afl-fuzz, when compiling with their clang wrapper
# add_executable (afl_fuzz_proactor_recv fuzz_proactor_recv.c ../../../examples/c/proactor/libuv_proactor.c afl_driver.cpp)

target_link_libraries (afl_fuzz_proactor_recv qpid-proton ${Libuv_LIBRARIES} m stdc++)
set_target_properties(afl_fuzz_proactor_recv PROPERTIES
      COMPILE_DEFINITIONS  "PN_PROACTOR_INCLUDE=\"libuv_proactor.h\"")

# add_executable (fuzz_engine fuzz_engine.c)
# target_link_libraries (fuzz_engine qpid-proton Fuzzer m stdc++)

add_executable (fuzz_url fuzz_url.c)
target_link_libraries (fuzz_url qpid-proton Fuzzer m stdc++)

add_executable (fuzz_message_decode fuzz_message_decode.c)
target_link_libraries (fuzz_message_decode qpid-proton Fuzzer m stdc++)

add_executable (fuzz_connection_driver fuzz_connection_driver.c)
target_link_libraries (fuzz_connection_driver qpid-proton Fuzzer m stdc++)

pn_add_c_test (c-object-tests object.c)
pn_add_c_test (c-message-tests message.c)
pn_add_c_test (c-engine-tests engine.c)
pn_add_c_test (c-parse-url-tests parse-url.c)
pn_add_c_test (c-refcount-tests refcount.c)
pn_add_c_test (c-reactor-tests reactor.c)
pn_add_c_test (c-event-tests event.c)
pn_add_c_test (c-data-tests data.c)
pn_add_c_test (c-condition-tests condition.c)
