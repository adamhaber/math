#ifndef STAN_MATH_REV_FUN_MULTIPLY_LOWER_TRI_SELF_TRANSPOSE_HPP
#define STAN_MATH_REV_FUN_MULTIPLY_LOWER_TRI_SELF_TRANSPOSE_HPP

#include <stan/math/rev/meta.hpp>
#include <stan/math/rev/core.hpp>
#include <stan/math/rev/fun/dot_product.hpp>
#include <stan/math/rev/fun/dot_self.hpp>
#include <stan/math/rev/fun/typedefs.hpp>
#include <stan/math/prim/fun/Eigen.hpp>

namespace stan {
namespace math {

template <typename T, require_eigen_vt<is_var, T>* = nullptr>
inline auto multiply_lower_tri_self_transpose(const T& L) {
  using ret_type = promote_scalar_t<var, decltype(L * L.transpose())>;

  if (L.size() == 0)
    return ret_type();

  arena_t<T> arena_L = L.transpose();

  auto arena_L_val
      = to_arena(value_of(arena_L).template triangularView<Eigen::Upper>());

  if (L.size() > 16) {
    arena_t<ret_type> res
        = arena_L_val.transpose()
          * arena_L_val.template triangularView<Eigen::Upper>();

    reverse_pass_callback([res, arena_L, arena_L_val]() mutable {
      const auto& adj = to_ref(res.adj());

      arena_L.adj() += (arena_L_val.template triangularView<Eigen::Upper>()
                        * (adj.transpose() + adj))
                           .template triangularView<Eigen::Upper>();
    });

    return ret_type(res);
  } else {
    arena_t<ret_type> res = arena_L_val.transpose().lazyProduct(arena_L_val);

    reverse_pass_callback([res, arena_L, arena_L_val]() mutable {
      const auto& adj = to_ref(res.adj());

      arena_L.adj() += (arena_L_val.lazyProduct(adj.transpose() + adj))
                           .template triangularView<Eigen::Upper>();
    });

    return ret_type(res);
  }
}

}  // namespace math
}  // namespace stan
#endif
