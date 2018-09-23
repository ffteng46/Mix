#pragma once
#include ".\ThostApi\ThostFtdcMdApi.h"
#include ".\ThostApi\ThostFtdcTraderApi.h"
#include <string>


CThostFtdcTraderApi* pTraderApi;
CThostFtdcMdApi* pMdUserApi;

int RequestID = 0;
int nOrderRef;
int nRequestID;


TThostFtdcBrokerIDType	BROKER_ID;
TThostFtdcInvestorIDType INVESTOR_ID;
TThostFtdcPasswordType  PASSWORD;
TThostFtdcUserIDType USER_ID;
TThostFtdcExchangeIDType EXCHANGE_ID;

TThostFtdcFrontIDType	FRONT_ID;
TThostFtdcSessionIDType	SESSION_ID;


char *ppInstrumentID[2];
int iInstrumentID;

char x[10];
char y[10];

char MD_FRONT_ADDR[30];		// market quotes connect front address
char TRADER_FRONT_ADDR[30] ;	// trader connect front address



   