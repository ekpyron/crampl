add_executable (crampl_test main.cpp MemberComparator.cpp MultiKeyMap.cpp ContainerContainer.cpp ClosureContainer.cpp)
target_link_libraries(crampl_test crampl Catch2::Catch2)
catch_discover_tests(crampl_test)
