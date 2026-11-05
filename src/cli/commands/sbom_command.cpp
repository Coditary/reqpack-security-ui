#include "rqps/cli/commands/sbom_command.h"

#include "rqps/cli/cli_context.h"
#include "rqps/sbom_service.h"

#include <fstream>
#include <iostream>

namespace rqps::cli {

int cmd_sbom(int argc, char* argv[]) {
    std::vector<std::string> args = argv_to_args(argc, argv);
    const GlobalOptions global = parse_global_pre_args(args);

    if (args.size() >= 2 && args[1] == "merge") {
        if (args.size() < 4) {
            std::cerr << "usage: rqps sbom merge <a.json> <b.json> [-o out.json]\n";
            return 2;
        }
        std::filesystem::path outputPath = "merged-sbom.json";
        std::vector<std::filesystem::path> inputs;
        for (std::size_t index = 2; index < args.size(); ++index) {
            if (args[index] == "-o" && index + 1 < args.size()) {
                outputPath = args[++index];
                continue;
            }
            inputs.emplace_back(args[index]);
        }
        const auto merged = merge_syft_json_sboms(inputs);
        if (!merged.has_value()) {
            std::cerr << "merge failed\n";
            return 2;
        }
        std::ofstream output(outputPath);
        output << merged.value();
        std::cout << "wrote " << outputPath << '\n';
        return 0;
    }

    std::filesystem::path projectPath = ".";
    std::filesystem::path outputPath;
    std::string format = "json";
    for (std::size_t index = 1; index < args.size(); ++index) {
        if (args[index] == "-o" && index + 1 < args.size()) {
            outputPath = args[++index];
            continue;
        }
        if (args[index] == "--format" && index + 1 < args.size()) {
            format = args[++index];
            continue;
        }
        if (!starts_with(args[index], "-")) {
            projectPath = args[index];
        }
    }

    RqpsConfig config = load_config_for_path(global, projectPath);
    const auto sbom = run_syft_sbom(config.discover, projectPath, format);
    if (!sbom.has_value()) {
        std::cerr << "sbom generation failed (syft?)\n";
        return 2;
    }
    if (outputPath.empty()) {
        std::cout << sbom.value();
    } else {
        std::ofstream output(outputPath);
        output << sbom.value();
        std::cout << "wrote " << outputPath << '\n';
    }
    return 0;
}

}  // namespace rqps::cli
