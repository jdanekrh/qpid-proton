SET(CMAKE_C_OUTPUT_EXTENSION .bc)
SET(CMAKE_CXX_OUTPUT_EXTENSION .bc)

find_program(compiler "compiler.py")
set(CMAKE_C_COMPILER ${compiler})
set(CMAKE_CXX_COMPILER ${compiler})