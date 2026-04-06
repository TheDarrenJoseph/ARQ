#!/bin/bash

set -e

# Parses the LCOV file (lcov.info) to read out the total lines found (LF) and total lines hit (LH)
# We then sum each and calculate the total % that the sum of LH is of the sum of LF
function analyse_totals() {
    TOTAL_LINES_FOUND=$(grep "LF" target/coverage/lcov.info | cut -d: -f2 | xargs |  sed -e 's/\ /+/g' | bc)
    TOTAL_LINES_HIT=$(grep "LH" target/coverage/lcov.info | cut -d: -f2 | xargs |  sed -e 's/\ /+/g' | bc)
    echo "Total lines found: $TOTAL_LINES_FOUND" >&2
    echo "Total lines hit: $TOTAL_LINES_HIT" >&2
    echo "$TOTAL_LINES_HIT / $TOTAL_LINES_FOUND" >&2
    TOTAL_LINE_COVERAGE_PERCENTAGE=$(echo "scale=6; $TOTAL_LINES_HIT / $TOTAL_LINES_FOUND * 100" | bc -l)
    BADGE_PERCENTAGE=$(echo $TOTAL_LINE_COVERAGE_PERCENTAGE | grep -Eo "[0-9]*\.[0-9]{2}")
    echo "Total lines covered (percentage): $BADGE_PERCENTAGE" >&2
    echo "$BADGE_PERCENTAGE"
}

export RUSTFLAGS="-Cinstrument-coverage"
rm -rf ./target/coverage
mkdir -p "./target/coverage"
export LLVM_PROFILE_FILE="target/coverage/%p-%m.profraw"
cargo test

# Run grcov to generate the LCOV file (lcov.info)
grcov . --binary-path ./target/debug/ -s . -t lcov --branch --ignore-not-existing --ignore "/*" -o ./target/coverage/lcov.info
# Remove the custom test module src/test from the coverage reporting
lcov --remove ./target/coverage/lcov.info -o ./target/coverage/lcov.info\
    'src/test'

# Generate the HTML version of the report for development usage
genhtml --ignore-errors unmapped -o ./target/coverage/ --show-details --ignore-errors source --legend ./target/coverage/lcov.info

# Generate the total line coverage % badge
BADGE_PERCENTAGE=$(analyse_totals)
wget https://img.shields.io/badge/Line_Coverage-$BADGE_PERCENTAGE%-green -O ../images/badge.svg
