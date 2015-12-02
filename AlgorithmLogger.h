#ifndef ALGORITHM_LOGGER
#define ALGORITHM_LOGGER

#include <unordered_map>

template <class AlgorithmEvent_>
struct NoAlgorithmLogger {
    using AlgorithmEvent = AlgorithmEvent_;
    using StateSharedPtr = typename AlgorithmEvent::StateSharedPtr;

    void log(const StateSharedPtr &s, const AlgorithmEvent &e) {
        (void)s; (void)e;
    }
    void dump() { std::cout << "NoLogger!" << std::endl; }
};

template <class AlgorithmEvent_>
struct AlgorithmLogger {
    using AlgorithmEvent = AlgorithmEvent_;
    using StateSharedPtr = typename AlgorithmEvent::StateSharedPtr;

    void log(AlgorithmEvent e) {
        const StateSharedPtr &s = e.state();
        stateToLastEventStep_[s] = events_.size();
        auto it = stateToLastEventStep_.find(s);

        e.setLastEventStep(-1);
        if (it != stateToLastEventStep_.end())
            e.setLastEventStep(it->second);
        events_.push_back(e);
    }
    void dump() {
        std::cout << "Events in order:" << std::endl;
        int i = 0;
        for (auto el: events_)
            std::cout << "Event " << i++ << ". " << el << std::endl;
        std::cout << "\nLast events:" << std::endl;
        for (auto el: stateToLastEventStep_)
            std::cout << *(el.first) << ": " << el.second << std::endl;
    }
private:
    // -1 means no previous event
    std::unordered_map<StateSharedPtr, int> stateToLastEventStep_;

    std::vector<AlgorithmEvent> events_;

    const AlgorithmEvent &getLastEvent(const StateSharedPtr &s) {
        return events_[stateToLastEventStep_[s]];
    }
};

#endif
