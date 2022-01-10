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
// @file Declaration of interfaces for auxiliary components for the SHA256 component.
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_ZK_BLUEPRINT_PLONK_POSEIDON_5_WIRES_HPP
#define CRYPTO3_ZK_BLUEPRINT_PLONK_POSEIDON_5_WIRES_HPP

#include <nil/crypto3/zk/components/blueprint.hpp>
#include <nil/crypto3/zk/components/blueprint_variable.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace components {

                template<typename TArithmetization,
                         typename CurveType,
                         std::size_t W0 = 4,
                         std::size_t W1 = 0,
                         std::size_t W2 = 1,
                         std::size_t W3 = 2,
                         std::size_t W4 = 3>
                class poseidon_plonk;

                template<typename TBlueprintField,
                         typename CurveType,
                         std::size_t W0,
                         std::size_t W1,
                         std::size_t W2,
                         std::size_t W3,
                         std::size_t W4>
                class poseidon_plonk<snark::plonk_constraint_system<TBlueprintField, 5>,
                                                       CurveType,
                                                       W0,
                                                       W1,
                                                       W2,
                                                       W3,
                                                       W4>
                    : public detail::n_wires_helper<snark::plonk_constraint_system<TBlueprintField, 5>,
                                                    W0,
                                                    W1,
                                                    W2,
                                                    W3,
                                                    W4> {

                    typedef snark::plonk_constraint_system<TBlueprintField, 5> TArithmetization;
                    typedef blueprint<TArithmetization> blueprint_type;

                    constexpr algebra::matrix<typename CurveType::scalar_field_type::value_type, 3, 3> M;
                    constexpr algebra::vector<typename CurveType::scalar_field_type::value_type, 3> RC;

                    std::size_t j;

                    using n_wires_helper =
                        detail::n_wires_helper<snark::plonk_constraint_system<TBlueprintField, 5>,
                                               W0,
                                               W1,
                                               W2,
                                               W3,
                                               W4>;

                    using n_wires_helper::w;
                    enum indices { m2 = 0, m1, cur, p1, p2 };

                public:

                    poseidon_plonk(blueprint_type &bp) :
                        detail::n_wires_helper<TArithmetization, W0, W1, W2, W3, W4>(bp) {

                        j = bp.allocate_row();

                    }

                    void generate_gates() {
                        // For $j + 0$:
                        bp.add_gate(j + 0, w[4][cur] - (w[1][cur]^5 * M[0][0] + w[2][cur]^5 * M[0][1] + w[3][cur]^5 * M[0][2] + RC[0]));
                        bp.add_gate(j + 0, w[5][cur] - (w[1][cur]^5 * M[1][0] + w[2][cur]^5 * M[1][1] + w[3][cur]^5 * M[1][2] + RC[1]));
                        bp.add_gate(j + 0, w[6][cur] - (w[1][cur]^5 * M[2][0] + w[2][cur]^5 * M[2][1] + w[3][cur]^5 * M[2][2] + RC[2]));

                        bp.add_gate(j + 0, w[7][cur] - (w[3][cur]^5 * M[0][0] + w[4][cur]^5 * M[0][1] + w[5][cur]^5 * M[0][2] + RC[0]));
                        bp.add_gate(j + 0, w[8][cur] - (w[3][cur]^5 * M[1][0] + w[4][cur]^5 * M[1][1] + w[5][cur]^5 * M[1][2] + RC[1]));
                        bp.add_gate(j + 0, w[9][cur] - (w[3][cur]^5 * M[2][0] + w[4][cur]^5 * M[2][1] + w[5][cur]^5 * M[2][2] + RC[2]));

                        // For $j + 1$:
                        bp.add_gate(j + 1, w[1][cur] - (w[3][cur]^5 * M[0][0] + w[8][cur]^5 * M[0][1] + w[9][cur]^5 * M[0][2] + RC[0]));
                        bp.add_gate(j + 1, w[2][cur] - (w[3][cur]^5 * M[1][0] + w[8][cur]^5 * M[1][1] + w[9][cur]^5 * M[1][2] + RC[1]));
                        bp.add_gate(j + 1, w[3][cur] - (w[3][cur]^5 * M[2][0] + w[8][cur]^5 * M[2][1] + w[9][cur]^5 * M[2][2] + RC[2]));

                        bp.add_gate(j + 1, w[4][cur] - (w[1][cur]^5 * M[0][0] + w[2][cur]^5 * M[0][1] + w[3][cur]^5 * M[0][2] + RC[0]));
                        bp.add_gate(j + 1, w[5][cur] - (w[1][cur]^5 * M[1][0] + w[2][cur]^5 * M[1][1] + w[3][cur]^5 * M[1][2] + RC[1]));
                        bp.add_gate(j + 1, w[6][cur] - (w[1][cur]^5 * M[2][0] + w[2][cur]^5 * M[2][1] + w[3][cur]^5 * M[2][2] + RC[2]));

                        bp.add_gate(j + 1, w[7][cur] - (w[4][cur]^5 * M[0][0] + w[5][cur]^5 * M[0][1] + w[6][cur]^5 * M[0][2] + RC[0]));
                        bp.add_gate(j + 1, w[8][cur] - (w[4][cur]^5 * M[1][0] + w[5][cur]^5 * M[1][1] + w[6][cur]^5 * M[1][2] + RC[1]));
                        bp.add_gate(j + 1, w[9][cur] - (w[4][cur]^5 * M[2][0] + w[5][cur]^5 * M[2][1] + w[6][cur]^5 * M[2][2] + RC[2]));
                        
                        // For $j + k$, $k \in \{2, 19\}$:    
                        for (std::size_t z=2; z <= 19; z++){
                            bp.add_gate(j + z, w[1][cur] - (w[7][m1] * M[0][0] + w[8][m1] * M[0][1] + w[9][m1]^5 * M[0][2] + RC[0]));
                            bp.add_gate(j + z, w[2][cur] - (w[7][m1] * M[1][0] + w[8][m1] * M[1][1] + w[9][m1]^5 * M[1][2] + RC[1]));
                            bp.add_gate(j + z, w[3][cur] - (w[7][m1] * M[2][0] + w[8][m1] * M[2][1] + w[9][m1]^5 * M[2][2] + RC[2]));

                            bp.add_gate(j + z, w[4][cur] - (w[1][cur] * M[0][0] + w[2][cur] * M[0][1] + w[3][cur]^5 * M[0][2] + RC[0]));
                            bp.add_gate(j + z, w[5][cur] - (w[1][cur] * M[1][0] + w[2][cur] * M[1][1] + w[3][cur]^5 * M[1][2] + RC[1]));
                            bp.add_gate(j + z, w[6][cur] - (w[1][cur] * M[2][0] + w[2][cur] * M[2][1] + w[3][cur]^5 * M[2][2] + RC[2]));

                            bp.add_gate(j + z, w[7][cur] - (w[4][cur] * M[0][0] + w[5][cur] * M[0][1] + w[6][cur]^5 * M[0][2] + RC[0]));
                            bp.add_gate(j + z, w[8][cur] - (w[4][cur] * M[1][0] + w[5][cur] * M[1][1] + w[6][cur]^5 * M[1][2] + RC[1]));
                            bp.add_gate(j + z, w[9][cur] - (w[4][cur] * M[2][0] + w[5][cur] * M[2][1] + w[6][cur]^5 * M[2][2] + RC[2]));
                        }

                    }

                    void generate_assignments() {
                        
                    }
                };

            }    // namespace components
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_ZK_BLUEPRINT_PLONK_POSEIDON_5_WIRES_HPP
