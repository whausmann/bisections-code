#pragma once
// Shared graph6, graph validation, and bridge construction routines.
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using U64 = std::uint64_t;
struct Graph {
    int n = 0;
    std::array<U64, 64> adj{};
    void add(int u, int v) { adj[u] |= U64{1} << v; adj[v] |= U64{1} << u; }
    void remove(int u, int v) { adj[u] &= ~(U64{1} << v); adj[v] &= ~(U64{1} << u); }
};
inline int take(U64& x) { int v = std::countr_zero(x); x &= x - 1; return v; }
inline int number(const char* s) {
    std::string t(s); std::size_t used = 0;
    int k = std::stoi(t, &used);
    if (used != t.size()) throw std::runtime_error("invalid integer: " + t);
    return k;
}
inline Graph decode(const std::string& s) {
    for (unsigned char c : s)
        if (c < 63 || c > 126) throw std::runtime_error("invalid graph6 character");
    if (s.empty()) throw std::runtime_error("empty graph6 record");
    Graph g; std::size_t p = 1;
    g.n = static_cast<unsigned char>(s[0]) - 63;
    if (g.n == 63) {
        if (s.size() < 4 || s[1] == '~') throw std::runtime_error("unsupported graph6 order");
        g.n = ((s[1] - 63) << 12) | ((s[2] - 63) << 6) | (s[3] - 63);
        if (g.n < 63) throw std::runtime_error("noncanonical graph6 order header");
        p = 4;
    }
    if (g.n < 1 || g.n > 64) throw std::runtime_error("graph order must be 1..64");
    int bits = g.n * (g.n - 1) / 2;
    if (s.size() != p + static_cast<std::size_t>((bits + 5) / 6))
        throw std::runtime_error("incorrect graph6 record length");
    int k = 0;
    for (int j = 1; j < g.n; ++j) for (int i = 0; i < j; ++i, ++k)
        if (((s[p + k / 6] - 63) >> (5 - k % 6)) & 1) g.add(i, j);
    if (bits % 6 && ((s.back() - 63) & ((1 << (6 - bits % 6)) - 1)))
        throw std::runtime_error("nonzero graph6 padding");
    return g;
}
inline std::string encode(const Graph& g) {
    std::string s;
    if (g.n <= 62) s += static_cast<char>(g.n + 63);
    else {
        s += '~'; s += static_cast<char>(63 + ((g.n >> 12) & 63));
        s += static_cast<char>(63 + ((g.n >> 6) & 63)); s += static_cast<char>(63 + (g.n & 63));
    }
    int value = 0, count = 0;
    for (int j = 1; j < g.n; ++j) for (int i = 0; i < j; ++i) {
        value = (value << 1) | static_cast<int>((g.adj[i] >> j) & 1);
        if (++count == 6) { s += static_cast<char>(value + 63); value = count = 0; }
    }
    if (count) s += static_cast<char>((value << (6 - count)) + 63);
    return s;
}
inline bool read_graph(std::istream& in, Graph& g, std::string& s) {
    while (std::getline(in, s)) {
        if (!s.empty() && s.back() == '\r') s.pop_back();
        if (s.rfind(">>graph6<<", 0) == 0) s.erase(0, 10);
        if (s.empty()) continue;
        g = decode(s); return true;
    }
    if (!in.eof()) throw std::runtime_error("input read failed");
    return false;
}
inline bool connected(const Graph& g) {
    U64 seen = 1, todo = 1;
    while (todo) {
        int v = take(todo); U64 fresh = g.adj[v] & ~seen;
        seen |= fresh; todo |= fresh;
    }
    return std::popcount(seen) == g.n;
}
inline void require_cubic(const Graph& g) {
    for (int v = 0; v < g.n; ++v)
        if (std::popcount(g.adj[v]) != 3) throw std::runtime_error("input is not cubic");
    if (!connected(g)) throw std::runtime_error("input is disconnected");
}
inline bool girth_at_least(const Graph& g, int minimum) {
    // Every non-tree edge in each BFS gives a cycle-length upper bound.
    // A shortest cycle is detected by a BFS rooted on that cycle.
    for (int s = 0; s < g.n; ++s) {
        std::array<int, 64> dist, parent, queue;
        dist.fill(-1); parent.fill(-1);
        int head = 0, tail = 0; queue[tail++] = s; dist[s] = 0;
        while (head < tail) {
            int u = queue[head++]; U64 nb = g.adj[u];
            while (nb) {
                int v = take(nb);
                if (dist[v] < 0) { dist[v] = dist[u] + 1; parent[v] = u; queue[tail++] = v; }
                else if (parent[u] != v && parent[v] != u && dist[u] + dist[v] + 1 < minimum)
                    return false;
            }
        }
    }
    return true;
}
inline void finish(std::ofstream& out) {
    out.close();
    if (!out) throw std::runtime_error("output write/close failed");
}

inline int root(const Graph& p) {
    int r = -1;
    for (int v = 0; v < p.n; ++v) {
        int d = std::popcount(p.adj[v]);
        if (d == 2 && r == -1) r = v;
        else if (d != 3) throw std::runtime_error("piece must have one degree-2 root and all other degrees 3");
    }
    if (r == -1 || !connected(p)) throw std::runtime_error("invalid or disconnected piece");
    return r;
}
inline Graph subdivide(const Graph& h, int a, int b) {
    Graph p = h; int r = p.n++;
    p.remove(a, b); p.add(a, r); p.add(b, r);
    return p;
}
inline Graph join(const Graph& a, int ra, const Graph& b, int rb) {
    Graph g = a; g.n = a.n + b.n;
    for (int v = 0; v < b.n; ++v) g.adj[a.n + v] = b.adj[v] << a.n;
    g.add(ra, a.n + rb); return g;
}

// Each callback receives one candidate; duplicates from different bridges may occur.
// Validate pieces once, then join and consume graphs one at a time, on one thread.
template<class Consume>
void for_each_candidate(int n, const std::string& range, int girth,
                        const std::string& filename, Consume consume) {
    if (n < 4 || n > 64 || n % 2 || girth < 4 ||
        (range != "exact" && range != "upto"))
        throw std::runtime_error("require even N in 4..64, GIRTH >=4, and exact|upto");
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("cannot open pieces file");
    struct Piece { Graph g; int r; };
    std::array<std::vector<Piece>, 65> pieces;
    Graph p; std::string s;
    while (read_graph(in, p, s)) {
        int r = root(p);
        if (!girth_at_least(p, girth)) throw std::runtime_error("piece has insufficient girth");
        pieces[p.n].push_back({p, r});
    }
    for (int a = 1; a < n; ++a) for (int b = a; a + b <= n; ++b) {
        if (range == "exact" && a + b != n) continue;
        for (std::size_t i = 0; i < pieces[a].size(); ++i)
            for (std::size_t j = (a == b ? i : 0); j < pieces[b].size(); ++j) {
                const auto& x = pieces[a][i]; const auto& y = pieces[b][j];
                consume(join(x.g, x.r, y.g, y.r));
            }
    }
}
