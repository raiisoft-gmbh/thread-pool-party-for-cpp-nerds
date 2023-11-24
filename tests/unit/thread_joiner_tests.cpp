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

#include "pool_party/detail/thread_joiner.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>

using testing::Return;

namespace {
class ThreadFake {
    using ThreadFunction = std::function<void()>;

public:
    ThreadFake(ThreadFunction thread_function) : m_thread_function{thread_function} {}
    MOCK_METHOD(void, join, ());
    MOCK_METHOD(bool, joinable, ());

private:
    ThreadFunction m_thread_function{};
};
}  // namespace

class ThreadJoinerTests : public testing::Test {
protected:
    pool_party::detail::ThreadJoiner<testing::NiceMock<ThreadFake>> m_thread_joiner{[]() {}};
};

TEST_F(ThreadJoinerTests, JoinThreadWithThreadJoinerWhenItIsJoinable) {
    EXPECT_CALL(m_thread_joiner.get(), joinable()).WillOnce(Return(true));
    EXPECT_CALL(m_thread_joiner.get(), join());
}

TEST_F(ThreadJoinerTests, DontJoinWhenJoinableIsFalse) {
    EXPECT_CALL(m_thread_joiner.get(), joinable()).WillOnce(Return(false));
    EXPECT_CALL(m_thread_joiner.get(), join()).Times(0);
}
