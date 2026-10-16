#!/bin/sh
# Run with sh, not source. A failure exits this script, not your Terminal.
set -eu
run_started=$(date +%s)
export LC_ALL=C
cd "$(dirname "$0")"
root=$(pwd)
mode=${1:-upto}
n=${2:-24}
girth=${3:-5}
action=${4:-run}
usage() { echo 'Usage: sh run.sh [exact|upto] [even N: 4..64] [girth: 4..64] [run|plan]' >&2; echo '       sh run.sh test' >&2; exit 1; }
[ "$#" -le 4 ] || usage
case "$mode" in exact|upto|test) ;; *) usage;; esac
case "$n" in ''|*[!0-9]*|0*) usage;; esac
case "$girth" in ''|*[!0-9]*|0*) usage;; esac
[ "${#n}" -le 2 ] && [ "${#girth}" -le 2 ] || usage
[ "$n" -ge 4 ] && [ "$n" -le 64 ] && [ "$((n % 2))" -eq 0 ] || usage
[ "$girth" -ge 4 ] && [ "$girth" -le 64 ] || usage
case "$action" in run|plan) ;; *) usage;; esac
# Safe lower bounds on each odd-order piece. For girth >=5, the radius-two
# argument gives >=11. For girth 4, >=5 suffices as a conservative bound.
minpiece=5
if [ "$girth" -ge 5 ]; then minpiece=11; fi
minbase=$((minpiece - 1))
maxbase=$((n - minpiece - 1))
printf 'Mode: %s; N: %s; girth >= %s; threshold: 4 per side\n' "$mode" "$n" "$girth"
printf 'Base orders:'
k=$minbase
while [ "$k" -le "$maxbase" ]; do printf ' %s' "$k"; k=$((k + 2)); done
if [ "$maxbase" -lt "$minbase" ]; then printf ' none (no possible bridged graphs)'; fi
printf '\n'
if [ "$action" = plan ]; then
    echo 'Plan only: no compilation or graph generation. 64 is a representation limit, not a feasible exhaustive-search guarantee.'
    exit 0
fi
cxx=${CXX:-clang++}
# Preserve explicit SDKROOT. Otherwise use the SDK known to work on Will's Mac
# if present. This affects only this process and its children.
if [ "$(uname -s)" = Darwin ] && [ -z "${SDKROOT:-}" ] &&
   [ -d /Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk ]; then
    SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk
    export SDKROOT
fi
snark=${SNARKHUNTER:-$root/bin/snarkhunter-64}
# Resolve before changing directory; an explicit path should be absolute.
if ! command -v "$snark" >/dev/null 2>&1; then
    echo 'Snarkhunter missing. Run: sh setup-snarkhunter.sh' >&2
    exit 1
fi
snark=$(command -v "$snark")
for tool in "$cxx" shortg; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Missing command: $tool. See README.md." >&2; exit 1
    fi
done
printf 'Compiler: %s; SDK: %s\n' "$cxx" "${SDKROOT:-compiler default}"
mkdir -p bin cache results
out=$(mktemp -d "$root/results/${mode}-${n}-g${girth}.XXXXXX")
printf 'Output folder: %s\n' "$out"
timing_log="$out/timings.txt"
printf 'Wall-clock timings, whole-second resolution (0 s means less than about 1 s).\n' > "$timing_log"
stage_start() {
    stage_label=$1
    stage_started=$(date +%s)
    printf '%s...\n' "$stage_label"
}
stage_done() {
    stage_finished=$(date +%s)
    printf '[time] %s: %s s | total so far: %s s\n' "$stage_label" \
        "$((stage_finished - stage_started))" "$((stage_finished - run_started))" | tee -a "$timing_log"
}
report_total() {
    run_finished=$(date +%s)
    printf '[time] TOTAL wall-clock runtime: %s s\n' "$((run_finished - run_started))" | tee -a "$timing_log"
}
stage_start 'Compile bridge constructor'
"$cxx" -std=c++20 -O3 -Wall -Wextra -Wpedantic bridge_constructor.cpp -o bin/bridge_constructor
stage_done
stage_start 'Compile bisection checker'
"$cxx" -std=c++20 -O3 -Wall -Wextra -Wpedantic bisection_checker.cpp -o bin/bisection_checker
stage_done
cd "$out"
stage_start 'Small-case smoke test'
# Check nauty using K4, and the checker using the bundled bridged fixture.
"$snark" 4 3 s o g > smoke.g6 2> smoke.snarkhunter.log
test "$(wc -l < smoke.g6)" -eq 1
cat smoke.g6 smoke.g6 > smoke.twice.g6
shortg -q smoke.twice.g6 smoke.unique.g6
test "$(wc -l < smoke.unique.g6)" -eq 1
"$root/bin/bisection_checker" "$root/smoke-bridged.g6" smoke.csv smoke.failures.g6 > smoke.summary.txt
test "$(wc -l < smoke.failures.g6)" -eq 1
awk -F, 'NR==2 {ok=($1==10 && $4==1 && $5==0)} END {exit !(NR==2 && ok)}' smoke.csv
echo 'Small-case smoke test passed.'
stage_done
if [ "$mode" = test ]; then report_total; exit 0; fi
: > pieces.g6
stage_start 'Prepare all bases and deduplicated pieces (all required orders)'
k=$minbase
while [ "$k" -le "$maxbase" ]; do
    cached="$root/cache/v1-g${girth}-base${k}-pieces.g6"
    if [ -f "$cached" ]; then
        printf 'Reusing pieces from base order %s\n' "$k"
    else
        printf 'Snarkhunter: base order %s; then construct and deduplicate pieces\n' "$k"
        base_girth=3
        if [ "$girth" -ge 5 ]; then base_girth=4; fi
        # s=exact order; o=stdout; g=graph6. No colourability/connectivity filter.
        "$snark" "$k" "$base_girth" s o g > base.work.g6 2> "snarkhunter-base${k}.log"
        "$root/bin/bridge_constructor" pieces "$girth" base.work.g6 pieces.raw.work.g6
        shortg -q pieces.raw.work.g6 pieces.unique.work.g6
        mv pieces.unique.work.g6 "$cached"
        rm base.work.g6 pieces.raw.work.g6
    fi
    cat "$cached" >> pieces.g6
    k=$((k + 2))
done
stage_done
stage_start 'Construct and check candidates (one at a time)'
"$root/bin/bisection_checker" --pieces "$n" "$mode" "$girth" pieces.g6 \
    candidate-failures.csv candidate-failures.g6 > candidate-summary.txt
stage_done
cat candidate-summary.txt
stage_start 'Deduplicate only failing graphs'
shortg -q candidate-failures.g6 unique-failures.g6
stage_done
# Recheck canonical representatives to produce a matching unique CSV and exact
# unique-failure category counts. This processes only the failures, not successes.
stage_start 'Write unique failure report'
"$root/bin/bisection_checker" unique-failures.g6 failures.csv failures.g6 > failure-summary.txt
cmp unique-failures.g6 failures.g6
awk -F, 'NR>1 {n++; if ($4==1) e++; else z++}
    END {printf "unique failing graphs: %d\nexternal but insufficient: %d\nno external bisection: %d\n", n,e,z}' \
    failures.csv > unique-failure-summary.txt
{
    echo 'Candidate counts may include isomorphic copies:'
    cat candidate-summary.txt
    echo
    echo 'Unique failing graphs (after isomorphism removal):'
    cat unique-failure-summary.txt
} > summary.txt
stage_done
cat unique-failure-summary.txt
report_total
printf '\nFinished. Results: %s\n' "$out"
