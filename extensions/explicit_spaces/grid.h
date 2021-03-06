
#ifndef SLB_EXTENSIONS_EXPLICIT_SPACES_GRID_H
#define SLB_EXTENSIONS_EXPLICIT_SPACES_GRID_H

/// \file
/// \brief The \ref slb::ext::explicit_space::Grid class.
/// \author Meir Goldenberg

#ifndef SLB_COST_DIAGONAL
/// Default value for the diagonal move cost.
#define SLB_COST_DIAGONAL 1
#endif

namespace slb {
namespace ext {
namespace explicit_space {

using core::util::sureRead;
using core::util::split;

/// The type for 4 or 8-connected grid explicit domain.
/// \tparam CostType_ Type for action cost in the search domain.
template <typename CostType_>
struct Grid {
    /// Type for action cost in the search domain.
    using CostType = CostType_;

    /// The type used internally by this explicit space to represent a state.
    /// To distinguish this state type from the state type used by search
    /// algorithms, we call this state type \c location.
    using Location = int;

    /// The type of iterator over passable locations.
    using LocationIterator = core::util::IndexIterator<
        std::vector<bool>,
        core::util::VectorSkipConstIterator<std::vector<bool>>>;

    /// The state iterator type.
    template <class ExplicitState>
    struct StateIterator : LocationIterator {
        using LocationIterator::LocationIterator;
        /// The dereference operator.
        /// \return The state corresponding to the iterator.
        ExplicitState operator*() const {
            return ExplicitState{LocationIterator::operator*()};
        }
    };

    /// Initializes the grid based on the contents of the given file.
    /// \param fileName The name of the file that stores the grid.
    Grid(std::string fileName) {
        std::ifstream fin(fileName);
        if (!fin)
            throw std::invalid_argument("Could not find " + fileName);

        sureRead(fin, "type");
        octileFlag_ = (sureRead(fin) == "octile");
        sureRead(fin, "height");
        height_ = std::stoi(sureRead(fin));
        sureRead(fin, "width");
        width_ = std::stoi(sureRead(fin));
        sureRead(fin, "map");

        for (int i = 0; i < height_; i++) {
            std::string str = sureRead(fin);
            assert((int)str.size() == width_);
            for (char c: str)
                v_.push_back(c == '.');
        }
    }

    /// Returns location based on row and column in the grid.
    /// \param r The row.
    /// \param c The column.
    /// \return The location corresponding to the position \c (r,c) in the grid.
    Location location(int r, int c) const { return r * width_ + c; }

    /// Returns row based on location.
    /// \param loc The location.
    /// \ return The row corresponding to \c loc.
    int row(Location loc) const { return loc / width_; }

    /// Returns column based on location.
    /// \param loc The location.
    /// \return The column corresponding to \c loc.
    int column(Location loc) const { return loc % width_; }

    /// Returns \c true if the location \c (r,c) is passable and \c false
    /// otherwise.
    /// \return \c true if the location \c (r,c) is passable and \c false
    /// otherwise.
    bool passable(int r, int c) const {
        if (r < 0 || r >= height_) return false;
        if (c < 0 || c >= width_) return false;
        return v_[location(r, c)];
    }

    /// Returns \c true if the location \c loc is passable and \c false
    /// otherwise.
    /// \return \c true if the location \c loc is passable and \c false
    /// otherwise.
    bool passable(Location loc) const {
        return passable(row(loc), column(loc));
    }

    /// Computes the set of neighbors of the given location.
    /// \tparam Neighbor The type for a single neighbor of a given state.
    /// \param loc The location whose set of neighbors is to be computed.
    /// \return The set of neighbors of the given location.
    /// \note The \c Neighbor type holds the states as used by the search
    /// algorithms, not locations. This is done to avoid extra copying.
    template<class Neighbor>
    std::vector<Neighbor> neighbors(int loc) const {
        std::vector<Neighbor> res;
        int r = row(loc), c = column(loc);
        assert(passable(r, c));

        bool c1, c2;

        if ((c1 = passable(r - 1, c)))
            addNeighbor(res, location(r - 1, c));
            //res.push_back(Neighbor(location(r - 1, c), 1));

        // Going clock-wise with diagonal directions starting with r-1, c-1
        if ((c2 = passable(r, c - 1))) addNeighbor(res, location(r, c - 1));
        if (octileFlag_ && c1 && c2 && passable(r - 1, c - 1))
            addNeighbor(res, location(r - 1, c - 1), true);

        if ((c2 = passable(r, c + 1))) addNeighbor(res, location(r, c + 1));
        if (octileFlag_ && c1 && c2 && passable(r - 1, c + 1))
            addNeighbor(res, location(r - 1, c + 1), true);

        if ((c1 = passable(r + 1, c))) addNeighbor(res, location(r + 1, c));
        if (octileFlag_ && c1 && c2 && passable(r + 1, c + 1))
            addNeighbor(res, location(r + 1, c + 1), true);

        c2 = passable(r, c - 1);
        if (octileFlag_ && c1 && c2 && passable(r + 1, c - 1))
            addNeighbor(res, location(r + 1, c - 1), true);

        return res;
    }

