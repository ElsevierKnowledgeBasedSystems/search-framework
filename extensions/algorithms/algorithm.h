#ifndef ALOGORITHM_H
#define ALOGORITHM_H

/// \file
/// \brief Abstract base for search algorithms.
/// \author Meir Goldenberg

/// Common template parameters for search algorithms.
// See http://stackoverflow.com/q/34130672/2725810 for the discussion of
// the use of template template parameters.
#define ALG_TPARAMS                                                            \
    bool logFlag_, class Node_ = SLB_NODE,                                     \
                         template <class>                                      \
                         class GoalHandler_ = SLB_GOAL_HANDLER,                \
                         template <class>                                      \
                         class InitialHeuristic_ = SLB_INIT_HEURISTIC,         \
                         template <class>                                      \
                         class Generator_ = SLB_GENERATOR

/// Same as \ref ALG_TPARAMS, but without default values; needed for partial
/// specializations of AlgorithmTraits.
#define ALG_TPARAMS_NO_DEFAULTS                                                \
    bool logFlag_, class Node_, template <class>                               \
                                class GoalHandler_, template <class>           \
                                                    class InitialHeuristic_,   \
        template <class>                                                       \
        class Generator_

/// Template arguments to be used for inheriting from \ref
/// slb::ext::algorithm::Algorithm.
#define ALG_TARGS logFlag_, Node_, GoalHandler_, InitialHeuristic_, Generator_

/// Data members of \ref slb::ext::algorithm::Algorithm to be used by all search
/// algorithms.
#define ALG_DATA                                                               \
    using Base = Algorithm<MyType, ALG_TARGS>;                                 \
    using Base::log_;                                                          \
    using Base::instance_;                                                     \
    using Base::start_;                                                        \
    using Base::goalHandler_;                                                  \
    using Base::initialHeuristic_;                                                    \
    using Base::generator_;                                                    \
    using Base::expanded_;                                                     \
    using Base::generated_;                                                    \
    using Base::time_;                                                         \
    using Base::cost_;                                                         \
    using Base::res_;

/// Type aliases to be defined by all search algorithms. See the documentation
/// for these aliases inside the \ref slb::ext::algorithm::Algorithm class.
#define BASE_TRAITS_TYPES                                   \
    using Traits = BaseTraits<ALG_TARGS>;                   \
    using Node = typename Traits::Node;                     \
    using NodeData = typename Traits::NodeData;             \
    using CostType = typename Traits::CostType;             \
    using ReturnType = double;                              \
    using NodeUniquePtr = typename Traits::NodeUniquePtr;   \
    using State = typename Traits::State;                   \
    using MyInstance = typename Traits::MyInstance;         \
    using MyAlgorithmLog = typename Traits::MyAlgorithmLog;

namespace slb {
namespace ext {
namespace algorithm {

/// Traits shared by all algorithms.
template <ALG_TPARAMS>
struct BaseTraits {
    /// The search node type.
    using Node = Node_;

    /// The type of data stored with a search node.
    using NodeData = typename Node::NodeData;

    /// Type for action cost in the search domain.
    using CostType = typename Node::CostType;

    /// Type for the return value of the algorithm. Not the same as CostType due
    /// to possible average among several goals.
    using ReturnType = double;

    /// Unique pointer to search node.
    using NodeUniquePtr = typename Node::NodeUniquePtr;

    /// The state type, represents the domain.
    using State = typename Node::State;

    /// The problem instance type.
    using MyInstance = Instance<State>;

    /// The type for the log of events generated by the algorithm.
    using MyAlgorithmLog =
        typename std::conditional<logFlag_, AlgorithmLog<Node>, Nothing>::type;
};

/// Types associated with concrete algorithms. See also http://stackoverflow.com/questions/8401827/crtp-and-type-visibility
/// \tparam MyAlgorithm The search algorithm.
template <typename MyAlgorithm> class AlgorithmTraits {};

/// Abstract base for search algorithms.
/// \tparam logFlag_ If \c true, the events generated by the search algorithm
/// are logged. Otherwise, they are not.
/// \tparam Node_ The search node type.
/// \tparam GoalHandler The policy for handling goal conditions.
/// \tparam Heuristic The heuristic used by the search algorithm.
/// \tparam Generator The generator used by the search algorithm.
template <class Concrete, ALG_TPARAMS>
struct Algorithm {
    BASE_TRAITS_TYPES

    /// \c true if the events generated by the search algorithm
    /// are logged and \c false otherwise.
    static constexpr bool logFlag = logFlag_;

    // The following types cannot be part of BASE_TRAITS_TYPES, since they are
    // dependent on Concrete.

    /// The goal handler policy type.
    using GoalHandler = GoalHandler_<Concrete>;

    /// The initial heuristic policy type.
    using InitialHeuristic = InitialHeuristic_<Concrete>;

    /// The generator policy type.
    using Generator = Generator_<Concrete>;

    /// Returns the statistics about the search algorithm's
    /// performance for solving the particular instance.
    /// \return The statistics about the search algorithm's
    /// performance for solving the particular instance.
    MeasureSet measures() const {
        return {time_, cost_, expanded_, generated_};
    }

    /// Returns the log of events generated by the search algorithm.
    /// \return Const reference to the log of events generated by the search
    /// algorithm.
    MyAlgorithmLog &log() { return log_; }

    /// \name Services for policies.
    /// @{

    MyInstance &instance() { return instance_; }
    GoalHandler &goalHandler() { return goalHandler_; }
    InitialHeuristic &initalHeuristic() { return initialHeuristic_; }
    Generator &generator() { return generator_; }

    /// Returns the current time stamp of the search algorithm.
    /// The number of generated nodes is currently used as the stamp.
    /// \return The current time stamp of the search algorithm.
    int stamp() { return generated_.value(); }

    /// Returns reference to the result.
    /// \return Reference to the result.
    ReturnType &res() { return res_; }

    /// @}
protected:
    /// Initializes the algorithm based on the problem instance.
    /// \param instance The problem instance.
    Algorithm(const MyInstance &instance)
        : instance_(instance), start_(instance_.start()),
          goalHandler_(static_cast<Concrete &>(*this)),
          initialHeuristic_(static_cast<Concrete &>(*this)),
          generator_(static_cast<Concrete &>(*this)) {}

    MyInstance instance_; ///< The problem instance.

    /// The log of events generated by the search algorithm.
    MyAlgorithmLog log_;

    /// The start state (multiple start states are to be handled in the future).
    State start_;

    /// The policy for handling conditions related to goal states.
    GoalHandler goalHandler_;

    /// The initial heuristic used by the search algorithm.
    InitialHeuristic initialHeuristic_;

    /// The generator used by the search algorithm.
    Generator generator_;

    /// \name Statistics about the search algorithm's performance.
    /// \note These statistics pertain to solving the particular instance.
    /// @{

    /// The number of nodes expanded by the search algorithm.
    Measure expanded_{"Expanded"};

    /// The number of nodes generated by the search algorithm.
    Measure generated_{"Generated"};

    /// Time taken by the search algorithm.
    Timer time_{"Time (ms.)"};

    /// The cost of the solution.
    Measure cost_{"Cost"};

    /// The solution cost. -1 stands for no solution.
    ReturnType res_{-1};
    /// @}
};

} // namespace
} // namespace
} // namespace

#endif
