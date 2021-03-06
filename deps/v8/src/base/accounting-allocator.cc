// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/base/accounting-allocator.h"

#include <cstdlib>

#if V8_LIBC_BIONIC
#include <malloc.h>  // NOLINT
#endif

namespace v8 {
namespace base {

void* AccountingAllocator::Allocate(size_t bytes) {
  void* memory = malloc(bytes);
  if (memory) {
    AtomicWord current =
        NoBarrier_AtomicIncrement(&current_memory_usage_, bytes);
    AtomicWord max = NoBarrier_Load(&max_memory_usage_);
    while (current > max) {
      max = NoBarrier_CompareAndSwap(&max_memory_usage_, max, current);
    }
  }
  return memory;
}

void AccountingAllocator::Free(void* memory, size_t bytes) {
  free(memory);
  NoBarrier_AtomicIncrement(&current_memory_usage_,
                            -static_cast<AtomicWord>(bytes));
}

size_t AccountingAllocator::GetCurrentMemoryUsage() const {
  return NoBarrier_Load(&current_memory_usage_);
}

size_t AccountingAllocator::GetMaxMemoryUsage() const {
  return NoBarrier_Load(&max_memory_usage_);
}

}  // namespace base
}  // namespace v8
