#pragma once

#include <channels/Channel.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <cassert>

namespace lute::tm::channels {

class InMemoryChannel : public Channel<InMemoryChannel> {
public:
    explicit InMemoryChannel(std::size_t capacity_power_of_two)
        : capacity_(capacity_power_of_two),
          mask_(capacity_power_of_two - 1),
          buffer_(std::make_unique<std::byte[]>(capacity_power_of_two)),
          write_index_(0),
          read_index_(0)
    {
        assert((capacity_power_of_two & (capacity_power_of_two - 1)) == 0);
    }

    std::size_t send(const void* data, std::size_t size) {
        const std::size_t w = write_index_.load(std::memory_order_relaxed);
        const std::size_t r = read_index_.load(std::memory_order_acquire);

        const std::size_t available = capacity_ - (w - r);
        const std::size_t to_write = (size < available) ? size : available;

        if (to_write == 0) return 0;

        const std::size_t write_pos = w & mask_;
        const std::size_t first_chunk = std::min(to_write, capacity_ - write_pos);

        std::memcpy(buffer_.get() + write_pos, data, first_chunk);
        std::memcpy(buffer_.get(), 
                    static_cast<const std::byte*>(data) + first_chunk,
                    to_write - first_chunk);

        write_index_.store(w + to_write, std::memory_order_release);
        return to_write;
    }

    std::size_t receive(void* data, std::size_t size) {
        const std::size_t r = read_index_.load(std::memory_order_relaxed);
        const std::size_t w = write_index_.load(std::memory_order_acquire);

        const std::size_t available = w - r;
        const std::size_t to_read = (size < available) ? size : available;

        if (to_read == 0) return 0;

        const std::size_t read_pos = r & mask_;
        const std::size_t first_chunk = std::min(to_read, capacity_ - read_pos);

        std::memcpy(data, buffer_.get() + read_pos, first_chunk);
        std::memcpy(static_cast<std::byte*>(data) + first_chunk,
                    buffer_.get(),
                    to_read - first_chunk);

        read_index_.store(r + to_read, std::memory_order_release);
        return to_read;
    }

private:
    const std::size_t capacity_;
    const std::size_t mask_;

    std::unique_ptr<std::byte[]> buffer_;

    alignas(64) std::atomic<std::size_t> write_index_;
    alignas(64) std::atomic<std::size_t> read_index_;
};

} // namespace lute::tm::channels
