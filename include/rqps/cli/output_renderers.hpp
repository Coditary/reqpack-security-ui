#pragma once

#include "rqps/cli/i_output_renderer.hpp"
#include "rqps/cli/scan_output.h"

#include <memory>

namespace rqps::cli {

class TtyRenderer final : public IOutputRenderer {
public:
    int renderPolicy(const PolicyEvaluation& evaluation, std::ostream& output) const override;
};

class JsonRenderer final : public IOutputRenderer {
public:
    int renderPolicy(const PolicyEvaluation& evaluation, std::ostream& output) const override;
};

class QuietRenderer final : public IOutputRenderer {
public:
    int renderPolicy(const PolicyEvaluation& evaluation, std::ostream& output) const override;
};

std::unique_ptr<IOutputRenderer> make_output_renderer(const ScanOutputOptions& options);

}  // namespace rqps::cli
