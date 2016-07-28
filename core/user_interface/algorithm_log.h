#ifndef ALGORITHM_LOGGER_H
#define ALGORITHM_LOGGER_H

/// \file algorithm_log.h
/// \brief Facilities for keeping the log of events generated by search
/// algorithms: \ref AlgorithmLog class and the \ref log function.
/// \author Meir Goldenberg

namespace Events {
template <class Node> struct Base;
}

/// The log of events generated by an algorithm.
/// \tparam Node The search node type.
template <class Node>
struct AlgorithmLog {
    /// Smart pointer to an event generated by the algorithm.
    using Event = std::shared_ptr<Events::Base<Node>>;

    /// The state type, represents the domain.
    using State = typename Node::State;

    /// Smart pointer to state.
    using StateSharedPtr = std::shared_ptr<const State>;

    /// Adds an event relating to a given state to the log. This method should
    /// not normally be used directly. Rather, the free standing \ref log
    /// function should be used instead.
    /// \param s Shared pointer to state to which the event relates.
    /// \param e The event.
    void log(const StateSharedPtr &s, Event e) {
        stateToLastEventStep_[s] = events_.size();
        events_.push_back(e);
        push_back_unique(eventStrings_, e->eventStr());
    }

    /// Returns the number of events in the log.
    /// \return The number of events in the log.
    int size() const { return events_.size(); }

    /// Returns the event with the given index.
    /// \param step The index; indices begin at zero.
    /// \return The event with the index \c step.
    const Event event(int step) const { return events_[step]; }

    /// Outputs the log to a given stream.
    /// \tparam Stream The stream type.
    /// \param o The stream for output.
    /// \param dumpLastEvents If \c true, there will be an additional section in
    /// the output which has each state for which there are related events in
    /// the log with the last event recorded for that state.
    /// \return The stream \c o after performing the output.
    template <class Stream>
    Stream &dump(Stream &o, bool dumpLastEvents = false) const {
        o << std::right << std::setfill(' ') << "ALGORITHM LOG"
                  << std::endl;
        o << std::setw(4) << "Num.";
        Events::Base<Node>::dumpTitle(o);

        for (auto el: events_)
            o << *el << std::endl;
        if (!dumpLastEvents) return o;
        o << "\nLast events:" << std::endl;
        for (auto el: stateToLastEventStep_)
            o << *(el.first) << ": " << el.second << std::endl;
        return o;
    }

    /// Returns all events in the log.
    /// \return Reference to the vector of (smart pointers to) all events in the
    /// log.
    const std::vector<Event> &events() const {return events_;}

    /// Returns all unique descriptions of the events in the log.
    /// \return Reference to the vector of unique descriptions of the events in
    /// the log.
    const std::vector<std::string> &eventStrings() const {return eventStrings_;}

    /// Returns the last recorded event related to the given state. Optionally
    /// throws an exception if there is no recorded event for the given state.
    /// \param s Smart pointer to state.
    /// \param throwFlag If \c true, an exception is thrown if there is no
    /// recorded event for \c s.
    /// \return The last recorded event related to \c s.
    const Event getLastEvent(const StateSharedPtr &s,
                             bool throwFlag = true) const {
        auto it = stateToLastEventStep_.find(s);
        if (it == stateToLastEventStep_.end()) {
            if (!throwFlag) return nullptr;
            throw std::runtime_error("Could not find last event for " + str(*s));
        }
        return events_[it->second];
    }

    /// Returns the last event prior to the given point in time related to the
    /// given state. Optionally throws an exception if there is no recorded
    /// event for the given state.
    /// \param s Smart pointer to state.
    /// \param step The given point in time.
    /// \param throwFlag If \c true, an exception is thrown if there is no
    /// recorded event for \c s.
    /// \return The last recorded event related to \c s.
    const Event getLastEvent(const StateSharedPtr &s, int step,
                             bool throwFlag = true) const {
        auto e = getLastEvent(s, throwFlag);
        while (e && e->step() >= step)
            e = e->previousEvent();
        if (throwFlag && !e)
            throw std::runtime_error("Could not find last event before step " +
                                     str(step) + " for the state " + str(*s));
        return e;
    }
private:
    /// Maps each state to the last recorded event related to that state.
    std::unordered_map<StateSharedPtr, int, StateSharedPtrHash<State>,
                       StateSharedPtrEq<State>>
        stateToLastEventStep_;

    /// All events.
    std::vector<Event> events_;

    /// Event descriptions. Each string appears only once.
    std::vector<std::string> eventStrings_;
};

/// Used by the tag dispatch of \ref log for the case of no log.
template <template <class> class Event, class Log, class Node>
void rawLog(Log &, const Node *, const Node *, std::false_type) {}

/// Used by the tag dispatch of \ref log for the case when there is a log.
template <template <class> class Event, class Log, class Node>
void rawLog(Log &log, const Node *n, const Node *parentSubstitution,
            std::true_type) {
    log.log(n->shareState(),
               std::make_shared<Event<Node>>(log, n, parentSubstitution));
}

/// The function that all algorithms use for logging an event. The function uses
/// tag dispatch on the type of the log to make sure that no run-time overhead
/// is incurred when no log is used (i.e. when the type of the log is \ref
/// Nothing).
/// \tparam Event The particular event class. It should be a class derived
/// from \ref Events::Base<Node>.
/// \tparam Log The type of the log. It should be either an instantiation of \ref AlgorithmLog or \ref Nothing.
/// \tparam Node The node type.
/// \param log The log.
/// \param n The current node in the search.
/// \param parentSubstitution The node whose state should be used as a parent
/// state instead of the parent stored in \n. See How the \ref
/// Events::NothingToDo event is generated in \ref Astar for an example of when
/// this is useful.
template <template <class> class Event, class Log, class Node>
void log(Log &log, const Node *n,
         const Node *parentSubstitution = nullptr) {
    constexpr bool logFlag = !std::is_same<Log, Nothing>::value;
    return rawLog<Event>(log, n, parentSubstitution,
                         std::integral_constant<bool, logFlag>{});
}

#endif
