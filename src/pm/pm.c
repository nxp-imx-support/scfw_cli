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
/* Common helper functions */
#include "../common/common.h"

static const char * sc_power_mode_str[] = {
    "SC_PM_PW_MODE_OFF",
    "SC_PM_PW_MODE_STBY",
	"SC_PM_PW_MODE_LP",
	"SC_PM_PW_MODE_ON"
};

typedef enum pm_opt_e{
	get_pw = 1,						/* Get Resource Power Mode option*/
	set_pw,							/* Set Resource Power Mode option */
	get_clk_rate,					/* Get clock rate */
	set_clk_rate,					/* Set clock rate */
	inv
}pm_opt_t;

/* Defines */
#define LAST_CLK	4

static void pm_get_power_mode(int fd, char *param);
static void pm_set_power_mode(int fd, char *param);
static void pm_get_clk_rate(int fd, char *param);
static void pm_set_clk_rate(int fd, char *param);

/* Helper functions */
static const char* sc_pw2str(sc_pm_power_mode_t pw);

void pm_service_main(int fd, char *argv[]){
	pm_opt_t pm_opt = inv;

	/* Process arguments */
	if(argv[0] == NULL){
		int svc;
		argv[1] = NULL;				/* If no service option was selected the parameters are invalid */
		printf("Select option: \n");
		printf("1.- Get resources power mode \n");
		printf("2.- Set resource power mode \n");
		printf("3.- Get clock rate\n");
		printf("4.- Set clock rate\n");

		scanf("%d", &svc);
		if((svc >= inv) | (svc < get_pw)){
			pm_opt = inv;
			printf("Please select a valid option\n");
		} else{
			pm_opt = svc;
		}
	} else {
		/* Check if a valid service option was passed */
		if(strncmp(argv[0], "get_resource_power_mode", strlen("get_resource_power_mode")) == 0){
			pm_opt = get_pw;
		} else if(strncmp(argv[0], "set_resource_power_mode", strlen("set_resource_power_mode")) == 0){
			pm_opt = set_pw;
		} else if(strncmp(argv[0], "get_clock_rate", strlen("get_clock_rate")) == 0){
			pm_opt = get_clk_rate;
		} else if(strncmp(argv[0], "set_clock_rate", strlen("set_clock_rate")) == 0){
			pm_opt = set_clk_rate;
		} else{
			pm_opt = inv;
			printf("PM: %s is an invalid service option\n", argv[0]);
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
			case get_clk_rate:
				pm_get_clk_rate(fd, argv[1]);
				break;
			case set_clk_rate:
				pm_set_clk_rate(fd, argv[1]);
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
	sc_pm_power_mode_t power_mode = SC_PM_PW_MODE_ON;			/* On is usually less harmful that turning things off*/

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


void pm_get_clk_rate(int fd, char *param){

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
			printf("PM:SET_CLK_RATE:Invalid -param=\n");
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

	/* Get resources clock rate */
	printf("Resource, Clock, Clock rate\n");
	imx_dcmd_sc_pm_clock_rate_t clk_rate;
	sc_err_t status;
	int err, cnt = 0;
	for(; r_first <= r_last; r_first++){
		clk_rate.resource = r_first;

		for(clk_rate.clk = 0; clk_rate.clk <= LAST_CLK; clk_rate.clk++){
			/* Get resource clock rate for all clocks */
			do {
				err = devctl(fd, IMX_DCMD_SC_PM_GET_CLOCK_RATE, &clk_rate, sizeof(imx_dcmd_sc_pm_clock_rate_t), (int *) &status);
			} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
			if(status == SC_ERR_NONE){
				printf("%s,clock:%d, rate:%d\n", sc_rsrc2str(r_first), clk_rate.clk, clk_rate.rate);
			} else {
				printf("ERR:%s,%s\n", sc_rsrc2str(r_first), sc_status2str(status));
			}
		}
	}
}

void pm_set_clk_rate(int fd, char *param){

	imx_dcmd_sc_pm_clock_rate_t clk_rate;

	/* Process arguments */
	if(param == NULL){
		int ret, valid = 0;
		do {
			printf("Enter resource number: \n");
			ret = scanf("%d", &clk_rate.resource);
			if(ret == 1){
				/* Keep resources within limits */
				int tmp = clk_rate.resource; /* Workaround for scanf bug */
				printf("Enter clock type to be set, options:\n");
				printf("0 - SC_PM_CLK_SLV_BUS\n");
				printf("1 - SC_PM_CLK_MST_BUS\n");
				printf("2 - SC_PM_CLK_PER\n");
				printf("3 - SC_PM_CLK_PHY\n");
				printf("4 - SC_PM_CLK_MISC\n");
				printf("0 - SC_PM_CLK_MISC0\n");
				printf("1 - SC_PM_CLK_MISC1\n");
				printf("2 - SC_PM_CLK_MISC2\n");
				printf("3 - SC_PM_CLK_MISC3\n");
				printf("4 - SC_PM_CLK_MISC4\n");
				printf("2 - SC_PM_CLK_CPU\n");
				printf("4 - SC_PM_CLK_PLL\n");
				printf("4 - SC_PM_CLK_BYPASS\n");
				scanf("%d", &clk_rate.clk);

				printf("Enter clock rate in Hz:\n");
				scanf("%u", &clk_rate.rate);

				clk_rate.resource = tmp; /* Workaround for bug where scanf overwrites resource while reading clock rate*/
				printf("Setting resource %s clock %d  to: %u\n",sc_rsrc2str(clk_rate.resource), clk_rate.clk, clk_rate.rate);
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
			clk_rate.resource = tmp[0];
			clk_rate.clk = tmp[1];
			clk_rate.rate = tmp[2];
		} else {
			printf("PM:SET_CLK_RATE:Invalid -param=\n");
			return;
		}
	}

	/* Check parameters */
	if(clk_rate.resource > SC_R_LAST){
		printf("Invalid resource\n");
		return;
	}
	if(clk_rate.clk > LAST_CLK){
		printf("Invalid clock type\n");
		return;
	}

	/* Set resources power mode */
	sc_err_t status;
	int err, cnt = 0;

	/* Set resource clock rate */
	do {
		err = devctl(fd, IMX_DCMD_SC_PM_SET_CLOCK_RATE, &clk_rate, sizeof(imx_dcmd_sc_pm_clock_rate_t), (int *) &status);
	} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
	if(status == SC_ERR_NONE){
		printf("%s clock %d rate set to: %u\n", sc_rsrc2str(clk_rate.resource), clk_rate.clk, clk_rate.rate);
	} else {
		printf("Failed to set %s clock %d rate to: %u, err: %s (%d)\n", sc_rsrc2str(clk_rate.resource), clk_rate.clk, clk_rate.rate, sc_status2str(status), status);
	}
}

/* Returns a string from a power mode type
 *
 * */
static const char* sc_pw2str(sc_pm_power_mode_t pw){
	if(pw <= SC_PM_PW_MODE_ON){
		return sc_power_mode_str[pw];
	} else {
		return "INVALID POWER MODE";
	}
}
