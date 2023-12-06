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

#include "mocks/joinable_mock.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <stdexcept>

using testing::_;

namespace {
class ThreadFactoryMock {
public:
    using thread_type = pool_party::test::NiceJoinableMockMovable;
    MOCK_METHOD(pool_party::test::NiceJoinableMockMovable, create, (std::function<void()>) );
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
    MOCK_METHOD(void, waitThenExecute, (std::function<bool()>, std::function<void(UniqueLock &)>) );
    MOCK_METHOD(void, notifyOne, ());
    MOCK_METHOD(void, notifyAll, ());
    MOCK_METHOD(void, executeLocked, (std::function<void()>) );
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
    std::vector<std::function<void()>> m_worker_functions{};
    pool_party::test::NiceJoinableMock m_joinable_mock{};

    void SetUp() override {
        ON_CALL(m_sync_mock, executeLocked(_)).WillByDefault([](std::function<void()> func) { func(); });

        // Save all thread worker functions
        ON_CALL(m_thread_factory_mock, create(_)).WillByDefault([this](std::function<void()> f) {
            m_worker_functions.push_back(f);
            return pool_party::test::NiceJoinableMockMovable{m_joinable_mock};
        });
    }

    pool_party::detail::ThreadPool<NiceThreadFactoryMock, NiceSyncMock> createPool() {
        pool_party::detail::ThreadPool created_pool{m_thread_count, m_thread_factory_mock, m_sync_mock};

        ON_CALL(m_sync_mock, waitThenExecute(_, _))
        .WillByDefault(
        [&created_pool](std::function<bool()>, std::function<void(UniqueLock &)>) { created_pool.shutdown(); });

        return created_pool;
    }

    template<typename Func>
    void executeEach(const std::vector<Func> &container) {
        for (const auto &func : container) {
            func();
        }
    }

    template<typename Func>
    void executeFirst(const std::vector<Func> &container) {
        container.front()();
    }

    void activateWaiting(pool_party::detail::ThreadPool<NiceThreadFactoryMock, NiceSyncMock> &tp) {
        EXPECT_CALL(m_sync_mock, waitThenExecute(_, _))
        .WillRepeatedly([this, &tp](std::function<bool()>, std::function<void(UniqueLock &)> wait_callable) {
            wait_callable(m_lock);
            tp.shutdown();
        });
    }
};

TEST_F(ThreadPoolTests, CreatedNumberOfThreads) {
    EXPECT_CALL(m_thread_factory_mock, create(_)).Times(m_thread_count);
    createPool();
}

TEST_F(ThreadPoolTests, AddingTaskToThreadPoolNotifiesThread) {
    auto thread_pool{createPool()};
    EXPECT_CALL(m_sync_mock, notifyOne());
    thread_pool.enqueue([]() {});
}

TEST_F(ThreadPoolTests, ProcessingEnqueuedTask) {
    auto thread_pool{createPool()};
    activateWaiting(thread_pool);

    const int task_result{5};
    auto future{thread_pool.enqueue([&task_result]() { return task_result; })};

    executeFirst(m_worker_functions);
    EXPECT_THAT(future.get(), testing::Eq(task_result));
}

TEST_F(ThreadPoolTests, ReleaseLockBeforeProcessingEnqueuedTask) {
    auto thread_pool{createPool()};
    activateWaiting(thread_pool);

    EXPECT_CALL(m_mutex_mock, unlock());
    auto future{thread_pool.enqueue([this]() {
        testing::Mock::VerifyAndClearExpectations(&m_mutex_mock);
        return 0;
    })};

    executeFirst(m_worker_functions);
}

TEST_F(ThreadPoolTests, KeepWaitingWhenNoTaskIsAvailable) {
    auto thread_pool{createPool()};

    EXPECT_CALL(m_sync_mock, waitThenExecute(_, _))
    .WillOnce([&thread_pool](std::function<bool()> predicate, std::function<void(UniqueLock &)>) {
        EXPECT_FALSE(predicate());
        thread_pool.shutdown();
    });

    executeFirst(m_worker_functions);
}

TEST_F(ThreadPoolTests, StopWaitingWhenPoolIsShutDown) {
    auto thread_pool{createPool()};

    EXPECT_CALL(m_sync_mock, waitThenExecute(_, _))
    .WillOnce([&thread_pool](std::function<bool()> predicate, std::function<void(UniqueLock &)>) {
        thread_pool.shutdown();
        EXPECT_TRUE(predicate());
    });

    executeFirst(m_worker_functions);
}

TEST_F(ThreadPoolTests, FinishWorkWhenThreadPoolWasShutDown) {
    auto thread_pool{createPool()};

    EXPECT_CALL(m_sync_mock, waitThenExecute(_, _))
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
    auto thread_pool{createPool()};

    EXPECT_CALL(m_sync_mock, waitThenExecute(_, _))
    .WillOnce([&thread_pool, this](std::function<bool()>, std::function<void(UniqueLock &)> wait_callable) {
        thread_pool.shutdown();
        wait_callable(m_lock);
    });

    executeFirst(m_worker_functions);
}

TEST_F(ThreadPoolTests, ShutdownNotifiesAllThreads) {
    auto thread_pool{createPool()};
    // notifyAll is called manually on shutdown() and on destruction of the thread_pool object.
    EXPECT_CALL(m_sync_mock, notifyAll()).Times(2);
    thread_pool.shutdown();
}

TEST_F(ThreadPoolTests, DontEnqueueTasksAfterShutdown) {
    auto thread_pool{createPool()};
    thread_pool.shutdown();

    EXPECT_THROW(
    {
        try {
            thread_pool.enqueue([]() {});
        } catch (const std::runtime_error &e) {
            EXPECT_STREQ("Thread pool already shut down, enqueuing failed.", e.what());
            throw;
        }
    },
    std::runtime_error);
}

TEST_F(ThreadPoolTests, TaskOrderIsFifo) {
    auto thread_pool{createPool()};

    int task_index{0};
    thread_pool.enqueue([&task_index]() { task_index = 1; });
    thread_pool.enqueue([&task_index]() { task_index = 2; });

    EXPECT_CALL(m_sync_mock, waitThenExecute(_, _))
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

TEST_F(ThreadPoolTests, ShutdownPoolWhileDestruction) {
    EXPECT_CALL(m_sync_mock, notifyAll());
    auto thread_pool{createPool()};
}
