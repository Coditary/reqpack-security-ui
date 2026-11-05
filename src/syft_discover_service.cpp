#include "rqps/syft_discover_service.h"

#include "rqps/adapters/system/path_guard.hpp"
#include "rqps/json_util.h"
#include "rqps/ports/i_process_runner.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <map>
#include <set>
#include <sstream>

namespace {

std::string map_syft_type_to_system(const std::string& syftType) {
    std::string lowered = syftType;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

    if (lowered == "npm" || lowered == "javascript" || lowered == "yarn") {
        return "npm";
    }
    if (lowered == "python" || lowered == "pypi" || lowered == "pip") {
        return "pip";
    }
    if (lowered == "go-module" || lowered == "golang" || lowered == "go") {
        return "go";
    }
    if (lowered == "cargo" || lowered == "rust") {
        return "cargo";
    }
    if (lowered == "gem" || lowered == "rubygems" || lowered == "ruby") {
        return "gem";
    }
    if (lowered == "java-archive" || lowered == "maven" || lowered == "java") {
        return "maven";
    }
    if (lowered == "composer" || lowered == "php") {
        return "composer";
    }
    if (lowered == "nuget" || lowered == "dotnet") {
        return "nuget";
    }
    if (lowered == "deb" || lowered == "debian" || lowered == "dpkg") {
        return "apt";
    }
    if (lowered == "rpm" || lowered == "redhat") {
        return "dnf";
    }
    if (lowered == "apk" || lowered == "alpine") {
        return "apk";
    }
    return lowered.empty() ? "unknown" : lowered;
}

std::string package_key(const std::string& system, const std::string& name, const std::string& version) {
    return system + '\0' + name + '\0' + version;
}

rqps::Result<std::string> run_syft_json(const SyftDiscoverOptions& options, const std::filesystem::path& directory,
                                        rqps::IProcessRunner& processRunner) {
    const std::string dirArg = "dir:" + directory.string();
    const rqps::Result<rqps::ProcessOutput> output =
        processRunner.run(options.syftExecutable, {dirArg, "-o", "json", "-q"});
    if (!output.has_value()) {
        return rqps::Result<std::string>::fail(output.error());
    }
    if (output.value().exitCode != 0) {
        return rqps::Result<std::string>::fail(rqps::RqpsError::make(
            rqps::ErrorCode::ToolFailed, "syft failed (is it installed?)", directory.string(), output.value().exitCode));
    }
    return rqps::Result<std::string>::ok(output.value().stdoutText);
}

rqps::Result<rqps::DiscoverSummary> parse_syft_output(const std::string& syftOutput, const std::filesystem::path& canonical,
                                                      const SyftDiscoverOptions& options) {
    boost::property_tree::ptree tree;
    try {
        std::istringstream stream(syftOutput);
        boost::property_tree::read_json(stream, tree);
    } catch (const std::exception& exception) {
        return rqps::Result<rqps::DiscoverSummary>::fail(rqps::RqpsError::make(
            rqps::ErrorCode::InvalidJson, std::string("syft JSON parse error: ") + exception.what()));
    }

    rqps::DiscoverSummary summary;
    summary.canonicalPath = canonical;
    summary.sourceType = tree.get("source.type", "directory");

    std::set<std::string> ecosystems;
    std::set<std::string> catalogers;
    std::map<std::string, rqps::PackageItem> uniquePackages;

    if (const auto artifacts = tree.get_child_optional("artifacts")) {
        summary.artifactCount = artifacts->size();
        for (const auto& [_, artifact] : artifacts.value()) {
            const std::string name = artifact.get("name", "");
            const std::string version = artifact.get("version", "");
            if (name.empty() || version.empty()) {
                continue;
            }
            const std::string syftType = artifact.get("type", "");
            const std::string system = map_syft_type_to_system(syftType);
            ecosystems.insert(syftType.empty() ? system : syftType);

            std::string foundBy = artifact.get("foundBy", "");
            if (foundBy.empty()) {
                if (const auto metadata = artifact.get_child_optional("metadata")) {
                    foundBy = metadata->get("foundBy", "");
                }
            }
            if (!foundBy.empty()) {
                catalogers.insert(foundBy);
            }

            const std::string key = package_key(system, name, version);
            if (!uniquePackages.contains(key)) {
                uniquePackages.emplace(key, rqps::PackageItem{system, name, version});
            }
            if (uniquePackages.size() > options.maxPackages) {
                break;
            }
        }
    }

    summary.truncated = uniquePackages.size() >= options.maxPackages;
    summary.packageCount = uniquePackages.size();
    summary.ecosystems.assign(ecosystems.begin(), ecosystems.end());
    summary.catalogers.assign(catalogers.begin(), catalogers.end());
    for (const auto& [_, package] : uniquePackages) {
        summary.inventory.packages.push_back(package);
    }
    if (summary.truncated) {
        summary.warnings.push_back("package list truncated at " + std::to_string(options.maxPackages) +
                                   " unique packages");
    }
    return rqps::Result<rqps::DiscoverSummary>::ok(std::move(summary));
}

}  // namespace

