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

#include "pool_party/detail/thread_factory.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>

namespace {
using testing::Eq;

class ThreadFake {
public:
    template<typename Callable, typename... Args>
    explicit ThreadFake(Callable&& callable, Args&&... args) {
        callable(std::forward<Args>(args)...);
    }
};
}  // namespace

class ThreadFactoryTests : public testing::Test {};

TEST_F(ThreadFactoryTests, CreateThreadWithFactory) {
    bool thread_function_called{false};

    pool_party::detail::ThreadFactory<ThreadFake> thread_factory{};

    auto thread_function{[&thread_function_called]() { thread_function_called = true; }};
    auto created_thread{thread_factory.create(thread_function)};
    EXPECT_TRUE(thread_function_called);
}

TEST_F(ThreadFactoryTests, PassACallableWithVariadicParameters) {
    int parameter{0};
    const int expected_parameter{42};

    pool_party::detail::ThreadFactory<ThreadFake> thread_factory{};

    auto thread_function{[&parameter](int p) { parameter = p; }};
    auto created_thread{thread_factory.create(thread_function, expected_parameter)};
    EXPECT_THAT(parameter, Eq(expected_parameter));
}
