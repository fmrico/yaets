cmake_minimum_required(VERSION 3.5)
project(yaets)
set(version 0.1)

set(CMAKE_CXX_STANDARD 17)
find_package(ament_cmake REQUIRED)

include_directories(include)
add_library(${PROJECT_NAME} SHARED
  src/yaets/tracing.cpp)


install(DIRECTORY include/
  DESTINATION include/
)

install(PROGRAMS
  scripts/gantt.py
  scripts/histogram.py
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  ${PROJECT_NAME}
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

ament_package()
