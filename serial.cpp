#include "serial.hpp"

// All the deserialising code is digusting but I cba to think of another
// way of doing it :3 *smiles* *nozzles you*

struct StringChomper {
    char* ptr;
    int index;
};

void StringChomper_Create(StringChomper* chomper, char* buffer) {
    chomper->ptr = buffer;
    chomper->index = 0;
}

char* StringChomper_GetPointer(StringChomper* chomper) {
    return &chomper->ptr[chomper->index];
}

void StringChomper_InsertTerminator(StringChomper* chomper) {
    *StringChomper_GetPointer(chomper) = '\0';
}

bool StringChomper_ReadStringLiteralIsTerminalChar(char c) {
    return c == '\"' || c == '\0';
}

void StringChomper_Step(StringChomper* chomp) {
    chomp->index++;
}

char StringChomper_Peek(const StringChomper* chomp) {
    return chomp->ptr[chomp->index];
}

bool StringChomper_IsWhitespace(char c) {
    return c == ' ' || c == '\r' || c == '\n';
}

void StringChomper_SkipWhitespace(StringChomper* chomp) {
    while (StringChomper_IsWhitespace(StringChomper_Peek(chomp))) 
        StringChomper_Step(chomp);
}

bool StringChomper_ReadStringLiteral(
    StringChomper* chomp,
    char** out_ptr,
    std::uint32_t* out_literal_size
) {
    auto* l = StringChomper_GetPointer(chomp);
    auto i = 0;
    while (!StringChomper_ReadStringLiteralIsTerminalChar(StringChomper_Peek(chomp))) {
        ++i;
        StringChomper_Step(chomp);
    }

    auto terminated = StringChomper_Peek(chomp) == '\"';
    if (terminated) {
        StringChomper_InsertTerminator(chomp);
        StringChomper_Step(chomp);
    }
    *out_ptr = (char*)l;
    *out_literal_size = i;
    return terminated;
}

bool StringChomper_ReadStringIsTerminalChar(char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\0';
}

void StringChomper_ReadString(
    StringChomper* chomp, 
    char** out_ptr, 
    std::uint32_t* out_string_size
) {
    auto* l = StringChomper_GetPointer(chomp);
    auto i = 0;
    while (!StringChomper_ReadStringIsTerminalChar(StringChomper_Peek(chomp))) {
        i++;
        StringChomper_Step(chomp);
    }

    if (out_ptr)
		*out_ptr = (char*)l;
    if (out_string_size)
        *out_string_size = i;
}

// Returns whether or not the string was converted into an integer...
bool StringChomper_ReadInteger(
    StringChomper* chomp, 
    int* out_num
) {
    auto* start = StringChomper_GetPointer(chomp);
    char* number_string;
    StringChomper_ReadString(chomp, &number_string, nullptr);

    char* parse_ptr;
    auto result = strtol(number_string, &parse_ptr, 10);
    if (parse_ptr != start) {
        if (out_num)
            *out_num = result;
        return true;
    }
    return false;
}

// Returns whether or not the string was converted into an integer.
bool StringChomper_ReadUnsignedInteger(
    StringChomper* chomp, 
    std::uint32_t* out_num
) {
    auto* start = StringChomper_GetPointer(chomp);
    char* number_string;
    StringChomper_ReadString(chomp, &number_string, nullptr);

    char* parse_ptr;
    auto result = strtoul(number_string, &parse_ptr, 10);
    if (parse_ptr != start) {
        if (out_num)
            *out_num = result;
        return true;
    }
    return false;
}

bool StringChomper_ReadBoolean(
    StringChomper* chomp, 
    bool* out_b
) {
    int b;
    auto result = StringChomper_ReadInteger(chomp, &b);
    if (!result)
        return result;
    if (b != 0 && b != 1)
        return false;
    *out_b = b;
    return true;
}


