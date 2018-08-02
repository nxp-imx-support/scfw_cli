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

/* String list of Power modes */
static const char * sc_power_mode_str[] = {
    "SC_PM_PW_MODE_OFF",
    "SC_PM_PW_MODE_STBY",
	"SC_PM_PW_MODE_LP",
	"SC_PM_PW_MODE_ON"
};
#define sc_power_mode_str_len  (sizeof(sc_power_mode_str)/sizeof(sc_power_mode_str[0]))

/* String list of clock types */
static const char *sc_clock_str[] = {
	"SC_PM_CLK_SLV_BUS",
	"SC_PM_CLK_MST_BUS",
	"SC_PM_CLK_PER",
	"SC_PM_CLK_PHY",
	"SC_PM_CLK_MISC",
	"SC_PM_CLK_MISC0",
	"SC_PM_CLK_MISC1",
	"SC_PM_CLK_MISC2",
	"SC_PM_CLK_MISC3",
	"SC_PM_CLK_MISC4",
	"SC_PM_CLK_CPU",
	"SC_PM_CLK_PLL",
	"SC_PM_CLK_BYPASS"
};
#define sc_clock_str_len  (sizeof(sc_clock_str)/sizeof(sc_clock_str[0]))

/* List of values for clock types */
static const char sc_clock_val[] = {
	0U,      /*!< Slave bus clock */
	1U,      /*!< Master bus clock */
	2U,      /*!< Peripheral clock */
	3U,      /*!< Phy clock */
	4U,      /*!< Misc clock */
	0U,      /*!< Misc 0 clock */
	1U,      /*!< Misc 1 clock */
	2U,      /*!< Misc 2 clock */
	3U,      /*!< Misc 3 clock */
	4U,      /*!< Misc 4 clock */
	2U,      /*!< CPU clock */
	4U,      /*!< PLL */
	4U       /*!< Bypass clock */
};

/* Defines */
#define LAST_CLK	4

typedef enum pm_opt_e{
	get_pw = 1,						/* Get Resource Power Mode option*/
	set_pw,							/* Set Resource Power Mode option */
	get_clk_rate,					/* Get clock rate */
	set_clk_rate,					/* Set clock rate */
	inv
}pm_opt_t;

static void pm_get_power_mode(int fd, char *param);
static void pm_set_power_mode(int fd, char *param);
static void pm_get_clk_rate(int fd, char *param);
static void pm_set_clk_rate(int fd, char *param);

/* Helper functions */
static const char* sc_pw2str(sc_pm_power_mode_t pw);
static int sc_str2pw(char *str);
static int get_resource_clock_rate(char *str_input, sc_rsrc_t *resource, sc_pm_clk_t *clk_type, sc_pm_clock_rate_t *clk_rate);
static int get_resource_pw(char *str_input, int *resource, sc_pm_power_mode_t *power_mode);

/*
 * Main routine for power management service
 * */
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

/*
 * Routine to handle power mode get requests
 * */
void pm_get_power_mode(int fd, char *param){

	char rsrc_str[256];
	int rsrc;

	/* Process arguments */
	if(param == NULL){
		int ret, valid = 0;
		do{
			printf("Enter the resource name, for instance: SC_R_A53 \n");
			printf("If you wish to dump all resources information enter SC_R_ALL\n");
			ret = scanf("%s", &rsrc_str);
			rsrc = sc_str2rsrc(rsrc_str);
			if((ret == 1) && (rsrc != -1)){
				valid = 1;
			} else{
				printf("%s (%d) is an invalid option\n", rsrc_str, rsrc);
				valid = 0;
			}
		} while(valid == 0);
	} else{
		/* Parse parameters */
		rsrc = sc_str2rsrc(param);
		if(rsrc == -1){
			printf("PM:GET_PW:Invalid -param=%s\n", param);
			return;
		}
	}

	/* Get resources power mode */
	imx_dcmd_sc_pm_res_t res_pm;
	sc_err_t status;
	int err, cnt = 0, rsrc_last;

	/* If rsrc is SC_R_ALL populate variables so that the loop traverses all resources,
	 * otherwise just enter the loop once to dump the desire resource status
	 * */
	if(rsrc == SC_R_ALL){
		rsrc = 0;
		rsrc_last = SC_R_LAST - 1;
	} else{
		rsrc_last = rsrc;
	}

	/* Print header */
	printf("Resource,Power mode\n");
	for(; rsrc <= rsrc_last; rsrc++){
		/* Populate PM structure with desired resource */
		res_pm.resource = rsrc;

		/* Get resource power mode */
		do {
			err = devctl(fd, IMX_DCMD_SC_PM_GET_RESOURCE_POWER_MODE, &res_pm, sizeof(imx_dcmd_sc_pm_res_t), (int *) &status);
		} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));

		/* Print resource status */
		if(status == SC_ERR_NONE){
			printf("%s,%s\n", sc_rsrc2str(rsrc), sc_pw2str(res_pm.mode));
		} else {
			printf("ERR:%s,%s\n", sc_rsrc2str(rsrc), sc_status2str(status));
		}
	}
}

