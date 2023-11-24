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

#include "pool_party/detail/thread_pool.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <stdexcept>

using testing::_;

namespace {

class ThreadMock {};
class ThreadFactoryMock {
public:
    using thread_type = ThreadMock;
    MOCK_METHOD(ThreadMock, create, (std::function<void()>) );
};

class MutexMock {
public:
    MOCK_METHOD(void, unlock, ());
    MOCK_METHOD(void, lock, ());
};
using NiceMutexMock = testing::NiceMock<MutexMock>;
using UniqueLock    = std::unique_lock<NiceMutexMock>;

class SyncMock {
public:
    using mutex_type = NiceMutexMock;
    MOCK_METHOD(void, wait, (std::function<bool()>, std::function<void(UniqueLock &)>) );
    MOCK_METHOD(void, notifyOne, ());
    MOCK_METHOD(void, notifyAll, ());
    MOCK_METHOD(void, executeProtected, (std::function<void()>) );
};

using NiceSyncMock          = testing::NiceMock<SyncMock>;
using NiceThreadFactoryMock = testing::NiceMock<ThreadFactoryMock>;

}  // namespace

class ThreadPoolTests : public testing::Test {
protected:
    std::size_t m_thread_count{4};
    NiceThreadFactoryMock m_thread_factory_mock{};
    NiceSyncMock m_sync_mock{};
    NiceMutexMock m_mutex_mock{};
    UniqueLock m_lock{m_mutex_mock};
    pool_party::detail::ThreadPool<NiceThreadFactoryMock, NiceSyncMock> m_thread_pool{m_thread_count,
                                                                                      m_thread_factory_mock,
                                                                                      m_sync_mock};
    std::vector<std::function<void()>> m_worker_functions{};

    void SetUp() override {
        ON_CALL(m_sync_mock, executeProtected(_)).WillByDefault([](std::function<void()> func) { func(); });

        // Deactivate waiting by default
        ON_CALL(m_sync_mock, wait(_, _))
        .WillByDefault([this](std::function<bool()>, std::function<void(UniqueLock &)>) { m_thread_pool.shutdown(); });

        // Save all thread worker functions
        ON_CALL(m_thread_factory_mock, create(_)).WillByDefault([this](std::function<void()> f) {
            m_worker_functions.push_back(f);
            return ThreadMock{};
        });
    }

    template<typename Func>
    void executeEach(const std::vector<Func> &container) {
        std::for_each(container.begin(), container.end(), [](const auto &func) { func(); });
    }

    template<typename Func>
    void executeFirst(const std::vector<Func> &container) {
        container.front()();
    }

    void activateWaiting(pool_party::detail::ThreadPool<NiceThreadFactoryMock, NiceSyncMock> &tp) {
        EXPECT_CALL(m_sync_mock, wait(_, _))
        .WillRepeatedly([this, &tp](std::function<bool()>, std::function<void(UniqueLock &)> wait_callable) {
            wait_callable(m_lock);
            tp.shutdown();
        });
    }
};

TEST_F(ThreadPoolTests, CreatedNumberOfThreads) {
    EXPECT_CALL(m_thread_factory_mock, create(_)).Times(m_thread_count);
    pool_party::detail::ThreadPool{m_thread_count, m_thread_factory_mock, m_sync_mock};
}

TEST_F(ThreadPoolTests, AddingTaskToThreadPoolNotifiesThread) {
    EXPECT_CALL(m_sync_mock, notifyOne());
    m_thread_pool.enqueue([]() {});
}

TEST_F(ThreadPoolTests, ProcessingEnqueuedTask) {
    pool_party::detail::ThreadPool thread_pool{m_thread_count, m_thread_factory_mock, m_sync_mock};
    activateWaiting(thread_pool);

    int task_result{5};
    auto future{thread_pool.enqueue([&task_result]() { return task_result; })};

    executeFirst(m_worker_functions);
    EXPECT_THAT(future.get(), testing::Eq(task_result));
}

TEST_F(ThreadPoolTests, ReleaseLockBeforeProcessingEnqueuedTask) {
    pool_party::detail::ThreadPool thread_pool{m_thread_count, m_thread_factory_mock, m_sync_mock};
    activateWaiting(thread_pool);

    EXPECT_CALL(m_mutex_mock, unlock());
    auto future{thread_pool.enqueue([this]() {
        testing::Mock::VerifyAndClearExpectations(&m_mutex_mock);
        return 0;
    })};

    executeFirst(m_worker_functions);
}

