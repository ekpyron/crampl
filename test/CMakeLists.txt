add_executable (crampl_test MemberComparator.cpp MultiKeyMap.cpp ContainerContainer.cpp Function.cpp)
target_link_libraries(crampl_test crampl Catch2::Catch2WithMain)
catch_discover_tests(crampl_test)
