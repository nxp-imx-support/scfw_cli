/*
 * Copyright (c) 2018 NXP
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
#include "msc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* System Controller manager definitions */
#include <sys/imx_sci_mgr.h>
/* Err code declarations for devctl */
#include <errno.h>
/* delay function declaration */
#include <fcntl.h>
/* Include needed for sc_rsrc2str(), sc_status2str() functions only */
#include <sys/sci.h>
/* Common helper functions */
#include "../common/common.h"

typedef enum misc_opt_e{
	get_ctl = 1,				/* Get Control */
	set_ctl,					/* Set Control */
	get_temp,					/* Get temperature */
	set_temp,					/* Set temperature */
	bld_inf,					/* Build info */
	inv
}misc_opt_t;

/* Prototypes */
static void misc_get_ctl(int fd, char *param);
static void misc_set_ctl(int fd, char *param);
static void misc_get_temp(int fd, char *param);
static void misc_set_temp(int fd, char *param);
static void misc_bld_inf(int fd);

void misc_service_main(int fd, char *argv[]){
	misc_opt_t misc_opt = inv;

	/* Process arguments */
	if(argv[0] == NULL){
		int svc;
		argv[1] = NULL;				/* If no service option was selected the parameters are invalid */
		printf("Select option: \n");
		printf("1.- Get control \n");
		printf("2.- Set control \n");
		printf("3.- Get temperature \n");
		printf("4.- Set temperature \n");
		printf("5.- Get build info \n");

		scanf("%d", &svc);
		if((svc >= inv) | (svc < get_ctl)){
			misc_opt = inv;
			printf("Please select a valid option\n");
		} else{
			misc_opt = svc;
		}
	} else {
		/* Check if a valid service option was passed */
		if(strncmp(argv[0], "get_control", strlen("get_control")) == 0){
			misc_opt = get_ctl;
		} else if(strncmp(argv[0], "set_control", strlen("set_control")) == 0){
			misc_opt = set_ctl;
		} else if(strncmp(argv[0], "get_temp", strlen("get_temp")) == 0){
			misc_opt = get_temp;
		} else if(strncmp(argv[0], "set_temp", strlen("set_temp")) == 0){
			misc_opt = set_temp;
		} else if(strncmp(argv[0], "build_info", strlen("build_info")) == 0){
			misc_opt = bld_inf;
		} else{
			misc_opt = inv;
			printf("MISC: %s is an invalid service option\n", argv[0]);
		}
	}

	/* Call service option */
	switch(misc_opt){
			case get_ctl:
				misc_get_ctl(fd, argv[1]);
				break;
			case set_ctl:
				misc_set_ctl(fd, argv[1]);
				break;
			case get_temp:
				misc_get_temp(fd, argv[1]);
				break;
			case set_temp:
				misc_set_temp(fd, argv[1]);
				break;
			case bld_inf:
				misc_bld_inf(fd);
				break;
			default:
				printf("MISC: invalid service option\n");
				break;
		}
}

void misc_get_ctl(int fd, char *param){

	imx_dcmd_sc_misc_control_t ctl;

	/* Process arguments */
	if(param == NULL){
		int ret, valid = 0;
		do{
			printf("Enter resource number: \n");
			ret = scanf("%d", &ctl.resource);
			printf("Enter control number: \n");
			scanf("%d", &ctl.ctrl);
			//TODO print control options or implement function to parse string
			if((ret == 1) && (ctl.resource <= SC_R_LAST)){
				valid = 1;
			} else{
				printf("Invalid option\n");
				valid = 0;
			}
		} while(valid == 0);
	} else{
		/* Parse parameters */
		int tmp[2];
		int err;
		err = parse_param(param, tmp, 2);
		if(err == 2){
			ctl.resource = tmp[0];
			ctl.ctrl = tmp[1];
		} else {
			printf("MISC:GET_CTL:Invalid -param=\n");
			return;
		}
	}

	/* Keep resources within limits */
	if(ctl.resource >= SC_R_LAST){
		ctl.resource = SC_R_LAST - 1;
	}

	/* Get control */
	printf("Resource, Control, Value\n");
	sc_err_t status;
	int err, cnt = 0;

	/* Get resource power mode */
	do {
		err = devctl(fd, IMX_DCMD_SC_MISC_GET_CONTROL, &ctl, sizeof(imx_dcmd_sc_misc_control_t), (int *) &status);
	} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
	if(status == SC_ERR_NONE){
		printf("%s, %d, %d\n", sc_rsrc2str(ctl.resource), ctl.ctrl, ctl.val);
	} else {
		printf("ERR:%s,%s\n", sc_rsrc2str(ctl.resource), sc_status2str(status));
	}
}

void misc_set_ctl(int fd, char *param){
	//TODO
}

void misc_get_temp(int fd, char *param){
	//TODO
}

void misc_set_temp(int fd, char *param){
	//TODO
}

void misc_bld_inf(int fd){
	imx_dcmd_sc_misc_build_info_t build_info;
	sc_err_t status;
	int err, cnt = 0;

	/* Get build info */
	do{
		err = devctl(fd, IMX_DCMD_SC_MISC_BUILD_INFO, &build_info, sizeof(imx_dcmd_sc_misc_build_info_t), (int *) &status);
	} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
	if(status == SC_ERR_NONE){
		printf("SCFW version: %d, commit: %x\n", build_info.build, build_info.commit);
	} else{
		printf("Failed to get build info, %s (%d)\n", sc_status2str(status), status);
	}
}
