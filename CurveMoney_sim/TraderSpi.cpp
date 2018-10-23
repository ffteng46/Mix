#include <iostream>
#include "property.h"
#include <vector>
#include <unordered_map>
using namespace std;
#include <boost/lexical_cast.hpp>
#include "ctpapi/ThostFtdcUserApiDataType.h"
#include "ctpapi/ThostFtdcMdApi.h"
#include "MdSpi.h"
#include "TraderSpi.h"
#include <glog/logging.h>
#include <boost/thread/recursive_mutex.hpp>
#pragma warning(disable : 4996)
list<string> tradequeue;
///日志消息队列
//extern list<string> loglist;
extern boost::thread_group thread_log_group;
extern boost::lockfree::queue<LogMsg*> logqueue;///日志消息队列
extern CThostFtdcMdApi* mdUserApi;
// USER_API参数
extern CThostFtdcTraderApi* ptradeApi;
extern int notActiveInsertAmount;//不活跃合约重复下单次数
extern int orderInsertInterval;//不活跃合约重复下单次数

extern unordered_map<string, PriceGap*> instr_price_gap;			//价格差
							   // 配置参数
extern char MARKET_FRONT_ADDR[];
extern char TRADE_FRONT_ADDR[];		// 前置地址
extern char BROKER_ID[];		// 经纪公司代码
extern char INVESTOR_ID[];		// 投资者代码
extern char PASSWORD[];			// 用户密码
char INSTRUMENT_ID[20];	// 合约代码
extern TThostFtdcPriceType	LIMIT_PRICE;	// 价格
extern TThostFtdcDirectionType	DIRECTION;	// 买卖方向
extern char BROKER_ID_1[];		// 经纪公司代码
extern char INVESTOR_ID_1[];		// 投资者代码
extern char PASSWORD_1[];			// 用户密码
extern char typ;
extern vector<string> quoteList;	//合约列表
									// 请求编号
extern int iRequestID;
//报单引用
extern int iOrderRef;
//extern unordered_map<string, TradeInfo*> willTradeMap;
// 会话参数
TThostFtdcFrontIDType	FRONT_ID;	//前置编号
TThostFtdcSessionIDType	SESSION_ID;	//会话编号
TThostFtdcOrderRefType	ORDER_REF;	//报单引用
									//持仓是否已经写入定义字段
bool isPositionDefFieldReady = false;
//成交文件是否已经写入定义字段
bool isTradeDefFieldReady = false;
//用户对冲报单文件是否已经写入定义字段
bool isOrderInsertDefFieldReady = false;
//将成交信息组装成对冲报单
CThostFtdcInputOrderField assamble(CThostFtdcTradeField *pTrade);
////将投资者对冲报单信息写入文件保存
void saveInvestorOrderInsertHedge(CThostFtdcInputOrderField *order, string filepath);
//保存报单回报信息
void saveRspOrderInsertInfo(CThostFtdcInputOrderField *pInputOrder);
//提取投资者报单信息
string getInvestorOrderInsertInfo(CThostFtdcInputOrderField *order);
//以分隔符方式记录投资者报单委托信息
string getInvestorOrderInsertInfoByDelimater(CThostFtdcInputOrderField *order);
//提取投资者报单信息
string getOrderActionInfoByDelimater(CThostFtdcInputOrderActionField *order);
//提取委托回报信息
string getRtnOrderInfoByDelimater(CThostFtdcOrderField *order);
//成交明细
string getPositionDetail(CThostFtdcInvestorPositionDetailField  *pInvestorPosition);
//提取成交回报信息
string getRtnTradeInfoByDelimater(CThostFtdcTradeField *order);
//将交易所报单回报响应写入文件保存
void saveRtnOrder(CThostFtdcOrderField *pOrder);
//获取交易所响应信息
string getRtnOrder(CThostFtdcOrderField *pOrder);

int storeInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition);
void initpst(CThostFtdcInvestorPositionField *pInvestorPosition);
CThostFtdcRspUserLoginField * loginRef;
extern string tradingDayT;//2010-01-01
//高频参数
//unordered_map<string, unordered_map<string, int>> positionmap;
extern unordered_map<string, HoldPositionInfo*> positionmap;
extern unordered_map<string, list<HoldPositionDetail*>*> positionDetailMap;//持仓明细
extern unordered_map<string, TechMetric*> techMetricMap;
//extern list<HoldPositionDetail*> positionDetailMap;//持仓明细
extern unordered_map<string, vector<string>> instr_map;				//一个合约和哪些合约配对
unordered_map<string, unordered_map<string, int64_t>> seq_map_orderref;
extern boost::lockfree::queue<LogMsg*> networkTradeQueue;///报单、成交消息队列,网络通讯使用
unordered_map<string, string> seq_map_ordersysid;
extern boost::recursive_mutex pstDetail_mtx;//持仓明细
extern boost::recursive_mutex queryPst_mtx;//查询各种持仓
extern boost::recursive_mutex pst_mtx;
//orderinsertkey与ordersysid对应关系锁
extern boost::recursive_mutex order_mtx;
//套利组合单列表锁
extern boost::recursive_mutex willTrade_mtx;
//盈利组合单列表锁
extern boost::recursive_mutex stopProfit_mtx;
//action order lock
extern boost::recursive_mutex actionOrderMTX;
//等待止盈套利组合单列表锁
extern boost::recursive_mutex alreadyTrade_mtx;
extern boost::recursive_mutex priceGap_mtx;//查询组合持仓的gap		
//extern boost::recursive_mutex arbVolume_mtx;//套利组合单数量的gap	
extern int isTwoStartStrategy;//等于2的时候，表示明细和汇总持仓查询完毕，启动系统
int isbegin = 0;//是否启动策略
int realLongPstLimit = 0;
int realShortPstLimit = 0;
int lastABSSpread = 0;
int firstGap = 2;
int secondGap = 5;
int longpstlimit;
//shortpstlimit
int shortpstlimit;
//记录时间 
int long_offset_flag;
int short_offset_flag;
//买平标志,1开仓；2平仓
//有空头持仓，可以执行"买平仓"
int longPstIsClose;
//有多头持仓，可以执行"卖平仓"
int shortPstIsClose;
int start_process = 0;
extern int arbVolume;//当前持仓量
extern int arbVolumeMetric;//套利单总共能下多少手，单边
///多个字段组合时使用的分隔符
string sep = ";";
char tradingDay[12] = { "\0" };
void CTraderSpi::OnFrontConnected()
{
	cerr << "--->>> " << "OnFrontConnected" << endl;
	///用户登录请求
    ReqUserLogin();
}
int CTraderSpi::getMaxOrderRef() {
	int iNextOrderRef = atoi(loginRef->MaxOrderRef);
	return iNextOrderRef;
}
void CTraderSpi::ReqUserLogin()
{
	LOG(INFO) << "trade" + boost::lexical_cast<string>(PthreadSelf());
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
    cout << "经纪公司代码=" << BROKER_ID <<",size="<<strlen(BROKER_ID) <<endl;
    cout << "投资者代码=" << INVESTOR_ID<<",size="<<strlen(INVESTOR_ID) << endl;
    cout << "用户密码=" << PASSWORD<<",size="<<strlen(PASSWORD)  << endl;
    cout << "trade前置=" << TRADE_FRONT_ADDR<<",size="<<strlen(TRADE_FRONT_ADDR)  << endl;

	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.UserID, INVESTOR_ID);
	strcpy(req.Password, PASSWORD);
	int iResult = ptradeApi->ReqUserLogin(&req, ++iRequestID);
	cerr << "--->>> 发送用户登录请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
	string tmpstr("--->>> 发送用户登录请求: ");
	tmpstr.append(((iResult == 0) ? "成功" : "失败"));
	LOG(INFO) << (tmpstr);
	LogMsg *logmsg = new LogMsg();
	logmsg->setMsg(tmpstr);
	networkTradeQueue.push(logmsg);
}

void CTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	loginRef = pRspUserLogin;
	cerr << "--->>> " << "OnRspUserLogin" << endl;
	LOG(INFO) << ("--->>>OnRspUserLogin\0");
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
        // 保存会话参数
        FRONT_ID = pRspUserLogin->FrontID;
        SESSION_ID = pRspUserLogin->SessionID;
        int iNextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
        iNextOrderRef++;
        sprintf(ORDER_REF, "%d", iNextOrderRef);
        string str = "frontID=";
        char tmpfront[20];
        string strFront = boost::lexical_cast<string>(FRONT_ID);
        str.append(strFront); str += ";";
        str.append("sessionID=");
        char tmpsession[20];
        str.append(boost::lexical_cast<string>(SESSION_ID)); str += ";";
        str.append("maxOrderRef=");
        str.append(pRspUserLogin->MaxOrderRef); str += ";";
        //char tradingDay[12];
        strcpy(tradingDay, ptradeApi->GetTradingDay());
        string year = boost::lexical_cast<string>(tradingDay[0]) + boost::lexical_cast<string>(tradingDay[1]) + boost::lexical_cast<string>(tradingDay[2]) + boost::lexical_cast<string>(tradingDay[3]);
        string month = boost::lexical_cast<string>(tradingDay[4]) + boost::lexical_cast<string>(tradingDay[5]);
        string day = boost::lexical_cast<string>(tradingDay[6]) + boost::lexical_cast<string>(tradingDay[7]);
        tradingDayT = year + "-" + month + "-" + day;
        ///获取当前交易日
        cerr << "--->>> current tradingday = " << tradingDay << endl;
        string tmpstr = "--->>> current tradingday =";
        tmpstr.append(tradingDay);
        LOG(INFO) << (tmpstr);
        LogMsg *logmsg = new LogMsg();
        logmsg->setMsg(tmpstr);
        networkTradeQueue.push(logmsg);
        //thread_log_group.create_thread(ReqQryInstrument);
        //ReqQryInstrument();
        //boost::this_thread::sleep(boost::posix_time::seconds(4));
		///投资者结算结果确认
        ReqSettlementInfoConfirm();
        //ReqQryTradingAccount();
        //boost::thread thr11(&CTraderSpi::ReqQryInvestorPosition);
        //thread_log_group.create_thread(CTraderSpi::ReqQryInvestorPosition);
        //boost::this_thread::sleep(boost::posix_time::seconds(2));
        //ReqQryInvestorPosition();


        //boost::this_thread::sleep(boost::posix_time::seconds(1));

	}
}

void CTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	int iResult = ptradeApi->ReqSettlementInfoConfirm(&req, ++iRequestID);
	cerr << "--->>> 投资者结算结果确认: " << ((iResult == 0) ? "成功" : "失败") << endl;
	string tmpstr("--->>> 投资者结算结果确认: ");
	tmpstr.append(((iResult == 0) ? "成功" : "失败"));
	LOG(INFO) << (tmpstr);
	LogMsg *logmsg = new LogMsg();
	logmsg->setMsg(tmpstr);
	networkTradeQueue.push(logmsg);
}

void CTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspSettlementInfoConfirm" << endl;
	LOG(INFO) << ("--->>>OnRspSettlementInfoConfirm");
	tradequeue.push_back("--->>>OnRspSettlementInfoConfirm");
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		LOG(INFO) << ("投资者结算单确认完毕");
		tradequeue.push_back("投资者结算单确认完毕");
        boost::this_thread::sleep(boost::posix_time::seconds(1));
       // ReqQryTradingAccount();
		///请求查询合约
        ReqQryInstrument();
	}
}
bool isRsp = false;
void CTraderSpi::ReqQryInstrument()
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
    int iResult = ptradeApi->ReqQryInstrument(&req, ++iRequestID);
    cerr << "--->>> 请求查询合约: " << ((iResult == 0) ? "成功" : "失败") << endl;
    //return;
    //for (int i = 0, j = quoteList.size(); i < j; i++) {
      //  strcpy(req.InstrumentID, quoteList[i].c_str());
        //int iResult = ptradeApi->ReqQryInstrument(&req, ++iRequestID);
        //cerr << "--->>> 请求查询合约: " << ((iResult == 0) ? "成功" : "失败") << endl;
        //boost::this_thread::sleep(boost::posix_time::seconds(1));
    //}
}

void CTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pInstrument) {
		processRspReqInstrument(pInstrument);
	}
    //cerr << "--->>> " << "OnRspQryInstrument" << endl;
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
        boost::this_thread::sleep(boost::posix_time::seconds(1));
		///请求查询合约
        ReqQryTradingAccount();
        //ReqQryInvestorPosition();
	}
}

void CTraderSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
    cout << BROKER_ID << "," << INVESTOR_ID <<endl;
	int iResult = ptradeApi->ReqQryTradingAccount(&req, ++iRequestID);
	cerr << "--->>> 请求查询资金账户: " << ((iResult == 0) ? "成功" : "失败") << endl;
	string tmpstr = "--->>> 请求查询资金账户: ";
	tmpstr.append(((iResult == 0) ? "成功" : "失败"));
	LOG(INFO) << (tmpstr);
	LogMsg *logmsg = new LogMsg();
	logmsg->setMsg(tmpstr);
	networkTradeQueue.push(logmsg);
}

void CTraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspQryTradingAccount" << endl;
	LOG(INFO) << ("--->>>OnRspQryTradingAccount");
	string str;
	str.append("--->>> OnRspQryTradingAccount:查询投资者账户信息\n");
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
        if(pTradingAccount){
            str.append(pTradingAccount->BrokerID);
            str.append("\t");
            str.append(pTradingAccount->AccountID);
            str.append("\t");
            char a[100], b[100];
            sprintf(a, "%f", pTradingAccount->Available);
            sprintf(b, "%f", pTradingAccount->ExchangeMargin);
            str.append(a);
            str.append("\t");
            str.append(b);
        }

		LOG(INFO) << (str);
        boost::this_thread::sleep(boost::posix_time::seconds(1));
		///请求查询投资者持仓
        ReqQryInvestorPosition();
		//ReqQryInvestorPositionDetail();
	}
	// 	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	// 	{
	// 		LOG(INFO)<<("请求查询资金账户完毕");
	// 		tradequeue.push_back("请求查询资金账户完毕");
	// 		///请求查询投资者持仓
	// 		//ReqQryInvestorPosition();
	// 	}
}

