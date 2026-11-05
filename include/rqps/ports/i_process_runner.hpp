#pragma once

#include "rqps/domain/error.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace rqps {

struct ProcessOutput {
    std::string stdoutText;
    int exitCode{0};
};

class IProcessRunner {
public:
    virtual ~IProcessRunner() = default;

    virtual Result<ProcessOutput> run(const std::filesystem::path& executable,
                                      const std::vector<std::string>& arguments,
                                      const std::optional<std::filesystem::path>& workingDirectory = std::nullopt) = 0;
};

IProcessRunner& default_process_runner();

}  // namespace rqps
