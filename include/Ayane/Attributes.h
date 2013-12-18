/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_ATTRIBUTES_H_
#define AYANE_ATTRIBUTES_H_


#if (defined(__GNUC__) || defined(__GNUG__)) && !(defined(__clang__) || defined(__INTEL_COMPILER))

    // Is a GNU compiler.
    #define GNU_COMPILER 1

    // GNU VERSION
    #define COMPILER_VERSION_MAJOR (__GNUC__)
    #define COMPILER_VERSION_MINOR (__GNUC_MINOR__)
    #define COMPILER_VERSION_REVISION (__GNUC_PATCHLEVEL__)

    // G++ or GCC?
    #if (defined(__GNUC__) && defined(__GNUG))
        // G++
        #define GNU_CXX_COMPILER 1
    #else
        // GCC
        #define GNG_C_COMPILER 1
    #endif

#elif defined(__clang__)

    // Is a Clang compiler.
    #define CLANG_COMPILER 1

    // Clang version
    #define COMPILER_VERSION_MAJOR (__clang_major__)
    #define COMPILER_VERSION_MINOR (__clang_minor__)
    #define COMPILER_VERSION_REVISION (__clang_patchlevel__)

    // Clang++ or Clang?
    #if (defined(__GNUC__) && defined(__GNUG))
        // Clang++
        #define CLANG_CXX_COMPILER 1
    #else
        // Clang
        #define CLANG_C_COMPILER 1
    #endif

#elif defined(_MSC_VER)

    // Visual C compiler
    #define MSVC_COMPILER 1

    // MSVC version (seriously Microsoft?)
    #define COMPILER_VERSION_MAJOR (_MSC_VER / 100)
    #define COMPILER_VERSION_MINOR (_MSC_VER % 100)
    #define COMPILER_VERSION_REVISION (0)

#else

    // Darn...
    #error "Unsupported compiler."

#endif


#if ( defined(GNU_COMPILER) || defined(CLANG_COMPILER) )
// GNU GCC/G++ and Clang compatible

    #define force_inline __attribute__((always_inline)) __inline__
    #define unreachable __builtin_unreachable

#elif defined(MSVC_COMPILER)
// Microsoft Visual C

    #define force_inline __forceinline
    #define unreachable __assume(0)

#endif

#endif
