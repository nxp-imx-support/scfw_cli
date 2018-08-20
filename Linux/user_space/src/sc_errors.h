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
#ifndef SC_ERRORS_H
#define SC_ERRORS_H
const char *sc_errors_str[] = {
	"SC_ERR_NONE",
	"SC_ERR_VERSION",
	"SC_ERR_CONFIG",
	"SC_ERR_PARM",
	"SC_ERR_NOACCESS",
	"SC_ERR_LOCKED",
	"SC_ERR_UNAVAILABLE",
	"SC_ERR_NOTFOUND",
	"SC_ERR_NOPOWER",
	"SC_ERR_IPC",
	"SC_ERR_BUSY",
	"SC_ERR_FAIL",
	"SC_ERR_LAST"
};
#define sc_errors_str_len  (sizeof(sc_errors_str)/sizeof(sc_errors_str[0]))

#endif
