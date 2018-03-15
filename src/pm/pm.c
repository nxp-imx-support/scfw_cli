/*
 * pm.c
 *
 *  Created on: Feb 15, 2018
 *      Author: Manuel Rodriguez
 */
#include "pm.h"
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

static const char * sc_power_mode_str[] = {
    "SC_PM_PW_MODE_OFF",
    "SC_PM_PW_MODE_STBY",
	"SC_PM_PW_MODE_LP",
	"SC_PM_PW_MODE_ON"
};

typedef enum pm_opt_e{
	get_pw = 1,						/* Get Resource Power Mode option*/
	set_pw,						/* Set Resource Power Mode option */
	inv
}pm_opt_t;

static void pm_get_power_mode(int fd, char *param);
static void pm_set_power_mode(int fd, char *param);

/* Helper functions */
static const char* sc_pw2str(sc_pm_power_mode_t pw);
static int parse_param(char *param, int param_array[], int param_length);

void pm_service_main(int fd, char *argv[]){
	pm_opt_t pm_opt = inv;

	/* Process arguments */
	if(argv[0] == NULL){
		int svc;
		argv[1] = NULL;				/* If no service option was selected the parameters are invalid */
		printf("Select option: \n");
		printf("1.- Get resources power mode \n");
		printf("2.- Set resource power mode \n");

		scanf("%d", &svc);
		switch(svc){
			case get_pw:
				pm_opt = get_pw;
				break;
			case set_pw:
				pm_opt = set_pw;
				break;
			default:
				printf("Please select a valid option\n");
				break;
		}
	} else {
		/* Check if a valid service option was passed */
		if (argv[0] != NULL){
			if(strncmp(argv[0], "get-rsrc-pw", strlen("get-rsrc-pw")) == 0){
				pm_opt = get_pw;
			} else if(strncmp(argv[0], "set-rsrc-pw", strlen("set-rsrc-pw")) == 0){
				pm_opt = set_pw;
			} else{
				pm_opt = inv;
				printf("PM: %s is an invalid service option\n", argv[0]);
			}
		}
	}

	/* Call service option */
	switch(pm_opt){
			case get_pw:
				pm_get_power_mode(fd, argv[1]);
				break;
			case set_pw:
				pm_set_power_mode(fd, argv[1]);
				break;
			default:
				printf("PM: invalid service option\n");
				break;
		}
}

