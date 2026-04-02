#include "serial.hpp"
#include <stdarg.h>
#include <cassert>

// All the deserialising code is digusting but I cba to think of another
// way of doing it :3 *smiles* *nozzles you*

bool Serial_IsWhitespace(char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}


void Serial_AppendSnprintfToStdString(std::string& buffer, const char* format, ...) {
    constexpr static auto provisional_buffer_size = 1000;
    static char provisional_buffer[provisional_buffer_size ]{};
    va_list args;

    va_start(args, format);

    // snprintf into local buffer.
    auto snprintf_num_chars = vsnprintf(provisional_buffer, provisional_buffer_size, format, args);

    // Do we need to allocate more space?
    if (snprintf_num_chars >= provisional_buffer_size) {
        auto heap_buffer_size = snprintf_num_chars + 10;     // + 10 headroom just for safety
        auto* formatted_output_heap_buffer = (char*)malloc(heap_buffer_size);

        // No need to memset to 0 here, vsnprintf appends a null terminator for us.
		auto ammended_snprintf_num_chars = vsnprintf(provisional_buffer, provisional_buffer_size, format, args);
        assert(ammended_snprintf_num_chars < heap_buffer_size);

        // Copy result from local buffer
        buffer.append(formatted_output_heap_buffer);
    }
    else {
        // Copy result from local buffer
        buffer.append(provisional_buffer);
    }

    va_end(args);
}

void Serial_TrySerialiseInteger(int num, std::string& buffer) {
    Serial_AppendSnprintfToStdString(buffer, "%d ", num);
}

void Serial_TrySerialiseStringLiteral(const char* literal, std::string& buffer) {
    Serial_AppendSnprintfToStdString(buffer, "\"%s\" ", literal);
}

void Serial_TrySerialiseStringRaw(const char* literal, std::string& buffer) {
    Serial_AppendSnprintfToStdString(buffer, "%s ", literal);
}

void Serial_TrySerialiseDouble(double num, std::string& buffer) {
    Serial_AppendSnprintfToStdString(buffer, "%f ", num);
}

void Serial_TrySerialiseUnsignedInteger(std::uint32_t num, std::string& buffer) {
    Serial_AppendSnprintfToStdString(buffer, "%u ", num);
}

void Serial_TrySerialiseColour(const GameColour* c, std::string& buffer) {
    Serial_TrySerialiseUnsignedInteger(c->r, buffer);
    Serial_TrySerialiseUnsignedInteger(c->g, buffer);
    Serial_TrySerialiseUnsignedInteger(c->b, buffer);
    Serial_TrySerialiseUnsignedInteger(c->a, buffer);
}

void Serial_TrySerialiseBoolean(bool val, std::string& buffer) {
    Serial_TrySerialiseInteger(val, buffer);
}

void Serial_TrySerialiseAudioResource(AudioResource rsc, std::string& buffer) {
    Serial_TrySerialiseInteger((int)rsc, buffer);
}

void Serial_TrySerialiseGraphicResource(GraphicResource rsc, std::string& buffer) {
    Serial_TrySerialiseInteger((int)rsc, buffer);
}

void Serial_TryNewline(std::string& buffer) {
    if (Serial_IsWhitespace(buffer.back())) {
        buffer.pop_back();
    }
	buffer.append("\r\n");
}

// [top_left_x:double] [top_left_y:double] [w:double] [h:double] [r:int] [g:int] [b:int] [a:int]
void Serial_SerialiseRectangleObstacle(
    const EntityRectangleObstacle* rect,
    std::string& out_serial
) {
    double top_left_x{};
    double top_left_y{};
    double w{};
    double h{};
    GameColour c{};
    std::uint8_t r{};
    std::uint8_t g{};
    std::uint8_t b{};
    std::uint8_t a{};
    GraphicResource rsc{};

    AABB_GetCornerCoords(&rect->aabb, AABB_TOP_LEFT, &top_left_x, &top_left_y);
    w = AABB_GetWidth(&rect->aabb);
    h = AABB_GetHeight(&rect->aabb);
    c = rect->colour;
    rsc = rect->graphic;
    
    Serial_TrySerialiseDouble(top_left_x, out_serial);
    Serial_TrySerialiseDouble(top_left_y, out_serial);
    Serial_TrySerialiseDouble(w, out_serial);
    Serial_TrySerialiseDouble(h, out_serial);
    Serial_TrySerialiseColour(&c, out_serial);
    Serial_TrySerialiseGraphicResource(rsc, out_serial);
}

