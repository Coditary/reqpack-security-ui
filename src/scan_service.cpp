#include "rqps/scan_service.h"

#include "rqps/inventory_json.h"
#include "rqps/json_util.h"

#include "rqp/security/validator.h"

#include <boost/graph/graph_traits.hpp>

#include <sstream>

std::string findings_to_json(const std::vector<ValidationFinding>& findings) {
    std::ostringstream stream;
    stream << "{\"kind\":\"audit-report\",\"findings\":[";
    for (std::size_t index = 0; index < findings.size(); ++index) {
        const ValidationFinding& finding = findings[index];
        if (index > 0) {
            stream << ',';
        }
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
        stream << "{"
               << "\"id\":\"" << json_escape(finding.id) << "\","
               << "\"kind\":\"" << json_escape(finding.kind) << "\","
               << "\"source\":\"" << json_escape(finding.source) << "\","
               << "\"severity\":\"" << json_escape(finding.severity) << "\","
               << "\"score\":" << finding.score << ","
               << "\"message\":\"" << json_escape(finding.message) << "\","
               << "\"packageRef\":\"" << json_escape(packageRef) << "\""
               << "}";
    }
    stream << "],\"packages\":[]}";
    return stream.str();
}

std::vector<ValidationFinding> filter_vulnerability_findings(const std::vector<ValidationFinding>& findings) {
    std::vector<ValidationFinding> vulnerabilities;
    for (const ValidationFinding& finding : findings) {
        if (finding.kind == "vulnerability") {
            vulnerabilities.push_back(finding);
        }
    }
    return vulnerabilities;
}

InventoryScanResult scan_inventory_graph(const SecuritySettings& settings, Graph graph) {
    InventoryScanResult result;
    if (boost::num_vertices(graph) == 0) {
        result.error = "no packages in inventory";
        return result;
    }

    SecuritySettings scanSettings = settings;
    scanSettings.security.enabled = true;
    scanSettings.interaction.interactive = false;

    Validator validator(nullptr, scanSettings);
    result.findings = validator.audit(&graph);
    return result;
}

ScanResult run_inventory_scan(const SecuritySettings& settings, const std::string& inputJson) {
    ScanResult result;
    std::string parseError;
    const Graph graph = graph_from_inventory_json(inputJson, parseError);
    if (!parseError.empty()) {
        result.error = parseError;
        return result;
    }

    const InventoryScanResult scanned = scan_inventory_graph(settings, graph);
    if (scanned.error.has_value()) {
        result.error = scanned.error;
        return result;
    }
    result.json = findings_to_json(scanned.findings);
    return result;
}
