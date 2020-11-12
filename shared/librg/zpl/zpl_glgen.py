#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This Software is dual licensed under the following licenses:

# Unlicense
# This is free and unencumbered software released into the public domain.

# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.

# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

# For more information, please refer to < http: // unlicense.org/>

# Apache 2.0
# Copyright 2017-2018 Dominik Madarász < zaklaus@outlook.com >
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http: // www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# glgen generator module

# Allow Python 2.6+ to use the print() function
from __future__ import print_function

import re
import os

# Try to import Python 3 library urllib.request
# and if it fails, fall back to Python 2 urllib2
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2

# CODE_HEADER copyright header
CODE_HEADER = r'''/*

ZPL - GLGEN module

Usage:
  #define ZPL_GLGEN_IMPLEMENTATION exactly in ONE source file right BEFORE including the library, like:

  #define ZPL_GLGEN_IMPLEMENTATION
  #include"zpl_glgen.h"

  Other files include just the header file.

Credits:
  See AUTHORS.md

GitHub:
  https://github.com/zpl-c/zpl

Version History:
  1.0.0 - Initial version

  This Software is dual licensed under the following licenses:

  Unlicense
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org/>

  Apache 2.0
  Copyright 2017-2018 Dominik Madarász <zaklaus@outlook.com>
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
    
      http://www.apache.org/licenses/LICENSE-2.0
    
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License. 

*/

'''


# Download glcorearb.h
if not os.path.exists('glcorearb.h'):
    print('Downloading glcorearb.h...')
    web = urllib2.urlopen('https://www.opengl.org/registry/api/GL/glcorearb.h')
    with open('glcorearb.h', 'wb') as f:
        f.writelines(web.readlines())
else:
    print('Reusing glcorearb.h...')

# Parse function names from glcorearb.h
print('Parsing glcorearb.h header...')
procs = []
p = re.compile(r'GLAPI.*APIENTRY\s+(\w+)')
with open('glcorearb.h', 'r') as f:
    for line in f:
        m = p.match(line)
        if m:
            procs.append(m.group(1))
procs.sort()


def proc_t(proc):
    return {'p': proc,
            'p_s': 'glgen' + proc[2:],
            'p_t': 'PFN' + proc.upper() + 'PROC'}


