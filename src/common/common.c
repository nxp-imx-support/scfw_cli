/*
 * common.c
 *
 *  Created on: Apr 12, 2018
 *      Author: Manuel Rodriguez
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Parses a string of N parameters delimited by _
 *
 * Return number of parameters parsed
 * */
int parse_param(char *param, int param_array[], int param_length){
	char *tmp;
	char *tok = {"_"};
	int i = 0;

	tmp = strtok(param, tok);
	while((tmp != NULL) && (i < param_length)){
		param_array[i] = atoi(tmp);
		++i;
		tmp = strtok(NULL, tok);
	}

	return i;
}
