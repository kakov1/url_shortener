#include <gtest/gtest.h>

#include <chrono>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "concurrency/queue/queue.hpp"

namespace shortener {
namespace {

TEST(ThreadSafeQueueTest, PushThenPopReturnsValue) {
  ThreadSafeQueue<int> queue;

  queue.push(42);
  const auto value = queue.pop();

  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, 42);
}

TEST(ThreadSafeQueueTest, PopReturnsNulloptAfterCloseWhenQueueIsEmpty) {
  ThreadSafeQueue<int> queue;

  queue.close();
  const auto value = queue.pop();

  EXPECT_FALSE(value.has_value());
}

TEST(ThreadSafeQueueTest, CloseIsIdempotent) {
  ThreadSafeQueue<int> queue;

  queue.close();
  queue.close();

  EXPECT_TRUE(queue.closed());
}

TEST(ThreadSafeQueueTest, PushOnClosedQueueThrows) {
  ThreadSafeQueue<int> queue;
  queue.close();

  EXPECT_THROW(queue.push(1), std::runtime_error);
}

TEST(ThreadSafeQueueTest, PreservesFifoOrder) {
  ThreadSafeQueue<int> queue;

  queue.push(1);
  queue.push(2);
  queue.push(3);

  const auto first = queue.pop();
  const auto second = queue.pop();
  const auto third = queue.pop();

  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(second.has_value());
  ASSERT_TRUE(third.has_value());

  EXPECT_EQ(*first, 1);
  EXPECT_EQ(*second, 2);
  EXPECT_EQ(*third, 3);
}

TEST(ThreadSafeQueueTest, PopUnblocksAfterProducerPushesValue) {
  ThreadSafeQueue<int> queue;
  std::optional<int> result;

  std::thread consumer([&]() { result = queue.pop(); });

  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  queue.push(99);

  consumer.join();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 99);
}

TEST(ThreadSafeQueueTest, PopUnblocksAfterClose) {
  ThreadSafeQueue<int> queue;
  std::optional<int> result;

  std::thread consumer([&]() { result = queue.pop(); });

  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  queue.close();

  consumer.join();

  EXPECT_FALSE(result.has_value());
}

} // namespace
} // namespace shortener