#ifndef VISUALIZER_DATA_H
#define VISUALIZER_DATA_H

/// \file
/// \brief The \ref VisualizerData class.
/// \author Meir Goldenberg

#include "log_window.h"

/// The base of \ref ui::Visualizer holding the visualizer's constituent components.
template <class Node>
struct VisualizerData {
    /// The type of the log of events generated by the search algorithm.
    using MyAlgorithmLog = AlgorithmLog<Node>;

    /// The type of the log of visual events.
    using MyVisualLog = VisualLog<Node>;

    /// The type of the (partial) domain graph.
    using Graph = StateGraph<typename Node::State>;

    /// The type of the drawer.
    using DrawerType = Drawer<Node>;

    /// Whether the visualizer is in paused or motion state.
    enum class VISUALIZER_STATE{PAUSE, GO};

    /// Initializes the visualizer data with the given (partial) domain graph
    /// and log of events generated by the algorithm.
    VisualizerData(Graph &g, const MyAlgorithmLog &log)
        : g_(g), log_(log, g), drawer_(g, log_), logWindow_(log_),
          filter_(log.eventStrings()), searchFilter_(log.eventStrings()) {
        logWindow_.fillEventsPad();
    }

    /// Returns the log of visual events.
    /// \return Reference to the log of visual events.
    MyVisualLog &log() { return log_; }

    /// Returns the drawer object.
    /// \return Reference to the drawer object.
    DrawerType &drawer() { return drawer_; }

    /// Returns the log window object.
    /// \return Reference to the log window object.
    LogWindow<Node> &logWindow() { return logWindow_; }

    /// Returns the current state of the visualizer.
    /// \return The current state of the visualizer.
    void state(VISUALIZER_STATE s) { s_ = s; }

    /// Sets the visualization speed to the given value.
    /// \param s The visualization speed (number of events per second).
    void speed(int s) { speed_ = s; }

    /// Returns the current filter used in the log window.
    /// \return The current filter used in the log window.
    Filter<Node> &filter() { return filter_; }

    /// Returns the current filter used to search for events.
    /// \return The current filter used to search for events.
    Filter<Node> &searchFilter() { return searchFilter_; }

protected:
    Graph &g_;                  ///< The (partial) domain graph.
    MyVisualLog log_;           ///< The log of visual events.
    DrawerType drawer_;         ///< The drawer object.
    LogWindow<Node> logWindow_; ///< The log window object.

    /// The current state of the visualizer.
    VISUALIZER_STATE s_ = VISUALIZER_STATE::PAUSE;

    /// The current visualization speed (number of events per second).
    int speed_ = 2;

    Filter<Node> filter_; ///< The current filter used in the log window.

    /// The current filter used to search for events.
    Filter<Node> searchFilter_;
};

#endif
