#include "Lepton_I2C.h"

#include <iostream>

#include "leptonSDKEmb32PUB/LEPTON_SDK.h"
#include "leptonSDKEmb32PUB/LEPTON_SYS.h"
#include "leptonSDKEmb32PUB/LEPTON_Types.h"
#include "leptonSDKEmb32PUB/LEPTON_AGC.c"

#define BAUD_RATE 400

bool _connected;

LEP_CAMERA_PORT_DESC_T _port;
LEP_AGC_ENABLE_E_PTR agcEnableStatePtr;
LEP_SYS_AUX_TEMPERATURE_CELCIUS_T_PTR auxTempPtr;

int lepton_connect() {
	LEP_OpenPort(1, LEP_CCI_TWI, BAUD_RATE, &_port);
	_connected = true;
	return 0;
}

void lepton_perform_ffc() {
	if(!_connected) {
		lepton_connect();
	}
	LEP_RunSysFFCNormalization(&_port);
}

void lepton_get_agc() {
	if(!_connected) {
		lepton_connect();
	}
	agcEnableStatePtr = (LEP_AGC_ENABLE_E_PTR)malloc(sizeof(int));
	LEP_GetAgcEnableState(&_port, agcEnableStatePtr);
	std::cout << *agcEnableStatePtr << '\n';
}

void lepton_aux_temp() {
	if(!_connected) {
		lepton_connect();
	}
	auxTempPtr = (LEP_SYS_AUX_TEMPERATURE_CELCIUS_T_PTR)malloc(sizeof(LEP_SYS_AUX_TEMPERATURE_CELCIUS_T));
	LEP_GetSysAuxTemperatureCelcius(&_port, auxTempPtr);
	std::cout << *auxTempPtr << '\n';
}

//presumably more commands could go here if desired
