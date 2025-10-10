#!/bin/bash
TEST_DIR="tests"
INCLUDE_DIR="include"
BIN_DIR="bin"

build() {
    mkdir -p "$BIN_DIR"

    if [ $# -eq 0 ] || [ "$1" == "all" ]; then
        echo "Building all tests..."
        for src in "$TEST_DIR"/test_*.c; do
            local base=$(basename "$src" .c)
            local out="$BIN_DIR/$base"
            gcc -I"$INCLUDE_DIR" "$src" -o "$out"
        done
    else
        for name in "$@"; do
            local src="$TEST_DIR/test_$name.c"
            local out="$BIN_DIR/test_$name"

            if [ ! -f "$src" ]; then
                echo "Source file $src does not exist."
                exit 1
            fi

            echo "Building test_$name"
            gcc -g3 -I"$INCLUDE_DIR" "$src" -o "$out"
        done
    fi
}


run_all() {
    for exe in "$BIN_DIR"/test_*; do
        echo "Running $(basename "$exe")"
        "$exe"
    done
}


run() {
    local allowed=("map" "vector" "heap" "list" "queue" "stack" "bitset" "all")

    if [ $# -eq 0 ]; then
        echo "Missing test name(s). Usage: run <test_name1> [test_name2 ...]"
        exit 1
    fi

    if [ "$1" == "all" ]; then
        run_all
        exit 0
    fi

    for name in "$@"; do
        if [[ ! " ${allowed[*]} " =~ " ${name} " ]]; then
            echo "Invalid test name: '$name'"
            echo "Allowed test names: ${allowed[*]}"
            exit 1
        fi

        local exe="$BIN_DIR/test_$name"

        if [ ! -f "$exe" ]; then
            echo "$exe not found. Rebuilding..."
            gcc -I"$INCLUDE_DIR" "$TEST_DIR/test_$name.c" -o "$exe"
        fi

        echo "Running test_$name"
        "$exe"
        echo
    done
}


clean() {
    rm -rf "$BIN_DIR"
}

case "$1" in
    build)
        shift
        build "$@"
        ;;
    run)
        shift
        run "$@"
        ;;
    run_all)
        run_all
        ;;
    clean)
        clean
        ;;
    *)
        echo "Usage: $0 {build [all|<name1> <name2> ...] | run <name1> ... | run_all | clean}"
        ;;
esac
