#pragma once

#include "rqps/ports/i_process_runner.hpp"

namespace rqps {

class OsProcessRunner final : public IProcessRunner {
public:
    Result<ProcessOutput> run(const std::filesystem::path& executable, const std::vector<std::string>& arguments,
                              const std::optional<std::filesystem::path>& workingDirectory = std::nullopt) override;
};

}  // namespace rqps
