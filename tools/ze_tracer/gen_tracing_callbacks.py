#==============================================================
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# =============================================================

import os
import sys
import re

# Parse Level Zero Headers ####################################################

STATE_NORMAL = 0
STATE_CONDITION = 1
STATE_SKIP = 2

def get_comma_count(line):
  count = 0
  level = 0
  for symbol in line:
    if symbol == "(":
      level += 1
    elif symbol == ")":
      assert level > 0
      level -= 1
    elif symbol == "," and level == 0:
      count += 1
  return count

def remove_comments(line):
  pos = line.find("//")
  if pos != -1:
    line = line[0:pos]
  return line

def get_func_name(callback_struct_name):
  assert callback_struct_name.strip().find("ze_pfn") == 0
  assert callback_struct_name.strip().find("Cb_t") + len("Cb_t") == len(callback_struct_name.strip())
  body = callback_struct_name.strip()
  body = body.split("ze_pfn")[1]
  body = body.split("Cb_t")[0]
  return "ze" + body

def get_struct_range(lines, struct_name):
  start = -1
  for i in range(len(lines)):
    if lines[i].find(struct_name) != -1:
      start = i
      break
  assert start >= 0

  while lines[start].find("{") == -1:
    start +=1
  start += 1
  assert start < len(lines)

  end = start
  while lines[end].find("}") == -1:
    end += 1
  assert end < len(lines)

  return start, end

def get_callback_struct_map(f, struct_name):
  f.seek(0)
  lines = f.readlines()

  start, end = get_struct_range(lines, struct_name)

  struct_map = {}
  cond = ""
  state = STATE_NORMAL
  for i in range(start, end):
    if lines[i].find("#if") >= 0:
      items = lines[i].strip().split()
      assert len(items) == 2
      state = STATE_CONDITION
      cond = items[1].strip()
      continue
    elif lines[i].find("#else") >= 0:
      assert state == STATE_CONDITION
      state = STATE_SKIP
      continue
    elif lines[i].find("#endif") >= 0:
      assert state != STATE_NORMAL
      state = STATE_NORMAL
      cond = ""
      continue

    if state == STATE_SKIP:
      continue

    items = lines[i].strip().split()
    assert len(items) == 2
    type_name = items[0].strip()
    field_name = items[1].strip().strip(";")
    assert not (type_name in struct_map)
    struct_map[type_name] = (field_name, cond)

  return struct_map

def get_params(text, param_struct_name):
  # Callback parameter structs hold a pointer to each argument of the call.
  match = re.search(r"typedef\s+struct\s+_" + param_struct_name +
                    r"\s*\{([^}]*)\}\s*" + param_struct_name + r"\s*;", text)
  assert match, param_struct_name

  params = []
  for member in match.group(1).split(";"):
    items = member.strip().split()
    if not items:
      continue # e.g. zerGetDefaultContext has no arguments
    assert len(items) >= 2

    type = " ".join(items[0:len(items) - 1])
    assert type[len(type) - 1] == "*"
    type = type[0:len(type) - 1]

    name = items[len(items) - 1]
    assert name[0] == "p"
    name = name[1:len(name)]

    assert not ((name, type) in params)
    params.append((name, type))

  return params

def get_callback_param_struct_map(text):
  # Callback function-pointer type -> (type of its parameter struct, type of
  # the result), e.g. ze_pfnInitCb_t -> (ze_init_params_t, ze_result_t)
  # (ze_api.h for the 1.0 APIs, zel_tracing_register_cb.h for the newer
  # ones). A few APIs return something else, e.g. zeDriverGetDefaultContext.
  callback_map = {}
  for callback_type, params, result in re.findall(
      r"typedef\s+void\s*\(\s*ZE_APICALL\s*\*\s*(\w+)\s*\)\s*\(\s*"
      r"(\w+)\s*\*\s*params\s*,\s*([\w\s\*]+?)\s*\bresult\s*,", text):
    callback_map[callback_type] = (params, " ".join(result.split()))
  return callback_map

def get_register_func_list(text):
  # APIs with a per-API registration function in zel_tracing_register_cb.h:
  # zelTracer<Api>RegisterCallback(tracer, type, <prefix>_pfn<Api>Cb_t),
  # where <prefix> is ze or zer (Level Zero runtime API, zer_api.h).
  func_list = []
  for name, prefix, cb_name in re.findall(
      r"zelTracer(\w+)RegisterCallback\(\s*zel_tracer_handle_t\s+\w+\s*,"
      r"\s*zel_tracer_reg_t\s+\w+\s*,\s*(ze|zer)_pfn(\w+)Cb_t\b", text):
    assert name == cb_name
    func = prefix + name
    assert not (func in func_list)
    func_list.append(func)
  return func_list

