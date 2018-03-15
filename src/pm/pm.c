/*
 * pm.c
 *
 *  Created on: Feb 15, 2018
 *      Author: manuel
 */
#include "pm.h"
#include <stdio.h>

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

static void pm_get_power_mode(int fd, int argc, char *argv[]);
static void pm_set_power_mode(int fd, int argc, char *argv[]);
static const char* sc_pw2str(sc_pm_power_mode_t pw);

void pm_service_main(int fd, int argc, char *argv[]){

	/* Process arguments */
	if(argc == 0){
		int svc;
		printf("Select option: \n");
		printf("1.- Get resources power mode \n");
		printf("2.- Set resource power mode \n");

		scanf("%d", &svc);
		switch(svc){
			case 1:
				pm_get_power_mode(fd, 0, NULL);
				break;
			case 2:
				pm_set_power_mode(fd, 0, NULL);
				break;
			default:
				printf("Please select a valid option\n");
				break;
		}
	} else{
		/* TODO implement argument friendly set-up*/
		return;
	}
}

void pm_get_power_mode(int fd, int argc, char *argv[]){

	int r_first, r_last;

	/* Process arguments */
	if(argc == 0){
		int ret, valid = 0;
		do{
			printf("Enter resource numbers in the following format: \n");
			printf("r_first r_last For instance 1 4 to get resources 1 through 4\n");
			printf("if only one resource is required call rsrc rsrc e.g. 1 1 for resource no 1\n");
			printf("or enter 0 %d for all resources\n", SC_R_LAST);
			ret = scanf("%d %d", &r_first, &r_last);
			if(ret == 2){
				/* Keep resources within limits */
				if(r_last >= SC_R_LAST){
					r_last = SC_R_LAST - 1;
				}
				printf("Getting power mode for %s (%d) through %s (%d) \n", sc_rsrc2str(r_first), r_first, sc_rsrc2str(r_last), r_last);
				valid = 1;
			} else{
				printf("Invalid option\n");
				valid = 0;
			}
		} while(valid == 0);
	} else{
		/* TODO implement argument friendly set-up*/
		return;
	}

	/* Get resources power mode */
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
			printf("%s (%d) power mode: %s (%d)\n", sc_rsrc2str(r_first), r_first, sc_pw2str(res_pm.mode), res_pm.mode);
		} else {
			printf("Failed to get %s (%d) power mode, err: %s (%d)\n",sc_rsrc2str(r_first), r_first, sc_status2str(status), status);
		}
	}
}

void pm_set_power_mode(int fd, int argc, char *argv[]){

	int r_first, r_last;
	sc_pm_power_mode_t power_mode;

	/* Process arguments */
	if(argc == 0){
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
				if(r_last >= SC_R_LAST){
					r_last = SC_R_LAST - 1;
				}

				printf("Enter power mode to be set, options:\n");
				printf("0 - SC_PM_PW_MODE_OFF\n");
				printf("1 - SC_PM_PW_MODE_STBY\n");
				printf("2 - SC_PM_PW_MODE_LP\n");
				printf("3 - SC_PM_PW_MODE_ON\n");
				scanf("%d", &power_mode);
				r_first = r_first_tmp; /* Workaround for bug where scanf overwrites r_first while reading power_mode*/
				/* Check power mode */
				if(power_mode > SC_PM_PW_MODE_ON){
					printf("Invalid power mode setting to SC_PM_PW_MODE_ON\n");
					power_mode = SC_PM_PW_MODE_ON;
				}
				printf("Setting power mode %s (%d) for %s (%d) through %s (%d) \n",sc_pw2str(power_mode), power_mode, sc_rsrc2str(r_first), r_first, sc_rsrc2str(r_last), r_last);
				valid = 1;
			} else{
				printf("Invalid option\n");
				valid = 0;
			}
		} while (valid == 0);
	} else{
		/* TODO implement argument friendly set-up*/
		return;
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
