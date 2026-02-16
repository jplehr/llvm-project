# Create the shared profiler unittest target
add_executable(SharedProfilerUnitTests
    ProfilerTest.cpp)

# Include directories
target_include_directories(SharedProfilerUnitTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${LLVM_INCLUDE_DIRS})

# Link required libraries
target_link_libraries(SharedProfilerUnitTests PRIVATE
    LLVMSupport
    llvm_gtest
    llvm_gtest_main)

# Set target properties
set_target_properties(SharedProfilerUnitTests PROPERTIES
    FOLDER "Tests/UnitTests/Offload")

# Add the test to the check-offload target if it exists
if(TARGET check-offload)
    add_dependencies(check-offload SharedProfilerUnitTests)
endif()

# Add it to OffloadUnitTests target as well
if(TARGET OffloadUnitTests)
    add_dependencies(OffloadUnitTests SharedProfilerUnitTests)
endif()