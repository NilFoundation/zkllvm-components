//---------------------------------------------------------------------------//
// Copyright (c) 2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2021 Nikita Kaskov <nbering@nil.foundation>
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
// @file Declaration of interfaces for PLONK unified addition component.
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_ZK_BLUEPRINT_PLONK_UNIFIED_ADDITION_COMPONENT_11_WIRES_HPP
#define CRYPTO3_ZK_BLUEPRINT_PLONK_UNIFIED_ADDITION_COMPONENT_11_WIRES_HPP

#include <cmath>

#include <nil/marshalling/algorithms/pack.hpp>

#include <nil/crypto3/zk/snark/arithmetization/plonk/constraint_system.hpp>

#include <nil/crypto3/zk/blueprint/plonk.hpp>
#include <nil/crypto3/zk/assignment/plonk.hpp>
#include <nil/crypto3/zk/component.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace components {

                template<typename ArithmetizationType,
                         typename CurveType,
                         std::size_t... WireIndexes>
                class curve_element_unified_addition;

                template<typename BlueprintFieldType,
                         typename ArithmetizationParams,
                         typename CurveType,
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
                         std::size_t W10>
                class curve_element_unified_addition<
                    snark::plonk_constraint_system<BlueprintFieldType,
                        ArithmetizationParams>,
                    CurveType,
                    W0, W1, W2, W3, W4,
                    W5, W6, W7, W8, W9,
                    W10>{

                    typedef snark::plonk_constraint_system<BlueprintFieldType,
                        ArithmetizationParams> ArithmetizationType;

                    using var = snark::plonk_variable<BlueprintFieldType>;

                public:
                    constexpr static const std::size_t required_rows_amount = 1;

                    struct params_type {
                        struct var_ec_point {
                            var x = var(0, 0, false);
                            var y = var(0, 0, false);
                        };
                        
                        var_ec_point P;
                        var_ec_point Q;
                    };

                    // To obtain the result from outside:
                    // TODO: bind columns in result_type to the one actually used in
                    // circuit generation
                    struct result_type {
                        var X = var(0, 0, false);
                        var Y = var(0, 0, false);
                        result_type(const std::size_t row_start_index = 0) {
                            X = var(W4, row_start_index, false, var::column_type::witness);
                            Y = var(W5, row_start_index, false, var::column_type::witness);
                        }
                    };

                    static std::size_t allocate_rows (blueprint<ArithmetizationType> &bp){

                        return bp.allocate_rows(required_rows_amount);
                    }

                    static std::vector<std::size_t> allocate_rows (blueprint<ArithmetizationType> &bp,
                        std::size_t components_amount){

                        std::vector<std::size_t> rows(components_amount);

                        for (std::size_t i = 0; i < components_amount; i++) {
                            rows[i] = bp.allocate_rows(required_rows_amount);;
                        }

                        return rows;
                    }

                    static void generate_circuit(
                        blueprint<ArithmetizationType> &bp,
                        blueprint_assignment_table<ArithmetizationType> &assignment,
                        const std::vector<params_type> params,
                        const std::vector<std::size_t> row_start_indices){

                        assert(params.size() == row_start_indices.size());

                        generate_gates(bp, assignment, params, row_start_indices);
                        generate_copy_constraints(bp, assignment, params, row_start_indices);
                    }

                    static void generate_assignments(
                            blueprint_assignment_table<ArithmetizationType>
                                &assignment,
                            const std::vector<params_type> params,
                            const std::vector<std::size_t> row_start_indices){

                        assert(params.size() == row_start_indices.size());

                        for (std::size_t component_index = 0;
                            component_index < row_start_indices.size();
                            component_index++){

                            const std::size_t j = row_start_indices[component_index];
                            const params_type cur_params = params[component_index];

                            assignment.public_input(0)[0] = ArithmetizationType::field_type::value_type::zero();

                            typename BlueprintFieldType::value_type p_x = assignment.var_value(cur_params.P.x);
                            typename BlueprintFieldType::value_type p_y = assignment.var_value(cur_params.P.y);
                            typename CurveType::template 
                                g1_type<algebra::curves::coordinates::affine>::value_type P(p_x, p_y);

                            typename BlueprintFieldType::value_type q_x = assignment.var_value(cur_params.Q.x);
                            typename BlueprintFieldType::value_type q_y = assignment.var_value(cur_params.Q.y);  
                            typename CurveType::template 
                                g1_type<algebra::curves::coordinates::affine>::value_type Q(q_x, q_y);

                            const typename CurveType::template
                                g1_type<algebra::curves::coordinates::affine>::value_type R = P + Q;
                            
                            assignment.witness(W0)[j] = P.X;
                            assignment.witness(W1)[j] = P.Y;
                            assignment.witness(W2)[j] = Q.X;
                            assignment.witness(W3)[j] = Q.Y;
                            assignment.witness(W4)[j] = R.X;
                            assignment.witness(W5)[j] = R.Y;

                            // TODO: check, if this one correct:
                            assignment.witness(W6)[j] = R.is_zero();

                            if (P.X != Q.X){
                                assignment.witness(W7)[j] = 0;
                                assignment.witness(W8)[j] = (P.Y - Q.Y)/(P.X - Q.X);

                                assignment.witness(W9)[j] = 0;

                                assignment.witness(W10)[j] = (Q.X - P.X).inversed();
                            } else {
                                assignment.witness(W7)[j] = 1;

                                if (P.Y != Q.Y) { 
                                    assignment.witness(W9)[j] = (Q.Y - P.Y).inversed();
                                } else { // doubling
                                    if (P.Y != 0) {
                                        assignment.witness(W8)[j] = (3 * (P.X * P.X))/(2 * P.Y);
                                    } else {
                                        assignment.witness(W8)[j] = 0;
                                    }
                                    
                                    assignment.witness(W9)[j] = 0;
                                }

                                assignment.witness(W10)[j] = 0;
                            }
                        }
                    }

                private:
                    static void generate_gates(
                        blueprint<ArithmetizationType> &bp,
                        blueprint_assignment_table<ArithmetizationType> &assignment, 
                        const std::vector<params_type> params,
                        const std::vector<std::size_t> row_start_indices) {

                        assert(params.size() == row_start_indices.size());

                        std::size_t selector_index = assignment.add_selector(row_start_indices);

                        auto constraint_1 = bp.add_constraint(
                            var(W7, 0) * (var(W2, 0) - var(W0, 0)));
                        auto constraint_2 = bp.add_constraint(
                            (var(W2, 0) - var(W0, 0)) * var(W10, 0) - 
                            (1 - var(W7, 0)));
                        auto constraint_3 = bp.add_constraint(
                            var(W7, 0) * (2*var(W8, 0) * var(W1, 0) - 
                            3*(var(W0, 0) * var(W0, 0))) + (1 - var(W7, 0)) * 
                            ((var(W2, 0) - var(W0, 0)) * var(W8, 0) - 
                            (var(W3, 0) - var(W1, 0))));
                        auto constraint_4 = bp.add_constraint(
                            (var(W8, 0) * var(W8, 0)) - (var(W0, 0) + var(W2, 0) + var(W4, 0)));
                        auto constraint_5 = bp.add_constraint(
                            var(W5, 0) - (var(W8, 0) * (var(W0, 0) - 
                            var(W4, 0)) - var(W1, 0)));
                        auto constraint_6 = bp.add_constraint(
                            (var(W3, 0) - var(W1, 0)) * (var(W7, 0) - var(W6, 0)));
                        auto constraint_7 = bp.add_constraint(
                            (var(W3, 0) - var(W1, 0)) * var(W9, 0) - var(W6, 0));
                        
                        bp.add_gate(selector_index, 
                            { constraint_1, constraint_2, constraint_3,
                            constraint_4, constraint_5, constraint_6,
                            constraint_7
                        });
                    }

                    static void generate_copy_constraints(
                            blueprint<ArithmetizationType> &bp,
                            blueprint_assignment_table<ArithmetizationType> &assignment,
                            const std::vector<params_type> params,
                            const std::vector<std::size_t> row_start_indices){

                        std::size_t public_input_column_index = 0;
                        for (std::size_t component_index = 0;
                            component_index < row_start_indices.size();
                            component_index++){

                            const std::size_t j = row_start_indices[component_index];

                            bp.add_copy_constraint({{W6, static_cast<int>(j), false},
                                {public_input_column_index, 0, false, var::column_type::public_input}});
                        }
                    }
                };
            }    // namespace components
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_ZK_BLUEPRINT_PLONK_UNIFIED_ADDITION_COMPONENT_11_WIRES_HPP
