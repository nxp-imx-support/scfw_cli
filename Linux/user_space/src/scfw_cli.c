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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
 
/* IOCTL definitions */
#include "../../kernel_module/sc.h"

/* Helper functions */
#include "common.h"


void ioctl_misc_get_control(int fd, char *param){
	sc_misc_ctl_t misc_ctl;
	int param_array[2];
	
	/* Parse parameters and populate structure */
	parse_parameters(param, param_array, 2);
	misc_ctl.resource = param_array[0];
	misc_ctl.ctl = param_array[1];

	/* Make IOCTL call*/
	if(ioctl(fd, SC_MISC_GET_CONTROL, &misc_ctl) == -1){
		perror("IOCTL_MISC_GET_CONTROL FAILED\n");
	} else {
		if(misc_ctl.err == 0){
			printf("%s, %s, %d\n", sc_rsrc2str(misc_ctl.resource), sc_ctl2str(misc_ctl.ctl), misc_ctl.val);
		} else {
			printf("ERR:%s, %s\n", sc_rsrc2str(misc_ctl.resource), sc_error2str(misc_ctl.err));
		}	
	} 
}

void ioctl_misc_set_control(int fd, char *param){
	sc_misc_ctl_t misc_ctl;
	int param_array[3];
	
	/* Parse parameters and populate structure */
	parse_parameters(param, param_array, 3);
	misc_ctl.resource = param_array[0];
	misc_ctl.ctl = param_array[1];
	misc_ctl.val = param_array[2];

	/* Make IOCTL call*/
	if(ioctl(fd, SC_MISC_SET_CONTROL, &misc_ctl) == -1){
		perror("IOCTL_MISC_SET_CONTROL FAILED\n");
	} else {
		if(misc_ctl.err == 0){
			printf("%s, %s, %d\n", sc_rsrc2str(misc_ctl.resource), sc_ctl2str(misc_ctl.ctl), misc_ctl.val);
		} else {
			printf("ERR:%s, %s\n", sc_rsrc2str(misc_ctl.resource), sc_error2str(misc_ctl.err));
		}	
	}
}

void ioctl_misc_build_info(int fd){
	sc_misc_build_info_t build_info;

	/* Make IOCTL call*/
	if(ioctl(fd, SC_MISC_BUILD_INFO, &build_info) == -1){
		perror("IOCTL_MISC_BUILD_INFO FAILED\n");
	} else {
		printf("%d, %d\n", build_info.build, build_info.commit);	
	}
}

void ioctl_pm_get_power_mode(int fd, char *param){
	sc_pm_res_t pm;
	int param_array[1];
	
	/* Parse parameters and populate structure */
	parse_parameters(param, param_array, 1);
	pm.resource = param_array[0];

	/* Make IOCTL call*/
	if(ioctl(fd, SC_PM_GET_POWER_MODE, &pm) == -1){
		perror("IOCTL_PM_GET_POWER_MODE FAILED\n");
	} else {
		if(pm.err == 0){
			printf("%s, %s\n", sc_rsrc2str(pm.resource), sc_pw2str(pm.mode));
		} else {
			printf("ERR:%s, %s\n", sc_rsrc2str(pm.resource), sc_error2str(pm.err));
		}	
	}
}

void ioctl_pm_set_power_mode(int fd, char *param){
	sc_pm_res_t pm;
	int param_array[2];
	
	/* Parse parameters and populate structure */
	parse_parameters(param, param_array, 2);
	pm.resource = param_array[0];
	pm.mode = param_array[1];

	/* Make IOCTL call*/
	if(ioctl(fd, SC_PM_SET_POWER_MODE, &pm) == -1){
		perror("IOCTL_PM_SET_POWER_MODE FAILED\n");
	} else {
		if(pm.err == 0){
			printf("%s, %s\n", sc_rsrc2str(pm.resource), sc_pw2str(pm.mode));
		} else {
			printf("ERR:%s, %s\n", sc_rsrc2str(pm.resource), sc_error2str(pm.err));
		}	
	}
}

void print_usage(){

	fprintf(stderr, "Usage: scfw_cli -option=param1,param2,...,param3\n");
	fprintf(stderr, "Examples:\n");
	fprintf(stderr, "-misc_get_control=SC_R_X,SC_C_X\n");
	fprintf(stderr, "-misc_set_control=SC_R_X,SC_C_X,val\n");
	fprintf(stderr, "-misc_build_info\n");
	fprintf(stderr, "-pm_get_power_mode=SC_R_X\n");
	fprintf(stderr, "-pm_set_power_mode=SC_R_X,SC_PM_PW_MODE_X\n");
	exit(EXIT_FAILURE);
}
 
int main(int argc, char *argv[]){

	/* System Controller device */
    	char *file_name = "/dev/sc";
    	int fd;
    	char *param_str;

	/* Available options */
	enum{
	misc_get_control,
	misc_set_control,
	misc_build_info,
	pm_get_power_mode,
	pm_set_power_mode
	} option;
	
    	/* Check argument variables */
    	if (argc == 1){
		print_usage();
    	} else if (argc = 2){
        	if (strncmp(argv[1], "-misc_get_control=", strlen("-misc_get_control=")) == 0){
            		param_str = argv[1] + strlen("-misc_get_control=");
            		option = misc_get_control;
        	} else if (strncmp(argv[1], "-misc_set_control=", strlen("-misc_set_control=")) == 0){
             		param_str = argv[1] + strlen("-misc_set_control=");           		
			option = misc_set_control;
        	} else if (strncmp(argv[1], "-misc_build_info", strlen("-misc_build_info")) == 0){
            		option = misc_build_info;
        	} else if (strncmp(argv[1], "-pm_get_power_mode=", strlen("-pm_get_power_mode=")) == 0){
            		param_str = argv[1] + strlen("-pm_get_power_mode=");
            		option = pm_get_power_mode;
        	} else if (strncmp(argv[1], "-pm_set_power_mode=", strlen("-pm_set_power_mode=")) == 0){
            		param_str = argv[1] + strlen("-pm_set_power_mode=");
            		option = pm_set_power_mode;
        	} else{
            		print_usage();
        	}
    	} else {
        	print_usage();
    	}

    	/* Open System Controller device */
    	fd = open(file_name, O_RDWR);
    	if (fd == -1) {
        	perror("Unable to open dev/sc");
        	return 2;
    	}
 
    	/* Select option */
    	switch (option)
    	{
        	case misc_get_control:
            		ioctl_misc_get_control(fd, param_str);
            		break;
        	case misc_set_control:
            		ioctl_misc_set_control(fd, param_str);
            		break;
		case misc_build_info:
            		ioctl_misc_build_info(fd);
            		break;;
		case pm_get_power_mode:
            		ioctl_pm_get_power_mode(fd, param_str);
            		break;
		case pm_set_power_mode:
            		ioctl_pm_set_power_mode(fd, param_str);
            		break;
        	default:
        	    	break;
    	}
 
    	/* Close System controller device */
    	close(fd);
    	return 0;
}
