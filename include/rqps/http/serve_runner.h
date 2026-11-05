#pragma once

#include "rqps/domain/config_model.hpp"

#include <filesystem>
#include <string>

struct ServeOptions {
    std::filesystem::path configPath;
    std::string host{"127.0.0.1"};
    int port{8787};
};

int run_rqps_serve(const ServeOptions& options);