def get_callback_type_name(func):
  # zeInit -> ze_pfnInitCb_t, zerGetDefaultContext -> zer_pfnGetDefaultContextCb_t
  prefix = "zer" if func.startswith("zer") else "ze"
  return prefix + "_pfn" + func[len(prefix):] + "Cb_t"

def get_struct_types(include_path):
  # Struct (and union) types: arguments passed by value with these types
  # can't be printed directly.
  struct_types = set()
  for file_name in os.listdir(include_path):
    if file_name.endswith(".h"):
      with open(os.path.join(include_path, file_name), "rt") as f:
        struct_types.update(re.findall(
            r"typedef\s+(?:struct|union)\s+_\w+\s+(\w+)\s*;", f.read()))
  return struct_types

def find_enums(f, enum_map):
  f.seek(0)
  lines = f.readlines()

  enum_list = []
  for line in lines:
    if line.find("typedef enum") != -1:
      items = line.strip().rstrip("{").split()
      assert len(items) == 3
      enum_list.append(items[2].strip().lstrip("_"))

  for enum_name in enum_list:
    params = {}
    start, end = get_struct_range(lines, enum_name)
    default_value = 0
    has_unresolved_values = False
    for i in range(start, end):
      line = remove_comments(lines[i]).strip()
      if not line or line.startswith('#'):
        continue
      comma_count = get_comma_count(line)
      assert comma_count == 0 or comma_count == 1
      if line.find("=") == -1:
        assert not has_unresolved_values
        field_name = line.rstrip(",")
        field_value = str(default_value)
        default_value += 1
      else:
        items = line.split("=")
        assert len(items) == 2
        field_name = items[0].strip()
        field_value = items[1].strip().rstrip(",")
        if field_value.find("0x") == 0:
          field_value = str(int(field_value, 16))
        if all(symbol.isdigit() for symbol in field_value):
          default_value = int(field_value)
          default_value += 1
        else:
          has_unresolved_values = True
      assert not (field_name in params)
      params[field_name] = field_value
    assert len(params) > 0
    assert not (enum_name in enum_map)
    enum_map[enum_name] = params

def get_func_list(f):
  f.seek(0)
  func_list = []
  for line in f.readlines():
    if line.find("ZE_APICALL") != -1 and line.find("ze_pfn") != -1:
      items = line.split("ze_pfn")
      assert len(items) == 2
      assert items[1].find("Cb_t") != -1
      items = items[1].split("Cb_t")
      assert len(items) == 2
      func_list.append("ze" + items[0].strip())
  return func_list

def get_callback_group_map(f):
  group_map = {}

  base_map = get_callback_struct_map(f, "ze_callbacks_t")
  assert len(base_map) > 0

  for key, value in base_map.items():
    func_map = get_callback_struct_map(f, key)
    for fkey, fvalue in func_map.items():
      func_name = get_func_name(fkey)
      assert not (func_name in group_map)
      group_map[func_name] = (value, fvalue)

  return group_map

def get_param_map(text, func_list):
  param_map = {}
  callback_map = get_callback_param_struct_map(text)

  for func in func_list:
    assert not (func in param_map)
    callback_type = get_callback_type_name(func)
    assert callback_type in callback_map, callback_type
    param_struct_name, result_type = callback_map[callback_type]
    param_map[func] = (param_struct_name, result_type,
                       get_params(text, param_struct_name))

  return param_map

def get_enum_map(include_path):
  enum_map = {}

  for file_name in os.listdir(include_path):
    if file_name.endswith(".h") or file_name.endswith(".hpp"):
      file_path = os.path.join(include_path, file_name)
      file = open(file_path, "rt")
      find_enums(file, enum_map)
      file.close()

  return enum_map

# Generate Callbacks ##########################################################

