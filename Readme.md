# System Controller Firmware command line interface (scfw_cli)

A command line interface that allows you to make SCFW requests using QNX's console.

## Building

In order to build this on your system you will need to modify the Makefile:
You will need to build the BSP for the board you want to use and modify the BSP_DIR
variable in the Makefile to point to your BSP directory. 
e.g:
BSP_DIR= /home/Your/BSP/root/dir

The BSP contains the headers/libraries that this project needs to link against.

## Usage

The scfw_cli utility can be called without options and the user will be prompted
to select the SCFW function call to make. The user can select the service only for instance
and it will be prompted to select a function within that service, if a service and function is 
selected but no parameter the user will be prompted to enter the parameters.

	scfw_cli [options]

	options:
		-svc=[string]			Select the service to call.
		-opt=[string]			Select the function to call.
		-param=[int]_[int]_...		Pass the parameters of the function that will be called.

	-svc: The services supported are:
		Power Management Service =pm
	-opt: The functions supported are:
		sc_pm_set_resource_power_mode =set-rsrc-pw
		sc_pm_get_resource_power_mode =get_rsrc-pw
	-param: The parameters will vary depending on the function that will be called, but all parameters are delimited by '_'.

	Parameters used by each function:
		Power management service:
			set-rsrc-pw: This functions takes 3 parameters, firstResource_lastResource_powerMode
					The firstResource parameter is the first resource that will change it's powerMode.
					The lastResource parameter is the last resource that will change it's powerMode.
					The powerMode parameter is the power mode to set for the parameters, the options are:
							OFF = 0
							STANDBY = 1
							LOWPOWER= 2
							ON = 3
					If only one resource is needed then set firstResource and lastResource equal.
					To turn on resources 0 through 541 call: 
					    scfw_cli -svc=pm -opt=set-rsrc-pw -param=0_541_3
					To turn off resource 0 call: 
					    scfw_cli -svc=pm -opt=set-rsrc-pw -param=0_0_0
			get-rsrc-pw: This function takes 2 parameters, firstResource_lastResource
					The firsResource parameter is the first resource to get its power mode from.
					The lastResource parameter is the last resource to get its power mode from.
					If only one resource is needed then set firstResource and lastResource equal.
					To get the power mode from all resources call: 
					    scfw_cli -svc=pm -opt=get-rsrc-pw -param=0_541
					To get the power mode from resource 0 call: 
					    scfw_cli -svc=pm -opt=get-rsrc-pw -param=0_0

## Authors

* **Manuel Rodriguez {manuel.rodriguez@nxp.com}** 