void CTraderSpi::ReqQryInvestorPosition()
{
	//查询持仓之前，把持仓数据清除
	boost::recursive_mutex::scoped_lock SLock(pst_mtx);
	positionmap.clear();
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	strcpy(req.InstrumentID, INSTRUMENT_ID);
	int iResult = ptradeApi->ReqQryInvestorPosition(&req, ++iRequestID);
	cerr << "--->>> 请求查询投资者持仓: " << ((iResult == 0) ? "成功" : "失败") << endl;
	//查询结果
    string strRst = ((iResult == 0) ? "成功" : "失败");
    strRst.append("--->>> 请求查询投资者持仓: ");
    LOG(INFO) << (strRst);
}
void CTraderSpi::ReqQryInvestorPositionDetail()
{
	//查询持仓明细之前，把持仓数据清除
	boost::recursive_mutex::scoped_lock SLock(pstDetail_mtx);
	//positionDetailMap.clear();
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	strcpy(req.InstrumentID, INSTRUMENT_ID);
	int iResult = ptradeApi->ReqQryInvestorPositionDetail(&req, ++iRequestID);
	cerr << "--->>> 请求查询投资者持仓明细: " << ((iResult == 0) ? "成功" : "失败") << endl;
	//查询结果
    string strRst = ((iResult == 0) ? "成功" : "失败");
    strRst.append("--->>> 请求查询投资者持仓明细: ");
    LOG(INFO) << (strRst);
}
//查询投资者持仓
void CTraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField  *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspQryInvestorPositionDetail" << endl;
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition) {
		boost::recursive_mutex::scoped_lock SLock(pstDetail_mtx);
		HoldPositionDetail* detail = new HoldPositionDetail();
		detail->instrumentID = boost::lexical_cast<string>(pInvestorPosition->InstrumentID);
		detail->brokerID = boost::lexical_cast<string>(pInvestorPosition->BrokerID);
		detail->brokerID = boost::lexical_cast<string>(pInvestorPosition->BrokerID);
		detail->direction = boost::lexical_cast<string>(pInvestorPosition->Direction);
		detail->exchangeID = boost::lexical_cast<string>(pInvestorPosition->ExchangeID);
		detail->hedgeFlag = boost::lexical_cast<string>(pInvestorPosition->HedgeFlag);
		detail->tradeID = boost::lexical_cast<string>(pInvestorPosition->TradeID);
		detail->openDate = boost::lexical_cast<string>(pInvestorPosition->OpenDate);
		detail->volume = pInvestorPosition->Volume;
		detail->openPrice = pInvestorPosition->OpenPrice;
		unordered_map<string, list<HoldPositionDetail*>*>::iterator pdmit = positionDetailMap.find(detail->instrumentID);
		if (pdmit == positionDetailMap.end()) {//初始化
			list<HoldPositionDetail*>* tmplist = new list<HoldPositionDetail*>();
			tmplist->emplace_back(detail);
			positionDetailMap[detail->instrumentID] = tmplist;
		} else {
			list<HoldPositionDetail*>* tmplist = pdmit->second;
			tmplist->emplace_back(detail);
		}
		string dt = getPositionDetail(pInvestorPosition);
		LOG(INFO) << "持仓明细：" + dt;
		cout<< dt;
		LogMsg *logmsg = new LogMsg();
		logmsg->setMsg(dt);
		networkTradeQueue.push(logmsg);
	}
	if (bIsLast && !IsErrorRspInfo(pRspInfo)){
		unordered_map<string, list<HoldPositionDetail*>*>::iterator pdmit = positionDetailMap.find(boost::lexical_cast<string>(pInvestorPosition->InstrumentID));
		int size = 0;
		if (pdmit != positionDetailMap.end()) {
			list<HoldPositionDetail*>* tmplist = pdmit->second;
			size = tmplist->size();
		}
		string tmpstr = "持仓明细查询完成,共" + boost::lexical_cast<string>(size) + "笔成交明细记录";
		LOG(INFO) << tmpstr;
		LogMsg *logmsg = new LogMsg();
		logmsg->setMsg(tmpstr);
		networkTradeQueue.push(logmsg);
	}
}
void setInitAvaHoldPosition() {
	for (unordered_map<string, HoldPositionInfo*>::iterator tmpit = positionmap.begin(); tmpit != positionmap.end();tmpit++) {
		string firstInstrumentID = tmpit->first;
		HoldPositionInfo* firstHold = tmpit->second;
		//组合持仓量计算
		unordered_map<string, vector<string>>::iterator mapit = instr_map.find(firstInstrumentID);
		if (mapit != instr_map.end()) {
			vector<string> vclist = mapit->second;
			for (unsigned int i = 0, j = vclist.size(); i < j;i++) {
				string secInstrumentID = vclist[i];
				unordered_map<string, HoldPositionInfo*>::iterator pstIt = positionmap.find(secInstrumentID);
				if (pstIt == positionmap.end()) {
					continue;
				}
				HoldPositionInfo* secHold = pstIt->second;
				string insComKey = getComInstrumentKey(firstInstrumentID,secInstrumentID);
				unordered_map<string, TechMetric*>::iterator tmIT = techMetricMap.find(insComKey);
				TechMetric* TM;
				if (tmIT == techMetricMap.end()) {
					TM = new TechMetric();
                    list<double>* tmpmaseq = new list<double>();
					list<PriceGapMarketData*>* tmpkdseq = new list<PriceGapMarketData*>();
					TM->pgDataSeq = tmpkdseq;
                    TM->MAGapSeq = tmpmaseq;
					techMetricMap[insComKey] = TM;
				} else {
					TM = tmIT->second;
				}
				PriceGap* pg = getPriceGap(insComKey);
				if (firstInstrumentID < secInstrumentID) {//first=1801,sec=1805
					//double asHoldMeanGap = 0;
					//double dsHoldMeanGap = 0;
					if (firstHold->longHoldAvgPrice != 0 && secHold->shortHoldAvgPrice != 0) {
						TM->asHoldMeanGap = firstHold->longHoldAvgPrice - secHold->shortHoldAvgPrice;
					}
					if (firstHold->shortHoldAvgPrice != 0 && secHold->longHoldAvgPrice != 0) {
						TM->dsHoldMeanGap = firstHold->shortHoldAvgPrice - secHold->longHoldAvgPrice;
					}
					int asLongHold = firstHold->longTotalPosition;
					int asShortHold = secHold->shortTotalPosition;
					int dsShortHold = firstHold->shortTotalPosition;
					int dsLongHold = secHold->longTotalPosition;
					if (asLongHold >= asShortHold) {//正套持仓
						pg->arbComVolumeAS = asShortHold;
					} else {
						pg->arbComVolumeAS = asLongHold;
					}
					if (dsShortHold >= dsLongHold) {
						pg->arbComVolumeDS = dsLongHold;
					} else {
						pg->arbComVolumeDS = dsShortHold;
					}
                    initOverTickNums(TM, insComKey);
                    /*
					//组合持仓开平参数设置
					if (insComKey == "zn1801-zn1805") {
						TM->overMAGapTickNums = 2;
						TM->downMAGapTickNums = 2;
					} else if (insComKey == "pp1801-pp1805") {
						TM->overMAGapTickNums = 6;
						TM->downMAGapTickNums = 6;
					} else if (insComKey == "ru1801-ru1805") {
						TM->overMAGapTickNums = 1;
						TM->downMAGapTickNums = 1;
					} else if (insComKey == "rb1801-rb1805") {
						TM->overMAGapTickNums = 1;
						TM->downMAGapTickNums = 1;
					} else if (insComKey == "i1801-i1805") {
						TM->overMAGapTickNums = 3.5;
						TM->downMAGapTickNums = 3;
					} else if (insComKey == "MA801-MA805") {
						TM->overMAGapTickNums = 2.1;
						TM->downMAGapTickNums = 2.3;
					} else if (insComKey == "hc1801-hc1805") {
						TM->overMAGapTickNums = 3;
						TM->downMAGapTickNums = 3.3;
					} else if (insComKey == "ni1801-ni1805") {
                        TM->overMAGapTickNums = 5.5;
                        TM->downMAGapTickNums = 5.5;
					} else if (insComKey == "jm1801-jm1805") {
                        TM->overMAGapTickNums = 3.8;
                        TM->downMAGapTickNums = 3.8;
                    }*/
					cout << insComKey + "正套持仓数量=" + boost::lexical_cast<string>(pg->arbComVolumeAS) + ",反套持仓数量=" + boost::lexical_cast<string>(pg->arbComVolumeDS) +",正套持仓均价=" + boost::lexical_cast<string>(TM->asHoldMeanGap) + ",反套持仓均价=" + boost::lexical_cast<string>(TM->dsHoldMeanGap) <<endl;
				}
			}
		}
		
	}
}
//查询投资者持仓
void CTraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspQryInvestorPosition" << endl;
	if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition) {
		initpst(pInvestorPosition);
	}
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		//unordered_map<string, unordered_map<string, int>>::iterator tmpit = positionmap.begin();
		unordered_map<string, HoldPositionInfo*>::iterator tmpit = positionmap.begin();
		if (tmpit == positionmap.end()) {
			cout << "当前无持仓信息" << endl;
		} else {
			int tmpArbVolume = 0;
			for (; tmpit != positionmap.end(); tmpit++) {
				string str_instrument = tmpit->first;
				HoldPositionInfo* tmppst = tmpit->second;
				char char_tmp_pst[10] = { '\0' };
				char char_longyd_pst[10] = { '\0' };
				char char_longtd_pst[10] = { '\0' };
				//sprintf(char_tmp_pst, "%d", tmppst["longTotalPosition"]);
				//sprintf(char_longyd_pst, "%d", tmppst["longYdPosition"]);
				//sprintf(char_longtd_pst, "%d", tmppst["longTdPosition"]);
				sprintf(char_tmp_pst, "%d", tmppst->longTotalPosition);
				sprintf(char_longyd_pst, "%d", tmppst->longYdPosition);
				sprintf(char_longtd_pst, "%d", tmppst->longTdPosition);
				char char_tmp_pst2[10] = { '\0' };
				char char_shortyd_pst[10] = { '\0' };
				char char_shorttd_pst[10] = { '\0' };
				//sprintf(char_tmp_pst2, "%d", tmppst["shortTotalPosition"]);
				//sprintf(char_shortyd_pst, "%d", tmppst["shortYdPosition"]);
				//sprintf(char_shorttd_pst, "%d", tmppst["shortTdPosition"]);
				sprintf(char_tmp_pst2, "%d", tmppst->shortTotalPosition);
				sprintf(char_shortyd_pst, "%d", tmppst->shortYdPosition);
				sprintf(char_shorttd_pst, "%d", tmppst->shortTdPosition);
				if (tmppst->longYdPosition > 0) {
					shortPstIsClose = 2;
					short_offset_flag = 4;
				}
				if (tmppst->shortYdPosition > 0) {
					longPstIsClose = 2;
					long_offset_flag = 4;
				}
				//
				int currHoldPst = 0;
				int pdHoldPst = 0;
				if (tmppst->longTotalPosition == 0) {
					currHoldPst = tmppst->shortTotalPosition;
				} else if (tmppst->shortTotalPosition == 0) {
					currHoldPst = tmppst->longTotalPosition;
				}else if (tmppst->longTotalPosition >= tmppst->shortTotalPosition) {
					currHoldPst = tmppst->shortTotalPosition;
				} else {
					currHoldPst = tmppst->longTotalPosition;
				}
				//组合持仓量计算
				unordered_map<string, vector<string>>::iterator mapit = instr_map.find(str_instrument);
				string pd_instrumentid = "";
				if (mapit != instr_map.end()) {
					vector<string> mapvec = mapit->second;
					for (int i = 0, ma = mapvec.size();i < ma ; i++) {
						pd_instrumentid = mapvec[i];
						unordered_map<string, HoldPositionInfo*>::iterator tit = positionmap.find(pd_instrumentid);
						if (tit != positionmap.end()) {
							HoldPositionInfo* hpi = tit->second;
							int pdlong = hpi->longTotalPosition;
							int pdshort = hpi->shortTotalPosition;
							if (pdlong == 0) {
								pdHoldPst = pdshort;
							} else if (pdshort == 0) {
								pdHoldPst = pdlong;
							} else if (pdlong >= pdshort) {
								pdHoldPst = pdshort;
							} else {
								pdHoldPst = pdlong;
							}
						}
					}
				}
				if (currHoldPst == 0) {
					tmpArbVolume = 0;
				} else if (pdHoldPst == 0) {
					tmpArbVolume = 0;
				} else if (currHoldPst >= pdHoldPst) {
					tmpArbVolume = pdHoldPst;
				} else {
					tmpArbVolume = currHoldPst;
				}
				//                int longpst = tmppst["longTotalPosition"];
				//                int shortpst = tmppst["shortTotalPosition"];
				//                char char_longpst[12] = {'\0'};
				//                char char_shortpst[12] = {'\0'};
				//                sprintf(char_longpst,"%d",longpst);
				//                sprintf(char_shortpst,"%d",shortpst);
				string pst_msg = "持仓结构:" + str_instrument + ",多头持仓量=" + string(char_tmp_pst) + ",今仓数量=" + string(char_longtd_pst) + ",昨仓数量=" + string(char_longyd_pst) + ",可平量=" + boost::lexical_cast<string>(tmppst->longAvaClosePosition) + ",持仓均价=" + boost::lexical_cast<string>(tmppst->longHoldAvgPrice) +
					";空头持仓量=" + string(char_tmp_pst2) + ",今仓数量=" + string(char_shorttd_pst) + ",昨仓数量=" + string(char_shortyd_pst) + ",可平量=" + boost::lexical_cast<string>(tmppst->shortAvaClosePosition) + ",持仓均价=" + boost::lexical_cast<string>(tmppst->shortHoldAvgPrice) +
					";组合持仓量=" + boost::lexical_cast<string>(tmpArbVolume) + ",阈值=" + boost::lexical_cast<string>(arbVolumeMetric);
				string insComKey = getComInstrumentKey(str_instrument, pd_instrumentid);
				unordered_map<string, PriceGap*>::iterator gapIt =  instr_price_gap.find(insComKey);
				if (gapIt == instr_price_gap.end()) {
					LOG(ERROR) << "无法找到组合合约的gap信息：" + insComKey;
				} else {
					PriceGap* pg = gapIt->second;
					//pg->arbComVolume = tmpArbVolume;
					//pg->arbComVolMetric = 100;
					LOG(ERROR) << insComKey + "设置组合合约的持仓量=" + boost::lexical_cast<string>(tmpArbVolume);
				}
				cout << pst_msg << endl;
				LOG(INFO) << pst_msg;
				LogMsg *logmsg = new LogMsg();
				logmsg->setMsg(pst_msg);
				networkTradeQueue.push(logmsg);
				//startStrategy();
			}
		}
		//call tradeParaProcess method to set close or open
		tradeParaProcessTwo();
		setInitAvaHoldPosition();
        cout << ">>>>>>>>>>>>if ok,please press Enter!<<<<<<<<<<<<"<< endl;
        getchar();
	}
}

