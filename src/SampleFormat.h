#pragma once

#include "raudio2/raudio2_common.h"
#include "miniaudio.h"

constexpr ma_format GetMiniAudioFormat(RAudio2_SampleFormat format) noexcept
{
    switch (format)
    {
    default:
    case RAUDIO2_SAMPLE_FORMAT_UNKNOWN:
        return ma_format_unknown;
    case RAUDIO2_SAMPLE_FORMAT_U8:
        return ma_format_u8;
    case RAUDIO2_SAMPLE_FORMAT_S16:
        return ma_format_s16;
    case RAUDIO2_SAMPLE_FORMAT_S24:
        return ma_format_unknown;
    case RAUDIO2_SAMPLE_FORMAT_S32:
        return ma_format_s32;
    case RAUDIO2_SAMPLE_FORMAT_F32:
        return ma_format_f32;
    case RAUDIO2_SAMPLE_FORMAT_COUNT:
        return ma_format_count;
    }
}
