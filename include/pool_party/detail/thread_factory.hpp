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

#ifndef POOL_PARTY_DETAIL_THREAD_FACTORY_HPP_
#define POOL_PARTY_DETAIL_THREAD_FACTORY_HPP_

#include <utility>

namespace pool_party {
namespace detail {

/**
 * @brief Factory to fabricate threads.
 *
 * This class is an abstraction for creating threads.
 *
 * @tparam ThreadType The thread type to fabricate.
 */
template<typename ThreadType>
class ThreadFactory {
public:
    using thread_type = ThreadType;

    /**
     * @brief Creates a thread of type ThreadType.
     *
     * @param thread_function The callable function which is passed to the thread.
     */
    template<typename Callable, typename... Args>
    ThreadType create(Callable&& thread_function, Args&&... args) {
        return ThreadType{std::forward<Callable>(thread_function), std::forward<Args>(args)...};
    }
};
}  // namespace detail
}  // namespace pool_party

#endif  // POOL_PARTY_DETAIL_THREAD_FACTORY_HPP_
