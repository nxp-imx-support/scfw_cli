/*
 * msc.c
 *
 *  Created on: Apr 11, 2018
 *      Author: Manuel Rodriguez
 */
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

typedef enum msc_opt_e{
	get_ctl = 1,				/* Get Control */
	set_ctl,					/* Set Control */
	get_temp,					/* Get temperature */
	set_temp,					/* Set temperature */
	bld_inf,					/* Build info */
	inv
}msc_opt_t;

/* Prototypes */
static void msc_get_ctl(int fd, char *param);
static void msc_set_ctl(int fd, char *param);
static void msc_get_temp(int fd, char *param);
static void msc_set_temp(int fd, char *param);
static void msc_bld_inf(int fd);

void msc_service_main(int fd, char *argv[]){
	msc_opt_t msc_opt = inv;

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
			msc_opt = inv;
			printf("Please select a valid option\n");
		} else{
			msc_opt = svc;
		}
	} else {
		/* Check if a valid service option was passed */
		if(strncmp(argv[0], "get-ctl", strlen("get-ctl")) == 0){
			msc_opt = get_ctl;
		} else if(strncmp(argv[0], "set-ctl", strlen("set-ctl")) == 0){
			msc_opt = set_ctl;
		} else if(strncmp(argv[0], "get-temp", strlen("get-temp")) == 0){
			msc_opt = get_temp;
		} else if(strncmp(argv[0], "set-temp", strlen("set-temp")) == 0){
			msc_opt = set_temp;
		} else if(strncmp(argv[0], "bld-inf", strlen("bld-inf")) == 0){
			msc_opt = bld_inf;
		} else{
			msc_opt = inv;
			printf("MSC: %s is an invalid service option\n", argv[0]);
		}
	}

	/* Call service option */
	switch(msc_opt){
			case get_ctl:
				msc_get_ctl(fd, argv[1]);
				break;
			case set_ctl:
				msc_set_ctl(fd, argv[1]);
				break;
			case get_temp:
				msc_get_temp(fd, argv[1]);
				break;
			case set_temp:
				msc_set_temp(fd, argv[1]);
				break;
			case bld_inf:
				msc_bld_inf(fd);
				break;
			default:
				printf("MSC: invalid service option\n");
				break;
		}
}

void msc_get_ctl(int fd, char *param){

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
			printf("MSC:GET_CTL:Invalid -param=\n");
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

void msc_set_ctl(int fd, char *param){
	//TODO
}

void msc_get_temp(int fd, char *param){
	//TODO
}

void msc_set_temp(int fd, char *param){
	//TODO
}

void msc_bld_inf(int fd){
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
