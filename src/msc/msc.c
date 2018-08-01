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
	unique_id,					/* Unique id */
	inv
}misc_opt_t;

/* Array of control strings */
const char *sc_controls_str[] = {
"SC_C_TEMP",
"SC_C_TEMP_HI",
"SC_C_TEMP_LOW",
"SC_C_PXL_LINK_MST1_ADDR",
"SC_C_PXL_LINK_MST2_ADDR",
"SC_C_PXL_LINK_MST_ENB",
"SC_C_PXL_LINK_MST1_ENB",
"SC_C_PXL_LINK_MST2_ENB",
"SC_C_PXL_LINK_SLV1_ADDR",
"SC_C_PXL_LINK_SLV2_ADDR",
"SC_C_PXL_LINK_MST_VLD",
"SC_C_PXL_LINK_MST1_VLD",
"SC_C_PXL_LINK_MST2_VLD",
"SC_C_SINGLE_MODE",
"SC_C_ID",
"SC_C_PXL_CLK_POLARITY",
"SC_C_LINESTATE",
"SC_C_PCIE_G_RST",
"SC_C_PCIE_BUTTON_RST",
"SC_C_PCIE_PERST",
"SC_C_PHY_RESET",
"SC_C_PXL_LINK_RATE_CORRECTION",
"SC_C_PANIC",
"SC_C_PRIORITY_GROUP",
"SC_C_TXCLK",
"SC_C_CLKDIV",
"SC_C_DISABLE_50",
"SC_C_DISABLE_125",
"SC_C_SEL_125",
"SC_C_MODE",
"SC_C_SYNC_CTRL0",
"SC_C_KACHUNK_CNT",
"SC_C_KACHUNK_SEL",
"SC_C_SYNC_CTRL1",
"SC_C_DPI_RESET",
"SC_C_MIPI_RESET",
"SC_C_DUAL_MODE",
"SC_C_VOLTAGE",
"SC_C_PXL_LINK_SEL",
"SC_C_OFS_SEL",
"SC_C_OFS_AUDIO",
"SC_C_OFS_PERIPH",
"SC_C_OFS_IRQ",
"SC_C_RST0",
"SC_C_RST1",
"SC_C_SEL0",
"SC_C_CALIB0",
"SC_C_CALIB1",
"SC_C_CALIB2",
"SC_C_IPG_DEBUG",
"SC_C_IPG_DOZE",
"SC_C_IPG_WAIT",
"SC_C_IPG_STOP",
"SC_C_IPG_STOP_MODE",
"SC_C_IPG_STOP_ACK",
"SC_C_LAST"
};
#define sc_controls_str_len  (sizeof(sc_controls_str)/sizeof(sc_controls_str[0]))

/* Array of temperature strings */
const char *sc_temp_str[] = {
"SC_MISC_TEMP",
"SC_MISC_TEMP_HIGH",
"SC_MISC_TEMP_LOW"
};
#define sc_temp_str_len  (sizeof(sc_temp_str)/sizeof(sc_temp_str[0]))

/* Prototypes */
static void misc_get_ctl(int fd, char *param);
static void misc_set_ctl(int fd, char *param);
static void misc_get_temp(int fd, char *param);
static void misc_set_temp(int fd, char *param);
static void misc_bld_inf(int fd);
static void misc_unique_id(int fd);

/* Helper functions */
static const char* sc_con2str(sc_ctrl_t con);
static int sc_str2con(char *str);
static const char* sc_temp2str(sc_misc_temp_t temp);
static int sc_str2temp(char *str);
static int get_resource_temp(char *str_input, sc_rsrc_t *resource, sc_misc_temp_t *temp);
static int get_resource_temp_cel_frac(char *str_input, sc_rsrc_t *resource, sc_misc_temp_t *temp, int16_t *celsius, int8_t *tenths);
static int get_resource_control_val(char *str_input, sc_rsrc_t *resource, sc_ctrl_t *control, uint32_t *val);
static int get_resource_control(char *str_input, sc_rsrc_t *resource, sc_ctrl_t *control);

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
		printf("6.- Get Unique ID \n");

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
		}  else if(strncmp(argv[0], "unique_id", strlen("unique_id")) == 0){
			misc_opt = unique_id;
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
			case unique_id:
				misc_unique_id(fd);
				break;
			default:
				printf("MISC: invalid service option\n");
				break;
		}
}