def gen_api(f, func_list, group_map, ext_func_list, api_func_name):
  f.write("static void " + api_func_name + "(zel_tracer_handle_t tracer) {\n")
  f.write("  zet_core_callbacks_t prologue = {};\n")
  f.write("  zet_core_callbacks_t epilogue = {};\n")
  f.write("\n")
  for func in func_list:
    if not func in group_map:
      continue

    group, callback = group_map[func]
    group_name = group[0]
    group_cond = group[1]
    assert not group_cond
    callback_name = callback[0]
    callback_cond = callback[1]
    if callback_cond:
      f.write("#if " + callback_cond + "\n")
    f.write("  prologue." + group_name + "." + callback_name + " = " + func + "OnEnter;\n")
    f.write("  epilogue." + group_name + "." + callback_name + " = " + func + "OnExit;\n")
    if callback_cond:
      f.write("#endif //" + callback_cond + "\n")
  f.write("\n")
  f.write("  ze_result_t status = ZE_RESULT_SUCCESS;\n")
  f.write("  status = zelTracerSetPrologues(tracer, &prologue);\n")
  f.write("  PTI_ASSERT(status == ZE_RESULT_SUCCESS);\n")
  f.write("  status = zelTracerSetEpilogues(tracer, &epilogue);\n")
  f.write("  PTI_ASSERT(status == ZE_RESULT_SUCCESS);\n")
  if ext_func_list:
    f.write("\n")
    f.write("  // APIs newer than zet_core_callbacks_t, registered one by one. Older\n")
    f.write("  // loaders lack some of them (and can't dispatch them): skipped.\n")
    for func in ext_func_list:
      name = func[len("zer"):] if func.startswith("zer") else func[len("ze"):]
      f.write("  utils::ze::SetTracerCallbacks<" + get_callback_type_name(func) + ">(\n")
      f.write("      tracer, \"zelTracer" + name + "RegisterCallback\",\n")
      f.write("      " + func + "OnEnter, " + func + "OnExit);\n")
  f.write("}\n")
  f.write("\n")

def gen_structure_type_converter(f, enum_map):
  struct_type_enum = {}
  for name in enum_map["ze_structure_type_t"]:
    struct_type_enum[name] = int(enum_map["ze_structure_type_t"][name])
  struct_type_enum = sorted(struct_type_enum.items(), key=lambda x:x[1])
  assert "ze_structure_type_t" in enum_map
  f.write("static const char* GetStructureTypeString(unsigned structure_type) {\n")
  f.write("  switch (structure_type) {\n")
  for name, value in struct_type_enum:
    f.write("    case " + name + ":\n")
    f.write("      return \"" + name + "\";\n")
  f.write("    default:\n")
  f.write("      break;\n")
  f.write("  }\n")
  f.write("  return \"UNKNOWN\";\n")
  f.write("}\n")
  f.write("\n")

def gen_result_converter(f, enum_map):
  result_enum = {}
  for name in enum_map["ze_result_t"]:
    result_enum[name] = int(enum_map["ze_result_t"][name])
  result_enum = sorted(result_enum.items(), key=lambda x:x[1])
  assert "ze_result_t" in enum_map
  f.write("static const char* GetResultString(unsigned result) {\n")
  f.write("  switch (result) {\n")
  for name, value in result_enum:
    f.write("    case " + name + ":\n")
    f.write("      return \"" + name + "\";\n")
  f.write("    default:\n")
  f.write("      break;\n")
  f.write("  }\n")
  f.write("  return \"UNKNOWN\";\n")
  f.write("}\n")
  f.write("\n")

def gen_enum(f, enum_map, enum_name, param_name):
  assert enum_name in enum_map
  f.write("    switch (" + param_name + ") {\n")
  for name, value in enum_map[enum_name].items():
    f.write("      case " + value + ":\n")
    f.write("        stream << \"" + name + "\";\n")
    f.write("        break;\n")
  f.write("      default:\n")
  f.write("        stream << \"<UNKNOWN>\";\n")
  f.write("        break;\n")
  f.write("    }\n")

def is_struct_type(type, struct_types):
  return type.replace("const ", "").strip() in struct_types

# Launch APIs that the kernel collectors trace: the exit callbacks print the
# kernel id that the kernel collector assigned to the call.
KERNEL_APPEND_FUNCS = [
  "zeCommandListAppendLaunchKernel",
  "zeCommandListAppendLaunchKernelWithArguments",
  "zeCommandListAppendLaunchCooperativeKernel",
  "zeCommandListAppendLaunchKernelIndirect",
  "zeCommandListAppendMemoryCopy",
  "zeCommandListAppendMemoryFill",
  "zeCommandListAppendBarrier",
  "zeCommandListAppendMemoryRangesBarrier",
  "zeCommandListAppendMemoryCopyRegion",
  "zeCommandListAppendMemoryCopyFromContext",
  "zeCommandListAppendImageCopy",
  "zeCommandListAppendImageCopyRegion",
  "zeCommandListAppendImageCopyToMemory",
  "zeCommandListAppendImageCopyFromMemory"]

