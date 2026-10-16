#include "graph_common.hpp"

static bool bridge_dfs(const Graph& g, int v, int parent, int& timer,
                       std::array<int, 64>& tin, std::array<int, 64>& low) {
    tin[v] = low[v] = timer++; bool found = false;
    U64 nb = g.adj[v];
    while (nb) {
        int w = take(nb); if (w == parent) continue;
        if (tin[w] >= 0) low[v] = std::min(low[v], tin[w]);
        else {
            bool below = bridge_dfs(g, w, v, timer, tin, low);
            found = found || below || low[w] > tin[v];
            low[v] = std::min(low[v], low[w]);
        }
    }
    return found;
}
static bool has_bridge(const Graph& g) {
    std::array<int, 64> tin, low; tin.fill(-1); low.fill(-1); int timer = 0;
    return bridge_dfs(g, 0, -1, timer, tin, low);
}
struct Result { bool external = false, sufficient = false; };
class Search {
    const Graph& g;
    std::array<int, 64> order{};
    Result result;
    // Invariant: every assigned vertex has at most one assigned neighbor
    // on its own side. Only v and its same-side neighbors can change.
    bool allowed(int v, U64 side) const {
        U64 same = g.adj[v] & side;
        if (std::popcount(same) > 1) return false;
        if (same && (g.adj[std::countr_zero(same)] & side)) return false;
        return true;
    }
    bool dfs(int depth, int left_count, U64 left, U64 right) {
        const int half = g.n / 2, right_count = depth - left_count;
        if (left_count > half || right_count > half) return false;
        if (depth == g.n) {
            // The invariant and side capacities now prove external bisection.
            result.external = true;
            int l3 = 0; U64 vertices = left;
            while (vertices) {
                int v = take(vertices);
                if (!(g.adj[v] & left) && ++l3 >= 4) {
                    // For external bisections in cubic graphs, |L3| = |R3|.
                    result.sufficient = true; return true;
                }
            }
            return false;
        }
        const int v = order[depth]; const U64 bit = U64{1} << v;
        if (left_count < half && allowed(v, left) && dfs(depth + 1, left_count + 1, left | bit, right))
            return true;
        if (right_count < half && allowed(v, right) && dfs(depth + 1, left_count, left, right | bit))
            return true;
        return false;
    }
public:
    explicit Search(const Graph& graph) : g(graph) {
        // Fixed BFS order, with ascending labels for neighbors; no adaptive heuristic.
        U64 seen = 1; int head = 0, tail = 1; order[0] = 0;
        while (head < tail) {
            int v = order[head++]; U64 fresh = g.adj[v] & ~seen;
            seen |= fresh;
            while (fresh) order[tail++] = take(fresh);
        }
    }
    Result run() { dfs(1, 1, 1, 0); return result; }
};
int main(int argc, char** argv) {
    try {
        const bool pairing = argc == 8 && std::string(argv[1]) == "--pieces";
        if (!pairing && argc != 4) throw std::runtime_error(
            "usage: bisection_checker graphs.g6 failures.csv failures.g6\n"
            "   or: bisection_checker --pieces N exact|upto GIRTH pieces.g6 failures.csv failures.g6");
        const std::string input = argv[pairing ? 5 : 1];
        const std::string csvname = argv[pairing ? 6 : 2], badname = argv[pairing ? 7 : 3];
        if (input == csvname || input == badname || csvname == badname)
            throw std::runtime_error("input and output paths must differ");
        std::ofstream csv(csvname), bad(badname);
        if (!csv || !bad) throw std::runtime_error("cannot open outputs");
        csv << "n,graph_index,graph6,has_external_bisection,sufficient_bisection,seconds\n"
            << std::fixed << std::setprecision(9);
        std::uint64_t count = 0, sufficient = 0, insufficient = 0, none = 0;
        using Clock = std::chrono::steady_clock;
        const auto start = Clock::now();
        auto check = [&](const Graph& g) {
            // File inputs are validated below. Joined candidates are cubic, connected,
            // and bridged by construction from the validated pieces.
            ++count;
            const auto t = Clock::now(); Result r = Search(g).run();
            const double seconds = std::chrono::duration<double>(Clock::now() - t).count();
            if (r.sufficient) ++sufficient;
            else {
                if (r.external) ++insufficient; else ++none;
                const std::string s = encode(g); // Encode only failures.
                csv << g.n << ',' << count << ',' << s << ',' << int(r.external) << ",0," << seconds << '\n';
                bad << s << '\n';
                if (!csv || !bad) throw std::runtime_error("failure output write failed");
            }
        };
        if (pairing) {
            for_each_candidate(number(argv[2]), argv[3], number(argv[4]), input, check);
        } else {
            std::ifstream in(input);
            if (!in) throw std::runtime_error("cannot open graph input");
            Graph g; std::string s;
            while (read_graph(in, g, s)) {
                require_cubic(g);
                if (!has_bridge(g)) throw std::runtime_error("input graph has no bridge");
                check(g);
            }
        }
        finish(csv); finish(bad);
        std::cout << (pairing ? "candidate constructions checked: " : "input graph records checked: ") << count << '\n'
                  << "sufficient records: " << sufficient << '\n'
                  << "external but insufficient records: " << insufficient << '\n'
                  << "records with no external bisection: " << none << '\n'
                  << "elapsed seconds: " << std::fixed << std::setprecision(6)
                  << std::chrono::duration<double>(Clock::now() - start).count() << '\n';
        if (!std::cout) throw std::runtime_error("summary write failed");
        return 0;
    } catch (const std::exception& e) { std::cerr << "error: " << e.what() << '\n'; return 1; }
}
