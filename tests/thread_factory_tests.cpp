/**
 * Copyright (c) 2023 RAIISoft GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "pool_party/detail/thread_factory.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>

namespace {
class ThreadFake {
    using ThreadFunction = std::function<void()>;

public:
    ThreadFake(ThreadFunction thread_function) : m_thread_function{std::move(thread_function)} {}

    void run_thread_function() {
        m_thread_function();
    }

private:
    ThreadFunction m_thread_function{};
};
}  // namespace

class ThreadFactoryTests : public testing::Test {};

TEST_F(ThreadFactoryTests, CreateThreadWithFactory) {
    bool thread_function_called{false};

    pool_party::detail::ThreadFactory<ThreadFake> thread_factory{};

    auto thread_function{[&thread_function_called]() { thread_function_called = true; }};
    auto created_thread{thread_factory.create(thread_function)};
    created_thread.run_thread_function();
    EXPECT_THAT(thread_function_called, testing::IsTrue);
}
