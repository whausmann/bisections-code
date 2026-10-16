# Supplied run evidence

Source: user-uploaded `bridge-search-mac 2(1).zip`, identified by the author as the code and output used for the completed 36-vertex run. All small result files are retained unchanged, including deliberately failing smoke-test fixtures. `smoke.failures.g6` is not a counterexample to the observation.

| Run bound | Candidate constructions | Sufficient | Failures |
|---:|---:|---:|---:|
| 32 | 444,609 | 444,609 | 0 |
| 34 | 7,899,584 | 7,899,584 | 0 |
| 36 | 149,585,205 | 149,585,205 | 0 |

These are cumulative counts, not nonisomorphic graph counts. All three runs have a final summary and total timing line. For the 36 run, preparation took 1,331 s and construction/checking 633 s; total wall time was 1,965 s. The base-24 log reports 23,780,814 generated bases and 41.1 s generator CPU time. The preparation stage includes considerably more than base generation.

The 32 run supplies base logs for orders 10–20, the 34 run adds order 22, and the 36 run adds order 24. This is consistent with incremental cache reuse. No cache manifest or historical source hashes were recorded by the original runner. The SHA256SUMS file authenticates the packaged files against one another; it does not retrospectively prove which executable produced historical output.

Excluded: precompiled binaries, generated piece files, operating-system metadata. No successful-graph witnesses were saved by this version. Missing large pieces are regenerable from the code. The full 36 run was not repeated during packaging.

Author-confirmed run metadata: October 9, 2026; 2026 MacBook Air; 16 GB RAM. Chip, macOS version, compiler version, shortg version and run timezone were not supplied. The author-confirmed date takes precedence over archive timestamps. Intended repository: https://github.com/whausmann/bisections-code/tree/main/cubic-graph-girth-five-bisection . Repository publication has not been verified. The setup script names Snarkhunter 2.0b and nauty 2.9.3; exact historical build identity is not independently established by the logs.
