module;

#include <System/Macros.h>

export module ABRSimulation360.BitrateAllocators.BitrateAllocatorFactory;

import System.Base;

import ABRSimulation360.Base;
import ABRSimulation360.BitrateAllocators.BOLAAllocator;
import ABRSimulation360.BitrateAllocators.DragonflyAllocator;
import ABRSimulation360.BitrateAllocators.FlareAllocator;
import ABRSimulation360.BitrateAllocators.HybridAllocator;
import ABRSimulation360.BitrateAllocators.IBitrateAllocator;
import ABRSimulation360.BitrateAllocators.OnlineLearningAllocator;

using namespace std;

#define TRY_CREATE(T) \
    if (const auto *const _options = dynamic_cast<const CONCAT(T, Options) *>(&options)) \
        return make_unique<T>(streamingConfig, *_options);

/// Represents a factory for bitrate allocators.
export class BitrateAllocatorFactory {
public:
    /// Creates a bitrate allocator with the specified configuration and options.
    /// @param streamingConfig The adaptive bitrate streaming configuration.
    /// @param options The options for the bitrate allocator.
    /// @returns A bitrate allocator with the specified configuration and options.
    [[nodiscard]] static unique_ptr<IBitrateAllocator> Create(const StreamingConfig &streamingConfig,
                                                              const BaseBitrateAllocatorOptions &options) {
        FOR_EACH(TRY_CREATE, (
                     BOLAAllocator,
                     DragonflyAllocator,
                     FlareAllocator,
                     HybridAllocator,
                     OnlineLearningAllocator
                 ))
        throw runtime_error("Unknown bitrate allocator options.");
    }
};
