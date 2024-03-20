//---------------------------------------------------------------------------//
// Copyright (c) 2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2021 Nikita Kaskov <nbering@nil.foundation>
// Copyright (c) 2022 Ilia Shirobokov <i.shirobokov@nil.foundation>
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
// @file Declaration of interfaces for auxiliary components for the SHA256 component.
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_KIMCHI_DETAIL_MAP_FQ_HPP
#define CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_KIMCHI_DETAIL_MAP_FQ_HPP

#include <nil/marshalling/algorithms/pack.hpp>

#include <nil/crypto3/zk/snark/arithmetization/plonk/constraint_system.hpp>

#include <nil/blueprint/blueprint/plonk/circuit.hpp>
#include <nil/blueprint/blueprint/plonk/assignment.hpp>
#include <nil/blueprint/component.hpp>

#include <nil/blueprint/algorithms/generate_circuit.hpp>

#include <nil/blueprint/components/systems/snark/plonk/kimchi/types/proof.hpp>

#include <nil/blueprint/components/systems/snark/plonk/kimchi/detail/binding.hpp>

namespace nil {
    namespace blueprint {
        namespace components {

            // map_fq set copy constraints between input fq_data (which is input for scalar field components) and
            // recalculated fq_data (base field components output)
            // Input: common data (generated by the base part of the veridier) for scalar and base verifiers
            // Output: -
            template<typename ArithmetizationType,
                     typename CurveType,
                     typename KimchiParamsType,
                     typename KimchiCommitmentParamsType,
                     std::size_t BatchSize,
                     std::size_t... WireIndexes>
            class map_fq;

            template<typename CurveType,
                     typename KimchiParamsType,
                     typename KimchiCommitmentParamsType,
                     std::size_t BatchSize,
                     std::size_t W0,
                     std::size_t W1,
                     std::size_t W2,
                     std::size_t W3,
                     std::size_t W4,
                     std::size_t W5,
                     std::size_t W6,
                     std::size_t W7,
                     std::size_t W8,
                     std::size_t W9,
                     std::size_t W10,
                     std::size_t W11,
                     std::size_t W12,
                     std::size_t W13,
                     std::size_t W14>
            class map_fq<crypto3::zk::snark::plonk_constraint_system<typename CurveType::base_field_type>,
                         CurveType,
                         KimchiParamsType,
                         KimchiCommitmentParamsType,
                         BatchSize,
                         W0,
                         W1,
                         W2,
                         W3,
                         W4,
                         W5,
                         W6,
                         W7,
                         W8,
                         W9,
                         W10,
                         W11,
                         W12,
                         W13,
                         W14> {

                using BlueprintFieldType = typename CurveType::base_field_type;

                typedef crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType> ArithmetizationType;

                using var = crypto3::zk::snark::plonk_variable<typename BlueprintFieldType::value_type>;

                using proof_binding = typename binding<ArithmetizationType, BlueprintFieldType, KimchiParamsType>;

                using fq_data = typename proof_binding::template fq_data<var>;

                constexpr static const std::size_t selector_seed = 0x0f2D;

            public:
                constexpr static const std::size_t rows_amount = 0;
                constexpr static const std::size_t gates_amount = 0;

                struct params_type {
                    fq_data data_public;
                    fq_data data_recalculated;
                };

                struct result_type { };

                static result_type generate_circuit(blueprint<ArithmetizationType> &bp,
                                                    assignment<ArithmetizationType> &assignment,
                                                    const params_type &params,
                                                    const std::size_t start_row_index) {
                    std::size_t row = start_row_index;

                    generate_assignments_constant(bp, assignment, params, start_row_index);

                    return result_type();
                }

                static result_type generate_assignments(assignment<ArithmetizationType> &assignment,
                                                        const params_type &params,
                                                        std::size_t start_row_index) {

                    std::size_t row = start_row_index;

                    return result_type();
                }

            private:
                static void generate_gates(blueprint<ArithmetizationType> &bp,
                                           assignment<ArithmetizationType> &assignment,
                                           const params_type &params,
                                           std::size_t component_start_row = 0) {
                }

                static void generate_copy_constraints(blueprint<ArithmetizationType> &bp,
                                                      assignment<ArithmetizationType> &assignment,
                                                      const params_type &params,
                                                      std::size_t component_start_row = 0) {
                }

                static void generate_assignments_constant(blueprint<ArithmetizationType> &bp,
                                                          assignment<ArithmetizationType> &assignment,
                                                          const params_type &params,
                                                          std::size_t component_start_row) {
                    std::size_t row = component_start_row;
                }
            };
        }    // namespace components
    }    // namespace blueprint
}    // namespace nil

#endif    // CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_KIMCHI_DETAIL_MAP_FQ_HPP