def gen_enter_callback(f, func, params, enum_map, struct_types):
  f.write("  ZeApiCollector* collector =\n")
  f.write("    reinterpret_cast<ZeApiCollector*>(global_user_data);\n")
  f.write("  PTI_ASSERT(collector != nullptr);\n")
  f.write("  PTI_ASSERT(collector->correlator_ != nullptr);\n")
  f.write("\n")
  f.write("  if (!collector->correlator_->IsCollectionEnabled()) {\n")
  f.write("    *reinterpret_cast<uint64_t*>(instance_user_data) = 0;\n")
  f.write("    return;\n")
  f.write("  }\n")
  f.write("\n")
  f.write("  if (collector->options_.call_tracing) {\n")
  f.write("    std::stringstream stream;\n")
  f.write("    stream << \">>>> [\" << collector->GetTimestamp() << \"] \";\n")
  f.write("    if (collector->options_.need_pid) {\n")
  f.write("      stream << \"<PID:\" << utils::GetPid() << \"> \";\n")
  f.write("    }\n")
  f.write("    if (collector->options_.need_tid) {\n")
  f.write("      stream << \"<TID:\" << utils::GetTid() << \"> \";\n")
  f.write("    }\n")
  f.write("    stream << \"" + func + "\" << \":\";\n")
  for name, type in params:
    if type == "ze_ipc_mem_handle_t" or type == "ze_ipc_event_pool_handle_t":
      f.write("    stream << \" " + name + " = \" << (params->p" + name + ")->data;\n")
    elif is_struct_type(type, struct_types):
      # Passed by value (e.g. zeCommandListAppendLaunchKernelWithArguments)
      if type.find("ze_group_count_t") >= 0:
        f.write("    stream << \" " + name + " = {\" << (params->p" + name + ")->groupCountX << \", \";\n")
        f.write("    stream << (params->p" + name + ")->groupCountY << \", \";\n")
        f.write("    stream << (params->p" + name + ")->groupCountZ << \"}\";\n")
      elif type.find("ze_group_size_t") >= 0:
        f.write("    stream << \" " + name + " = {\" << (params->p" + name + ")->groupSizeX << \", \";\n")
        f.write("    stream << (params->p" + name + ")->groupSizeY << \", \";\n")
        f.write("    stream << (params->p" + name + ")->groupSizeZ << \"}\";\n")
      else:
        f.write("    stream << \" " + name + " = \" <<\n")
        f.write("      static_cast<const void*>(params->p" + name + ");\n")
    else:
      if ( (type.find("char*") >= 0 and type.find("char*") == len(type) - len("char*"))
          or (type.find("ze_bool_t*") >= 0) ):
        if func == "zeModuleGetFunctionPointer" or func == "zeModuleGetGlobalPointer":
          f.write("    if (*(params->p" + name + ") == nullptr) {\n")
          f.write("      stream << \" " + name + " = \" << \"0\";\n")
          f.write("    } else if (strlen(*(params->p" + name +")) == 0) {\n")
          f.write("      stream << \" " + name + " = \\\"\\\"\";\n")
          f.write("    } else {\n")
          f.write("      stream << \" " + name + " = \\\"\" << *(params->p" + name + ") << \"\\\"\";\n")
          f.write("    }\n")
        else:
          f.write("    if (*(params->p" + name + ") == nullptr) {\n")
          f.write("      stream << \" " + name + " = \" << \"0\";\n")
          f.write("    } else {\n")
          f.write("      stream << \" " + name + " = \" <<\n")
          if type.startswith("const "):
            f.write("        reinterpret_cast<const void*>(*(params->p" + name + "));\n")
          else:
            f.write("        reinterpret_cast<void*>(*(params->p" + name + "));\n")
          f.write("    }\n")
      else:
        f.write("    stream << \" " + name + " = \" << *(params->p" + name + ");\n")
        if name.find("Kernel") >= 0 and (func == "zeCommandListAppendLaunchKernel" or
            func == "zeCommandListAppendLaunchKernelWithArguments"):
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      bool demangle = collector->options_.demangle;\n")
          f.write("      std::string kernel_name =\n")
          f.write("        utils::ze::GetKernelName(*(params->p" + name + "), demangle);\n")
          f.write("      if (!kernel_name.empty()) {\n")
          f.write("        stream << \" (\" << kernel_name << \")\";\n")
          f.write("      }\n")
          f.write("    }\n")
        if ((name.find("ph") == 0 or name.find("pptr") == 0 or name.find("pCount") == 0) and
            (type == "ze_ipc_mem_handle_t*" or type == "ze_ipc_event_pool_handle_t*" or
             not is_struct_type(type[0:len(type) - 1], struct_types))):
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          if type == "ze_ipc_mem_handle_t*" or type == "ze_ipc_event_pool_handle_t*":
            f.write("      stream << \" (" + name[1:] + " = \" << (*(params->p" + name + "))->data << \")\";\n")
          else:
            f.write("      stream << \" (" + name[1:] + " = \" << **(params->p" + name + ") << \")\";\n")
          f.write("    }\n")
        elif type.find("ze_group_count_t*") >= 0:
          f.write("    if (*(params->p" + name +") != nullptr) {\n")
          f.write("      stream << \" {\" << (*(params->p" + name + "))->groupCountX << \", \";\n")
          f.write("      stream << (*(params->p" + name + "))->groupCountY << \", \";\n")
          f.write("      stream << (*(params->p" + name + "))->groupCountZ << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_event_pool_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->count << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_command_queue_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->ordinal << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->index << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->mode << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->priority << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_kernel_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \" \";\n")
          f.write("      if ((*(params->p" + name + "))->pKernelName == nullptr) {\n")
          f.write("        stream << \"0\";\n")
          f.write("      } else if (strlen((*(params->p" + name + "))->pKernelName) == 0) {\n")
          f.write("        stream << \" " + name + " = \\\"\\\"\";\n")
          f.write("      } else {\n")
          f.write("        stream << \"\\\"\" << (*(params->p" + name + "))->pKernelName << \"\\\"\";\n")
          f.write("        if (collector->options_.demangle) {\n")
          f.write("          stream << \" (\" << utils::Demangle((*(params->p" + name + "))->pKernelName) << \")\";\n")
          f.write("        }\n")
          f.write("        stream << \"}\";\n")
          f.write("      }\n")
          f.write("    }\n")
        elif type.find("ze_device_mem_alloc_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->ordinal << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_context_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_command_list_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->commandQueueGroupOrdinal << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_event_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->index << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->signal << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->wait << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_fence_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_image_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->type << \" \";\n")
          f.write("      stream << \"{\" << (*(params->p" + name +"))->format.layout << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->format.type << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->format.x << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->format.y << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->format.z << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->format.w << \"}\" << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->width << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->height << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->depth << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->arraylevels << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->miplevels << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_host_mem_alloc_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_external_memory_export_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_module_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->format << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->inputSize << \" \";\n")
          f.write("      stream << static_cast<const void*>((*(params->p" + name + "))->pInputModule) << \" \";\n")
          f.write("      if ((*(params->p" + name + ")) -> pBuildFlags != nullptr) \n")
          f.write("        stream << (*(params->p" + name + "))->pBuildFlags << \" \";\n")
          f.write("      else stream << 0 << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->pConstants << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_sampler_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->addressMode << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->filterMode << \" \";\n")
          f.write("      stream << static_cast<int>((*(params->p" + name + "))->isNormalized) << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_physical_mem_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->size << \"}\";\n")
          f.write("    }\n")
        elif type.find("ze_raytracing_mem_alloc_ext_desc_t*") >= 0:
          f.write("    if (*(params->p" + name + ") != nullptr) {\n")
          f.write("      stream << \" {\" << GetStructureTypeString((*(params->p" + name + "))->stype)\n")
          f.write("        << \"(0x\" << std::hex << (*(params->p" + name + "))->stype << std::dec << \") \";\n")
          f.write("      stream << (*(params->p" + name + "))->pNext << \" \";\n")
          f.write("      stream << (*(params->p" + name + "))->flags << \"}\";\n")
          f.write("    }\n")
  f.write("    stream << std::endl;\n")
  f.write("    collector->correlator_->Log(stream.str());\n")
  f.write("  }\n")
  f.write("  uint64_t& start_time = *reinterpret_cast<uint64_t*>(instance_user_data);\n")
  f.write("  start_time = collector->GetTimestamp();\n")