bool StringChomper_ReadColour(
    StringChomper* chomp,
    GameColour* out_colour
) {
    auto r = 0u;
    auto g = 0u;
    auto b = 0u;
    auto a = 0u;

    if (!StringChomper_ReadUnsignedInteger(chomp, &r)) return false;
    if (!StringChomper_ReadUnsignedInteger(chomp, &g)) return false;
    if (!StringChomper_ReadUnsignedInteger(chomp, &b)) return false;
    if (!StringChomper_ReadUnsignedInteger(chomp, &a)) return false;

    out_colour->r = (std::uint8_t)(r & 0xff);
    out_colour->g = (std::uint8_t)(g & 0xff);
    out_colour->b = (std::uint8_t)(b & 0xff);
    out_colour->a = (std::uint8_t)(a & 0xff);
    return true;
}

// Returns whether or not the string was converted into an integer.
bool StringChomper_ReadDouble(
    StringChomper* chomp, 
    double* out_num
) {
    auto* start = StringChomper_GetPointer(chomp);
    char* number_string;
    StringChomper_ReadString(chomp, &number_string, nullptr);

    char* parse_ptr;
    auto result = strtod(number_string, &parse_ptr);
    if (parse_ptr != start) {
        if (out_num)
            *out_num = result;
        return true;
    }
    return false;
}

int Serial_TrySerialiseInteger(int num, char** p_buffer, int remaining_buffer_size) {
    if (remaining_buffer_size <= 0)
        return remaining_buffer_size;

    auto* buffer = *p_buffer;
    auto text_size = snprintf(buffer, remaining_buffer_size, "%d", num);
    auto result = remaining_buffer_size - (text_size + 1);		// + 1 for null terminator
    if (text_size <= remaining_buffer_size) {
        buffer[text_size] = ' ';                            // Overwrite null terminator
        *p_buffer += text_size + 1;                         // + 1 to skip null terminator
    }
    return result;
}

int Serial_TrySerialiseStringLiteral(const char* literal, char** p_buffer, int remaining_buffer_size) {
    if (remaining_buffer_size <= 0)
        return remaining_buffer_size;

    auto* buffer = *p_buffer;
    auto text_size = snprintf(buffer, remaining_buffer_size, "\"%s\"", literal);
    auto result = remaining_buffer_size - (text_size + 1);		// + 1 for null terminator
    if (text_size <= remaining_buffer_size) {
        buffer[text_size] = ' ';                            // Overwrite null terminator
        *p_buffer += text_size + 1;
    }
    return result;
}

int Serial_TrySerialiseDouble(double num, char** p_buffer, int remaining_buffer_size) {
    if (remaining_buffer_size <= 0)
        return remaining_buffer_size;

    auto* buffer = *p_buffer;
    auto text_size = snprintf(buffer, remaining_buffer_size, "%f", num);
    auto result = remaining_buffer_size - (text_size + 1);		// + 1 for null terminator
    if (text_size <= remaining_buffer_size) {
        buffer[text_size] = ' ';                            // Overwrite null terminator
        *p_buffer += text_size + 1;
    }
    return result;
}

int Serial_TrySerialiseUnsignedInt(std::uint32_t num, char** p_buffer, int remaining_buffer_size) {
    if (remaining_buffer_size <= 0)
        return remaining_buffer_size;

    auto* buffer = *p_buffer;
    auto text_size = snprintf(buffer, remaining_buffer_size, "%u", num);
    auto result = remaining_buffer_size - (text_size + 1);		// + 1 for null terminator
    if (text_size <= remaining_buffer_size) {
		buffer[text_size] = ' ';                            // Overwrite null terminator
		*p_buffer += text_size + 1;
	}
    return result;
}

