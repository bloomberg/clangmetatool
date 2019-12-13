#include "value_context_ordering.h"

namespace clangmetatool {
namespace propagation {
namespace types {

void ValueContextOrdering::print(std::ostream &stream, Value value) {
  switch (value) {
  case ValueContextOrdering::CONTROL_FLOW_MERGE:
    stream << "Control flow merge";
    break;
  case ValueContextOrdering::CHANGED_BY_CODE:
    stream << "Changed by code";
    break;
  default:
    break;
  }
}

std::ostream &operator<<(std::ostream &stream,
                         ValueContextOrdering::Value value) {
  ValueContextOrdering::print(stream, value);
  return stream;
}
} // namespace types
} // namespace propagation
} // namespace clangmetatool
