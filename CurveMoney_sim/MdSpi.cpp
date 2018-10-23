#pragma comment (lib, "thosttraderapi.lib")
#define WIN32_LEAN_AND_MEAN     // 在#include<windows.h>前定义
#include "MdSpi.h"
#include "TimeProcesser.h"

//#include <windows.h>
#include "ctpapi/ThostFtdcTraderApi.h"
#include "TraderSpi.h"
#include "property.h"
#include <iostream>
#include <sstream>
#include <list>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <thread>
#include <mutex>
#include <glog/logging.h>
#include <boost/thread/recursive_mutex.hpp>
using namespace std;

#pragma warning(disable : 4996)
extern string systemID;//系统编号，每个产品一个编号
extern bool isLogout;
extern string tradingDayT;
//存放行情消息队列
extern list<string> mkdata;
//行情各字段分割符
extern string sep;
// USER_API参数
extern CThostFtdcMdApi* mdUserApi;
//连接到服务器端的客户数量
extern int customercount;
// 配置参数
extern char TRADE_FRONT_ADDR[];
extern TThostFtdcBrokerIDType	BROKER_ID;
extern TThostFtdcInvestorIDType INVESTOR_ID;
extern TThostFtdcPasswordType	PASSWORD;
// 会话参数
extern TThostFtdcFrontIDType	FRONT_ID;	//前置编号
extern TThostFtdcSessionIDType	SESSION_ID;	//会话编号
extern char** ppInstrumentID;
extern int iInstrumentID;
extern string hedgeFlag;//账号类型，组合投机套保标 投机 '1'套保 '3' 
extern int defaultVolume;//默认下单手数
extern unordered_map<string, vector<string>> instr_map;				//一个合约和哪些合约配对
//extern unordered_map<string, double> instr_price_map;					//合约的价格
extern unordered_map<string, PriceGap*> instr_price_gap;			//价格差
extern unordered_map<string, InstrumentInfo*> instruments;			//合约信息
extern unordered_map<string, list<TradeInfo*>*> alreadyTradeMapAS;			//保存已成交套利单信息,正套 
extern unordered_map<string, list<TradeInfo*>*> alreadyTradeMapDS;			//保存已成交套利单信息,反套
extern recursive_mutex g_lock_ti;//tradeinfo lock
extern int arbVolume;//当前持仓量
extern int arbVolumeMetric;//套利单总共能下多少手，单边
extern int maxUntradeNums;//最大未成交套利单笔数(非手数，手数=maxUntradeNums*defaultVolume)
extern int maxFollowTimes;//最大追单次数
unordered_map<string, MarketData*> marketdata_map;
extern boost::lockfree::queue<LogMsg*> networkTradeQueue;///报单、成交消息队列,发送到客户端线程使用
//交易对象
// UserApi对象
extern CThostFtdcTraderApi* ptradeApi;
extern CTraderSpi* pTradeUserSpi;
///日志消息队列
extern list<string> loglist;
// 请求编号
extern int iRequestID;
// 报单引用
extern int iOrderRef;
//触发套利单时，保存套利信息
unordered_map<string, list<TradeInfo*>*> willTradeMap;
//止盈报单
unordered_map<string, list<TradeInfo*>*> stopProfitWillTradeMap;
void decompose();
void notActiveImmediateTrade(CThostFtdcDepthMarketDataField *pDepthMarketData);
//double* priceDefine(MarketData* lastMarketData, MarketData* forwardMarketData, double preSetGap, bool isLastActive);

double* isImmediateTrade(MarketData* lastMarketData, MarketData* forwardMarketData, PriceGap* preSetGapInfo, bool isLastActive);

