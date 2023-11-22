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

#include "mocks/joinable_mock.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>

using testing::Return;

class ThreadJoinerTests : public testing::Test {
protected:
    pool_party::test::NiceJoinableMock m_mock{};
    pool_party::detail::ThreadJoiner<pool_party::test::NiceJoinableMockMovable> m_thread_joiner{
    pool_party::test::NiceJoinableMockMovable{m_mock}};
};

TEST_F(ThreadJoinerTests, JoinThreadWithThreadJoinerWhenItIsJoinable) {
    testing::Sequence s{};
    EXPECT_CALL(m_thread_joiner.get().getMock(), joinable()).WillOnce(Return(true)).InSequence(s);
    EXPECT_CALL(m_thread_joiner.get().getMock(), join()).InSequence(s);
}

TEST_F(ThreadJoinerTests, DontJoinWhenJoinableIsFalse) {
    testing::Sequence s{};
    EXPECT_CALL(m_thread_joiner.get().getMock(), joinable()).WillOnce(Return(false)).InSequence(s);
    EXPECT_CALL(m_thread_joiner.get().getMock(), join()).Times(0).InSequence(s);
}
