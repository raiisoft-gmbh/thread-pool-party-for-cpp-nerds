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

#ifndef POOL_PARTY_THREAD_POOL_HPP_
#define POOL_PARTY_THREAD_POOL_HPP_

#include "detail/sync.hpp"
#include "detail/thread_factory.hpp"
#include "detail/thread_joiner.hpp"
#include "detail/thread_pool.hpp"

#include <condition_variable>
#include <cstddef>
#include <future>
#include <mutex>
#include <utility>

namespace pool_party {
/**
 * @brief ThreadPool implementation
 *
 * This class contains the thread pool client interface.
 */
class ThreadPool {
public:
    /**
     * @brief Constructor of ThreadPool
     *
     * This function creates all neccessary threads and prepares them to work on the
     * thread pools tasks.
     *
     * @param number_of_threads The number of threads the thread pool should consist of.
     */
    ThreadPool(std::size_t number_of_threads) : m_thread_pool{number_of_threads, m_thread_factory, m_sync} {}

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
        return m_thread_pool.enqueue(std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

    /**
     * @brief Shutdown the thread pool
     *
     * This function signals a shutdown to all worker threads. The threads will process the
     * remaining stored tasks. After shuting down the thread pool enqueuing is not allowed anymore.
     * All worker threads are going to join.
     */
    void shutdown() {
        m_thread_pool.shutdown();
    }

private:
    using SyncType          = pool_party::detail::Sync<std::mutex, std::condition_variable>;
    using ThreadType        = pool_party::detail::ThreadJoiner<std::thread>;
    using ThreadFactoryType = pool_party::detail::ThreadFactory<ThreadType>;
    using ThreadPoolType    = pool_party::detail::ThreadPool<ThreadFactoryType, SyncType>;

    SyncType m_sync{};                     ///< Sync object which synchronizes the worker threads
    ThreadFactoryType m_thread_factory{};  ///< Thread factory for creating worker threads
    ThreadPoolType m_thread_pool;          ///< Thread pool detail implementation
};

}  // namespace pool_party

#endif  // POOL_PARTY_THREAD_POOL_HPP_
