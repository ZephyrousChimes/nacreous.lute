#include <gtest/gtest.h>
#include <channels/InMemoryChannel.h>

#include <array>
#include <thread>
#include <vector>
#include <numeric>
#include <algorithm>

using namespace lute::tm::channels;

class InMemoryChannelTest : public ::testing::Test {
protected:
    static constexpr std::size_t DEFAULT_CAPACITY = 1024;
    
    void SetUp() override {
        channel = std::make_unique<InMemoryChannel>(DEFAULT_CAPACITY);
    }

    std::unique_ptr<InMemoryChannel> channel;
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST_F(InMemoryChannelTest, ConstructorInitializesCorrectly) {
    // Test various power-of-two capacities
    EXPECT_NO_THROW(InMemoryChannel(64));
    EXPECT_NO_THROW(InMemoryChannel(128));
    EXPECT_NO_THROW(InMemoryChannel(256));
    EXPECT_NO_THROW(InMemoryChannel(512));
    EXPECT_NO_THROW(InMemoryChannel(1024));
}

TEST_F(InMemoryChannelTest, SendAndReceiveSingleByte) {
    std::byte data = std::byte{42};
    std::byte received = std::byte{0};

    std::size_t sent = channel->send(&data, sizeof(data));
    EXPECT_EQ(sent, sizeof(data));

    std::size_t received_count = channel->receive(&received, sizeof(received));
    EXPECT_EQ(received_count, sizeof(received));
    EXPECT_EQ(received, data);
}

TEST_F(InMemoryChannelTest, SendAndReceiveMultipleBytes) {
    std::array<std::byte, 10> data;
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = std::byte{static_cast<unsigned char>(i)};
    }

    std::size_t sent = channel->send(data.data(), data.size());
    EXPECT_EQ(sent, data.size());

    std::array<std::byte, 10> received{};
    std::size_t received_count = channel->receive(received.data(), received.size());
    EXPECT_EQ(received_count, data.size());
    EXPECT_EQ(received, data);
}

TEST_F(InMemoryChannelTest, SendAndReceiveStruct) {
    struct TestData {
        int id;
        double value;
        char name[16];
    };

    TestData sent_data{123, 45.67, "test"};
    TestData received_data{};

    std::size_t sent = channel->send(&sent_data, sizeof(sent_data));
    EXPECT_EQ(sent, sizeof(sent_data));

    std::size_t received_count = channel->receive(&received_data, sizeof(received_data));
    EXPECT_EQ(received_count, sizeof(received_data));
    EXPECT_EQ(received_data.id, sent_data.id);
    EXPECT_DOUBLE_EQ(received_data.value, sent_data.value);
    EXPECT_STREQ(received_data.name, sent_data.name);
}

// ============================================================================
// Boundary Conditions Tests
// ============================================================================

TEST_F(InMemoryChannelTest, ReceiveFromEmptyChannel) {
    std::byte buffer[10];
    std::size_t received = channel->receive(buffer, sizeof(buffer));
    EXPECT_EQ(received, 0);
}

TEST_F(InMemoryChannelTest, SendToFullChannel) {
    // Fill the channel
    std::vector<std::byte> data(DEFAULT_CAPACITY);
    std::size_t sent = channel->send(data.data(), data.size());
    EXPECT_EQ(sent, data.size());

    // Try to send more - should return 0 (no space)
    std::byte extra = std::byte{1};
    std::size_t extra_sent = channel->send(&extra, sizeof(extra));
    EXPECT_EQ(extra_sent, 0);
}

TEST_F(InMemoryChannelTest, PartialSendWhenNotEnoughSpace) {
    // Fill channel almost completely
    std::vector<std::byte> data(DEFAULT_CAPACITY - 10);
    channel->send(data.data(), data.size());

    // Try to send 20 bytes (only 10 should fit)
    std::vector<std::byte> large_data(20);
    std::size_t sent = channel->send(large_data.data(), large_data.size());
    EXPECT_EQ(sent, 10);
}

TEST_F(InMemoryChannelTest, PartialReceiveWhenNotEnoughData) {
    // Send 10 bytes
    std::vector<std::byte> data(10);
    channel->send(data.data(), data.size());

    // Try to receive 20 bytes (only 10 available)
    std::vector<std::byte> buffer(20);
    std::size_t received = channel->receive(buffer.data(), buffer.size());
    EXPECT_EQ(received, 10);
}

