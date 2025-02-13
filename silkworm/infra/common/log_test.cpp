/*
   Copyright 2022 The Silkworm Authors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "log.hpp"

#include <iostream>
#include <string>
#include <thread>

#include <catch2/catch.hpp>

#include <silkworm/infra/test_util/log.hpp>

namespace silkworm::log {

//! Custom LogBuffer just for testing to access buffered content
template <Level level>
class LogBuffer_ForTest : public LogBuffer<level> {
  public:
    explicit LogBuffer_ForTest() : LogBuffer<level>() {}
    explicit LogBuffer_ForTest(std::string_view msg, Args args) : LogBuffer<level>(msg, args) {}

    [[nodiscard]] std::string content() const { return LogBuffer<level>::ss_.str(); }
};

//! Utility test function enforcing that log buffered content *IS* empty
template <Level level>
void check_log_empty() {
    auto log_buffer = LogBuffer_ForTest<level>();
    log_buffer << "test";
    CHECK(log_buffer.content().empty());
}

//! Utility test function enforcing that log buffered content *IS NOT* empty
template <Level level>
void check_log_not_empty() {
    auto log_buffer = LogBuffer_ForTest<level>();
    log_buffer << "test";
    CHECK(log_buffer.content().find("test") != std::string::npos);
}

//! Build the prettified key-value pair using color scheme
static std::string prettified_key_value(const std::string& key, const std::string& value) {
    std::string kv_pair{kColorGreen};
    kv_pair.append(key);
    kv_pair.append(kColorReset);
    kv_pair.append("=");
    kv_pair.append(kColorReset);
    kv_pair.append(kColorWhite);
    kv_pair.append(value);
    return kv_pair;
}

TEST_CASE("LogBuffer", "[silkworm][common][log]") {
    // Temporarily override std::cout and std::cerr with null stream to avoid terminal output
    test_util::StreamSwap cout_swap{std::cout, test_util::null_stream()};
    test_util::StreamSwap cerr_swap{std::cerr, test_util::null_stream()};

    SECTION("LogBuffer stores nothing for verbosity higher than default") {
        check_log_empty<Level::kDebug>();
        check_log_empty<Level::kTrace>();
    }

    SECTION("LogBuffer stores content for verbosity lower than or equal to default") {
        check_log_not_empty<Level::kInfo>();
        check_log_not_empty<Level::kWarning>();
        check_log_not_empty<Level::kError>();
        check_log_not_empty<Level::kCritical>();
        check_log_not_empty<Level::kNone>();
    }

    SECTION("LogBuffer stores nothing for verbosity higher than configured one") {
        test_util::SetLogVerbosityGuard guard{Level::kWarning};
        check_log_empty<Level::kInfo>();
        check_log_empty<Level::kDebug>();
        check_log_empty<Level::kTrace>();
    }

    SECTION("LogBuffer stores content for verbosity lower than or equal to configured one") {
        test_util::SetLogVerbosityGuard guard{Level::kWarning};
        check_log_not_empty<Level::kWarning>();
        check_log_not_empty<Level::kError>();
        check_log_not_empty<Level::kCritical>();
        check_log_not_empty<Level::kNone>();
    }

    SECTION("Settings enable/disable thread tracing") {
        // Default thread tracing
        std::stringstream thread_id_stream;
        thread_id_stream << std::this_thread::get_id();
        auto log_buffer1 = LogBuffer_ForTest<Level::kInfo>();
        log_buffer1 << "test";
        CHECK(log_buffer1.content().find(thread_id_stream.str()) == std::string::npos);

        // Enable thread tracing
        Settings log_settings;
        log_settings.log_threads = true;
        init(log_settings);
        auto log_buffer2 = LogBuffer_ForTest<Level::kInfo>();
        log_buffer2 << "test";
        CHECK(log_buffer2.content().find(thread_id_stream.str()) != std::string::npos);

        // Disable thread tracing
        log_settings.log_threads = false;
        init(log_settings);
        auto log_buffer3 = LogBuffer_ForTest<Level::kInfo>();
        log_buffer3 << "test";
        CHECK(log_buffer3.content().find(thread_id_stream.str()) == std::string::npos);
    }

    SECTION("Variable arguments: constructor") {
        auto log_buffer = LogBuffer_ForTest<Level::kInfo>("test", {"key1", "value1", "key2", "value2"});
        CHECK(log_buffer.content().find("test") != std::string::npos);
        CHECK(log_buffer.content().find(prettified_key_value("key1", "value1")) != std::string::npos);
        CHECK(log_buffer.content().find(prettified_key_value("key2", "value2")) != std::string::npos);
    }

    SECTION("Variable arguments: accumulators") {
        auto log_buffer = LogBuffer_ForTest<Level::kInfo>();
        log_buffer << "test" << Args{"key1", "value1", "key2", "value2"};
        CHECK(log_buffer.content().find("test") != std::string::npos);
        CHECK(log_buffer.content().find(prettified_key_value("key1", "value1")) != std::string::npos);
        CHECK(log_buffer.content().find(prettified_key_value("key2", "value2")) != std::string::npos);
    }
}

}  // namespace silkworm::log
