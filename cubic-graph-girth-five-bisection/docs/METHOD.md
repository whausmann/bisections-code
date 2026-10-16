# Why the search is exhaustive

Scope: connected simple cubic graphs, girth at least five, order at most 36, containing at least one bridge.

Delete any bridge uv. Each resulting component has exactly one degree-two vertex (u or v); every other vertex has degree three. Its order m is odd, since its degree sum is 3m−1. Choose a vertex outside the degree-two vertex and its two neighbours. A radius-two neighbourhood around it contains 1+3+6 distinct vertices: triangles and four-cycles are forbidden. Thus m ≥ 10, and oddness gives m ≥ 11.

Suppress the unique degree-two vertex in each component: remove it and replace its two incident edges by a single edge. This produces a connected simple cubic base. A pre-existing edge between its neighbours would give a triangle in the piece, so no parallel edge is introduced. Suppression shortens a cycle by at most one, so the base is triangle-free. For total order at most 36 the piece orders are at most 25 and the base orders at most 24. Consequently the even base orders 10,12,...,24 suffice.

Snarkhunter is invoked as `snarkhunter-64 k 4 s o g`: all connected simple cubic bases of exact order k and girth at least four, with graph6 output. Neither a snark-only nor a higher-connectivity filter is imposed. See the upstream manual: https://caagt.ugent.be/cubic/README.txt .

The constructor subdivides every edge of every base and keeps precisely pieces whose girth is at least five. Hence it recovers every possible bridge component. The unique degree-two vertex intrinsically identifies the attachment root, so ordinary unrooted isomorphism deduplication of pieces preserves all required rooted types. All unordered pairs of pieces with total order at most 36 are joined, including two copies of the same piece. Every target graph is obtained by reversing the deletion of any of its bridges. Multiple bridges can cause repeated isomorphic candidates; no target is lost.

## Exact bisection search

The checker assigns vertices in a fixed breadth-first order. Vertex 0 is fixed on the left, which removes only the symmetry exchanging the two parts. A partial assignment is rejected if either side exceeds half the order or if an assigned vertex has two same-side assigned neighbours. These are necessary conditions for any completion to an external bisection. The `allowed` routine checks both the newly placed vertex and its same-side neighbour, preserving the invariant for every assigned vertex.

At a complete assignment the sides are equal and each induced subgraph has maximum degree one, equivalent to an external bisection in a cubic graph. The checker counts left-side vertices with no same-side neighbour and accepts if there are at least four. For an external bisection, equality of the cross-edge counts gives 2|L|+|L₃| = 2|R|+|R₃|, so |L₃|=|R₃|. Thus testing the left threshold suffices.

Search stops only on a successful witness; otherwise it explores every feasible assignment. Isomorphic copies have the same property. Deduplicating only failures therefore preserves exhaustive verification. File-input mode validates cubicity, connectedness and existence of a bridge; the girth check is enforced in piece-generation/pairing mode.

The claim depends on correct operation of the generator, compiler, nauty and search code, and on complete, unmodified caches. It is a computational verification, not a separately archived certificate for each successful graph.
