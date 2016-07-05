#ifndef GOAL_HANDLER_H
#define GOAL_HANDLER_H

/// \file
/// \brief Policies for handling conditions related to goal states.
/// \author Meir Goldenberg

template <class Handler>
constexpr bool onSelectReturns() {
    return std::is_same<decltype(std::declval<Handler>().onSelect()),
                        bool>::value;
}

/// Handling conditions related to goal states for the case when the set of the
/// goal states is empty.
/// \tparam Algorithm The search algorithm.
template <class MyAlgorithm> // for uniformity
struct NoGoalHandler {
    POLICY_TYPES

    /// Initializes the policy based on a problem instance and a log of events
    /// generated by the search algorithm.
    NoGoalHandler(MyAlgorithm &) {}

    /// Handles selection of a node by the search algorithm.
    void onSelect() {}

    /// Returns \c true if all the goals were found with the required quality
    /// and the search algorithm should terminate. Returns \c false otherwise.
    /// \return \c true if all the goals were found with the required quality
    /// and \c false otherwise.
    bool done() const {return false;}

    /// Passes the goals to the log of the search algorithm. In this particular
    /// case, there are no goals to pass.
    /// \tparam Node The search node type.
    void logInit() {}
};

/// Handling conditions related to goal states for the case when there is a
/// single goal state.
template <class State, class Log>
struct SingleGoalHandler {
    /// The problem instance type.
    using MyInstance = Instance<State>;

    /// Type for action cost in the search domain.
    using CostType = typename State::CostType;

    /// Initializes the policy based on a problem instance and a log of events
    /// generated by the search algorithm.
    /// \param instance The problem instance.
    /// \param log The log of events generated by the search algorithm.
    SingleGoalHandler(MyInstance &instance, Log &log)
        : goal_(instance.goal()), log_(log) {}

    /// Handles selection of a node by the search algorithm.
    /// \tparam Node The search node type.
    /// \param n Pointer to the node that was selected by the search algorithm.
    /// \param res The cost that will be returned by the search algorithm to be
    /// filled out. In this case, it's trivially \c n->f when \c n is
    /// corresponds to the goal.
    template <class Node> void onSelect(const Node *n, CostType &res) {
        if (n->state() == goal_) {
            // std::cerr << "DONE_GOAL " << n->state() << " " << n->f
            //           << std::endl;
            res = n->g;
            done_ = true;
            log<Events::SolvedGoal>(log_, n);
            log<Events::HideLast>(log_, n);
        }
    }

    /// Returns \c true if all the goals were found with the required quality
    /// and the search algorithm should terminate. Returns \c false otherwise.
    /// \return \c true if all the goals were found with the required quality
    /// and \c false otherwise.
    bool done() const {return done_;}

    /// Passes the goals to the log of the search algorithm. In this particular
    /// case, there is only a single goal to pass.
    /// \tparam Node The search node type.
    template <class Node> void logInit() {
        log<Events::MarkedGoal>(log_, std::make_shared<Node>(goal_).get());
    }
private:
    State goal_; ///< The goal state.
    Log &log_; ///< The log of events generated by the search algorithm.
    bool done_ = false; ///< Flag showing whether the search algorithm has
                        ///accomplished its mission and should terminate.
};

/// Handling conditions related to goal states for the case when there are
/// multiple goal states.
/// \tparam MyAlgorithm The search algorithm.
template <class MyAlgorithm>
struct MultipleGoalHandler {
    POLICY_TYPES

    /// The type for a goal that has not been solved with the required quality.
    using ActiveGoal = std::pair<State, int>;

    /// Initializes the policy based on a problem instance and a log of events
    /// generated by the search algorithm.
    /// \param alg Reference to the search algorithm.
    MultipleGoalHandler(MyAlgorithm &alg) : alg_(alg), log_(alg.log()) {
        auto const &goals = alg.instance().goals();
        int i = -1;
        for (const auto &g: goals)
            activeGoals_.push_back(std::make_pair(g, ++i));
    }

    /// Handles selection of a node by the search algorithm.
    void onSelect() {
        Node *n = alg_.cur();
        if (isActive<true>(n->state())) doneGoal(n);
    }

    /// Returns \c true if all the goals were found with the required quality
    /// and the search algorithm should terminate. Returns \c false otherwise.
    /// \return \c true if all the goals were found with the required quality
    /// and \c false otherwise.
    bool done() const {return done_;}

    /// Passes the goals to the log of the search algorithm.
    /// \tparam Node The search node type.
    void logInit() {
        for (auto &g : activeGoals_)
            log<Events::MarkedGoal>(log_,
                                    std::make_shared<Node>(g.first).get());
    }

