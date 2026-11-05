#pragma once

#include "rqps/domain/models.hpp"

#include <iosfwd>

namespace rqps::cli {

class IOutputRenderer {
public:
    virtual ~IOutputRenderer() = default;

    virtual int renderPolicy(const PolicyEvaluation& evaluation, std::ostream& output) const = 0;
};

}  // namespace rqps::cli
