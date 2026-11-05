#include "rqps/cli/scan_output.h"

#include "rqps/application/policy_evaluator.hpp"
#include "rqps/cli/output_renderers.hpp"

int print_policy_evaluation(const rqps::PolicyEvaluation& evaluation, const ScanOutputOptions& options,
                            std::ostream& output) {
    const std::unique_ptr<rqps::cli::IOutputRenderer> renderer = rqps::cli::make_output_renderer(options);
    return renderer->renderPolicy(evaluation, output);
}

int print_scan_results(const SecuritySettings& settings, const std::vector<ValidationFinding>& rawFindings,
                       const ScanOutputOptions& options, std::ostream& output) {
    const rqps::PolicyEvaluation evaluation = rqps::evaluate_policy(settings, rawFindings, options.failOnFindings);
    return print_policy_evaluation(evaluation, options, output);
}
