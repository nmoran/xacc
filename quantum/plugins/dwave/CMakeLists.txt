set(LIBRARY_NAME xacc-dwave)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-security")

file(GLOB SRC
          accelerator/DWAccelerator.cpp
          DWaveActivator.cpp
          compiler/DWQMICompiler.cpp
          compiler/DWQMIListener.cpp
          compiler/generated/*.cpp)

usfunctiongetresourcesource(TARGET ${LIBRARY_NAME} OUT SRC)
usfunctiongeneratebundleinit(TARGET ${LIBRARY_NAME} OUT SRC)

add_library(${LIBRARY_NAME} SHARED ${SRC})

target_include_directories(${LIBRARY_NAME}
                           PUBLIC accelerator
                                  compiler
                                  compiler/generated
                                  embedding
                                  ${CMAKE_SOURCE_DIR}/tpls/rapidjson/include
                                  ${CMAKE_SOURCE_DIR}/tpls/antlr/runtime/src)

if(APPLE)
target_link_libraries(${LIBRARY_NAME}
                      PUBLIC xacc
                             xacc-quantum-aqc
                             ${CMAKE_SOURCE_DIR}/dist/libantlr4-runtime.dylib
                             CppMicroServices)
else()
target_link_libraries(${LIBRARY_NAME}
                      PUBLIC xacc
                             xacc-quantum-aqc
                             ${CMAKE_SOURCE_DIR}/dist/libantlr4-runtime.so
                             CppMicroServices)
endif()

set(_bundle_name xacc_dwave)
set_target_properties(${LIBRARY_NAME}
                      PROPERTIES COMPILE_DEFINITIONS
                                 US_BUNDLE_NAME=${_bundle_name}
                                 US_BUNDLE_NAME
                                 ${_bundle_name})

usfunctionembedresources(TARGET
                         ${LIBRARY_NAME}
                         WORKING_DIRECTORY
                         ${CMAKE_CURRENT_SOURCE_DIR}
                         FILES
                         manifest.json)

if(APPLE)
  set_target_properties(${LIBRARY_NAME}
                        PROPERTIES INSTALL_RPATH "@loader_path/../lib")
  set_target_properties(${LIBRARY_NAME}
                        PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
else()
  set_target_properties(${LIBRARY_NAME}
                        PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
  set_target_properties(${LIBRARY_NAME} PROPERTIES LINK_FLAGS "-shared")
endif()

if(XACC_BUILD_TESTS)
  add_subdirectory(tests)
endif()

install(TARGETS ${LIBRARY_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins)

file(GLOB PYDECORATORS ${CMAKE_CURRENT_SOURCE_DIR}/decorators/*.py)
install(FILES ${PYDECORATORS} DESTINATION ${CMAKE_INSTALL_PREFIX}/py-plugins)

add_subdirectory(ir)