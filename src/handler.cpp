#include <TMBEL/handler.hpp>

namespace ec {

////////////////////////////////////////////////////////////
// HandlerBase implementation
////////////////////////////////////////////////////////////

HandlerBase::HandlerBase() = default;

HandlerBase::HandlerBase(Container* container) : Base(container) {}

HandlerBase::HandlerBase(Position position, Container* container) : Base(position, container) {}

void HandlerBase::onRemove() {}

}  // namespace ec