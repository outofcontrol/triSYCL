#ifndef TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_WAITER_HPP
#define TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_WAITER_HPP

/** \file A helper class to wait for the buffer<> detail

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>
#include <future>

// \todo Use C++17 optional when it is mainstream
#include <boost/optional.hpp>

#include "CL/sycl/buffer/detail/buffer.hpp"
#include "CL/sycl/buffer_allocator.hpp"
#include "CL/sycl/detail/shared_ptr_implementation.hpp"

namespace cl {
namespace sycl {
namespace detail {

/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** A helper class to wait for the final buffer destruction if the
    conditions for blocking are met
*/
template <typename T,
          std::size_t Dimensions = 1,
          typename Allocator = buffer_allocator<T>>
class buffer_waiter :
    public detail::shared_ptr_implementation<buffer_waiter<T,
                                                           Dimensions,
                                                           Allocator>,
                                             detail::buffer<T, Dimensions>>,
    detail::debug<buffer_waiter<T, Dimensions, Allocator>> {

  // The type encapsulating the implementation
  using implementation_t =
    detail::shared_ptr_implementation<buffer_waiter<T, Dimensions, Allocator>,
                                      detail::buffer<T, Dimensions>>;

public:

  // Make the implementation member directly accessible in this class
  using implementation_t::implementation;

  /// Create a new buffer_waiter on top of a detail::buffer
  buffer_waiter(detail::buffer<T, Dimensions> *b) : implementation_t { b } {}


  /** The buffer_waiter destructor waits for any data to be written
      back to the host, if any
  */
  ~buffer_waiter() {
    /* Get a future from the implementation if we have to wait for its
       destruction */
    auto f = implementation->get_destructor_future();
    if (f) {
      /* No longer carry for the implementation buffer which is free to
         live its life up to its destruction */
      implementation.reset();
      TRISYCL_DUMP_T("~buffer_waiter() is waiting");
      // Then wait for its end in some other thread
      f->wait();
      TRISYCL_DUMP_T("~buffer_waiter() is done");
    }
  }
};


/// Helper function to create a new buffer_waiter
template <typename T,
          std::size_t Dimensions = 1>
inline auto waiter(detail::buffer<T, Dimensions> *b) {
  return new buffer_waiter<T, Dimensions> { b };
}

/// @} End the data Doxygen group

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_BUFFER_DETAIL_BUFFER_WAITER_HPP