//初始化持仓信息
void initpst(CThostFtdcInvestorPositionField *pInvestorPosition)
{
	boost::recursive_mutex::scoped_lock SLock(pst_mtx);
	///合约代码
	char	*InstrumentID = pInvestorPosition->InstrumentID;
	string str_instrumentid = string(InstrumentID);
	///持仓多空方向
	TThostFtdcPosiDirectionType	dir = pInvestorPosition->PosiDirection;
	char PosiDirection[] = { dir,'\0' };
	///投机套保标志
	TThostFtdcHedgeFlagType	flag = pInvestorPosition->HedgeFlag;
	char HedgeFlag[] = { flag,'\0' };
	///上日持仓
	TThostFtdcVolumeType	ydPosition = pInvestorPosition->YdPosition;
	char YdPosition[100];
	sprintf(YdPosition, "%d", ydPosition);
	///今日持仓
	TThostFtdcVolumeType	position = pInvestorPosition->Position;
	char Position[100];
	sprintf(Position, "%d", position);
	string str_dir = string(PosiDirection);
	double multiplier = getMultipler(str_instrumentid);
	///持仓日期
	TThostFtdcPositionDateType	positionDate = pInvestorPosition->PositionDate;
	char PositionDate[] = { positionDate,'\0' };
	string str_pst_date = string(PositionDate);
	if (positionmap.find(str_instrumentid) == positionmap.end()) {//暂时没有处理，不需要考虑多空方向
		unordered_map<string, int> tmpmap;
		HoldPositionInfo* tmpinfo = new HoldPositionInfo();
		if ("2" == str_dir) {//买  //多头
			tmpinfo->longTotalPosition = position;
			tmpinfo->longAvaClosePosition = position;
			tmpinfo->longAmount = pInvestorPosition->PositionCost;
			tmpinfo->longHoldAvgPrice = pInvestorPosition->PositionCost / (multiplier*position);
			//tmpmap["longTotalPosition"] = position;
			//空头
			tmpinfo->shortTotalPosition = 0;
			//tmpmap["shortTotalPosition"] = 0;
			if ("2" == str_pst_date) {//昨仓
				tmpinfo->longYdPosition = position;
			} else if ("1" == str_pst_date) {//今仓
				tmpinfo->longTdPosition = position;
			}
		} else if ("3" == str_dir) {//空
									//空头
									//tmpmap["shortTotalPosition"] = position;
			tmpinfo->longTotalPosition = 0;
			tmpinfo->shortTotalPosition = position;
			tmpinfo->shortAvaClosePosition = position;
			tmpinfo->shortAmount = pInvestorPosition->PositionCost;
			tmpinfo->shortHoldAvgPrice = pInvestorPosition->PositionCost / (multiplier*position);
			//tmpmap["longTotalPosition"] = 0;
			if ("2" == str_pst_date) {//昨仓
				tmpinfo->shortYdPosition = position;
			} else if ("1" == str_pst_date) {//今仓
				tmpinfo->shortTdPosition = position;
			}
		} else {
			//cout << InstrumentID << ";error:持仓类型无法判断PosiDirection=" << str_dir << endl;
			LOG(ERROR) << string(InstrumentID) + ";error:持仓类型无法判断PosiDirection=" + str_dir;
			return;
		}
		positionmap[str_instrumentid] = tmpinfo;
	} else {
		unordered_map<string, HoldPositionInfo*>::iterator tmpmap = positionmap.find(str_instrumentid);
		HoldPositionInfo* tmpinfo = tmpmap->second;
		//对应的反方向应该已经存在，这里后续需要确认
		if ("2" == str_dir) {//多 
							 //多头
							 //tmpmap->second["longTotalPosition"] = position + tmpmap->second["longTotalPosition"];
			tmpinfo->longTotalPosition = position + tmpinfo->longTotalPosition;
			tmpinfo->longAvaClosePosition = position + tmpinfo->longAvaClosePosition;
			tmpinfo->longAmount = pInvestorPosition->PositionCost + tmpinfo->longAmount;
			tmpinfo->longHoldAvgPrice = tmpinfo->longAmount / (multiplier*tmpinfo->longTotalPosition);
			if ("2" == str_pst_date) {//昨仓
				tmpinfo->longYdPosition = position + tmpinfo->longYdPosition;
			} else if ("1" == str_pst_date) {//今仓
				tmpinfo->longTdPosition = position + tmpinfo->longTdPosition;
			}
		} else if ("3" == str_dir) {//空
									//空头
									//tmpmap->second["shortTotalPosition"] = position + tmpmap->second["shortTotalPosition"];
			tmpinfo->shortTotalPosition = position + tmpinfo->shortTotalPosition;
			tmpinfo->shortAvaClosePosition = position + tmpinfo->shortAvaClosePosition;
			tmpinfo->shortAmount = pInvestorPosition->PositionCost + tmpinfo->shortAmount;
			tmpinfo->shortHoldAvgPrice = tmpinfo->shortAmount / (multiplier*tmpinfo->shortTotalPosition);
			if ("2" == str_pst_date) {//昨仓
				tmpinfo->shortYdPosition = position + tmpinfo->shortYdPosition;
			} else if ("1" == str_pst_date) {//今仓
				tmpinfo->shortTdPosition = position + tmpinfo->shortTdPosition;
			}
		} else {
			//cout << InstrumentID << ";error:持仓类型无法判断PosiDirection=" << str_dir << endl;
			LOG(ERROR) << string(InstrumentID) + ";error:持仓类型无法判断PosiDirection=" + str_dir;
			return;
		}
	}
	storeInvestorPosition(pInvestorPosition);
}

void CTraderSpi::ReqOrderInsert(list<string> orderInsert)
{
	char InstrumentID[31] = { "\0" };
	int RequestID = 0;
	char Direction[2] = { "\0" };
	///投机 '1';套保'3'
	char HedgeFlag[2] = { "\0" };
	//组合开平标志: 开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'
	char OffsetFlag[2] = { "\0" };
	///价格 double
	TThostFtdcPriceType Price = 0;
	///报单价格条件
	TThostFtdcOrderPriceTypeType orderPriceType[2] = "2";
	///报单类型,默认当日有效
	TThostFtdcTimeConditionType timeCondition[2] = "3";
	//开仓手数
	int Volume;
	//报单引用编号
	char OrderRef[13] = "\0";
	//报单结构体
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	cout << "------------>ReqOrderInsert function" << endl;
	const char * split = "="; //分割符号
	int fieldSize = orderInsert.size();
	/************************************************************************/
	/* 每个字段，按照=分隔符进行分割                                        */
	/************************************************************************/
	try {
		int i = 0;
		for (list<string>::iterator beg = orderInsert.begin(); beg != orderInsert.end(); beg++) {
			//for(int i = 0;i < fieldSize;i ++){
			i++;
			string tmpstr = *beg;
			cout << "orderinsert data:" << tmpstr << endl;
			//分割之后的字符
			char * p = 0;
			//string转char*
			char * rawfields = new char[tmpstr.size() + 1];
			strcpy(rawfields, tmpstr.c_str());
			p = strtok(rawfields, split); //分割字符串
			vector<string> strlist;
			while (p != NULL)
			{
				//cout << p <<endl;
				strlist.push_back(p);
				p = strtok(NULL, split); //指向下一个指针
			}
			if (strlist.size() != 2) {
				//有字段为空，不填
				string tmpstr2 = "some column value is null:";
				LOG(INFO) << (tmpstr2 += tmpstr);
				string tmpstr3 = "there is field value is null!!!:";
				tmpstr3.append(tmpstr);
				tradequeue.push_back(tmpstr3);
				continue;
			}
			/************************************************************************/
			/* 变量赋值                                                                     */
			/*InstrumentID		1
			/*RequestID			2
			/*Direction			3
			/*HedgeFlag			4
			/*OffsetFlag		5
			/*Price				6
			/*Volume			7
			/*OrderRef			8
			/*priceType         9
			/*timeCondition     10
			/************************************************************************/
			string ttt = strlist.at(1);
			//cout << "赋值为:" + ttt<<endl;
			if (i == 1) {
				strcpy(InstrumentID, ttt.c_str());
			} else if (i == 2) {
				RequestID = atoi(ttt.c_str());
			} else if (i == 3) {
				strcpy(Direction, ttt.c_str());
			} else if (i == 4) {
				strcpy(HedgeFlag, ttt.c_str());
			} else if (i == 5) {
				strcpy(OffsetFlag, ttt.c_str());
			} else if (i == 6) {
				Price = atof(ttt.c_str());
			} else if (i == 7) {
				Volume = atoi(ttt.c_str());
			} else if (i == 8) {
				strcpy(OrderRef, ttt.c_str());
			} else if (i == 9) {
				strcpy(orderPriceType, ttt.c_str());
			} else if (i == 10) {
				strcpy(timeCondition, ttt.c_str());
			}
		}
		///经纪公司代码
		strcpy(req.BrokerID, BROKER_ID);
		///投资者代码
		strcpy(req.InvestorID, INVESTOR_ID);
		///合约代码
		strcpy(req.InstrumentID, InstrumentID);
		///报单引用
		strcpy(req.OrderRef, OrderRef);
		req.RequestID = RequestID;
		///用户代码
		//	TThostFtdcUserIDType	UserID;
		///报单价格条件: 限价
		//req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
		req.OrderPriceType = orderPriceType[0];
		///买卖方向: 
		req.Direction = Direction[0];
		///组合开平标志: 开仓
		req.CombOffsetFlag[0] = OffsetFlag[0];
		///组合投机套保标志
		req.CombHedgeFlag[0] = HedgeFlag[0];
		///价格
		req.LimitPrice = Price;
		///数量: 1
		req.VolumeTotalOriginal = Volume;
		///有效期类型: 当日有效
		req.TimeCondition = timeCondition[0];
		///GTD日期
		//	TThostFtdcDateType	GTDDate;
		///成交量类型: 任何数量
		req.VolumeCondition = THOST_FTDC_VC_AV;
		///最小成交量: 1
		req.MinVolume = 1;
		///触发条件: 立即
		req.ContingentCondition = THOST_FTDC_CC_Immediately;
		///止损价
		//	TThostFtdcPriceType	StopPrice;
		///强平原因: 非强平
		req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		///自动挂起标志: 否
		req.IsAutoSuspend = 0;
		///业务单元
		//	TThostFtdcBusinessUnitType	BusinessUnit;
		///请求编号
		//	TThostFtdcRequestIDType	RequestID;
		///用户强评标志: 否
		req.UserForceClose = 0;
	}
	catch (const runtime_error &re) {
		cerr << re.what() << endl;
	}
	catch (exception* e)
	{
		cerr << e->what() << endl;
		LOG(INFO) << (e->what());
	}
	//委托类操作，使用客户端定义的请求编号格式
	int iResult = ptradeApi->ReqOrderInsert(&req, RequestID);

	cerr << "--->>> ReqOrderInsert:" << ((iResult == 0) ? "成功" : "失败") << endl;
	//记录报单录入信息
	string orderinsertstr = getInvestorOrderInsertInfoByDelimater(&req);
	LOG(INFO) << ("ReqOrderInsert--->>>" + orderinsertstr);
	string tmpstr = "--->>> ReqOrderInsert: ";
	tmpstr.append(((iResult == 0) ? "success" : "failed"));
	LOG(INFO) << (tmpstr);
	//返回客户端信息
	string tmpstr2;
	if (iResult == 0) {
		tmpstr2.append("businessType=110").append(sep).append("result=0").append(sep).append(orderinsertstr);
	} else {
		tmpstr2.append("businessType=110").append(sep).append("result=1").append(sep).append(orderinsertstr);
	}
	//tradequeue.push_back(tmpstr2);
}

void CTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//记录错误信息
	bool err = IsErrorRspInfo(pRspInfo);
	cout << "--->>> " << "OnRspOrderInsert" << "响应请求编号：" << nRequestID << " CTP回报请求编号" << pInputOrder->RequestID << endl;
	cerr << "--->>> " << "OnRspOrderInsert" << "响应请求编号：" << nRequestID << " CTP回报请求编号" << pInputOrder->RequestID << endl;
	char tt[20];
	sprintf(tt, "%d", pRspInfo->ErrorID);
	string sInputOrderInfo = getInvestorOrderInsertInfoByDelimater(pInputOrder);
	string tmpstr2;
	tmpstr2.append("businessType=100").append(sep).append("result=1").append(sep).append(sInputOrderInfo).append("ErrorID=").append(tt);
	tmpstr2.append(sep).append("ErrorMsg=").append(pRspInfo->ErrorMsg);
	tradequeue.push_back(tmpstr2);
	// 	string sResult = "";
	// 	if(err){
	// 		char cErrorID[100] ;
	// 		itoa(pRspInfo->ErrorID,cErrorID,10);
	// 		char cRequestid[100];
	// 		sprintf(cRequestid,"%d",nRequestID);
	// 		char ctpRequestId[100];
	// 		sprintf(ctpRequestId,"%d",pInputOrder->RequestID);
	// 		sResult.append("--->>>CTP报单回报信息: ErrorID=");
	// 		sResult.append(cErrorID);
	// 		sResult.append(", ErrorMsg=");
	// 		sResult.append(pRspInfo->ErrorMsg);
	// 	}
	// 	
	// 
	// 	tradequeue.push_back(sResult);
	// 	
	// 	sInputOrderInfo.append("\t");
	// 	sInputOrderInfo.append(sResult);
	// 	sInputOrderInfo.append(";响应请求编号:nRequestI=");
	// 	sInputOrderInfo.append(cRequestid);
	// 	sInputOrderInfo.append( "; CTP回报请求编号:RequestID=");
	// 	sInputOrderInfo.append(ctpRequestId);
	LOG(INFO) << ("OnRspOrderInsert:" + sInputOrderInfo);
}
///报单录入错误回报
void CTraderSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	//记录错误信息
	bool err = IsErrorRspInfo(pRspInfo);
	string sInputOrderInfo = getInvestorOrderInsertInfoByDelimater(pInputOrder);
	if (err) {
		string sResult;
		sResult.append("--->>>交易所报单录入错误回报:;ErrorID=");
        sResult.append(boost::lexical_cast<string>(pRspInfo->ErrorID));
		sResult.append(";ErrorMsg=");
		sResult.append(pRspInfo->ErrorMsg).append(";");
		sInputOrderInfo.append(sResult);
	}
	//记录错误回报报单信息
	LOG(INFO) << (sInputOrderInfo);
	string str = "businessType=400;result=1;";
	str.append(sInputOrderInfo);
	tradequeue.push_back(str);

}
///报单操作请求
void CTraderSpi::ReqOrderActionTmp(list<string> orderAction) {
	CThostFtdcOrderField pOrder = AssambleOrderActionTwo(orderAction);
	ReqOrderAction(&pOrder);
}
void CTraderSpi::ReqOrderAction(CThostFtdcOrderField *pOrder)
{
	//暂时不适用
	// 	static bool ORDER_ACTION_SENT = false;		//是否发送了报单
	// 	if (ORDER_ACTION_SENT)
	// 		return;

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy(req.BrokerID, pOrder->BrokerID);
	if (strcmp(pOrder->InvestorID, "0") == 0) {
		return;
	}
	///投资者代码
	strcpy(req.InvestorID, pOrder->InvestorID);
	///报单操作引用
	//	TThostFtdcOrderActionRefType	OrderActionRef;
	///报单引用
	strcpy(req.OrderRef, pOrder->OrderRef);
	///请求编号
	req.RequestID = pOrder->RequestID;
	///前置编号
	req.FrontID = FRONT_ID;
	///会话编号
	req.SessionID = SESSION_ID;
	///交易所代码
	//strcpy(req.ExchangeID, pOrder->ExchangeID);
	//	TThostFtdcExchangeIDType	ExchangeID;
	///报单编号
	strcpy(req.OrderSysID, pOrder->OrderSysID);
	//	TThostFtdcOrderSysIDType	OrderSysID;
	///操作标志
	req.ActionFlag = THOST_FTDC_AF_Delete;
	///合约代码
	strcpy(req.InstrumentID, pOrder->InstrumentID);

	int iResult = ptradeApi->ReqOrderAction(&req, pOrder->RequestID);
	cerr << "--->>> 报单操作请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
	//ORDER_ACTION_SENT = true;
	//记录报单录入信息
	string orderinsertstr = getOrderActionInfoByDelimater(&req);
	LOG(INFO) << ("--->>>操作报单录入信息为:" + orderinsertstr);
	string tmpstr = "--->>> 操作报单录入请求: ";
	tmpstr.append(((iResult == 0) ? "成功" : "失败"));
	LOG(INFO) << (tmpstr);
	string tmpstr2 = ((iResult == 0) ? "success" : "failed");
	//tradequeue.push_back("orderAction info:" + orderinsertstr);
	//tradequeue.push_back("orderAction result:" + tmpstr2);
}

void CTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "--->>> " << "OnRspOrderAction" << endl;
	bool err = IsErrorRspInfo(pRspInfo);
	string sInputOrderInfo = getOrderActionInfoByDelimater(pInputOrderAction);
	tradequeue.push_back("OnRspOrderAction:" + sInputOrderInfo);
	string sResult = "";
	// 	if(err){
	// 		char cErrorID[100] ;
	// 		itoa(pRspInfo->ErrorID,cErrorID,10);
	// 		char cRequestid[100];
	// 		sprintf(cRequestid,"%d",nRequestID);
	// 		char ctpRequestId[100];
	// 		sprintf(ctpRequestId,"%d",pInputOrderAction->RequestID);
	// 		sResult.append("--->>>CTP报单回报信息: ErrorID=");
	// 		sResult.append(cErrorID);
	// 		sResult.append(", ErrorMsg=");
	// 		sResult.append(pRspInfo->ErrorMsg);
	// 	}
	// 
	// 
	// 	tradequeue.push_back(sResult);
	// 
	// 	sInputOrderInfo.append("\t");
	// 	sInputOrderInfo.append(sResult);
	// 	sInputOrderInfo.append(";响应请求编号:nRequestI=");
	// 	sInputOrderInfo.append(cRequestid);
	// 	sInputOrderInfo.append( "; CTP回报请求编号:RequestID=");
	// 	sInputOrderInfo.append(ctpRequestId);
	LOG(INFO) << (sInputOrderInfo);
}

///报单通知
void CTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	if (start_process == 0) {
		return;
	}
	cerr << "--->>> " << "OnRtnOrder" << endl;
	string tmpstr = getRtnOrderInfoByDelimater(pOrder);
	char	status[] = { pOrder->OrderSubmitStatus,'\0' };
	string tmpstr2;
	tmpstr2.append("businessType=300").append(sep).append("result=").append(status).append(sep).append(tmpstr);
	LOG(INFO) << ("--->>>OnRtnOrder:" + tmpstr);
	//tradequeue.push_back(tmpstr2);
	///报单提交状态
	TThostFtdcOrderSubmitStatusType orderSubStatus = pOrder->OrderSubmitStatus;
	if (orderSubStatus == '4' || orderSubStatus == '5' || orderSubStatus == '6') {//报单错误
		LOG(INFO) << ("报单错误:" + string(pOrder->StatusMsg));
	}
	string statusMsg = boost::lexical_cast<string>(pOrder->StatusMsg);
	///报单状态
	TThostFtdcOrderStatusType orderStatus = pOrder->OrderStatus;
	/*
	///全部成交
	#define THOST_FTDC_OST_AllTraded '0'
	///部分成交还在队列中
	#define THOST_FTDC_OST_PartTradedQueueing '1'
	///部分成交不在队列中
	#define THOST_FTDC_OST_PartTradedNotQueueing '2'
	///未成交还在队列中
	#define THOST_FTDC_OST_NoTradeQueueing '3'
	///未成交不在队列中
	#define THOST_FTDC_OST_NoTradeNotQueueing '4'
	///撤单
	#define THOST_FTDC_OST_Canceled '5'
	///未知
	#define THOST_FTDC_OST_Unknown 'a'
	///尚未触发
	#define THOST_FTDC_OST_NotTouched 'b' */
	//锁持仓处理
	boost::recursive_mutex::scoped_lock SLock3(willTrade_mtx);
	//boost::recursive_mutex::scoped_lock SLock6(arbVolume_mtx);//加锁处理
	boost::recursive_mutex::scoped_lock SLock5(priceGap_mtx);//锁定
	//锁持仓处理
	boost::recursive_mutex::scoped_lock SLock2(stopProfit_mtx);
	boost::recursive_mutex::scoped_lock SLock(pst_mtx);//锁定
	boost::recursive_mutex::scoped_lock SLock4(alreadyTrade_mtx);//锁定
	if (orderStatus == '5') {//撤单，说明报单未成功;不活跃合约需要持续报单，活跃合约暂时不处理
							 //正常撤单都是由frontid+sessionid+orderref选择；但是手动撤单采用ordersysid+brokerorderseq
							 //判断是否是不活跃合约
		processHowManyHoldsCanBeClose(pOrder,"release");//释放持仓
		string tmpOrderRef = pOrder->OrderRef;
		int* rst = decideOrderType(pOrder);//手动撤单判断为“0”
		int insType = rst[0];
		int amount = rst[1];
		LOG(INFO) << boost::lexical_cast<string>(insType) + ";" + boost::lexical_cast<string>(amount);
		if (insType == 1 || insType == 3) {//不活跃合约，需要持续报单;连续下5笔，没有成交则删除原套利（包括开仓和平仓）
			/*
			if (amount > notActiveInsertAmount) {
				return;
			} else {
				string isDelete = setNotActiveOrderInsertAmount(pOrder);
				if (isDelete == "1") {//说明下单超过次数，删除套利单
					return;
				}
			}*/
			string isDelete = setNotActiveOrderInsertAmount(pOrder);
			if (isDelete == "1") {//说明下单超过次数，删除套利单
				return;
			}
            boost::this_thread::sleep(boost::posix_time::microseconds(orderInsertInterval));    //microsecond,millisecn
            //Sleep(orderInsertInterval);
			//报单结构体
			CThostFtdcInputOrderField req;
			string newOrderRef = boost::lexical_cast<string>(iOrderRef++);
			memset(&req, 0, sizeof(req));
			///经纪公司代码
			strcpy(req.BrokerID, BROKER_ID);
			///投资者代码
			strcpy(req.InvestorID, INVESTOR_ID);
			///合约代码
			strcpy(req.InstrumentID, pOrder->InstrumentID);
			///报单引用
			//string orderRef = boost::lexical_cast<string>(iOrderRef++);
			strcpy(req.OrderRef, newOrderRef.c_str());
			req.RequestID = iRequestID++;
			///用户代码
			//	TThostFtdcUserIDType	UserID;
			///报单价格条件: 限价
			req.OrderPriceType = pOrder->OrderPriceType;
			///买卖方向: 
			req.Direction = pOrder->Direction;
			///组合开平标志: 开仓
			req.CombOffsetFlag[0] = pOrder->CombOffsetFlag[0];
			///组合投机套保标志
			req.CombHedgeFlag[0] = pOrder->CombHedgeFlag[0];
			///价格
			req.LimitPrice = pOrder->LimitPrice;
			///数量: 1
			req.VolumeTotalOriginal = pOrder->VolumeTotalOriginal;
			///有效期类型: 当日有效
			//req.TimeCondition = THOST_FTDC_TC_GFD;
			req.TimeCondition = pOrder->TimeCondition;
			///GTD日期
			//	TThostFtdcDateType	GTDDate;
			///成交量类型: 任何数量
			req.VolumeCondition = pOrder->VolumeCondition;
			///最小成交量: 1
			req.MinVolume = 1;
			///触发条件: 立即
			req.ContingentCondition = pOrder->ContingentCondition;
			///止损价
			//	TThostFtdcPriceType	StopPrice;
			///强平原因: 非强平
			req.ForceCloseReason = pOrder->ForceCloseReason;
			///自动挂起标志: 否
			req.IsAutoSuspend = 0;
			///业务单元
			//	TThostFtdcBusinessUnitType	BusinessUnit;
			///请求编号
			//	TThostFtdcRequestIDType	RequestID;
			///用户强评标志: 否
			req.UserForceClose = 0;
			//委托类操作，使用客户端定义的请求编号格式
			int iResult = ptradeApi->ReqOrderInsert(&req, iRequestID++);
			if (iResult == 0) {//报入成功，需要更新对应套利合约报单的orderRef
				string rst = resetOrderRef(pOrder, tmpOrderRef, newOrderRef);
				string msg = "";
				if (rst == "1") {
					msg = "change instrumentID=" + string(pOrder->InstrumentID) + " orderRef success!";
				} else {
					msg = "change instrumentID=" + string(pOrder->InstrumentID) + " orderRef failed!";
				}
				LOG(INFO) << (msg);
			}
			cerr << "报单重新报入:ReqOrderInsert:" << ((iResult == 0) ? "成功" : "失败") << endl;
			//记录报单录入信息
			string orderinsertstr = getInvestorOrderInsertInfoByDelimater(&req);
			LOG(INFO) << ("ReqOrderInsert caused by untraded order action.--->>>" + orderinsertstr);
			//返回客户端信息
			string tmpstr2;
			if (iResult == 0) {
				tmpstr2.append("businessType=110").append(sep).append("result=0").append(sep).append(orderinsertstr);
			} else {
				tmpstr2.append("businessType=110").append(sep).append("result=1").append(sep).append(orderinsertstr);
			}
			//tradequeue.push_back(tmpstr2);
		} else if (insType == 0 ) {//有可能为手动撤单,或者短线重启
			manualOrderActioin(pOrder);
		} else if (insType == 2 || insType == 4) {//活跃合约开仓撤单。当前逻辑表示活跃合约价格偏移后追单
            if (statusMsg == "当前状态禁止此项操作") {//收盘,多的部分需要手动平仓。以少的一边持仓为准进行找平。
                LOG(ERROR) << "当前状态禁止此项操作:收盘之后无法报单,该笔组合单需要手动删除补平。";
                manualOrderActioin(pOrder);
                return;
            }
			TradeInfo* tradeinfo = getOriginalTradeInfo(pOrder);
			string tmpstr = getArbDetail(tradeinfo);
			LOG(INFO) << ("追单时，原始报单情况" + tmpstr);
			 //报单结构体
			CThostFtdcInputOrderField req;
			string newOrderRef = boost::lexical_cast<string>(iOrderRef++);
			memset(&req, 0, sizeof(req));
			///经纪公司代码
			strcpy(req.BrokerID, BROKER_ID);
			///投资者代码
			strcpy(req.InvestorID, INVESTOR_ID);
			///合约代码
			strcpy(req.InstrumentID, pOrder->InstrumentID);
			///报单引用
			//string orderRef = boost::lexical_cast<string>(iOrderRef++);
			strcpy(req.OrderRef, newOrderRef.c_str());
			req.RequestID = iRequestID++;
			///用户代码
			//	TThostFtdcUserIDType	UserID;
			///报单价格条件: 限价;必须是追单价格
			req.OrderPriceType = pOrder->OrderPriceType;
			///买卖方向: 
			req.Direction = pOrder->Direction;
			///组合开平标志: 开仓
			req.CombOffsetFlag[0] = pOrder->CombOffsetFlag[0];
			///组合投机套保标志
			req.CombHedgeFlag[0] = pOrder->CombHedgeFlag[0];
			///价格
			req.LimitPrice = tradeinfo->activeOrderInsertPrice;
			///数量: 1
			req.VolumeTotalOriginal = pOrder->VolumeTotalOriginal;
			///有效期类型: 当日有效
			//req.TimeCondition = THOST_FTDC_TC_GFD;
			req.TimeCondition = pOrder->TimeCondition;
			///GTD日期
			//	TThostFtdcDateType	GTDDate;
			///成交量类型: 任何数量
			req.VolumeCondition = pOrder->VolumeCondition;
			///最小成交量: 1
			req.MinVolume = 1;
			///触发条件: 立即
			req.ContingentCondition = pOrder->ContingentCondition;
			///止损价
			//	TThostFtdcPriceType	StopPrice;
			///强平原因: 非强平
			req.ForceCloseReason = pOrder->ForceCloseReason;
			///自动挂起标志: 否
			req.IsAutoSuspend = 0;
			///用户强评标志: 否
			req.UserForceClose = 0;
			//委托类操作，使用客户端定义的请求编号格式
			int iResult = ptradeApi->ReqOrderInsert(&req, iRequestID++);
			if (iResult == 0) {//报入成功，需要更新对应套利合约报单的orderRef
				//更改追单状态:已追单
				tradeinfo->activeOrderActionStatus = boost::lexical_cast<string>("120");
				string rst = resetOrderRef(pOrder, tmpOrderRef, newOrderRef);
				string msg = "";
				if (rst == "1") {
					msg = "change instrumentID=" + string(pOrder->InstrumentID) + " orderRef success!";
				} else {
					msg = "change instrumentID=" + string(pOrder->InstrumentID) + " orderRef failed!";
				}
				LOG(INFO) << (msg);
			}
			cerr << "报单重新报入:ReqOrderInsert:" << ((iResult == 0) ? "成功" : "失败") << endl;
			//记录报单录入信息
			string orderinsertstr = getInvestorOrderInsertInfoByDelimater(&req);
			LOG(INFO) << ("ReqOrderInsert：追单撤单之后，追单限价单报入成功.--->>>" + orderinsertstr);
		}
	} else {//非撤单，修改报单状态
		if (orderStatus == 'a') {//未知单状态，锁定可平量
			processHowManyHoldsCanBeClose(pOrder, "lock");//锁定持仓
		} else if (orderStatus == '0') {//成交也需要释放锁定的持仓
			processHowManyHoldsCanBeClose(pOrder, "release");//释放持仓
		}
		string rst = processArbRtnOrder(pOrder);
		string msg = "";
		if (rst == "1") {
			msg = "process instrumentID=" + string(pOrder->InstrumentID) + " rtnOrder success!";
		} else {
			msg = "process instrumentID=" + string(pOrder->InstrumentID) + " rtnOrder failed!";
		}
		LOG(INFO) << (msg);
	}
}

///成交通知
void CTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	if (start_process == 0) {
		return;
	}
	cerr << "--->>> " << "OnRtnTrade" << endl;
	//锁持仓处理
	boost::recursive_mutex::scoped_lock SLock3(willTrade_mtx);
	//boost::recursive_mutex::scoped_lock SLock4(arbVolume_mtx);//加锁处理
	boost::recursive_mutex::scoped_lock SLock5(priceGap_mtx);//锁定
	//锁持仓处理
	boost::recursive_mutex::scoped_lock SLock2(stopProfit_mtx);
	//锁持仓处理
	boost::recursive_mutex::scoped_lock SLock(pst_mtx);
	boost::recursive_mutex::scoped_lock SLock6(alreadyTrade_mtx);
	//成交后判断是否是单边成交还是双边成交
	string rst = processArbRtnTrade(pTrade);
	string msg = "";
	if (rst == "1") {
		msg = "process instrumentID=" + string(pTrade->InstrumentID) + " OnRtnTrade success!";
	} else {
		msg = "process instrumentID=" + string(pTrade->InstrumentID) + " OnRtnTrade failed!";
	}
	//处理持仓情况
	processtrade(pTrade);
	LOG(INFO) << (msg);
	string tmpstr = getRtnTradeInfoByDelimater(pTrade);
	LOG(INFO) << ("--->>>OnRtnTrade:" + tmpstr);
	string tmpstr2;
	tmpstr2.append("businessType=200").append(sep).append("result=0").append(sep).append(tmpstr);
	tradequeue.push_back(tmpstr2);
}

void CTraderSpi::OnFrontDisconnected(int nReason)
{
	cerr << "--->>> " << "OnFrontDisconnected" << endl;
	cerr << "--->>> Reason = " << nReason << endl;
}

void CTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << "--->>> " << "OnHeartBeatWarning" << endl;
	cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void CTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << "------------------>>> " << "OnRspError" << endl;
	IsErrorRspInfo(pRspInfo);
}
//委托有错误时，才会有该报文；否则 pRspInfo本身就是空指针。
bool CTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult) {
		cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
		string tmpstr = "--->>> ErrorID=";
		char tmpchar[50];
        tmpstr.append(boost::lexical_cast<string>(pRspInfo->ErrorID));
		tmpstr.append(", ErrorMsg=");
		tmpstr.append(pRspInfo->ErrorMsg);
		LOG(INFO) << (tmpstr);
		tradequeue.push_back(tmpstr);
	}
	return bResult;
}

bool CTraderSpi::IsMyOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->FrontID == FRONT_ID) &&
		(pOrder->SessionID == SESSION_ID) &&
		(strcmp(pOrder->OrderRef, ORDER_REF) == 0));
}

bool CTraderSpi::IsTradingOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}
//提取投资者报单信息
string getInvestorOrderInsertInfo(CThostFtdcInputOrderField *order)
{
	///经纪公司代码
	char	*BrokerID = order->BrokerID;
	///投资者代码
	char	*InvestorID = order->InvestorID;
	///合约代码
	char	*InstrumentID = order->InstrumentID;
	///报单引用
	char	*OrderRef = order->OrderRef;
	///用户代码
	char	*UserID = order->UserID;
	///报单价格条件
	char	OrderPriceType = order->OrderPriceType;
	///买卖方向
	char	Direction[] = { order->Direction,'\0' };
	///组合开平标志
	char	*CombOffsetFlag = order->CombOffsetFlag;
	///组合投机套保标志
	char	*CombHedgeFlag = order->CombHedgeFlag;
	///价格
	TThostFtdcPriceType	limitPrice = order->LimitPrice;
	char LimitPrice[100];
	sprintf(LimitPrice, "%f", limitPrice);
	///数量
	TThostFtdcVolumeType	volumeTotalOriginal = order->VolumeTotalOriginal;
	char VolumeTotalOriginal[100];
	sprintf(VolumeTotalOriginal, "%d", volumeTotalOriginal);
	///有效期类型
	TThostFtdcTimeConditionType	TimeCondition = order->TimeCondition;
	///GTD日期
	//TThostFtdcDateType	GTDDate = order->GTDDate;
	///成交量类型
	TThostFtdcVolumeConditionType	VolumeCondition[] = { order->VolumeCondition,'\0' };
	///最小成交量
	TThostFtdcVolumeType	MinVolume = order->MinVolume;
	///触发条件
	TThostFtdcContingentConditionType	ContingentCondition = order->ContingentCondition;
	///止损价
	TThostFtdcPriceType	StopPrice = order->StopPrice;
	///强平原因
	TThostFtdcForceCloseReasonType	ForceCloseReason = order->ForceCloseReason;
	///自动挂起标志
	TThostFtdcBoolType	IsAutoSuspend = order->IsAutoSuspend;
	///业务单元
	//TThostFtdcBusinessUnitType	BusinessUnit = order->BusinessUnit;
	///请求编号
	TThostFtdcRequestIDType	requestID = order->RequestID;
	char RequestID[100];
	sprintf(RequestID, "%d", requestID);
	///用户强评标志
	TThostFtdcBoolType	UserForceClose = order->UserForceClose;

	string ordreInfo;
	if (!isOrderInsertDefFieldReady)
	{
		ordreInfo.append("BrokerID\t");
		ordreInfo.append("InvestorID\t");
		ordreInfo.append("InstrumentID\t");
		ordreInfo.append("OrderRef\t");
		ordreInfo.append("UserID\t");
		ordreInfo.append("Direction\t");
		ordreInfo.append("CombOffsetFlag\t");
		ordreInfo.append("CombHedgeFlag\t");
		ordreInfo.append("LimitPrice\t");
		ordreInfo.append("VolumeTotalOriginal\t");
		ordreInfo.append("VolumeCondition\t");
		ordreInfo.append("RequestID\n");
		isOrderInsertDefFieldReady = true;
	}
	ordreInfo.append(BrokerID); ordreInfo.append("\t");
	ordreInfo.append(InvestorID); ordreInfo.append("\t");
	ordreInfo.append(InstrumentID); ordreInfo.append("\t");
	ordreInfo.append(OrderRef); ordreInfo.append("\t");
	ordreInfo.append(UserID); ordreInfo.append("\t");
	ordreInfo.append(Direction); ordreInfo.append("\t");
	ordreInfo.append(CombOffsetFlag); ordreInfo.append("\t");
	ordreInfo.append(CombHedgeFlag); ordreInfo.append("\t");
	ordreInfo.append(LimitPrice); ordreInfo.append("\t");
	ordreInfo.append(VolumeTotalOriginal); ordreInfo.append("\t");
	ordreInfo.append(VolumeCondition); ordreInfo.append("\t");
	ordreInfo.append(RequestID); ordreInfo.append("\t");
	cout << ordreInfo << endl;
	LOG(INFO) << (ordreInfo);
	return ordreInfo;
}
//提取投资者报单信息
string getInvestorOrderInsertInfoByDelimater(CThostFtdcInputOrderField *order)
{
	///经纪公司代码
	char	*BrokerID = order->BrokerID;
	///投资者代码
	char	*InvestorID = order->InvestorID;
	///合约代码
	char	*InstrumentID = order->InstrumentID;
	///报单引用
	char	*OrderRef = order->OrderRef;
	///用户代码
	char	*UserID = order->UserID;
	///报单价格条件
	char	OrderPriceType = order->OrderPriceType;
	///买卖方向
	char	Direction[] = { order->Direction,'\0' };
	///组合开平标志
	char	*CombOffsetFlag = order->CombOffsetFlag;
	///组合投机套保标志
	char	*CombHedgeFlag = order->CombHedgeFlag;
	///价格
	TThostFtdcPriceType	limitPrice = order->LimitPrice;
	char LimitPrice[100];
	sprintf(LimitPrice, "%f", limitPrice);
	///数量
	TThostFtdcVolumeType	volumeTotalOriginal = order->VolumeTotalOriginal;
	char VolumeTotalOriginal[100];
	sprintf(VolumeTotalOriginal, "%d", volumeTotalOriginal);
	///有效期类型
	TThostFtdcTimeConditionType	TimeCondition = order->TimeCondition;
	///GTD日期
	//TThostFtdcDateType	GTDDate = order->GTDDate;
	///成交量类型
	TThostFtdcVolumeConditionType	VolumeCondition[] = { order->VolumeCondition,'\0' };
	///最小成交量
	TThostFtdcVolumeType	MinVolume = order->MinVolume;
	///触发条件
	TThostFtdcContingentConditionType	ContingentCondition = order->ContingentCondition;
	///止损价
	TThostFtdcPriceType	StopPrice = order->StopPrice;
	///强平原因
	TThostFtdcForceCloseReasonType	ForceCloseReason = order->ForceCloseReason;
	///自动挂起标志
	TThostFtdcBoolType	IsAutoSuspend = order->IsAutoSuspend;
	///业务单元
	//TThostFtdcBusinessUnitType	BusinessUnit = order->BusinessUnit;
	///请求编号
	TThostFtdcRequestIDType	requestID = order->RequestID;
	char RequestID[100];
	sprintf(RequestID, "%d", requestID);
	///用户强评标志
	TThostFtdcBoolType	UserForceClose = order->UserForceClose;

	string ordreInfo;
	if (!isOrderInsertDefFieldReady)
	{
		// 		ordreInfo.append("BrokerID\t");
		// 		ordreInfo.append("InvestorID\t");
		// 		ordreInfo.append("InstrumentID\t");
		// 		ordreInfo.append("OrderRef\t");
		// 		ordreInfo.append("UserID\t");
		// 		ordreInfo.append("Direction\t");
		// 		ordreInfo.append("CombOffsetFlag\t");
		// 		ordreInfo.append("CombHedgeFlag\t");
		// 		ordreInfo.append("LimitPrice\t");
		// 		ordreInfo.append("VolumeTotalOriginal\t");
		// 		ordreInfo.append("VolumeCondition\t");
		// 		ordreInfo.append("RequestID\n");
		isOrderInsertDefFieldReady = true;
	}
	ordreInfo.append("BrokerID="); ordreInfo.append(BrokerID); ordreInfo.append(sep);
	ordreInfo.append("InvestorID="); ordreInfo.append(InvestorID); ordreInfo.append(sep);
	ordreInfo.append("InstrumentID="); ordreInfo.append(InstrumentID); ordreInfo.append(sep);
	ordreInfo.append("OrderRef="); ordreInfo.append(OrderRef); ordreInfo.append(sep);
	ordreInfo.append("UserID="); ordreInfo.append(UserID); ordreInfo.append(sep);
	ordreInfo.append("Direction="); ordreInfo.append(Direction); ordreInfo.append(sep);
	ordreInfo.append("CombOffsetFlag="); ordreInfo.append(CombOffsetFlag); ordreInfo.append(sep);
	ordreInfo.append("CombHedgeFlag="); ordreInfo.append(CombHedgeFlag); ordreInfo.append(sep);
	ordreInfo.append("LimitPrice="); ordreInfo.append(LimitPrice); ordreInfo.append(sep);
	ordreInfo.append("VolumeTotalOriginal="); ordreInfo.append(VolumeTotalOriginal); ordreInfo.append(sep);
	ordreInfo.append("VolumeCondition="); ordreInfo.append(VolumeCondition); ordreInfo.append(sep);
	ordreInfo.append("RequestID="); ordreInfo.append(RequestID); ordreInfo.append(sep);
	cout << ordreInfo << endl;
	//LOG(INFO)<<(ordreInfo);
	return ordreInfo;
}
//提取委托回报信息
string getRtnOrderInfoByDelimater(CThostFtdcOrderField *order)
{
	///经纪公司代码
	char	*BrokerID = order->BrokerID;
	///投资者代码
	char	*InvestorID = order->InvestorID;
	///合约代码
	char	*InstrumentID = order->InstrumentID;
	///报单引用
	char	*OrderRef = order->OrderRef;
	///用户代码
	char	*UserID = order->UserID;
	///报单价格条件
	char	OrderPriceType = order->OrderPriceType;
	///买卖方向
	char	Direction[] = { order->Direction,'\0' };
	///组合开平标志
	char	*CombOffsetFlag = order->CombOffsetFlag;
	///组合投机套保标志
	char	*CombHedgeFlag = order->CombHedgeFlag;
	///价格
	TThostFtdcPriceType	limitPrice = order->LimitPrice;
	char LimitPrice[100];
	sprintf(LimitPrice, "%f", limitPrice);
	///数量
	TThostFtdcVolumeType	volumeTotalOriginal = order->VolumeTotalOriginal;
	char VolumeTotalOriginal[100];
	sprintf(VolumeTotalOriginal, "%d", volumeTotalOriginal);
	///有效期类型
	TThostFtdcTimeConditionType	TimeCondition = order->TimeCondition;
	///GTD日期
	//TThostFtdcDateType	GTDDate = order->GTDDate;
	///成交量类型
	TThostFtdcVolumeConditionType	VolumeCondition[] = { order->VolumeCondition,'\0' };
	///最小成交量
	TThostFtdcVolumeType	minVolume = order->MinVolume;
	char MinVolume[100];
	sprintf(MinVolume, "%d", minVolume);
	///触发条件
	TThostFtdcContingentConditionType	ContingentCondition = order->ContingentCondition;
	///止损价
	TThostFtdcPriceType	stopPrice = order->StopPrice;
	char StopPrice[100];
	sprintf(StopPrice, "%d", stopPrice);
	///强平原因
	TThostFtdcForceCloseReasonType	ForceCloseReason = order->ForceCloseReason;
	///自动挂起标志
	TThostFtdcBoolType	isAutoSuspend = order->IsAutoSuspend;
	char IsAutoSuspend[100];
	sprintf(IsAutoSuspend, "%d", isAutoSuspend);
	///业务单元
	//TThostFtdcBusinessUnitType	BusinessUnit = order->BusinessUnit;
	///请求编号
	TThostFtdcRequestIDType	requestID = order->RequestID;
	char RequestID[100];
	sprintf(RequestID, "%d", requestID);
	///本地报单编号
    char *OrderLocalID = order->OrderLocalID;
	///交易所代码
	char * ExchangeID = order->ExchangeID;
	///会员代码
	char* ParticipantID = order->ParticipantID;
	///客户代码
	char*	ClientID = order->ClientID;
	///合约在交易所的代码
	char*	ExchangeInstID = order->ExchangeInstID;
	///交易所交易员代码
	char*	TraderID = order->TraderID;
	///安装编号
	int	installID = order->InstallID;
	char InstallID[10];
	sprintf(InstallID, "%d", installID);
	///报单提交状态
	char	OrderSubmitStatus[] = { order->OrderSubmitStatus,'\0' };
	///报单提示序号
	int	notifySequence = order->NotifySequence;
	char NotifySequence[20];
	sprintf(NotifySequence, "%d", notifySequence);
	///交易日
	char*	TradingDay = order->TradingDay;
	///结算编号
	int	settlementID = order->SettlementID;
	char SettlementID[10];
	sprintf(SettlementID, "%d", settlementID);
	///报单编号
	char*	OrderSysID = order->OrderSysID;
	///报单来源
	char	OrderSource[] = { order->OrderSource,'\0' };
	///报单状态
	char	OrderStatus[] = { order->OrderStatus,'\0' };
	///报单类型
	//TThostFtdcOrderTypeType	OrderType;
	///今成交数量
	int	volumeTraded = order->VolumeTraded;
	char VolumeTraded[10];
	sprintf(VolumeTraded, "%d", volumeTraded);
	///剩余数量
	int	volumeTotal = order->VolumeTotal;
	char VolumeTotal[10];
	sprintf(VolumeTotal, "%d", volumeTotal);
	///报单日期
	char*	InsertDate = order->InsertDate;
	///委托时间
	char*	InsertTime = order->InsertTime;
	///激活时间
	char*	ActiveTime = order->ActiveTime;
	///挂起时间
	char*	SuspendTime = order->SuspendTime;
	///最后修改时间
	char*	UpdateTime = order->UpdateTime;
	///撤销时间
	char*	CancelTime = order->CancelTime;
	///最后修改交易所交易员代码
	char*	ActiveTraderID = order->ActiveTraderID;
	///结算会员编号
	char*	ClearingPartID = order->ClearingPartID;
	///序号
	int	sequenceNo = order->SequenceNo;
	char SequenceNo[20];
	sprintf(SequenceNo, "%d", sequenceNo);
	///前置编号
	int	frontID = order->FrontID;
	char FrontID[20];
	sprintf(FrontID, "%d", frontID);
	///会话编号
	int	sessionID = order->SessionID;
	char SessionID[20];
	sprintf(SessionID, "%d", sessionID);
	///用户端产品信息
	char*	UserProductInfo = order->UserProductInfo;
	///状态信息
	char*	StatusMsg = order->StatusMsg;
	///用户强评标志
	int	userForceClose = order->UserForceClose;
	char UserForceClose[20];
	sprintf(UserForceClose, "%d", userForceClose);
	///操作用户代码
	char*	ActiveUserID = order->ActiveUserID;
	///经纪公司报单编号
	int	brokerOrderSeq = order->BrokerOrderSeq;
	char BrokerOrderSeq[20];
	sprintf(BrokerOrderSeq, "%d", brokerOrderSeq);


	string ordreInfo;
	ordreInfo.append("BrokerID="); ordreInfo.append(BrokerID); ordreInfo.append(sep);
	ordreInfo.append("InvestorID="); ordreInfo.append(InvestorID); ordreInfo.append(sep);
	ordreInfo.append("InstrumentID="); ordreInfo.append(InstrumentID); ordreInfo.append(sep);
	ordreInfo.append("OrderRef="); ordreInfo.append(OrderRef); ordreInfo.append(sep);
	ordreInfo.append("UserID="); ordreInfo.append(UserID); ordreInfo.append(sep);
	//ordreInfo.append("OrderPriceType=");ordreInfo.append(OrderPriceType);ordreInfo.append(sep);
	ordreInfo.append("Direction="); ordreInfo.append(Direction); ordreInfo.append(sep);
	ordreInfo.append("CombOffsetFlag="); ordreInfo.append(CombOffsetFlag); ordreInfo.append(sep);
	ordreInfo.append("CombHedgeFlag="); ordreInfo.append(CombHedgeFlag); ordreInfo.append(sep);
	ordreInfo.append("LimitPrice="); ordreInfo.append(LimitPrice); ordreInfo.append(sep);
	ordreInfo.append("VolumeTotalOriginal="); ordreInfo.append(VolumeTotalOriginal); ordreInfo.append(sep);
	//ordreInfo.append("TimeCondition=");ordreInfo.append(TimeCondition);ordreInfo.append(sep);
	ordreInfo.append("MinVolume="); ordreInfo.append(MinVolume); ordreInfo.append(sep);
	ordreInfo.append("VolumeCondition="); ordreInfo.append(VolumeCondition); ordreInfo.append(sep);
	//ordreInfo.append("ContingentCondition=");ordreInfo.append(ContingentCondition);ordreInfo.append(sep);
	ordreInfo.append("StopPrice="); ordreInfo.append(StopPrice); ordreInfo.append(sep);
	//ordreInfo.append("ForceCloseReason=");ordreInfo.append(ForceCloseReason);ordreInfo.append(sep);
	ordreInfo.append("IsAutoSuspend="); ordreInfo.append(IsAutoSuspend); ordreInfo.append(sep);
	ordreInfo.append("RequestID="); ordreInfo.append(RequestID); ordreInfo.append(sep);

	ordreInfo.append("OrderLocalID="); ordreInfo.append(OrderLocalID); ordreInfo.append(sep);
	ordreInfo.append("ExchangeID="); ordreInfo.append(ExchangeID); ordreInfo.append(sep);
	ordreInfo.append("ParticipantID="); ordreInfo.append(ParticipantID); ordreInfo.append(sep);
	ordreInfo.append("ClientID="); ordreInfo.append(ClientID); ordreInfo.append(sep);
	ordreInfo.append("ExchangeInstID="); ordreInfo.append(ExchangeInstID); ordreInfo.append(sep);
	ordreInfo.append("TraderID="); ordreInfo.append(TraderID); ordreInfo.append(sep);
	ordreInfo.append("InstallID="); ordreInfo.append(InstallID); ordreInfo.append(sep);
	ordreInfo.append("OrderSubmitStatus="); ordreInfo.append(OrderSubmitStatus); ordreInfo.append(sep);
	ordreInfo.append("NotifySequence="); ordreInfo.append(NotifySequence); ordreInfo.append(sep);
	ordreInfo.append("TradingDay="); ordreInfo.append(TradingDay); ordreInfo.append(sep);
	ordreInfo.append("SettlementID="); ordreInfo.append(SettlementID); ordreInfo.append(sep);
	ordreInfo.append("OrderSysID="); ordreInfo.append(OrderSysID); ordreInfo.append(sep);
	ordreInfo.append("OrderSource="); ordreInfo.append(OrderSource); ordreInfo.append(sep);
	ordreInfo.append("OrderStatus="); ordreInfo.append(OrderStatus); ordreInfo.append(sep);
	ordreInfo.append("VolumeTraded="); ordreInfo.append(VolumeTraded); ordreInfo.append(sep);
	ordreInfo.append("VolumeTotal="); ordreInfo.append(VolumeTotal); ordreInfo.append(sep);
	ordreInfo.append("InsertDate="); ordreInfo.append(InsertDate); ordreInfo.append(sep);
	ordreInfo.append("InsertTime="); ordreInfo.append(InsertTime); ordreInfo.append(sep);
	ordreInfo.append("ActiveTime="); ordreInfo.append(ActiveTime); ordreInfo.append(sep);
	ordreInfo.append("SuspendTime="); ordreInfo.append(SuspendTime); ordreInfo.append(sep);
	ordreInfo.append("UpdateTime="); ordreInfo.append(UpdateTime); ordreInfo.append(sep);
	ordreInfo.append("CancelTime="); ordreInfo.append(CancelTime); ordreInfo.append(sep);
	ordreInfo.append("ActiveTraderID="); ordreInfo.append(ActiveTraderID); ordreInfo.append(sep);
	ordreInfo.append("ClearingPartID="); ordreInfo.append(ClearingPartID); ordreInfo.append(sep);
	ordreInfo.append("SequenceNo="); ordreInfo.append(SequenceNo); ordreInfo.append(sep);
	ordreInfo.append("FrontID="); ordreInfo.append(FrontID); ordreInfo.append(sep);
	ordreInfo.append("SessionID="); ordreInfo.append(SessionID); ordreInfo.append(sep);
	ordreInfo.append("UserProductInfo="); ordreInfo.append(UserProductInfo); ordreInfo.append(sep);
	ordreInfo.append("StatusMsg="); ordreInfo.append(StatusMsg); ordreInfo.append(sep);
	ordreInfo.append("UserForceClose="); ordreInfo.append(UserForceClose); ordreInfo.append(sep);
	ordreInfo.append("ActiveUserID="); ordreInfo.append(ActiveUserID); ordreInfo.append(sep);
	ordreInfo.append("BrokerOrderSeq="); ordreInfo.append(BrokerOrderSeq); ordreInfo.append(sep);

	cout << ordreInfo << endl;
	//LOG(INFO)<<(ordreInfo);
	return ordreInfo;
}
//提取成交回报信息
string getRtnTradeInfoByDelimater(CThostFtdcTradeField *order)
{
	///经纪公司代码
	char	*BrokerID = order->BrokerID;
	///投资者代码
	char	*InvestorID = order->InvestorID;
	///合约代码
	char	*InstrumentID = order->InstrumentID;
	///报单引用
	char	*OrderRef = order->OrderRef;
	///用户代码
	char	*UserID = order->UserID;
	///交易所代码
	char * ExchangeID = order->ExchangeID;
	///成交编号
	char*	TradeID = order->TradeID;
	///买卖方向
	char	Direction[] = { order->Direction,'\0' };
	///报单编号
	char*	OrderSysID = order->OrderSysID;
	//会员代码
	char*	ParticipantID = order->ParticipantID;
	///客户代码
	char*	ClientID = order->ClientID;
	///合约在交易所的代码
	char*	ExchangeInstID = order->ExchangeInstID;
	///开平标志
	char	OffsetFlag[] = { order->OffsetFlag,'\0' };
	///投机套保标志
	char	HedgeFlag[] = { order->HedgeFlag,'\0' };
	///价格
	TThostFtdcPriceType	price = order->Price;
	char Price[100];
	sprintf(Price, "%f", price);
	///数量
	TThostFtdcVolumeType	volume = order->Volume;
	char Volume[100];
	sprintf(Volume, "%d", volume);
	///成交时期
	char*	TradeDate = order->TradeDate;
	///成交时间
	char*	TradeTime = order->TradeTime;
	///成交类型
	char	TradeType[] = { order->TradeType,'\0' };
	///成交价来源
	char	PriceSource[] = { order->PriceSource,'\0' };
	///交易所交易员代码
	char*	TraderID = order->TraderID;
	///本地报单编号
    char *OrderLocalID = order->OrderLocalID;
	///结算编号
	int	settlementID = order->SettlementID;
	char SettlementID[10];
	sprintf(SettlementID, "%d", settlementID);
	///结算会员编号
	char*	ClearingPartID = order->ClearingPartID;
	///序号
	int	sequenceNo = order->SequenceNo;
	char SequenceNo[20];
	sprintf(SequenceNo, "%d", sequenceNo);
	///交易日
	char*	TradingDay = order->TradingDay;
	///经纪公司报单编号
	int	brokerOrderSeq = order->BrokerOrderSeq;
	char BrokerOrderSeq[20];
	sprintf(BrokerOrderSeq, "%d", brokerOrderSeq);


	string ordreInfo;
	ordreInfo.append("BrokerID="); ordreInfo.append(BrokerID); ordreInfo.append(sep);
	ordreInfo.append("InvestorID="); ordreInfo.append(InvestorID); ordreInfo.append(sep);
	ordreInfo.append("InstrumentID="); ordreInfo.append(InstrumentID); ordreInfo.append(sep);
	ordreInfo.append("OrderRef="); ordreInfo.append(OrderRef); ordreInfo.append(sep);
	ordreInfo.append("UserID="); ordreInfo.append(UserID); ordreInfo.append(sep);
	ordreInfo.append("ExchangeID="); ordreInfo.append(ExchangeID); ordreInfo.append(sep);
	ordreInfo.append("TradeID="); ordreInfo.append(TradeID); ordreInfo.append(sep);
	ordreInfo.append("Direction="); ordreInfo.append(Direction); ordreInfo.append(sep);
	ordreInfo.append("OrderSysID="); ordreInfo.append(OrderSysID); ordreInfo.append(sep);
	ordreInfo.append("ParticipantID="); ordreInfo.append(ParticipantID); ordreInfo.append(sep);
	ordreInfo.append("ClientID="); ordreInfo.append(ClientID); ordreInfo.append(sep);
	ordreInfo.append("ExchangeInstID="); ordreInfo.append(ExchangeInstID); ordreInfo.append(sep);
	ordreInfo.append("OffsetFlag="); ordreInfo.append(OffsetFlag); ordreInfo.append(sep);
	ordreInfo.append("HedgeFlag="); ordreInfo.append(HedgeFlag); ordreInfo.append(sep);
	ordreInfo.append("Price="); ordreInfo.append(Price); ordreInfo.append(sep);
	ordreInfo.append("Volume="); ordreInfo.append(Volume); ordreInfo.append(sep);
	ordreInfo.append("TradeDate="); ordreInfo.append(TradeDate); ordreInfo.append(sep);
	ordreInfo.append("TradeTime="); ordreInfo.append(TradeTime); ordreInfo.append(sep);
	ordreInfo.append("TradeType="); ordreInfo.append(TradeType); ordreInfo.append(sep);
	ordreInfo.append("PriceSource="); ordreInfo.append(PriceSource); ordreInfo.append(sep);

	ordreInfo.append("TraderID="); ordreInfo.append(TraderID); ordreInfo.append(sep);
	ordreInfo.append("OrderLocalID="); ordreInfo.append(OrderLocalID); ordreInfo.append(sep);
	ordreInfo.append("ClearingPartID="); ordreInfo.append(ClearingPartID); ordreInfo.append(sep);
	//ordreInfo.append("BusinessUnit=");ordreInfo.append(BusinessUnit);ordreInfo.append(sep);
	//ordreInfo.append("TimeCondition=");ordreInfo.append(TimeCondition);ordreInfo.append(sep);
	ordreInfo.append("SequenceNo="); ordreInfo.append(SequenceNo); ordreInfo.append(sep);
	ordreInfo.append("TradingDay="); ordreInfo.append(TradingDay); ordreInfo.append(sep);
	//ordreInfo.append("ContingentCondition=");ordreInfo.append(ContingentCondition);ordreInfo.append(sep);
	ordreInfo.append("SettlementID="); ordreInfo.append(SettlementID); ordreInfo.append(sep);
	//ordreInfo.append("ForceCloseReason=");ordreInfo.append(ForceCloseReason);ordreInfo.append(sep);
	ordreInfo.append("BrokerOrderSeq="); ordreInfo.append(BrokerOrderSeq); ordreInfo.append(sep);

	cout << ordreInfo << endl;
	//LOG(INFO)<<(ordreInfo);
	return ordreInfo;
}
//提取投资者报单信息
string getOrderActionInfoByDelimater(CThostFtdcInputOrderActionField *order)
{
	///经纪公司代码
	char	*BrokerID = order->BrokerID;
	///投资者代码
	char	*InvestorID = order->InvestorID;
	///合约代码
	char	*InstrumentID = order->InstrumentID;
	///报单操作引用
	TThostFtdcOrderActionRefType	orderActionRef = order->OrderActionRef;
	char OrderActionRef[20];
	sprintf(OrderActionRef, "%d", orderActionRef);
	///报单引用
	char	*OrderRef = order->OrderRef;
	///用户代码
	char	*UserID = order->UserID;
	///请求编号
	TThostFtdcRequestIDType	requestID = order->RequestID;
	char RequestID[100];
	sprintf(RequestID, "%d", requestID);
	///前置编号 int
	TThostFtdcFrontIDType	frontID = order->FrontID;
	char FrontID[20];
	sprintf(FrontID, "%d", frontID);
	///会话编号 int
	TThostFtdcSessionIDType	sessionID = order->SessionID;
	char SessionID[20];
	sprintf(SessionID, "%d", sessionID);
	///交易所代码
	char*	ExchangeID = order->ExchangeID;
	///报单编号
	char*	OrderSysID = order->OrderSysID;
	///操作标志
	char	ActionFlag[] = { order->ActionFlag,'\0' };
	///价格 double
	string LimitPrice = boost::lexical_cast<string>(order->LimitPrice);
	///数量变化int
	string VolumeChange = boost::lexical_cast<string>(order->VolumeChange);

	string ordreInfo;
	ordreInfo.append("BrokerID="); ordreInfo.append(BrokerID); ordreInfo.append(sep);
	ordreInfo.append("InvestorID="); ordreInfo.append(InvestorID); ordreInfo.append(sep);
	ordreInfo.append("InstrumentID="); ordreInfo.append(InstrumentID); ordreInfo.append(sep);
	ordreInfo.append("OrderActionRef="); ordreInfo.append(OrderActionRef); ordreInfo.append(sep);
	ordreInfo.append("OrderRef="); ordreInfo.append(OrderRef); ordreInfo.append(sep);
	ordreInfo.append("UserID="); ordreInfo.append(UserID); ordreInfo.append(sep);
	ordreInfo.append("RequestID="); ordreInfo.append(RequestID); ordreInfo.append(sep);
	ordreInfo.append("FrontID="); ordreInfo.append(FrontID); ordreInfo.append(sep);
	ordreInfo.append("SessionID="); ordreInfo.append(SessionID); ordreInfo.append(sep);
	ordreInfo.append("ExchangeID="); ordreInfo.append(ExchangeID); ordreInfo.append(sep);
	ordreInfo.append("OrderSysID="); ordreInfo.append(OrderSysID); ordreInfo.append(sep);
	ordreInfo.append("ActionFlag="); ordreInfo.append(ActionFlag); ordreInfo.append(sep);
	ordreInfo.append("LimitPrice="); ordreInfo.append(LimitPrice); ordreInfo.append(sep);
	ordreInfo.append("VolumeChange="); ordreInfo.append(VolumeChange); ordreInfo.append(sep);
	cout << ordreInfo << endl;
	//LOG(INFO)<<(ordreInfo);
	return ordreInfo;
}
//将报单回报响应信息写入文件保存
void saveRspOrderInsertInfo(CThostFtdcInputOrderField *pInputOrder)
{
	saveInvestorOrderInsertHedge(pInputOrder, "d:\\test\\rsporderinsert.txt");
}
//获取交易所回报响应
string getRtnOrder(CThostFtdcOrderField *pOrder)
{
	///经纪公司代码
	char	*BrokerID = pOrder->BrokerID;
	///投资者代码
	char	*InvestorID = pOrder->InvestorID;
	///合约代码
	char	*InstrumentID = pOrder->InstrumentID;
	///报单引用
	char	*OrderRef = pOrder->OrderRef;
	///用户代码
	TThostFtdcUserIDType	UserID;
	///报单价格条件
	TThostFtdcOrderPriceTypeType	OrderPriceType;
	///买卖方向
	TThostFtdcDirectionType	Direction = pOrder->Direction;
	///组合开平标志
	TThostFtdcCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
	TThostFtdcCombHedgeFlagType	CombHedgeFlag;
	///价格
	TThostFtdcPriceType	LimitPrice;
	///数量
	TThostFtdcVolumeType	VolumeTotalOriginal;
	///有效期类型
	TThostFtdcTimeConditionType	TimeCondition;
	///GTD日期
	TThostFtdcDateType	GTDDate;
	///成交量类型
	TThostFtdcVolumeConditionType	VolumeCondition;
	///最小成交量
	TThostFtdcVolumeType	MinVolume;
	///触发条件
	TThostFtdcContingentConditionType	ContingentCondition;
	///止损价
	TThostFtdcPriceType	StopPrice;
	///强平原因
	TThostFtdcForceCloseReasonType	ForceCloseReason;
	///自动挂起标志
	TThostFtdcBoolType	IsAutoSuspend;
	///业务单元
	TThostFtdcBusinessUnitType	BusinessUnit;
	///请求编号
	TThostFtdcRequestIDType	RequestID = pOrder->RequestID;
	char cRequestId[100];
	sprintf(cRequestId, "%d", RequestID);
	///本地报单编号
	char	*OrderLocalID = pOrder->OrderLocalID;
	///交易所代码
	TThostFtdcExchangeIDType	ExchangeID;
	///会员代码
	TThostFtdcParticipantIDType	ParticipantID;
	///客户代码
	char	*ClientID = pOrder->ClientID;
	///合约在交易所的代码
	TThostFtdcExchangeInstIDType	ExchangeInstID;
	///交易所交易员代码
	TThostFtdcTraderIDType	TraderID;
	///安装编号
	TThostFtdcInstallIDType	InstallID;
	///报单提交状态
	char	OrderSubmitStatus = pOrder->OrderSubmitStatus;
	char cOrderSubmitStatus[] = { OrderSubmitStatus,'\0' };
	//sprintf(cOrderSubmitStatus,"%s",OrderSubmitStatus);
	///报单状态
	TThostFtdcOrderStatusType	OrderStatus = pOrder->OrderStatus;
	char cOrderStatus[] = { OrderStatus,'\0' };
	//sprintf(cOrderStatus,"%s",OrderStatus);
	///报单提示序号
	TThostFtdcSequenceNoType	NotifySequence;
	///交易日
	TThostFtdcDateType	TradingDay;
	///结算编号
	TThostFtdcSettlementIDType	SettlementID;
	///报单编号
	char	*OrderSysID = pOrder->OrderSysID;
	char replace[] = { 'n','\0' };
	if (strlen(OrderSysID) == 0)
	{
		OrderSysID = replace;
	}
	///报单来源
	TThostFtdcOrderSourceType	OrderSource;
	///报单类型
	TThostFtdcOrderTypeType	OrderType;
	///今成交数量
	TThostFtdcVolumeType	VolumeTraded = pOrder->VolumeTraded;
	char cVolumeTraded[100];
	sprintf(cVolumeTraded, "%d", VolumeTraded);
	///剩余数量
	TThostFtdcVolumeType	VolumeTotal = pOrder->VolumeTotal;
	char iVolumeTotal[100];
	sprintf(iVolumeTotal, "%d", VolumeTotal);
	///报单日期
	TThostFtdcDateType	InsertDate;
	///委托时间
	TThostFtdcTimeType	InsertTime;
	///激活时间
	TThostFtdcTimeType	ActiveTime;
	///挂起时间
	TThostFtdcTimeType	SuspendTime;
	///最后修改时间
	TThostFtdcTimeType	UpdateTime;
	///撤销时间
	TThostFtdcTimeType	CancelTime;
	///最后修改交易所交易员代码
	TThostFtdcTraderIDType	ActiveTraderID;
	///结算会员编号
	TThostFtdcParticipantIDType	ClearingPartID;
	///序号
	TThostFtdcSequenceNoType	SequenceNo;
	///前置编号
	TThostFtdcFrontIDType	FrontID;
	///会话编号
	TThostFtdcSessionIDType	SessionID;
	///用户端产品信息
	TThostFtdcProductInfoType	UserProductInfo;
	///状态信息
	char	*StatusMsg = pOrder->StatusMsg;
	///用户强评标志
	TThostFtdcBoolType	UserForceClose;
	///操作用户代码
	TThostFtdcUserIDType	ActiveUserID;
	///经纪公司报单编号
	TThostFtdcSequenceNoType	BrokerOrderSeq;
	string info;
	info.append(BrokerID); info.append("\t");
	info.append(InvestorID); info.append("\t");
	info.append(InstrumentID); info.append("\t");
	info.append(OrderRef); info.append("\t");
	info.append(cRequestId); info.append("\t");
	info.append(ClientID); info.append("\t");
	info.append(OrderSysID); info.append("\t");
	info.append(cVolumeTraded); info.append("\t");
	info.append(iVolumeTotal); info.append("\t");
	info.append(StatusMsg); info.append("\t");
	info.append(cOrderSubmitStatus); info.append("\t");
	info.append(cOrderStatus); info.append("\t");
	return info;
}

