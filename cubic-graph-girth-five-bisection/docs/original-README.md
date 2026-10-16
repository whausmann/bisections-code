# Bridge search: Snarkhunter + check before deduplicating failures

## Upgrade from the earlier version

Wait for any old run to finish before replacing files in its folder: the old
runner may still need its binaries and later commands. Extract this ZIP, then
copy ALL the supplied files into your existing bridge-search-mac folder,
replacing the old sources and scripts. In particular, graph_common.hpp is new.
Keep your cache/ and results/ folders; existing completed pieces are compatible.
Do not run the old and new versions concurrently in the same folder.

In Terminal:

```sh
cd "$HOME/Documents/bridge-search-mac"
sh setup-snarkhunter.sh
sh run.sh test
caffeinate -i sh run.sh upto 36
```

The last command starts a NEW run. Run it only when ready to start a search.
The setup is one-time, downloads official Snarkhunter and nauty source, builds
locally without administrator access, and checks a known small count. It does
not replace Homebrew's nauty. Its build directory and logs are printed.
You still need shortg on PATH (`brew install nauty` if necessary), plus Apple's
Command Line Tools. The working SDK 26.5 selection remains automatic if present;
an explicit SDKROOT overrides it. Keep the laptop lid open for long runs.

## Commands

```sh
sh run.sh upto 32
sh run.sh exact 34
sh run.sh upto 20 4
sh run.sh exact 64 5 plan
```

N is even, 4..64. Girth is 4..64, default 5. Default invocation is upto 24 5.
`plan` only prints required base orders. `test` compiles and runs small checks.
64 is a representation limit; generation and memory can become impractical
well below it. Girth 3 is unsupported because suppression can make parallel
edges. Girth 4 means triangle-free, not all cubic graphs.

## What changed

1. Snarkhunter generates exact-order connected simple cubic bases in graph6:
   `snarkhunter-64 N 4 s o g` for final girth >=5; base girth 3 for final girth 4.
   No snark-only, colourability, or higher-connectivity filter is used.
   The constructor tests the full requested final girth, including when it is
   larger than 5. The 64-bit Snarkhunter build supports the required larger bases.
2. Pieces are still deduplicated and cached per girth and base order.
3. The checker loads validated pieces, joins one pair, and immediately tests
   that candidate. Pairing and checking are one sequential, single-threaded
   process. There is no candidate-graph file and no successful-graph output.
4. Only failures are written, then shortg deduplicates those failures.
5. Canonical failure representatives are rechecked to produce a CSV with
   exactly one row per unique failing graph and accurate failure categories.

The exact bisection search itself is unchanged. It stops at the first external
bisection with |L3|>=4 and |R3|>=4, and exhausts the search if none is sufficient.
Repeated isomorphic candidates cannot hide a failure. They mean the main
summary counts candidate CONSTRUCTIONS, not unique graphs. The final failure
list is unique up to isomorphism; there is no unique-success count.

## Timings and progress

The script reports duration and total time after compilation, smoke test,
ALL base/piece preparation combined, combined construction/checking, failure
deduplication, and the final failure report. Preparation prints progress by
base order but no separate timing for each order. Timings are also saved in
timings.txt, using whole-second wall-clock resolution. They include sleep if
the Mac sleeps; caffeinate prevents idle sleep while the script runs.
The C++ checker also prints a finer steady-clock duration. In --pieces mode
this includes reading/validating pieces, constructing and checking candidates,
and failure output, rather than only the recursive search.

## Results

Each run creates and prints a fresh results/ subfolder:

- failures.g6: final UNIQUE failing graphs only.
- failures.csv: one row per final unique failure, with columns
  n,graph_index,graph6,has_external_bisection,sufficient_bisection,seconds.
  graph_index is the index in the canonical unique-failure list; seconds is
  its recheck time. These are not original candidate indices or timings.
- summary.txt: candidate totals and unique-failure totals, separately labeled.
- timings.txt: stage and total wall-clock times.
- candidate-failures.g6 and candidate-failures.csv: original failure records,
  potentially repeated; the CSV preserves candidate indices and first-pass times.
- candidate-summary.txt and failure-summary.txt: separate summaries.
- unique-failures.g6: canonical intermediate copy of the final failure list.
- pieces.g6: the selected pieces; smoke.*: smoke-test data only.
- snarkhunter-baseN.log: generator messages for newly generated base orders.

Empty failures.g6 and header-only failures.csv mean all candidates passed,
provided the run finished successfully. The smoke fixture is deliberately a
failure and is never mixed into the actual search.

No full candidate or full unique-graph output is written. Old results are
preserved. Interrupted runs are incomplete; errors stop the sequential script
before a successful final summary. Completed piece caches alone are reused.
If no new piece sizes are needed, Snarkhunter generation is skipped.

## Cache and completeness

For girth >=5, each side of a bridge has at least 11 vertices, so base orders
10,12,...,N-12 suffice. For girth 4 the safe conservative minimum of five
vertices gives base orders 4,6,...,N-6; invalid pieces are rejected.
After subdividing an edge, the piece's unique degree-2 vertex marks its root.
Pairing includes equal pieces, and removes the obvious symmetry of the two
sides. Additional duplicates from different bridges are harmless during the
check and removed only for failing graphs.
The previous v1-gG-baseN-pieces.g6 cache files remain valid because the class
of bases and the piece construction have not changed. Do not modify caches.
Changes to construction rules require clearing or versioning the cache.

## Manual compilation / interfaces

Keep graph_common.hpp alongside both .cpp files:

```sh
clang++ -std=c++20 -O3 bridge_constructor.cpp -o bin/bridge_constructor
clang++ -std=c++20 -O3 bisection_checker.cpp -o bin/bisection_checker
```

The runner applies the SDK setting automatically. For direct compilation on
this Mac you may first export SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk.

```sh
bin/bridge_constructor pieces GIRTH bases.g6 pieces.g6
bin/bridge_constructor pair N exact GIRTH pieces.g6 candidates.g6
bin/bisection_checker --pieces N exact GIRTH pieces.g6 bad.csv bad.g6
bin/bisection_checker input.g6 bad.csv bad.g6
```

The legacy constructor pair mode is retained for verification/manual use.
The default runner uses --pieces and never writes candidates.g6.
CXX selects an alternative C++ compiler; CC selects the setup's C compiler.
SNARKHUNTER may specify an absolute path to another 64-bit Snarkhunter binary.
No Python, parallel generation, MILP, or other C++ libraries are required.

## Sources and testing

Official Snarkhunter: https://caagt.ugent.be/cubic/
Manual: https://caagt.ugent.be/cubic/README.txt
Nauty: https://users.cecs.anu.edu.au/~bdm/nauty/
The setup fetches Snarkhunter 2.0b and nauty 2.9.3; upstream source retains its
own license. No third-party source or prebuilt executable is bundled here.

Validation was performed on Linux, not a physical Mac. The revised sources
compile with C++20 and warnings enabled. Snarkhunter and geng generated identical
canonical base sets at orders 10,12,14,16 (girth >=4). The new runner reproduces
9 passing candidates through 24 and 127 through 26 at girth >=5. Duplicate
failure records were reduced to a single graph and single CSV row. The exact
recursive search is unchanged; shared graph helpers eliminate duplicated code.
