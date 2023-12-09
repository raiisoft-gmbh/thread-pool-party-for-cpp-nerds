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

#ifndef POOL_PARTY_DETAIL_THREAD_POOL_HPP_
#define POOL_PARTY_DETAIL_THREAD_POOL_HPP_

#include "thread_joiner.hpp"

#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <utility>
#include <vector>

namespace pool_party {
namespace detail {

/**
 * @brief ThreadPool detail implementation
 *
 * This class contains all the thread pool related implementation details.
 *
 * @tparam ThreadFactory, a dependency for thread creation
 * @tparam Sync manages the synchronization between threads of the thread pool
 *
 * @see pool_party::detail::Sync
 * @see pool_party::detail::ThreadFactory
 */
template<typename ThreadFactory, typename Sync>
class ThreadPool {
    using ThreadType       = typename ThreadFactory::thread_type;
    using ThreadJoinerType = ThreadJoiner<ThreadType>;

public:
    /**
     * @brief Constructor of ThreadPool
     *
     * This function creates all neccessary threads and prepares them to work on the
     * thread pools tasks.
     *
     * @param number_of_threads The number of threads the thread pool should consist of.
     * @param thread_factory Takes care of thread creation
     * @param sync Handles synchronization of threads
     */
    ThreadPool(std::size_t number_of_threads, ThreadFactory& thread_factory, Sync& sync) : m_sync{sync} {
        m_workers.reserve(number_of_threads);
        for (std::size_t current_thread{0}; current_thread < number_of_threads; ++current_thread) {
            m_workers.push_back(ThreadJoinerType{thread_factory.create([this]() { work(); })});
        }
    }
    ThreadPool(const ThreadPool&)            = default;
    ThreadPool(ThreadPool&&)                 = default;
    ThreadPool& operator=(const ThreadPool&) = default;
    ThreadPool& operator=(ThreadPool&&)      = default;

    /**
     * @brief Custom destructor for shutting down thread pool automatically
     */
    ~ThreadPool() {
        shutdown();
    }

    /**
     * @brief Enqueue a new task
     *
     * This function allows to pass a new tasks to the thread pool which is then handled asynchronous
     * by one of the worker threads.
     *
     * After signaling a shutdown, enqueuing new tasks is NOT allowed and punished with a
     * corresponding exception.
     *
     * @tparam Callable Type of tasks function
     * @tparam Args Variadic template type of tasks function arguments
     * @tparam R Automatically generated result type
     *
     * @param callable The callable which contains the task
     * @param args Variadic arguments which are passed to the tasks callable
     *
     * @exception std::runtime_error is thrown when the thread pool is already shut down
     *
     * @returns std::future<R> with tasks result
     */
    template<typename Callable, typename... Args, typename R = typename std::result_of<Callable(Args...)>::type>
    std::future<R> enqueue(Callable&& callable, Args&&... args) {
        std::packaged_task<R()> task{std::bind(std::forward<Callable>(callable), std::forward<Args>(args)...)};
        auto future{task.get_future()};

        m_sync.get().executeLocked([&task, this]() {
            throwWhenPoolIsShutDown();
            m_tasks.emplace_front(std::move(task));
        });
        m_sync.get().notifyOne();

        return future;
    }

    /**
     * @brief Shutdown the thread pool
     *
     * This function signals a shutdown to all worker threads. The threads will process the
     * remaining stored tasks. After shuting down the thread pool enqueuing is not allowed anymore.
     */
    void shutdown() {
        m_sync.get().executeLocked([this]() { is_shutdown = true; });
        m_sync.get().notifyAll();
    }

private:
    using TaskType     = std::packaged_task<void()>;
    using TaskLockType = std::unique_lock<typename Sync::mutex_type>;

    std::reference_wrapper<Sync> m_sync{};      ///< Reference to used synchronization object
    std::deque<TaskType> m_tasks{};             ///< Task queue which stores tasks with fifo strategy
    std::vector<ThreadJoinerType> m_workers{};  ///< Vector of thread pools worker threads
    bool is_shutdown{false};                    ///< Boolean for internal shutdown state

    /**
     * @brief Worker function
     *
     * Each thread of the thread pool calls this function initially. The threads are either waiting
     * in this function or processing the incoming tasks.
     */
    void work() {
        auto check_wait_condition{[this]() { return hasWork() || is_shutdown; }};
        auto execute_oldest_task{[this](TaskLockType& lock) { executeOldestTask(lock); }};

        while (!is_shutdown || hasWork()) {
            m_sync.get().waitThenExecute(check_wait_condition, execute_oldest_task);
        }
    }

    /**
     * @brief Checks if thread pool has work to do
     *
     * @returns True if tasks are queued, false otherwise
     */
    bool hasWork() {
        return !m_tasks.empty();
    }

    /**
     * @brief Executes the oldest task from the task queue
     *
     * This function either executes and removes the oldest task in the queue or finishes
     * directly when no task is remaining. The lock is unlocked before the task function
     * is executed to allow other tasks executions in parallel.
     *
     * @pre taskQueueLock must be already locked when function is executed
     *
     * @param taskQueueLock A unique lock which protectes the queue
     */
    void executeOldestTask(TaskLockType& taskQueueLock) {
        if (!hasWork()) {
            return;
        }

        auto task{popOldestTaskFromQueue()};
        taskQueueLock.unlock();
        task();
    }

    /**
     * @brief Gets and removes the oldest task from the queue
     *
     * @pre This function must be used in critical section
     *
     * @returns Oldest tasks from queue
     */
    TaskType popOldestTaskFromQueue() {
        auto task{std::move(m_tasks.back())};
        m_tasks.pop_back();
        return task;
    }

    /**
     * @brief Throws exception when shutdown state is set
     *
     * @pre This function must be used in critical section
     *
     * @exception std::runtime_error is thrown when the thread pool is already shut down and the
     * client is trying to enqueue tasks
     *
     * @see pool_party::detail::ThreadPool::enqueue
     */
    void throwWhenPoolIsShutDown() {
        if (is_shutdown) {
            throw std::runtime_error{"Thread pool already shut down, enqueuing failed."};
        }
    }
};

}  // namespace detail
}  // namespace pool_party

#endif  // POOL_PARTY_DETAIL_THREAD_POOL_HPP_