//将交易所报单回报响应写入文件保存
void saveRtnOrder(CThostFtdcOrderField *pOrder)
{
	string info = getRtnOrder(pOrder);
	LOG(INFO) << (info);
}
//将投资者持仓信息写入文件保存
int storeInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition)
{
	///合约代码
	char	*InstrumentID = pInvestorPosition->InstrumentID;
	///经纪公司代码
	char	*BrokerID = pInvestorPosition->BrokerID;
	///投资者代码
	char	*InvestorID = pInvestorPosition->InvestorID;
	///持仓多空方向
	TThostFtdcPosiDirectionType	dir = pInvestorPosition->PosiDirection;
	char PosiDirection[] = { dir,'\0' };
	///投机套保标志
	TThostFtdcHedgeFlagType	flag = pInvestorPosition->HedgeFlag;
	char HedgeFlag[] = { flag,'\0' };
	///持仓日期
	TThostFtdcPositionDateType	positionDate = pInvestorPosition->PositionDate;
	char PositionDate[] = { positionDate,'\0' };
	///上日持仓
	TThostFtdcVolumeType	ydPosition = pInvestorPosition->YdPosition;
	char YdPosition[100];
	sprintf(YdPosition, "%d", ydPosition);
	///今日持仓
	TThostFtdcVolumeType	position = pInvestorPosition->Position;
	if (position == 0)
	{
		return 0;
	}
	char Position[100];
	sprintf(Position, "%d", position);
	///多头冻结
	TThostFtdcVolumeType	LongFrozen = pInvestorPosition->LongFrozen;
	///空头冻结
	TThostFtdcVolumeType	ShortFrozen = pInvestorPosition->ShortFrozen;
	///开仓冻结金额
	TThostFtdcMoneyType	LongFrozenAmount = pInvestorPosition->LongFrozenAmount;
	///开仓冻结金额
	TThostFtdcMoneyType	ShortFrozenAmount = pInvestorPosition->ShortFrozenAmount;
	///开仓量
	TThostFtdcVolumeType	openVolume = pInvestorPosition->OpenVolume;
	char OpenVolume[100];
	sprintf(OpenVolume, "%d", openVolume);
	///平仓量
	TThostFtdcVolumeType	closeVolume = pInvestorPosition->CloseVolume;
	char CloseVolume[100];
	sprintf(CloseVolume, "%d", closeVolume);
	///开仓金额
	TThostFtdcMoneyType	OpenAmount = pInvestorPosition->OpenAmount;
	///平仓金额
	TThostFtdcMoneyType	CloseAmount = pInvestorPosition->CloseAmount;
	///持仓成本
	TThostFtdcMoneyType	positionCost = pInvestorPosition->PositionCost;
	char PositionCost[100];
	sprintf(PositionCost, "%f", positionCost);
	///上次占用的保证金
	TThostFtdcMoneyType	PreMargin = pInvestorPosition->PreMargin;
	///占用的保证金
	TThostFtdcMoneyType	UseMargin = pInvestorPosition->UseMargin;
	///冻结的保证金
	TThostFtdcMoneyType	FrozenMargin = pInvestorPosition->FrozenMargin;
	///冻结的资金
	TThostFtdcMoneyType	FrozenCash = pInvestorPosition->FrozenCash;
	///冻结的手续费
	TThostFtdcMoneyType	FrozenCommission = pInvestorPosition->FrozenCommission;
	///资金差额
	TThostFtdcMoneyType	CashIn = pInvestorPosition->CashIn;
	///手续费
	TThostFtdcMoneyType	Commission = pInvestorPosition->Commission;
	///平仓盈亏
	TThostFtdcMoneyType	CloseProfit = pInvestorPosition->CloseProfit;
	///持仓盈亏
	TThostFtdcMoneyType	PositionProfit = pInvestorPosition->PositionProfit;
	///上次结算价
	TThostFtdcPriceType	preSettlementPrice = pInvestorPosition->PreSettlementPrice;
	char PreSettlementPrice[100];
	sprintf(PreSettlementPrice, "%f", preSettlementPrice);
	///本次结算价
	TThostFtdcPriceType	SettlementPrice = pInvestorPosition->PreSettlementPrice;
	///交易日
	char	*TradingDay = pInvestorPosition->TradingDay;
	///结算编号
	TThostFtdcSettlementIDType	SettlementID;
	///开仓成本
	TThostFtdcMoneyType	openCost = pInvestorPosition->OpenCost;
	char OpenCost[100];
	sprintf(OpenCost, "%f", openCost);
	///交易所保证金
	TThostFtdcMoneyType	exchangeMargin = pInvestorPosition->ExchangeMargin;
	char ExchangeMargin[100];
	sprintf(ExchangeMargin, "%f", exchangeMargin);
	///组合成交形成的持仓
	TThostFtdcVolumeType	CombPosition;
	///组合多头冻结
	TThostFtdcVolumeType	CombLongFrozen;
	///组合空头冻结
	TThostFtdcVolumeType	CombShortFrozen;
	///逐日盯市平仓盈亏
	TThostFtdcMoneyType	CloseProfitByDate = pInvestorPosition->CloseProfitByDate;
	///逐笔对冲平仓盈亏
	TThostFtdcMoneyType	CloseProfitByTrade = pInvestorPosition->CloseProfitByTrade;
	///今日持仓
	TThostFtdcVolumeType	todayPosition = pInvestorPosition->TodayPosition;
	char TodayPosition[100];
	sprintf(TodayPosition, "%d", todayPosition);
	///保证金率
	TThostFtdcRatioType	marginRateByMoney = pInvestorPosition->MarginRateByMoney;
	char MarginRateByMoney[100];
	sprintf(MarginRateByMoney, "%f", marginRateByMoney);
	///保证金率(按手数)
	TThostFtdcRatioType	marginRateByVolume = pInvestorPosition->MarginRateByVolume;
	char MarginRateByVolume[100];
	sprintf(MarginRateByVolume, "%f", marginRateByVolume);
	string sInvestorInfo;
	//文件写入字段定义
	if (!isPositionDefFieldReady)
	{
		isPositionDefFieldReady = true;
		sInvestorInfo.append("InstrumentID\t");
		sInvestorInfo.append("BrokerID\t");
		sInvestorInfo.append("InvestorID\t");
		sInvestorInfo.append("PosiDirection\t");
		sInvestorInfo.append("HedgeFlag\t");
		sInvestorInfo.append("PositionDate\t");
		sInvestorInfo.append("YdPosition\t");
		sInvestorInfo.append("Position\t");
		sInvestorInfo.append("OpenVolume\t");
		sInvestorInfo.append("CloseVolume\t");
		sInvestorInfo.append("PositionCost\t");
		sInvestorInfo.append("PreSettlementPrice\t");
		sInvestorInfo.append("TradingDay\t");
		sInvestorInfo.append("OpenCost\t");
		sInvestorInfo.append("ExchangeMargin\t");
		sInvestorInfo.append("TodayPosition\t");

		sInvestorInfo.append("MarginRateByMoney\t");
		sInvestorInfo.append("MarginRateByVolume\t");
		LOG(INFO) << (sInvestorInfo);
	}
	sInvestorInfo.clear();
	sInvestorInfo.append(InstrumentID); sInvestorInfo.append("\t");
	sInvestorInfo.append(BrokerID); sInvestorInfo.append("\t");
	sInvestorInfo.append(InvestorID); sInvestorInfo.append("\t");
	sInvestorInfo.append(PosiDirection); sInvestorInfo.append("\t");
	sInvestorInfo.append(HedgeFlag); sInvestorInfo.append("\t");
	sInvestorInfo.append(PositionDate); sInvestorInfo.append("\t");
	sInvestorInfo.append(YdPosition); sInvestorInfo.append("\t");
	sInvestorInfo.append(Position); sInvestorInfo.append("\t");
	sInvestorInfo.append(OpenVolume); sInvestorInfo.append("\t");
	sInvestorInfo.append(CloseVolume); sInvestorInfo.append("\t");
	sInvestorInfo.append(PositionCost); sInvestorInfo.append("\t");
	sInvestorInfo.append(PreSettlementPrice); sInvestorInfo.append("\t");

	sInvestorInfo.append(TradingDay); sInvestorInfo.append("\t");
	sInvestorInfo.append(OpenCost); sInvestorInfo.append("\t");
	sInvestorInfo.append(ExchangeMargin); sInvestorInfo.append("\t");
	sInvestorInfo.append(TodayPosition); sInvestorInfo.append("\t");
	sInvestorInfo.append(MarginRateByMoney); sInvestorInfo.append("\t");
	sInvestorInfo.append(MarginRateByVolume); sInvestorInfo.append("\t");
	LOG(INFO) << (sInvestorInfo);
}
////将投资者对冲报单信息写入文件保存
void saveInvestorOrderInsertHedge(CThostFtdcInputOrderField *order, string filepath)
{
	string ordreInfo = getInvestorOrderInsertInfo(order);
	//cerr << "--->>> 写入对冲信息开始" << endl;
	LOG(INFO) << (ordreInfo);
	//cerr << "--->>> 写入对冲信息结束" << endl;
}

