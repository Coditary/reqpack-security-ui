#include "rqps/cli/output_renderers.hpp"

#include "rqps/cli/scan_output.h"
#include "rqps/cli_util.h"
#include "rqps/scan_service.h"

#include <unistd.h>

#include <map>
#include <memory>

namespace rqps::cli {

namespace {

std::string package_ref(const ValidationFinding& finding) {
    std::string packageRef = finding.package.system;
    if (!finding.package.name.empty()) {
        if (!packageRef.empty()) {
            packageRef += ':';
        }
        packageRef += finding.package.name;
        if (!finding.package.version.empty()) {
            packageRef += '@' + finding.package.version;
        }
    }
    return packageRef;
}

bool stdout_is_tty() {
    return isatty(STDOUT_FILENO) != 0;
}

}  // namespace

int TtyRenderer::renderPolicy(const PolicyEvaluation& evaluation, std::ostream& output) const {
    const std::vector<ValidationFinding>& vulnerabilities = evaluation.activeFindings;
    if (vulnerabilities.empty()) {
        if (stdout_is_tty()) {
            output << "\033[32m✓\033[0m rqps: no vulnerabilities\n";
        } else {
            output << "✓ rqps: no vulnerabilities\n";
        }
    } else {
        output << "PKG\tSEVERITY\tID\tMESSAGE\n";
        for (const ValidationFinding& finding : vulnerabilities) {
            output << package_ref(finding) << '\t' << finding.severity << '\t' << finding.id << '\t' << finding.message
                   << '\n';
        }
        std::map<std::string, int> counts;
        for (const ValidationFinding& finding : vulnerabilities) {
            counts[finding.severity]++;
        }
        if (stdout_is_tty()) {
            output << "\033[31m✗\033[0m rqps: " << vulnerabilities.size() << " vulnerabilities";
        } else {
            output << "✗ rqps: " << vulnerabilities.size() << " vulnerabilities";
        }
        for (const auto& [severity, count] : counts) {
            output << " (" << count << ' ' << severity << ')';
        }
        output << '\n';
    }
    return evaluation.suggestedExitCode;
}

int JsonRenderer::renderPolicy(const PolicyEvaluation& evaluation, std::ostream& output) const {
    output << findings_to_json(evaluation.activeFindings) << '\n';
    return evaluation.suggestedExitCode;
}

int QuietRenderer::renderPolicy(const PolicyEvaluation& evaluation, std::ostream& output) const {
    output << (evaluation.failed ? "FAIL\n" : "PASS\n");
    return evaluation.suggestedExitCode;
}

std::unique_ptr<IOutputRenderer> make_output_renderer(const ScanOutputOptions& options) {
    if (options.json) {
        return std::make_unique<JsonRenderer>();
    }
    if (options.quiet) {
        return std::make_unique<QuietRenderer>();
    }
    return std::make_unique<TtyRenderer>();
}

}  // namespace rqps::cli
