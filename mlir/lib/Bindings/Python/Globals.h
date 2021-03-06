//===- Globals.h - MLIR Python extension globals --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_BINDINGS_PYTHON_GLOBALS_H
#define MLIR_BINDINGS_PYTHON_GLOBALS_H

#include <string>
#include <vector>

#include "PybindUtils.h"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSet.h"

namespace mlir {
namespace python {

/// Globals that are always accessible once the extension has been initialized.
class PyGlobals {
public:
  PyGlobals();
  ~PyGlobals();

  /// Most code should get the globals via this static accessor.
  static PyGlobals &get() {
    assert(instance && "PyGlobals is null");
    return *instance;
  }

  /// Get and set the list of parent modules to search for dialect
  /// implementation classes.
  std::vector<std::string> &getDialectSearchPrefixes() {
    return dialectSearchPrefixes;
  }
  void setDialectSearchPrefixes(std::vector<std::string> newValues) {
    dialectSearchPrefixes.swap(newValues);
  }

  /// Loads a python module corresponding to the given dialect namespace.
  /// No-ops if the module has already been loaded or is not found. Raises
  /// an error on any evaluation issues.
  /// Note that this returns void because it is expected that the module
  /// contains calls to decorators and helpers that register the salient
  /// entities.
  void loadDialectModule(const std::string &dialectNamespace);

  /// Decorator for registering a custom Dialect class. The class object must
  /// have a DIALECT_NAMESPACE attribute.
  pybind11::object registerDialectDecorator(pybind11::object pyClass);

  /// Adds a concrete implementation dialect class.
  /// Raises an exception if the mapping already exists.
  /// This is intended to be called by implementation code.
  void registerDialectImpl(const std::string &dialectNamespace,
                           pybind11::object pyClass);

  /// Adds a concrete implementation operation class.
  /// Raises an exception if the mapping already exists.
  /// This is intended to be called by implementation code.
  void registerOperationImpl(const std::string &operationName,
                             pybind11::object pyClass,
                             pybind11::object rawClass);

  /// Looks up a registered dialect class by namespace. Note that this may
  /// trigger loading of the defining module and can arbitrarily re-enter.
  llvm::Optional<pybind11::object>
  lookupDialectClass(const std::string &dialectNamespace);

private:
  static PyGlobals *instance;
  /// Module name prefixes to search under for dialect implementation modules.
  std::vector<std::string> dialectSearchPrefixes;
  /// Map of dialect namespace to bool flag indicating whether the module has
  /// been successfully loaded or resolved to not found.
  llvm::StringSet<> loadedDialectModules;
  /// Map of dialect namespace to external dialect class object.
  llvm::StringMap<pybind11::object> dialectClassMap;
  /// Map of full operation name to external operation class object.
  llvm::StringMap<pybind11::object> operationClassMap;
  /// Map of operation name to custom subclass that directly initializes
  /// the OpView base class (bypassing the user class constructor).
  llvm::StringMap<pybind11::object> rawOperationClassMap;
};

} // namespace python
} // namespace mlir

#endif // MLIR_BINDINGS_PYTHON_GLOBALS_H