# Generate glgen.h
print('Generating zpl_glgen.h...')
with open('zpl_glgen.h', 'wb') as f:
    f.write(bytes(CODE_HEADER, 'utf-8'))
    f.write(br'''#ifndef ZPL_INCLUDE_ZPL_GLGEN_H
#define ZPL_INCLUDE_ZPL_GLGEN_H

#include "glcorearb.h"

#ifndef __gl_h_
#define __gl_h_
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*zpl_gl3WglProc)(void);

/* glgen api */
int zpl_glgen_init(void);
int zpl_glgen_is_supported(int major, int minor);
zpl_gl3WglProc zpl_glgen_get_proc_address(char const *proc);

/* OpenGL functions */
''')
    for proc in procs:
        f.write('extern {0[p_t]: <52} {0[p_s]};\n'.format(
            proc_t(proc)).encode("utf-8"))
    f.write(b'\n')
    for proc in procs:
        f.write('#define {0[p]: <45} {0[p_s]}\n'.format(
            proc_t(proc)).encode("utf-8"))
    f.write(br'''
#ifdef __cplusplus
}
#endif

#endif

#if defined(ZPL_GLGEN_IMPLEMENTATION) && !defined(ZPL_GLGEN_IMPLEMENTATION_DONE)
#define ZPL_GLGEN_IMPLEMENTATION_DONE

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#include <windows.h>

static HMODULE zpl__glgen_libgl;

static void zpl__glgen_open_libgl (void) { zpl__glgen_libgl = LoadLibraryA("opengl32.dll"); }
static void zpl__glgen_close_libgl(void) { FreeLibrary(zpl__glgen_libgl); }

static zpl_gl3WglProc zpl__glgen_get_proc(char const *proc)
{
	zpl_gl3WglProc res;

	res = (zpl_gl3WglProc) wglGetProcAddress(proc);
	if (!res)
		res = (zpl_gl3WglProc) GetProcAddress(zpl__glgen_libgl, proc);
	return res;
}

#elif defined(__APPLE__) || defined(__APPLE_CC__)

#include <Carbon/Carbon.h>

CFBundleRef zpl__glgen_bundle;
CFURLRef zpl__glgen_bundleURL;

static void zpl__glgen_open_libgl(void)
{
	zpl__glgen_bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
		CFSTR("/System/Library/Frameworks/OpenGL.framework"),
		kCFURLPOSIXPathStyle, true);

	zpl__glgen_bundle = CFBundleCreate(kCFAllocatorDefault, zpl__glgen_bundleURL);
	assert(zpl__glgen_bundle != NULL);
}

static void zpl__glgen_close_libgl(void)
{
	CFRelease(zpl__glgen_bundle);
	CFRelease(zpl__glgen_bundleURL);
}

static GLGENglProc zpl__glgen_get_proc(char const *proc)
{
	GLGENglProc res;

	CFStringRef procname = CFStringCreateWithCString(kCFAllocatorDefault, proc,
		kCFStringEncodingASCII);
	res = (GLGENglProc) CFBundleGetFunctionPointerForName(zpl__glgen_bundle, procname);
	CFRelease(procname);
	return res;
}

#else

#include <dlfcn.h>
#include <GL/glx.h>

static void *zpl__glgen_libgl;
static PFNGLXGETPROCADDRESSPROC zpl__glgen_glx_get_proc_address;

static void zpl__glgen_open_libgl(void)
{
	zpl__glgen_libgl = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
	zpl__glgen_glx_get_proc_address = (PFNGLXGETPROCADDRESSPROC) dlsym(zpl__glgen_libgl, "glXGetProcAddressARB");
}

static void zpl__glgen_close_libgl(void) { dlclose(zpl__glgen_libgl); }

static zpl_gl3WglProc zpl__glgen_get_proc(char const *proc)
{
	zpl_gl3WglProc res;

	res = (zpl_gl3WglProc) zpl__glgen_glx_get_proc_address((const GLubyte *) proc);
	if (!res)
		res = (zpl_gl3WglProc) dlsym(zpl__glgen_libgl, proc);
	return res;
}

#endif

static struct {
	int major, minor;
} zpl__glgen_version;

static int zpl__glgen_parse_version(void)
{
	if (!glGetIntegerv)
		return -1;

	glGetIntegerv(GL_MAJOR_VERSION, &zpl__glgen_version.major);
	glGetIntegerv(GL_MINOR_VERSION, &zpl__glgen_version.minor);

	if (zpl__glgen_version.major < 3)
		return -1;
	return 0;
}

static void zpl__glgen_load_procs(void);

int zpl_glgen_init(void)
{
	zpl__glgen_open_libgl();
	zpl__glgen_load_procs();
	zpl__glgen_close_libgl();
	return zpl__glgen_parse_version();
}

int zpl_glgen_is_supported(int major, int minor)
{
	if (major < 3)
		return 0;
	if (zpl__glgen_version.major == major)
		return zpl__glgen_version.minor >= minor;
	return zpl__glgen_version.major >= major;
}

zpl_gl3WglProc zpl_glgen_get_proc_address(char const *proc)
{
	return zpl__glgen_get_proc(proc);
}

''')
    for proc in procs:
        f.write('{0[p_t]: <52} {0[p_s]};\n'.format(
            proc_t(proc)).encode("utf-8"))
    f.write(br'''
static void zpl__glgen_load_procs(void)
{
''')
    for proc in procs:
        f.write('\t{0[p_s]} = ({0[p_t]}) zpl__glgen_get_proc("{0[p]}");\n'.format(
            proc_t(proc)).encode("utf-8"))
    f.write(b'''}


#endif /* ZPL_GLGEN_IMPLEMENTATION */
''')
