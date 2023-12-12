//---------------------------------------------------------------------------//
// Copyright (c) 2023 Alexey Yashunsky <a.yashunsky@nil.foundation>
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
// @file Declaration of the exponentiation for BLS12-381 pairing
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_BLS12_EXPONENTIATION_HPP
#define CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_BLS12_EXPONENTIATION_HPP

#include <nil/crypto3/algebra/fields/detail/element/fp12_2over3over2.hpp>
#include <nil/crypto3/algebra/fields/fp12_2over3over2.hpp>

#include <nil/crypto3/zk/snark/arithmetization/plonk/constraint_system.hpp>

#include <nil/blueprint/blueprint/plonk/assignment.hpp>
#include <nil/blueprint/blueprint/plonk/circuit.hpp>
#include <nil/blueprint/component.hpp>
#include <nil/blueprint/manifest.hpp>

#include <nil/blueprint/components/algebra/fields/plonk/non_native/detail/abstract_fp12.hpp>
#include <nil/blueprint/components/algebra/fields/plonk/non_native/detail/fp12_frobenius_coefs.hpp>

#include <nil/blueprint/components/algebra/pairing/weierstrass/plonk/detail/fp12_power_t.hpp>
#include <nil/blueprint/components/algebra/pairing/weierstrass/plonk/detail/fp12_power_tminus1sq_over3.hpp>
namespace nil {
    namespace blueprint {
        namespace components {
            //
            // Component for raising to power N = (p^12 - 1)/(t^4 - t^2 + 1)
            // with -t = 0xD201000000010000 in F_p^12
            // Input: x[12]
            // Output: y[12]: y = x^N as elements of F_p^12
            //
            // We realize the circuit in two versions - 12-column and 24-column.
            //

            using namespace detail;

            template<typename ArithmetizationType, typename BlueprintFieldType>
            class bls12_exponentiation;