    /// Returns the active goals.
    /// \return Const reference to the vector of active goals.
    const std::vector<ActiveGoal> &activeGoals() { return activeGoals_; }

protected:
    /// Reference to the search algorithm.
    MyAlgorithm &alg_;

    /// The log of events generated by the search algorithm.
    Log &log_;

    /// The active goal states.
    std::vector<ActiveGoal> activeGoals_;

    /// Flag showing whether the search algorithm has accomplished its mission
    /// and should terminate.
    bool done_ = false;

    template <bool eraseFlag = false>
    bool isActive(const State &goal) {
        for (auto it = activeGoals_.begin(); it != activeGoals_.end(); ++it)
            if (it->first == goal) {
                if (eraseFlag)
                    activeGoals_.erase(it);
                return true;
            }
        return false;
    }

    virtual void doneGoal(Node *n) {
        log<Events::SolvedGoal>(log_, n);
        log<Events::HideLast>(log_, n);
        CostType &res = alg_.res();
        int nDoneGoals = alg_.instance().goals().size() - activeGoals_.size();
        res = (res * (nDoneGoals - 1) + n->g) / nDoneGoals;
        if (activeGoals_.empty()) done_ = true;
    }
};

/// Handling conditions related to goal states for the case when there are
/// multiple goal states and a heuristic that stores the goal state responsible
/// for the heuristic value is used.
/// \tparam MyAlgorithm The search algorithm.
template <class MyAlgorithm>
struct MinHeuristicGoalHandler: MultipleGoalHandler<MyAlgorithm> {
    POLICY_TYPES
    /// The direct base.
    using DirectBase = MultipleGoalHandler<MyAlgorithm>;

    /// Using the direct base's constructors.
    using DirectBase::DirectBase;

    using DirectBase::alg_;
    using DirectBase::log_;
    using DirectBase::activeGoals_;
    using DirectBase::done_;
    using DirectBase::isActive;

    /// Handles selection of a node by the search algorithm. If the goal
    /// responsible for the heuristic value of the node had been found with the
    /// required quality, the heuristic is re-computed based on the remaining
    /// goals. If the node's cost grows based on that check, then \c false is
    /// returned indicating the denial of expansion of this node.
    /// \return \c true if the currently selected node should be expanded and \c
    /// false otherwise.
    bool onSelect() {
        Node *n = alg_.cur();

        // Check identity of goal resposible for heuristic
        if (!isActive(n->responsibleGoal)) {
            log<Events::SuspendedExpansion>(log_, n);
            return !isDenied(n);
        }

        if (this->isActive<true>(n->state())) {
            this->doneGoal(n);
            update(n);
            return false;
        }

        return true;
    }

protected:
    template <bool eraseFlag = false>
    bool isActive(int goalId) {
        for (auto it = activeGoals_.begin(); it != activeGoals_.end(); ++it)
            if (it->second == goalId) {
                if (eraseFlag)
                    activeGoals_.erase(it);
                return true;
            }
        return false;
    }

    bool isDenied(Node *n) {
        auto oldCost = n->f;
        n->f = n->g + alg_.heuristic()(n);
        if (n->f > oldCost) {
            ++alg_.denied();
            log<Events::DeniedExpansion>(log_, n);
            alg_.oc().reInsert(n);
            return true;
        }
        log<Events::ResumedExpansion>(log_, n);
        return false;
    }

    void update(Node *n) {
        n->updateH(alg_.heuristic());
        alg_.oc().reInsert(n);
    }
};

/// Handling conditions related to goal states for the case when there are
/// multiple goal states and the whole open list must be re-computed whenever
/// a goal is solved with the required quality.
/// \tparam MyAlgorithm The search algorithm.
template <class MyAlgorithm>
struct MaxHeuristicGoalHandler: MultipleGoalHandler<MyAlgorithm> {
    POLICY_TYPES
    /// The direct base.
    using DirectBase = MultipleGoalHandler<MyAlgorithm>;

    /// Using the direct base's constructors.
    using DirectBase::DirectBase;

    using DirectBase::alg_;

    /// Returns the time stamp of the last found goal. This is also when the
    /// open list was re-computed last.
    /// \return /// The time stamp when the last goal was found.
    int stamp() { return stamp_; }
protected:
    /// Time stamp when the last goal was found.
    int stamp_;

    /// Handles the found goal. Recomputes the open list and saves the time
    /// stamp.
    /// \param n The goal node.
    virtual void doneGoal(Node *n) {
        DirectBase::doneGoal(n);
        alg_.recomputeOpen();
        stamp_ = alg_.stamp();
    }
};

#endif