void misc_get_ctl(int fd, char *param){

	imx_dcmd_sc_misc_control_t ctl;
	char str_input[500];

	/* Process arguments */
	if(param == NULL){
		int valid = 0;
		do{
			printf("Enter the resource name and control to dump in the following format: \n");
			printf("Resource,Control\n");
			printf("for instance: SC_R_A53,SC_C_TEMP\n");
			printf("The command above will get the temperature sensor value for SC_R_A53\n");
			printf("For a complete list of available controls refer to the SCFW API reference guide \n");
			scanf("%s", str_input);

			if(get_resource_control(str_input, &ctl.resource, &ctl.ctrl) == 0){
				valid = 1;
			} else{
				printf("%s is an invalid option\n", str_input);
				valid = 0;
			}
		} while(valid == 0);
	} else{
		/* Parse parameters */
		if(get_resource_control(param, &ctl.resource, &ctl.ctrl) != 0){
			printf("MISC:GET_CTL:Invalid -param=%s\n", param);
			return;
		}
	}

	/* Get control */
	printf("Resource, Control, Value\n");
	sc_err_t status;
	int err, cnt = 0;

	/* Get resource control */
	do {
		err = devctl(fd, IMX_DCMD_SC_MISC_GET_CONTROL, &ctl, sizeof(imx_dcmd_sc_misc_control_t), (int *) &status);
	} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
	if(status == SC_ERR_NONE){
		printf("%s, %s (%d), %d\n", sc_rsrc2str(ctl.resource), sc_con2str(ctl.ctrl), ctl.ctrl, ctl.val);
	} else {
		printf("ERR:%s,%s\n", sc_rsrc2str(ctl.resource), sc_status2str(status));
	}
}

void misc_set_ctl(int fd, char *param){
	imx_dcmd_sc_misc_control_t ctl;
	char str_input[500];

	/* Process arguments */
	if(param == NULL){
		int valid = 0;
		do {
			printf("Enter the resource name, control and value to set in the following format: \n");
			printf("Resource,control,value \n");
			printf("For instance: SC_R_A53,SC_C_TEMP_HI,0 \n");
			printf("The command above will set SC_R_A53 High limit temperature alarm to 0 \n");
			printf("For a complete list of available controls refer to the SCFW API reference guide \n");
			scanf("%s", &str_input);

			/* Parse string for Resource and Power mode */
			if(get_resource_control_val(str_input, &ctl.resource, &ctl.ctrl, &ctl.val) == 0){
				valid = 1;
			} else{
				printf("Invalid option: %s\n", str_input);
				valid = 0;
			}
		} while (valid == 0);
	} else{
		/* Parse parameters */
		if(get_resource_control_val(param, &ctl.resource, &ctl.ctrl, &ctl.val) != 0){
			printf("PM:SET_CLK_RATE:Invalid -param=%s\n", param);
			return;
		}
	}

	/* Set control */
	sc_err_t status;
	int err, cnt = 0;

	/* Set resource control */
	do {
		err = devctl(fd, IMX_DCMD_SC_MISC_SET_CONTROL, &ctl, sizeof(imx_dcmd_sc_misc_control_t), (int *) &status);
	} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
	if(status == SC_ERR_NONE){
		printf("%s, %s (%d) set to %d\n", sc_rsrc2str(ctl.resource), sc_con2str(ctl.ctrl), ctl.ctrl, ctl.val);
	} else {
		printf("Failed to set %s, %s to %d, %s\n", sc_rsrc2str(ctl.resource), sc_con2str(ctl.ctrl), ctl.val, sc_status2str(status));
	}
}

