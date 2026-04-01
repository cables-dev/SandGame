#include "common.hpp"
#include <cassert>

void StringChomper_Create(StringChomper* chomper, char* buffer) {
    chomper->ptr = buffer;
}

char* StringChomper_GetPointer(StringChomper* chomper) {
    return chomper->ptr;
}

void StringChomper_InsertTerminator(StringChomper* chomper) {
    *StringChomper_GetPointer(chomper) = '\0';
}

bool StringChomper_ReadStringLiteralIsTerminalChar(char c) {
    return c == '\"' || c == '\0';
}

void StringChomper_Step(StringChomper* chomp) {
    chomp->ptr++;
}

char StringChomper_Peek(const StringChomper* chomp) {
    return *chomp->ptr;
}

bool IsWhitespace(char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

void StringChomper_SkipWhitespace(StringChomper* chomp) {
    while (IsWhitespace(StringChomper_Peek(chomp))) 
        StringChomper_Step(chomp);
}

bool StringChomper_ReadStringLiteral(
    StringChomper* chomp,
    char** out_ptr,
    std::uint32_t* out_literal_size
) {
    if (StringChomper_Peek(chomp) != '\"')
        return false;

    StringChomper_Step(chomp);
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
    if (out_ptr)
		*out_ptr = (char*)l;
    if (out_literal_size)
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
    StringChomper_InsertTerminator(chomp);
    StringChomper_Step(chomp);
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
    auto result = strtof(number_string, &parse_ptr);
    if (parse_ptr != start) {
        if (out_num)
            *out_num = result;
        return true;
    }
    return false;
}
