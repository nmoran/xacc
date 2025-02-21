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
#include "Utils.hpp"
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "RuntimeOptions.hpp"
#include <iostream>
#include <sstream>
#include <istream>
#include <dirent.h>

namespace xacc {

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

bool fileExists(const std::string &name) {
  if (FILE *file = fopen(name.c_str(), "r")) {
    fclose(file);
    return true;
  } else {
    return false;
  }
}

bool directoryExists(const std::string path) {
 auto p = path.c_str();
 if ( p == NULL) return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir (p);

    if (pDir != NULL)
    {
        bExists = true;
        (void) closedir (pDir);
    }

    return bExists;
}

std::string base64_decode(std::string const &encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && (encoded_string[in_] != '=') &&
         is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_];
    in_++;
    if (i == 4) {
      for (i = 0; i < 4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] =
          (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] =
          ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 4; j++)
      char_array_4[j] = 0;

    for (j = 0; j < 4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] =
        ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++)
      ret += char_array_3[j];
  }

  return ret;
}

void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// trim from both ends (in place)
void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}
XACCLogger::XACCLogger()
    : useColor(!RuntimeOptions::instance()->exists("no-color")),
      useCout(RuntimeOptions::instance()->exists("use-cout")) {
  std::string loggerName = "xacc-logger";
  if (RuntimeOptions::instance()->exists("logger-name")) {
    loggerName = (*RuntimeOptions::instance())["logger-name"];
  }
  auto _log = spdlog::get(loggerName);
  if (_log) {
    logger = _log;
  } else {
    logger = spdlog::stdout_logger_mt(loggerName);
  }

  logger->set_level(spdlog::level::info);
}

void XACCLogger::info(const std::string &msg, MessagePredicate predicate) {
  if (useCout) {
    if (predicate() && globalPredicate()) {
      if (useColor) {
        std::cout << "\033[1;34m[XACC Info] " + msg + "\033[0m \n";
      } else {
        std::cout << "[XACC Info] " + msg + "\n";
      }
    }
  } else {
    if (predicate() && globalPredicate()) {
      if (useColor) {
        logger->info("\033[1;34m" + msg + "\033[0m");
      } else {
        logger->info(msg);
      }
    }
  }
}

void XACCLogger::warning(const std::string &msg, MessagePredicate predicate) {
  if (useCout) {
    if (predicate() && globalPredicate()) {
      if (useColor) {
        std::cout << "\033[1;33m[XACC Warning] " + msg + "\033[0m \n";
      } else {
        std::cout << "[XACC Warning] " + msg + "\n";
      }
    }
  } else {
    if (predicate() && globalPredicate()) {
      if (useColor) {
        logger->info("\033[1;33m" + msg + "\033[0m");
      } else {
        logger->info(msg);
      }
    }
  }
}

void XACCLogger::debug(const std::string &msg, MessagePredicate predicate) {
  if (useCout) {
    if (predicate() && globalPredicate()) {
      if (useColor) {
        std::cout << "\033[1;32m[XACC Debug] " + msg + "\033[0m \n";
      } else {
        std::cout << "[XACC Debug] " + msg + "\n";
      }
    }
  } else {
    if (predicate() && globalPredicate()) {
      if (useColor) {
        logger->info("\033[1;32m" + msg + "\033[0m");
      } else {
        logger->info(msg);
      }
    }
  }
}
void XACCLogger::error(const std::string &msg, MessagePredicate predicate) {
  if (useCout) {
    if (predicate() && globalPredicate())
      std::cerr << msg << "\n";
  } else {
    if (predicate() && globalPredicate()) {
      logger->error("\033[1;31m[XACC Error] " + msg + "\033[0m");
    }
  }
}

} // namespace xacc
