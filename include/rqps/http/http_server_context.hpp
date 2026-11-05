#pragma once

#include "rqps/domain/config_model.hpp"
#include "rqps/ports/i_process_runner.hpp"
#include "rqps/ports/i_scanner.hpp"

namespace rqps::http {

struct HttpServerContext {
    RqpsConfig config;
    IProcessRunner& processRunner;
    IVulnerabilityScanner& scanner;
};

}  // namespace rqps::http
