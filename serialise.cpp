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
    NEEDS_FREE const Entity* game_entities{};                // Deepcopy. Preserves game order. Iterate using indicies, not internal list pointers.
    std::uint32_t num_entities{};
};

void Serialise_FillInBOMCopyGameEntities(SerialiseBillOfMaterials* out_bom, const SandGame* game) {
    static Entity* deepcopy;
    static int deepcopy_i;
    auto num_entities = SandGame_GetNumEntities(game);
    if (num_entities <= 0)
        return;
    deepcopy = (Entity*)(malloc(sizeof(Entity) * num_entities ));
    deepcopy_i = 0;
    auto cb = [](Entity* ent) {
        deepcopy[deepcopy_i] = *ent;            // Copy element
        deepcopy[deepcopy_i]._next = nullptr;   // Clear internal list pointers since they are of no use to us.
        deepcopy[deepcopy_i]._prev = nullptr;   
        deepcopy_i++;
	};
    SandGame_ForEachEntity(game, cb);

    out_bom->game_entities = deepcopy;
    out_bom->num_entities = num_entities;
}

void Serialise_FillInBOM(SerialiseBillOfMaterials* out_bom, const SandGame* game) {
    // header
    AABB_GetCornerCoords(&game->player.bbox, AABB_TOP_LEFT, &out_bom->player_start_x, &out_bom->player_start_y);
    out_bom->pit_screens_horizontal = game->pit.num_screens_horizontal;
    out_bom->pit_screens_vertical = game->pit.num_screens_vertical;
    out_bom->sand_stubbornness = game->pit.stubbornness;
    out_bom->do_time_tick = game->do_time_tick;

    // entities
    Serialise_FillInBOMCopyGameEntities(out_bom, game);
}

void Serialise_DestroyBOM(SerialiseBillOfMaterials* out_bom, const SandGame* game) {
    free((void*)out_bom->game_entities);                // De-const
}

void Serialise_SerialiseHeader(
    const SerialiseBillOfMaterials* bom,
    std::string& out_serial
) {
    Serial_TrySerialiseStringRaw(SERIALISE_HEADER_MAGIC, out_serial);
    Serial_TrySerialiseDouble(bom->player_start_x, out_serial);
    Serial_TrySerialiseDouble(bom->player_start_y, out_serial);
    Serial_TrySerialiseUnsignedInteger(bom->pit_screens_horizontal, out_serial);
    Serial_TrySerialiseUnsignedInteger(bom->pit_screens_vertical, out_serial);
    Serial_TrySerialiseUnsignedInteger(bom->sand_stubbornness, out_serial);
    Serial_TrySerialiseBoolean(bom->do_time_tick, out_serial);
    Serial_TryNewline(out_serial);
}

void Serialise_SerialiseEntityHeader(
    const Entity* ent,
    std::string& out_serial
) {
    Serial_TrySerialiseStringRaw(SERIALISE_ENTITY_DEFINITION_COMMAND, out_serial);
    Serial_TrySerialiseUnsignedInteger(ent->type, out_serial);
}

void Serialise_SerialiseEntity(
    const Entity* ent, 
    std::string& out_serial
) {
    Serialise_SerialiseEntityHeader(ent, out_serial);

    switch (ent->type) {
    case ENTITY_RECTANGLE: { Serial_SerialiseRectangleObstacle(&ent->entity.rect, out_serial); break; }
    case ENTITY_HINT_BOX: { Serial_SerialiseHintBox(&ent->entity.hint_box, out_serial); break; }
    case ENTITY_BARREL: { Serial_SerialiseBarrel(&ent->entity.barrel, out_serial); break; }
    case ENTITY_LEVEL_DOOR: { Serial_SerialiseLevelDoor(&ent->entity.door, out_serial); break; }
    case ENTITY_LADYBIRD: { Serial_SerialiseLadybird(&ent->entity.ladybird, out_serial); break; }
    }

    Serial_TryNewline(out_serial);
}

void Serialise_SerialiseEntities(
    const SerialiseBillOfMaterials* bom,
    std::string& out_serial
) {
    for (int i = 0; i < bom->num_entities; i++) {
        auto* ent = &bom->game_entities[i];
        Serialise_SerialiseEntity(ent, out_serial);
    }
}

void Serialise_CopyStdStringToBuffer(
    const std::string& buffer,
    NEEDS_FREE char** out_buffer,
    std::uint32_t* out_buffer_size
) {
    auto string_size = (buffer.size() + 1);               // + 1 for null terminator (assuming 1-byte-width chars)
    auto local_buffer_size = sizeof(char) * string_size;
    char* result = (char*)malloc(local_buffer_size);
    memcpy_s(result, local_buffer_size, buffer.c_str(), string_size);
    *out_buffer = result;
    *out_buffer_size = buffer.size();
}

void Serialise_SerialiseBOM(
    const SerialiseBillOfMaterials* bom, 
    NEEDS_FREE char** out_buffer, 
    std::uint32_t* out_buffer_size
) {
    std::string buffer{};

    Serialise_SerialiseHeader(bom, buffer);
    Serialise_SerialiseEntities(bom, buffer);
    Serialise_CopyStdStringToBuffer(buffer, out_buffer, out_buffer_size);
}

bool Serialise_SerialiseGame(
    const SandGame* game, 
    NEEDS_FREE char** out_buffer,
    std::uint32_t* out_buffer_size
) {
    SerialiseBillOfMaterials bom{};
    Serialise_FillInBOM(&bom, game);
    Serialise_SerialiseBOM(&bom, out_buffer, out_buffer_size);

    return true;
}
