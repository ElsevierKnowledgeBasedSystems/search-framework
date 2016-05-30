#ifndef EVENTS_H
#define EVENTS_H

/// \file
/// \brief Events generated by the search algorithms.
/// \author Meir Goldenberg

namespace Events {

/// Event that cancels the visual effect of the previous event.
/// \tparam Node The search node type.
/// \note Previous event should be for the same state.
template <class Node = SLB_NODE> struct HideLast : NoChange<Node> {
    /// Using the direct base's constructors.
    using NoChange<Node>::NoChange;

private:
    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Hide Last Event"; }

    /// Returns the event type (see \ref EventType).
    /// \ref The event type.
    EventType eventType() const { return EventType::HIDE_LAST_EVENT; }
};

/// Event that marks the start state.
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct MarkedStart : VertexEmphasis<Node> {
    /// Using the direct base's constructors.
    using VertexEmphasis<Node>::VertexEmphasis;

private:
    /// Returns the color of the mark that will be applied to the vertex.
    /// \return the color of the mark that will be applied to the vertex.
    virtual ::Color color() const override { return ::Color::VIVID_GREEN; }

    /// Returns the size of the mark that will be applied to the vertex.
    /// \return the size of the mark that will be applied to the vertex.
    virtual double size() const override { return 0.5; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Marked Start"; }
};

/// Event that marks the goal state.
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct MarkedGoal : VertexEmphasis<Node> {
    /// Using the direct base's constructors.
    using VertexEmphasis<Node>::VertexEmphasis;

private:
    /// Returns the color of the mark that will be applied to the vertex.
    /// \return the color of the mark that will be applied to the vertex.
    virtual ::Color color() const override { return ::Color::DEEP_BLUE; }

    /// Returns the size of the mark that will be applied to the vertex.
    /// \return the size of the mark that will be applied to the vertex.
    virtual double size() const override { return 0.5; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Marked Goal"; }
};

/// Event that visualizes node generation.
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct Generated : VertexEdgeColor<Node> {
    /// Using the direct base's constructors.
    using VertexEdgeColor<Node>::VertexEdgeColor;

private:
    /// Returns the new color of both the vertex of the state graph
    /// corresponding to the state associated with the event and the edge by
    /// which the search algorithm arrived to this state.
    /// \return The new color of both the vertex of the state graph
    /// corresponding to the state associated with the event and the edge by
    /// which the search algorithm arrived to this state.
    virtual ::Color color() const override { return ::Color::SUNSHINE_YELLOW; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Generated"; }
};

/// Event that visualizes change of parent node.
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct NotParent : VertexEdgeColor<Node> {
    /// Using the direct base's constructors.
    using VertexEdgeColor<Node>::VertexEdgeColor;

private:
    /// Returns the new color of both the vertex of the state graph
    /// corresponding to the state associated with the event and the edge by
    /// which the search algorithm arrived to this state.
    /// \return The new color of both the vertex of the state graph
    /// corresponding to the state associated with the event and the edge by
    /// which the search algorithm arrived to this state.
    virtual ::Color color() const override { return EdgeStyle::defaultColor; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Not Parent"; }
};

/// Event that visualizes the fact that a node was generated, but nothing needed
/// to be done for it.
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct NothingToDo : VertexEdgeColor<Node> {
    /// Using the direct base's constructors.
    using VertexEdgeColor<Node>::VertexEdgeColor;

private:
    /// Returns the new color of both the vertex of the state graph
    /// corresponding to the state associated with the event and the edge by
    /// which the search algorithm arrived to this state.
    /// \return The new color of both the vertex of the state graph
    /// corresponding to the state associated with the event and the edge by
    /// which the search algorithm arrived to this state.
    virtual ::Color color() const override { return ::Color::WHITE; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Nothing To Do"; }
};

/// Event that visualizes a node entering the Open List.
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct EnteredOpen : VertexEdgeColor<Node> {
    /// Using the direct base's constructors.
    using VertexEdgeColor<Node>::VertexEdgeColor;

private:
    /// Returns the new color of both the vertex of the state graph
    /// corresponding to the state associated with the event and the edge by
    /// which the search algorithm arrived to this state.
    /// \return The new color of both the vertex of the state graph
    /// corresponding to the state associated with the event and the edge by
    /// which the search algorithm arrived to this state.
    virtual ::Color color() const override { return ::Color::PALE_YELLOW; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Entered OPEN"; }
};

/// Event that visualizes having selected a node.
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct Selected : VertexColor<Node> {
    /// Using the direct base's constructors.
    using VertexColor<Node>::VertexColor;

private:
    /// Returns the new color of the vertex corresponding to the state
    /// associated with the event.
    /// \return The new color of the vertex corresponding to the state
    /// associated with the event.
    virtual ::Color color() const override { return Color::VIVID_PURPLE; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Selected"; }
};

/// Event that visualizes a node's expansion being suspended. The expansion may
/// be refused or resumed pending a check specific to the particular search
/// algorithm.
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct SuspendedExpansion : VertexColor<Node> {
    /// Using the direct base's constructors.
    using VertexColor<Node>::VertexColor;

private:
    /// Returns the new color of the vertex corresponding to the state
    /// associated with the event.
    /// \return The new color of the vertex corresponding to the state
    /// associated with the event.
    virtual ::Color color() const override { return ::Color::ORANGE; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Suspended Expansion"; }
};

/// Event that visualizes a node's expansion being resumed following suspension
/// (see \ref SuspendedExpansion).
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct ResumedExpansion : VertexColor<Node> {
    /// Using the direct base's constructors.
    using VertexColor<Node>::VertexColor;

private:
    /// Returns the new color of the vertex corresponding to the state
    /// associated with the event.
    /// \return The new color of the vertex corresponding to the state
    /// associated with the event.
    virtual ::Color color() const override { return ::Color::RICH_PURPLE; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Resumed Expansion"; }
};

/// Event that visualizes a node's expansion being denied following suspension
/// (see \ref SuspendedExpansion).
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct DeniedExpansion : VertexColor<Node> {
    /// Using the direct base's constructors.
    using VertexColor<Node>::VertexColor;

private:
    /// Returns the new color of the vertex corresponding to the state
    /// associated with the event.
    /// \return The new color of the vertex corresponding to the state
    /// associated with the event.
    virtual ::Color color() const override { return ::Color::YELLOWISH_TAN; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Denied Expansion"; }
};

/// Event that visualizes having closed a node.
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct Closed : VertexEdgeColor<Node> {
    /// Using the direct base's constructors.
    using VertexEdgeColor<Node>::VertexEdgeColor;

private:
    /// Returns the new color of both the vertex of the state graph
    /// corresponding to the state associated with the event and the edge by
    /// which the search algorithm arrived to this state.
    /// \return The new color of both the vertex of the state graph
    /// corresponding to the state associated with the event and the edge by
    /// which the search algorithm arrived to this state.
    virtual ::Color color() const override { return Color::WARM_BROWN; }

    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Closed"; }
};

/// Event that visualizes having found a path with required qualities to a goal.
/// \tparam Node The search node type.
template <class Node = SLB_NODE> struct SolvedGoal : SolutionPathChange<Node> {
    /// Shared pointer to constant state.
    using StateSharedPtr = typename UniformChange<Node>::StateSharedPtr;

    /// The type representing an edge in the state graph.
    using Edge = typename UniformChange<Node>::Edge;

    /// Using the \c state_ variable of the base.
    using Base<Node>::state_;

    /// Using the direct base's constructors.
    using SolutionPathChange<Node>::SolutionPathChange;

protected:
    /// Modifies the visual representation style of the given participating
    /// edge of the state graph.
    virtual void change(EdgeStyle &style, const Edge &) const override {
        style.color = Color::VIVID_GREEN;
        style.arrow = true;
    };

private:
    /// Returns the string describing the event. This string is displayed in the
    /// log window.
    /// \return The string describing the event.
    std::string eventStr() const override { return "Solved Goal"; }
};

}
#endif