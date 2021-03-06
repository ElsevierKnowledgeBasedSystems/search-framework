#ifndef SLB_CORE_SEARCH_BASE_HEADERS_H
#define SLB_CORE_SEARCH_BASE_HEADERS_H

/// \file
/// \brief Facilities that support implementation of various heuristic search
/// algorithms.
/// \author Meir Goldenberg

namespace slb {
namespace core {

/// \namespace slb::core::sb
/// The search base. These are facilities that support implementation of various
/// heuristic search algorithms.
namespace sb {
using util::operator<<; // Printing states.
} // namespace
} // namespace
} // namespace

#include "instance.h"
#include "open_closed_list.h"
#include "domain_base.h"
#include "explicit_state.h"
#include "neighbor.h"
#include "node.h"

#endif
