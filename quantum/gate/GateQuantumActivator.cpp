/*******************************************************************************
 * Copyright (c) 2017 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompanies this
 * distribution. The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html and the Eclipse Distribution
 *License is available at https://eclipse.org/org/documents/edl-v10.php
 *
 * Contributors:
 *   Alexander J. McCaskey - initial API and implementation
 *******************************************************************************/
#include "QFT.hpp"
#include "InverseQFT.hpp"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceProperties.h"

#include "GateIRProvider.hpp"
#include "DigitalGates.hpp"

#include "CircuitOptimizer.hpp"

#include "ROErrorDecorator.hpp"
#include "ImprovedSamplingDecorator.hpp"
#include "RichExtrapDecorator.hpp"

#include <memory>
#include <set>

using namespace cppmicroservices;

namespace {

/**
 */
class US_ABI_LOCAL GateQuantumActivator : public BundleActivator {

public:
  GateQuantumActivator() {}

  /**
   */
  void Start(BundleContext context) {
    auto qft = std::make_shared<xacc::quantum::QFT>();
    auto iqft = std::make_shared<xacc::quantum::InverseQFT>();

    auto giservice = std::make_shared<xacc::quantum::GateIRProvider>();

    auto opt = std::make_shared<xacc::quantum::CircuitOptimizer>();

    auto roed = std::make_shared<xacc::quantum::ROErrorDecorator>();
    auto impsamplingd = std::make_shared<xacc::quantum::ImprovedSamplingDecorator>();
    auto richextrap = std::make_shared<xacc::quantum::RichExtrapDecorator>();

    context.RegisterService<xacc::IRProvider>(giservice);
    context.RegisterService<xacc::IRGenerator>(iqft);
    context.RegisterService<xacc::IRGenerator>(qft);

    context.RegisterService<xacc::IRTransformation>(opt);
    context.RegisterService<xacc::OptionsProvider>(opt);

    context.RegisterService<xacc::AcceleratorDecorator>(roed);
    context.RegisterService<xacc::Accelerator>(roed);

    context.RegisterService<xacc::AcceleratorDecorator>(richextrap);
    context.RegisterService<xacc::Accelerator>(richextrap);

    context.RegisterService<xacc::AcceleratorDecorator>(impsamplingd);
    context.RegisterService<xacc::Accelerator>(impsamplingd);

    auto h = std::make_shared<xacc::quantum::Hadamard>();
    auto cn = std::make_shared<xacc::quantum::CNOT>();
    auto cp = std::make_shared<xacc::quantum::CPhase>();
    auto cz = std::make_shared<xacc::quantum::CZ>();
    auto id = std::make_shared<xacc::quantum::Identity>();
    auto m = std::make_shared<xacc::quantum::Measure>();
    auto rx = std::make_shared<xacc::quantum::Rx>();
    auto ry = std::make_shared<xacc::quantum::Ry>();
    auto rz = std::make_shared<xacc::quantum::Rz>();
    auto x = std::make_shared<xacc::quantum::X>();
    auto y = std::make_shared<xacc::quantum::Y>();
    auto z = std::make_shared<xacc::quantum::Z>();
    auto sw = std::make_shared<xacc::quantum::Swap>();
    auto u = std::make_shared<xacc::quantum::U>();

    auto s = std::make_shared<xacc::quantum::S>();
    auto sdg = std::make_shared<xacc::quantum::Sdg>();
    auto t = std::make_shared<xacc::quantum::T>();
    auto tdg = std::make_shared<xacc::quantum::Tdg>();

    auto cy = std::make_shared<xacc::quantum::CY>();
    auto crz= std::make_shared<xacc::quantum::CRZ>();
    auto ch = std::make_shared<xacc::quantum::CH>();

    context.RegisterService<xacc::quantum::GateInstruction>(h);
    context.RegisterService<xacc::quantum::GateInstruction>(cn);
    context.RegisterService<xacc::quantum::GateInstruction>(cp);
    context.RegisterService<xacc::quantum::GateInstruction>(cz);
    context.RegisterService<xacc::quantum::GateInstruction>(id);
    context.RegisterService<xacc::quantum::GateInstruction>(m);
    context.RegisterService<xacc::quantum::GateInstruction>(rx);
    context.RegisterService<xacc::quantum::GateInstruction>(ry);
    context.RegisterService<xacc::quantum::GateInstruction>(rz);
    context.RegisterService<xacc::quantum::GateInstruction>(x);
    context.RegisterService<xacc::quantum::GateInstruction>(y);
    context.RegisterService<xacc::quantum::GateInstruction>(z);
    context.RegisterService<xacc::quantum::GateInstruction>(sw);
    context.RegisterService<xacc::quantum::GateInstruction>(u);

    context.RegisterService<xacc::quantum::GateInstruction>(s);
    context.RegisterService<xacc::quantum::GateInstruction>(sdg);
    context.RegisterService<xacc::quantum::GateInstruction>(t);
    context.RegisterService<xacc::quantum::GateInstruction>(tdg);
    context.RegisterService<xacc::quantum::GateInstruction>(cy);
    context.RegisterService<xacc::quantum::GateInstruction>(crz);
    context.RegisterService<xacc::quantum::GateInstruction>(ch);

  }

  /**
   */
  void Stop(BundleContext /*context*/) {}
};

} // namespace

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(GateQuantumActivator)
