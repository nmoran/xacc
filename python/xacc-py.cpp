/*******************************************************************************
 * Copyright (c) 2018 UT-Battelle, LLC.
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
#include "XACC.hpp"
#include "xacc_service.hpp"

#include "IRGenerator.hpp"
#include "IRProvider.hpp"
#include "InstructionIterator.hpp"
#include "AcceleratorBuffer.hpp"
#include "AcceleratorDecorator.hpp"
#include "InstructionParameter.hpp"
#include "EmbeddingAlgorithm.hpp"
#include "PauliOperator.hpp"

#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/eigen.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

namespace py = pybind11;
using namespace xacc;
using namespace xacc::quantum;

// `boost::variant` as an example -- can be any `std::variant`-like container
namespace pybind11 {
namespace detail {
template <typename... Ts>
struct type_caster<Variant<Ts...>> : variant_caster<Variant<Ts...>> {};

// Specifies the function used to visit the variant -- `apply_visitor` instead
// of `visit`
template <> struct visit_helper<Variant> {
  template <typename... Args>
  static auto call(Args &&... args) -> decltype(mpark::visit(args...)) {
    return mpark::visit(args...);
  }
};

template <typename... Ts>
struct type_caster<mpark::variant<Ts...>>
    : variant_caster<mpark::variant<Ts...>> {};

// Specifies the function used to visit the variant -- `apply_visitor` instead
// of `visit`
template <> struct visit_helper<mpark::variant> {
  template <typename... Args>
  static auto call(Args &&... args) -> decltype(mpark::visit(args...)) {
    return mpark::visit(args...);
  }
};
} // namespace detail
} // namespace pybind11

class PyAccelerator : public xacc::Accelerator {
public:
  /* Inherit the constructors */
  using Accelerator::Accelerator;

  const std::string name() const override {
    PYBIND11_OVERLOAD_PURE(const std::string, xacc::Accelerator, name);
  }

  const std::string description() const override { return ""; }

  void initialize() override { return; }

  AcceleratorType getType() override {
    return Accelerator::AcceleratorType::qpu_gate;
  }

  std::vector<std::shared_ptr<IRTransformation>>
  getIRTransformations() override {
    return {};
  }

  bool isValidBufferSize(const int n) override { return true; }

  /* Trampoline (need one for each virtual function) */
  void execute(std::shared_ptr<xacc::AcceleratorBuffer> buf,
               std::shared_ptr<xacc::Function> f) override {
    PYBIND11_OVERLOAD_PURE(void, xacc::Accelerator, execute, buf, f);
  }

  std::vector<std::shared_ptr<AcceleratorBuffer>>
  execute(std::shared_ptr<AcceleratorBuffer> buffer,
          const std::vector<std::shared_ptr<Function>> functions) override {
    return {};
  }

  std::shared_ptr<AcceleratorBuffer>
  createBuffer(const std::string &varId) override {
    return std::make_shared<AcceleratorBuffer>(varId, 100);
  }

  std::shared_ptr<AcceleratorBuffer> createBuffer(const std::string &varId,
                                                  const int size) override {
    return std::make_shared<AcceleratorBuffer>(varId, size);
  }
};

