#pragma once

#include <atomic>
#include "AudioBuffer.h"
#include <miniaudio.h>

struct AudioDataSystem {
    ma_context context;        // miniaudio context data
    ma_device device;          // miniaudio device
    ma_mutex lock;             // miniaudio mutex lock
    std::atomic<bool> isReady; // Check if audio device is ready
    int32_t pcmBufferSize;     // Pre-allocated buffer size
    void* pcmBuffer;           // Pre-allocated buffer to read audio data from file/memory
};

struct AudioDataBuffer {
    AudioBuffer* first;  // Pointer to first AudioBuffer in the list
    AudioBuffer* last;   // Pointer to last AudioBuffer in the list
    int32_t defaultSize; // Default audio buffer size for audio streams
};

// Audio data context
struct AudioData {
    AudioDataSystem system;
    AudioDataBuffer buffer;
    AudioProcessor* mixedProcessor;
};