TEST_F(ThreadPoolTests, KeepWaitingWhenNoTaskIsAvailable) {
    pool_party::detail::ThreadPool thread_pool{m_thread_count, m_thread_factory_mock, m_sync_mock};

    EXPECT_CALL(m_sync_mock, wait(_, _))
    .WillOnce([&thread_pool](std::function<bool()> predicate, std::function<void(UniqueLock &)>) {
        EXPECT_FALSE(predicate());
        thread_pool.shutdown();
    });

    executeFirst(m_worker_functions);
}

TEST_F(ThreadPoolTests, StopWaitingWhenPoolIsShutDown) {
    pool_party::detail::ThreadPool thread_pool{m_thread_count, m_thread_factory_mock, m_sync_mock};

    EXPECT_CALL(m_sync_mock, wait(_, _))
    .WillOnce([&thread_pool](std::function<bool()> predicate, std::function<void(UniqueLock &)>) {
        thread_pool.shutdown();
        EXPECT_TRUE(predicate());
    });

    executeFirst(m_worker_functions);
}

TEST_F(ThreadPoolTests, FinishWorkWhenThreadPoolWasShutDown) {
    pool_party::detail::ThreadPool thread_pool{m_thread_count, m_thread_factory_mock, m_sync_mock};

    EXPECT_CALL(m_sync_mock, wait(_, _))
    .WillOnce(
    [&thread_pool, this](std::function<bool()>, std::function<void(UniqueLock &)>) { thread_pool.shutdown(); })
    .WillOnce([&thread_pool, this](std::function<bool()>, std::function<void(UniqueLock &)> wait_callable) {
        wait_callable(m_lock);
    });

    int task_result{5};
    auto future{thread_pool.enqueue([&task_result]() { return task_result; })};

    executeFirst(m_worker_functions);
    EXPECT_THAT(future.get(), testing::Eq(task_result));
}

TEST_F(ThreadPoolTests, PreventQueueProcessingWhenShutdownAndWorkDone) {
    pool_party::detail::ThreadPool thread_pool{m_thread_count, m_thread_factory_mock, m_sync_mock};

    EXPECT_CALL(m_sync_mock, wait(_, _))
    .WillOnce([&thread_pool, this](std::function<bool()>, std::function<void(UniqueLock &)> wait_callable) {
        thread_pool.shutdown();
        wait_callable(m_lock);
    });

    executeFirst(m_worker_functions);
}

TEST_F(ThreadPoolTests, ShutdownNotifiesAllThreads) {
    EXPECT_CALL(m_sync_mock, notifyAll());
    m_thread_pool.shutdown();
}

TEST_F(ThreadPoolTests, DontEnqueueTasksAfterShutdown) {
    m_thread_pool.shutdown();

    EXPECT_THROW(
    {
        try {
            m_thread_pool.enqueue([]() {});
        } catch (const std::runtime_error &e) {
            EXPECT_STREQ("Thread pool already shut down, enqueuing failed.", e.what());
            throw;
        }
    },
    std::runtime_error);
}

TEST_F(ThreadPoolTests, TaskOrderIsFifo) {
    pool_party::detail::ThreadPool thread_pool{m_thread_count, m_thread_factory_mock, m_sync_mock};

    int task_index{0};
    thread_pool.enqueue([&task_index]() { task_index = 1; });
    thread_pool.enqueue([&task_index]() { task_index = 2; });

    EXPECT_CALL(m_sync_mock, wait(_, _))
    .WillOnce(
    [&thread_pool, &task_index, this](std::function<bool()>, std::function<void(UniqueLock &)> wait_callable) {
        std::unique_lock<NiceMutexMock> ul{m_mutex_mock};
        wait_callable(ul);
        EXPECT_THAT(task_index, testing::Eq(1));
    })
    .WillOnce(
    [&thread_pool, &task_index, this](std::function<bool()>, std::function<void(UniqueLock &)> wait_callable) {
        std::unique_lock<NiceMutexMock> ul{m_mutex_mock};
        wait_callable(ul);
        EXPECT_THAT(task_index, testing::Eq(2));
        thread_pool.shutdown();
    });

    executeFirst(m_worker_functions);
}
