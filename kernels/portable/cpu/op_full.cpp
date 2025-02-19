/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <executorch/kernels/portable/cpu/scalar_utils.h>
#include <executorch/runtime/kernel/kernel_includes.h>

namespace torch {
namespace executor {
namespace native {

using Tensor = executorch::aten::Tensor;
using ScalarType = executorch::aten::ScalarType;

Tensor& full_out(
    KernelRuntimeContext& ctx,
    const IntArrayRef sizes,
    const Scalar& fill_value,
    Tensor& out) {
  (void)ctx;

  ScalarType val_type = utils::get_scalar_dtype(fill_value);
  ScalarType out_type = out.scalar_type();

  // Resize for dynamic shape
  ET_KERNEL_CHECK_MSG(
      ctx,
      resize_tensor(out, sizes) == Error::Ok,
      InvalidArgument,
      out,
      "Failed to resize output tensor.");

  constexpr auto name = "full.out";

  ET_SWITCH_SCALAR_OBJ_TYPES(val_type, ctx, name, CTYPE_VAL, [&] {
    CTYPE_VAL val;
    ET_KERNEL_CHECK(
        ctx, utils::extract_scalar(fill_value, &val), InvalidArgument, );

    ET_SWITCH_REALHBBF16_TYPES(out_type, ctx, name, CTYPE_OUT, [&] {
      CTYPE_OUT val_casted = static_cast<CTYPE_OUT>(val);
      auto data_out = out.mutable_data_ptr<CTYPE_OUT>();
      for (size_t i = 0; i < out.numel(); ++i) {
        data_out[i] = val_casted;
      }
    });
  });

  return out;
}

} // namespace native
} // namespace executor
} // namespace torch
