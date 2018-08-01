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
to select the SCFW function call to make. The user can select the service only and it will be 
prompted to select a function within that service, if a service and function is 
selected but no parameters are passed the user will be prompted to enter the parameters.

The utility usage information can be displayed on QNX by calling 'use scfw_cli'

	scfw_cli [options]

	options:
		-svc=[string]			Select the service to call.
		-opt=[string]			Select the function to call.
		-param=[string],[string],...	Pass the parameters of the function that will be called.

	-svc: The services supported are:
		Power Management Service =pm
		Miscellaneous service =misc
	-opt: The functions currently supported are:
		sc_pm_set_resource_power_mode =set_resource_power_mode
		sc_pm_get_resource_power_mode =get_resource_power_mode
		sc_pm_set_clock_rate =set_clock_rate
		sc_pm_get_clock_rate =get_clock_rate
		sc_misc_get_control =get_control
		sc_misc_set_control =set_control
		sc_misc_get_temp =get_temp
		sc_misc_set_temp =set_temp
		sc_misc_build_info =build_info
	-param: The parameters will vary depending on the function that will be called, but all parameters are delimited by ','.
		Refer to the SCFW documentation for a definition of parameters that each function takes.

	Usage examples:
		Power management service:
			set_resource_power_mode: This function takes 2 parameters- Resource,Power_mode
					For instance to turn on SC_R_A53 call: 
					    scfw_cli -svc=pm -opt=set_resource_power_mode -param=SC_R_ALL,SC_PM_PW_MODE_ON
					For a complete list of power modes refer to the SCFW API guide.
			get_resource_power_mode: This function takes 1 parameter- Resource
					For instance to get the power mode from all resources call: 
					    scfw_cli -svc=pm -opt=get_resource_power_mode -param=SC_R_ALL
			set_clock_rate: This functions takes 3 parameters- Resource,Clock_type,Clock_rate_in_Hz
					To set the CPU clock rate to 1200MHz on SC_R_A53 call: 
					    scfw_cli -svc=pm -opt=set_clock_rate -param=SC_R_A53,SC_PM_CLK_CPU,1200000000
					Refer to the SCFW documentation 'Clock list' for the type of clock that each resource has.
			get_clock_rate: This function takes 1 parameter- Resource
					To get the clock rates from all resources call: 
					    scfw_cli -svc=pm -opt=get_clock_rate -param=SC_R_ALL
		Miscellaneous service:
			get_control: 	This function takes 2 parameters- Resource,Control
					For a complete list of controls and resource refer to the SCFW API guide.
					  scfw_cli -svc=misc -opt=get_control -param=SC_R_SYSTEM,SC_C_ID
			set_control:    This function takes 3 parameters- Resource,Control,Value
					  scfw_cli -svc=misc -opt=set_control -param=SC_R_SDHC_0,SC_C_VOLTAGE,3300
			get_temp: 	This function takes 2 parameters- Resource,Temperature_sensor
					For a complete list of temperature sensors refer to the SCFW API guide.
				  	  scfw_cli -svc=misc -opt=get_temp -param=SC_R_A53,SC_MISC_TEMP
			set_temp: 	This function takes 4 parameters- Resource,Temperature_sensor,Celsius_value,Tenths_value
					  scfw_cli -svc=misc -opt=set_temp -param=SC_R_A53,SC_MISC_TEMP_HIGH,80,0
			build_info: 	This function returns the build information for the SCFW, it is called as:
					  scfw_cli -svc=misc -opt=build_info

		The same model is used for all available functions and the parameters order follows the order in the SCFW API guide.

## Authors

* **Manuel Rodriguez {manuel.rodriguez@nxp.com}** 