extern int start_process;
extern boost::recursive_mutex pst_mtx;
//套利组合单列表锁
extern boost::recursive_mutex willTrade_mtx;
extern boost::recursive_mutex unique_mtx;//unique lock
//盈利组合单列表锁
extern boost::recursive_mutex stopProfit_mtx;
//行情锁
extern boost::recursive_mutex mkdata_mtx;
//等待止盈套利组合单列表锁
extern boost::recursive_mutex alreadyTrade_mtx;
extern int long_offset_flag;
extern int short_offset_flag;
//买平标志,1开仓；2平仓
extern int longPstIsClose;
extern int shortPstIsClose;
//extern unordered_map<string, unordered_map<string, int>> positionmap;
extern unordered_map<string, HoldPositionInfo*> positionmap;
extern boost::lockfree::queue<PriceGapMarketData*> detectOpenQueue;///开仓列表
extern boost::lockfree::queue<PriceGapMarketData*> detectCloseQueue;///止盈列表
extern boost::lockfree::queue<MarketData*> techMetricQueue;///技术指标处理列表
void CMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
	int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	IsErrorRspInfo(pRspInfo);
}

void CMdSpi::OnFrontDisconnected(int nReason)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	cerr << "--->>> Reason = " << nReason << endl;
}

void CMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void CMdSpi::OnFrontConnected()
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	///用户登录请求
	ReqUserLogin();
}

void CMdSpi::ReqUserLogin()
{
	cout << "market" + boost::lexical_cast<string>(PthreadSelf());
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.UserID, INVESTOR_ID);
	strcpy(req.Password, PASSWORD);
	int iResult = mdUserApi->ReqUserLogin(&req, ++iRequestID);
    cerr << "market--->>> 发送用户登录请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
	//
	// 初始化UserApi
	/*
	ptradeApi = CThostFtdcTraderApi::CreateFtdcTraderApi();			// 创建UserApi
	pTradeUserSpi = new CTraderSpi();
	ptradeApi->RegisterSpi((CThostFtdcTraderSpi*)pTradeUserSpi);			// 注册事件类
	ptradeApi->SubscribePublicTopic(TERT_RESUME);					// 注册公有流
	ptradeApi->SubscribePrivateTopic(TERT_RESUME);					// 注册私有流
	ptradeApi->RegisterFront(TRADE_FRONT_ADDR);
	ptradeApi->Init();
	*/
}

void CMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << __FUNCTION__ << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///获取当前交易日
		cerr << "--->>> 获取当前交易日 = " << mdUserApi->GetTradingDay() << endl;
		// 请求订阅行情
		while (start_process == 0) {
            boost::this_thread::sleep(boost::posix_time::seconds(1));    // 这种更好用
            boost::this_thread::yield();
            //cout<<"start_process="<<start_process<<endl;
            //Sleep(10);
		}
		SubscribeMarketData();
	}
}

void CMdSpi::SubscribeMarketData()
{
	int iResult =mdUserApi->SubscribeMarketData(ppInstrumentID, iInstrumentID);
	cerr << "--->>> 发送行情订阅请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void CMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "=========================>hello" << pSpecificInstrument->InstrumentID << endl;
	string mkinfo;
	mkinfo.append("InstrumentID=");
	mkinfo.append(pSpecificInstrument->InstrumentID);
	cerr << __FUNCTION__ << endl;
}

void CMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << __FUNCTION__ << endl;
}
void decompose() {
	string mkprice;
	//找到对应合约的行情价格
	for (unordered_map<string, MarketData*>::iterator instr_price_map_it = marketdata_map.begin(); instr_price_map_it != marketdata_map.end(); instr_price_map_it++) {
		string key = instr_price_map_it->first;
		MarketData *pd_market = instr_price_map_it->second;
		string instrumentid = pd_market->instrumentID;
		double lastPrice = pd_market->lastPrice;
		int volume = pd_market->volume;
		double bidprice = pd_market->bidPrice;
		double askprice = pd_market->askPrice;
		string tmpstr = "key=" + key + ";instrumentID=" + instrumentid + ";volume=" + boost::lexical_cast<string>(volume) + ";lastprice=" + boost::lexical_cast<string>(lastPrice) +
			";bidprice=" + boost::lexical_cast<string>(bidprice) + ";askprice=" + boost::lexical_cast<string>(askprice);
		mkprice += tmpstr + ";";
	}
	cout << mkprice << endl;
	LOG(INFO)<<"行情" + (mkprice);
	//配对合约预警值
	string gap;
	for (unordered_map<string, PriceGap*>::iterator tmpit = instr_price_gap.begin(); tmpit != instr_price_gap.end(); tmpit++) {
		string key = tmpit->first;
		PriceGap* price_gap = tmpit->second;
		gap += "key=" + key + ";gap=(" + boost::lexical_cast<string>(price_gap->minGap) + "," + boost::lexical_cast<string>(price_gap->maxGap) +"),closeGap="+boost::lexical_cast<string>(price_gap->profitGap ) + ";";
	}
	cout << gap << endl;
	LOG(INFO)<<(gap);
	//下单情况
	string orderinfo = getAllUntradeInfo();
	LOG(INFO) << ("当前正在处理的套利单情况;" + orderinfo);
}
void CMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	if (start_process == 0) {
		return;
	}
	//开始时间
	boost::posix_time::ptime startTime = getCurrentTimeByBoost();
	//lock_guard<recursive_mutex> locker(g_lock_ti);
    //decompose();
	//4 she 5 ru
    unsigned char buf[20]={'\0'};
    memset(buf,0,20);
    if (abs(pDepthMarketData->AskPrice1) > 10000000000 || abs(pDepthMarketData->LastPrice) > 10000000000 ) {
        LOG(ERROR) << (boost::lexical_cast<string>(pDepthMarketData->InstrumentID) + "初始化行情出现问题;");
        return;
    }
    if (pDepthMarketData->InstrumentID == NULL||pDepthMarketData->BidPrice1 == NULL) {

        LOG(ERROR) << (boost::lexical_cast<string>(pDepthMarketData->InstrumentID) +"行情数据为空！！！！！");
        return;
    }
    //4she5ru
    int a = (pDepthMarketData->LastPrice + 0.005)*100;
    pDepthMarketData->LastPrice = a/100;
    a = (pDepthMarketData->BidPrice1 + 0.005)*100;
    pDepthMarketData->BidPrice1 = a/100;
    a = (pDepthMarketData->AskPrice1 + 0.005)*100;
    pDepthMarketData->AskPrice1 = a/100;
	//string str_mkdata = "businessType=100;"
	//mkdata.push_back(marketdata);
	//cerr << __FUNCTION__ << endl;
	//cerr <<"---------------------------------->>"<<marketdata<<endl;
    boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
    //先看是否能市价下单
    {
        //cout << "====================1=================";
        //交易逻辑
        //notActiveImmediateTrade(pDepthMarketData);
        //cout << "====================2=================";
        //是否追单
        processFollow(pDepthMarketData);
//		cout << "=====================================";
    }
    string instrumentID = string(pDepthMarketData->InstrumentID);
    double currentPrice = pDepthMarketData->LastPrice;
    //instr_price_map[pDepthMarketData->InstrumentID] = currentPrice;
    int volume = pDepthMarketData->Volume;//成交量
    MarketData* marketdatainfo = new MarketData();//保存当前行情数据
    marketdatainfo->updateTime = string(pDepthMarketData->UpdateTime);
    marketdatainfo->instrumentID = instrumentID;
    marketdatainfo->volume = volume;
    marketdatainfo->bidPrice = pDepthMarketData->BidPrice1;
    marketdatainfo->askPrice = pDepthMarketData->AskPrice1;
    marketdatainfo->lastPrice = currentPrice;
    marketdatainfo->lowestPrice;
    marketdatainfo->highestPrice;
    marketdatainfo->turnover = pDepthMarketData->Turnover;
    //cout << "############";
    {
        //boost::recursive_mutex::scoped_lock SLock3(mkdata_mtx);
        marketdata_map[instrumentID] = marketdatainfo;
    }
    //detectOpenQueue.push(marketdatainfo);
    //detectCloseQueue.push(marketdatainfo);
    //replace queue for metricProcesserForSingleThread
    //techMetricQueue.push(marketdatainfo);
    metricProcesserForSingleThread(marketdatainfo);\
    //结束时间
    boost::posix_time::ptime endTime = getCurrentTimeByBoost();
    int seconds = getTimeInterval(startTime, endTime, "t");
    if(isLogout){
        LOG(INFO) << instrumentID +",OnRtnDepthMarketData,处理时长=" + boost::lexical_cast<string>(seconds);
    }
    string stg = "businessType=wtm_4;tradingDay=" + tradingDayT
        + ";" + ";processTime=" + boost::lexical_cast<string>(seconds) + ";type=onrtnMarketData;seq=" + boost::lexical_cast<string>(endTime)+";systemID=" + systemID;
    LogMsg *logmsg = new LogMsg();
    logmsg->setMsg(stg);
    networkTradeQueue.push(logmsg);
}

