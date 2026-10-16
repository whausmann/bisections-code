# Cubic graph girth-five bisection

Exhaustive computational verification of the following statement:

> Every connected simple cubic graph of girth at least five with a bridge and at most 36 vertices admits an external bisection (L,R) with |L₃| = |R₃| ≥ 4.

An external bisection has equal parts and every vertex has at least two neighbours across the cut. L₃ and R₃ consist of vertices with all three neighbours across the cut.

Author: Will Hausmann. Developed with assistance from ChatGPT. The author is responsible for review and the reported computational claim.

## Recorded result

The supplied completed run through 36 vertices reports **149,585,205 candidate constructions, all sufficient, with zero failures**. Candidates may include isomorphic copies. This is not a count of nonisomorphic graphs. Total recorded wall time is 1,965 seconds (32 min 45 s), with previously computed smaller pieces reused. It is not a cold-cache benchmark.

See [the run record](results/upto-36-g5.zEdNt5/summary.txt), [timings](results/upto-36-g5.zEdNt5/timings.txt), and [provenance](docs/PROVENANCE.md). The core C++ files and both shell scripts are byte-for-byte copies of the submitted run folder. Large regenerable pieces and platform-specific binaries are omitted.

## Build and reproduce

Requirements: a C++20 compiler, a C compiler, make, curl, tar, unzip, and nauty's `shortg` on PATH. On macOS install Apple's Command Line Tools and nauty (for example, `brew install nauty`). On Linux install these dependencies using your distribution; use `CXX=g++` if clang++ is unavailable.

```sh
sh setup-snarkhunter.sh
sh run.sh test
sh run.sh upto 26
sh run.sh upto 36
```

On macOS prevent idle sleep during a long run with:

```sh
caffeinate -i sh run.sh upto 36
```

Setup downloads Snarkhunter 2.0b and nauty 2.9.3 and builds locally. Network access is needed for setup. `shortg` is a separate installed dependency. For a clean reproduction use a fresh extraction with no cache. Do not run two searches in the same directory simultaneously. Runtime and RAM/disk usage grow rapidly; 64 is only a representation limit.

The threshold is fixed at four vertices per side. `sh run.sh exact 34` checks one order; `sh run.sh exact 64 5 plan` only prints the plan. New output is placed in `results/`; pieces are cached in `cache/`. Successful graphs are checked immediately and discarded; only failures are deduplicated.

## Verification and documentation

- [Completeness and checker argument](docs/METHOD.md)
- [Recorded results and limitations](docs/PROVENANCE.md)
- [Publication instructions](docs/PUBLISHING.md)
- [Paper text](docs/paper-snippet.tex) and [BibTeX template](docs/citation.bib)
- [Original usage documentation](docs/original-README.md)

Run the independent small-case checker test (Python 3, no third-party Python packages):

```sh
python3 tests/test_checker.py
```

This test supplements, rather than reruns, the recorded 36-vertex computation. See `docs/VALIDATION.txt` for this package's test output.

## Citation and licensing

Use the eventual tagged release/DOI when citing; `CITATION.cff` supplies author/title metadata. The intended repository is https://github.com/whausmann/bisections-code/tree/main/cubic-graph-girth-five-bisection; release identifiers remain to be filled in after publication. The project's own code is MIT licensed. Dependencies retain their own licenses; no third-party source or executable is redistributed. See [THIRD_PARTY.md](THIRD_PARTY.md).
