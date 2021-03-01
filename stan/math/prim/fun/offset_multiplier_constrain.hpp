#ifndef STAN_MATH_PRIM_FUN_OFFSET_MULTIPLIER_CONSTRAIN_HPP
#define STAN_MATH_PRIM_FUN_OFFSET_MULTIPLIER_CONSTRAIN_HPP

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/err.hpp>
#include <stan/math/prim/fun/fma.hpp>
#include <stan/math/prim/fun/identity_constrain.hpp>
#include <stan/math/prim/fun/multiply_log.hpp>
#include <stan/math/prim/fun/size.hpp>
#include <stan/math/prim/fun/sum.hpp>
#include <stan/math/prim/fun/to_ref.hpp>
#include <cmath>

namespace stan {
namespace math {

/**
 * Return the linearly transformed value for the specified unconstrained input
 * and specified offset and multiplier.
 *
 * <p>The transform applied is
 *
 * <p>\f$f(x) = mu + sigma * x\f$
 *
 * <p>where mu is the offset and sigma is the multiplier.
 *
 * <p>If the offset is zero and the multiplier is one this
 * reduces to <code>identity_constrain(x)</code>.
 *
 * @tparam T type of scalar
 * @tparam M type of offset
 * @tparam S type of multiplier
 * @param[in] x Unconstrained scalar input
 * @param[in] mu offset of constrained output
 * @param[in] sigma multiplier of constrained output
 * @return linear transformed value corresponding to inputs
 * @throw std::domain_error if sigma <= 0
 * @throw std::domain_error if mu is not finite
 */
template <typename T, typename M, typename S>
inline auto offset_multiplier_constrain(const T& x, const M& mu,
                                        const S& sigma) {
  const char* function = "offset_multiplier_constrain";
  const auto& mu_ref = to_ref(mu);
  const auto& sigma_ref = to_ref(sigma);
  check_finite(function, "offset", value_of_rec(mu_ref));
  check_positive_finite(function, "multiplier", value_of_rec(sigma_ref));
  check_consistent_sizes(function, "offset", mu, "multiplier", sigma,
                         "parameter", x);
  return fma(sigma_ref, x, mu_ref);
}

/**
 * Return the linearly transformed value for the specified unconstrained input
 * and specified offset and multiplier, incrementing the specified
 * reference with the log absolute Jacobian determinant of the
 * transform.
 *
 * <p>The transform applied is
 *
 * <p>\f$f(x) = mu + sigma * x\f$
 *
 * <p>where mu is the offset and sigma is the multiplier.
 *
 * If the offset is zero and multiplier is one, this function
 * reduces to <code>identity_constraint(x, lp)</code>.
 *
 * @tparam T type of scalar
 * @tparam M type of offset
 * @tparam S type of multiplier
 * @param[in] x Unconstrained scalar input
 * @param[in] mu offset of constrained output
 * @param[in] sigma multiplier of constrained output
 * @param[in,out] lp Reference to log probability to increment.
 * @return linear transformed value corresponding to inputs
 * @throw std::domain_error if sigma <= 0
 * @throw std::domain_error if mu is not finite
 */
template <typename T, typename M, typename S>
inline auto offset_multiplier_constrain(const T& x, const M& mu, const S& sigma,
                                        return_type_t<T, M, S>& lp) {
  const char* function = "offset_multiplier_constrain";
  const auto& mu_ref = to_ref(mu);
  const auto& sigma_ref = to_ref(sigma);
  check_finite(function, "offset", value_of_rec(mu_ref));
  check_positive_finite(function, "multiplier", value_of_rec(sigma_ref));
  check_consistent_sizes(function, "offset", mu, "multiplier", sigma,
                         "parameter", x);
  if (size(sigma_ref) == 1 && size(x) > 1) {
    lp += sum(multiply_log(size(x), sigma_ref));
  } else {
    lp += sum(log(sigma_ref));
  }
  return fma(sigma_ref, x, mu_ref);
}

/**
 * Overload for array of x and non-array mu and sigma
 */
template <typename T, typename M, typename S,
          require_all_not_std_vector_t<M, S>* = nullptr>
inline auto offset_multiplier_constrain(const std::vector<T>& x, const M& mu,
                                        const S& sigma) {
  std::vector<
      plain_type_t<decltype(offset_multiplier_constrain(x[0], mu, sigma))>>
      ret;
  ret.reserve(x.size());
  const auto& mu_ref = to_ref(mu);
  const auto& sigma_ref = to_ref(sigma);
  for (size_t i = 0; i < x.size(); ++i) {
    ret.emplace_back(offset_multiplier_constrain(x[i], mu_ref, sigma_ref));
  }
  return ret;
}

/**
 * Overload for array of x and non-array mu and sigma with lp
 */
template <typename T, typename M, typename S,
          require_all_not_std_vector_t<M, S>* = nullptr>
inline auto offset_multiplier_constrain(const std::vector<T>& x, const M& mu,
                                        const S& sigma,
                                        return_type_t<T, M, S>& lp) {
  std::vector<
      plain_type_t<decltype(offset_multiplier_constrain(x[0], mu, sigma, lp))>>
      ret;
  ret.reserve(x.size());
  const auto& mu_ref = to_ref(mu);
  const auto& sigma_ref = to_ref(sigma);
  for (size_t i = 0; i < x.size(); ++i) {
    ret.emplace_back(offset_multiplier_constrain(x[i], mu_ref, sigma_ref, lp));
  }
  return ret;
}

/**
 * Overload for array of x and sigma and non-array mu
 */
template <typename T, typename M, typename S,
          require_not_std_vector_t<M>* = nullptr>
inline auto offset_multiplier_constrain(const std::vector<T>& x, const M& mu,
                                        const std::vector<S>& sigma) {
  check_matching_dims("offset_multiplier_constrain", "x", x, "sigma", sigma);
  std::vector<
      plain_type_t<decltype(offset_multiplier_constrain(x[0], mu, sigma[0]))>>
      ret;
  ret.reserve(x.size());
  const auto& mu_ref = to_ref(mu);
  for (size_t i = 0; i < x.size(); ++i) {
    ret.emplace_back(offset_multiplier_constrain(x[i], mu_ref, sigma[i]));
  }
  return ret;
}

/**
 * Overload for array of x and sigma and non-array mu with lp
 */
template <typename T, typename M, typename S,
          require_not_std_vector_t<M>* = nullptr>
inline auto offset_multiplier_constrain(const std::vector<T>& x, const M& mu,
                                        const std::vector<S>& sigma,
                                        return_type_t<T, M, S>& lp) {
  check_matching_dims("offset_multiplier_constrain", "x", x, "sigma", sigma);
  std::vector<plain_type_t<decltype(
      offset_multiplier_constrain(x[0], mu, sigma[0], lp))>>
      ret;
  ret.reserve(x.size());
  const auto& mu_ref = to_ref(mu);
  for (size_t i = 0; i < x.size(); ++i) {
    ret.emplace_back(offset_multiplier_constrain(x[i], mu_ref, sigma[i], lp));
  }
  return ret;
}

/**
 * Overload for array of x and mu and non-array sigma
 */
template <typename T, typename M, typename S,
          require_not_std_vector_t<S>* = nullptr>
inline auto offset_multiplier_constrain(const std::vector<T>& x,
                                        const std::vector<M>& mu,
                                        const S& sigma) {
  check_matching_dims("offset_multiplier_constrain", "x", x, "mu", mu);
  std::vector<
      plain_type_t<decltype(offset_multiplier_constrain(x[0], mu[0], sigma))>>
      ret;
  ret.reserve(x.size());
  const auto& sigma_ref = to_ref(sigma);
  for (size_t i = 0; i < x.size(); ++i) {
    ret.emplace_back(offset_multiplier_constrain(x[i], mu[i], sigma_ref));
  }
  return ret;
}

/**
 * Overload for array of x and mu and non-array sigma with lp
 */
template <typename T, typename M, typename S,
          require_not_std_vector_t<S>* = nullptr>
inline auto offset_multiplier_constrain(const std::vector<T>& x,
                                        const std::vector<M>& mu,
                                        const S& sigma,
                                        return_type_t<T, M, S>& lp) {
  check_matching_dims("offset_multiplier_constrain", "x", x, "mu", mu);
  std::vector<plain_type_t<decltype(
      offset_multiplier_constrain(x[0], mu[0], sigma, lp))>>
      ret;
  ret.reserve(x.size());
  const auto& sigma_ref = to_ref(sigma);
  for (size_t i = 0; i < x.size(); ++i) {
    ret.emplace_back(offset_multiplier_constrain(x[i], mu[i], sigma_ref, lp));
  }
  return ret;
}

/**
 * Overload for array of x, mu, and sigma
 */
template <typename T, typename M, typename S>
inline auto offset_multiplier_constrain(const std::vector<T>& x,
                                        const std::vector<M>& mu,
                                        const std::vector<S>& sigma) {
  check_matching_dims("offset_multiplier_constrain", "x", x, "mu", mu);
  check_matching_dims("offset_multiplier_constrain", "x", x, "sigma", sigma);
  std::vector<plain_type_t<decltype(
      offset_multiplier_constrain(x[0], mu[0], sigma[0]))>>
      ret;
  ret.reserve(x.size());
  for (size_t i = 0; i < x.size(); ++i) {
    ret.emplace_back(offset_multiplier_constrain(x[i], mu[i], sigma[i]));
  }
  return ret;
}

/**
 * Overload for array of x, mu, and sigma with lp
 */
template <typename T, typename M, typename S>
inline auto offset_multiplier_constrain(const std::vector<T>& x,
                                        const std::vector<M>& mu,
                                        const std::vector<S>& sigma,
                                        return_type_t<T, M, S>& lp) {
  check_matching_dims("offset_multiplier_constrain", "x", x, "mu", mu);
  check_matching_dims("offset_multiplier_constrain", "x", x, "sigma", sigma);
  std::vector<plain_type_t<decltype(
      offset_multiplier_constrain(x[0], mu[0], sigma[0], lp))>>
      ret;
  ret.reserve(x.size());
  for (size_t i = 0; i < x.size(); ++i) {
    ret.emplace_back(offset_multiplier_constrain(x[i], mu[i], sigma[i], lp));
  }
  return ret;
}

}  // namespace math
}  // namespace stan

#endif