// ============================================================================
// Wrap-Around Tests
// ============================================================================

TEST_F(InMemoryChannelTest, WrapAroundSend) {
    // Send data that wraps around the ring buffer
    std::vector<std::byte> data1(DEFAULT_CAPACITY - 50);
    for (std::size_t i = 0; i < data1.size(); ++i) {
        data1[i] = std::byte{1};
    }
    channel->send(data1.data(), data1.size());

    // Receive part of it to make space
    std::vector<std::byte> buffer1(DEFAULT_CAPACITY - 100);
    channel->receive(buffer1.data(), buffer1.size());

    // Send data that will wrap around
    std::vector<std::byte> data2(100);
    for (std::size_t i = 0; i < data2.size(); ++i) {
        data2[i] = std::byte{2};
    }
    std::size_t sent = channel->send(data2.data(), data2.size());
    EXPECT_EQ(sent, data2.size());

    // Verify we can receive the remaining data
    std::vector<std::byte> buffer2(150);
    std::size_t received = channel->receive(buffer2.data(), buffer2.size());
    EXPECT_EQ(received, 150); // 50 from data1 + 100 from data2
}

TEST_F(InMemoryChannelTest, WrapAroundReceive) {
    // Fill buffer near the end
    std::vector<std::byte> data1(DEFAULT_CAPACITY - 50);
    channel->send(data1.data(), data1.size());

    // Read part of it
    std::vector<std::byte> buffer1(DEFAULT_CAPACITY - 100);
    channel->receive(buffer1.data(), buffer1.size());

    // Send more to wrap around
    std::vector<std::byte> data2(100);
    for (std::size_t i = 0; i < data2.size(); ++i) {
        data2[i] = std::byte{static_cast<unsigned char>(i)};
    }
    channel->send(data2.data(), data2.size());

    // Receive remaining old data
    std::vector<std::byte> buffer2(50);
    channel->receive(buffer2.data(), buffer2.size());

    // Receive new data that wrapped around
    std::vector<std::byte> buffer3(100);
    std::size_t received = channel->receive(buffer3.data(), buffer3.size());
    EXPECT_EQ(received, 100);
    EXPECT_EQ(buffer3, data2);
}

// ============================================================================
// Multiple Operations Tests
// ============================================================================

TEST_F(InMemoryChannelTest, MultipleSmallOperations) {
    for (int i = 0; i < 100; ++i) {
        std::byte data = std::byte{static_cast<unsigned char>(i)};
        channel->send(&data, sizeof(data));
        
        std::byte received;
        channel->receive(&received, sizeof(received));
        EXPECT_EQ(received, data);
    }
}

TEST_F(InMemoryChannelTest, InterleavedSendAndReceive) {
    std::vector<std::byte> sent_data;
    std::vector<std::byte> received_data;

    // Perform interleaved operations
    for (int i = 0; i < 50; ++i) {
        // Send 10 bytes
        std::array<std::byte, 10> chunk;
        for (int j = 0; j < 10; ++j) {
            chunk[j] = std::byte{static_cast<unsigned char>(i * 10 + j)};
            sent_data.push_back(chunk[j]);
        }
        channel->send(chunk.data(), chunk.size());

        // Receive 5 bytes
        std::array<std::byte, 5> recv_chunk;
        std::size_t received = channel->receive(recv_chunk.data(), recv_chunk.size());
        for (std::size_t k = 0; k < received; ++k) {
            received_data.push_back(recv_chunk[k]);
        }
    }

    // Drain remaining data
    while (true) {
        std::byte b;
        std::size_t received = channel->receive(&b, sizeof(b));
        if (received == 0) break;
        received_data.push_back(b);
    }

    EXPECT_EQ(sent_data, received_data);
}

// ============================================================================
// FIFO Order Tests
// ============================================================================

TEST_F(InMemoryChannelTest, PreservesFIFOOrder) {
    std::vector<int> sent_values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    for (int val : sent_values) {
        channel->send(&val, sizeof(val));
    }

    std::vector<int> received_values;
    for (std::size_t i = 0; i < sent_values.size(); ++i) {
        int val;
        channel->receive(&val, sizeof(val));
        received_values.push_back(val);
    }

    EXPECT_EQ(sent_values, received_values);
}

// ============================================================================
// Concurrent Access Tests
// ============================================================================