    /// Returns a location based on the given string, e.g. "[105, 400]".
    /// \param str The string.
    /// \return The location based on the given string.
    Location location(const std::string str) const {
        auto parse = split(str, {' ', ',', '[', ']'});
        assert(parse.size() >= 2);
        return location(std::stoi(parse[1]), std::stoi(parse[0]));
    }

    /// Returns a random location.
    /// \return A random location.
    Location random() const {
        static std::vector<Location> locs;
        if (!locs.size()) {
            Location cur = 0;
            for (auto b : v_) {
                if (b) locs.push_back(cur);
                ++cur;
            }
        }
        return locs[rand() % locs.size()];
    }

    /// Returns the default location.
    /// \return The default location.
    /// \note Right now this function simply calls \ref random.
    Location defaultLocation() const {
        return random();
    }

    /// Fills out the coordinates for the vertex representing the state.
    /// \param loc The state.
    /// \param x The x-coordinate to be filled out.
    /// \param y The y-coordinate to be filled out.
    void visualLocation(Location loc, double &x, double &y) const {
        x = column(loc);
        y = row(loc);
    }

    /// Computes the Manhattan distance heuristic.
    /// \param location The location from which the heuristic is required.
    /// \param goal The location to which the heuristic is required.
    /// \return The Manhattan distance heuristic from \c location to \c goal.
    CostType manhattan(Location location, Location goal) const {
        return std::abs(row(location) - row(goal)) +
               std::abs(column(location) - column(goal));
    }

    /// Computes the octile distance heuristic.
    /// \param location The location from which the heuristic is required.
    /// \param goal The location to which the heuristic is required.
    /// \return The octile distance heuristic from \c location to \c goal.
    CostType octile(Location location, Location goal) const {
        double dr = std::abs(row(location) - row(goal));
        double dc = std::abs(column(location) - column(goal));
        return std::min(dr, dc) * SLB_COST_DIAGONAL +
               (std::max(dr, dc) - std::min(dr, dc));
    }

    /// Returns the string representation of the given location.
    /// \param location The location whose string representation is needed.
    /// \return The string representation of \c location.
    std::string locationShortStr(Location location) const {
        return "[" + str(column(location)) + " " + str(row(location)) + "]";
    }

    /// Returns the string representation of the given location.
    /// \param location The location whose string representation is needed.
    /// \return The string representation of \c location.
    std::string locationStr(Location location) const {
        return locationShortStr(location) + " (" + str(location) + ")";
    }

    /// Return the size of \ref v_.
    /// \return The size of \ref v_.
    int rawSize() const { return v_.size(); }

private:
    /// The encoding of the map. \c true stands for passable, \c false stands
    /// for impassable.
    std::vector<bool> v_;

    int width_;  ///< Map's width (i.e. number of columns).
    int height_; ///< Map's height (i.e. number of rows).

    /// If \c true, the map is 8-connected; otherwise, the map is 4-connected.
    bool octileFlag_;

    /// Adds a neighbor corresponding to the given location to the given set of
    /// neighbors.
    /// \tparam Neighbor The type for a single neighbor of a given state.
    /// \param res The set of neighbors to be updated.
    /// \param n The location whose corresponding state is to be added.
    /// \param diagonalFlag \c true indicates that a diagonal move led to \c n.
    /// \c false indicates that a straight move led to \c n.
    /// \note The \c Neighbor type holds the states as used by the search
    /// algorithms, not locations. This is done to avoid extra copying.
    template <class Neighbor>
    void addNeighbor(std::vector<Neighbor> &res, Location n,
                     bool diagonalFlag = false) const {
        CostType cost = diagonalFlag ? static_cast<CostType>(SLB_COST_DIAGONAL)
            : static_cast<CostType>(1.0);
        res.push_back(Neighbor((typename Neighbor::State{n}), cost));
    }

    /// Returns the iterator over the locations of the grid set to the first
    /// location.
    /// \return The iterator over the locations of the grid set to the first
    /// location.
    LocationIterator locationIterator() {
        return LocationIterator(v_, v_.begin(), 0);
    }

    /// Returns the iterator over the states of the given domain corresponding
    /// to the locations of the grid set to the state corresponding to the first location.
    /// \tparam ExplicitState The type representing the given explicit domain.
    /// \return The iterator over the states of the given domain corresponding
    /// to the locations of the grid set to the state corresponding to the first location.
    template <class ExplicitState>
    StateIterator<ExplicitState> beginStateIterator() {
        return StateIterator<ExplicitState>(v_, v_.begin(), 0);
    }

    /// Returns the iterator over the states of the given domain corresponding
    /// to the locations of the grid set past the state corresponding to the
    /// last location.
    /// \tparam ExplicitState The type representing the given explicit domain.
    /// \return The iterator over the states of the given domain corresponding
    /// to the locations of the grid set past the state corresponding to the
    /// last location.
    template <class ExplicitState>
    StateIterator<ExplicitState> endStateIterator() {
        return StateIterator<ExplicitState>(v_, v_.end(), 0);
    }
};

} // namespace
} // namespace
} // namespace

#endif
