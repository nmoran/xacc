/*******************************************************************************
 * Copyright (c) 2017 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompanies this
 * distribution. The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html and the Eclipse Distribution
 * License is available at https://eclipse.org/org/documents/edl-v10.php
 *
 * Contributors:
 *   Alexander J. McCaskey - initial API and implementation
 *******************************************************************************/
#ifndef QUANTUM_GATEQIR_QFUNCTION_HPP_
#define QUANTUM_GATEQIR_QFUNCTION_HPP_

#include "Function.hpp"
#include "IRProvider.hpp"
#include "GateInstruction.hpp"
#include "XACC.hpp"
#include "exprtk.hpp"


namespace xacc {

class Graph;

namespace quantum {

static constexpr double pi = 3.141592653589793238;

using symbol_table_t = exprtk::symbol_table<double>;
using expression_t = exprtk::expression<double>;
using parser_t = exprtk::parser<double>;

/**
 * The GateFunction is a realization of Function for gate-model
 * quantum computing. It is composed of GateInstructions that
 * are themselves derivations of the Instruction class.
 */
class GateFunction : public Function,
                     public std::enable_shared_from_this<GateFunction> {

public:
  /**
   * The constructor, takes the function unique id and its name.
   *
   * @param id
   * @param name
   */
  GateFunction(const std::string &name)
      : functionName(name), parameters(std::vector<InstructionParameter>{}) {}
  GateFunction(const std::string &name,
               std::vector<InstructionParameter> params)
      : functionName(name), parameters(params) {}

  GateFunction(const std::string &name, const std::string &_tag)
      : functionName(name), parameters(std::vector<InstructionParameter>{}),
        tag(_tag) {}
  GateFunction(const std::string &name, const std::string &_tag,
               std::vector<InstructionParameter> params)
      : functionName(name), parameters(params), tag(_tag) {}

  GateFunction(const GateFunction &other)
      : functionName(other.functionName), parameters(other.parameters) {}

  virtual void mapBits(std::vector<int> bitMap) override;

  const int nInstructions() override;
  const int nRequiredBits() const override {
    XACCLogger::instance()->error(
        "GateFunction nRequiredBits() not implemented.");
    return 0;
  }

  void setBitMap(const std::vector<int> bMap) override {bitMap = bMap;}
  const std::vector<int> getBitMap() override {return bitMap;}
  bool hasBeenBitMapped() override {return !bitMap.empty();}

  void persist(std::ostream &outStream) override;
  void load(std::istream &inStream) override;

  InstPtr getInstruction(const int idx) override;

  std::list<InstPtr> getInstructions() override;

  std::shared_ptr<Function> enabledView() override {
    auto newF = std::make_shared<GateFunction>(functionName, parameters);
    for (int i = 0; i < nInstructions(); i++) {
      auto inst = getInstruction(i);
      if (inst->isEnabled()) {
        // FIXME CLONE and add parameters and bits...
        newF->addInstruction(inst);
      }
    }
    return newF;
  }

  void enable() override {
    for (int i = 0; i < nInstructions(); i++) {
      getInstruction(i)->enable();
    }
  }

  const bool isAnalog() const override {
    for (int i = 0; i < instructions.size(); i++) {
      auto inst = *std::next(instructions.begin(), i);
      if (inst->isAnalog()) {
        return true;
      }
    }
    return false;
  }

  void expandIRGenerators(
      std::map<std::string, InstructionParameter> irGenMap) override;
  bool hasIRGenerators() override;
  /**
   * Remove an instruction from this
   * quantum intermediate representation
   *
   * @param instructionID
   */
  void removeInstruction(const int idx) override;

  /**
   * Add an instruction to this quantum
   * intermediate representation.
   *
   * @param instruction
   */
  void addInstruction(InstPtr instruction) override;

  /**
   * Replace the given current quantum instruction
   * with the new replacingInst quantum Instruction.
   *
   * @param currentInst
   * @param replacingInst
   */
  void replaceInstruction(const int idx, InstPtr replacingInst) override;

  void insertInstruction(const int idx, InstPtr newInst) override;

  /**
   * Return the name of this function
   * @return
   */
  const std::string name() const override;

  /**
   * Return the description of this instance
   * @return description The description of this object.
   */
  const std::string description() const override { return ""; }

  /**
   * Return the qubits this function acts on.
   * @return
   */
  const std::vector<int> bits() override;

  const int depth() override;
  const std::string persistGraph() override;

  /**
   * Return an assembly-like string representation for this function .
   * @param bufferVarName
   * @return
   */
  const std::string toString(const std::string &bufferVarName) override;

  /**
   * Return this instruction's assembly-like string
   * representation.
   *
   * @return qasm The instruction as qasm
   */
  const std::string toString() override { return toString("q"); }

  InstructionParameter getParameter(const int idx) const override;

  void setParameter(const int idx, InstructionParameter &p) override;

  void addParameter(InstructionParameter instParam) override;

  std::vector<InstructionParameter> getParameters() override;

  bool isParameterized() override;

  const int nParameters() override;

  std::shared_ptr<Function>
  operator()(const std::vector<double> &params) override;

  std::shared_ptr<Graph> toGraph() override;

  /**
   * Return the number of logical qubits.
   *
   * @return nLogical The number of logical qubits.
   */
  const int nLogicalBits() override;

  /**
   * Return the number of physical qubits.
   *
   * @return nPhysical The number of physical qubits.
   */
  const int nPhysicalBits() override;

  /**
   * Return true if this Instruction has
   * customizable options.
   *
   * @return hasOptions
   */
  bool hasOptions() override { return !options.empty(); }

  /**
   * Set the value of an option with the given name.
   *
   * @param optName The name of the option.
   * @param option The value of the option
   */
  void setOption(const std::string optName,
                 InstructionParameter option) override {
    options.insert({optName, option});
  }

  /**
   * Get the value of an option with the given name.
   *
   * @param optName Then name of the option.
   * @return option The value of the option.
   */
  InstructionParameter getOption(const std::string optName) override {
    return options[optName];
  }

  /**
   * Return all the Instructions options as a map.
   *
   * @return optMap The options map.
   */
  std::map<std::string, InstructionParameter> getOptions() override {
    return options;
  }

  DEFINE_VISITABLE()

protected:
  /**
   * The name of this function
   */
  std::string functionName;

  std::list<InstPtr> instructions;

  std::vector<InstructionParameter> parameters;

  std::string tag = "";

  std::map<std::string, InstructionParameter> options;

  std::vector<int> bitMap;

};

} // namespace quantum
} // namespace xacc

#endif
