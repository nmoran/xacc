include_directories(${CMAKE_SOURCE_DIR}/quantum/gate/utils)
include_directories(${CMAKE_SOURCE_DIR}/quantum/gate/ir)
include_directories(${CMAKE_SOURCE_DIR}/quantum/gate/ir/instructions)
include_directories(${CMAKE_SOURCE_DIR}/quantum/utils)
include_directories(${CMAKE_SOURCE_DIR}/quantum/gate/accelerator)
include_directories(${CMAKE_SOURCE_DIR}/quantum/gate/observable/pauli)

include_directories(${CMAKE_BINARY_DIR})

#add_xacc_test(ReadoutErrorAcceleratorBufferPostprocessor)
add_xacc_test(ImprovedSamplingDecorator)
add_xacc_test(RichExtrapDecorator)
add_xacc_test(ROErrorDecorator)

target_link_libraries(RichExtrapDecoratorTester CppMicroServices xacc-quantum-gate)
target_link_libraries(ImprovedSamplingDecoratorTester CppMicroServices xacc-quantum-gate)
target_link_libraries(ROErrorDecoratorTester CppMicroServices xacc-quantum-gate xacc-pauli)