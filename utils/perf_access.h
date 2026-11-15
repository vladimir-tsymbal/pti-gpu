//==============================================================
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#ifndef PTI_UTILS_PERF_ACCESS_H_
#define PTI_UTILS_PERF_ACCESS_H_

// Access check for GPU performance (OA) metric streams on Linux. The kernel
// driver decides who may open them: i915 uses
// /proc/sys/dev/i915/perf_stream_paranoid, xe uses
// /proc/sys/dev/xe/observation_paranoid. A value of 0 (or root) allows
// unprivileged users. Both drivers may be loaded at once (one GPU each), so
// every file that exists is checked. On other platforms access is not
// restricted this way and the checks always succeed.

#if !defined(_WIN32)
#include <unistd.h>
#endif

#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace utils {

inline constexpr const char* const kPerfParanoidFiles[] = {
  "/proc/sys/dev/xe/observation_paranoid",
  "/proc/sys/dev/i915/perf_stream_paranoid",
};

// Paranoid files of the GPU kernel drivers that are loaded now.
inline std::vector<std::string> GetPerfParanoidPaths() {
  std::vector<std::string> paths;
#if defined(__gnu_linux__)
  for (const char* path : kPerfParanoidFiles) {
    std::ifstream file(path);
    if (file.good()) {
      paths.push_back(path);
    }
  }
#endif
  return paths;
}

// True if the current process may open metric streams. If no paranoid file
// exists, the result is false: access can't be determined.
inline bool IsPerfAccessAllowed() {
#if defined(__gnu_linux__)
  if (geteuid() == 0) {
    return true;
  }
  std::vector<std::string> paths = GetPerfParanoidPaths();
  if (paths.empty()) {
    return false;
  }
  for (const std::string& path : paths) {
    std::ifstream file(path);
    std::string value;
    if (!(file >> value) || value != "0") {
      return false;
    }
  }
#endif
  return true;
}

// The paranoid file(s) to name in messages: the ones that exist, or all known
// ones if none does (no driver loaded, or the kernel is too old for xe OA).
inline std::string GetPerfParanoidHint() {
  std::vector<std::string> paths = GetPerfParanoidPaths();
  if (paths.empty()) {
    paths.assign(std::begin(kPerfParanoidFiles), std::end(kPerfParanoidFiles));
  }
  std::string hint;
  for (size_t i = 0; i < paths.size(); ++i) {
    if (i > 0) {
      hint += " or ";
    }
    hint += paths[i];
  }
  return hint;
}

} // namespace utils

#endif // PTI_UTILS_PERF_ACCESS_H_