def gen_exit_callback(f, func, params, enum_map, struct_types,
                      result_type="ze_result_t"):
  f.write("  ZeApiCollector* collector =\n")
  f.write("    reinterpret_cast<ZeApiCollector*>(global_user_data);\n")
  f.write("  PTI_ASSERT(collector != nullptr);\n")
  f.write("  uint64_t end_time = collector->GetTimestamp();\n")
  f.write("  PTI_ASSERT(collector->correlator_ != nullptr);\n")
  f.write("\n")
  f.write("  uint64_t& start_time = *reinterpret_cast<uint64_t*>(instance_user_data);\n")
  f.write("\n")
  f.write("  if (start_time == 0) {\n")
  f.write("    return;\n")
  f.write("  }\n")
  f.write("\n")
  f.write("  PTI_ASSERT(start_time <= end_time);\n")
  f.write("  uint64_t time = end_time - start_time;\n")
  f.write("  collector->AddFunctionTime(\"" + func + "\", time);\n")
  f.write("  if (collector->options_.call_tracing) {\n")
  f.write("    std::stringstream stream;\n")
  f.write("    stream << \"<<<< [\" << end_time << \"] \";\n")
  f.write("    if (collector->options_.need_pid) {\n")
  f.write("      stream << \"<PID:\" << utils::GetPid() << \"> \";\n")
  f.write("    }\n")
  f.write("    if (collector->options_.need_tid) {\n")
  f.write("      stream << \"<TID:\" << utils::GetTid() << \"> \";\n")
  f.write("    }\n")
  f.write("    stream << \"" + func + "\";\n")
  if func in KERNEL_APPEND_FUNCS:
    f.write("    uint64_t kernel_id = collector->correlator_->GetKernelId();\n")
    f.write("    if (kernel_id > 0) {\n")
    f.write("      stream << \"(\" << kernel_id << \")\";\n")
    f.write("    }\n")
  elif func == "zeCommandQueueExecuteCommandLists":
    f.write("    uint32_t command_list_count = *(params->pnumCommandLists);\n")
    f.write("    ze_command_list_handle_t* command_lists = *(params->pphCommandLists);\n")
    f.write("    std::string kernel_call_id;\n")
    f.write("    if (command_lists != nullptr) {\n")
    f.write("      for (uint32_t i = 0; i < command_list_count; ++i) {\n")
    f.write("        std::vector<uint64_t> kernel_id_list =\n")
    f.write("          collector->correlator_->GetKernelId(command_lists[i]);\n")
    f.write("        std::vector<uint64_t> call_id_list =\n")
    f.write("          collector->correlator_->GetCallId(command_lists[i]);\n")
    f.write("        PTI_ASSERT(kernel_id_list.size() == call_id_list.size());\n")
    f.write("        for (size_t j = 0; j < kernel_id_list.size(); ++j) {\n")
    f.write("          kernel_call_id += std::to_string(kernel_id_list[j]) + \".\"\n")
    f.write("            + std::to_string(call_id_list[j]) + \",\";\n")
    f.write("        }\n")
    f.write("      }\n")
    f.write("    }\n")
    f.write("    \n")
    f.write("    if (!kernel_call_id.empty()) {\n")
    f.write("      kernel_call_id = kernel_call_id.substr(0, kernel_call_id.size() - 1);\n")
    f.write("      stream << \"(\" << kernel_call_id << \")\";\n")
    f.write("    }\n")
  f.write("    stream << \" [\" << time << \" ns]\";\n")
  if result_type == "ze_result_t":
    f.write("    if (result == ZE_RESULT_SUCCESS) {\n")
  else:
    f.write("    {\n") # no status: the result is the returned value
  for name, type in params:
    if (type != "ze_ipc_mem_handle_t*" and type != "ze_ipc_event_pool_handle_t*" and
        type.endswith("*") and is_struct_type(type[0:len(type) - 1], struct_types)):
      continue # struct output (e.g. a newer IPC handle type): not printed
    if name.find("ph") == 0:
      if func == "zeDeviceGet" or func == "zeDeviceGetSubDevices":
        f.write("      if (*(params->p" + name + ") != nullptr &&\n")
        f.write("          *(params->ppCount) != nullptr) {\n")
        f.write("        for (uint32_t i = 0; i < **(params->ppCount); ++i) {\n")
        f.write("          stream << \" " + name[1:] + "[\" << i << \"] = \" <<\n")
        f.write("            (*(params->p" + name + "))[i];\n")
        f.write("        }\n")
        f.write("      }\n")
      else:
        f.write("      if (*(params->p" + name + ") != nullptr) {\n")
        if type == "ze_ipc_mem_handle_t*" or type == "ze_ipc_event_pool_handle_t*":
          f.write("        stream << \" " + name[1:] + " = \" << (*(params->p" + name + "))->data;\n")
        else:
          f.write("        stream << \" " + name[1:] + " = \" << **(params->p" + name + ");\n")
        f.write("      }\n")
    elif name.find("pptr") == 0 or name == "pCount" or name == "pSize":
      f.write("      if (*(params->p" + name + ") != nullptr) {\n")
      if type == "ze_ipc_mem_handle_t*" or type == "ze_ipc_event_pool_handle_t*":
        f.write("        stream << \" " + name[1:] + " = \" << (*(params->p" + name + "))->data;\n")
      else:
        f.write("        stream << \" " + name[1:] + " = \" << **(params->p" + name + ");\n")
      f.write("      }\n")
    elif name.find("groupSize") == 0 and type.find("uint32_t*") == 0:
      f.write("      if (*(params->p" + name + ") != nullptr) {\n")
      f.write("        stream << \" " + name + " = \" << **(params->p" + name + ");\n")
      f.write("      }\n")
    elif name == "pName":
      f.write("      if (*(params->p" + name + ") != nullptr) {\n")
      f.write("        if (strlen(*(params->p" + name +")) == 0) {\n")
      f.write("          stream << \" " + name[1:] + " = \\\"\\\"\";\n")
      f.write("        } else {\n")
      f.write("          stream << \" " + name[1:] + " = \\\"\" << *(params->p" + name + ") << \"\\\"\";\n")
      f.write("        }\n")
      f.write("      }\n")
  f.write("    }\n")
  if result_type == "ze_result_t":
    f.write("    stream << \" -> \" << GetResultString(result) << \n")
    f.write("      \"(0x\" << result << \")\" << std::endl;\n")
  else:
    f.write("    stream << \" -> \" << result << std::endl;\n")
  f.write("    collector->correlator_->Log(stream.str());\n")
  f.write("  }\n")
  f.write("\n")
  f.write("  if (collector->callback_ != nullptr) {\n")
  if func in KERNEL_APPEND_FUNCS:
    f.write("    collector->callback_(\n")
    f.write("        collector->callback_data_,\n")
    f.write("        std::to_string(collector->correlator_->GetKernelId()),\n")
    f.write("        \"" + func + "\",\n")
    f.write("        start_time, end_time);\n")
  elif func == "zeCommandQueueExecuteCommandLists":
    f.write("    uint32_t command_list_count = *(params->pnumCommandLists);\n")
    f.write("    ze_command_list_handle_t* command_lists = *(params->pphCommandLists);\n")
    f.write("    std::string kernel_call_id;\n")
    f.write("    if (command_lists != nullptr) {\n")
    f.write("      for (uint32_t i = 0; i < command_list_count; ++i) {\n")
    f.write("        std::vector<uint64_t> kernel_id_list =\n")
    f.write("          collector->correlator_->GetKernelId(command_lists[i]);\n")
    f.write("        std::vector<uint64_t> call_id_list =\n")
    f.write("          collector->correlator_->GetCallId(command_lists[i]);\n")
    f.write("        PTI_ASSERT(kernel_id_list.size() == call_id_list.size());\n")
    f.write("        for (size_t j = 0; j < kernel_id_list.size(); ++j) {\n")
    f.write("          kernel_call_id += std::to_string(kernel_id_list[j]) + \".\"\n")
    f.write("            + std::to_string(call_id_list[j]) + \",\";\n")
    f.write("        }\n")
    f.write("      }\n")
    f.write("    }\n")
    f.write("    \n")
    f.write("    if (!kernel_call_id.empty()) {\n")
    f.write("      kernel_call_id = kernel_call_id.substr(0, kernel_call_id.size() - 1);\n")
    f.write("    } else {\n")
    f.write("      kernel_call_id = \"0\";\n")
    f.write("    }\n")
    f.write("    collector->callback_(\n")
    f.write("        collector->callback_data_, kernel_call_id,\n")
    f.write("        \"" + func + "\",\n")
    f.write("        start_time, end_time);\n")
  else:
    f.write("    collector->callback_(\n")
    f.write("        collector->callback_data_, \"0\",\n")
    f.write("        \"" + func + "\",\n")
    f.write("        start_time, end_time);\n")
  f.write("  }\n")