//提取投资者报单信息

//将投资者成交信息写入文件保存
void storeInvestorTrade(CThostFtdcTradeField *pTrade)
{
	string tradeInfo;
	///经纪公司代码
	char	*BrokerID = pTrade->BrokerID;
	///投资者代码
	char	*InvestorID = pTrade->InvestorID;
	///合约代码
	char	*InstrumentID = pTrade->InstrumentID;
	///报单引用
	char	*OrderRef = pTrade->OrderRef;
	///用户代码
	char	*UserID = pTrade->UserID;
	///交易所代码
	char	*ExchangeID = pTrade->ExchangeID;
	///成交编号
	//TThostFtdcTradeIDType	TradeID;
	///买卖方向
	TThostFtdcDirectionType	direction = pTrade->Direction;
	char Direction[] = { direction,'\0' };
	//sprintf(Direction,"%s",direction);
	///报单编号
	char	*OrderSysID = pTrade->OrderSysID;
	///会员代码
	//TThostFtdcParticipantIDType	ParticipantID;
	///客户代码
	char	*ClientID = pTrade->ClientID;
	///交易角色
	//TThostFtdcTradingRoleType	TradingRole;
	///合约在交易所的代码
	//TThostFtdcExchangeInstIDType	ExchangeInstID;
	///开平标志
	TThostFtdcOffsetFlagType	offsetFlag = pTrade->OffsetFlag;
	char OffsetFlag[] = { offsetFlag,'\0' };
	//sprintf(OffsetFlag,"%s",offsetFlag);
	///投机套保标志
	TThostFtdcHedgeFlagType	hedgeFlag = pTrade->HedgeFlag;
	char HedgeFlag[] = { hedgeFlag,'\0' };
	//sprintf(HedgeFlag,"%s",hedgeFlag);
	///价格
	TThostFtdcPriceType	price = pTrade->Price;
	char Price[100];
	sprintf(Price, "%f", price);
	///数量
	TThostFtdcVolumeType	volume = pTrade->Volume;
	char Volume[100];
	sprintf(Volume, "%d", volume);
	///成交时期
	//TThostFtdcDateType	TradeDate;
	///成交时间
	char	*TradeTime = pTrade->TradeTime;
	///成交类型
	TThostFtdcTradeTypeType	tradeType = pTrade->TradeType;
	char TradeType[] = { tradeType,'\0' };
	//sprintf(TradeType,"%s",tradeType);
	///成交价来源
	//TThostFtdcPriceSourceType	PriceSource;
	///交易所交易员代码
	//TThostFtdcTraderIDType	TraderID;
	///本地报单编号
	char	*OrderLocalID = pTrade->OrderLocalID;
	///结算会员编号
	//TThostFtdcParticipantIDType	ClearingPartID;
	///业务单元
	//TThostFtdcBusinessUnitType	BusinessUnit;
	///序号
	//TThostFtdcSequenceNoType	SequenceNo;
	///交易日
	char	*TradingDay = pTrade->TradingDay;
	///结算编号
	//TThostFtdcSettlementIDType	SettlementID;
	///经纪公司报单编号
	//TThostFtdcSequenceNoType	BrokerOrderSeq;
	if (!isTradeDefFieldReady)
	{
		isTradeDefFieldReady = true;
		tradeInfo.append("BrokerID\t");
		tradeInfo.append("InvestorID\t");
		tradeInfo.append("InstrumentID\t");
		tradeInfo.append("OrderRef\t");
		tradeInfo.append("UserID\t");
		tradeInfo.append("ExchangeID\t");
		tradeInfo.append("Direction\t");
		tradeInfo.append("ClientID\t");
		tradeInfo.append("OffsetFlag\t");
		tradeInfo.append("HedgeFlag\t");
		tradeInfo.append("Price\t");
		tradeInfo.append("Volume\t");
		tradeInfo.append("TradeTime\t");
		tradeInfo.append("TradeType\t");
		tradeInfo.append("OrderLocalID\t");
		tradeInfo.append("TradingDay\t");
		tradeInfo.append("ordersysid\t");
		LOG(INFO) << (tradeInfo);
	}
	tradeInfo.clear();
	tradeInfo.append(BrokerID); tradeInfo.append("\t");
	tradeInfo.append(InvestorID); tradeInfo.append("\t");
	tradeInfo.append(InstrumentID); tradeInfo.append("\t");
	tradeInfo.append(OrderRef); tradeInfo.append("\t");
	tradeInfo.append(UserID); tradeInfo.append("\t");
	tradeInfo.append(ExchangeID); tradeInfo.append("\t");
	tradeInfo.append(Direction); tradeInfo.append("\t");
	tradeInfo.append(ClientID); tradeInfo.append("\t");
	tradeInfo.append(OffsetFlag); tradeInfo.append("\t");
	tradeInfo.append(HedgeFlag); tradeInfo.append("\t");
	tradeInfo.append(Price); tradeInfo.append("\t");
	tradeInfo.append(Volume); tradeInfo.append("\t");
	tradeInfo.append(TradeTime); tradeInfo.append("\t");
	tradeInfo.append(TradeType); tradeInfo.append("\t");
	tradeInfo.append(OrderLocalID); tradeInfo.append("\t");
	tradeInfo.append(TradingDay); tradeInfo.append("\t");
	tradeInfo.append(OrderSysID); tradeInfo.append("\t");
	LOG(INFO) << (tradeInfo);
}
//将成交信息组装成对冲报单
// CThostFtdcInputOrderField assamble(CThostFtdcTradeField *pTrade)
// {
// 	CThostFtdcInputOrderField order;
// 	memset(&order,0,sizeof(order));
// 	//经济公司代码
// 	strcpy(order.BrokerID,pTrade->BrokerID);
// 	///投资者代码
// 	strcpy(order.InvestorID,pTrade->InvestorID);
// 	///合约代码
// 	strcpy(order.InstrumentID,pTrade->InstrumentID);
// 	///报单引用
// 	strcpy(order.OrderRef ,pTrade->OrderRef);
// 	///报单价格条件: 限价
// 	order.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
// 	///买卖方向: 这个要和对手方的一致，即如果我的成交为买，那么这里变成卖
// 	TThostFtdcDirectionType	Direction = pTrade->Direction;
// 	if (Direction == '0'){
// 		order.Direction = THOST_FTDC_D_Sell;
// 	} else {
// 		order.Direction = THOST_FTDC_D_Buy;
// 	}
// 	///组合开平标志: 和对手方一致
// 	order.CombOffsetFlag[0] = pTrade->OffsetFlag;
// 	///组合投机套保标志
// 	order.CombHedgeFlag[0] = pTrade->HedgeFlag;
// 	///价格
// 	TThostFtdcPriceType price = pTrade->Price;
// 	if (order.Direction == THOST_FTDC_D_Sell){
// 		//在原对手方报价基础上加上自定义tick
// 		order.LimitPrice = price + tickSpreadSell * tick;
// 	} else {
// 		//在原对手方报价基础上减去自定义tick
// 		order.LimitPrice = price - tickSpreadSell * tick;
// 	}
// 	///数量: 1
// 	order.VolumeTotalOriginal = pTrade->Volume;
// 	///有效期类型: 当日有效
// 	order.TimeCondition = THOST_FTDC_TC_GFD;
// 	///成交量类型: 任何数量
// 	order.VolumeCondition = THOST_FTDC_VC_AV;
// 	///最小成交量: 1
// 	order.MinVolume = 1;
// 	///触发条件: 立即
// 	order.ContingentCondition = THOST_FTDC_CC_Immediately;
// 	///强平原因: 非强平
// 	order.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
// 	///自动挂起标志: 否
// 	order.IsAutoSuspend = 0;
// 	///用户强评标志: 否
// 	order.UserForceClose = 0;
// 	return order;
// }
///撤单报单组装
CThostFtdcOrderField CTraderSpi::AssambleOrderAction(list<string> orderAction) {
	LOG(INFO) << ("开始组装撤单、修改报单请求信息......");
	///经纪公司代码
	TThostFtdcBrokerIDType	Broker_ID;
	///投资者代码
	TThostFtdcInvestorIDType Investor_ID;
	///合约代码
	char InstrumentID[31];
	///请求编号
	int RequestID = 0;
	//报单引用编号
	char OrderRef[13];
	///前置编号
	TThostFtdcFrontIDType FrontID = 1;
	///会话编号
	TThostFtdcSessionIDType SessionID = 0;
	///操作标志
	//char ActionFlag[3];
	///交易所代码
	TThostFtdcExchangeIDType	ExchangeID = "";
	///报单编号
	TThostFtdcOrderSysIDType OrderSysID = "";

	//报单结构体
	CThostFtdcOrderField req;
	//CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	//cout << "~~~~~~~~~~~~~~============>报单录入"<<endl;

	const char * split = "="; //分割符号
	int fieldSize = orderAction.size();
	/************************************************************************/
	/* 每个字段，按照=分隔符进行分割                                        */
	/************************************************************************/
	try {
		int i = 0;
		for (list<string>::iterator beg = orderAction.begin(); beg != orderAction.end(); beg++) {
			i++;
			string tmpstr = *beg;
			cout << tmpstr << endl;
			//分割之后的字符
			char * p = 0;
			//string转char*
			char * rawfields = new char[tmpstr.size() + 1];
			strcpy(rawfields, tmpstr.c_str());
			p = strtok(rawfields, split); //分割字符串
			vector<string> strlist;
			while (p != NULL)
			{
				//cout << p <<endl;
				strlist.push_back(p);
				p = strtok(NULL, split); //指向下一个指针
			}
			if (strlist.size() != 2) {
				//有字段为空，不填
				string tmpstr2 = "字段值为空:";
				LOG(INFO) << (tmpstr2 += tmpstr);
				string tmpstr3 = "there is field value is null!!!:";
				tmpstr3.append(tmpstr);
				tradequeue.push_back(tmpstr3);
				continue;
			}
			/************************************************************************/
			/* 变量赋值                                                                     */
			/*Broker_ID			1
			/*Investor_ID		2
			/*InstrumentID		3
			/*RequestID			4
			/*OrderRef			5
			/*FrontID			6
			/*SessionID			7
			/*ExchangeID		8
			/*OrderSysID		9
			/************************************************************************/
			string ttt = strlist.at(1);
			//cout << "赋值为:" + ttt<<endl;
			if (i == 1) {
				strcpy(Broker_ID, ttt.c_str());
			} else if (i == 2) {
				strcpy(Investor_ID, ttt.c_str());
			} else if (i == 3) {
				strcpy(InstrumentID, ttt.c_str());
			} else if (i == 4) {
				RequestID = atoi(ttt.c_str());
			} else if (i == 5) {
				strcpy(OrderRef, ttt.c_str());
			} else if (i == 6) {
				FrontID = atoi(ttt.c_str());
			} else if (i == 7) {
				SessionID = atol(ttt.c_str());
			} else if (i == 8) {
				strcpy(ExchangeID, ttt.c_str());
			} else if (i == 9) {
				strcpy(OrderSysID, ttt.c_str());
			}
		}
		///经纪公司代码

		strcpy(req.BrokerID, Broker_ID);
		///投资者代码
		strcpy(req.InvestorID, Investor_ID);
		///合约代码
		strcpy(req.InstrumentID, InstrumentID);
		///报单引用
		strcpy(req.OrderRef, OrderRef);
		req.RequestID = RequestID;
		///前置编号
		req.FrontID = FrontID;
		req.SessionID = SessionID;
		strcpy(req.ExchangeID, ExchangeID);
		strcpy(req.OrderSysID, OrderSysID);
	}
	catch (const runtime_error &re) {
		cerr << re.what() << endl;
	}
	catch (exception* e)
	{
		cerr << e->what() << endl;
		//	LOG(INFO)<<(e->what());
		LOG(ERROR) << e->what();  //输出一个Error日志
	}
	return req;
}
///撤单报单组装2
CThostFtdcOrderField CTraderSpi::AssambleOrderActionTwo(list<string> orderAction) {
	LogMsg *logmsg = new LogMsg();
	logmsg->setMsg("开始组装撤单、修改报单请求信息......");
	logqueue.push(logmsg);
	///经纪公司代码
	TThostFtdcBrokerIDType	Broker_ID = { "\0" };
	///投资者代码
	TThostFtdcInvestorIDType Investor_ID = { "\0" };
	///合约代码
	char InstrumentID[31] = { "\0" };
	///请求编号
	int RequestID = 0;
	//报单引用编号
	char OrderRef[13] = { "\0" };
	///前置编号
	TThostFtdcFrontIDType FrontID = 1;
	///会话编号
	TThostFtdcSessionIDType SessionID = 0;
	///操作标志
	//char ActionFlag[3];
	///交易所代码
	TThostFtdcExchangeIDType	ExchangeID = { "\0" };
	///报单编号
	TThostFtdcOrderSysIDType OrderSysID = { "\0" };

	//报单结构体
	CThostFtdcOrderField req;
	//CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	//cout << "~~~~~~~~~~~~~~============>报单录入"<<endl;

	//const char * split = "="; //分割符号
	int fieldSize = orderAction.size();
	/************************************************************************/
	/* 每个字段，按照=分隔符进行分割                                        */
	/************************************************************************/
	try {
		for (list<string>::iterator beg = orderAction.begin(); beg != orderAction.end(); beg++) {
			string tmpstr = *beg;
			vector<string> vec = split(tmpstr, "=");
			if ("FrontID" == vec[0]) {
				FrontID = boost::lexical_cast<int>(vec[1]);
				req.FrontID = FrontID;
			}
			if ("SessionID" == vec[0]) {
				SessionID = boost::lexical_cast<int>(vec[1]);
				req.SessionID = SessionID;
			}
			if ("OrderRef" == vec[0]) {
				strcpy(OrderRef, vec[1].c_str());
				strcpy(req.OrderRef, OrderRef);
			}
			if ("InstrumentID" == vec[0]) {
				strcpy(InstrumentID, vec[1].c_str());
				strcpy(req.InstrumentID, InstrumentID);
			}
			if ("OrderSysID" == vec[0]) {
				strcpy(OrderSysID, vec[1].c_str());
				strcpy(req.OrderSysID, OrderSysID);
			}
			if ("ExchangeID" == vec[0]) {
				strcpy(ExchangeID, vec[1].c_str());
				strcpy(req.ExchangeID, ExchangeID);
			}
		}
		///经纪公司代码

		//strcpy(req.BrokerID, Broker_ID);
		///投资者代码
		//strcpy(req.InvestorID, Investor_ID);
		///合约代码
		//strcpy(req.InstrumentID, InstrumentID);
		///报单引用
		//strcpy(req.OrderRef, OrderRef);
		//req.RequestID = RequestID;
		///前置编号
		//req.FrontID = FrontID;
		//req.SessionID = SessionID;
		//strcpy(req.ExchangeID , ExchangeID);
		//strcpy(req.OrderSysID , OrderSysID);
	}
	catch (const runtime_error &re) {
		cerr << re.what() << endl;
	}
	catch (exception* e)
	{
		cerr << e->what() << endl;
		LogMsg *logmsg = new LogMsg();
		logmsg->setMsg(e->what());
		logqueue.push(logmsg);
	}
	return req;
}
string getPositionDetail(CThostFtdcInvestorPositionDetailField  *pInvestorPosition) {
	string info;
	info.append("instrumentID=" + boost::lexical_cast<string>(pInvestorPosition->InstrumentID) + ";");
	info.append("direction=" + boost::lexical_cast<string>(pInvestorPosition->Direction) + ";");
	info.append("hedgeFlag=" + boost::lexical_cast<string>(pInvestorPosition->HedgeFlag) + ";");
	info.append("volume=" + boost::lexical_cast<string>(pInvestorPosition->Volume) + ";");
	info.append("openDate=" + boost::lexical_cast<string>(pInvestorPosition->OpenDate) + ";");
	info.append("tradeID=" + boost::lexical_cast<string>(pInvestorPosition->TradeID) + ";");
	return info;
}
