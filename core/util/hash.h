#ifndef UTIL_HASH_H
#define UTIL_HASH_H

///@file
///@brief Hashing-related utilities.
/// \author Meir Goldenberg

namespace slb {
namespace core {
namespace util {

/// A functor for computing the hash-value of a search state.
/// \tparam State The search state type.
template <class State>
struct StateHash {
    /// The call operator. Computes the hash-value of a search state.
    /// \param s The state.
    /// \return The hash-value of \c s.
    std::size_t operator()(const State &s) const {
        return s.hash();
    }
};

/// A functor for computing the hash-value of a search state based on a smart
/// pointer.
/// \tparam State The search state type.
template <class State>
struct StateSharedPtrHash {
    /// The call operator. Computes the hash-value of a search state based on a
    /// smart pointer.
    /// \param s The smart pointer to the state.
    /// \return The hash-value of \c s.
    std::size_t operator()(const deref_shared_ptr<const State> &s) const {
        return s->hash();
    }
};

} // namespace
} // namespace
} // namespace

#endif
