#pragma once

#include <channels/IChannel.h>

#include <cstddef>

namespace lute::tm::channels {

/**
 * @class Channel
 * @brief CRTP base class for channel implementations
 * 
 * This class uses the Curiously Recurring Template Pattern (CRTP) to provide
 * static polymorphism while maintaining interface compatibility with IChannel.
 */
template<typename ChannelImpl>
class Channel: public IChannel {
public:
    Channel() = default;

    std::size_t receive(void* const buffer, const std::size_t maxCapacity) override {
        return static_cast<ChannelImpl*>(this)->receive(buffer, maxCapacity);
    }

    std::size_t send(void* const buffer, const std::size_t size) override {
        return static_cast<ChannelImpl*>(this)->send(buffer, size);
    }
    
private:
};

} // lute::tm::channels