/*定义是否有价差，以及价差买卖,
lastMarketData:近月合约
forwardMarketData:远月合约
preSetGapInfo:预设的价差信息
isLastActive:近月合约是否活跃，true：活跃
返回值：返回有三个值得数值数组[one,two,thr]
	one:近月合约的下单价格
	two:远月合约的下单价格
	thr:价格判断情况，根据以下定义判断如何下单
		0，价差未达到，不能下单
		10,可以普通价格套利下单；
		11，可以市价套利下单；
		20,价差回归之后，市价平仓止盈离场
		21,价差回归之后，快速平仓止盈离场
		22,价差回归之后，普通价格平仓止盈离场
*/
MarketPriceAction* priceDefine(MarketData* lastMarketData,MarketData* forwardMarketData,PriceGap* preSetGapInfo,bool isLastActive) {
	double* temp = new double[3];
	MarketPriceAction* mpa = new MarketPriceAction();
	temp[0] = 0;
	temp[1] = 0;
	temp[2] = 0;
	//近月
	double currLastPrice = lastMarketData->lastPrice;
	double currBidPrice = lastMarketData->bidPrice;
	double currAskPrice = lastMarketData->askPrice;
	int currVolume = lastMarketData->volume;
	double currPriceTick = lastMarketData->priceTick;
	string currInstrumentID = lastMarketData->instrumentID;
	string cstr = getMarketData(lastMarketData);
	//远月
	double f_lastPrice = forwardMarketData->lastPrice;
	double f_bidPrice = forwardMarketData->bidPrice;
	double f_askPrice = forwardMarketData->askPrice;
	int f_volume = forwardMarketData->volume;
	double f_priceTick = forwardMarketData->priceTick;
	string f_instrumentID = forwardMarketData->instrumentID;
	string fstr = getMarketData(forwardMarketData);
	//预设价差
	double openMinGap = preSetGapInfo->minGap;//-20
	double openMaxGap = preSetGapInfo->maxGap;//-15
	double closeGap = preSetGapInfo->profitGap;//-10
	string insComKey = getComInstrumentKey(currInstrumentID,f_instrumentID);
	string datainfo = cstr + ";" + fstr;
	//价格形成机制：买入近月合约，卖出远月合约，正套
	if (isLastActive) {//近月合约为活跃
		double gap1 = currAskPrice - f_bidPrice;//市价可以成交gap
		double midprice = round((f_bidPrice + f_askPrice) / 2/f_priceTick) * f_priceTick;
		//double gap2 = currBidPrice - f_bidPrice;//普通gap,修改成不活跃合约必须对价成交
		double gap2 = currLastPrice - midprice;//普通gap
		//平仓合约价格:卖出近月合约，买入远月合约(远月不活跃,先交易)
		double closeGap1 = currBidPrice - f_askPrice;//市价可以成交
		double closeGap2 = currAskPrice - f_askPrice;//市价可以成交
		/*
		double closeGap3 = currBidPrice - midprice;//快速成交
		double closeGap4 = currAskPrice - midprice;//快速成交
		double closeGap5 = currBidPrice - f_bidPrice;//不活跃可以快速成交
		double closeGap6 = currAskPrice - f_bidPrice;//都不能快速成交
		*/
		//gap1较大，gap2较小
		if (gap1 <= openMaxGap && gap1 >= openMinGap) {
			LOG(INFO) << "近月合约:" + currInstrumentID + "为活跃,市价可以成交,设置市价成交套利组合,gap=" + boost::lexical_cast<string>(gap1) + ",阈值为" + boost::lexical_cast<string>(openMaxGap) + "到" + boost::lexical_cast<string>(openMinGap)
				+ ";" + datainfo;
			temp[0] = currAskPrice;
			temp[1] = f_bidPrice;
			temp[2] = 11;
			mpa->lastInstrInsertPrice = currAskPrice;
			mpa->forwardInstrInsertPrice = f_bidPrice;
			mpa->hopeGap = gap1;
			mpa->orderType = "11";
			return mpa;
		}/*
		else if (gap2 <= openMaxGap && gap2 >= openMinGap) {
			LOG(INFO) << "近月合约:" + currInstrumentID + "为活跃,普通价差可以成交,设置普通价差成交套利组合.gap=" + boost::lexical_cast<string>(gap2) + ",阈值为" + boost::lexical_cast<string>(openMaxGap) + "到" + boost::lexical_cast<string>(openMinGap) + ";" + datainfo;
			temp[0] = currBidPrice;
			temp[1] = f_bidPrice;
			temp[2] = 10;
			return temp;
		}*/
		//以下为盈利离场///////////////////
		//第二次修改时，需要市价成交才下单
		//第三次修改成，每次独立判断，市价成交才下单
		unordered_map<string, list<TradeInfo*>*>::iterator it = alreadyTradeMapAS.find(insComKey);
		if (it != alreadyTradeMapAS.end()) {
			list<TradeInfo*>* tradeList = it->second;
			for (list<TradeInfo*>::iterator tradeIt = tradeList->begin(); tradeIt != tradeList->end();tradeIt++) {
				TradeInfo* tmpTradeInfo = *tradeIt;
				double hopeCloseGap = tmpTradeInfo->hopeCloseGap;
				if (closeGap1 >= hopeCloseGap) {//套利回归，盈利立场（负数,绝对值小）
					LOG(INFO) << "止盈离场：近月合约:" + currInstrumentID + "为活跃,市价双边可以成交,市价Gap=" + boost::lexical_cast<string>(closeGap1) + ",阈值为" + boost::lexical_cast<string>(hopeCloseGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
					temp[0] = currBidPrice;
					temp[1] = f_askPrice;
					temp[2] = 20;
					if (tmpTradeInfo->stopProfitStatus == "200") {//刚发送追单中的撤单请求=100；撤单中(接受到撤单回报=5)；追单已经成交=0时，不能继续撤单
						LOG(INFO) << "虽市价触发,此组合状态 stopProfitStatus=" + boost::lexical_cast<string>(tmpTradeInfo->stopProfitStatus) + ",已处理，不能下单。";
						continue;
					} else {
						tmpTradeInfo->stopProfitStatus == "200";//设置状态为已处理
						LOG(INFO) << "设置套利组合状态为已处理";
					}
					mpa->lastInstrInsertPrice = currBidPrice;
					mpa->forwardInstrInsertPrice = f_askPrice;
					mpa->orderType = "20";
					mpa->hopeGap = closeGap1;
					mpa->happyID = tmpTradeInfo->happyID;
					return mpa;
				}
			}
		}
		
		/*3
		if (closeGap1 >= closeGap) {//套利回归，盈利立场
			LOG(INFO) << "止盈离场：近月合约:" + currInstrumentID + "为活跃,市价双边可以成交,gap=" + boost::lexical_cast<string>(closeGap1) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = currBidPrice;
			temp[1] = f_askPrice;
			temp[2] = 20;
			return temp;
		}*/
		/*2
		if (closeGap2 >= closeGap ) {
			LOG(INFO) << "止盈离场：近月合约:" + currInstrumentID + "为活跃,市价可以成交,其中不活跃合约可以立即成交.gap=" + boost::lexical_cast<string>(closeGap2) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = currAskPrice;
			temp[1] = f_askPrice;
			temp[2] = 20;
			return temp;
		}
		
		if (closeGap3 >= closeGap) {
			LOG(INFO) << "止盈离场：近月合约:" + currInstrumentID + "为活跃,普通价差可以成交,其中不活跃合约取中间价.gap=" + boost::lexical_cast<string>(closeGap3) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = currBidPrice;
			temp[1] = midprice;
			temp[2] = 21;
			return temp;
		}
		if (closeGap4 >= closeGap) {
			LOG(INFO) << "止盈离场：近月合约:" + currInstrumentID + "为活跃,普通价差可以成交,双边需要等待成交.gap=" + boost::lexical_cast<string>(closeGap4) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = currAskPrice;
			temp[1] = midprice;
			temp[2] = 21;
			return temp;
		}
		if (closeGap5 >= closeGap ) {
			LOG(INFO) << "止盈离场：近月合约:" + currInstrumentID + "为活跃,普通价差可以成交,不活跃可以快速成交.gap=" + boost::lexical_cast<string>(closeGap5) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = currBidPrice;
			temp[1] = f_bidPrice;
			temp[2] = 22;
			return temp;
		}
		if (closeGap6 >= closeGap ) {
			LOG(INFO) << "止盈离场：近月合约:" + currInstrumentID + "为活跃,普通价差可以成交,双边需要等待成交.gap=" + boost::lexical_cast<string>(closeGap6) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = currAskPrice;
			temp[1] = f_bidPrice;
			temp[2] = 22;
			return temp;
		}*/
	}else {//远月为活跃合约
		double gap1 = currAskPrice - f_bidPrice;//市价可以成交gap
		//double midprice = round((currBidPrice + currAskPrice) / 2/currPriceTick)*currPriceTick;
		double gap2 = currAskPrice - f_askPrice;//普通gap
		//平仓合约价格:卖出近月合约，买入远月合约(近月不活跃,先交易)
		double closeGap1 = currBidPrice - f_askPrice;//市价可以成交
		double closeGap2 = currBidPrice - f_bidPrice;//市价可以成交
		/*
		double closeGap3 = midprice -f_askPrice;//快速成交
		double closeGap4 = midprice - f_bidPrice;//快速成交
		double closeGap5 = currAskPrice - f_askPrice;//不活跃可以快速成交
		double closeGap6 = currAskPrice - f_bidPrice;//都不能快速成交
		*/
		if (gap1 <= openMaxGap && gap1 >= openMinGap) {
			LOG(INFO) << "远月合约:" + f_instrumentID + "为活跃,市价可以成交,设置市价成交套利组合.gap=" + boost::lexical_cast<string>(gap1) + ",阈值为" + boost::lexical_cast<string>(openMaxGap) + "到" + boost::lexical_cast<string>(openMinGap) + ";" + datainfo;
			temp[0] = currAskPrice;
			temp[1] = f_bidPrice;
			temp[2] = 11;
			mpa->lastInstrInsertPrice = currAskPrice;
			mpa->forwardInstrInsertPrice = f_bidPrice;
			mpa->hopeGap = gap1;
			mpa->orderType = "11";
			return mpa;
		}/*
		else if (gap2 <= openMaxGap && gap2 >= openMinGap) {
			LOG(INFO) << "远月合约:" + f_instrumentID + "为活跃,普通价差可以成交,设置普通价差成交套利组合.gap=" + boost::lexical_cast<string>(gap2) + ",阈值为" + boost::lexical_cast<string>(openMaxGap) + "到" + boost::lexical_cast<string>(openMinGap) + ";" + datainfo;
			temp[0] = currAskPrice;
			temp[1] = f_askPrice;
			temp[2] = 10;
			return temp;
		}*/
		//以下为止盈离场
		//第二次修改时，需要市价成交才下单
		//第三次修改成，每次独立判断，市价成交才下单
		unordered_map<string, list<TradeInfo*>*>::iterator it = alreadyTradeMapAS.find(insComKey);
		if (it != alreadyTradeMapAS.end()) {
			list<TradeInfo*>* tradeList = it->second;
			for (list<TradeInfo*>::iterator tradeIt = tradeList->begin(); tradeIt != tradeList->end();tradeIt++) {
				TradeInfo* tmpTradeInfo = *tradeIt;
				double hopeCloseGap = tmpTradeInfo->hopeCloseGap;
				if (closeGap1 >= hopeCloseGap) {//套利回归，盈利立场（负数,绝对值小）
					//LOG(INFO) << "止盈离场：近月合约:" + currInstrumentID + "为活跃,市价双边可以成交,市价Gap=" + boost::lexical_cast<string>(closeGap1) + ",阈值为" + boost::lexical_cast<string>(hopeCloseGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
					LOG(INFO) << "止盈离场:远月合约:" + f_instrumentID + "为活跃,市价可以成交,设置市价成交止盈组合.市价gap=" + boost::lexical_cast<string>(closeGap1) + ",阈值为" + boost::lexical_cast<string>(hopeCloseGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
					temp[0] = currBidPrice;
					temp[1] = f_askPrice;
					temp[2] = 20;
					if (tmpTradeInfo->stopProfitStatus == "200") {//
						LOG(INFO) << "虽市价触发,此组合状态 stopProfitStatus=" + boost::lexical_cast<string>(tmpTradeInfo->stopProfitStatus) + ",已处理，不能下单。";
						continue;
					} else {
						tmpTradeInfo->stopProfitStatus == "200";//设置状态为已处理
						LOG(INFO) << "设置套利组合状态为已处理";
					}
					mpa->lastInstrInsertPrice = currBidPrice;
					mpa->forwardInstrInsertPrice = f_askPrice;
					mpa->hopeGap = closeGap1;
					mpa->orderType = "20";
					mpa->happyID = tmpTradeInfo->happyID;
					return mpa;
				}
			}
		}
		/*
		if(closeGap1 >= closeGap ) {
			LOG(INFO) << "止盈离场:远月合约:" + f_instrumentID + "为活跃,市价可以成交,设置市价成交止盈组合.gap=" + boost::lexical_cast<string>(closeGap1) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = currBidPrice;
			temp[1] = f_askPrice;
			temp[2] = 20;
			mpa->lastInstrInsertPrice = currBidPrice;
			mpa->forwardInstrInsertPrice = f_askPrice;
			mpa->orderType = "20";
			return mpa;
		}*/
		/*
		if (closeGap2 >= closeGap) {
			LOG(INFO) << "止盈离场:远月合约:" + f_instrumentID + "为活跃,市价可以成交,其中不活跃合约可以立即成交.gap=" + boost::lexical_cast<string>(closeGap2) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = currBidPrice;
			temp[1] = f_bidPrice;
			temp[2] = 20;
			return temp;
		}
		
		if (closeGap3 >= closeGap ) {
			LOG(INFO) << "止盈离场:远月合约:" + f_instrumentID + "为活跃,普通价差可以成交,其中不活跃合约取中间价.gap=" + boost::lexical_cast<string>(closeGap3) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = midprice;
			temp[1] = f_askPrice;
			temp[2] = 21;
			return temp;
		}
		if (closeGap4 >= closeGap ) {
			LOG(INFO) << "止盈离场:远月合约:" + f_instrumentID + "为活跃,普通价差可以成交,双边需要等待成交.gap=" + boost::lexical_cast<string>(closeGap4) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = midprice;
			temp[1] = f_bidPrice;
			temp[2] = 21;
			return temp;
		}
		if (closeGap5 >= closeGap ) {
			LOG(INFO) << "止盈离场:远月合约:" + f_instrumentID + "为活跃,普通价差可以成交,不活跃可以快速成交.gap=" + boost::lexical_cast<string>(closeGap5) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = currAskPrice;
			temp[1] = f_askPrice;
			temp[2] = 22;
			return temp;
		}
		if (closeGap6 >= closeGap ) {
			LOG(INFO) << "止盈离场:远月合约:" + f_instrumentID + "为活跃,普通价差可以成交,双边需要等待成交.gap=" + boost::lexical_cast<string>(closeGap6) + ",阈值为" + boost::lexical_cast<string>(closeGap) + ";" + datainfo + ";套利单数量:" + boost::lexical_cast<string>(arbVolume);
			temp[0] = currAskPrice;
			temp[1] = f_bidPrice;
			temp[2] = 22;
			return temp;
		}*/
	}
	return mpa;

}
/*判断是否可以市价成交一组套利单,
lastMarketData:近月合约
forwardMarketData:远月合约
isLastActive:近月合约是否活跃，true：活跃
返回值：1：可以市价成交
2：远月合约的下单价格
3:1,可以套利下单；0，价差未达到，不能下单;2,价差回归之后，平仓止盈离场
0:不能市价成交 
*/
double* isImmediateTrade(MarketData* lastMarketData, MarketData* forwardMarketData, PriceGap* preSetGapInfo, bool isLastActive) {
	double* temp = new double[3];
	temp[0] = 0;
	temp[1] = 0;
	temp[2] = 0;
	//近月
	double currLastPrice = lastMarketData->lastPrice;
	double currBidPrice = lastMarketData->bidPrice;
	double currAskPrice = lastMarketData->askPrice;
	int currVolume = lastMarketData->volume;
	double currPriceTick = lastMarketData->priceTick;
	string currInstrumentID = lastMarketData->instrumentID;
	//远月
	double f_lastPrice = forwardMarketData->lastPrice;
	double f_bidPrice = forwardMarketData->bidPrice;
	double f_askPrice = forwardMarketData->askPrice;
	int f_volume = forwardMarketData->volume;
	double f_priceTick = forwardMarketData->priceTick;
	string f_instrumentID = forwardMarketData->instrumentID;
	//预设价差
	double openMinGap = preSetGapInfo->minGap;//-20
	double openMaxGap = preSetGapInfo->maxGap;//-15
	double closeGap = preSetGapInfo->profitGap;//-10
											   //价格形成机制：买入近月合约，卖出远月合约，正套
	if (isLastActive) {//近月合约为活跃
		double gap1 = currAskPrice - f_bidPrice;//市价可以成交gap
		if (gap1 <= openMaxGap && gap1 >= openMinGap) {
			LOG(INFO) << "近月合约:" + currInstrumentID + "为活跃,市价可以成交,设置市价成交套利组合,gap=" + boost::lexical_cast<string>(gap1) + ",阈值为" + boost::lexical_cast<string>(openMaxGap) + "到" + boost::lexical_cast<string>(openMinGap);
			temp[0] = currAskPrice;
			temp[1] = f_bidPrice;
			temp[2] = 1;
			return temp;
		}
	}
	else {//远月为活跃合约
		double gap1 = currAskPrice - f_bidPrice;//市价可以成交gap
		double midprice = round((currBidPrice + currAskPrice) / 2 / currPriceTick)*currPriceTick;
		double gap2 = midprice - f_lastPrice;//普通gap
											 //平仓合约价格:卖出近月合约，买入远月合约(近月不活跃,先交易)
		if (gap1 <= openMaxGap && gap1 >= openMinGap) {
			LOG(INFO) << "远月合约:" + f_instrumentID + "为活跃,市价可以成交,设置市价成交套利组合.gap=" + boost::lexical_cast<string>(gap1) + ",阈值为" + boost::lexical_cast<string>(openMaxGap) + "到" + boost::lexical_cast<string>(openMinGap);
			temp[0] = currAskPrice;
			temp[1] = f_bidPrice;
			temp[2] = 1;
			return temp;
		}
	}
	return temp;

}

bool CMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	return bResult;
}



