/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_KERNEL_FS_FILESYSTEM_H_
#define XENIA_KERNEL_FS_FILESYSTEM_H_

#include <xenia/common.h>
#include <xenia/core.h>

#include <xenia/kernel/fs/entry.h>


namespace xe {
namespace kernel {
namespace fs {


class Device;


class FileSystem {
public:
  FileSystem(xe_pal_ref pal);
  ~FileSystem();

  int RegisterDevice(const char* path, Device* device);
  int RegisterLocalDirectoryDevice(const char* path,
                                   const xechar_t* local_path);
  int RegisterDiscImageDevice(const char* path, const xechar_t* local_path);

  int CreateSymbolicLink(const char* path, const char* target);
  int DeleteSymbolicLink(const char* path);

  Entry* ResolvePath(const char* path);

private:
  xe_pal_ref pal_;
};


}  // namespace fs
}  // namespace kernel
}  // namespace xe


#endif  // XENIA_KERNEL_FS_FILESYSTEM_H_