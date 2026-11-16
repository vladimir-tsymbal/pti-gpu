import os
import signal
import sys
import subprocess
import time

# Per-test deadline (time.monotonic() value) set by run.py; None means no limit.
_deadline = None

class TestTimeout(Exception):
  pass

class TestSkipped(Exception):
  """Raised by a test (or a sample it depends on) that cannot run here."""
  pass

def skip(reason):
  raise TestSkipped(reason)

# Files that control access to GPU performance (OA) metric streams.
PERF_PARANOID_FILES = ["/proc/sys/dev/i915/perf_stream_paranoid",
                       "/proc/sys/dev/xe/observation_paranoid"]

# Set by run.py --skip-oa, or by the user, to skip every test that uses GPU
# performance (OA) metrics, e.g. on hosts where OA streams are unstable.
SKIP_OA_ENV = "PTI_TEST_SKIP_OA"

def check_perf_access():
  """Returns None if metric streams may be opened, otherwise the reason."""
  if os.environ.get(SKIP_OA_ENV, "0") not in ("", "0"):
    return "OA metrics tests disabled (--skip-oa / " + SKIP_OA_ENV + "=1)"
  if sys.platform == 'win32':
    return None
  if os.geteuid() == 0:
    return None
  for path in PERF_PARANOID_FILES:
    if os.path.exists(path):
      with open(path) as f:
        value = f.read().strip()
      if value != "0":
        return (os.path.basename(path) + "=" + value + ", metrics need 0 " +
                "or root (echo 0 | sudo tee " + path + ")")
  return None

# GPUs the GTPin tools (instcount, memaccess) are validated on. On other GPUs
# the pinned GTPin may find no kernels or break the application (seen on an
# Arc Pro B70 with SYCL, and on an Arrow Lake iGPU): those tests SKIP there.
GTPIN_VALIDATED_DEVICES = ["Data Center GPU Max 1550", "Arc(TM) B570"]

def skip_if_gtpin_unsupported(stdout, stderr):
  """Skips a GTPin tool test whose run failed on a GPU GTPin isn't validated
  on: GTPin found no kernels, or the application failed under GTPin."""
  device = None
  for line in (stdout or "").splitlines():
    if line.startswith("Target device: "):
      device = line[len("Target device: "):].strip()
      break
  if device is None or any(name in device for name in GTPIN_VALIDATED_DEVICES):
    return
  if (stderr or "").find("GTPin didn't find any kernels") != -1:
    skip("GTPin found no kernels on " + device +
         " (not validated with the pinned GTPin)")
  if (stdout or "").find(" CORRECT") == -1:
    skip("the application fails under GTPin on " + device +
         " (not validated with the pinned GTPin)")

def require_perf_access():
  reason = check_perf_access()
  if reason:
    skip(reason)

def require_mdapi():
  if sys.platform == 'win32':
    return
  import ctypes
  for name in ["libigdmd.so", "libigdmd.so.1"]:
    try:
      ctypes.CDLL(name)
      return
    except OSError:
      pass
  skip("Intel(R) Metrics Discovery API library (libigdmd.so) is not found")

def require_icpx():
  if sys.platform == 'win32':
    return
  import shutil
  if shutil.which("icpx") is None:
    skip("icpx (Intel(R) oneAPI DPC++/C++ Compiler) is not found, " +
         "source <oneapi>/setvars.sh")

def check_device_timing(command, stderr, kernel=None):
  """Returns an error if a device timing (-d) run doesn't report the kernel.

  Kernels launched through APIs the tracer doesn't hook (e.g. the
  zeCommandListAppendLaunchKernelWithArguments launches of the SYCL Level
  Zero v2 adapter) are otherwise silently missing from the report.
  --conditional-collection may legitimately collect nothing."""
  if "-d" not in command or "--conditional-collection" in command:
    return None
  if kernel is None:
    # The GEMM kernel of cl_gemm, ze_gemm and dpc_gemm; omp_gemm offloads
    # ComputeOnGPU().
    is_omp = any(str(arg).endswith("omp_gemm") for arg in command)
    kernel = "ComputeOnGPU" if is_omp else "GEMM"
  if not stderr or stderr.find(kernel) == -1:
    return ("Kernel " + kernel + " is missing from the device timing " +
            "report:\n" + (stderr or ""))
  return None

