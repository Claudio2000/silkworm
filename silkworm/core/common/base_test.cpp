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

#include "base.hpp"

#include <bit>

#include <catch2/catch.hpp>

#include <silkworm/core/rlp/encode.hpp>

#include "util.hpp"

namespace silkworm {

TEST_CASE("Byteviews") {
    Bytes source{'0', '1', '2'};
    ByteView bv1(source);
    bv1.remove_prefix(3);
    REQUIRE(bv1.empty());
    ByteView bv2{};
    REQUIRE(bv2.empty());
    REQUIRE(bv1 == bv2);
    REQUIRE_FALSE(bv1.data() == bv2.data());
    REQUIRE(bv2.is_null());
}

TEST_CASE("Empty hashes") {
    const ByteView empty_string;
    const ethash::hash256 hash_of_empty_string{keccak256(empty_string)};
    CHECK(std::bit_cast<evmc_bytes32>(hash_of_empty_string) == kEmptyHash);

    const Bytes empty_list_rlp(1, rlp::kEmptyListCode);
    const ethash::hash256 hash_of_empty_list_rlp{keccak256(empty_list_rlp)};
    CHECK(std::bit_cast<evmc_bytes32>(hash_of_empty_list_rlp) == kEmptyListHash);

    // See https://github.com/ethereum/yellowpaper/pull/852
    const Bytes empty_string_rlp(1, rlp::kEmptyStringCode);
    const ethash::hash256 hash_of_empty_string_rlp{keccak256(empty_string_rlp)};
    CHECK(std::bit_cast<evmc_bytes32>(hash_of_empty_string_rlp) == kEmptyRoot);
}

}  // namespace silkworm
