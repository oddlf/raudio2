#pragma once

#include "raudio2/raudio2.hpp"

struct AudioProcessor;

// Audio processor struct
// NOTE: Useful to apply effects to an AudioBuffer
struct AudioProcessor
{
    AudioCallback process; // Processor callback function
    AudioProcessor* next;  // Next audio processor on the list
    AudioProcessor* prev;  // Previous audio processor on the list
};