/*
 * Routine to handle power mode set requests
 * */
void pm_set_power_mode(int fd, char *param){

	int rsrc = 0;
	sc_pm_power_mode_t power_mode = SC_PM_PW_MODE_ON;			/* On is usually less harmful that turning things off*/
	char str_input[500];


	/* Process arguments */
	if(param == NULL){
		int valid = 0;
		do {
			printf("Enter the resource name and power mode to set in the following format: \n");
			printf("Resource,Power_mode \n");
			printf("For instance: SC_R_A53,SC_PM_PW_MODE_ON \n");
			printf("or enter SC_R_ALL,SC_PM_PW_MODE_X to set all resources to X power mode\n");
			scanf("%s", &str_input);

			/* Parse string for Resource and Power mode */
			if(get_resource_pw(str_input, &rsrc, &power_mode) == 0){
				valid = 1;
			} else{
				printf("Invalid option: %s\n", str_input);
				valid = 0;
			}
		} while (valid == 0);
	} else{
		/* Parse parameters */
		if(get_resource_pw(param, &rsrc, &power_mode) != 0){
			printf("PM:GET_PW:Invalid -param=%s\n", param);
			return;
		}
	}

	/* Check parameters */
	if(power_mode > SC_PM_PW_MODE_ON){
		printf("Invalid power mode setting!\n");
		return;
	}

	/* Set resources power mode */
	imx_dcmd_sc_pm_res_t res_pm;
	sc_err_t status;
	res_pm.mode = power_mode;
	int err, cnt = 0, rsrc_last;

	/* If rsrc is SC_R_ALL populate variables so that the loop traverses all resources,
	 * otherwise just enter the loop once to dump the desire resource status
	 * */
	if(rsrc == SC_R_ALL){
		rsrc = 0;
		rsrc_last = SC_R_LAST - 1;
	} else{
		rsrc_last = rsrc;
	}

	for(; rsrc <= rsrc_last; rsrc++){
		res_pm.resource = rsrc;

		/* Set resource power mode */
		do {
			err = devctl(fd, IMX_DCMD_SC_PM_SET_RESOURCE_POWER_MODE, &res_pm, sizeof(imx_dcmd_sc_pm_res_t), (int *) &status);
		} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
		if(status == SC_ERR_NONE){
			printf("%s (%d) power mode set to %s (%d)\n", sc_rsrc2str(rsrc), rsrc, sc_pw2str(res_pm.mode), res_pm.mode);
		} else {
			printf("Failed to set %s (%d) power mode %s, err: %s (%d)\n", sc_rsrc2str(rsrc), rsrc, sc_pw2str(res_pm.mode), sc_status2str(status), status);
		}
	}
}

/*
 * Routine to handle clock rate get requests
 * */
void pm_get_clk_rate(int fd, char *param){

	char rsrc_str[256];
	int rsrc;

	/* Process arguments */
	if(param == NULL){
		int ret, valid = 0;
		do{
			printf("Enter the resource name, for instance: SC_R_A53 \n");
			printf("If you wish to dump all resources information enter SC_R_ALL\n");
			ret = scanf("%s", &rsrc_str);
			rsrc = sc_str2rsrc(rsrc_str);
			if((ret == 1) && (rsrc != -1)){
				valid = 1;
			} else{
				printf("%s (%d) is an invalid option\n", rsrc_str, rsrc);
				valid = 0;
			}
		} while(valid == 0);
	} else{
		/* Parse parameters */
		rsrc = sc_str2rsrc(param);
		if(rsrc == -1){
			printf("PM:SET_CLK_RATE:Invalid -param=%s\n", param);
			return;
		}
	}

	/* Get resources clock rate */
	printf("Resource, Clock, Clock rate\n");
	imx_dcmd_sc_pm_clock_rate_t clk_rate;
	sc_err_t status;
	int err, cnt = 0, rsrc_last;

	/* If rsrc is SC_R_ALL populate variables so that the loop traverses all resources,
	 * otherwise just enter the loop once to dump the desire resource status
	 * */
	if(rsrc == SC_R_ALL){
		rsrc = 0;
		rsrc_last = SC_R_LAST - 1;
	} else{
		rsrc_last = rsrc;
	}

	for(; rsrc <= rsrc_last; rsrc++){
		clk_rate.resource = rsrc;

		for(clk_rate.clk = 0; clk_rate.clk <= LAST_CLK; clk_rate.clk++){
			/* Get resource clock rate for all clocks */
			do {
				err = devctl(fd, IMX_DCMD_SC_PM_GET_CLOCK_RATE, &clk_rate, sizeof(imx_dcmd_sc_pm_clock_rate_t), (int *) &status);
			} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
			if(status == SC_ERR_NONE){
				printf("%s,clock:%d, rate:%d\n", sc_rsrc2str(rsrc), clk_rate.clk, clk_rate.rate);
			} else {
				printf("ERR:%s,%s\n", sc_rsrc2str(rsrc), sc_status2str(status));
			}
		}
	}
}

