#include "serialise.hpp"

// Let's collect all the information we need up-front so we don't
// have to scatter direct accesses through code.
struct SerialseBillOfMaterials {
    // header

};

bool Serialise_SerialiseGame(
    const SandGame* game, 
    char* out_buffer, 
    std::uint32_t buffer_size
) {
    return false;
}
