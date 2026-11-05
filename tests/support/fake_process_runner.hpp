#pragma once

#include "rqps/ports/i_process_runner.hpp"

#include <map>
#include <string>

namespace rqps::test {

class FakeProcessRunner final : public IProcessRunner {
public:
    void set_response(const std::string& key, ProcessOutput output) { responses_[key] = std::move(output); }

    Result<ProcessOutput> run(const std::filesystem::path& executable, const std::vector<std::string>& arguments,
                              const std::optional<std::filesystem::path>& workingDirectory = std::nullopt) override {
        (void)workingDirectory;
        const std::string key = make_key(executable.string(), arguments);
        const auto iterator = responses_.find(key);
        if (iterator == responses_.end()) {
            return Result<ProcessOutput>::fail(
                RqpsError::make(ErrorCode::ToolNotFound, "no fake response", key));
        }
        return Result<ProcessOutput>::ok(iterator->second);
    }

    static std::string make_key(const std::string& executable, const std::vector<std::string>& arguments) {
        std::string key = executable;
        for (const std::string& argument : arguments) {
            key.push_back('\0');
            key += argument;
        }
        return key;
    }

private:
    std::map<std::string, ProcessOutput> responses_;
};

}  // namespace rqps::test
