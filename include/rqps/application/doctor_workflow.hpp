#pragma once

#include "rqps/domain/config_model.hpp"
#include "rqps/domain/models.hpp"
#include "rqps/ports/i_process_runner.hpp"

namespace rqps {

DoctorReport build_doctor_report(const RqpsConfig& config, IProcessRunner& processRunner);

}  // namespace rqps