            template<typename BlueprintFieldType, typename ArithmetizationParams>
            class bls12_exponentiation<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>,
                           BlueprintFieldType>
                : public plonk_component<BlueprintFieldType, ArithmetizationParams, 0, 0> {

            static std::size_t gates_amount_internal(std::size_t witness_amount) {
                return (witness_amount == 12) ? 9 : 10;
            }

            public:
                using component_type = plonk_component<BlueprintFieldType, ArithmetizationParams, 0, 0>;

                using var = typename component_type::var;
                using manifest_type = plonk_component_manifest;
                using power_tm1sq3_type = fp12_power_tm1sq3<
                    crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>,
                    BlueprintFieldType>;
                using power_t_type = fp12_power_t<
                    crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>,
                    BlueprintFieldType>;

                class gate_manifest_type : public component_gate_manifest {
                public:
                    std::size_t witness_amount;

                    gate_manifest_type(std::size_t witness_amount_) : witness_amount(witness_amount_) {}

                    std::uint32_t gates_amount() const override {
                        return bls12_exponentiation::gates_amount_internal(witness_amount);
                    }

                    bool operator<(const component_gate_manifest *other) const override {
                        return (witness_amount < dynamic_cast<const gate_manifest_type*>(other)->witness_amount);
                    }
                };

                static gate_manifest get_gate_manifest(std::size_t witness_amount,
                                                       std::size_t lookup_column_amount) {
                    static gate_manifest manifest =
                        gate_manifest(gate_manifest_type(witness_amount)) ;
//                        .merge_with(power_t_type::get_gate_manifest(witness_amount,lookup_column_amount));
                    return manifest;
                }

                static manifest_type get_manifest() {
                    static manifest_type manifest = manifest_type(
                        std::shared_ptr<manifest_param>(new manifest_range_param(12,24,12)), // 12 or 24
                        false
                    ).merge_with(power_t_type::get_manifest());
                    return manifest;
                }

                constexpr static std::size_t get_rows_amount(std::size_t witness_amount,
                                                             std::size_t lookup_column_amount) {
                    return ((witness_amount == 12)? (7 + 3 + 3 + 4 + 10) : (4 + 2 + 2 + 2 + 6)) +
                            power_tm1sq3_type::get_rows_amount(witness_amount,lookup_column_amount) +
                            3 * power_t_type::get_rows_amount(witness_amount, lookup_column_amount);
                }

                const std::size_t rows_amount = get_rows_amount(this->witness_amount(), 0);

                struct input_type {
                    std::array<var,12> x;

                    std::vector<var> all_vars() const {
                        std::vector<var> res = {};
                        for(auto & e : x) { res.push_back(e); }
                        return res;
                    }
                };

                struct result_type {
		    std::array<var,12> output;

                    result_type(const bls12_exponentiation &component, std::uint32_t start_row_index) {
                        const std::size_t WA = component.witness_amount();
                        std::size_t last_row = start_row_index + component.rows_amount - 1;

                        for(std::size_t i = 0; i < 12; i++) {
                            output[i] = var(component.W(i), last_row, false, var::column_type::witness);
                        }
                    }

                    std::vector<var> all_vars() const {
                        std::vector<var> res = {};

                        for(auto & e : output) { res.push_back(e); }
                        return res;
                    }
                };

                template<typename ContainerType>
                explicit bls12_exponentiation(ContainerType witness) : component_type(witness, {}, {}, get_manifest()) {};

                template<typename WitnessContainerType, typename ConstantContainerType,
                         typename PublicInputContainerType>
                bls12_exponentiation(WitnessContainerType witness, ConstantContainerType constant,
                         PublicInputContainerType public_input) :
                    component_type(witness, constant, public_input, get_manifest()) {};

                bls12_exponentiation(
                    std::initializer_list<typename component_type::witness_container_type::value_type>
                        witnesses,
                    std::initializer_list<typename component_type::constant_container_type::value_type>
                        constants,
                    std::initializer_list<typename component_type::public_input_container_type::value_type>
                        public_inputs) :
                    component_type(witnesses, constants, public_inputs, get_manifest()) {};
            };

            template<typename BlueprintFieldType, typename ArithmetizationParams>
            using plonk_bls12_exponentiation =
                bls12_exponentiation<
                    crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>,
                    BlueprintFieldType>;

