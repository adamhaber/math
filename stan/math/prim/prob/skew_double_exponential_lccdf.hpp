#ifndef STAN_MATH_PRIM_PROB_SKEW_DOUBLE_EXPONENTIAL_LCCDF_HPP
#define STAN_MATH_PRIM_PROB_SKEW_DOUBLE_EXPONENTIAL_LCCDF_HPP

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/err.hpp>
#include <stan/math/prim/fun/constants.hpp>
#include <stan/math/prim/fun/exp.hpp>
#include <stan/math/prim/fun/inv.hpp>
#include <stan/math/prim/fun/log1m.hpp>
#include <stan/math/prim/fun/max_size.hpp>
#include <stan/math/prim/fun/size_zero.hpp>
#include <stan/math/prim/fun/value_of.hpp>
#include <stan/math/prim/functor/operands_and_partials.hpp>
#include <cmath>

namespace stan {
namespace math {

/** \ingroup prob_dists
 * Returns the skew double exponential log complementary cumulative density
 * function. Given containers of matching sizes, returns the log sum of
 * probabilities.
 *
 * @tparam T_y type of real parameter.
 * @tparam T_loc type of location parameter.
 * @tparam T_scale type of scale parameter.
 * @tparam T_skewness type of skewness parameter.
 * @param y real parameter
 * @param mu location parameter
 * @param sigma scale parameter
 * @param tau skewness parameter
 * @return log probability or log sum of probabilities
 * @throw std::domain_error if mu is infinite or sigma is nonpositive or tau is
 *  not bound between 0.0 and 1.0
 * @throw std::invalid_argument if container sizes mismatch
 */
template <typename T_y, typename T_loc, typename T_scale, typename T_skewness>
return_type_t<T_y, T_loc, T_scale, T_skewness> skew_double_exponential_lccdf(
    const T_y& y, const T_loc& mu, const T_scale& sigma,
    const T_skewness& tau) {
  using std::exp;
  using std::log;
  using T_partials_return = partials_return_t<T_y, T_loc, T_scale, T_skewness>;
  using T_y_ref = ref_type_if_t<!is_constant<T_y>::value, T_y>;
  using T_mu_ref = ref_type_if_t<!is_constant<T_loc>::value, T_loc>;
  using T_sigma_ref = ref_type_if_t<!is_constant<T_scale>::value, T_scale>;
  using T_tau_ref = ref_type_if_t<!is_constant<T_skewness>::value, T_skewness>;
  static const char* function = "skew_double_exponential_lccdf";
  check_consistent_sizes(function, "Random variable", y, "Location parameter",
                         mu, "Shape parameter", sigma, "Skewness parameter",
                         tau);
  T_y_ref y_ref = y;
  T_mu_ref mu_ref = mu;
  T_sigma_ref sigma_ref = sigma;
  T_tau_ref tau_ref = tau;

  check_not_nan(function, "Random variable", y_ref);
  check_finite(function, "Location parameter", mu_ref);
  check_positive_finite(function, "Scale parameter", sigma_ref);
  check_bounded(function, "Skewness parameter", tau_ref, 0.0, 1.0);

  if (size_zero(y, mu, sigma, tau)) {
    return 0.0;
  }

  T_partials_return cdf_log(0.0);
  operands_and_partials<T_y_ref, T_mu_ref, T_sigma_ref, T_tau_ref> ops_partials(
      y_ref, mu_ref, sigma_ref, tau_ref);

  scalar_seq_view<T_y_ref> y_vec(y_ref);
  scalar_seq_view<T_mu_ref> mu_vec(mu_ref);
  scalar_seq_view<T_sigma_ref> sigma_vec(sigma_ref);
  scalar_seq_view<T_tau_ref> tau_vec(tau_ref);

  int size_sigma = stan::math::size(sigma);
  int N = max_size(y, mu, sigma, tau);

  VectorBuilder<true, T_partials_return, T_scale> inv_sigma(size_sigma);
  for (int i = 0; i < size_sigma; ++i) {
    inv_sigma[i] = inv(value_of(sigma_vec[i]));
  }

  for (int i = 0; i < N; ++i) {
    const T_partials_return y_dbl = value_of(y_vec[i]);
    const T_partials_return mu_dbl = value_of(mu_vec[i]);
    const T_partials_return sigma_dbl = value_of(sigma_vec[i]);
    const T_partials_return tau_dbl = value_of(tau_vec[i]);

    const T_partials_return y_m_mu = y_dbl - mu_dbl;
    const T_partials_return diff_sign = sign(y_m_mu);
    const T_partials_return diff_sign_smaller_0 = step(-diff_sign);
    const T_partials_return abs_diff_y_mu = fabs(y_m_mu);
    const T_partials_return abs_diff_y_mu_over_sigma
        = abs_diff_y_mu * inv_sigma[i];
    const T_partials_return expo = (diff_sign_smaller_0 + diff_sign * tau_dbl)
                                   * abs_diff_y_mu_over_sigma;
    const T_partials_return inv_exp_2_expo_tau
        = inv(exp(2.0 * (tau_dbl - 1.0) * y_m_mu * inv_sigma[i]) - tau_dbl);
    const T_partials_return tau_m1_tau = (tau_dbl - 1.0) * tau_dbl;

    const T_partials_return rep_deriv
        = y_dbl < mu_dbl ? 2.0 * tau_m1_tau * inv_sigma[i] * inv_exp_2_expo_tau
                         : -2.0 * inv_sigma[i] * tau_dbl;
    const T_partials_return sig_deriv
        = y_dbl < mu_dbl ? -2.0 * tau_m1_tau * y_m_mu * inv_sigma[i]
                               * inv_sigma[i] * inv_exp_2_expo_tau
                         : 2.0 * inv_sigma[i] * expo;
    const T_partials_return skew_deriv
        = y_dbl < mu_dbl ? -(sigma_dbl + 2.0 * tau_dbl * y_m_mu * diff_sign)
                               * inv_sigma[i] * inv_exp_2_expo_tau
                         : 1.0 / (tau_dbl - 1.0) - 2.0 * inv_sigma[i] * y_m_mu;

    if (y_dbl <= mu_dbl) {
      cdf_log += log1m(tau_dbl * exp(-2.0 * expo));
    } else {
      cdf_log += log(1 - tau_dbl) - 2.0 * expo;
    }

    if (!is_constant_all<T_y>::value) {
      ops_partials.edge1_.partials_[i] += rep_deriv;
    }
    if (!is_constant_all<T_loc>::value) {
      ops_partials.edge2_.partials_[i] -= rep_deriv;
    }
    if (!is_constant_all<T_scale>::value) {
      ops_partials.edge3_.partials_[i] += sig_deriv;
    }
    if (!is_constant_all<T_skewness>::value) {
      ops_partials.edge4_.partials_[i] += skew_deriv;
    }
  }
  return ops_partials.build(cdf_log);
}
}  // namespace math
}  // namespace stan
#endif
