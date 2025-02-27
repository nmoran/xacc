set(LIBRARY_NAME xacc-rigetti-qcs)

file (GLOB_RECURSE HEADERS *.hpp)
file (GLOB SRC *.cpp)

usFunctionGetResourceSource(TARGET ${LIBRARY_NAME} OUT SRC)
usFunctionGenerateBundleInit(TARGET ${LIBRARY_NAME} OUT SRC)

include_directories(${XACC_INCLUDE_ROOT}/pybind11/include)
include_directories(${PYTHON_INCLUDE_DIR})

add_library(${LIBRARY_NAME} SHARED ${SRC})

target_include_directories(${LIBRARY_NAME}
                           PUBLIC . ../accelerator
                                  ${CMAKE_SOURCE_DIR}/tpls/pybind11/include
                                  ${PYTHON_INCLUDE_DIR})
target_link_libraries(${LIBRARY_NAME}
                      PUBLIC xacc xacc-quantum-gate xacc-quantum-aqc
                             CppMicroServices ${PYTHON_LIBRARY})

set(_bundle_name xacc_rigetti_qcs)

set_target_properties(${LIBRARY_NAME} PROPERTIES
  # This is required for every bundle
  COMPILE_DEFINITIONS US_BUNDLE_NAME=${_bundle_name}
  # This is for convenience, used by other CMake functions
  US_BUNDLE_NAME ${_bundle_name}
  )

# Embed meta-data from a manifest.json file
usFunctionEmbedResources(TARGET ${LIBRARY_NAME}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  FILES
    manifest.json
  )

if(APPLE)
   set_target_properties(${LIBRARY_NAME} PROPERTIES INSTALL_RPATH "@loader_path/../lib")
   set_target_properties(${LIBRARY_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
else()
   set_target_properties(${LIBRARY_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
   set_target_properties(${LIBRARY_NAME} PROPERTIES LINK_FLAGS "-shared")
endif()


install(TARGETS ${LIBRARY_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins)

if(XACC_BUILD_TESTS)
  add_subdirectory(tests)
endif()