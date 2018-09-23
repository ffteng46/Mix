#include "stdafx.h"
#include "api_market.h"
//#include "dataCollector.h"
#include <fstream>
#include <vector>
#include "Strategy.h"

extern vector<Strategy* > allRunningTasks;

//#include "product.h"
//extern vector<product* > allProducts;

using namespace std;




void CMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	CMdSpi::IsErrorRspInfo(pRspInfo);
}


void CMdSpi::OnFrontDisconnected(int nReason)
{
	cout << "---- Market Quotes Front Disconnected:  " << nReason << endl;
}

void CMdSpi::OnHeartBeatWarning(int nTimeLapse)
{

}

void CMdSpi::OnFrontConnected()
{
	///user login for market quotes
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, BROKER_ID);
	strcpy_s(req.UserID, INVESTOR_ID);
	strcpy_s(req.Password, PASSWORD);
	//market login request
	int iResult1 = pMdUserApi->ReqUserLogin(&req, ++RequestID);
	if (iResult1 == 0)
	{
		cout << "---- Market Quotes Login Request: Success" << endl;
		for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
		{
			char *ppInstrumentID2[1];
			ppInstrumentID2[0] = (*it)->InstrumentID;
			pMdUserApi->SubscribeMarketData(ppInstrumentID2, 1);
		}
	}
	else
	{
		cout << "---- Market Quotes Login Request: Fail" << " " << iResult1 << endl;
	}

}

void CMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo->ErrorID == 0)
	{
		cout << "---- Market Quotes Login: Success" << endl;
	}
	else
	{
		CMdSpi::IsErrorRspInfo(pRspInfo);
	}
}




void CMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo->ErrorID == 0)
	{
		cout << "---- Subscribtion Success: " << pSpecificInstrument->InstrumentID << endl;
	}
	else
	{
		CMdSpi::IsErrorRspInfo(pRspInfo);
	}
}

void CMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo->ErrorID == 0)
	{
		cout << "---- UnSubscribtion Success: " << pSpecificInstrument->InstrumentID << endl;
	}
	else
	{
		CMdSpi::IsErrorRspInfo(pRspInfo);
	}
}

void CMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	//int h1, m1, s1;
	//sscanf(pDepthMarketData->UpdateTime, "%d:%d:%d", &h1, &m1, &s1);

	/*if (h1 >= 15 && h1 < 20)
	{
		return;
	}*/

	for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
	{
		if (strcmp(pDepthMarketData->InstrumentID, (*it)->InstrumentID)==0)
		{
			(*it)->RunMarketData(pDepthMarketData);
		}
	}

}

bool CMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cout << "---- Mdspi Error: " << pRspInfo->ErrorID << " " << pRspInfo->ErrorMsg << endl;
	return bResult;
}