TEST_F(InMemoryChannelTest, SingleProducerSingleConsumer) {
    constexpr std::size_t NUM_MESSAGES = 1000;
    std::atomic<bool> start{false};
    std::atomic<bool> producer_done{false};

    // Producer thread
    std::thread producer([&]() {
        while (!start.load()) { /* spin wait */ }
        
        for (std::size_t i = 0; i < NUM_MESSAGES; ++i) {
            while (channel->send(&i, sizeof(i)) == 0) {
                std::this_thread::yield();
            }
        }
        producer_done.store(true);
    });

    // Consumer thread
    std::thread consumer([&]() {
        while (!start.load()) { /* spin wait */ }
        
        std::vector<std::size_t> received;
        while (received.size() < NUM_MESSAGES) {
            std::size_t val;
            if (channel->receive(&val, sizeof(val)) > 0) {
                received.push_back(val);
            } else {
                std::this_thread::yield();
            }
        }

        // Verify order
        for (std::size_t i = 0; i < NUM_MESSAGES; ++i) {
            EXPECT_EQ(received[i], i);
        }
    });

    start.store(true);
    producer.join();
    consumer.join();
}

TEST_F(InMemoryChannelTest, ConcurrentStressTest) {
    constexpr std::size_t NUM_ITERATIONS = 10000;
    std::atomic<std::size_t> total_sent{0};
    std::atomic<std::size_t> total_received{0};

    std::thread producer([&]() {
        for (std::size_t i = 0; i < NUM_ITERATIONS; ++i) {
            int val = static_cast<int>(i);
            while (channel->send(&val, sizeof(val)) == 0) {
                std::this_thread::yield();
            }
            total_sent.fetch_add(1);
        }
    });

    std::thread consumer([&]() {
        for (std::size_t i = 0; i < NUM_ITERATIONS; ++i) {
            int val;
            while (channel->receive(&val, sizeof(val)) == 0) {
                std::this_thread::yield();
            }
            total_received.fetch_add(1);
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(total_sent.load(), NUM_ITERATIONS);
    EXPECT_EQ(total_received.load(), NUM_ITERATIONS);
}

// ============================================================================
// Interface Tests (IChannel polymorphism)
// ============================================================================

TEST_F(InMemoryChannelTest, WorksThroughIChannelInterface) {
    IChannel* iface = channel.get();
    
    int sent_value = 42;
    std::size_t sent = iface->send(&sent_value, sizeof(sent_value));
    EXPECT_EQ(sent, sizeof(sent_value));

    int received_value = 0;
    std::size_t received = iface->receive(&received_value, sizeof(received_value));
    EXPECT_EQ(received, sizeof(received_value));
    EXPECT_EQ(received_value, sent_value);
}

// ============================================================================
// Edge Cases Tests
// ============================================================================

TEST_F(InMemoryChannelTest, ZeroSizeOperations) {
    std::byte data;
    
    std::size_t sent = channel->send(&data, 0);
    EXPECT_EQ(sent, 0);

    std::size_t received = channel->receive(&data, 0);
    EXPECT_EQ(received, 0);
}

TEST_F(InMemoryChannelTest, LargeDataTransfer) {
    // Test transferring data larger than channel capacity
    std::vector<std::byte> large_data(DEFAULT_CAPACITY * 2);
    for (std::size_t i = 0; i < large_data.size(); ++i) {
        large_data[i] = std::byte{static_cast<unsigned char>(i % 256)};
    }

    std::vector<std::byte> received_data;
    std::size_t offset = 0;

    // Send in chunks
    while (offset < large_data.size()) {
        std::size_t sent = channel->send(
            large_data.data() + offset,
            large_data.size() - offset
        );
        offset += sent;

        // Receive what's available
        std::array<std::byte, 128> buffer;
        std::size_t recv = channel->receive(buffer.data(), buffer.size());
        received_data.insert(received_data.end(), buffer.begin(), buffer.begin() + recv);
    }

    // Drain remaining
    while (received_data.size() < large_data.size()) {
        std::array<std::byte, 128> buffer;
        std::size_t recv = channel->receive(buffer.data(), buffer.size());
        if (recv == 0) break;
        received_data.insert(received_data.end(), buffer.begin(), buffer.begin() + recv);
    }

    EXPECT_EQ(received_data.size(), large_data.size());
    EXPECT_EQ(received_data, large_data);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}