bool syft_is_available(const std::string& executable, rqps::IProcessRunner& processRunner) {
    const rqps::Result<rqps::ProcessOutput> output = processRunner.run(executable, {"version"});
    return output.has_value() && output.value().exitCode == 0 && !output.value().stdoutText.empty();
}

std::string syft_discover_info_json(const SyftDiscoverOptions& options, rqps::IProcessRunner& processRunner) {
    const bool available = syft_is_available(options.syftExecutable, processRunner);
    std::ostringstream stream;
    stream << "{\"enabled\":" << (options.enabled ? "true" : "false")
           << ",\"syftAvailable\":" << (available ? "true" : "false") << ",\"syftPath\":\""
           << json_escape(options.syftExecutable) << "\",\"allowedRoots\":[";
    for (std::size_t index = 0; index < options.allowedRoots.size(); ++index) {
        if (index > 0) {
            stream << ',';
        }
        stream << '\"' << json_escape(options.allowedRoots[index].string()) << '\"';
    }
    stream << "]}";
    return stream.str();
}

std::string discover_summary_to_json(const rqps::DiscoverSummary& summary) {
    std::ostringstream json;
    json << '{';
    json << "\"path\":\"" << json_escape(summary.canonicalPath.string()) << "\"";
    json << ",\"sourceType\":\"" << json_escape(summary.sourceType) << "\"";
    json << ",\"artifactCount\":" << summary.artifactCount;
    json << ",\"packageCount\":" << summary.packageCount;
    json << ",\"truncated\":" << (summary.truncated ? "true" : "false");

    json << ",\"ecosystems\":[";
    for (std::size_t index = 0; index < summary.ecosystems.size(); ++index) {
        if (index > 0) {
            json << ',';
        }
        json << '\"' << json_escape(summary.ecosystems[index]) << '\"';
    }
    json << ']';

    json << ",\"catalogers\":[";
    for (std::size_t index = 0; index < summary.catalogers.size(); ++index) {
        if (index > 0) {
            json << ',';
        }
        json << '\"' << json_escape(summary.catalogers[index]) << '\"';
    }
    json << ']';

    json << ",\"manifestPaths\":[]";

    json << ",\"inventory\":{\"packages\":[";
    for (std::size_t index = 0; index < summary.inventory.packages.size(); ++index) {
        if (index > 0) {
            json << ',';
        }
        const rqps::PackageItem& package = summary.inventory.packages[index];
        json << '{'
             << "\"system\":\"" << json_escape(package.system) << "\","
             << "\"name\":\"" << json_escape(package.name) << "\","
             << "\"version\":\"" << json_escape(package.version) << "\""
             << '}';
    }
    json << "]}";

    json << ",\"warnings\":[";
    for (std::size_t index = 0; index < summary.warnings.size(); ++index) {
        if (index > 0) {
            json << ',';
        }
        json << '\"' << json_escape(summary.warnings[index]) << '\"';
    }
    json << "]}";

    json << '}';
    return json.str();
}

rqps::Result<rqps::DiscoverSummary> syft_discover_summary(const SyftDiscoverOptions& options,
                                                          const std::filesystem::path& requestedPath,
                                                          rqps::IProcessRunner& processRunner) {
    const rqps::Result<std::filesystem::path> validated =
        rqps::PathGuard::validateDiscoverPath(options, requestedPath);
    if (!validated.has_value()) {
        return rqps::Result<rqps::DiscoverSummary>::fail(validated.error());
    }

    const rqps::Result<std::string> syftOutput = run_syft_json(options, validated.value(), processRunner);
    if (!syftOutput.has_value()) {
        return rqps::Result<rqps::DiscoverSummary>::fail(syftOutput.error());
    }

    return parse_syft_output(syftOutput.value(), validated.value(), options);
}

SyftDiscoverResult run_syft_discover(const SyftDiscoverOptions& options, const std::filesystem::path& requestedPath) {
    SyftDiscoverResult result;
    const rqps::Result<rqps::DiscoverSummary> summary =
        syft_discover_summary(options, requestedPath, rqps::default_process_runner());
    if (!summary.has_value()) {
        result.error = summary.error().message;
        return result;
    }
    result.json = discover_summary_to_json(summary.value());
    return result;
}