bool Serial_DeserialiseRectangleObstacle(EntityRectangleObstacle* out_rect, char** serial, DeserialiseError* out_error) {
    double top_left_x{};
    double top_left_y{};
    double w{};
    double h{};
    bool result{};
    GameColour c{};
    GraphicResource rsc{};
    StringChomper chomp;

    StringChomper_Create(&chomp, *serial);
    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &top_left_x);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read top_left_x for RectangleObstacle!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &top_left_y);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read top_left_y for RectangleObstacle!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &w);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read width for RectangleObstacle!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &h);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read height for RectangleObstacle!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadColour(&chomp, &c);
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read colour for RectangleObstacle!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadInteger(&chomp, (int*)&rsc);
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read colour for RectangleObstacle!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    RectangleObstacle_Create(out_rect, top_left_x, top_left_y, w, h, c, rsc);
    *serial = StringChomper_GetPointer(&chomp);
    return true;
}

void Serial_SerialiseHintBox(const EntityHintBox* hint, std::string& out_serial) {
    double top_left_x{};
    double top_left_y{};
    double w{};
    double h{};
    const char* message{};
    bool only_once{};
    AudioResource rsc{};

    AABB_GetCornerCoords(&hint->aabb, AABB_TOP_LEFT, &top_left_x, &top_left_y);
    w = AABB_GetWidth(&hint->aabb);
    h = AABB_GetHeight(&hint->aabb);
    message = hint->message;
    only_once = hint->only_once;
    rsc = hint->audio_rsc;

    Serial_TrySerialiseDouble(top_left_x, out_serial);
    Serial_TrySerialiseDouble(top_left_y, out_serial);
    Serial_TrySerialiseDouble(w, out_serial);
    Serial_TrySerialiseDouble(h, out_serial);
    Serial_TrySerialiseStringLiteral(message, out_serial);
    Serial_TrySerialiseBoolean(only_once, out_serial);
    Serial_TrySerialiseAudioResource(rsc, out_serial);
}

bool Serial_DeserialiseHintBox(EntityHintBox* out_hint, char** serial, DeserialiseError* out_error) {
    double top_left_x{};
    double top_left_y{};
    double w{};
    double h{};
    char* message{};
    bool result{};
    bool only_once{};
    AudioResource rsc{};
    StringChomper chomp{};

    StringChomper_Create(&chomp, *serial);
    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &top_left_x);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read top_left_x for Hint Box!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &top_left_y);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read top_left_y for Hint Box!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &w);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read width for Hint Box!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &h);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read height for Hint Box!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadStringLiteral(&chomp, &message, nullptr);
    if (!result) {
        *out_error = DeserialiseError{ "Could not read message for Hint Box!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadBoolean(&chomp, &only_once);
    if (!result) {
        *out_error = DeserialiseError{ "Could not read only_once for Hint Box!" , StringChomper_GetPointer(&chomp) }; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadInteger(&chomp, &rsc);
    if (!result) {
        *out_error = DeserialiseError{ "Could not read audio resource for Hint Box!", StringChomper_GetPointer(&chomp) };
        return false;
    }

    HintBox_Create(out_hint, message, only_once, top_left_x, top_left_y, w, h);
    *serial = StringChomper_GetPointer(&chomp);
    return true;
}

void Serial_SerialiseBarrel(const EntityBarrel* barrel, std::string& out_serial) {
    double bottom_x{};
    double bottom_y{};
    double w{};
    double h{};
    GraphicResource idle{};
    GraphicResource explode{};

    AABB_GetCornerCoords(&barrel->aabb, AABB_BOTTOM, &bottom_x, &bottom_y);
    w = AABB_GetWidth(&barrel->aabb);
    h = AABB_GetHeight(&barrel->aabb);
    idle = barrel->idle_sprite;
    explode = barrel->explode_sprite;

    Serial_TrySerialiseDouble(bottom_x, out_serial);
    Serial_TrySerialiseDouble(bottom_y, out_serial);
    Serial_TrySerialiseDouble(w, out_serial);
    Serial_TrySerialiseDouble(h, out_serial);
    Serial_TrySerialiseGraphicResource(idle, out_serial);
    Serial_TrySerialiseGraphicResource(explode, out_serial);
}

bool Serial_DeserialiseBarrel(EntityBarrel* out_barrel, char** serial, DeserialiseError* out_error) {
    double bottom_x{};
    double bottom_y{};
    double w{};
    double h{};
    bool result{};
    GraphicResource idle{};
    GraphicResource explode{};
	StringChomper chomp{};

    StringChomper_Create(&chomp, *serial);
    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &bottom_x);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read bottom_x for Barrel!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &bottom_y);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read bottom_y for Barrel!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &w);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read width for Barrel!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &h);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read height for Barrel!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadInteger(&chomp, &idle);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read idle graphic resource for Barrel!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadInteger(&chomp, &explode);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read explode graphic resource for Barrel!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    Barrel_Create(out_barrel, bottom_x, bottom_y, w, h, idle, explode);
    *serial = StringChomper_GetPointer(&chomp);
    return true;
}

