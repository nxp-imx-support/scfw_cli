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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/neutrino.h>

#include <sys/imx_sci_mgr.h>

/* File control */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Services */
#include "pm/pm.h"
#include "msc/msc.h"

#include <errno.h>

/* Local types */
typedef enum svc_e{
	pm = 1,   			/* Power Management Service */
	misc,			/* Miscellaneous service */
	rm,				/* Resource Management Service */
	pad,			/* Pad configuration service */
	timer,			/* Timer service */
	irq,			/* Interrupt service */
	inv				/* Invalid selection*/
}svc_t;

#define PARAM 2		/* Holds the amount of valid parameters to pass to the services (opt and param)*/

int main(int argc, char *argv[]) {
	int sci_fd;
    sc_err_t status;
    svc_t svc = inv;
    char *options[PARAM] = {NULL, NULL};	/* Array used to pass information to the services */
    int cnt = 0;
    int err;

    /* Enable IO capability */
    if (ThreadCtl(_NTO_TCTL_IO, NULL ) == -1) {
        printf("Netio error:  ThreadCtl.");
        return EXIT_FAILURE;
    }

    /* Open System Controller device */
    sci_fd = open("/dev/sc", O_RDWR);
    if (sci_fd < 0) {
        printf("%s failed to open /dev/sci device, errno %i.\n", __FUNCTION__, errno);
        return EXIT_FAILURE;
    }

    /* Open IPC channel */
    do {
        err = devctl(sci_fd, IMX_DCMD_SC_OPEN, NULL, 0, (int *) &status);
    } while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));

    /* Process parameters */
    if(argc == 1){
    	int tmp;
    	printf("Select service:\n");
    	printf("1.- Power Management Service\n");
    	printf("2.- Miscellaneous Service\n");

    	scanf("%d", &tmp);
    	switch(tmp){
    		case pm:
    			svc = pm;
    			break;
    		case misc:
    			svc = misc;
    			break;
    		default:
    			svc = inv;
    			printf("Please select a valid option\n");
    			break;
    	}
    } else{
    	const char *tmp_str;

    	/* Parse command options */
		for(int i = 1; i < argc; i++){
			if(strncmp(argv[i], "-svc=", strlen("-svc=")) == 0){
				tmp_str = argv[i] + strlen("-svc=");
				/* Get service */
				if(strncmp(tmp_str, "pm", strlen("pm")) == 0){
					svc = pm;
				} else if(strncmp(tmp_str, "misc", strlen("misc")) == 0){
					svc = misc;
				} else{
					svc = inv;
					printf("Invalid -svc option.\n");
				}
			} else if(strncmp(argv[i], "-opt=", strlen("-opt=")) == 0){
				options[0] = argv[i] + strlen("-opt=");
			} else if(strncmp(argv[i], "-param=", strlen("-param=")) == 0){
				options[1] = argv[i] + strlen("-param=");
			} else{
				printf("%s is an invalid option\n", argv[i]);
			}
		}
    }

    /* Call Service */
	switch(svc){
		case pm:
			pm_service_main(sci_fd, options);
			break;
		case misc:
			misc_service_main(sci_fd, options);
			break;
		case inv:
		default:
			printf("Invalid service selection\n");
			break;
	}


    /* Close IPC channel between SCU */
    do {
        err = devctl(sci_fd, IMX_DCMD_SC_CLOSE, NULL, 0, NULL);
    } while ((err == EAGAIN) && (cnt++ < 10) && (delay(10) == 0));

    /* Close System Controller device */
    err = close(sci_fd);
    if (err < 0) {
        printf("%s failed to close /dev/sc device, errno %i.\n", __FUNCTION__, errno);
        return EXIT_FAILURE;
    }

	return EXIT_SUCCESS;
}