def check_api_tracing(command, stderr):
  """Returns an error if an API tracing (-c, -h; always for ze_hot_functions)
  run doesn't list the API that launched the GEMM kernel.

  Launch APIs the tracer doesn't register (e.g. the
  zeCommandListAppendLaunchKernelWithArguments launches of the SYCL Level
  Zero v2 adapter) are otherwise silently missing from the output."""
  tool = os.path.basename(str(command[0]))
  if tool != "ze_hot_functions" and "-c" not in command and \
     "-h" not in command:
    return None
  is_cl = any(str(arg).endswith("cl_gemm") for arg in command)
  api = "clEnqueueNDRangeKernel" if is_cl else "zeCommandListAppendLaunch"
  if not stderr or stderr.find(api) == -1:
    return ("Kernel launch API " + api + "* is missing from the API " +
            "tracing output:\n" + (stderr or ""))
  return None

_sycl_l0_v2 = None

def sycl_uses_level_zero_v2():
  """True if the SYCL runtime drives the GPU with the Level Zero v2 adapter
  (the default on Xe2 GPUs, e.g. BMG, with oneAPI 2026), which launches
  kernels with zeCommandListAppendLaunchKernelWithArguments."""
  global _sycl_l0_v2
  if _sycl_l0_v2 is None:
    _sycl_l0_v2 = False
    if os.environ.get("UR_LOADER_USE_LEVEL_ZERO_V2", "") != "0":
      try:
        p = subprocess.run(["sycl-ls"], stdout=subprocess.PIPE,
                           stderr=subprocess.STDOUT, timeout=60,
                           universal_newlines=True)
        _sycl_l0_v2 = p.stdout.find("Level-Zero V2") != -1
      except (OSError, subprocess.SubprocessError):
        pass
  return _sycl_l0_v2

def check_gtpin_config(stderr):
  """Skips if the CMake output shows that GTPin is unavailable."""
  if stderr and (stderr.find("GTPin package download failed") != -1 or
                 stderr.find("Some parts of GTPin package not found") != -1):
    skip("GTPin is not available (download failed or GTPIN_PATH is " +
         "incomplete), see tools/instcount/README.md")

def set_deadline(seconds):
  global _deadline
  _deadline = (time.monotonic() + seconds) if seconds and seconds > 0 else None

def get_script_path():
  path, script = os.path.split(os.path.realpath(__file__))
  return path

def get_root_path():
  head, tail = os.path.split(get_script_path())
  return head

def get_sample_build_path(name):
  head, tail = os.path.split(get_script_path())
  path = os.path.join(head, "samples")
  path = os.path.join(path, name)
  assert os.path.exists(path)
  path = os.path.join(path, "build")
  if not os.path.exists(path):
    os.mkdir(path)
  return path

def get_sample_executable_path(name, build_type = ""):
  path = get_sample_build_path(name)
  assert os.path.exists(path)
  return path

def get_tool_build_path(name):
  head, tail = os.path.split(get_script_path())
  path = os.path.join(head, "tools")
  path = os.path.join(path, name)
  assert os.path.exists(path)
  path = os.path.join(path, "build")
  if not os.path.exists(path):
    os.mkdir(path)
  return path

def get_build_utils_path():
  head, tail = os.path.split(get_script_path())
  path = os.path.join(head, "build_utils")
  return path

def get_build_flag():
  build_flag = "Release"
  for i in range(1, len(sys.argv)):
    if sys.argv[i] == "-d":
      build_flag = "Debug"
  return build_flag

def add_env(env, name, val):
  if env:
    custom_env = env
  else:
    custom_env = os.environ.copy()
  custom_env[name] = val
  return custom_env

def _kill(p):
  if sys.platform != 'win32':
    try:
      os.killpg(p.pid, signal.SIGKILL)
      return
    except OSError:
      pass
  p.kill()

def run_process(command, path, environ = None):
  shell = True if sys.platform == 'win32' else False
  timeout = None
  if _deadline is not None:
    timeout = _deadline - time.monotonic()
    if timeout <= 0:
      raise TestTimeout(" ".join(command))
  # A new session lets a timeout kill the whole process tree (e.g. make, or a
  # tool that launches the application), not only the direct child.
  new_session = (sys.platform != 'win32')
  p = subprocess.Popen(command, cwd = path, shell = shell,\
    env = environ, stdout = subprocess.PIPE, stderr = subprocess.PIPE, encoding = "latin-1", text = True,\
    start_new_session = new_session)
  try:
    stdout, stderr = p.communicate(timeout = timeout)
  except subprocess.TimeoutExpired:
    _kill(p)
    p.communicate()
    raise TestTimeout(" ".join(command))

  # The OpenCL tracing tools need the Intel tracing extension of the OpenCL
  # runtime (utils/cl_api_tracer.h reports it once if it is missing).
  if stderr and stderr.find("Intel tracing extension " +
                            "(clCreateTracingHandleINTEL)") != -1:
    skip("the OpenCL runtime does not provide clCreateTracingHandleINTEL " +
         "(Intel OpenCL tracing extension)")

  return stdout, stderr
