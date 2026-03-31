#include "serialise.hpp"

// Let's collect all the information we need up-front so we don't
// have to scatter direct accesses through code.
struct SerialiseBillOfMaterials {
    // header
    double player_start_x{};
    double player_start_y{};
    std::uint32_t pit_screens_horizontal{};
    std::uint32_t pit_screens_vertical{};
    std::uint32_t sand_stubbornness{};
    bool do_time_tick{};
    // sounds?
    // graphics?
    // entities
    Entity* game_entities{};                // Watch out! Shallow copy
    std::uint32_t num_entities{};
};

void Serialise_CreateBOM(SerialiseBillOfMaterials* out_bom, const SandGame* game) {
    // header
    AABB_GetCornerCoords(&game->player.bbox, AABB_TOP_LEFT, &out_bom->player_start_x, &out_bom->player_start_y);
    out_bom->pit_screens_horizontal = game->pit.num_screens_horizontal;
    out_bom->pit_screens_vertical = game->pit.num_screens_vertical;
    out_bom->sand_stubbornness = game->pit.stubbornness;
    out_bom->do_time_tick = game->do_time_tick;

    // entities
    out_bom->game_entities = game->entities;
    out_bom->num_entities = SandGame_GetNumEntities(game);
}

bool Serialise_SerialiseGame(
    const SandGame* game, 
    NEEDS_FREE const char** out_buffer,
    std::uint32_t* out_buffer_size
) {
    return false;
}
