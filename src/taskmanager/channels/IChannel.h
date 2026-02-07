#pragma once

#include <cstddef>

namespace lute::tm::channels {

class IChannel {
public:
    virtual ~IChannel() = default;
    virtual std::size_t receive(void* buffer, std::size_t capacity) = 0;
    virtual std::size_t send(void* buffer, std::size_t size) = 0;
};

} // lute::tm::channels