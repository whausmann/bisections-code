#include "graph_common.hpp"

int main(int argc, char** argv) {
    try {
        if (argc < 2) throw std::runtime_error(
            "usage: bridge_constructor pieces GIRTH bases.g6 pieces.g6\n"
            "   or: bridge_constructor pair N exact|upto GIRTH pieces.g6 candidates.g6");
        const std::string mode = argv[1];
        if (mode == "pieces" && argc == 5) {
            const int girth = number(argv[2]);
            if (girth < 4) throw std::runtime_error("GIRTH must be >= 4");
            if (std::string(argv[3]) == argv[4]) throw std::runtime_error("input and output must differ");
            std::ifstream in(argv[3]); if (!in) throw std::runtime_error("cannot open bases file");
            std::ofstream out(argv[4]); if (!out) throw std::runtime_error("cannot open pieces output");
            Graph h; std::string s;
            while (read_graph(in, h, s)) {
                require_cubic(h);
                if (h.n > 62) throw std::runtime_error("base order must be <= 62");
                if (!girth_at_least(h, girth - 1)) continue;
                for (int a = 0; a < h.n; ++a) {
                    U64 nb = h.adj[a];
                    while (nb) {
                        int b = take(nb); if (b <= a) continue;
                        Graph p = subdivide(h, a, b);
                        if (girth_at_least(p, girth)) out << encode(p) << '\n';
                    }
                }
            }
            finish(out);
        } else if (mode == "pair" && argc == 7) {
            const int n = number(argv[2]), girth = number(argv[4]);
            const std::string range = argv[3];
            if (n < 4 || n > 64 || n % 2 || girth < 4 || (range != "exact" && range != "upto"))
                throw std::runtime_error("require even N in 4..64, GIRTH >= 4, and exact|upto");
            if (std::string(argv[5]) == argv[6]) throw std::runtime_error("input and output must differ");
            std::ofstream out(argv[6]);
            if (!out) throw std::runtime_error("cannot open candidates output");
            for_each_candidate(n, range, girth, argv[5], [&](const Graph& g) {
                out << encode(g) << '\n';
            });
            finish(out);
        } else {
            throw std::runtime_error("invalid arguments (run without arguments for usage)");
        }
        return 0;
    } catch (const std::exception& e) { std::cerr << "error: " << e.what() << '\n'; return 1; }
}
