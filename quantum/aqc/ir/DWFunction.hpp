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
#ifndef QUANTUM_AQC_DWFUNCTION_HPP_
#define QUANTUM_AQC_DWFUNCTION_HPP_

#include "Function.hpp"
#include "DWQMI.hpp"
#include "XACC.hpp"
#include "exprtk.hpp"

static constexpr double pi = 3.141592653589793238;

using symbol_table_t = exprtk::symbol_table<double>;
using expression_t = exprtk::expression<double>;
using parser_t = exprtk::parser<double>;

namespace xacc {
namespace quantum {

class DWFunction : public Function,
                   public std::enable_shared_from_this<DWFunction> {

protected:
  std::list<InstPtr> instructions;

  std::vector<InstructionParameter> parameters;

  std::string _name;

public:
  /**
   * The constructor, takes the function unique id and its name.
   *
   *
   * @param id
   * @param name
   */
  DWFunction(std::string kernelName)
      : _name(kernelName), parameters(std::vector<InstructionParameter>{}) {}

  DWFunction(std::string kernelName, std::vector<InstructionParameter> p)
      : _name(kernelName), parameters(p) {}

  std::shared_ptr<Function> enabledView() override {
    auto newF = std::make_shared<DWFunction>(_name, parameters);
    for (int i = 0; i < nInstructions(); i++) {
      auto inst = getInstruction(i);
      if (inst->isEnabled()) {
        newF->addInstruction(inst);
      }
    }
    return newF;
  }

  const int nInstructions() override { return instructions.size(); }

  InstPtr getInstruction(const int idx) override {
    InstPtr i;
    if (instructions.size() > idx) {
      i = *std::next(instructions.begin(), idx);
    } else {
      xacc::error("DWFunction getInstruction invalid instruction index - " +
                  std::to_string(idx) + ".");
    }
    return i;
  }

  void setBitMap(const std::vector<int> bMap) override {}
  const std::vector<int> getBitMap() override {return std::vector<int>{};}

  std::list<InstPtr> getInstructions() override { return instructions; }

  void removeInstruction(const int idx) override {
    instructions.remove(getInstruction(idx));
  }

  void mapBits(std::vector<int> bitMap) override {
    xacc::error("DWFunction.mapBits not implemented");
  }

  const int nRequiredBits() const override {
    XACCLogger::instance()->error(
        "DWFunction nRequiredBits() not implemented.");
    return 0;
  }
  /**
   * Add an instruction to this quantum
   * intermediate representation.
   *
   * @param instruction
   */
  void addInstruction(InstPtr instruction) override {
    xacc::InstructionParameter param = instruction->getParameter(0);
    bool dupParam = false;
    for (auto p : parameters) {
      if (p.toString() == param.toString()) {
        dupParam = true;
      }
    }
    if (!dupParam) {
      parameters.push_back(param);
    }
    instructions.push_back(instruction);
  }

  const int depth() override {
    xacc::error("DWFunction graph is undirected, cannot compute depth.");
    return 0;
  }

  const std::string persistGraph() override {
    std::stringstream s;
    toGraph()->write(s);
    return s.str();
  }

  std::shared_ptr<Graph> toGraph() override;

  void replaceInstruction(const int idx, InstPtr replacingInst) override {
    std::replace(instructions.begin(), instructions.end(), getInstruction(idx),
                 replacingInst);
  }

  void insertInstruction(const int idx, InstPtr newInst) override {
    auto iter = std::next(instructions.begin(), idx);
    instructions.insert(iter, newInst);
  }

  /**
   * Return the name of this function
   * @return
   */
  const std::string name() const override { return _name; }
  const std::string description() const override { return ""; }

  const std::vector<int> bits() override { return std::vector<int>{}; }

  const std::string toString(const std::string &bufferVarName) override {
    std::stringstream ss;
    for (auto i : instructions) {
      ss << i->toString("") << ";\n";
    }
    return ss.str();
  }

  const std::string toString() override { return toString(""); }

  /**
   * Return the number of logical qubits.
   *
   * @return nLogical The number of logical qubits.
   */
  const int nLogicalBits() override { return 0; }

  /**
   * Return the number of physical qubits.
   *
   * @return nPhysical The number of physical qubits.
   */
  const int nPhysicalBits() override { return 0; }

  std::vector<double> getAllBiases() {
    std::vector<double> biases;
    for (auto i : instructions) {
      if (i->bits()[0] == i->bits()[1]) {
        biases.push_back(mpark::get<double>(i->getParameter(0)));
      }
    }

    return biases;
  }

  std::vector<double> getAllCouplers() {
    std::vector<double> weights;
    for (auto i : instructions) {
      if (i->bits()[0] != i->bits()[1]) {
        weights.push_back(mpark::get<double>(i->getParameter(0)));
      }
    }

    return weights;
  }

  void persist(std::ostream &outStream) override;
  void load(std::istream &inStream) override;

  InstructionParameter getParameter(const int idx) const override {
    return parameters[idx];
  }

  void setParameter(const int idx, InstructionParameter &p) override {
    if (idx + 1 > parameters.size()) {
      XACCLogger::instance()->error(
          "DWFunction.setParameter: Invalid Parameter requested.");
    }

    parameters[idx] = p;
  }

  std::vector<InstructionParameter> getParameters() override {
    return parameters;
  }

  void addParameter(InstructionParameter instParam) override {
    parameters.push_back(instParam);
  }

  bool isParameterized() override { return nParameters() > 0; }

  const int nParameters() override { return parameters.size(); }

  std::shared_ptr<Function>
  operator()(const std::vector<double> &params) override {
    if (params.size() != nParameters()) {
      xacc::error("Invalid DWFunction evaluation: number "
                  "of parameters don't match. " +
                  std::to_string(params.size()) + ", " +
                  std::to_string(nParameters()));
    }

    std::vector<double> p = params;
    symbol_table_t symbol_table;
    symbol_table.add_constants();
    std::vector<std::string> variableNames;
    std::vector<double> values;
    for (int i = 0; i < params.size(); i++) {
      auto var = mpark::get<std::string>(getParameter(i));
      variableNames.push_back(var);
      symbol_table.add_variable(var, p[i]);
    }

    auto compileExpression = [&](InstructionParameter &p) -> double {
      auto expression = mpark::get<std::string>(p);
      expression_t expr;
      expr.register_symbol_table(symbol_table);
      parser_t parser;
      parser.compile(expression, expr);
      return expr.value();
    };

    auto evaluatedFunction = std::make_shared<DWFunction>("evaled_" + name());
    for (auto inst : getInstructions()) {
      if (inst->isComposite()) {
        // If a Function, call this method recursively
        auto evaled =
            std::dynamic_pointer_cast<Function>(inst)->operator()(params);
        evaluatedFunction->addInstruction(evaled);
      } else {
        if (inst->name() == "dw-qmi") {
          if (inst->getParameter(0).isVariable()) {
            InstructionParameter p = inst->getParameter(0);
            auto val = compileExpression(p);
            InstructionParameter pnew(val);
            auto updatedInst =
                std::make_shared<DWQMI>(inst->bits()[0], inst->bits()[1], pnew);
            evaluatedFunction->addInstruction(updatedInst);
          } else {
            evaluatedFunction->addInstruction(inst);
          }
        } else if (inst->name() == "anneal") {
          std::vector<InstructionParameter> newParams;
          for (int i = 0; i < inst->nParameters(); i++) {
            InstructionParameter p = inst->getParameter(i);
            if (inst->getParameter(i).isVariable()) {
              if (mpark::get<std::string>(p) == "forward" |
                  mpark::get<std::string>(p) == "reverse") {
                newParams.push_back(p);
              } else {
                auto val = compileExpression(p);
                newParams.push_back(InstructionParameter(val));
              }
            } else {
              newParams.push_back(p);
            }
          }
          auto newAnneal = std::make_shared<Anneal>(newParams);
          evaluatedFunction->addInstruction(newAnneal);
        } else {
          xacc::error("Invalid DW QMI during kernel evaluation - " +
                      inst->name() + ".");
        }
      }
    }

    return evaluatedFunction;
  }

  /**
   * Return true if this Instruction has
   * customizable options.
   *
   * @return hasOptions
   */
  bool hasOptions() override { return false; }

  /**
   * Set the value of an option with the given name.
   *
   * @param optName The name of the option.
   * @param option The value of the option
   */
  void setOption(const std::string optName,
                 InstructionParameter option) override {
    XACCLogger::instance()->error("setOption not implemented for DWFunction.");
    return;
  }

  /**
   * Get the value of an option with the given name.
   *
   * @param optName Then name of the option.
   * @return option The value of the option.
   */
  InstructionParameter getOption(const std::string optName) override {
    XACCLogger::instance()->error("getOption not implemented for DWFunction.");
    return InstructionParameter(0);
  }

  /**
   * Return all the Instructions options as a map.
   *
   * @return optMap The options map.
   */
  std::map<std::string, InstructionParameter> getOptions() override {
    XACCLogger::instance()->error("getOptions not implemented for DWFunction.");
    return std::map<std::string, InstructionParameter>();
  }
  EMPTY_DEFINE_VISITABLE()
};

} // namespace quantum
} // namespace xacc

#endif
