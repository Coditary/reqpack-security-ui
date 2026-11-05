#pragma once

#include "rqps/rqps_config.h"

#include <filesystem>
#include <string>
#include <vector>

namespace rqps::cli {

struct GlobalOptions {
    std::filesystem::path configPath;
    bool json{false};
    bool quiet{false};
    bool noSync{false};
};

bool starts_with(const std::string& value, const std::string& prefix);

GlobalOptions parse_global_pre_args(std::vector<std::string>& args);

RqpsConfig load_config_for_path(const GlobalOptions& global, const std::filesystem::path& projectPath);

bool ensure_db_if_needed(RqpsConfig& config, bool noSync, bool quiet);

std::vector<std::string> argv_to_args(int argc, char* argv[]);

void print_usage();

}  // namespace rqps::cli
