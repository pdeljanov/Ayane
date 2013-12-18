/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_DPOINTER_H_
#define AYANE_DPOINTER_H_

#define A_D(Class) Class##Private * const d = _d_func()
#define A_Q(Class) Class * const q = _q_func()

#define AYANE_DECLARE_PRIVATE(Class) \
    inline Class##Private* _d_func() \
    { \
        return reinterpret_cast<Class##Private *>(d_ptr); \
    } \
    inline const Class##Private* _d_func() const \
    { \
        return reinterpret_cast<const Class##Private *>(d_ptr); \
    } \
    friend class Class##Private;

#define AYANE_DECLARE_PUBLIC(Class) \
    inline Class* _q_func() \
    { \
        return static_cast<Class *>(q_ptr); \
    } \
    inline const Class* _q_func() const \
    { \
        return static_cast<const Class *>(q_ptr); \
    } \
    friend class Class;

#endif