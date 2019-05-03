#pragma once

enum Test_result
{
    PASSED = 0,
    FAILED = 1,
};

typedef Test_result (*test_fn)(void);



Test_result test_types();

