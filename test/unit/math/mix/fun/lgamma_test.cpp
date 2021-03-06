#include <test/unit/math/test_ad.hpp>

TEST(mathMixMatFun, lgamma) {
  auto f = [](const auto& x1) { return stan::math::lgamma(x1); };
  stan::test::expect_common_unary_vectorized(f);
  stan::test::expect_unary_vectorized(f, -2.6, -2, -0.2, 1, 1.3, 3, 3.7, 10,
                                      22);
}
