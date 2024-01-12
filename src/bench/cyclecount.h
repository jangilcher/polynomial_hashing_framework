// MIT License
//
// Copyright (c) 2023 Jan Gilcher, Jérôme Govinden
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef CYCLECOUNT_H
#define CYCLECOUNT_H
#include <stdint.h>

#if defined(__amd64__) || defined(__i386__)

static inline uint64_t rdtscp_start() {
    uint64_t v;
    uint32_t cycles_high, cycles_low;
    __asm__ volatile("CPUID\n\t"
                     "RDTSC\n\t"
                     "mov %%edx, %0\n\t"
                     "mov %%eax, %1\n\t"
                     : "=r"(cycles_high), "=r"(cycles_low)::"%rax", "%rbx",
                       "%rcx", "%rdx");
    v = ((uint64_t)cycles_high) << 32 | (uint64_t)cycles_low;
    return v;
}

static inline uint64_t rdtscp_stop() {
    uint64_t v;
    uint32_t cycles_high, cycles_low;
    __asm__ volatile("RDTSCP\n\t"
                     "mov %%edx, %0\n\t"
                     "mov %%eax, %1\n\t"
                     "CPUID\n\t"
                     : "=r"(cycles_high), "=r"(cycles_low)::"%rax", "%rbx",
                       "%rcx", "%rdx");
    v = ((uint64_t)cycles_high) << 32 | (uint64_t)cycles_low;
    return v;
}

#elif defined(__aarch64__)

#define rdtscp_start get_cycles
#define rdtscp_stop get_cycles

static inline uint64_t get_cycles() {
    register uint64_t ret;
    __asm__ __volatile__("isb; mrs %0, cntvct_el0" : "=r"(ret));
    return ret;
}

#endif

#endif