            template<typename BlueprintFieldType, typename ArithmetizationParams>
            typename plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>::result_type generate_assignments(
                const plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams> &component,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>>
                    &assignment,
                const typename plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>::input_type
                    &instance_input,
                const std::uint32_t start_row_index) {

                using component_type = plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>;
                using var = typename component_type::var;
                using value_type = typename BlueprintFieldType::value_type;

                typename BlueprintFieldType::integral_type field_p = BlueprintFieldType::modulus;
                const std::size_t WA = component.witness_amount();

                std::array<value_type,12> x;

                for(std::size_t i = 0; i < 12; i++) {
                    x[i] = var_value(assignment, instance_input.x[i]);
                    assignment.witness(component.W(i),start_row_index) = x[i];
                }

                using policy_type_fp12 = crypto3::algebra::fields::fp12_2over3over2<BlueprintFieldType>;
                using fp12_element = typename policy_type_fp12::value_type;

                fp12_element F = fp12_element({ {x[0],x[1]}, {x[2],x[3]}, {x[4],x[5]} }, { {x[6],x[7]}, {x[8],x[9]}, {x[10],x[11]} }),
                             Y = F;
                std::size_t slot = 0;

                auto fill_slot = [&assignment, &component, &start_row_index, &slot, &WA](fp12_element V) {
                    for(std::size_t i = 0; i < 12; i++) {
                        assignment.witness(component.W((12*slot + i) % WA),start_row_index + (12*slot)/WA) =
                            V.data[i/6].data[(i % 6)/2].data[i % 2];
                    }
                    slot++;
                };

                auto use_block = [&assignment, &component, &start_row_index, &WA]<typename block_type>
                                               (block_type B, std::size_t input_slot, std::size_t &row) {
                    std::array<var,12> transfer_vars;
                    for(std::size_t i = 0; i < 12; i++) {
                        transfer_vars[i] = var(component.W((12*input_slot + i) % WA),start_row_index + (12*input_slot)/WA,false);
                    }
                    typename block_type::input_type block_input = {transfer_vars};
                    typename block_type::result_type block_res = generate_assignments(B, assignment, block_input, row);
                    row += B.rows_amount;

                    std::array<value_type,12> v;
                    for(std::size_t i = 0; i < 12; i++) {
                        v[i] = var_value(assignment, block_res.output[i]);
                    }
                    return fp12_element({ {v[0],v[1]}, {v[2],v[3]}, {v[4],v[5]} }, { {v[6],v[7]}, {v[8],v[9]}, {v[10],v[11]} });
                };

                using power_tm1sq3_type = typename component_type::power_tm1sq3_type;
                using power_t_type = typename component_type::power_t_type;

                power_tm1sq3_type power_tm1sq3_instance( component._W, component._C, component._PI);
                power_t_type power_t_instance( component._W, component._C, component._PI);

                fill_slot(F.inversed()); // F^{-1}
                fill_slot(F); // F
                Y = F.unitary_inversed(); fill_slot(Y); // F^{p^6} = conjugated(F) = conjugated(a + wb) = a - wb
                fill_slot(F.inversed()); // F^{-1}
                Y = Y * F.inversed(); fill_slot(Y); // F^{p^6 - 1}
                fill_slot(Y.pow(field_p).pow(field_p)); // (F^{p^6 -1})^{p^2}
                Y = Y * Y.pow(field_p).pow(field_p); fill_slot(Y); // (F^{p^6 -1})^{p^2 + 1}

                // ---------------- start of g = y^{(1-t)^2/3}
                // g, g^{-1} and g^{p^3}
                slot--; // rewind to last slot
                std::size_t current_row = start_row_index + (12*slot)/WA + 1;
                fp12_element G = use_block(power_tm1sq3_instance,slot,current_row); // this computes g = y^{(1-t)^2/3}
                slot = ((current_row - start_row_index)*WA)/12;
                fill_slot(G.inversed()); fill_slot(G); // G^{-1}, G
                if (WA == 24) { fill_slot(G); } // additional slot for alignment when WA = 24
                fill_slot(G.pow(field_p).pow(field_p).pow(field_p)); // G^{p^3}

                // ---------------- start of g^{-t}
                slot--; // rewind to last slot
                current_row = start_row_index + (12*slot)/WA + 1;
                slot--; // take argument from the one but last slot (i.e. = G)
                fp12_element Gmt = use_block(power_t_instance,slot,current_row); // this computes G^{-t}
                slot = ((current_row - start_row_index)*WA)/12;
                std::size_t gmt_slot = slot; // save for future use
                fill_slot(Gmt); // G^{-t}
                Gmt = Gmt.inversed(); fill_slot(Gmt); // G^t
                if (WA == 24) { fill_slot(Gmt); } // additional slot for alignment when WA = 24
                Gmt = Gmt.pow(field_p).pow(field_p); fill_slot(Gmt); // (G^t)^{p^2}

                // ---------------- start of g^{t^2}
                current_row = start_row_index + (12*slot)/WA; // slot is pointing to the start of an empty row now
                fp12_element Gt2 = use_block(power_t_instance,gmt_slot,current_row); // this computes G^{t^2}
                slot = ((current_row - start_row_index)*WA)/12;
                fill_slot(Gt2); fill_slot(G.inversed()); // g^{t^2}, g^{-1}
                Gt2 = Gt2 * G.inversed(); fill_slot(Gt2); // g^{t^2-1},
                Gt2 = Gt2.pow(field_p); fill_slot(Gt2); // (g^{t^2-1})^p

                // ---------------- start of g^{-t(t^2-1)}
                current_row = start_row_index + (12*slot)/WA; // slot is pointing to the start of an empty row now
                fp12_element Gt3 = use_block(power_t_instance,slot-2,current_row);
                slot = ((current_row - start_row_index)*WA)/12;
                fill_slot(Gt3); fill_slot(Gt3.inversed()); // g^{-t(t^2-1)}, g^{t(t^2-1)}
                if (WA == 24) { fill_slot(Gt3.inversed()); } // additional copy for alignment
                fill_slot(Y); // get a very old value y, g = y^{(1-t)^2/3}
                Y = Y*Gt3.inversed(); fill_slot(Y); // y g^{t(t^2-1)}
                fill_slot(G.pow(field_p).pow(field_p).pow(field_p)); // g^{p^3}
                Y = Y * G.pow(field_p).pow(field_p).pow(field_p); fill_slot(Y); // y g^{t(t^2-1)} g^{p^3}
                fill_slot(Gt2); // (g^{t^2-1})^p
                Y = Y * Gt2; fill_slot(Y); // y g^{t(t^2-1)} g^{p^3} (g^{t^2-1})^p
                fill_slot(Gmt); // (g^t)^{p^2}
                Y = Y * Gmt; fill_slot(Y); // y g^{t(t^2-1)} g^{p^3} (g^{t^2-1})^p (g^t)^{p^2}

                return typename plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>::result_type(
                    component, start_row_index);
	    }

