#include "object.h"
#include "chunk.h"

namespace aankaa {

ObjFunction::ObjFunction() {
    chunk = new Chunk();
    type = OBJ_FUNCTION;
}

ObjFunction::~ObjFunction() {
    delete chunk;
}

} // namespace