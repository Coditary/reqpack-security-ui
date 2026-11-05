#include "rqps/cli_util.h"

#include <unistd.h>

bool cli_stdout_is_tty() {
    return isatty(STDOUT_FILENO) != 0;
}

std::string cli_green(const std::string& text) {
    if (!cli_stdout_is_tty()) {
        return text;
    }
    return "\033[32m" + text + "\033[0m";
}

std::string cli_red(const std::string& text) {
    if (!cli_stdout_is_tty()) {
        return text;
    }
    return "\033[31m" + text + "\033[0m";
}

std::string cli_yellow(const std::string& text) {
    if (!cli_stdout_is_tty()) {
        return text;
    }
    return "\033[33m" + text + "\033[0m";
}

void print_syft_missing_help(const std::string& syftPath) {
    std::cerr << "error: syft not found (looked for \"" << syftPath << "\")\n"
              << "hint: install Syft, then ensure it is on PATH\n"
              << "      https://github.com/anchore/syft#installation\n"
              << "      or set syftPath in ~/.config/rqps/config.json / .rqps.json\n";
}

void print_db_empty_help() {
    std::cerr << "hint: vulnerability database is empty — run: rqps db sync\n"
              << "      (or allow auto-sync on first rqps test; do not use --offline)\n";
}

void print_rqps_banner_help() {
    std::cerr << "Quick start:  cd your-project && rqps test\n"
              << "              rqps          (same as rqps test .)\n"
              << "              rqps doctor   rqps db sync\n\n";
}

void print_test_command_help() {
    std::cerr << "Usage: rqps test [path] [options]\n"
              << "  Scans the directory with Syft, then matches against the local OSV database.\n\n"
              << "Options:\n"
              << "  --dry-run          Catalog only (Syft), no vulnerability scan\n"
              << "  --max-packages N   Cap packages from Syft (default from config)\n"
              << "  --offline          Same as --no-sync; fail if DB is empty\n"
              << "  --no-sync          Do not auto-sync OSV data before scan\n"
              << "  --json             JSON findings on stdout\n"
              << "  --quiet            PASS/FAIL line only\n"
              << "  --config <path>    Config file (default ~/.config/rqps/config.json)\n";
}

bool is_rqps_known_command(const std::string& word) {
    return word == "test" || word == "audit" || word == "scan" || word == "discover" || word == "doctor" ||
           word == "status" || word == "db" || word == "policy" || word == "sbom" || word == "remote" ||
           word == "ping" || word == "health" || word == "serve" || word == "version" || word == "init" ||
           word == "help";
}