void Serial_SerialiseLevelDoor(const EntityLevelDoor* door, std::string& out_serial) {
    double top_left_x{};
    double top_left_y{};
    double w{};
    double h{};
    const char* next_level_path{};
    int lock_flags{};
    int unlock_flags{};
    GraphicResource rsc{};

    AABB_GetCornerCoords(&door->aabb, AABB_TOP_LEFT, &top_left_x, &top_left_y);
    w = AABB_GetWidth(&door->aabb);
    h = AABB_GetHeight(&door->aabb);
    next_level_path = door->next_level_path;
    lock_flags = door->lock_flag;
    unlock_flags = door->unlock_flag;
    rsc = door->sprite;

    Serial_TrySerialiseDouble(top_left_x, out_serial);
    Serial_TrySerialiseDouble(top_left_y, out_serial);
    Serial_TrySerialiseDouble(w, out_serial);
    Serial_TrySerialiseDouble(h, out_serial);
    Serial_TrySerialiseStringLiteral(next_level_path, out_serial);
    Serial_TrySerialiseInteger(lock_flags, out_serial);
    Serial_TrySerialiseInteger(unlock_flags, out_serial);
    Serial_TrySerialiseGraphicResource(rsc, out_serial);
}

bool Serial_DeserialiseLevelDoor(EntityLevelDoor* out_door, char** serial, DeserialiseError* out_error) {
	double top_left_x{};
    double top_left_y{};
    double w{};
    double h{};
    char* next_level_path{};
    int lock_flags{};
    int unlock_flags{};
    bool result{};
    GraphicResource rsc{};
	StringChomper chomp{};

    StringChomper_Create(&chomp, *serial);
    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &top_left_x);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read top_left_x for Level Door!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &top_left_y);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read top_left_y for Level Door!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &w);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read width for Level Door!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

	StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &h);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read height for Level Door!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadStringLiteral(&chomp, &next_level_path, nullptr);
    if (!result) {
        *out_error = DeserialiseError{ "Could not read next_level_path for Level Door!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadInteger(&chomp, &lock_flags);
    if (!result) {
        *out_error = DeserialiseError{ "Could not read lock_flags for Level Door!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadInteger(&chomp, &unlock_flags);
    if (!result) {
        *out_error = DeserialiseError{ "Could not read lock_flags for Level Door!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadInteger(&chomp, (GraphicResource*)&rsc);
    if (!result) {
        *out_error = DeserialiseError{ "Could not read sprite_resource for Level Door!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }
    
    LevelDoor_Create(out_door, top_left_x, top_left_y, w, h, next_level_path, rsc, lock_flags, unlock_flags);
    *serial = StringChomper_GetPointer(&chomp);
    return true;
}

void Serial_SerialiseLadybird(const EntityLadybird* ladybird, std::string& out_serial) {
    double feet_x{};
    double feet_y{};

    AABB_GetCornerCoords(&ladybird->aabb, AABB_BOTTOM, &feet_x, &feet_y);

    Serial_TrySerialiseDouble(feet_x, out_serial);
    Serial_TrySerialiseDouble(feet_y, out_serial);
}

bool Serial_DeserialiseLadybird(EntityLadybird* out_ladybird, char** serial, DeserialiseError* out_error) {
    double bottom_x{};
    double bottom_y{};
    bool result{};
	StringChomper chomp{};

    StringChomper_Create(&chomp, *serial);
    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &bottom_x);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read bottom_x for Ladybird!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    StringChomper_SkipWhitespace(&chomp);
    result = StringChomper_ReadDouble(&chomp, &bottom_y);  
    if (!result) { 
        *out_error = DeserialiseError{ "Could not read bottom_y for Ladybird!", StringChomper_GetPointer(&chomp)}; 
        return false;
    }

    Ladybird_Create(out_ladybird, bottom_x, bottom_y);
    *serial = StringChomper_GetPointer(&chomp);
    return true;
}