            template<typename BlueprintFieldType, typename ArithmetizationParams>
            std::vector<std::size_t> generate_gates(
                const plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams> &component,
                circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>> &bp,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>>
                    &assignment,
                const typename plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>::input_type
                    &instance_input) {

                using var = typename plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>::var;
                using constraint_type = crypto3::zk::snark::plonk_constraint<BlueprintFieldType>;

                using fp12_constraint = detail::abstract_fp12_element<constraint_type>;

                const std::size_t WA = component.witness_amount();
                std::vector<std::size_t> gate_list = {};

                fp12_constraint X, Y, Z, C;

                // inversion gate
                for(std::size_t i = 0; i < 12; i++) {
                    X[i] = var(component.W(i), -(WA == 12), true);
                    Y[i] = var(component.W((i+12) % WA), 0, true);
                }
                C = X * Y;

                std::vector<constraint_type> inversion_constrs = {};
                for(std::size_t i = 0; i < 12; i++) {
                    inversion_constrs.push_back(C[i] - (i > 0? 0 : 1));
                }
                gate_list.push_back(bp.add_gate(inversion_constrs));

                // conjugation gate (first column = conj(second column, prev row))
                for(std::size_t i = 0; i < 12; i++) {
                    X[i] = var(component.W((i+12) % WA), -1, true);
                    Y[i] = var(component.W(i), 0, true);
                }
                std::vector<constraint_type> conjugation_constrs = {};
                for(std::size_t i = 0; i < 6; i++) {
                    conjugation_constrs.push_back(X[i] - Y[i]);
                }
                for(std::size_t i = 6; i < 12; i++) {
                    conjugation_constrs.push_back(X[i] + Y[i]);
                }
                gate_list.push_back(bp.add_gate(conjugation_constrs));

                // power p^k gates
                std::array<std::array<constraint_type,2>,6> Z2;
                std::array<typename BlueprintFieldType::value_type,12> F;
                for( auto &Power : { p_one, p_two, p_three } ) {
                    for(std::size_t i = 0; i < 12; i++) {
                        X[i] = var(component.W(i), -(WA == 12), true);
                        Y[i] = var(component.W((i+12) % WA), 0, true);
                    }
                    F = get_fp12_frobenius_coefficients<BlueprintFieldType>(Power);
                    for(std::size_t i = 0; i < 6; i++) {
                        // i -> i/2 + 3(i % 2) is a rearrangement of coefficients by increasing w powers, w.r.t. v = w²
                        Z2[i][0] = X[2*(i/2 + 3*(i % 2))];
                        // Fp2 elements are conjugated when raising to p and p^3
                        Z2[i][1] = X[2*(i/2 + 3*(i % 2)) + 1] * (Power != p_two ? -1 : 1);

                        C[2*(i/2 + 3*(i % 2))] = Z2[i][0]*F[2*i] - Z2[i][1]*F[2*i+1];
                        C[2*(i/2 + 3*(i % 2)) + 1] = Z2[i][0]*F[2*i+1] + Z2[i][1]*F[2*i];
                    }

                    std::vector<constraint_type> frobenius_constrs = {};
                    for(std::size_t i = 0; i < 12; i++) {
                        frobenius_constrs.push_back(C[i] - Y[i]);
                    }
                    gate_list.push_back(bp.add_gate(frobenius_constrs));
                }
               // multiplication gate
                for(std::size_t i = 0; i < 12; i++) {
                    X[i] = var(component.W(i), -(WA == 12), true);
                    Y[i] = var(component.W((i+12) % WA), 0, true);
                    Z[i] = var(component.W(i), 1, true);
                }
                C = X * Y;

                std::vector<constraint_type> mult_constrs = {};
                for(std::size_t i = 0; i < 12; i++) {
                    mult_constrs.push_back(C[i] - Z[i]);
                }
                gate_list.push_back(bp.add_gate(mult_constrs));

                return gate_list;
            }

