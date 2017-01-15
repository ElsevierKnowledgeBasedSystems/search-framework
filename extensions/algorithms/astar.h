#ifndef SLB_EXTENSIONS_ALGORITHMS_ASTAR_H
#define SLB_EXTENSIONS_ALGORITHMS_ASTAR_H

/// \file
/// \brief The \ref slb::ext::algorithm::Astar class.
/// \author Meir Goldenberg

#include "algorithm.h"

namespace slb {
namespace ext {
namespace algorithm {

template <ALG_TPARAMS_NO_DEFAULTS, template <class> class Open>
struct Astar; // Forward declare for the following type traits to work.

/// The traits of \ref Astar.
/// \tparam logFlag If \c true, the events generated by the search algorithm are
/// logged. Otherwise, they are not.
/// \tparam Node_ The search node type.
/// \tparam GoalHandler The policy for handling goal conditions.
/// \tparam Heuristic The heuristic used by the search algorithm.
/// \tparam Open The open list type.
template <ALG_TPARAMS_NO_DEFAULTS, template <class> class Open>
struct AlgorithmTraits<ext::algorithm::Astar<ALG_TARGS, Open>> {
    BASE_TRAITS_TYPES
    /// The search algorithm, \ref Astar in this case.
    using MyAlgorithm = ext::algorithm::Astar<ALG_TARGS, Open>;
    /// The open and closed lists type.
    using OC = OpenClosedList<Open<MyAlgorithm>>;
    /// The generator policy.
    using Generator = Generator_<MyAlgorithm>;
    //using Neighbor = typename Generator::Neighbor;
};

/// The \c A* search algorithm.
/// \tparam logFlag If \c true, the events generated by the search algorithm are
/// logged. Otherwise, they are not.
/// \tparam Node_ The search node type.
/// \tparam GoalHandler The policy for handling goal conditions.
/// \tparam InitialHeuristic The initial heuristic used by the search algorithm.
/// \tparam Open The open list type.
/// \note See the documentation for \ref ALG_TPARAMS and \ref ALG_DATA.
template <ALG_TPARAMS, template <class> class Open_ = SLB_OL>
struct Astar : Algorithm<Astar<ALG_TARGS, Open_>, ALG_TARGS> {
    BASE_TRAITS_TYPES
    /// The open list type.
    using Open = Open_<Astar>;

    /// OpenClosed list type.
    using OC = typename AlgorithmTraits<Astar>::OC;

    /// Search neighbor type.
    using Neighbor = typename AlgorithmTraits<Astar>::Generator::Neighbor;

    /// The direct base.
    using DirectBase = Algorithm<Astar<ALG_TARGS, Open_>, ALG_TARGS>;

    /// The goal handler policy.
    using GoalHandler = typename DirectBase::GoalHandler;

    /// The initial heuristic policy.
    using InitialHeuristic = typename DirectBase::InitialHeuristic;

    /// The generator policy.
    using Generator = typename DirectBase::Generator;

    using MyType = Astar; ///< The type of \ref Astar; required for \ref ALG_DATA symbol.
    ALG_DATA

    using DistanceMap =
        std::unordered_map<State, CostType, core::util::StateHash<State>>;

    /// Initializes the algorithm based on the problem instance.
    /// \param instance The problem instance.
    Astar(const MyInstance &instance)
        : Base(instance), oc_(*this), cur_(nullptr) {}

    /// Runs the algorithm.
    /// \return The solution cost. If there is no solution, then \c CostType{-1}
    /// is returned.
    ReturnType run() {
        //std::cerr << "Starting the timing!" << std::endl;
        TimerLock lock{time_}; (void)lock;
        NodeUniquePtr startNode(new Node(start_));
        startNode->set(0, initialHeuristic_(startNode.get()), this->stamp());
        log<ext::event::MarkedStart>(log_, startNode.get());
        goalHandler_.logInit();
        oc_.add(startNode);
        while (!oc_.empty() && !goalHandler_.done()) {
            cur_ = oc_.minNode();
            log<ext::event::Selected>(log_, cur_);
            handleSelected();
        }
        if (!goalHandler_.done()) res_ = -1;
        cost_.set(res_);
        return res_;
    }

