#!/bin/sh
# One-time local build: no administrator privileges, no system installation.
set -eu
cd "$(dirname "$0")"
project_dir=$(pwd)
cc=${CC:-cc}
if [ "$(uname -s)" = Darwin ] && [ -z "${SDKROOT:-}" ] &&
   [ -d /Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk ]; then
    SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk
    export SDKROOT
fi
for tool in "$cc" make curl tar unzip; do
    command -v "$tool" >/dev/null 2>&1 || { echo "Missing: $tool" >&2; exit 1; }
done
# A path without spaces avoids configure/libtool issues.
build_dir=$(mktemp -d /tmp/bridge-snark-build.XXXXXX)
printf 'Building Snarkhunter in %s\n' "$build_dir"
cd "$build_dir"
curl -fL https://users.cecs.anu.edu.au/~bdm/nauty/nauty2_9_3.tar.gz -o nauty.tar.gz
curl -fL https://caagt.ugent.be/cubic/snarkhunter-2.0b.zip -o snarkhunter.zip
tar -xzf nauty.tar.gz
unzip -q snarkhunter.zip
cd nauty2_9_3
CC="$cc" CFLAGS=-O3 sh ./configure > "$build_dir/configure.log" 2>&1 || {
    cat "$build_dir/configure.log" >&2; exit 1;
}
make nautyL1.a > "$build_dir/build.log" 2>&1 || {
    cat "$build_dir/build.log" >&2; exit 1;
}
cd "$build_dir"
"$cc" -O3 -DWORDSIZE=64 -DMAXN=WORDSIZE -I"$build_dir/nauty2_9_3" \
    "$build_dir/snarkhunter-2.0b/snarkhunter.c" \
    "$build_dir/nauty2_9_3/nautyL1.a" -o snarkhunter-64
./snarkhunter-64 10 4 s o g > smoke.g6 2> smoke.log
test "$(wc -l < smoke.g6)" -eq 6
mkdir -p "$project_dir/bin"
cp snarkhunter-64 "$project_dir/bin/snarkhunter-64.new"
chmod +x "$project_dir/bin/snarkhunter-64.new"
mv "$project_dir/bin/snarkhunter-64.new" "$project_dir/bin/snarkhunter-64"
printf 'Snarkhunter installed and small count verified.\nBuild logs/sources: %s\n' "$build_dir"
