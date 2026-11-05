#pragma once

#include "rqps/domain/config_model.hpp"
#include "rqps/domain/error.hpp"
#include "rqps/domain/models.hpp"
#include "rqps/ports/i_process_runner.hpp"

#include <filesystem>
#include <optional>
#include <string>

struct SyftDiscoverResult {
    std::string json;
    std::optional<std::string> error;
};

SyftDiscoverResult run_syft_discover(const SyftDiscoverOptions& options, const std::filesystem::path& requestedPath);

rqps::Result<rqps::DiscoverSummary> syft_discover_summary(const SyftDiscoverOptions& options,
                                                          const std::filesystem::path& requestedPath,
                                                          rqps::IProcessRunner& processRunner);

std::string discover_summary_to_json(const rqps::DiscoverSummary& summary);

std::string syft_discover_info_json(const SyftDiscoverOptions& options, rqps::IProcessRunner& processRunner);

bool is_path_allowed_for_discover(const SyftDiscoverOptions& options, const std::filesystem::path& canonicalPath);

bool syft_is_available(const std::string& executable, rqps::IProcessRunner& processRunner);

inline bool syft_is_available(const std::string& executable) {
    return syft_is_available(executable, rqps::default_process_runner());
}

inline std::string syft_discover_info_json(const SyftDiscoverOptions& options) {
    return syft_discover_info_json(options, rqps::default_process_runner());
}
