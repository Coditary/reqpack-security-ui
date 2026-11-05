#include "rqps/application/doctor_workflow.hpp"

#include "rqps/local_status.h"
#include "rqps/syft_discover_service.h"

namespace rqps {

DoctorReport build_doctor_report(const RqpsConfig& config, IProcessRunner& processRunner) {
    DoctorReport report;
    report.syftPath = config.discover.syftExecutable;
    report.syftOk = syft_is_available(config.discover.syftExecutable, processRunner);

    const LocalStatusReport status = build_local_status(config.settings);
    report.lines = status.lines;
    report.warnings = status.warnings;
    report.hasBlockingIssues = status.hasBlockingIssues || !report.syftOk;
    return report;
}

}  // namespace rqps
