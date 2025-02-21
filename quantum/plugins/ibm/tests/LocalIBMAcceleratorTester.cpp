/***********************************************************************************
 * Copyright (c) 2017, UT-Battelle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the xacc nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributors:
 *   Initial API and implementation - Alex McCaskey
 *
 **********************************************************************************/
#include <memory>
#include <gtest/gtest.h>
#include "LocalIBMAccelerator.hpp"
#include "XACC.hpp"

using namespace xacc::quantum;

TEST(LocalIBMAcceleratorTester,checkSimple) {
    xacc::Initialize();

    // xacc::setOption("local-ibm-m-error-probs",".03,.08;.02,.07");

	auto acc = xacc::getAccelerator("local-ibm");
	acc->initialize();
	auto buffer = acc->createBuffer("qubits", 2);

	auto f = std::make_shared<GateFunction>("foo");

	auto h = std::make_shared<Hadamard>(0);
	auto h2 = std::make_shared<Hadamard>(1);
	// auto cn1 = std::make_shared<CNOT>(1, 2);
	// auto cn2 = std::make_shared<CNOT>(0, 1);
	// auto h2 = std::make_shared<Hadamard>(0);
	auto m0 = std::make_shared<Measure>(0, 0);
	auto m1 = std::make_shared<Measure>(1,1);
	// auto m2 = std::make_shared<Measure>(2,2);

	f->addInstruction(h);
    f->addInstruction(h2);
	f->addInstruction(m0);
    f->addInstruction(m1);

	acc->execute(buffer, f);

    for (auto& kv: buffer->getMeasurementCounts()) {
        std::cout << kv.first << ":" << kv.second << "\n";
    }
	xacc::Finalize();
}

int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