    /// Returns the statistics about the search algorithm's
    /// performance for solving the particular instance.
    /// \return The statistics about the search algorithm's
    /// performance for solving the particular instance.
    MeasureSet measures() const {
        return Base::measures().append(MeasureSet{denied_});
    }

    /// Computes the state graph based on the closed list.
    /// \return The state graph computed based on the closed list.
    core::ui::StateGraph<State> graph() const {
        core::ui::StateGraph<State> res;
        for (const auto &el: oc_.hash()) {
            auto from = core::util::make_deref_shared<const State>(
                el.second->state());
            res.add(from);
            for (auto &n : from->stateSuccessors()) {
                // `add` cares for duplicates
                res.add(from, core::util::make_deref_shared<const State>(
                                  n.state()),
                        n.cost());
            }
        }
        return res;
    }

    /// Computes the distance to the given state.
    /// Returns CostType{-1} if that distance has not been computed.
    /// \param s The state.
    /// \return The distance to \c s.
    CostType distance(const State &s) {
        auto n = oc_.getNode(s);
        if (!n) return CostType{-1};
        return n->g;
    }

    /// Computes the distance map based on the current state of the closed list.
    /// \return The distance map based on the current state of the closed list.
    DistanceMap distanceMap() const {
        DistanceMap res;
        for (const auto &el : oc_.hash()) res[el.first] = el.second->g;
        return res;
    }

    /// \name Services for policies.
    /// @{

    OC &oc() { return oc_; }
    Node *cur() { return cur_; }
    Measure &denied() { return denied_; }
    void recomputeOpen() { oc_.recomputeOpen(); }
    template <typename P>
    void partialRecomputeOpen(const P &p) { oc_.partialRecomputeOpen(p); }

    /// @}
private:
    OC oc_; ///< The open and closed lists.

    Node *cur_; ///< The currently selected node.

    // Stats
    Measure denied_{"Denied"}; ///< The number of denied expansions.

    /// Handles the selected node.
    void handleSelected() {
        // Tag dispatch to call the correct version depending on whether the
        // goal handler is dealing with the issue of suspending expansions.
        // In the latter case, onSelect returns bool, otherwise it returns
        // void.
        handleSelected(
            std::integral_constant<
                bool, policy::goalHandler::onSelectReturns<GoalHandler>()>());
    }

    /// Handles the selected node in the case that suspensions are to be dealt
    /// with.
    void handleSelected(std::true_type) {
        if (goalHandler_.onSelect()) expand();
    }

    /// Handles the selected node in the case that suspensions do not need to be
    /// dealt with.
    void handleSelected(std::false_type) {
        goalHandler_.onSelect();
        expand();
    }

    /// Expands the selected node.
    void expand() {
        if (goalHandler_.done()) {
            // std::cout << "Done!" << std::endl;
            return;
        }
        ++expanded_;
        auto neighbors = generator_.successors(cur_->state());
        for (auto &n : neighbors)
            handleNeighbor(n);
        log<ext::event::Closed>(log_, cur_);
    }

    /// Handles the current neighbor.
    /// \param n The current neighbor.
    void handleNeighbor(Neighbor &n) {
        auto childState = generator_.state(n);
        CostType myG = cur_->g + n.cost();
        ++generated_;
        auto childNode = oc_.getNode(childState);
        if (childNode) {
            if (myG < childNode->g) {
                // only consistent case for now
                // assert(childNode->bucketPosition() >= 0);
                log<ext::event::NotParent>(log_, childNode);
                auto oldPriority = oc_.priority(childNode);
                childNode->updateG(myG);
                childNode->setParent(cur_);
                oc_.update(childNode, oldPriority);
                log<ext::event::Generated>(log_, childNode);
                log<ext::event::EnteredOpen>(log_, childNode);
            }
            else {
                log<ext::event::NothingToDo>(log_, childNode, cur_);
                log<ext::event::HideLast>(log_, childNode);
            }

            return;
        }
        NodeUniquePtr newNode(new Node(childState));
        newNode->setParent(cur_);
        newNode->set(myG, generator_.heuristic(n, newNode.get()),
                     this->stamp());
        log<ext::event::Generated>(log_, newNode.get());
        log<ext::event::EnteredOpen>(log_, newNode.get());
        oc_.add(newNode);
    }
};

} // namespace
} // namespace
} // namespace

#endif
