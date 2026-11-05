#include "rqps/sbom_service.h"

#include "rqps/ports/i_process_runner.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <fstream>
#include <map>
#include <sstream>

namespace {

std::optional<std::string> run_syft_with_format(const SyftDiscoverOptions& options,
                                                const std::filesystem::path& directory, const std::string& formatFlag,
                                                rqps::IProcessRunner& processRunner) {
    const std::string dirArg = "dir:" + directory.string();
    const rqps::Result<rqps::ProcessOutput> output =
        processRunner.run(options.syftExecutable, {dirArg, "-o", formatFlag, "-q"});
    if (!output.has_value() || output.value().exitCode != 0) {
        return std::nullopt;
    }
    return output.value().stdoutText;
}

std::string artifact_key(const boost::property_tree::ptree& artifact) {
    return artifact.get("name", "") + '\0' + artifact.get("version", "") + '\0' + artifact.get("type", "");
}

}  // namespace

std::optional<std::string> run_syft_sbom(const SyftDiscoverOptions& options, const std::filesystem::path& directory,
                                         const std::string& outputFormat) {
    return run_syft_sbom(options, directory, outputFormat, rqps::default_process_runner());
}

std::optional<std::string> run_syft_sbom(const SyftDiscoverOptions& options, const std::filesystem::path& directory,
                                         const std::string& outputFormat, rqps::IProcessRunner& processRunner) {
    std::string format = outputFormat;
    if (format == "cyclonedx" || format == "cyclonedx-json") {
        format = "cyclonedx-json";
    } else if (format.empty() || format == "json") {
        format = "json";
    }
    return run_syft_with_format(options, directory, format, processRunner);
}

std::optional<std::string> merge_syft_json_sboms(const std::vector<std::filesystem::path>& inputs) {
    if (inputs.empty()) {
        return std::nullopt;
    }

    boost::property_tree::ptree merged;
    std::map<std::string, boost::property_tree::ptree> uniqueArtifacts;

    for (const std::filesystem::path& input : inputs) {
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(input.string(), tree);
        if (merged.empty()) {
            merged = tree;
        }
        if (const auto artifacts = tree.get_child_optional("artifacts")) {
            for (const auto& [_, artifact] : artifacts.value()) {
                uniqueArtifacts[artifact_key(artifact)] = artifact;
            }
        }
    }

    boost::property_tree::ptree artifactsNode;
    for (const auto& [_, artifact] : uniqueArtifacts) {
        artifactsNode.push_back({"", artifact});
    }
    merged.erase("artifacts");
    merged.add_child("artifacts", artifactsNode);

    std::ostringstream stream;
    boost::property_tree::write_json(stream, merged);
    return stream.str();
}
