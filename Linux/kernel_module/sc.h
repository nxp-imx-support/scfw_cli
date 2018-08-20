/*
 * Copyright 2018 NXP
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **/
#ifndef SC_H
#define SC_H
#include <linux/ioctl.h>
 
typedef struct{
	int resource;
	unsigned char mode;
	int err;
} sc_pm_res_t;
 
typedef struct{
	int resource;
	unsigned char ctl;
	unsigned int val;
	int err;
} sc_misc_ctl_t;

typedef struct{
	unsigned int build;
	unsigned int commit;
} sc_misc_build_info_t;

/* Miscellaneous Service */
#define SC_MISC_GET_CONTROL _IOWR('s', 1, sc_misc_ctl_t *)
#define SC_MISC_SET_CONTROL _IOWR('s', 2, sc_misc_ctl_t *)
#define SC_MISC_BUILD_INFO _IOR('s', 3, sc_misc_build_info_t *)

/* Power management service */
#define SC_PM_GET_POWER_MODE _IOR('s', 4, sc_pm_res_t *)
#define SC_PM_SET_POWER_MODE _IOWR('s', 5, sc_pm_res_t *)

#endif
