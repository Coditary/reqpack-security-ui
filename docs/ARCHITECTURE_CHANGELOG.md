# Architecture changelog (hexagonal refactor)

Refactor date: 2026-11-02. Scope: **reqpack-security-ui only**. CLI commands and `/api/v1/*` contracts unchanged.

## Layer map

| Layer | Path | Responsibility |
| --- | --- | --- |
| **Domain** | `include/rqps/domain/` | `RqpsError`, `Result<T>`, `Inventory`, `AuditReport`, `RqpsConfig` |
| **Ports** | `include/rqps/ports/` | `IProcessRunner`, `IVulnerabilityScanner` |
| **Application** | `include/rqps/application/`, `src/application/` | `AuditWorkflow`, `DoctorWorkflow`, `scan_inventory_json_report`, `evaluate_policy`, `default_*()` registry |
| **Adapters** | `include/rqps/adapters/`, `src/adapters/` | `OsProcessRunner`, `PathGuard`, `SecurityCoreScanner` |
| **CLI** | `include/rqps/cli/`, `src/cli/` | `cli_context`, `IOutputRenderer` (`Tty` / `Json` / `Quiet`), `commands/*`, thin `rqps_cli.cpp` |
| **HTTP** | `include/rqps/http/`, `src/http/` | `serve_runner`, `HttpServerContext`, `routes/*`, error → status mapping |

## CLI command modules

| Command | Header | Source |
| --- | --- | --- |
| `test` / `audit` | `cli/commands/test_command.h` | `src/cli/commands/test_command.cpp` |
| `scan` | `cli/commands/scan_command.h` | `src/cli/commands/scan_command.cpp` |
| `discover` | `cli/commands/discover_command.h` | `src/cli/commands/discover_command.cpp` |
| `doctor` | `cli/commands/doctor_command.h` | `src/cli/commands/doctor_command.cpp` |
| `db` / `status` | `cli/commands/db_command.h` | `src/cli/commands/db_command.cpp` |
| `sbom` | `cli/commands/sbom_command.h` | `src/cli/commands/sbom_command.cpp` |
| `init`, `version`, `policy`, HTTP | `cli/commands/misc_commands.h` | `src/cli/commands/misc_commands.cpp` |
| Dispatch only | — | `src/rqps_cli.cpp` (~100 lines) |

Shared flags / DB bootstrap: `cli/cli_context.h` + `src/cli/cli_context.cpp`.

## HTTP routes

| Route | Module |
| --- | --- |
| `GET /api/v1/health` | `http/routes/health_routes.*` |
| `GET /api/v1/status` | `http/routes/status_routes.*` |
| `GET/POST discover` | `http/routes/discover_routes.*` |
| `POST /api/v1/scan` | `http/routes/scan_routes.*` (uses `scan_inventory_json_report` + `IVulnerabilityScanner`) |
| Wiring + CORS | `http/routes/register_routes.*` |
| Entry | `http/serve_runner.cpp` |

Legacy shim: `include/rqps/serve_runner.h` → `include/rqps/http/serve_runner.h`.

## Presentation

- `IOutputRenderer` + `TtyRenderer` / `JsonRenderer` / `QuietRenderer` in `cli/output_renderers.*`
- `scan_output.cpp` delegates to `make_output_renderer()`

## Tests

```bash
cmake -B build -DRQPS_BUILD_TESTS=ON -DREQPACK_SECURITY_CORE_DIR=../reqpack-security-core
cmake --build build && ctest --test-dir build -R '^rqps::'
```

See also: `docs/MODULARITY.md`.
