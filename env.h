#pragma once

#include "maltypes.h"

namespace mal {

class Env {
public:
    Env() = default;
    std::shared_ptr<MalType> find(const MalType& symbol) const;
};

} // namespace mal
