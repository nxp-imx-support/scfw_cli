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
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include <assert.h>

/* String definitions */
#include "resources.h"
#include "controls.h"
#include "power_modes.h"
#include "sc_errors.h"

/* Searches a string in a string array and returns the index
 *
 * Returns the index where the string was found, if no string matches returns -1
 * */
int str2index(char *str, const char *str_array[], int str_len){
	int index = 0, match = -1;

	while((index < str_len) && (match != 0)){
		match = strcmp(str_array[index], str);
		/* If the string did not match continue with the next one */
		if(match != 0){
			index++;
		}
	}

	/* If the index reached the end of the array
	 * the string was not found
	 * */
	if(index == str_len){
		return -1;
	} else{
		return index;
	}
}

const char * sc_ctl2str(int ctl){
	if (ctl < sc_ctl_str_len) {
		return sc_ctl_str[ctl];
	}
	return "Invalid control";
}

const char * sc_rsrc2str(int resource){
	if (resource < sc_rsrc_str_len) {
		return sc_rsrc_str[resource];
	}
	return "Invalid resource";
}

const char * sc_pw2str(unsigned char pw){
        if(pw < sc_pw_str_len){
		return sc_pw_str[pw];
        }
        return "Invalid power mode";
}

const char * sc_error2str(int err){
        if(err < sc_errors_str_len){
		return sc_errors_str[err];
        }
        return "Invalid SC error";
}

int sc_str2rsrc(char * rsrc){
	return str2index(rsrc, sc_rsrc_str, sc_rsrc_str_len);
}

int sc_str2ctl(char * ctl){
	return str2index(ctl, sc_ctl_str, sc_ctl_str_len);
}

int sc_str2pw(char * pw){
	return str2index(pw, sc_pw_str, sc_pw_str_len);
}

/* Split a string of N parameters delimited by ,
 *
 * Return a list of strings
 * */
char** str_split(char* a_str){
	const char a_delim = ',';
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp){
        if (a_delim == *tmp){
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result){
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token){
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

void parse_parameters(char *param, int param_array[], int param_length){
	char **split_strings;

	/* Split strings */
	split_strings = str_split(param);

	/* Parse strings and fill parameters array  */
	if(split_strings){
		int param_cnt = 0, i = 0;
		for(i = 0; *(split_strings + i); i++){
			if(param_cnt < param_length){
				char *tmp_str = *(split_strings + i);
				if(strncmp(tmp_str, "SC_R", strlen("SC_R")) == 0){
					param_array[param_cnt++] = sc_str2rsrc(tmp_str);
				} else if(strncmp(tmp_str, "SC_C", strlen("SC_C")) == 0){
					param_array[param_cnt++] = sc_str2ctl(tmp_str);
				} else if(strncmp(tmp_str, "SC_PM_PW", strlen("SC_PM_PW")) == 0){
					param_array[param_cnt++] = sc_str2pw(tmp_str);
				} else{
					/* Check if is a digit */
					int number;
					char *endptr;
					number = strtol(tmp_str, &endptr, 10);
					if(tmp_str != endptr){
						param_array[param_cnt++] = number;
					}
				}
			}
			free(*(split_strings + i));
		}
	}
	free(split_strings);
}
