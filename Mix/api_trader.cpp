#include "stdafx.h"
#include "api_trader.h"
//#include "dataCollector.h"
#include <fstream>
#include <vector>
#include <string>
using namespace std;
#include "Strategy.h"
extern vector<Strategy* > allRunningTasks;
// Message for trader api

//#include "product.h"
//extern vector<product* > allProducts;

//#include "calculateCCI.h"
//extern calculateCCI calCCI;

//#include "dataCollectorDlg.h"
//#include "resource.h"
//extern CdataCollectorDlg* pDlg;

//////////////////////////////////////////Trader Spi//////////////////////////////////////

void CTraderSpi::OnFrontConnected()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, BROKER_ID);
	strcpy_s(req.UserID, INVESTOR_ID);
	strcpy_s(req.Password, PASSWORD);

	int iResult = pTraderApi->ReqUserLogin(&req, ++RequestID);
	if (iResult == 0)
	{
		cout << "---- Trader Login Request: Success" << endl;
	}
	else
	{
		cout << "---- Trader Login Request: Success " << iResult << endl;
	}
	tradErrorLog.open("tradErrorLog.txt", ios::app);
}

void CTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		cout << "---- Trader Login        : Success" << endl;
		cout << "---- Broker ID           : " << BROKER_ID << endl;
		cout << "---- Investor ID         : " << INVESTOR_ID << endl;

		// save communication info
		FRONT_ID = pRspUserLogin->FrontID;
		SESSION_ID = pRspUserLogin->SessionID;
		nOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		nOrderRef++;

		///settlement confirm
		CThostFtdcSettlementInfoConfirmField req;
		memset(&req, 0, sizeof(req));
		strcpy_s(req.BrokerID, BROKER_ID);
		strcpy_s(req.InvestorID, INVESTOR_ID);
		int iResult = pTraderApi->ReqSettlementInfoConfirm(&req, ++RequestID);
		if (iResult == 0)
		{
			cout << "---- Settlement Confirm  Request: Success" << endl;
		}
	}
}

void CTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo == 0)
	{
		cout << "---- Settlement Confirm         : Success" << endl;
	}
}

void CTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

	for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
	{
		if (strcmp(pInstrument->InstrumentID, (*it)->InstrumentID) == 0)
		{
			(*it)->tickPrice = pInstrument->PriceTick;
			(*it)->tickValue = pInstrument->VolumeMultiple;
			
			cout << "tickprice " << (*it)->tickPrice << " " << "tickvalue " << (*it)->tickValue << endl;
		}
	}

	if (pInstrument != NULL)
	{
		//for (unsigned int i = 0; i < allProducts.size(); i++)
		//{
		//	if (strcmp(pInstrument->ProductID, allProducts[i]->ProductID) == 0)
		//	{
		//		product::INSTRUMENTID temp2;
		//		strcpy(temp2.InstrumentID, pInstrument->InstrumentID);
		//		temp2.LastPrice = 0;
		//		temp2.OpenInterest = -1;
		//		allProducts[i]->allInstrumentID.push_back(temp2);
		//		allProducts[i]->PriceTick = pInstrument->PriceTick;

		//		char *ppInstrumentID2[1];
		//		ppInstrumentID2[0] = pInstrument->InstrumentID;
		//		pMdUserApi->SubscribeMarketData(ppInstrumentID2, 1);
		//	}
		//}
	}

	if (bIsLast)
	{
//		calCCI.isOn = true;
	}

}


void CTraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

void ReqQryInvestorPosition(char *FutureID)
{

}

void CTraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
	{
		//cout << "what is " << endl;
		//cout << " 持仓成本： " << pInvestorPosition->InstrumentID << " " << pInvestorPosition->Position << " " << pInvestorPosition->OpenCost << " END " << endl;
		//cout << " 开仓成本： " << pInvestorPosition->PositionCost << " " << pInvestorPosition->PosiDirection << " " << pInvestorPosition->TodayPosition << endl;


		//不用计算开仓成本，直接查询持仓成本即可，TA的保证金*2即是成交价
		//开仓买A，卖B，即算多仓；   开仓卖A，买B，即算空仓；

		if (strcmp(pInvestorPosition->InstrumentID, (*it)->InstrumentID) == 0)
		{

			//交易所会返回两次结果，所以要区分哪次是自己查的，那次是多余返回
			if (onoffbutton % 2 != 0)
			{

				if (pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long)
				{
					//(*it)->whole_longposition += pInvestorPosition->Position;
					(*it)->holdVolumeLong = pInvestorPosition->Position;
					(*it)->holdVolumeLong_average = (pInvestorPosition->PositionCost);
					//(*it)->cal_hold_volume.long_cost_sum;
				}

				if (pInvestorPosition->PosiDirection == THOST_FTDC_PD_Short)
				{
					//(*it)->whole_shortposition += pInvestorPosition->Position;
					(*it)->holdVolumeShort = pInvestorPosition->Position;
					(*it)->holdVolumeShort_average = (pInvestorPosition->PositionCost );

				}


				onoffbutton++;
				//cout << pInvestorPosition->InstrumentID << " " << pInvestorPosition->Position << " " << pInvestorPosition->PosiDirection << " " << pInvestorPosition->TodayPosition << endl;
				cout << "多仓：" << (*it)->holdVolumeLong << " 空仓： " << (*it)->holdVolumeShort << " ; " << (*it)->holdVolumeLong_average << " ;" << (*it)->holdVolumeShort_average << endl;

			}

			

		}



		//cout << "a: " << (*it)->openlong_a_num << " " << (*it)->openshort_a_num << " # b:" << (*it)->openlong_b_num << " " << (*it)->openshort_b_num << endl;

	}

}

void CTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
	{
		if (strcmp(pInputOrder->InstrumentID, (*it)->InstrumentID) == 0)
		{
			(*it)->RunOrderInsert(pInputOrder, pRspInfo);
		}
	}

	if (pRspInfo->ErrorID != 0)
	{
		SYSTEMTIME lt;
		GetLocalTime(&lt);
		tradErrorLog << "Insert Order  Error: " << pInputOrder->OrderRef << " " << pRspInfo->ErrorID << " " << pRspInfo->ErrorMsg << " " << lt.wHour << ": " << lt.wMinute << ": " << lt.wSecond << ": " << lt.wMilliseconds << endl;
		cout << "Insert Order  Error: " << pInputOrder->OrderRef << " " << pRspInfo->ErrorID << " " << pRspInfo->ErrorMsg << " " << lt.wHour << ": " << lt.wMinute << ": " << lt.wSecond << ": " << lt.wMilliseconds << endl;

	}
}

void CTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo->ErrorID != 0)
	{
		SYSTEMTIME lt;
		GetLocalTime(&lt);
		tradErrorLog << "cancle  Error: " << pInputOrderAction->InstrumentID << " " << pInputOrderAction->OrderSysID << " " << pRspInfo->ErrorID << " " << pRspInfo->ErrorMsg << " " << lt.wHour << ": " << lt.wMinute << ": " << lt.wSecond << ": " << lt.wMilliseconds << endl;
		cout << "cancle  Error: " << pInputOrderAction->InstrumentID << " " << pInputOrderAction->OrderSysID << " " << pRspInfo->ErrorID << " " << pRspInfo->ErrorMsg << " " << lt.wHour << ": " << lt.wMinute << ": " << lt.wSecond << ": " << lt.wMilliseconds << endl;
	}

}

///报单通知
void CTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
	{
		if (strcmp(pOrder->InstrumentID, (*it)->InstrumentID) == 0)
		{
			//(*it)->is_traded = true;						//成交
			(*it)->RunOrderReturn(pOrder);
		}
	}
}



///成交通知
void CTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
	{
		if (strcmp(pTrade->InstrumentID, (*it)->InstrumentID) == 0)
		{
			(*it)->is_traded = true;						//如果同时又有低频得单子成交，则会干扰
			(*it)->traded_price = pTrade->Price;
			(*it)->RunTradeReturn(pTrade);
		}
	}
}

void CTraderSpi::OnFrontDisconnected(int nReason)
{
	cerr << "----  Trader Front Disconnected: Reason = " << nReason << endl;
}

void CTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{

}

void CTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//IsErrorRspInfo(pRspInfo);
}

bool CTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cout << "---- Tradspi Error:" << pRspInfo->ErrorID << " " << pRspInfo->ErrorMsg << endl;
	return bResult;
}
