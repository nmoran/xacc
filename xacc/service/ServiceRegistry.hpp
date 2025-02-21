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
#ifndef XACC_UTILS_SERVICEREGISTRY_HPP_
#define XACC_UTILS_SERVICEREGISTRY_HPP_

#include "Singleton.hpp"
#include "Utils.hpp"
#include "RuntimeOptions.hpp"
#include "Identifiable.hpp"
#include "OptionsProvider.hpp"
#include "Cloneable.hpp"

#include "xacc_config.hpp"

#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleImport.h>

#include <map>
#include <dirent.h>

using namespace cppmicroservices;

namespace xacc {

/**
 * The ServiceRegistry provides the plugin infrastructure for XACC.
 * It delegates to the CppMicroServices Framework and BundleContexts,
 * and provides methods for creating instances of XACC plugins or services.
 */
class ServiceRegistry {

protected:
  /**
   * Reference to the CppMicroServices Framework instance
   */
  Framework framework;

  /**
   * The BundleContext instance, which provides
   * service references.
   */
  BundleContext context;

  bool initialized = false;

  std::string rootPathStr = "";

public:
  ServiceRegistry() : framework(FrameworkFactory().NewFramework()) {}
  const std::string getRootPathString() { return rootPathStr; }

  void initialize(const std::string rootPath);
  
  template <typename ServiceInterface> bool hasService(const std::string name) {
    auto allServiceRefs = context.GetServiceReferences<ServiceInterface>();
    for (auto s : allServiceRefs) {
      auto service = context.GetService(s);
      auto identifiable =
          std::dynamic_pointer_cast<xacc::Identifiable>(service);
      if (identifiable && identifiable->name() == name) {
        return true;
      }
    }
    return false;
  }

  template <typename ServiceInterface>
  std::shared_ptr<ServiceInterface> getService(const std::string name) {
    std::shared_ptr<ServiceInterface> ret;
    auto allServiceRefs = context.GetServiceReferences<ServiceInterface>();
    for (auto s : allServiceRefs) {
      auto service = context.GetService(s);
      auto identifiable =
          std::dynamic_pointer_cast<xacc::Identifiable>(service);
      if (identifiable && identifiable->name() == name) {
        auto checkCloneable =
            std::dynamic_pointer_cast<xacc::Cloneable<ServiceInterface>>(
                service);
        if (checkCloneable) {
          ret = checkCloneable->clone();
        } else {
          ret = service;
        }
      }
    }

    if (!ret) {
      XACCLogger::instance()->error("Could not find service with name " + name +
                                    ". "
                                    "Perhaps the service is not Identifiable.");
    }

    return ret;
  }

  template <typename ServiceInterface>
  std::vector<std::shared_ptr<ServiceInterface>> getServices() {
    std::vector<std::shared_ptr<ServiceInterface>> services;
    auto allServiceRefs = context.GetServiceReferences<ServiceInterface>();
    for (auto s : allServiceRefs) {
      services.push_back(context.GetService(s));
    }

    return services;
  }

  template <typename ServiceInterface>
  std::vector<std::string> getRegisteredIds() {
    std::vector<std::string> ids;
    auto allServiceRefs = context.GetServiceReferences<ServiceInterface>();
    for (auto s : allServiceRefs) {
      auto service = context.GetService(s);
      auto identifiable =
          std::dynamic_pointer_cast<xacc::Identifiable>(service);
      if (identifiable) {
        ids.push_back(identifiable->name());
      }
    }
    return ids;
  }

  std::vector<OptionPairs> getRegisteredOptions() {
    std::vector<OptionPairs> values;
    // Get all OptionsProvider services and call getOptions
    auto optionProviders = getServices<xacc::OptionsProvider>();
    for (auto o : optionProviders) {
      values.push_back(o->getOptions());
    }
    return values;
  }

  bool handleOptions(const std::map<std::string, std::string> &map) {
    auto returnedTrue = false;
    // Get all OptionProvider services and call handleOptions
    auto optionProviders = getServices<xacc::OptionsProvider>();
    for (auto o : optionProviders) {
      if (o->handleOptions(map)) {
        returnedTrue = true;
      }
    }
    return returnedTrue;
  }
};
} // namespace xacc

#endif
