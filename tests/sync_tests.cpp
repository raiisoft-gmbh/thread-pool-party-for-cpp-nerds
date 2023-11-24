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

#include "pool_party/detail/sync.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <mutex>

using testing::_;

namespace {
class MutexMock {
public:
    MOCK_METHOD(void, lock, ());
    MOCK_METHOD(void, unlock, ());
};
using NiceMutexMock = testing::NiceMock<MutexMock>;

class ConditionVariableMock {
public:
    MOCK_METHOD(void, wait, (std::unique_lock<NiceMutexMock>&, std::function<bool()>) );
    MOCK_METHOD(void, notify_one, ());
    MOCK_METHOD(void, notify_all, ());
};
using NiceConditionVariableMock = testing::NiceMock<ConditionVariableMock>;
}  // namespace

class SyncTests : public testing::Test {
protected:
    pool_party::detail::Sync<NiceMutexMock, NiceConditionVariableMock> m_sync{};
    NiceMutexMock& mutex_mock{m_sync.getMutex()};
    NiceConditionVariableMock& condition_variable_mock{m_sync.getConditionVariable()};
};

TEST_F(SyncTests, WaitForNotificationAndExecuteCallable) {
    auto predicate_function{[]() { return true; }};

    EXPECT_CALL(mutex_mock, lock());
    EXPECT_CALL(condition_variable_mock, wait(_, _));
    EXPECT_CALL(mutex_mock, unlock());

    bool callback_called{false};
    auto callback{[&callback_called](std::unique_lock<NiceMutexMock>&) { callback_called = true; }};

    m_sync.wait(predicate_function, callback);
    EXPECT_THAT(callback_called, testing::IsTrue);
}

TEST_F(SyncTests, IsPredicateFunctionPassedToCVWait) {
    bool predicate_function_called{false};
    auto predicate_function{[&predicate_function_called]() {
        predicate_function_called = true;
        return true;
    }};

    EXPECT_CALL(condition_variable_mock, wait(_, _))
    .WillOnce([](std::unique_lock<NiceMutexMock>&, std::function<bool()> func) { func(); });

    m_sync.wait(predicate_function, [](std::unique_lock<NiceMutexMock>&) {});

    EXPECT_TRUE(predicate_function_called);
}

TEST_F(SyncTests, NotifyWaitingThread) {
    EXPECT_CALL(condition_variable_mock, notify_one());
    m_sync.notifyOne();
}

TEST_F(SyncTests, ExecuteCodeLockedBySyncMutex) {
    EXPECT_CALL(mutex_mock, lock());
    EXPECT_CALL(mutex_mock, unlock());

    bool function_called{false};
    m_sync.executeProtected([&function_called] { function_called = true; });

    EXPECT_THAT(function_called, testing::IsTrue);
}

TEST_F(SyncTests, NotifyAllWaitingThreads) {
    EXPECT_CALL(condition_variable_mock, notify_all());
    m_sync.notifyAll();
}
