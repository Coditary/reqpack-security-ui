.PHONY: all build test test-ci clean help

NPROC := $(shell command -v nproc >/dev/null 2>&1 && nproc || sysctl -n hw.ncpu 2>/dev/null || echo 2)
JOBS := $(shell expr $(NPROC) / 4)
BUILD_DIR := build
REQPACK_MONOREPO_ROOT ?= $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

all: build

build:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DRQPS_BUILD_TESTS=ON
	cmake --build $(BUILD_DIR) -j$(JOBS)

test: build
	@ctest --test-dir $(BUILD_DIR) --output-on-failure -R '^rqps::'
	@test -x $(BUILD_DIR)/rqps
	@test -x $(BUILD_DIR)/rqps-ui

test-ci:
	@bash "$(REQPACK_MONOREPO_ROOT)/scripts/ci-local/run-act.sh" reqpack-security-ui "$(REQPACK_MONOREPO_ROOT)"

help:
	@echo "  make build     Configure and build rqps + rqps-ui"
	@echo "  make test      Unit tests + smoke binaries (matches CI)"
	@echo "  make test-ci   GitHub Actions ci.yml via nektos/act (make act-install at repo root if needed)"

clean:
	rm -rf $(BUILD_DIR)
