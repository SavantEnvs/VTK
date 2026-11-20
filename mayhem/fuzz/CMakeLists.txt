# Fuzz harnesses + known-answer oracle, built against the VTK build tree (mayhem/build.sh).
cmake_minimum_required(VERSION 3.13)
project(vtk_mayhem_fuzz C CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(VTK REQUIRED COMPONENTS
  CommonCore CommonDataModel CommonExecutionModel IOCore IOLegacy IOXML IOPLY IOGeometry IOImport)

set(FUZZERS legacy_fuzzer xml_fuzzer ply_fuzzer stl_fuzzer obj_fuzzer gltf_reader_fuzzer)
set(LIB_FUZZING_ENGINE "-fsanitize=fuzzer" CACHE STRING "libFuzzer link flag")
set(STANDALONE_FUZZ_MAIN "" CACHE FILEPATH "LLVM run-once driver")
set(LSAN_OFF "${CMAKE_SOURCE_DIR}/../lsan_off.cc" CACHE FILEPATH "LSan-off hook (mayhem/lsan_off.cc)")
set(OUT_DIR "${CMAKE_BINARY_DIR}/bin" CACHE PATH "output directory")

add_library(standalone_main OBJECT ${STANDALONE_FUZZ_MAIN})
set_source_files_properties(${STANDALONE_FUZZ_MAIN} PROPERTIES LANGUAGE C)

foreach(f ${FUZZERS})
  add_executable(${f} ${f}.cxx ${LSAN_OFF})
  target_link_libraries(${f} PRIVATE ${VTK_LIBRARIES})
  target_link_options(${f} PRIVATE ${LIB_FUZZING_ENGINE})
  set_target_properties(${f} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
  vtk_module_autoinit(TARGETS ${f} MODULES ${VTK_LIBRARIES})

  add_executable(${f}-standalone ${f}.cxx ${LSAN_OFF} $<TARGET_OBJECTS:standalone_main>)
  target_link_libraries(${f}-standalone PRIVATE ${VTK_LIBRARIES})
  set_target_properties(${f}-standalone PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
  vtk_module_autoinit(TARGETS ${f}-standalone MODULES ${VTK_LIBRARIES})
endforeach()

add_executable(vtk_oracle vtk_oracle.cxx ${LSAN_OFF})
target_link_libraries(vtk_oracle PRIVATE ${VTK_LIBRARIES})
set_target_properties(vtk_oracle PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
vtk_module_autoinit(TARGETS vtk_oracle MODULES ${VTK_LIBRARIES})
