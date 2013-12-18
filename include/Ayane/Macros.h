/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_MACROS_H_
#define AYANE_MACROS_H_

/*
 * A macro to disallow the default constructor.
 * This should be used in the private declarations for a class.
 */
#define AYANE_DISALLOW_DEFAULT_CTOR(ClassName) \
    ClassName();

/*
 * A macro to disallow the copy constructor and operator= functions.
 * This should be used in the private declarations for a class.
 */
#define AYANE_DISALLOW_COPY_AND_ASSIGN(ClassName)   \
    ClassName(const ClassName&);              \
    void operator=(const ClassName&)

/*
 * A macro to disallow the default constructor, copy constructor and 
 * operator= functions.
 * This should be used in the private declarations for a class.
 */
#define AYANE_DISALLOW_DEFAULT_CTOR_COPY_AND_ASSIGN(ClassName) \
            AYANE_DISALLOW_DEFAULT_CTOR(ClassName)             \
            AYANE_DISALLOW_COPY_AND_ASSIGN(ClassName)

#endif