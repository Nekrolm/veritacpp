add_executable(difs_test diffs.cpp)

add_test(NAME difs_test COMMAND difs_test)

add_executable(reference_test references.cpp)

add_test(NAME reference_test COMMAND reference_test)