PYBIND11_MODULE(_pyxacc, m) {
  m.doc() =
      "Python bindings for XACC. XACC provides a plugin infrastructure for "
      "programming, compiling, and executing quantum kernels in a language and "
      "hardware agnostic manner.";

  py::class_<xacc::InstructionParameter>(
      m, "InstructionParameter",
      "The InstructionParameter provides a variant structure "
      "to provide parameters to XACC Instructions at runtime. "
      "This type can be an int, double, float, string, or complex value.")
      .def(py::init<int>(), "Construct as an int.")
      .def(py::init<double>(), "Construct as a double.")
      .def(py::init<std::string>(), "Construct as a string.")
      .def(py::init<std::complex<double>>(), "Construct as a complex double.");

  py::class_<xacc::Instruction, std::shared_ptr<xacc::Instruction>>(
      m, "Instruction", "")
      .def("nParameters", &xacc::Instruction::nParameters, "")
      .def("toString",
           (const std::string (xacc::Instruction::*)()) &
               xacc::Instruction::toString,
           "")
      .def("toString",
           (const std::string (xacc::Instruction::*)(const std::string &)) &
               xacc::Instruction::toString,
           "")
      .def("isEnabled", &xacc::Instruction::isEnabled, "")
      .def("isComposite", &xacc::Instruction::isComposite, "")
      .def("bits", &xacc::Instruction::bits, "")
      .def("getParameter", &xacc::Instruction::getParameter, "")
      .def("getParameters", &xacc::Instruction::getParameters, "")
      .def("setParameter", &xacc::Instruction::setParameter, "")
      .def("mapBits", &xacc::Instruction::mapBits, "")
      .def("name", &xacc::Instruction::name, "")
      .def("description", &xacc::Instruction::description, "");

  py::class_<xacc::Function, std::shared_ptr<xacc::Function>>(m, "Function", "")
      .def("nInstructions", &xacc::Function::nInstructions, "")
      .def("getInstruction", &xacc::Function::getInstruction, "")
      .def("getInstructions", &xacc::Function::getInstructions, "")
      .def("removeInstruction", &xacc::Function::removeInstruction, "")
      .def("replaceInstruction", &xacc::Function::replaceInstruction, "")
      .def("insertInstruction", &xacc::Function::insertInstruction, "")
      .def("addInstruction", &xacc::Function::addInstruction, "")
      .def("addParameter", &xacc::Function::addParameter, "")
      .def("nParameters", &xacc::Function::nParameters, "")
      .def("eval", &xacc::Function::operator(), "")
      .def("name", &xacc::Function::name, "")
      .def("description", &xacc::Function::description, "")
      .def("nParameters", &xacc::Function::nParameters, "")
      .def("toString",
           (const std::string (xacc::Function::*)()) & xacc::Function::toString,
           "")
      .def("toString",
           (const std::string (xacc::Function::*)(const std::string &)) &
               xacc::Function::toString,
           "")
      .def("enabledView", &xacc::Function::enabledView, "")
      .def("enable", &xacc::Function::enable, "")
      .def("getParameter", &xacc::Function::getParameter, "")
      .def("getParameters", &xacc::Function::getParameters, "")
      .def("setParameter", &xacc::Function::setParameter, "")
      .def("depth", &xacc::Function::depth, "")
      .def("persistGraph", &xacc::Function::persistGraph, "")
      .def("mapBits", &xacc::Function::mapBits, "");

  // Expose the IR interface
  py::class_<xacc::IR, std::shared_ptr<xacc::IR>>(
      m, "IR",
      "The XACC Intermediate Representation, "
      "serves as a container for XACC Functions.")
      .def("getKernels", &xacc::IR::getKernels, "Return the kernels in this IR")
      .def("mapBits", &xacc::IR::mapBits, "")
      .def("addKernel", &xacc::IR::addKernel, "");

  py::class_<xacc::InstructionIterator>(m, "InstructionIterator", "")
      .def(py::init<std::shared_ptr<xacc::Function>>())
      .def("hasNext", &xacc::InstructionIterator::hasNext, "")
      .def("next", &xacc::InstructionIterator::next, "");


  py::class_<xacc::IRTransformation, std::shared_ptr<xacc::IRTransformation>>(
      m, "IRTransformation", "")
      .def("transform", &xacc::IRTransformation::transform, "");

  py::class_<xacc::IRGenerator, std::shared_ptr<xacc::IRGenerator>>(
      m, "IRGenerator", "")
      .def("generate",
           (std::shared_ptr<xacc::Function>(xacc::IRGenerator::*)(
               std::vector<xacc::InstructionParameter>)) &
               xacc::IRGenerator::generate,
           py::return_value_policy::reference, "")
      .def("generate",
           (std::shared_ptr<xacc::Function>(xacc::IRGenerator::*)(
               std::map<std::string, xacc::InstructionParameter>&)) &
               xacc::IRGenerator::generate,
           py::return_value_policy::reference, "")
      .def("analyzeResults", &xacc::IRGenerator::analyzeResults, "");

  // Expose the Accelerator
  py::class_<xacc::Accelerator, std::shared_ptr<xacc::Accelerator>,
             PyAccelerator>
      acc(m, "Accelerator",
          "Accelerator wraps the XACC C++ Accelerator class "
          "and provides a mechanism for creating buffers of qubits. Execution "
          "is handled by the XACC Kernels.");
  acc.def(py::init<>())
      .def("name", &xacc::Accelerator::name,
           "Return the name of this Accelerator.")
      .def("createBuffer",
           (std::shared_ptr<xacc::AcceleratorBuffer>(xacc::Accelerator::*)(
               const std::string &, const int)) &
               xacc::Accelerator::createBuffer,
           py::return_value_policy::reference,
           "Return a newly created register of qubits of the given size.")
      .def(
          "createBuffer",
          (std::shared_ptr<xacc::AcceleratorBuffer>(xacc::Accelerator::*)(
              const std::string &)) &
              xacc::Accelerator::createBuffer,
          py::return_value_policy::reference,
          "Return a newly allocated register of all qubits on the Accelerator.")
      .def("execute",
           (void (xacc::Accelerator::*)(std::shared_ptr<AcceleratorBuffer>,
                                        const std::shared_ptr<Function>)) &
               xacc::Accelerator::execute,
           "Execute the Function with the given AcceleratorBuffer.")
      .def("execute",
           (std::vector<std::shared_ptr<AcceleratorBuffer>>(
               xacc::Accelerator::*)(
               std::shared_ptr<AcceleratorBuffer>,
               const std::vector<std::shared_ptr<Function>>)) &
               xacc::Accelerator::execute,
           "Execute the Function with the given AcceleratorBuffer.")
      .def("initialize", &xacc::Accelerator::initialize, "")
      .def("getOneBitErrorRates", &xacc::Accelerator::getOneBitErrorRates, "")
      .def("getTwoBitErrorRates", &xacc::Accelerator::getTwoBitErrorRates, "");

  py::enum_<Accelerator::AcceleratorType>(acc, "AcceleratorType")
      .value("qpu_aqc", Accelerator::AcceleratorType::qpu_aqc)
      .value("qpu_gate", Accelerator::AcceleratorType::qpu_gate)
      .export_values();

  py::class_<xacc::AcceleratorDecorator, xacc::Accelerator,
             std::shared_ptr<xacc::AcceleratorDecorator>>
      accd(m, "AcceleratorDecorator", "");
  accd.def("setDecorated", &xacc::AcceleratorDecorator::setDecorated, "");

  // Expose the AcceleratorBuffer
  py::class_<xacc::AcceleratorBuffer, std::shared_ptr<xacc::AcceleratorBuffer>>(
      m, "AcceleratorBuffer",
      "The AcceleratorBuffer models a register of qubits.")
      .def(py::init<const std::string &, const int>())
      .def("getExpectationValueZ",
           &xacc::AcceleratorBuffer::getExpectationValueZ,
           "Return the expectation value with respect to the Z operator.")
      .def("resetBuffer", &xacc::AcceleratorBuffer::resetBuffer,
           "Reset this buffer for use in another computation.")
      .def("size", &xacc::AcceleratorBuffer::size, "")
      .def("appendMeasurement",
           (void (xacc::AcceleratorBuffer::*)(const std::string &)) &
               xacc::AcceleratorBuffer::appendMeasurement,
           "Append the measurement string")
      .def("getMeasurements",
           &xacc::AcceleratorBuffer::getMeasurements,
           "Return observed measurement bit strings")
      .def("computeMeasurementProbability",
           &xacc::AcceleratorBuffer::computeMeasurementProbability,
           "Compute the probability of a given bit string")
      .def("getMeasurementCounts",
           &xacc::AcceleratorBuffer::getMeasurementCounts,
           "Return the mapping of measure bit strings to their counts.")
      .def("getChildren",
           (std::vector<std::shared_ptr<AcceleratorBuffer>>(
               xacc::AcceleratorBuffer::*)(const std::string)) &
               xacc::AcceleratorBuffer::getChildren,
           "")
      .def("nChildren", &xacc::AcceleratorBuffer::nChildren, "")
      .def("getChildren",
           (std::vector<std::shared_ptr<AcceleratorBuffer>>(
               xacc::AcceleratorBuffer::*)()) &
               xacc::AcceleratorBuffer::getChildren,
           "")
      .def("getChildrenNames", &xacc::AcceleratorBuffer::getChildrenNames, "")
      .def("listExtraInfoKeys", &xacc::AcceleratorBuffer::listExtraInfoKeys, "")
      .def("getInformation",
           (std::map<std::string, ExtraInfo>(xacc::AcceleratorBuffer::*)()) &
               xacc::AcceleratorBuffer::getInformation,
           "")
      .def("addExtraInfo",
           (void (xacc::AcceleratorBuffer::*)(const std::string, ExtraInfo)) &
               xacc::AcceleratorBuffer::addExtraInfo,
           "")
      .def("getInformation",
           (ExtraInfo(xacc::AcceleratorBuffer::*)(const std::string)) &
               xacc::AcceleratorBuffer::getInformation,
           "")
      .def("appendChild", &xacc::AcceleratorBuffer::appendChild, "")
      .def("hasExtraInfoKey", &xacc::AcceleratorBuffer::hasExtraInfoKey, "")
      .def("name", &xacc::AcceleratorBuffer::name, "")
      .def("getAllUnique", &xacc::AcceleratorBuffer::getAllUnique,
           "Return all unique information with the provided string name")
      .def("__repr__", &xacc::AcceleratorBuffer::toString, "")
      .def("__str__", &xacc::AcceleratorBuffer::toString, "")
      .def("getChildren",
           (std::vector<std::shared_ptr<AcceleratorBuffer>>(
               xacc::AcceleratorBuffer::*)(const std::string, ExtraInfo)) &
               xacc::AcceleratorBuffer::getChildren,
           "");

  // Expose the Compiler
  py::class_<xacc::Compiler, std::shared_ptr<xacc::Compiler>>(
      m, "Compiler",
      "The XACC Compiler takes as input quantum kernel source code, "
      "and compiles it to the XACC intermediate representation.")
      .def("name", &xacc::Compiler::name, "Return the name of this Compiler.")
      .def("compile",
           (std::shared_ptr<xacc::IR>(xacc::Compiler::*)(
               const std::string &, std::shared_ptr<xacc::Accelerator>)) &
               xacc::Compiler::compile,
           "Compile the "
           "given source code against the given Accelerator.")
      .def("compile",
           (std::shared_ptr<xacc::IR>(xacc::Compiler::*)(const std::string &)) &
               xacc::Compiler::compile,
           "Compile the "
           "given source code.")
      .def("translate", &xacc::Compiler::translate,
           "Translate the given IR Function instance to source code in this "
           "Compiler's language.");

  py::class_<Term>(m, "Term").def("coeff", &Term::coeff).def("ops",&Term::ops);
  py::class_<PauliOperator>(m, "PauliOperator")
      .def(py::init<>())
      .def(py::init<std::complex<double>>())
      .def(py::init<double>())
      .def(py::init<std::string>())
      .def(py::init<std::map<int, std::string>>())
      .def(py::init<std::map<int, std::string>, double>())
      .def(py::init<std::map<int, std::string>, std::complex<double>>())
      .def(py::init<std::map<int, std::string>, std::string>())
      .def(py::init<std::map<int, std::string>, std::complex<double>,
                    std::string>())
      .def(py::self + py::self)
      .def(py::self += py::self)
      .def(py::self *= py::self)
      .def(py::self *= double())
      .def(py::self * py::self)
      .def(py::self *= std::complex<double>())
      .def(py::self -= py::self)
      .def(py::self - py::self)
      .def("__eq__", &PauliOperator::operator==)
      .def("__repr__", &PauliOperator::toString)
      .def("eval", &PauliOperator::eval)
      .def("toBinaryVectors", &PauliOperator::toBinaryVectors)
      .def("toXACCIR", &PauliOperator::toXACCIR)
      .def("fromXACCIR", &PauliOperator::fromXACCIR)
      .def("fromString", &PauliOperator::fromString)
      .def("nTerms", &PauliOperator::nTerms)
      .def("isClose", &PauliOperator::isClose)
      .def("commutes", &PauliOperator::commutes)
      .def("__len__", &PauliOperator::nTerms)
      .def("nQubits", &PauliOperator::nQubits)
      .def("computeActionOnKet", &PauliOperator::computeActionOnKet)
      .def("computeActionOnBra", &PauliOperator::computeActionOnBra)
      .def("__iter__",
           [](PauliOperator &op) {
             return py::make_iterator(op.begin(), op.end());
           },
           py::keep_alive<0, 1>());

  // Expose XACC API functions
  m.def("Initialize", (void (*)(std::vector<std::string>)) & xacc::Initialize,
        "Initialize the framework. Can provide a list of strings to model "
        "command line arguments. For instance, "
        "to print the XACC help message, pass ['--help'] to this function, or "
        "to set the compiler to use, ['--compiler','scaffold'].");
  m.def("Initialize", (void (*)()) & xacc::Initialize,
        "Initialize the framework. Use this if there are no command line "
        "arguments to pass.");
  m.def("PyInitialize", &xacc::PyInitialize,
        "Initialize the framework from Python.");
  // m.def("help", )
  m.def("getAccelerator",
        (std::shared_ptr<xacc::Accelerator>(*)(const std::string &)) &
            xacc::getAccelerator,
        py::return_value_policy::reference,
        "Return the accelerator with given name.");
  m.def("getCompiler",
        (std::shared_ptr<xacc::Compiler>(*)(const std::string &)) &
            xacc::getCompiler,
        py::return_value_policy::reference,
        "Return the Compiler of given name.");
  m.def("getIRTransformation",
        (std::shared_ptr<xacc::IRTransformation>(*)(const std::string &)) &
            xacc::getService<IRTransformation>,
        py::return_value_policy::reference,
        "Return the IRTransformation of given name.");
  m.def("getIRGenerator",
        (std::shared_ptr<xacc::IRGenerator>(*)(const std::string &)) &
            xacc::getService<IRGenerator>,
        py::return_value_policy::reference,
        "Return the IRGenerator of given name.");
  m.def("getConnectivity",
        [](const std::string acc) -> std::vector<std::pair<int,int>> {
          auto a = xacc::getAccelerator(acc);
          return a->getAcceleratorConnectivity();
        });
  m.def("translate", &xacc::translate,
        "Translate the provided IR Function to the given language.");
  m.def("setOption", [](const std::string s, InstructionParameter p) {
    xacc::setOption(s, p.toString());
  });
  m.def(
      "info", [](const std::string s) { xacc::info(s); }, "");
  m.def(
      "error", [](const std::string s) { xacc::error(s); }, "");
  m.def("setOption", &xacc::setOption, "Set an XACC framework option.");
  m.def(
      "setOptions",
      [](std::map<std::string, std::string> options) {
        for (auto &kv : options)
          xacc::setOption(kv.first, kv.second);
      },
      "Set a number of options at once.");
  m.def("getAcceleratorDecorator",
        [](const std::string name, std::shared_ptr<Accelerator> acc) {
          auto accd = xacc::getService<AcceleratorDecorator>(name);
          accd->setDecorated(acc);
          return accd;
        });
  m.def(
      "setOptions",
      [](std::map<std::string, InstructionParameter> options) {
        for (auto &kv : options)
          xacc::setOption(kv.first, kv.second.toString());
      },
      "Set a number of options at once.");
  m.def("unsetOption", &xacc::unsetOption, "");
  m.def("getOption",
        (const std::string (*)(const std::string &)) & xacc::getOption,
        "Get an XACC framework option.");
  m.def("hasAccelerator", &xacc::hasAccelerator,
        "Does XACC have the given Accelerator installed?");
  m.def("hasCompiler", &xacc::hasCompiler,
        "Does XACC have the given Accelerator installed?");
  m.def("optionExists", &xacc::optionExists, "Set an XACC framework option.");
  m.def("setIsPyApi", &xacc::setIsPyApi,
        "Indicate that this is using XACC via the Python API.");
  m.def("optimizeFunction", &xacc::optimizeFunction,
        "Run an optimizer on the given function.");
  m.def("analyzeBuffer",
        (void (*)(std::shared_ptr<AcceleratorBuffer>)) & xacc::analyzeBuffer,
        "Analyze the AcceleratorBuffer to produce problem-specific results.");
  m.def("getCache", &xacc::getCache, "");
  m.def(
      "appendCache",
      (void (*)(const std::string, const std::string, InstructionParameter &,const std::string)) &
          xacc::appendCache,
      "");
  m.def("Finalize", &xacc::Finalize, "Finalize the framework");
  m.def(
      "loadBuffer",
      [](const std::string json) {
        std::istringstream s(json);
        auto buffer = std::make_shared<AcceleratorBuffer>();
        buffer->load(s);
        return buffer;
      },
      "");
  m.def(
      "compileKernel",
      [](std::shared_ptr<Accelerator> acc, const std::string &src,
         const std::string &compilerName = "") -> std::shared_ptr<Function> {
        if (!compilerName.empty())
          xacc::setOption("compiler", compilerName);

        auto compiler = xacc::getCompiler();
        auto ir = compiler->compile(src, acc);
        return ir->getKernels()[0];
      },
      py::arg("acc"), py::arg("src"), py::arg("compilerName") = std::string(""),
      py::return_value_policy::move, "");

  m.def(
      "functionToInstruction",
      [](std::shared_ptr<Function> f) {
        return std::dynamic_pointer_cast<Instruction>(f);
      },
      py::return_value_policy::copy);

  py::module gatesub =
      m.def_submodule("gate", "Gate model quantum computing data structures.");
  gatesub.def(
      "create",
      [](const std::string &name, std::vector<int> qbits,
         std::vector<InstructionParameter> params =
             std::vector<InstructionParameter>{})
          -> std::shared_ptr<Instruction> {
        return xacc::getService<IRProvider>("gate")->createInstruction(
            name, qbits, params);
      },
      "Convenience function for creating a new GateInstruction.",
      py::arg("name"), py::arg("qbits"),
      py::arg("params") = std::vector<InstructionParameter>{});
  gatesub.def(
      "createFunction",
      [](const std::string &name, std::vector<int> qbits,
         std::vector<InstructionParameter> params =
             std::vector<InstructionParameter>{}) -> std::shared_ptr<Function> {
        return xacc::getService<IRProvider>("gate")->createFunction(name, qbits,
                                                                    params);
      },
      "Convenience function for creating a new GateFunction.", py::arg("name"),
      py::arg("qbits"),
      py::arg("params") = std::vector<InstructionParameter>{});
  gatesub.def(
      "createIR",
      []() -> std::shared_ptr<IR> {
        return xacc::getService<IRProvider>("gate")->createIR();
      },
      "Convenience function for creating a new GateIR.");

  gatesub.def(
      "getState",
      [](std::shared_ptr<Accelerator> acc, std::shared_ptr<Function> f) {
        auto results = acc->getAcceleratorState(f);
        Eigen::VectorXcd ret =
            Eigen::Map<Eigen::VectorXcd>(results.data(), results.size());
        return ret;
      },
      "Compute and return the state after execution of the given program on "
      "the given accelerator.");

  py::module aqcsub =
      m.def_submodule("dwave", "Gate model quantum computing data structures.");
  aqcsub.def(
      "create",
      [](const std::string &name, std::vector<int> qbits,
         std::vector<InstructionParameter> params =
             std::vector<InstructionParameter>{})
          -> std::shared_ptr<Instruction> {
        return xacc::getService<IRProvider>("dwave")->createInstruction(
            name, qbits, params);
      },
      "Convenience function for creating a new DWInstruction.", py::arg("name"),
      py::arg("qbits"),
      py::arg("params") = std::vector<InstructionParameter>{});
  aqcsub.def(
      "createFunction",
      [](const std::string &name, std::vector<int> qbits,
         std::vector<InstructionParameter> params =
             std::vector<InstructionParameter>{}) -> std::shared_ptr<Function> {
        return xacc::getService<IRProvider>("dwave")->createFunction(
            name, qbits, params);
      },
      "Convenience function for creating a new DWFunction.", py::arg("name"),
      py::arg("qbits"),
      py::arg("params") = std::vector<InstructionParameter>{});
  aqcsub.def(
      "createIR",
      []() -> std::shared_ptr<IR> {
        return xacc::getService<IRProvider>("dwave")->createIR();
      },
      "Convenience function for creating a new DWIR.");

  aqcsub.def(
      "embed",
      [](std::shared_ptr<Function> f, std::shared_ptr<Accelerator> acc,
         const std::string algo) -> std::map<int, std::vector<int>> {
        auto a = xacc::getService<xacc::quantum::EmbeddingAlgorithm>(algo);
        auto hardwareconnections = acc->getAcceleratorConnectivity();
        std::set<int> nUniqueBits;
        for (auto& edge : hardwareconnections) {
            nUniqueBits.insert(edge.first);
            nUniqueBits.insert(edge.second);
        }

        int nBits = *std::max_element(nUniqueBits.begin(), nUniqueBits.end()) + 1;

        auto hardware = xacc::getService<Graph>("boost-ugraph");
        for (int i = 0; i < nBits; i++) {
            std::map<std::string,InstructionParameter> m{{"bias",1.0}};
            hardware->addVertex(m);
        }

        for (auto& edge : hardwareconnections) {
            hardware->addEdge(edge.first, edge.second);
        }

        int maxBitIdx = 0;
        for (auto inst : f->getInstructions()) {
          if (inst->name() == "dw-qmi") {
            auto qbit1 = inst->bits()[0];
            auto qbit2 = inst->bits()[1];
            if (qbit1 > maxBitIdx) {
              maxBitIdx = qbit1;
            }
            if (qbit2 > maxBitIdx) {
              maxBitIdx = qbit2;
            }
          }
        }

        auto problemGraph = xacc::getService<Graph>("boost-ugraph");
        for (int i = 0; i < maxBitIdx+1;i++) {
            std::map<std::string,InstructionParameter> m{{"bias",1.0}};
            problemGraph->addVertex(m);
        }

        for (auto inst : f->getInstructions()) {
          if (inst->name() == "dw-qmi") {
            auto qbit1 = inst->bits()[0];
            auto qbit2 = inst->bits()[1];
            if (qbit1 != qbit2) {
              problemGraph->addEdge(qbit1, qbit2, 1.0);
            }
          }
        }


        return a->embed(problemGraph, hardware);
      },
      "");
}
