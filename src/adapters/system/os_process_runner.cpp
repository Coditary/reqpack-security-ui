#include "rqps/adapters/system/os_process_runner.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <vector>

namespace rqps {

namespace {

IProcessRunner& process_runner_instance() {
    static OsProcessRunner runner;
    return runner;
}

}  // namespace

IProcessRunner& default_process_runner() {
    return process_runner_instance();
}

Result<ProcessOutput> OsProcessRunner::run(const std::filesystem::path& executable,
                                           const std::vector<std::string>& arguments,
                                           const std::optional<std::filesystem::path>& workingDirectory) {
    (void)workingDirectory;

    std::vector<std::string> argvStrings;
    argvStrings.push_back(executable.string());
    argvStrings.insert(argvStrings.end(), arguments.begin(), arguments.end());

    int pipeFds[2];
    if (pipe(pipeFds) != 0) {
        return Result<ProcessOutput>::fail(
            RqpsError::make(ErrorCode::Internal, "failed to create pipe for subprocess"));
    }

    const pid_t child = fork();
    if (child < 0) {
        close(pipeFds[0]);
        close(pipeFds[1]);
        return Result<ProcessOutput>::fail(RqpsError::make(ErrorCode::Internal, "failed to fork subprocess"));
    }

    if (child == 0) {
        close(pipeFds[0]);
        dup2(pipeFds[1], STDOUT_FILENO);
        close(pipeFds[1]);

        std::vector<char*> argv;
        argv.reserve(argvStrings.size() + 1);
        for (std::string& argument : argvStrings) {
            argv.push_back(argument.data());
        }
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        _exit(127);
    }

    close(pipeFds[1]);
    ProcessOutput output;
    char buffer[8192];
    while (true) {
        const ssize_t bytes = read(pipeFds[0], buffer, sizeof(buffer));
        if (bytes <= 0) {
            break;
        }
        output.stdoutText.append(buffer, static_cast<std::size_t>(bytes));
    }
    close(pipeFds[0]);

    int status = 0;
    waitpid(child, &status, 0);
    output.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    if (output.exitCode == 127) {
        return Result<ProcessOutput>::fail(
            RqpsError::make(ErrorCode::ToolNotFound, "executable not found or not runnable", executable.string(),
                            output.exitCode));
    }
    return Result<ProcessOutput>::ok(std::move(output));
}

}  // namespace rqps