            template<typename BlueprintFieldType, typename ArithmetizationParams>
            void generate_copy_constraints(
                const plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams> &component,
                circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>> &bp,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>>
                    &assignment,
                const typename plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>::input_type &instance_input,
                const std::size_t start_row_index) {

                using var = typename plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>::var;

                const std::size_t WA = component.witness_amount();

                std::size_t slot = 1; // location of initial data
                for(std::size_t i = 0; i < 12; i++) {
                    bp.add_copy_constraint({var(component.W((12*slot + i) % WA), start_row_index + (12*slot)/WA, false),
                                                instance_input.x[i]});
                }

                std::vector<std::array<std::size_t,2>> pairs = (WA == 12)?
                    std::vector<std::array<std::size_t,2>>{{0,3}, {111,202}, {6,249}, {113,251}, {204,253}, {158,255}} :
                    std::vector<std::array<std::size_t,2>>{{0,3}, {120,217}, {6,267}, {123,269}, {219,271}, {171,273}, {121,122}, {169,170}, {265,266}};
                for( std::array<std::size_t,2> pair : pairs ) {
                    for(std::size_t i = 0; i < 12; i++) {
                        bp.add_copy_constraint({var(component.W((12*pair[0] + i) % WA), start_row_index + (12*pair[0])/WA, false),
                                                var(component.W((12*pair[1] + i) % WA), start_row_index + (12*pair[1])/WA, false)});
                    }
                }
            }

