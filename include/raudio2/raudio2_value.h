#ifndef RAUDIO2_VALUE_H
#define RAUDIO2_VALUE_H

#include <stdint.h>

typedef enum
{
    RAUDIO2_VALUE_NONE,
    RAUDIO2_VALUE_INT64,
    RAUDIO2_VALUE_DOUBLE,
    RAUDIO2_VALUE_STRING,
    RAUDIO2_VALUE_POINTER,
    RAUDIO2_VALUE_COUNT
} RAudio2_ValueType;

typedef struct RAudio2_Value {
    union {
        int64_t num;
        double numf;
        const char* str;
        void* ptr;
    } value;
    uint32_t size;
    int32_t type;

#ifdef __cplusplus
    RAudio2_Value() : size(0), type(RAUDIO2_VALUE_NONE)
    {
    }
#endif
} RAudio2_Value;

#endif // RAUDIO2_VALUE_H
