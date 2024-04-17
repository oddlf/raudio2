#include "raudio2_flac.h"
#include <array>
#include <FLAC/all.h>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include "raudio2/raudio2_waveinfo.hpp"

using namespace std::literals;

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return FLAC_MakeInputPlugin(plugin);
}
#endif

bool FLAC_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->open = FLAC_Open;
    plugin->seek = FLAC_Seek;
    plugin->read = FLAC_Read;
    plugin->close = FLAC_Close;
    plugin->getValue = FLAC_GetValue;

    return true;
}

static FLAC__StreamDecoderReadStatus FLAC_OnRead(const FLAC__StreamDecoder* decoder, FLAC__byte buffer[], size_t* bytes, void* client_data)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)client_data;
    if (*bytes > 0)
    {
        *bytes = file->read(file->handle, buffer, *bytes);
        if (*bytes == 0)
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        else
            return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }
    else
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}

static FLAC__StreamDecoderSeekStatus FLAC_OnSeek(const FLAC__StreamDecoder* decoder, FLAC__uint64 absolute_byte_offset, void* client_data)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)client_data;

    if (!file)
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;

    auto currPos = file->seek(file->handle, (int64_t)absolute_byte_offset, RAUDIO2_SEEK_SET);
    if (currPos < 0)
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;

    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

static FLAC__StreamDecoderTellStatus FLAC_OnTell(const FLAC__StreamDecoder* decoder, FLAC__uint64* absolute_byte_offset, void* client_data)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)client_data;

    if (!file)
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;

    auto currPos = file->tell(file->handle);
    if (currPos < 0)
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;

    *absolute_byte_offset = currPos;
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

static FLAC__StreamDecoderLengthStatus FLAC_OnLength(const FLAC__StreamDecoder* decoder, FLAC__uint64* stream_length, void* client_data)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)client_data;

    if (!file)
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;

    auto size = file->getSize(file->handle);
    if (size <= 0)
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;

    *stream_length = size;
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

static FLAC__bool FLAC_OnEof(const FLAC__StreamDecoder* decoder, void* client_data)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)client_data;

    return file->tell(file->handle) >= file->getSize(file->handle);
}

//  A function pointer matching this signature must be passed to one of
//  the FLAC__stream_decoder_init_*() functions.
//  The supplied function will be called when the decoder has decoded a
//  single audio frame. The decoder will pass the frame metadata as well
//  as an array of pointers (one for each channel) pointing to the
//  decoded audio.
//
// \note In general, FLAC__StreamDecoder functions which change the
// state should not be called on the decoder while in the callback.
//
// \param  decoder  The decoder instance calling the callback.
// \param  frame    The description of the decoded frame.  See
//                  FLAC__Frame.
// \param  buffer   An array of pointers to decoded channels of data.
//                  Each pointer will point to an array of signed
//                  samples of length frame->header.blocksize.
//                  Channels will be ordered according to the FLAC
//                  specification; see the documentation for the
//                  <A HREF="https://xiph.org/flac/format.html#frame_header">frame header</A>.
// \param  client_data  The callee's client data set through
//                      FLAC__stream_decoder_init_*().
// \retval FLAC__StreamDecoderWriteStatus
//    The callee's return status.
static FLAC__StreamDecoderWriteStatus FLAC_OnDecoderWrite(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data)
{
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
}

static void FLAC_OnMetadata(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data)
{
    //metadata->type
}

static void FLAC_OnError(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data)
{
}

bool FLAC_Open(RAudio2_WaveInfo* wave_)
{
    ra::WaveInfo wave = wave_;
    if (!wave.hasValidWave())
        return false;

    auto file = wave.getFile();

    if (!file)
        return false;

    char headerBytes[4]{};
    file.read(headerBytes, 4);

    bool isOgg = false;

    if (headerBytes[0] == 'O' && headerBytes[1] == 'g' && headerBytes[2] == 'g' && headerBytes[3] == 'S')
        isOgg = true;

    if (isOgg == false &&
        ((headerBytes[0] != 'f' && headerBytes[1] != 'L' && headerBytes[2] != 'a' && headerBytes[3] != 'C') ||
            (headerBytes[0] != 'F' && headerBytes[1] != 'L' && headerBytes[2] != 'A' && headerBytes[3] != 'C')))
        return false;

    file.seek(0);

    auto flacFile = FLAC__stream_decoder_new();

    if (!flacFile)
        return false;

    FLAC__stream_decoder_set_metadata_respond(flacFile, FLAC__METADATA_TYPE_STREAMINFO);
    FLAC__stream_decoder_set_metadata_respond(flacFile, FLAC__METADATA_TYPE_VORBIS_COMMENT);

    FLAC__StreamDecoderInitStatus ret{};
    if (isOgg)
        ret = FLAC__stream_decoder_init_ogg_stream(flacFile, FLAC_OnRead, FLAC_OnSeek, FLAC_OnTell,
            FLAC_OnLength, FLAC_OnEof, FLAC_OnDecoderWrite, FLAC_OnMetadata, FLAC_OnError, file.getVirtualIO());
    else
        ret = FLAC__stream_decoder_init_stream(flacFile, FLAC_OnRead, FLAC_OnSeek, FLAC_OnTell,
            FLAC_OnLength, FLAC_OnEof, FLAC_OnDecoderWrite, FLAC_OnMetadata, FLAC_OnError, file.getVirtualIO());

    if (ret == FLAC__STREAM_DECODER_INIT_STATUS_OK)
    {
        FLAC__stream_decoder_process_until_end_of_metadata(flacFile);
        FLAC__stream_decoder_process_single(flacFile);

        if (FLAC__stream_decoder_get_bits_per_sample(flacFile) <= 16)
            wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_S16);
        else
            wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_F32);

        wave.setChannels(FLAC__stream_decoder_get_channels(flacFile));
        wave.setSampleRate(FLAC__stream_decoder_get_sample_rate(flacFile));
        wave.setFrameCount(FLAC__stream_decoder_get_total_samples(flacFile));

        wave.setCtxData(flacFile);

        return true;
    }
    return false;
}

bool FLAC_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames)
{
    if (!wave)
        return false;
    if (!wave->file)
        return false;
    if (!wave->file->handle)
        return false;
    if (!wave->ctxData)
        return false;

    auto flacFile = (FLAC__StreamDecoder*)wave->ctxData;

    return FLAC__stream_decoder_seek_absolute(flacFile, positionInFrames) != 0;
}

int64_t FLAC_Read(RAudio2_WaveInfo* wave_, void* bufferOut, int64_t framesToRead)
{
    ra::WaveInfo wave = wave_;

    if (!wave)
        return 0;

    auto flacFile = (FLAC__StreamDecoder*)wave_->ctxData;

    //return drflac_read_pcm_frames_s16((drflac*)wave->ctxData, framesToRead, (drflac_int16*)bufferOut);
    return 0;
}

bool FLAC_Close(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->ctxData)
        return false;

    auto flacFile = (FLAC__StreamDecoder*)wave->ctxData;

    FLAC__stream_decoder_delete(flacFile);
    wave->ctxData = nullptr;
    return true;
}

bool FLAC_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    switch (ra::str2int(std::string_view(key, keyLength).substr(0, 32)))
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("flac"sv, *valueOut);
        return true;
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 4> extensions{ ".flac", ".oga", ".ogg", nullptr };
        return ra::MakeArrayValue(extensions, *valueOut);
    }
    default: {
        break;
    }
    }
    *valueOut = {};
    return false;
}