int Serial_TrySerialiseColour(const GameColour* c, char** p_buffer, int remaining_buffer_size) {
    remaining_buffer_size = Serial_TrySerialiseUnsignedInt(c->r, p_buffer, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseUnsignedInt(c->g, p_buffer, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseUnsignedInt(c->b, p_buffer, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseUnsignedInt(c->a, p_buffer, remaining_buffer_size);
    return remaining_buffer_size;
}

int Serial_TrySerialiseBoolean(bool val, char** p_buffer, int remaining_buffer_size) {
    return Serial_TrySerialiseInteger(val, p_buffer, remaining_buffer_size);
}

int Serial_TrySerialiseAudioResource(AudioResource rsc, char** p_buffer, int remaining_buffer_size) {
    return Serial_TrySerialiseInteger((int)rsc, p_buffer, remaining_buffer_size);
}

int Serial_TrySerialiseGraphicResource(GraphicResource rsc, char** p_buffer, int remaining_buffer_size) {
    return Serial_TrySerialiseInteger((int)rsc, p_buffer, remaining_buffer_size);
}

int Serial_TryNewline(char** p_buffer, int remaining_buffer_size) {
    if (remaining_buffer_size <= 1)
        return remaining_buffer_size - 2;

    **p_buffer = '\r';
    *(*p_buffer + 1) = '\n';
    *p_buffer += 2;
    return remaining_buffer_size - 2;
}

// [top_left_x:double] [top_left_y:double] [w:double] [h:double] [r:int] [g:int] [b:int] [a:int]
int Serial_SerialiseRectangleObstacle(
    const EntityRectangleObstacle* rect,
    char** out_serial,
    int remaining_buffer_size
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

    AABB_GetCornerCoords(&rect->aabb, AABB_TOP_LEFT, &top_left_x, &top_left_y);
    w = AABB_GetWidth(&rect->aabb);
    h = AABB_GetHeight(&rect->aabb);
    c = rect->colour;
    
    remaining_buffer_size = Serial_TrySerialiseDouble(top_left_x, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(top_left_y, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(w, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(h, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseColour(&c, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TryNewline(out_serial, remaining_buffer_size);
    return remaining_buffer_size;
}

bool Serial_DeserialiseRectangleObstacle(EntityRectangleObstacle* out_rect, char** serial, DeserialiseError* out_error) {
    double top_left_x{};
    double top_left_y{};
    double w{};
    double h{};
    bool result{};
    GameColour c{};
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

    RectangleObstacle_Create(out_rect, top_left_x, top_left_y, w, h, c);
    return true;
}

int Serial_SerialiseHintBox(const EntityHintBox* hint, char** out_serial, int remaining_buffer_size) {
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

    remaining_buffer_size = Serial_TrySerialiseDouble(top_left_x, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(top_left_y, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(w, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(h, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseStringLiteral(message, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseBoolean(only_once, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseAudioResource(rsc, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TryNewline(out_serial, remaining_buffer_size);
    return remaining_buffer_size;
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
    return true;
}

int Serial_SerialiseBarrel(const EntityBarrel* barrel, char** out_serial, int remaining_buffer_size) {
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

	remaining_buffer_size = Serial_TrySerialiseDouble(bottom_x, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(bottom_y, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(w, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(h, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseGraphicResource(idle, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseGraphicResource(explode, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TryNewline(out_serial, remaining_buffer_size);
	return remaining_buffer_size;
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
    return true;
}

int Serial_SerialiseLevelDoor(const EntityLevelDoor* door, char** out_serial, int remaining_buffer_size) {
    double top_left_x{};
    double top_left_y{};
    double w{};
    double h{};
    const char* next_level_path{};
    int lock_flags{};
    int unlock_flags{};

    AABB_GetCornerCoords(&door->aabb, AABB_TOP_LEFT, &top_left_x, &top_left_y);
    w = AABB_GetWidth(&door->aabb);
    h = AABB_GetHeight(&door->aabb);
    next_level_path = door->next_level_path;
    lock_flags = door->lock_flag;
    unlock_flags = door->unlock_flag;

	remaining_buffer_size = Serial_TrySerialiseDouble(top_left_x, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(top_left_y, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(w, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(h, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseStringLiteral(next_level_path, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseInteger(lock_flags, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseInteger(unlock_flags, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TryNewline(out_serial, remaining_buffer_size);
    return remaining_buffer_size;
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
    
    LevelDoor_Create(out_door, top_left_x, top_left_y, w, h, next_level_path, lock_flags, unlock_flags);
    return true;
}

int Serial_SerialiseLadybird(const EntityLadybird* ladybird, char** out_serial, int remaining_buffer_size) {
    double feet_x{};
    double feet_y{};

    AABB_GetCornerCoords(&ladybird->aabb, AABB_BOTTOM, &feet_x, &feet_y);

	remaining_buffer_size = Serial_TrySerialiseDouble(feet_x, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TrySerialiseDouble(feet_y, out_serial, remaining_buffer_size);
    remaining_buffer_size = Serial_TryNewline(out_serial, remaining_buffer_size);
    return remaining_buffer_size;
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
    return true;
}


