#include "rqps/serve_runner.h"

#include <iostream>
#include <string>

namespace {

void print_usage() {
    std::cerr << "Usage: rqps-ui [--config path] [--host 127.0.0.1] [--port 8787]\n"
              << "       rqps serve …   (preferred — same server via rqps CLI)\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    ServeOptions options;
    for (int index = 1; index < argc; ++index) {
        const std::string arg = argv[index];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
        if (arg == "--config" && index + 1 < argc) {
            options.configPath = argv[++index];
            continue;
        }
        if (arg == "--host" && index + 1 < argc) {
            options.host = argv[++index];
            continue;
        }
        if (arg == "--port" && index + 1 < argc) {
            options.port = std::stoi(argv[++index]);
            continue;
        }
        print_usage();
        return 1;
    }
    return run_rqps_serve(options);
}