void pm_get_power_mode(int fd, char *param){

	int r_first = 0, r_last = 0;

	/* Process arguments */
	if(param == NULL){
		int ret, valid = 0;
		do{
			printf("Enter resource numbers in the following format: \n");
			printf("r_first r_last For instance 1 4 to get resources 1 through 4\n");
			printf("if only one resource is required call rsrc rsrc e.g. 1 1 for resource no 1\n");
			printf("or enter 0 %d for all resources\n", SC_R_LAST);
			ret = scanf("%d %d", &r_first, &r_last);
			if(ret == 2){
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
			r_first = tmp[0];
			r_last = tmp[1];
		} else {
			printf("PM:GET_PW:Invalid -param=\n");
			return;
		}
	}

	/* Keep resources within limits */
	if(r_last >= SC_R_LAST){
		r_last = SC_R_LAST - 1;
	}
	if(r_first >= SC_R_LAST){
		r_first = SC_R_LAST - 1;
	}
	/* Get resources power mode */
	printf("Resource,Power mode\n");
	imx_dcmd_sc_pm_res_t res_pm;
	sc_err_t status;
	int err, cnt = 0;
	for(; r_first <= r_last; r_first++){
		res_pm.resource = r_first;

		/* Get resource power mode */
		do {
			err = devctl(fd, IMX_DCMD_SC_PM_GET_RESOURCE_POWER_MODE, &res_pm, sizeof(imx_dcmd_sc_pm_res_t), (int *) &status);
		} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
		if(status == SC_ERR_NONE){
			printf("%s,%s\n", sc_rsrc2str(r_first), sc_pw2str(res_pm.mode));
		} else {
			printf("ERR:%s,%s\n", sc_rsrc2str(r_first), sc_status2str(status));
		}
	}
}

void pm_set_power_mode(int fd, char *param){

	int r_first = 0, r_last = 0;
	sc_pm_power_mode_t power_mode = SC_PM_PW_MODE_ON;			/* On is usually less harmless that turning things off*/

	/* Process arguments */
	if(param == NULL){
		int ret, valid = 0;
		do {
			printf("Enter resource numbers in the following format: \n");
			printf("r_first r_last For instance 1 4 to set resources 1 through 4 \n");
			printf("if only one resource is required call rsrc rsrc e.g. 1 1 to set resource no 1 \n");
			printf("or enter 0 %d to set all resources\n", SC_R_LAST);
			ret = scanf("%d %d", &r_first, &r_last);
			if(ret == 2){
				/* Keep resources within limits */
				int r_first_tmp = r_first; /* Workaround for bug where scanf overwrites r_first while reading power_mode*/
				printf("Enter power mode to be set, options:\n");
				printf("0 - SC_PM_PW_MODE_OFF\n");
				printf("1 - SC_PM_PW_MODE_STBY\n");
				printf("2 - SC_PM_PW_MODE_LP\n");
				printf("3 - SC_PM_PW_MODE_ON\n");
				scanf("%d", &power_mode);
				r_first = r_first_tmp; /* Workaround for bug where scanf overwrites r_first while reading power_mode*/
				printf("Setting power mode %s (%d) for %s (%d) through %s (%d) \n",sc_pw2str(power_mode), power_mode, sc_rsrc2str(r_first), r_first, sc_rsrc2str(r_last), r_last);
				valid = 1;
			} else{
				printf("Invalid option\n");
				valid = 0;
			}
		} while (valid == 0);
	} else{
		/* Parse parameters */
		int tmp[3];
		int err;
		err = parse_param(param, tmp, 3);
		if(err == 3){
			r_first = tmp[0];
			r_last = tmp[1];
			power_mode = tmp[2];
		} else {
			printf("PM:GET_PW:Invalid -param=\n");
			return;
		}
	}

	/* Check parameters */
	if(power_mode > SC_PM_PW_MODE_ON){
		printf("Invalid power mode setting to SC_PM_PW_MODE_ON\n");
		power_mode = SC_PM_PW_MODE_ON;
	}
	if(r_last >= SC_R_LAST){
		r_last = SC_R_LAST - 1;
	}
	if(r_first >= SC_R_LAST){
		r_first = SC_R_LAST - 1;
	}

	/* Set resources power mode */
	imx_dcmd_sc_pm_res_t res_pm;
	sc_err_t status;
	res_pm.mode = power_mode;
	int err, cnt = 0;
	for(; r_first <= r_last; r_first++){
		res_pm.resource = r_first;

		/* Set resource power mode */
		do {
			err = devctl(fd, IMX_DCMD_SC_PM_SET_RESOURCE_POWER_MODE, &res_pm, sizeof(imx_dcmd_sc_pm_res_t), (int *) &status);
		} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
		if(status == SC_ERR_NONE){
			printf("%s (%d) power mode set to %s (%d)\n", sc_rsrc2str(r_first), r_first, sc_pw2str(res_pm.mode), res_pm.mode);
		} else {
			printf("Failed to set %s (%d) power mode %s, err: %s (%d)\n", sc_rsrc2str(r_first), r_first, sc_pw2str(res_pm.mode), sc_status2str(status), status);
		}
	}
}

static const char* sc_pw2str(sc_pm_power_mode_t pw){
	if(pw <= SC_PM_PW_MODE_ON){
		return sc_power_mode_str[pw];
	} else {
		return "INVALID POWER MODE";
	}
}

/* Parses a string of N parameters delimited by _
 *
 * Return number of parameters parsed
 * */
static int parse_param(char *param, int param_array[], int param_length){
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
