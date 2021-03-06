#ifndef STAN_MATH_PRIM_PROB_UNIFORM_CDF_HPP
#define STAN_MATH_PRIM_PROB_UNIFORM_CDF_HPP

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/err.hpp>
#include <stan/math/prim/fun/as_column_vector_or_scalar.hpp>
#include <stan/math/prim/fun/as_array_or_scalar.hpp>
#include <stan/math/prim/fun/max_size.hpp>
#include <stan/math/prim/fun/promote_scalar.hpp>
#include <stan/math/prim/fun/size.hpp>
#include <stan/math/prim/fun/size_zero.hpp>
#include <stan/math/prim/fun/to_ref.hpp>
#include <stan/math/prim/fun/value_of.hpp>
#include <stan/math/prim/functor/operands_and_partials.hpp>

namespace stan {
namespace math {

template <typename T_y, typename T_low, typename T_high>
return_type_t<T_y, T_low, T_high> uniform_cdf(const T_y& y, const T_low& alpha,
                                              const T_high& beta) {
  using T_partials_return = partials_return_t<T_y, T_low, T_high>;
  using T_y_ref = ref_type_if_t<!is_constant<T_y>::value, T_y>;
  using T_alpha_ref = ref_type_if_t<!is_constant<T_low>::value, T_low>;
  using T_beta_ref = ref_type_if_t<!is_constant<T_high>::value, T_high>;
  static const char* function = "uniform_cdf";
  check_consistent_sizes(function, "Random variable", y,
                         "Lower bound parameter", alpha,
                         "Upper bound parameter", beta);

  T_y_ref y_ref = y;
  T_alpha_ref alpha_ref = alpha;
  T_beta_ref beta_ref = beta;

  const auto& y_col = as_column_vector_or_scalar(y_ref);
  const auto& alpha_col = as_column_vector_or_scalar(alpha_ref);
  const auto& beta_col = as_column_vector_or_scalar(beta_ref);

  const auto& y_arr = as_array_or_scalar(y_col);
  const auto& alpha_arr = as_array_or_scalar(alpha_col);
  const auto& beta_arr = as_array_or_scalar(beta_col);

  ref_type_t<decltype(value_of(y_arr))> y_val = value_of(y_arr);
  ref_type_t<decltype(value_of(alpha_arr))> alpha_val = value_of(alpha_arr);
  ref_type_t<decltype(value_of(beta_arr))> beta_val = value_of(beta_arr);

  check_not_nan(function, "Random variable", y_val);
  check_finite(function, "Lower bound parameter", alpha_val);
  check_finite(function, "Upper bound parameter", beta_val);
  check_greater(function, "Upper bound parameter", beta_val, alpha_val);

  if (size_zero(y, alpha, beta)) {
    return 1.0;
  }

  if (sum(promote_scalar<int>(y_val < alpha_val))
      || sum(promote_scalar<int>(beta_val < y_val))) {
    return 0;
  }

  operands_and_partials<T_y_ref, T_alpha_ref, T_beta_ref> ops_partials(
      y_ref, alpha_ref, beta_ref);

  const auto& b_minus_a
      = to_ref_if<!is_constant_all<T_y, T_low, T_high>::value>(beta_val
                                                               - alpha_val);
  const auto& cdf_n = to_ref_if<!is_constant_all<T_y, T_low>::value>(
      (y_val - alpha_val) / b_minus_a);

  T_partials_return cdf = prod(cdf_n);

  if (!is_constant_all<T_y, T_low, T_high>::value) {
    const auto& rep_deriv
        = to_ref_if<(!is_constant_all<T_y, T_low>::value
                     && !is_constant_all<T_high>::value)>(cdf / b_minus_a);
    if (!is_constant_all<T_y, T_low>::value) {
      const auto& deriv_y
          = to_ref_if<(!is_constant_all<T_low>::value
                       && !is_constant_all<T_y>::value)>(rep_deriv / cdf_n);
      if (!is_constant_all<T_low>::value) {
        ops_partials.edge2_.partials_
            = (y_val - beta_val) * deriv_y / b_minus_a;
      }
      if (!is_constant_all<T_y>::value) {
        ops_partials.edge1_.partials_ = std::move(deriv_y);
      }
    }
    if (!is_constant_all<T_high>::value) {
      if (is_vector<T_y>::value && !is_vector<T_low>::value
          && !is_vector<T_high>::value) {
        ops_partials.edge3_.partials_
            = -rep_deriv * max_size(y, alpha, beta) / max_size(alpha, beta);
      } else {
        ops_partials.edge3_.partials_ = -rep_deriv;
      }
    }
  }

  return ops_partials.build(cdf);
}

}  // namespace math
}  // namespace stan
#endif
