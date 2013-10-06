/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <xenia/kernel/modules/xboxkrnl/xboxkrnl_module.h>

#include <xenia/kernel/shim_utils.h>
#include <xenia/kernel/xex2.h>
#include <xenia/kernel/modules/xboxkrnl/kernel_state.h>
#include <xenia/kernel/modules/xboxkrnl/xboxkrnl_private.h>
#include <xenia/kernel/modules/xboxkrnl/objects/xmodule.h>


using namespace xe;
using namespace xe::kernel;
using namespace xe::kernel::xboxkrnl;


namespace xe {
namespace kernel {
namespace xboxkrnl {


X_STATUS xeExGetXConfigSetting(
    uint16_t category, uint16_t setting, void* buffer, uint16_t buffer_size,
    uint16_t* required_size) {
  uint16_t setting_size = 0;
  uint32_t value = 0;

  // TODO(benvanik): have real structs here that just get copied from.
  // http://free60.org/XConfig
  switch (category) {
  case 0x0003:
    // XCONFIG_USER_CATEGORY
    switch (setting) {
    case 0x000A:
      // VideoFlags
      setting_size = 4;
      value = 0x00040000;
      break;
    default:
      XEASSERTALWAYS();
      return X_STATUS_INVALID_PARAMETER_2;
    }
    break;
  default:
    XEASSERTALWAYS();
    return X_STATUS_INVALID_PARAMETER_1;
  }

  if (buffer_size < setting_size) {
    return X_STATUS_BUFFER_TOO_SMALL;
  }
  if (!buffer && buffer_size) {
    return X_STATUS_INVALID_PARAMETER_3;
  }

  if (buffer) {
    XESETUINT32BE(buffer, value);
  }
  if (required_size) {
    *required_size = setting_size;
  }

  return X_STATUS_SUCCESS;
}


SHIM_CALL ExGetXConfigSetting_shim(
    xe_ppc_state_t* ppc_state, KernelState* state) {
  uint16_t category = SHIM_GET_ARG_16(0);
  uint16_t setting = SHIM_GET_ARG_16(1);
  uint32_t buffer_ptr = SHIM_GET_ARG_32(2);
  uint16_t buffer_size = SHIM_GET_ARG_16(3);
  uint32_t required_size_ptr = SHIM_GET_ARG_32(4);

  XELOGD(
      "ExGetXConfigSetting(%.4X, %.4X, %.8X, %.4X, %.8X)",
      category, setting, buffer_ptr, buffer_size, required_size_ptr);

  void* buffer = buffer_ptr ? SHIM_MEM_ADDR(buffer_ptr) : NULL;
  uint16_t required_size = 0;
  X_STATUS result = xeExGetXConfigSetting(
      category, setting, buffer, buffer_size, &required_size);

  if (required_size_ptr) {
    SHIM_SET_MEM_16(required_size_ptr, required_size);
  }

  SHIM_SET_RETURN(result);
}


int xeXexCheckExecutablePriviledge(uint32_t privilege) {
  KernelState* state = shared_kernel_state_;
  XEASSERTNOTNULL(state);

  // BOOL
  // DWORD Privilege

  // Privilege is bit position in xe_xex2_system_flags enum - so:
  // Privilege=6 -> 0x00000040 -> XEX_SYSTEM_INSECURE_SOCKETS
  uint32_t mask = 1 << privilege;

  XModule* module = state->GetExecutableModule();
  if (!module) {
    return 0;
  }
  xe_xex2_ref xex = module->xex();

  const xe_xex2_header_t* header = xe_xex2_get_header(xex);
  uint32_t result = (header->system_flags & mask) > 0;

  xe_xex2_release(xex);
  module->Release();

  return result;
}


SHIM_CALL XexCheckExecutablePrivilege_shim(
    xe_ppc_state_t* ppc_state, KernelState* state) {
  uint32_t privilege = SHIM_GET_ARG_32(0);

  XELOGD(
      "XexCheckExecutablePrivilege(%.8X)",
      privilege);

  int result = xeXexCheckExecutablePriviledge(privilege);

  SHIM_SET_RETURN(result);
}


int xeXexGetModuleHandle(const char* module_name,
                         X_HANDLE* module_handle_ptr) {
  KernelState* state = shared_kernel_state_;
  XEASSERTNOTNULL(state);

  // BOOL
  // LPCSZ ModuleName
  // LPHMODULE ModuleHandle

  XModule* module = state->GetModule(module_name);
  if (!module) {
    return 0;
  }

  // NOTE: we don't retain the handle for return.
  *module_handle_ptr = module->handle();

  module->Release();

  return 1;
}


SHIM_CALL XexGetModuleHandle_shim(
    xe_ppc_state_t* ppc_state, KernelState* state) {
  uint32_t module_name_ptr = SHIM_GET_ARG_32(0);
  const char* module_name = (const char*)SHIM_MEM_ADDR(module_name_ptr);
  uint32_t module_handle_ptr = SHIM_GET_ARG_32(1);

  XELOGD(
      "XexGetModuleHandle(%s, %.8X)",
      module_name, module_handle_ptr);

  X_HANDLE module_handle = 0;
  int result = xeXexGetModuleHandle(module_name, &module_handle);
  if (result) {
    SHIM_SET_MEM_32(module_handle_ptr, module_handle);
  }

  SHIM_SET_RETURN(result);
}


// SHIM_CALL XexGetModuleSection_shim(
//     xe_ppc_state_t* ppc_state, KernelState* state) {
// }


// SHIM_CALL XexGetProcedureAddress_shim(
//     xe_ppc_state_t* ppc_state, KernelState* state) {
// }


}  // namespace xboxkrnl
}  // namespace kernel
}  // namespace xe


void xe::kernel::xboxkrnl::RegisterModuleExports(
    ExportResolver* export_resolver, KernelState* state) {
  SHIM_SET_MAPPING("xboxkrnl.exe", ExGetXConfigSetting, state);

  SHIM_SET_MAPPING("xboxkrnl.exe", XexCheckExecutablePrivilege, state);

  SHIM_SET_MAPPING("xboxkrnl.exe", XexGetModuleHandle, state);
  // SHIM_SET_MAPPING("xboxkrnl.exe", XexGetModuleSection, state);
  // SHIM_SET_MAPPING("xboxkrnl.exe", XexGetProcedureAddress, state);
}