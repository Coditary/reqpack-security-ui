#pragma once

#include "rqps/ports/i_process_runner.hpp"

#include "rqps/domain/config_model.hpp"
#include <optional>
#include <string>
#include <vector>

std::optional<std::string> run_syft_sbom(const SyftDiscoverOptions& options, const std::filesystem::path& directory,
                                         const std::string& outputFormat);

std::optional<std::string> run_syft_sbom(const SyftDiscoverOptions& options, const std::filesystem::path& directory,
                                         const std::string& outputFormat, rqps::IProcessRunner& processRunner);

std::optional<std::string> merge_syft_json_sboms(const std::vector<std::filesystem::path>& inputs);