/*
 * Routine to handle clock rate set requests
 * */
void pm_set_clk_rate(int fd, char *param){

	imx_dcmd_sc_pm_clock_rate_t clk_rate;
	char str_input[500];

	/* Process arguments */
	if(param == NULL){
		int valid = 0;
		do {
			printf("Enter the resource name, clock type and clock rate to set in the following format: \n");
			printf("Resource,clock_type,clock_rate \n");
			printf("For instance: SC_R_A53,SC_PM_CLK_CPU,1200000000 \n");
			printf("The command above will set SC_R_A53 CPU clock to 1200MHz \n");
			printf("For a complete list of clock types refer to the SCFW API reference guide \n");
			scanf("%s", &str_input);

			/* Parse string for Resource and Power mode */
			if(get_resource_clock_rate(str_input, &clk_rate.resource, &clk_rate.clk, &clk_rate.rate) == 0){
				valid = 1;
			} else{
				printf("Invalid option: %s\n", str_input);
				valid = 0;
			}
		} while (valid == 0);
	} else{
		/* Parse parameters */
		if(get_resource_clock_rate(param, &clk_rate.resource, &clk_rate.clk, &clk_rate.rate) != 0){
			printf("PM:SET_CLK_RATE:Invalid -param=%s\n", param);
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

/**** Helper functions *****/

/* Returns an index from a power mode type
 *
 * */
static int sc_str2pw(char *str){

	return str2index(str, sc_power_mode_str, sc_power_mode_str_len);
}

/*
 * This function parses a string delimited by ','
 * and returns a resource and power mode.
 *
 * Returns 0 if successful, -1 if not.
 * */
static int get_resource_pw(char *str_input, int *resource, sc_pm_power_mode_t *power_mode){
	/* Parse string for Resource and Power mode */
	char **split_strings;

	split_strings = str_split(str_input);
	if(split_strings){
		/* Parse string and free allocated space*/
		int i;
		for(i = 0; *(split_strings + i); i++){
			/* Only the two first strings are taken everything else is discarded */
			if(i == 0){
				*resource = sc_str2rsrc(*(split_strings + i));
			} else if(i == 1){
				*power_mode = sc_str2pw(*(split_strings + i));
			}
			free(*(split_strings + i));
		}
		free(split_strings);
	}
	if((*resource == -1) || (*power_mode == -1)){
		return -1;
	} else{
		return 0;
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

/* Parses a clock name and return the clock type value
 *
 * Returns the clock type value, if no clock type matches returns -1
 * */
static int sc_str2clk(char *str){
	int index = 0;
	index = str2index(str, sc_clock_str, sc_clock_str_len);

	if(index == -1){
		return index;
	} else{
		return sc_clock_val[index];
	}
}

/*
 * This function parses a string delimited by ','
 * and returns a resource, clock type and clock rate.
 *
 * Returns 0 if successful, -1 if not.
 * */
static int get_resource_clock_rate(char *str_input, sc_rsrc_t *resource, sc_pm_clk_t *clk_type, sc_pm_clock_rate_t *clk_rate){
	/* Parse string for Resource and Power mode */
	char **split_strings;

	/* Alias variables since clk types are unsigned */
	int resource_alias = -1;
	int clk_type_alias = -1;
	int clk_rate_alias = -1;

	split_strings = str_split(str_input);
	if(split_strings){
		/* Parse string and free allocated space*/
		int i;
		for(i = 0; *(split_strings + i); i++){
			/* Only the three first strings are taken everything else is discarded */
			if(i == 0){
				resource_alias = sc_str2rsrc(*(split_strings + i));
			} else if(i == 1){
				clk_type_alias = sc_str2clk(*(split_strings + i));
			} else if(i == 2){
				clk_rate_alias = atoi(*(split_strings + i));
			}
			free(*(split_strings + i));
		}
		free(split_strings);
	}
	if((resource_alias == -1) || (clk_type_alias == -1) || (clk_rate_alias == -1)){
		return -1;
	} else{
		*resource = resource_alias;
		*clk_type = clk_type_alias;
		*clk_rate = clk_rate_alias;
		return 0;
	}
}
