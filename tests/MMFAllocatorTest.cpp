#include <gtest/gtest.h>
#include <mmf_allocator/MMFAllocator.hpp>
#include <string>
#include <cstdio>
#include <list>

class TestMMFAllocator : public ::testing::Test {
  protected:
};

TEST_F(TestMMFAllocator, Basic) {
    MMFAllocator allocator{"test_mmf.dat", 4096};
    allocator.open();

    auto ptr = allocator.allocate(sizeof(std::string));
    auto* str = new (ptr) std::string("test");
    ASSERT_EQ(*str, std::string("test"));

    allocator = {};
}

TEST_F(TestMMFAllocator, LoadTest) {
    constexpr std::size_t kMb = 1024 * 1024;
    MMFAllocator allocator{"test_mmf.dat", 16 * kMb};
    allocator.open();

    struct Message {
        char payload[1024];
    };

    using MessagePtr = Message*;
    std::list<MessagePtr> list;

    const auto msgCount = 16 * 1024;

    for (int i = 0; i < msgCount; ++i) {
        auto* ptr = allocator.allocate(sizeof(Message));
        auto* msg = new (ptr) Message();
        std::sprintf(&msg->payload[0], "Message %d", i);
        list.emplace_back(msg);
    }
    ASSERT_EQ(list.size(), msgCount);

    int i = 0;
    for (const auto& msgPtr : list) {
        ASSERT_STREQ(msgPtr->payload, (std::string("Message ") + std::to_string(i++)).c_str());
    }
}
