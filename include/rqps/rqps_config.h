#pragma once
#include "rqps/domain/config_model.hpp"

RqpsConfig load_rqps_config(const std::filesystem::path& explicitConfigPath = {});

RqpsConfig merge_project_config(RqpsConfig base, const std::filesystem::path& projectDirectory);

std::filesystem::path rqps_default_config_dir();

std::filesystem::path rqps_default_data_dir();
