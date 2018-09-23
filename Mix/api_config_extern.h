#pragma once
#include "./ctpapi/ThostFtdcMdApi.h"
#include "./ctpapi/ThostFtdcTraderApi.h"
#include "EESTraderDemo.h"
#include "EESQuoteDemo.h"
#include <vector>
#include <iostream>

extern TraderDemo* ptradeApi;
extern CThostFtdcMdApi* pMdUserApi;

extern char MD_FRONT_ADDR[];		// market quotes connect front address
extern char TRADER_FRONT_ADDR[];	// trader connect front address

extern TThostFtdcBrokerIDType	BROKER_ID;
extern TThostFtdcInvestorIDType INVESTOR_ID;    //"78901067";// "80036929";
extern TThostFtdcPasswordType  PASSWORD;        //"888888";// "123456";
extern TThostFtdcUserIDType USER_ID;
extern TThostFtdcExchangeIDType EXCHANGE_ID;


extern char *ppInstrumentID[2];
extern char x[10];
extern char y[10];

extern int iInstrumentID;

extern int RequestID;
extern int iOrderRef;
extern int iRequestID;

extern TThostFtdcFrontIDType	FRONT_ID;
extern TThostFtdcSessionIDType	SESSION_ID;

