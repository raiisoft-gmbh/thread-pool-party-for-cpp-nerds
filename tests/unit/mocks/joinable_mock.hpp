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

#ifndef POOL_PARTY_TESTS_UNIT_MOCKS_JOINABLE_MOCK_HPP_
#define POOL_PARTY_TESTS_UNIT_MOCKS_JOINABLE_MOCK_HPP_

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace pool_party {
namespace test {

class JoinableMock {
public:
    MOCK_METHOD(void, join, ());
    MOCK_METHOD(bool, joinable, ());
};

using NiceJoinableMock = testing::NiceMock<JoinableMock>;

class NiceJoinableMockMovable {
public:
    explicit NiceJoinableMockMovable(NiceJoinableMock& mock) : m_mock{mock} {}

    void join() {
        m_mock.get().join();
    }

    bool joinable() {
        return m_mock.get().joinable();
    }

    NiceJoinableMock& getMock() {
        return m_mock;
    }

private:
    std::reference_wrapper<NiceJoinableMock> m_mock;
};

}  // namespace test
}  // namespace pool_party

#endif  // POOL_PARTY_TESTS_UNIT_MOCKS_JOINABLE_MOCK_HPP_