            template<typename BlueprintFieldType, typename ArithmetizationParams>
            typename plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>::result_type generate_circuit(
                const plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams> &component,
                circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>> &bp,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>>
                    &assignment,
                const typename plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>::input_type &instance_input,
                const std::size_t start_row_index) {

                const std::size_t WA = component.witness_amount();

                std::vector<std::size_t> selector_index = generate_gates(component, bp, assignment, instance_input);

                auto apply_selector = [&assignment, &selector_index, &start_row_index](
                    std::size_t gate_id, std::vector<std::size_t> apply_list) {
                    for( std::size_t row : apply_list ) {
                        assignment.enable_selector(selector_index[gate_id], start_row_index + row);
                    }
                };

                // inversion gate #0
                apply_selector(0, (WA == 12)? std::vector<std::size_t>{1,112,157,248} : std::vector<std::size_t>{0,60,84,132});

                // conjugation gate #1
                apply_selector(1, (WA == 12)? std::vector<std::size_t>{2} : std::vector<std::size_t>{1});

                // Frobenius gates (powers p, p^2, p^3) ## 2,3,4
                std::array<std::size_t,3> frobenius_line = (WA == 12) ?
                                                           std::array<std::size_t,3>{204,5,113} :
                                                           std::array<std::size_t,3>{109,2,61};
                for(std::size_t i = 0; i < 3; i++) {
                    apply_selector(i+2, {frobenius_line[i]});
                }
                // one more power p^2 gate #3
                apply_selector(3,(WA == 12)? std::vector<std::size_t>{158} : std::vector<std::size_t>{85});

                // multiplication gate #5
                apply_selector(5, (WA == 12)? std::vector<std::size_t>{3,5,202,249,251,253,255} :
                                              std::vector<std::size_t>{1,2,108,133,134,135,136});

                using component_type = plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>;
                using var = typename component_type::var;
                using power_tm1sq3_type = typename component_type::power_tm1sq3_type;
                using power_t_type = typename component_type::power_t_type;

                power_tm1sq3_type power_tm1sq3_instance( component._W, component._C, component._PI);
                power_t_type power_t_instance( component._W, component._C, component._PI);

                std::size_t slot = 6;
                std::array<var,12> transfer_vars;
                for(std::size_t i = 0; i < 12; i++) {
                    transfer_vars[i] = var(component.W((12*slot + i) % WA),start_row_index + (12*slot)/WA,false);
                }
                typename power_tm1sq3_type::input_type power_tm1sq3_input = {transfer_vars};
                std::size_t current_row = start_row_index + (12*slot)/WA + 1;
                typename power_tm1sq3_type::result_type power_tm1sq3_res =
                    generate_circuit(power_tm1sq3_instance, bp, assignment, power_tm1sq3_input, current_row);

                slot = (WA == 12)? 112 : 121; // slot for linking output
                for(std::size_t i = 0; i < 12; i++) {
                    bp.add_copy_constraint({power_tm1sq3_res.output[i],
                                            var(component.W((12*slot + i) % WA), start_row_index + (12*slot)/WA, false)});
                }

                std::array<std::size_t,3> block_start,
                                          block_input,
                                          block_output;
                if (WA == 12) {
                    block_start = {114,159,205};
                    block_input = {112,156,203};
                    block_output= {156,201,247};
                } else {
                    block_start = {124,172,220};
                    block_input = {122,168,218};
                    block_output= {168,216,264};
                }

                for(std::size_t j = 0; j < block_start.size(); j++) {
                    std::array<var,12> transfer_vars;
                    std::size_t input_slot = block_input[j];
                    for(std::size_t i = 0; i < 12; i++) {
                        transfer_vars[i] = var(component.W((12*input_slot + i) % WA),start_row_index + (12*input_slot)/WA,false);
                    }
                    typename power_t_type::input_type power_t_input = {transfer_vars};
                    current_row = start_row_index + (12*block_start[j])/WA;
                    typename power_t_type::result_type power_t_res =
                        generate_circuit(power_t_instance, bp, assignment, power_t_input, current_row);
                    std::size_t output_slot = block_output[j];
                    for(std::size_t i = 0; i < 12; i++) {
                        bp.add_copy_constraint({power_t_res.output[i],
                                            var(component.W((12*output_slot + i) % WA), start_row_index + (12*output_slot)/WA, false)});
                    }
                }

                generate_copy_constraints(component, bp, assignment, instance_input, start_row_index);

                return typename plonk_bls12_exponentiation<BlueprintFieldType, ArithmetizationParams>::result_type(
                    component, start_row_index);
            }
        }    // namespace components
    }        // namespace blueprint
}    // namespace nil

#endif    // CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_BLS12_EXPONENTIATION_HPP