import os
import subprocess
import sys

from samples import cl_gemm
from samples import dpc_gemm
from samples import omp_gemm
from samples import ze_gemm
import utils

def config(path):
  cmake = ["cmake",\
    "-DCMAKE_BUILD_TYPE=" + utils.get_build_flag(), ".."]
  stdout, stderr = utils.run_process(cmake, path)
  if stderr and stderr.find("CMake Error") != -1:
    return stderr
  return None

def build(path):
  stdout, stderr = utils.run_process(["make"], path)
  if stderr and stderr.lower().find("error") != -1:
    return stderr
  return None

def run(path, option):
  if option == "cl":
    app_folder = utils.get_sample_executable_path("cl_gemm")
    app_file = os.path.join(app_folder, "cl_gemm")
    command = ["./oneprof", "-k", "-a", app_file, "gpu", "1024", "1"]
  elif option == "ze":
    app_folder = utils.get_sample_executable_path("ze_gemm")
    app_file = os.path.join(app_folder, "ze_gemm")
    command = ["./oneprof", "-k", "-a", app_file, "1024", "1"]
  elif option == "omp":
    app_folder = utils.get_sample_executable_path("omp_gemm")
    app_file = os.path.join(app_folder, "omp_gemm")
    command = ["./oneprof", "-k", "-a", app_file, "gpu", "1024", "1"]
  else:
    app_folder = utils.get_sample_executable_path("dpc_gemm")
    app_file = os.path.join(app_folder, "dpc_gemm")
    command = ["./oneprof", option, app_file, "gpu", "1024", "1"]
  stdout, stderr = utils.run_process(command, path)
  if option == "-e" and stderr.find(
      "Metric group EuStallSampling is not available") != -1:
    utils.skip("no EuStallSampling metric group on the device " +
               "(not supported, or metric streams not allowed)")
  if not stdout:
    return "stdout is empty"
  if not stderr:
    return "stderr is empty"
  if stdout.find(" CORRECT") == -1:
    return stdout
  if stderr.find("WARNING") != -1:
    return stderr
  if option == "-e":
    return check_stall_sampling(stderr)
  return None

# -e: the GEMM kernel has a row in the stall table with samples, and a hot
# instruction list (the disassembly in it needs IGA, so it isn't checked).
# oneprof prints its report to stderr unless -o is given.
def check_stall_sampling(report):
  if report.find("== EU Stall Sampling ==") == -1:
    return report
  for line in report.splitlines():
    fields = line.split(",")
    if len(fields) > 2 and fields[0].find("GEMM") != -1 and \
        fields[1].isdigit() and int(fields[1]) > 0:
      break
  else:
    return "no EU stall samples for the GEMM kernel:\n" + report
  if report.find("== Hot Instructions: ") == -1:
    return report
  return None

def main(option):
  utils.require_mdapi()
  if option != "-i": # -i collects kernel intervals only, no metric stream
    utils.require_perf_access()
  path = utils.get_tool_build_path("oneprof")
  if option == "cl":
    log = cl_gemm.main("gpu")
  elif option == "ze":
    log = ze_gemm.main(None)
  elif option == "omp":
    log = omp_gemm.main("gpu")
  else:
    log = dpc_gemm.main("gpu")
  if log:
    return log
  log = config(path)
  if log:
    return log
  log = build(path)
  if log:
    return log
  log = run(path, option)
  if log:
    return log

if __name__ == "__main__":
  option = "-m"
  if len(sys.argv) > 1 and sys.argv[1] == "-i":
    option = "-i"
  if len(sys.argv) > 1 and sys.argv[1] == "-k":
    option = "-k"
  if len(sys.argv) > 1 and sys.argv[1] == "-a":
    option = "-a"
  if len(sys.argv) > 1 and sys.argv[1] == "-q":
    option = "-q"
  if len(sys.argv) > 1 and sys.argv[1] == "-e":
    option = "-e"
  if len(sys.argv) > 1 and sys.argv[1] == "cl":
    option = "cl"
  if len(sys.argv) > 1 and sys.argv[1] == "ze":
    option = "ze"
  if len(sys.argv) > 1 and sys.argv[1] == "omp":
    option = "omp"
  log = main(option)
  if log:
    print(log)