def gen_hot_enter_callback(f, func):
  f.write("  PTI_ASSERT(global_user_data != nullptr);\n")
  f.write("  ZeApiCollector* collector =\n")
  f.write("    reinterpret_cast<ZeApiCollector*>(global_user_data);\n")
  f.write("  uint64_t& start_time = *reinterpret_cast<uint64_t*>(instance_user_data);\n")
  f.write("  start_time = collector->GetTimestamp();\n")

def gen_hot_exit_callback(f, func):
  f.write("  PTI_ASSERT(global_user_data != nullptr);\n")
  f.write("  ZeApiCollector* collector =\n")
  f.write("    reinterpret_cast<ZeApiCollector*>(global_user_data);\n")
  f.write("  uint64_t end_time = collector->GetTimestamp();\n")
  f.write("\n")
  f.write("  uint64_t& start_time = *reinterpret_cast<uint64_t*>(instance_user_data);\n")
  f.write("  PTI_ASSERT(start_time > 0);\n")
  f.write("  PTI_ASSERT(start_time <= end_time);\n")
  f.write("  uint64_t time = end_time - start_time;\n")
  f.write("  collector->AddFunctionTime(\"" + func + "\", time);\n")

def gen_callbacks(f, func_list, group_map, param_map, enum_map, struct_types,
                  hot_functions):
  for func in func_list:
    callback_cond = ""
    if func in group_map:
      group, callback = group_map[func]
      callback_cond = callback[1]

    assert func in param_map
    param_struct_name, result_type, params = param_map[func]
    if callback_cond:
      f.write("#if " + callback_cond + "\n")
    f.write("static void " + func + "OnEnter(\n")
    f.write("    " + param_struct_name + "* params,\n")
    f.write("    " + result_type + " result,\n")
    f.write("    void* global_user_data,\n")
    f.write("    void** instance_user_data) {\n")
    if hot_functions:
      gen_hot_enter_callback(f, func)
    else:
      gen_enter_callback(f, func, params, enum_map, struct_types)
    f.write("}\n")
    f.write("\n")
    f.write("static void " + func + "OnExit(\n")
    f.write("    " + param_struct_name + "* params,\n")
    f.write("    " + result_type + " result,\n")
    f.write("    void* global_user_data,\n")
    f.write("    void** instance_user_data) {\n")
    if hot_functions:
      gen_hot_exit_callback(f, func)
    else:
      gen_exit_callback(f, func, params, enum_map, struct_types,
                        result_type)
    f.write("}\n")
    if callback_cond:
      f.write("#endif //" + callback_cond + "\n")
    f.write("\n")

