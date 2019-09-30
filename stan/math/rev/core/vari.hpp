#ifndef STAN_MATH_REV_CORE_VARI_HPP
#define STAN_MATH_REV_CORE_VARI_HPP

#include <stan/math/rev/core/chainable_alloc.hpp>
#include <stan/math/rev/core/chainablestack.hpp>
#include <ostream>
#include <utility>

namespace stan {
namespace math {

// forward declaration of var
class var;

/**
 * The variable implementation base class.
 *
 * This class is complete (not abstract) and may be used for
 * constants.
 *
 * A variable implementation is constructed with a constant
 * value. It also stores the adjoint for storing the partial
 * derivative with respect to the root of the derivative tree.
 *
 * The chain() method applies the chain rule. Concrete extensions
 * of this class will represent base variables or the result
 * of operations such as addition or subtraction. These extended
 * classes will store operand variables and propagate derivative
 * information via an implementation of chain().
 */
class vari {
 private:
  friend class var;

 public:
  /**
   * The value of this variable.
   */
  const double val_{0.0};

  /**
   * The adjoint of this variable, which is the partial derivative
   * of this variable with respect to the root variable.
   */
  double adj_{0.0};
  bool stacked_{false};

  /**
   * Construct a variable implementation from a value.  The
   * adjoint is initialized to zero.
   *
   * All constructed variables are added to the stack.  Variables
   * should be constructed before variables on which they depend
   * to insure proper partial derivative propagation.  During
   * derivative propagation, the chain() method of each variable
   * will be called in the reverse order of construction.
   *
   * @param x Value of the constructed variable.
   */
  explicit vari(double x) : val_(x) {
    ChainableStack::instance_->var_stack_.emplace_back(this);
  }
  vari(double x, bool stacked) : val_(x), stacked_(stacked) {
    if (stacked) {
      ChainableStack::instance_->var_stack_.emplace_back(this);
    } else {
      ChainableStack::instance_->var_nochain_stack_.emplace_back(this);
    }
  }

  /**
   * Throw an illegal argument exception.
   *
   * <i>Warning</i>: Destructors should never called for var objects.
   *
   * @throw Logic exception always.
   */
  virtual ~vari() {
    // this will never get called
  }

  /**
   * Vari default constructor assumes the var goes on the stack tape
   */
  vari() {
    ChainableStack::instance_->var_stack_.emplace_back(this);
  };

  /**
   * Copy constructor
   * @param x vari to be copied
   */
  explicit vari(const vari& x) : val_(x.val_), adj_(x.adj_), stacked_(x.stacked_) {
    if (this->stacked_) {
      ChainableStack::instance_->var_stack_.emplace_back(this);
    } else {
      ChainableStack::instance_->var_nochain_stack_.emplace_back(this);
    }
  }

  /**
   * Copy assignment
   * @param x vari to be assigned by copies of it's elements
   */
  vari& operator=(const vari& x) {
    //this->val_ = x.val_;
    // Adjoint is const and cannot be assigned to
    this->adj_ = x.adj_;
    if (x.stacked_ && !this->stacked_) {
      ChainableStack::instance_->var_stack_.emplace_back(this);
    } else if (!x.stacked_ && this->stacked_) {
      ChainableStack::instance_->var_nochain_stack_.emplace_back(this);
    }
    this->stacked_ = x.stacked_;
    return *this;

  }

  /**
   * Move constructor
   * @param x vari to be assigned
   * Note: Because these members are 'plain old data' copies will be as fast as
   * moves while still leaving 'moved' from objects in a valid and coherent state.
   */
  explicit vari(vari&& x) noexcept : val_(x.val_), adj_(x.adj_), stacked_(x.stacked_) {
    if (this->stacked_) {
      ChainableStack::instance_->var_stack_.emplace_back(std::move(this));
    } else {
      ChainableStack::instance_->var_nochain_stack_.emplace_back(std::move(this));
    }
  }

  /**
   * Move constructor
   * @param x vari to be assigned
   * Note: Because these members are plain old data copies will be as fast as
   * moves will still leaving 'moved' from object in a valid and coherent state.
   */
  vari& operator=(vari&& x) noexcept {
    //this->val_ = x.val_;
    // Adjoint is const and cannot be assigned to
    this->adj_ = x.adj_;
    if (x.stacked_ && !this->stacked_) {
      ChainableStack::instance_->var_stack_.emplace_back(std::move(this));
    } else if (!x.stacked_ && this->stacked_) {
      ChainableStack::instance_->var_nochain_stack_.emplace_back(std::move(this));
    }
    this->stacked_ = x.stacked_;
    return *this;
  };

  /**
   * Apply the chain rule to this variable based on the variables
   * on which it depends.  The base implementation in this class
   * is a no-op.
   */
  virtual void chain() {}

  /**
   * Initialize the adjoint for this (dependent) variable to 1.
   * This operation is applied to the dependent variable before
   * propagating derivatives, setting the derivative of the
   * result with respect to itself to be 1.
   */
  void init_dependent() { adj_ = 1.0; }

  /**
   * Set the adjoint value of this variable to 0.  This is used to
   * reset adjoints before propagating derivatives again (for
   * example in a Jacobian calculation).
   */
  void set_zero_adjoint() { adj_ = 0.0; }

  /**
   * Insertion operator for vari. Prints the current value and
   * the adjoint value.
   *
   * @param os [in, out] ostream to modify
   * @param v [in] vari object to print.
   *
   * @return The modified ostream.
   */
  friend std::ostream& operator<<(std::ostream& os, const vari* v) {
    return os << v->val_ << ":" << v->adj_;
  }

  /**
   * Allocate memory from the underlying memory pool.  This memory is
   * is managed as a whole externally.
   *
   * Warning: Classes should not be allocated with this operator
   * if they have non-trivial destructors.
   *
   * @param nbytes Number of bytes to allocate.
   * @return Pointer to allocated bytes.
   */
  static inline void* operator new(size_t nbytes) {
    return ChainableStack::instance_->memalloc_.alloc(nbytes);
  }

  /**
   * Delete a pointer from the underlying memory pool.
   *
   * This no-op implementation enables a subclass to throw
   * exceptions in its constructor.  An exception thrown in the
   * constructor of a subclass will result in an error being
   * raised, which is in turn caught and calls delete().
   *
   * See the discussion of "plugging the memory leak" in:
   *   http://www.parashift.com/c++-faq/memory-pools.html
   */
  static inline void operator delete(void* /* ignore arg */) { /* no op */
  }
};

}  // namespace math
}  // namespace stan
#endif
