#include <test/unit/math/test_ad.hpp>
#include <vector>

TEST(mathMixMatFun, atanh) {
  auto f = [](const auto& x1) {
    using stan::math::atanh;
    return atanh(x1);
  };
  stan::test::expect_common_unary_vectorized(f);
  stan::test::expect_unary_vectorized(f, -0.9, 0.5);
  for (double re : std::vector<double>{-0.2, 0, 0.3}) {
    for (double im : std::vector<double>{-0.3, 0, 0.2}) {
      stan::test::expect_ad(f, std::complex<double>{re, im});
    }
  }
}

TEST(mathMixMatFun, atanh_varmat) {
  auto f = [](const auto& x1) {
    using stan::math::atanh;
    return atanh(x1);
  };
  auto com_args = stan::test::internal::common_args();
  std::vector<double> extra_args{-0.9, 0.5};
  Eigen::VectorXd A(com_args.size() + extra_args.size());
  int i = 0;
  for (double x : com_args) {
    A(i) = x;
    ++i;
  }
  for (double x : extra_args) {
    A(i) = x;
    ++i;
  }
  stan::test::expect_ad_matvar(f, A);
  std::vector<Eigen::VectorXd> A_vec;
  A_vec.push_back(A);
  A_vec.push_back(A);
  A_vec.push_back(A);
  stan::test::expect_ad_matvar(f, A_vec);
}
