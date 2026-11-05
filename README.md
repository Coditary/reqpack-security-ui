# reqpack-security-ui

Local **SCA CLI** (`rqps`) and optional **HTTP API** (`rqps-ui`) for [reqpack-security-core](https://github.com/Coditary/reqpack-security-core).  
**C++ only** — no Node.js, no browser frontend in this repo.  
Does **not** depend on the full `rqp` package manager.

**Modularität:** `rqps_core` (lokal, Syft, Scan) vs. optional `rqps_http` (REST + remote ping). ReqPack nutzt nur `reqpack-security-core`. Details: [docs/MODULARITY.md](docs/MODULARITY.md).

## Quick start — CLI (Snyk-style)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DREQPACK_SECURITY_CORE_DIR=/path/to/reqpack-security-core
cmake --build build
# Install syft: https://github.com/anchore/syft

cd your-project
./build/rqps               # same as rqps test .
./build/rqps test          # discover (local syft) + scan → exit 0/1
./build/rqps init          # optional .rqps.json in repo
./build/rqps doctor        # syft + OSV DB check (rqps doctor --fix creates dirs)
./build/rqps test --dry-run   # syft catalog only, no vuln scan
./build/rqps db sync       # populate ~/.local/share/rqps/osv
```

Optional global config: `~/.config/rqps/config.json` (see `config.example.json`).  
Per-repo overrides: `.rqps.json` in the project tree.

### `rqps` commands

| Command | Purpose |
|---------|---------|
| `test [path]` | Syft catalog + vulnerability scan (default `.`) |
| `audit [path]` | Like `test` (stricter CI defaults when supported) |
| `scan -f inventory.json` | Scan existing package list |
| `discover [path]` | Syft only (JSON summary) |
| `doctor` | Toolchain + DB readiness |
| `status` | Policy / DB summary |
| `db sync` / `db status` | OSV database |
| `sbom [path]` / `sbom merge …` | SBOM export / merge Syft JSON |
| `policy show` | Ignores, thresholds |
| `remote add\|list\|remove\|use` | Remote profiles (for `ping` / future offload) |
| `ping` / `health` | Remote server health |
| `serve` | Start HTTP JSON API (`rqps-ui` equivalent) |

Flags: `--json`, `--quiet`, `--no-sync`, `--config <path>`.

## Quick start — HTTP API (optional)

For automation or a custom client (curl, your own UI elsewhere):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

cp configs/rqps-ui.example.json rqps-ui.json   # edit paths / ecosystem map
./build/rqps-ui --config rqps-ui.json
# or: ./build/rqps serve --config rqps-ui.json
```

Example:

```bash
curl -s http://127.0.0.1:8787/api/v1/status
curl -s -X POST http://127.0.0.1:8787/api/v1/scan \
  -H 'Content-Type: application/json' \
  -d '{"input":"{\"packages\":[{\"system\":\"npm\",\"name\":\"left-pad\",\"version\":\"1.0.0\"}]}"}'
```

**Code coverage** for vulnerability matching lives in [reqpack-security-core](https://github.com/Coditary/reqpack-security-core) (`make test-coverage`). This repo’s `rqps` glue has no separate C++ unit-test target yet.

## Backend dependency (GitHub)

| CMake cache variable | Default | Purpose |
|---------------------|---------|---------|
| `REQPACK_SECURITY_CORE_REPO` | `https://github.com/Coditary/reqpack-security-core.git` | Upstream library |
| `REQPACK_SECURITY_CORE_TAG` | `main` | Branch, tag, or commit SHA |
| `REQPACK_SECURITY_CORE_DIR` | *(empty)* | Optional local checkout — **skips Git fetch** |
| Monorepo sibling | `../reqpack-security-core` | Auto-detected when present (same as ReqPack-Core) |

Pin a release for reproducible builds:

```bash
cmake -B build -DREQPACK_SECURITY_CORE_TAG=v0.1.0
```

Monorepo / co-development (optional):

```bash
cmake -B build -DREQPACK_SECURITY_CORE_DIR=/path/to/reqpack-security-core
```

## HTTP API

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/v1/health` | Liveness |
| GET | `/api/v1/status` | DB + policy diagnostics |
| GET | `/api/v1/discover/info` | Syft path, allowed roots, availability |
| POST | `/api/v1/discover` | JSON `{ "path": "/abs/repo" }` → Syft summary + ReqPack inventory |
| POST | `/api/v1/scan` | JSON `{ "input": "<inventory JSON string>", "format": "auto" }` → findings |

### Repo discovery (Syft)

The API can catalog a local repository with [Syft](https://github.com/anchore/syft) (`syft dir:… -o json`) and return ecosystems, catalogers, manifest paths, and a ReqPack inventory for scanning.

Configure in `rqps-ui.json`:

- `discoverEnabled` (default: true if present)
- `syftPath` — default `syft`
- `discoverAllowedRoots` — **required** list of absolute paths; only directories under these roots can be scanned (security boundary)
- `discoverMaxPackages` — cap unique packages in the inventory (default 8000)

Scan input uses **ReqPack inventory JSON** (`{ "packages": [ { "system", "name", "version" } ] }`). When you pin a newer security-core that includes `exchange.h`, CycloneDX/SPDX can be enabled via the exchange pipeline (see CMake status line at configure time).

Config file: JSON (see `configs/rqps-ui.example.json`).

## Layout

```text
reqpack-security-ui/
  CMakeLists.txt          # rqps_core, optional rqps_http, binaries
  cmake/
  include/rqps/
  src/
  configs/
  docs/MODULARITY.md
  docs/ARCHITECTURE_CHANGELOG.md   # hexagonal refactor map (2026-11)
```

CMake (nur lokale App ohne HTTP):

```bash
cmake -B build -DRQPS_BUILD_HTTP=OFF -DRQPS_BUILD_UI=OFF
```

Unit-Tests (`rqps_core` / Inventar / Config — kein Browser-Frontend):

```bash
cmake -B build -DRQPS_BUILD_TESTS=ON -DREQPACK_SECURITY_CORE_DIR=../reqpack-security-core
cmake --build build
ctest --test-dir build -R '^rqps::'
```

## Publishing this repo on GitHub

1. Create an empty repository (e.g. `Coditary/reqpack-security-ui`).
2. Push this directory as the root (not nested under ReqPack).
3. CI in `.github/workflows/ci.yml` builds `rqps` and `rqps-ui` on push.

The security **library** remains [reqpack-security-core](https://github.com/Coditary/reqpack-security-core); this repo only consumes it via FetchContent.
