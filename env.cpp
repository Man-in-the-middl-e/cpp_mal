#include "env.h"

namespace mal {

std::unique_ptr<MalType> Env::find(const MalType& symbol) const {
    if (symbol.asString().size() != 1){
        return std::make_unique<MalError>("Error: unknown operation");
    }
    return std::make_unique<MalOp>(symbol.asString()[0]);
}
} // namespace mal
