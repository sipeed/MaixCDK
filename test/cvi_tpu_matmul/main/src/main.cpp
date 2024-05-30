
#include "maix_basic.hpp"
#include "main.h"

using namespace maix;

#include <iostream>
#include <random>
#include <functional>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
#include "cviruntime.h"
#include "cviruntime_extra.h"

using system_clock = std::chrono::system_clock;
using duration = std::chrono::duration<double, std::milli>;

static void matmul(int8_t *left, int8_t *right, int32_t *res,
                   int32_t m, int32_t k, int32_t n) {
  for (auto _m = 0; _m < m; ++_m) {
    for (auto _n = 0; _n < n; ++_n) {
      int yi = _m * n + _n;
      res[yi] = 0;
      for (auto _k = 0; _k < k; ++_k) {
        auto li = _m * k + _k;
        auto ri = _k * n + _n;
        res[yi] += left[li] * right[ri];
      }
    }
  }
}

int _main(int argc, char **argv) {
  if (argc < 4)
    return 1;

  uint32_t m = std::atoi(argv[1]);
  uint32_t k = std::atoi(argv[2]);
  uint32_t n = std::atoi(argv[3]);

  std::cout << "L shape: (" << m << "x" << k << ")"
            << ", R shape: (" << n << "x" << k << ")"
            << "\n";


  CVI_RT_HANDLE ctx;
  CVI_RT_Init(&ctx);
  CVI_RT_MEM mem_a = CVI_RT_MemAlloc(ctx, m * k);
  CVI_RT_MEM mem_b = CVI_RT_MemAlloc(ctx, k * n);
  CVI_RT_MEM mem_r = CVI_RT_MemAlloc(ctx, m * n * sizeof(int));

  // create matmul kernel function
  auto kfn = CVI_NN_PrepareMatrixMulKernelFunc(ctx, CVI_FMT_INT8, m, k, n);

  auto vptr_a = (int8_t *)CVI_RT_MemGetVAddr(mem_a);
  auto vptr_b = (int8_t *)CVI_RT_MemGetVAddr(mem_b);

  std::random_device rd;
  auto randint_gen = [&](int begin, int end) {
    return std::bind(std::uniform_int_distribution<int>(begin, end),
                     std::default_random_engine(rd()));
  };
  auto random = randint_gen(-128, 127);
  for (uint32_t i = 0; i < m * k; ++i) {
    vptr_a[i] = random();
  }

  for (uint32_t i = 0; i < k * n; ++i) {
    vptr_b[i] = random();
  }

  std::vector<int32_t> ref(m * n);
  auto start0 = system_clock::now();
  matmul(vptr_a, vptr_b, ref.data(), m, k, n);
  auto end0 = system_clock::now();
  duration d0 = end0 - start0;
  std::cout << "soft run duration: " << d0.count() << "(ms)\n";

  // flush cpu cache data of left & right matrixes.
  CVI_RT_MemFlush(ctx, mem_a);
  CVI_RT_MemFlush(ctx, mem_b);

  // run matrix mul kernel function
  auto start = system_clock::now();
  CVI_NN_RunKernelFunc(kfn, 3,
                       CVI_RT_MemGetPAddr(mem_a),
                       CVI_RT_MemGetPAddr(mem_b),
                       CVI_RT_MemGetPAddr(mem_r));
  CVI_RT_MemInvld(ctx, mem_r);
  auto end = system_clock::now();
  duration d = end - start;
  std::cout << "hardware run duration: " << d.count() << "(ms)\n";

  // get result and compare with reference
  int32_t *vptr_r = (int32_t *)CVI_RT_MemGetVAddr(mem_r);

  uint32_t err = 0;
  for (uint32_t i = 0; i < m; ++i) {
    for (uint32_t j = 0; j < n; ++j) {
      uint32_t idx = i * n + j;
      if (vptr_r[idx] != ref[idx]) {
        std::cout << "compare fail, index: <" << i << "," << j << "> expect: "
                  << ref[idx] << ", got: " << vptr_r[idx] << "\n";
        err++;
      }
    }
  }

  CVI_RT_MemFree(ctx, mem_r);
  CVI_RT_MemFree(ctx, mem_b);
  CVI_RT_MemFree(ctx, mem_a);
  CVI_NN_DestroyKernelFunc(kfn);
  CVI_RT_DeInit(ctx);

  if (err) {
    return 1;
  } else {
    return 0;
  }
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


