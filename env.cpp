#include "env.h"

namespace mal {

std::shared_ptr<MalType> Env::find(const MalType& symbol) const {
    if (symbol.asString().size() != 1){
        return std::make_shared<MalError>("Error: unknown operation");
    }
    return std::make_shared<MalOp>(symbol.asString()[0]);
}
} // namespace mal
