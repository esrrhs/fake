#include <gtest/gtest.h>

#include "fakelua.h"
#include "test_jit.h"

using namespace fakelua;

TEST(regression_23, precise_mixed_numeric_comparison) {
    FakeluaStateGuard sg;
    State *s = sg.GetState();
    ASSERT_NE(s, nullptr);
    CompileFile(s, "./jit/test_regression_numeric_eval_once.lua", {});

    for (const auto jit_type: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(jit_type));
        int64_t result = -1;
        Call(s, jit_type, "test_precise_mixed_compare", result);
        EXPECT_EQ(result, 0);
    }
}

TEST(regression_23, jit_expressions_evaluate_operands_once) {
    FakeluaStateGuard sg;
    State *s = sg.GetState();
    ASSERT_NE(s, nullptr);
    CompileFile(s, "./jit/test_regression_numeric_eval_once.lua", {});

    for (const auto jit_type: AllJitTypes()) {
        SCOPED_TRACE(JitTypeName(jit_type));
        int64_t result = -1;
        Call(s, jit_type, "test_and_or_eval_once", result);
        EXPECT_EQ(result, 2);

        result = -1;
        Call(s, jit_type, "test_math_inline_eval_once", result);
        EXPECT_EQ(result, 5);

        result = -1;
        Call(s, jit_type, "test_numeric_ternary_once", result, 1);
        EXPECT_EQ(result, 1);
        result = -1;
        Call(s, jit_type, "test_numeric_ternary_once", result, -1);
        EXPECT_EQ(result, 1);
    }
}
