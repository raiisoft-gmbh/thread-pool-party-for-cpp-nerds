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

#ifndef POOL_PARTY_DETAIL_SYNC_HPP_
#define POOL_PARTY_DETAIL_SYNC_HPP_

#include <functional>
#include <mutex>
#include <utility>

namespace pool_party {
namespace detail {

/**
 * @brief Class to synchronize pool threads
 *
 * @tparam MutexType Mutex type which is used for synchronization
 * @tparam ConditionVariableType CV type which is used for synced waiting
 */
template<typename MutexType, typename ConditionVariableType>
class Sync {
public:
    using mutex_type = MutexType;
    /**
     * @brief Blocking wait until sync object is notified
     *
     * This function can be called from multiple threads which all block
     * until the sync gets notified and the predicate is true.
     *
     * @tparam Predicate Callable type for checking function
     *
     * @param predicate Callable which is checked to be true when sync woke up
     * @param protected_func Callable which is call in a synchronized scope
     */
    template<typename Predicate, typename Callable>
    void wait(Predicate &&predicate, Callable &&protected_func) {
        std::unique_lock<MutexType> ul{m_mtx};
        m_cv.wait(ul, std::forward<Predicate>(predicate));
        protected_func(ul);
    }

    /**
     * @brief Notifies one waiting thread
     */
    void notifyOne() {
        m_cv.notify_one();
    }

    /**
     * @brief Notifies all waiting threads
     */
    void notifyAll() {
        m_cv.notify_all();
    }

    /**
     * @details Executes a callable protected by mutex used for synchronization
     *
     * @tparam Callable Type of callable which gets executed
     *
     * @param function_to_call Function is going to be called protected
     */
    template<typename Callable>
    void executeProtected(Callable &&function_to_call) {
        std::lock_guard<MutexType> lg{m_mtx};
        function_to_call();
    }

    /**
     * @brief Getter for internally used mutex reference
     *
     * @returns Mutex reference
     */
    MutexType &getMutex() {
        return m_mtx;
    }

    /**
     * @brief Getter for internally used condition variable reference
     *
     * @returns Condition variable reference
     */
    ConditionVariableType &getConditionVariable() {
        return m_cv;
    }

private:
    MutexType m_mtx{};             ///< Internal mutex
    ConditionVariableType m_cv{};  ///< Internal condition variable
};

}  // namespace detail
}  // namespace pool_party

#endif  // POOL_PARTY_DETAIL_SYNC_HPP_