def main():
  args = [arg for arg in sys.argv[1:] if not arg.startswith("--")]
  options = [arg for arg in sys.argv[1:] if arg.startswith("--")]
  if len(args) < 2 or any(o != "--hot-functions" for o in options):
    print("Usage: python gen_tracing_callbacks.py <output_include_path> "
          "<l0_include_path> [--hot-functions]")
    print("  Writes <output_include_path>/tracing.gen: call logging and timing")
    print("  callbacks for ze_tracer/onetrace, or with --hot-functions the")
    print("  timing-only callbacks of samples/ze_hot_functions.")
    sys.exit(1)
  hot_functions = "--hot-functions" in options

  dst_path = args[0]
  if (not os.path.exists(dst_path)):
    os.mkdir(dst_path)

  dst_file_path = os.path.join(dst_path, "tracing.gen")
  if (os.path.isfile(dst_file_path)):
    os.remove(dst_file_path)

  l0_path = args[1]
  l0_file_path = os.path.join(l0_path, "ze_api.h")
  with open(l0_file_path, "rt") as l0_file:
    l0_text = l0_file.read()
    core_func_list = get_func_list(l0_file)
    group_map = get_callback_group_map(l0_file)

  # 1.0 APIs go through zet_core_callbacks_t. The newer ones (declared in
  # zel_tracing_register_cb.h, which old headers may lack) need per-API
  # registration.
  core_func_list = [func for func in core_func_list if func in group_map]
  ext_func_list = []
  register_file_path = os.path.join(l0_path, "layers",
                                    "zel_tracing_register_cb.h")
  if os.path.isfile(register_file_path):
    with open(register_file_path, "rt") as register_file:
      register_text = register_file.read()
    l0_text += register_text
    ext_func_list = [func for func in get_register_func_list(register_text)
                     if not func in group_map]

  func_list = core_func_list + ext_func_list
  param_map = get_param_map(l0_text, func_list)
  enum_map = get_enum_map(l0_path)
  struct_types = get_struct_types(l0_path)

  dst_file = open(dst_file_path, "wt")
  if hot_functions:
    gen_callbacks(dst_file, func_list, group_map, param_map, enum_map,
                  struct_types, True)
    gen_api(dst_file, core_func_list, group_map, ext_func_list,
            "SetTracingFunctions")
  else:
    gen_result_converter(dst_file, enum_map)
    gen_structure_type_converter(dst_file, enum_map)
    gen_callbacks(dst_file, func_list, group_map, param_map, enum_map,
                  struct_types, False)
    gen_api(dst_file, core_func_list, group_map, ext_func_list,
            "SetTracingAPIs")
  dst_file.close()

if __name__ == "__main__":
  main()
