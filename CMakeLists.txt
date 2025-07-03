cmake_minimum_required(VERSION 3.5)
project(yaets)
set(version 0.1)

set(CMAKE_CXX_STANDARD 17)
find_package(ament_cmake REQUIRED)

add_library(${PROJECT_NAME} SHARED
  src/yaets/tracing.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)


install(DIRECTORY include/
  DESTINATION include/
)

install(PROGRAMS
  scripts/gantt.py
  scripts/elapsed_time_histogram.py
  scripts/execution_time_histogram.py
  scripts/stats.py
  scripts/diagrams.py
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_cpplint_FOUND TRUE)
  set(ament_cmake_copyright_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)

  add_subdirectory(tests)
endif()


ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_targets(export_${PROJECT_NAME})

ament_package()
