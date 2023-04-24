//---------------------------------------------------------------------------//
// Copyright (c) 2023 Dmitrii Tabalin <d.tabalin@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE blueprint_plonk_non_native_bit_composition_test

#include <boost/test/unit_test.hpp>

#include <nil/crypto3/algebra/curves/pallas.hpp>
#include <nil/crypto3/algebra/fields/arithmetic_params/pallas.hpp>

#include <nil/crypto3/hash/keccak.hpp>

#include <nil/blueprint/blueprint/plonk/circuit.hpp>
#include <nil/blueprint/blueprint/plonk/assignment.hpp>
#include <nil/blueprint/components/algebra/fields/plonk/non_native/bit_composition.hpp>
#include <nil/blueprint/components/algebra/fields/plonk/non_native/bit_modes.hpp>

#include <boost/random/mersenne_twister.hpp>

#include <algorithm>

#include "../../../../test_plonk_component.hpp"

using namespace nil;

using mode = blueprint::components::bit_composition_mode;

template <typename BlueprintFieldType, std::uint32_t BitsAmount, mode Mode>
void test_bit_composition(std::array<bool, BitsAmount> &bits,
                          typename BlueprintFieldType::value_type expected_res){

    constexpr std::size_t WitnessColumns = 15;
    constexpr std::size_t PublicInputColumns = 1;
    constexpr std::size_t ConstantColumns = 0;
    constexpr std::size_t SelectorColumns = 3;
    using ArithmetizationParams =
        crypto3::zk::snark::plonk_arithmetization_params<WitnessColumns, PublicInputColumns, ConstantColumns, SelectorColumns>;
    using ArithmetizationType = crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>;
    using AssignmentType = blueprint::assignment<ArithmetizationType>;
    using hash_type = crypto3::hashes::keccak_1600<256>;
    constexpr std::size_t Lambda = 1;

    using var = crypto3::zk::snark::plonk_variable<BlueprintFieldType>;

    using component_type = blueprint::components::bit_composition<ArithmetizationType, 15, BitsAmount, Mode>;

    std::vector<typename BlueprintFieldType::value_type> public_input;
    public_input.resize(BitsAmount);
    for (std::size_t i = 0; i < BitsAmount; i++) {
        public_input[i] = typename BlueprintFieldType::value_type(bits[i]);
    }

    typename component_type::input_type instance_input;
    for (std::size_t i = 0; i < BitsAmount; i++) {
        instance_input.bits[i] = var(0, i, false, var::column_type::public_input);
    }

    auto result_check = [&expected_res](AssignmentType &assignment,
                                        typename component_type::result_type &real_res) {
        //std::cout << "Expected: " << expected_res.data << std::endl;
        //std::cout << "Real: " << var_value(assignment, real_res.output).data << std::endl;
        assert(expected_res == var_value(assignment, real_res.output));
    };

    component_type component_instance({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {}, {});

    crypto3::test_component<component_type, BlueprintFieldType, ArithmetizationParams, hash_type, Lambda>(
        component_instance, public_input, result_check, instance_input);
}

BOOST_AUTO_TEST_SUITE(blueprint_plonk_test_suite)

constexpr static const std::size_t random_tests_amount = 3;

template<typename BlueprintFieldType, std::uint32_t BitsAmount, mode Mode>
void calculate_expected_and_test_bit_decomposition(std::array<bool, BitsAmount> &bits) {

    typename BlueprintFieldType::value_type composed = 0;
    auto accumulator = [](typename BlueprintFieldType::value_type acc, bool b) {
        return typename BlueprintFieldType::value_type(2 * acc + (b ? 1 : 0));
    };
    if (Mode == mode::LSB) {
        composed = std::accumulate(bits.rbegin(), bits.rend(), composed, accumulator);
    } else {
        composed = std::accumulate(bits.begin(), bits.end(), composed, accumulator);
    }

    test_bit_composition<BlueprintFieldType, BitsAmount, Mode>(bits, composed);
}

template<typename BlueprintFieldType, std::uint32_t BitsAmount>
std::array<bool, BitsAmount> generate_random_bitstring(boost::random::mt19937 &rng) {
    std::array<bool, BitsAmount> res;
    for (std::size_t i = 0; i < BitsAmount; i++) {
        res[i] = rng() % 2;
    }
    return res;
}

BOOST_AUTO_TEST_CASE(blueprint_non_native_bit_decomposition_test1) {
    using field_type = typename crypto3::algebra::curves::pallas::base_field_type;
    boost::random::mt19937 rng;
    rng.seed(1337);

    // testing common sizes
    for (std::size_t j = 0; j < random_tests_amount; j++) {
        auto bits = generate_random_bitstring<field_type, 8>(rng);
        calculate_expected_and_test_bit_decomposition<field_type, 8, mode::MSB>(bits);
        calculate_expected_and_test_bit_decomposition<field_type, 8, mode::LSB>(bits);
    }
    for (std::size_t j = 0; j < random_tests_amount; j++) {
        auto bits = generate_random_bitstring<field_type, 16>(rng);
        calculate_expected_and_test_bit_decomposition<field_type, 16, mode::MSB>(bits);
        calculate_expected_and_test_bit_decomposition<field_type, 16, mode::LSB>(bits);
    }
    for (std::size_t j = 0; j < random_tests_amount; j++) {
        auto bits = generate_random_bitstring<field_type, 32>(rng);
        calculate_expected_and_test_bit_decomposition<field_type, 32, mode::MSB>(bits);
        calculate_expected_and_test_bit_decomposition<field_type, 32, mode::LSB>(bits);
    }
    for (std::size_t j = 0; j < random_tests_amount; j++) {
        auto bits = generate_random_bitstring<field_type, 64>(rng);
        calculate_expected_and_test_bit_decomposition<field_type, 64, mode::MSB>(bits);
        calculate_expected_and_test_bit_decomposition<field_type, 64, mode::LSB>(bits);
    }
    for (std::size_t j = 0; j < random_tests_amount; j++) {
        auto bits = generate_random_bitstring<field_type, 128>(rng);
        calculate_expected_and_test_bit_decomposition<field_type, 128, mode::MSB>(bits);
        calculate_expected_and_test_bit_decomposition<field_type, 128, mode::LSB>(bits);
    }
    // testing sizes which might be tricky for internal logic
    for (std::size_t j = 0; j < random_tests_amount; j++) {
        auto bits = generate_random_bitstring<field_type, 44>(rng);
        calculate_expected_and_test_bit_decomposition<field_type, 44, mode::MSB>(bits);
        calculate_expected_and_test_bit_decomposition<field_type, 44, mode::LSB>(bits);
    }
    for (std::size_t j = 0; j < random_tests_amount; j++) {
        auto bits = generate_random_bitstring<field_type, 45>(rng);
        calculate_expected_and_test_bit_decomposition<field_type, 45, mode::MSB>(bits);
        calculate_expected_and_test_bit_decomposition<field_type, 45, mode::LSB>(bits);
    }
    for (std::size_t j = 0; j < random_tests_amount; j++) {
        auto bits = generate_random_bitstring<field_type, 73>(rng);
        calculate_expected_and_test_bit_decomposition<field_type, 73, mode::MSB>(bits);
        calculate_expected_and_test_bit_decomposition<field_type, 73, mode::LSB>(bits);
    }
}

BOOST_AUTO_TEST_SUITE_END()