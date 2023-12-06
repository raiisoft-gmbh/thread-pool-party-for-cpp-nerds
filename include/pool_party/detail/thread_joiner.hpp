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

#ifndef POOL_PARTY_DETAIL_THREAD_JOINER_HPP_
#define POOL_PARTY_DETAIL_THREAD_JOINER_HPP_

#include <functional>
#include <utility>

namespace pool_party {
namespace detail {

/**
 * @brief Thread joiner joins a managed thread automatically on desctruction
 *
 * This function is abstraction for creating threads.
 *
 * @tparam Joinable The thread type to join.
 */
template<typename Joinable>
class ThreadJoiner {
public:
    /**
     * @brief Constructor for thread joiner
     *
     * Creates a new thread joiner which manages a joinable thread object.
     *
     * @param joinable Managed joinable
     */
    explicit ThreadJoiner(typename std::remove_reference<Joinable>::type&& joinable) :
            m_joinable{std::move(joinable)} {}
    ThreadJoiner(const ThreadJoiner&)            = default;
    ThreadJoiner& operator=(const ThreadJoiner&) = default;
    ThreadJoiner& operator=(ThreadJoiner&&)      = default;
    ThreadJoiner(ThreadJoiner&&)                 = default;

    /**
     * @brief Getter for managed joinable
     *
     * @return Reference to the managed joinable
     */
    Joinable& get() {
        return m_joinable;
    }

    /**
     * @brief Destructor joins the managed Joinable
     */
    ~ThreadJoiner() {
        if (m_joinable.joinable()) {
            m_joinable.join();
        }
    }

private:
    Joinable m_joinable;  ///< Managed joinable
};
}  // namespace detail
}  // namespace pool_party

#endif  // POOL_PARTY_DETAIL_THREAD_JOINER_HPP_