void misc_get_temp(int fd, char *param){
	imx_dcmd_sc_misc_temp_t temp;
	char str_input[500];

	/* Process arguments */
	if(param == NULL){
		int valid = 0;
		do{
			printf("Enter the resource name and temperature sensor, in the following format: \n");
			printf("Resource,temperature_sensor\n");
			printf("for instance: SC_R_A53,SC_MISC_TEMP\n");
			printf("The command above will get the temperature sensor value for SC_R_A53\n");
			printf("For a complete list of available controls refer to the SCFW API reference guide \n");
			scanf("%s", str_input);

			if(get_resource_temp(str_input, &temp.resource, &temp.temp) == 0){
				valid = 1;
			} else{
				printf("%s is an invalid option\n", str_input);
				valid = 0;
			}
		} while(valid == 0);
	} else{
		/* Parse parameters */
		if(get_resource_temp(param, &temp.resource, &temp.temp) != 0){
			printf("MISC:GET_TEMP:Invalid -param=%s\n", param);
			return;
		}
	}

	/* Get temperature */
	printf("Resource,Temperature sensor,celsius,tenths\n");
	sc_err_t status;
	int err, cnt = 0;

	/* Get temperature */
	do {
		err = devctl(fd, IMX_DCMD_SC_MISC_GET_TEMP, &temp, sizeof(imx_dcmd_sc_misc_temp_t), (int *) &status);
	} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
	if(status == SC_ERR_NONE){
		printf("%s, %s, %d, %d\n", sc_rsrc2str(temp.resource), sc_temp2str(temp.temp), temp.celsius, temp.tenths);
	} else {
		printf("ERR:%s,%s\n", sc_rsrc2str(temp.resource), sc_status2str(status));
	}
}

void misc_set_temp(int fd, char *param){
	imx_dcmd_sc_misc_temp_t temp;
	char str_input[500];

	/* Process arguments */
	if(param == NULL){
		int valid = 0;
		do{
			printf("Enter the resource, temperature, celsius value and tenth value in the following format: \n");
			printf("Resource,temperature_sensor,celsius_val,tenth_val\n");
			printf("for instance: SC_R_A53,SC_MISC_TEMP_HIGH,80,6\n");
			printf("The command above will get the temperature sensor alarm value for SC_R_A53\n");
			printf("For a complete list of available controls refer to the SCFW API reference guide \n");
			scanf("%s", str_input);

			if(get_resource_temp_cel_frac(str_input, &temp.resource, &temp.temp, &temp.celsius, &temp.tenths) == 0){
				valid = 1;
			} else{
				printf("%s is an invalid option\n", str_input);
				valid = 0;
			}
		} while(valid == 0);
	} else{
		/* Parse parameters */
		if(get_resource_temp_cel_frac(param, &temp.resource, &temp.temp, &temp.celsius, &temp.tenths) != 0){
			printf("MISC:GET_TEMP:Invalid -param=%s\n", param);
			return;
		}
	}

	/* Set temperature */
	sc_err_t status;
	int err, cnt = 0;

	/* Set temperature */
	do {
		err = devctl(fd, IMX_DCMD_SC_MISC_SET_TEMP, &temp, sizeof(imx_dcmd_sc_misc_temp_t), (int *) &status);
	} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
	if(status == SC_ERR_NONE){
		printf("%s, %s set to %d Celsius, %d tenths\n", sc_rsrc2str(temp.resource), sc_temp2str(temp.temp), temp.celsius, temp.tenths);
	} else {
		printf("ERR:%s,%s\n", sc_rsrc2str(temp.resource), sc_status2str(status));
	}
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

void misc_unique_id(int fd){
	imx_dcmd_sc_misc_unique_id_t unique_id;
	sc_err_t status;
	int err, cnt = 0;

	/* Get unique id */
	do{
		err = devctl(fd, IMX_DCMD_SC_MISC_UNIQUE_ID, &unique_id, sizeof(imx_dcmd_sc_misc_unique_id_t), (int *) &status);
	} while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));
	if(status == SC_ERR_NONE){
		printf("UID_H: %x, UID_L: %x\n", unique_id.id_h, unique_id.id_l);
	} else{
		printf("Failed to get Unique ID, %s (%d)\n", sc_status2str(status), status);
	}
}

/********** Helper functions ******************/

static const char* sc_con2str(sc_ctrl_t con){
	if(con < sc_controls_str_len){
		return sc_controls_str[con];
	} else {
		return "INVALID CONTROL";
	}
}

static int sc_str2con(char *str){
	int index = 0;
	index = str2index(str, sc_controls_str, sc_controls_str_len);

	return index;
}

static const char* sc_temp2str(sc_misc_temp_t temp){
	if(temp < sc_temp_str_len){
		return sc_temp_str[temp];
	} else {
		return "INVALID TEMP SENSOR";
	}
}

