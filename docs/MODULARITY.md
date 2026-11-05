# Modularität: Security-Core vs. rqps

## Zwei Ebenen

| Ebene | Repo / Target | Zweck |
| --- | --- | --- |
| **Security-Core** | `reqpack-security-core` → `reqpack_security_core` | OSV, DB, Validator, Sync-Backends, Gateway — **kein HTTP, kein Syft, kein Terminal** |
| **rqps-SDK** | `reqpack-security-ui` → `rqps_core` | Application core + adapters (config, Syft via `IProcessRunner`, scan via `IVulnerabilityScanner`, workflows). **No terminal I/O.** |
| **CLI presentation** | `rqps` binary (`src/cli/`) | Tables, colors, exit rendering (`scan_output`) |
| **HTTP (optional)** | `rqps_http` | `rqps-ui` / `serve` / `remote ping` — **cpp-httplib** |
| **Apps** | `rqps`, `rqps-ui` | Binaries; ReqPack nutzt nur **Security-Core** (+ Core-Bridge), kein rqps |

## Was in `reqpack-security-core` **Kern** ist (für jede App)

- `validator`, `validator_core`, OSV match, `vulnerability_database`, `vulnerability_matcher`
- `security_settings`, `security_types`, `sbom_inventory` (Datenmodell)
- `version_compare`, `security_paths`, `security_log`, `security_process` (Subprozesse generisch)
- Mindestens **ein** Sync-Weg zu OSV-Daten (heute alles in einem Target)

## Was **nicht zwingend** im monolithischen Core-Target sein muss (langfristig CMake-Optionen)

Diese Teile sind **Library-Code**, aber **optionale Produkte** — eigene Apps können sie weglassen:

| Modul | Inhalt | Wann weglassen |
| --- | --- | --- |
| **Sync: Snyk/Trivy** | `snyk_*`, `trivy_*` | CMake `-DRQP_SECURITY_BACKEND_SNYK=OFF` usw. |
| **Sync: GH Advisory** | `gh_advisory_*` | `-DRQP_SECURITY_BACKEND_GH_ADVISORY=OFF` |
| **Exchange** | `exchange/*`, CycloneDX/SPDX-Pipeline | Nur ReqPack-Inventar-JSON |
| **Gateway** | `security_gateway_service` | ReqPack-Executor-Gateway nicht genutzt |
| **security_network** | HTTP-Downloads für Sync | Offline-only mit fertiger DB |

ReqPack braucht **Validator + OSV-DB + passende Sync**; keinen Webserver, kein Syft in der Core-Lib.

## Was **nie** in Security-Core gehört (liegt richtig in rqps / ReqPack)

- HTTP-Server / REST (`serve_runner`, httplib)
- Syft-Aufruf und Repo-Discovery (`syft_discover_service`)
- Remote-Profile / `ping` (`remote_config`)
- Terminal-Tabellen, Exit-Codes der CLI (`scan_output`, `rqps_cli/commands/*`, `cli/output_renderers`)
- ReqPack: SARIF/Display, Orchestrator (`ReqPack-Core`)

## CMake in diesem Repo

```text
reqpack_security_core   (extern)
        ↑
    rqps_core             ← domain + application + adapters (no httplib, no TTY)
        ↑
    rqps (CLI)            ← scan_output + rqps_cli
        ↑
    rqps_http             ← optional (RQPS_BUILD_HTTP)
        ↑
   rqps-ui                ← HTTP binary
```

Eigene lokale App: nur `reqpack_security_core` + `rqps_core` (oder direkt nur Core, wenn du kein Syft/XDG-Config brauchst).
