#include <stdio.h>
#include <stdlib.h>

#include <sys/neutrino.h>

#include <sys/imx_sci_mgr.h>

/* File control */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Services */
#include "pm/pm.h"

#include <errno.h>

int main(int argc, char *argv[]) {
	int sci_fd;
    sc_err_t status;
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
    	int svc;
    	printf("Select service:\n");
    	printf("1.- Power Management Service\n");

    	scanf("%d", &svc);
    	switch(svc){
    		case 1:
    			pm_service_main(sci_fd, 0, NULL);
    			break;
    		default:
    			printf("Please select a valid option\n");
    			break;
    	}
    } else{
    	printf("%d, %s", argc, argv[0]);
		/* TODO implement argument friendly set-up*/
		return EXIT_FAILURE;
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