static int sc_str2temp(char *str){
	int index = 0;
	index = str2index(str, sc_temp_str, sc_temp_str_len);

	return index;
}

static int get_resource_temp_cel_frac(char *str_input, sc_rsrc_t *resource, sc_misc_temp_t *temp, int16_t *celsius, int8_t *tenths){
	/* Parse string for Resource, control and value */
	char **split_strings;

	/* Alias variables since types are unsigned */
	int resource_alias = -1;
	int temperature_alias = -1;

	split_strings = str_split(str_input);
	if(split_strings){
		/* Parse string and free allocated space*/
		int i;
		for(i = 0; *(split_strings + i); i++){
			/* Only the three first strings are taken everything else is discarded */
			if(i == 0){
				resource_alias = sc_str2rsrc(*(split_strings + i));
			} else if(i == 1){
				temperature_alias = sc_str2temp(*(split_strings + i));
			} else if(i == 2){
				*celsius = atoi(*(split_strings + i));
			} else if(i == 3){
				*tenths = atoi(*(split_strings + i));
			}
			free(*(split_strings + i));
		}
		free(split_strings);
	}
	if((resource_alias == -1) || (temperature_alias == -1) || (*celsius < 0) || (*tenths < 0)){
		return -1;
	} else{
		*resource = resource_alias;
		*temp = temperature_alias;
		return 0;
	}
}

static int get_resource_temp(char *str_input, sc_rsrc_t *resource, sc_misc_temp_t *temp){
	/* Parse string for Resource and Control */
	char **split_strings;

	/* Alias variables since types are unsigned */
	int resource_alias = -1;
	int temperature_alias = -1;

	split_strings = str_split(str_input);
	if(split_strings){
		/* Parse string and free allocated space*/
		int i;
		for(i = 0; *(split_strings + i); i++){
			/* Only the two first strings are taken everything else is discarded */
			if(i == 0){
				resource_alias = sc_str2rsrc(*(split_strings + i));
			} else if(i == 1){
				temperature_alias = sc_str2temp(*(split_strings + i));
			}
			free(*(split_strings + i));
		}
		free(split_strings);
	}
	if((resource_alias == -1) || (temperature_alias == -1)){
		return -1;
	} else{
		*resource = resource_alias;
		*temp = (uint8_t)temperature_alias;
		return 0;
	}
}

static int get_resource_control_val(char *str_input, sc_rsrc_t *resource, sc_ctrl_t *control, uint32_t *val){
	/* Parse string for Resource, control and value */
	char **split_strings;

	/* Alias variables since types are unsigned */
	int resource_alias = -1;
	int control_alias = -1;
	int val_alias = -1;

	split_strings = str_split(str_input);
	if(split_strings){
		/* Parse string and free allocated space*/
		int i;
		for(i = 0; *(split_strings + i); i++){
			/* Only the three first strings are taken everything else is discarded */
			if(i == 0){
				resource_alias = sc_str2rsrc(*(split_strings + i));
			} else if(i == 1){
				control_alias = sc_str2con(*(split_strings + i));
			} else if(i == 2){
				val_alias = atoi(*(split_strings + i));
			}
			free(*(split_strings + i));
		}
		free(split_strings);
	}
	if((resource_alias == -1) || (control_alias == -1) || (val_alias < 0)){
		return -1;
	} else{
		*resource = resource_alias;
		*control = control_alias;
		*val = val_alias;
		return 0;
	}
}

static int get_resource_control(char *str_input, sc_rsrc_t *resource, sc_ctrl_t *control){
	/* Parse string for Resource and Control */
	char **split_strings;

	/* Alias variables since types are unsigned */
	int resource_alias = -1;
	int control_alias = -1;

	split_strings = str_split(str_input);
	if(split_strings){
		/* Parse string and free allocated space*/
		int i;
		for(i = 0; *(split_strings + i); i++){
			/* Only the two first strings are taken everything else is discarded */
			if(i == 0){
				resource_alias = sc_str2rsrc(*(split_strings + i));
			} else if(i == 1){
				control_alias = sc_str2con(*(split_strings + i));
			}
			free(*(split_strings + i));
		}
		free(split_strings);
	}
	if((resource_alias == -1) || (control_alias == -1)){
		return -1;
	} else{
		*resource = resource_alias;
		*control = control_alias;
		return 0;
	}
}
