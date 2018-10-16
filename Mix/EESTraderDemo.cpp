#include "EESTraderDemo.h"
#include <iostream>
#include "property.h"
#include <dlfcn.h>
#include "TimeProcesser.h"
extern list<WaitForCloseInfo*> protectList;//protect order list
extern list<WaitForCloseInfo*> allTradeList;//before one normal
extern list<WaitForCloseInfo*> longReverseList;//before one normal
extern list<WaitForCloseInfo*> tmpLongReverseList;//before one normal
extern HoldPositionInfo userHoldPst;//not real hold position info
extern bool isInstrumentInit;
extern string currTime;
extern bool testSwitch;
extern Strategy techCls;
using std::cout;
using std::cin;
using std::endl;
extern string tradeServerIP;
extern int tradeServerPort;
extern string queryServerIP;
extern int queryServerPort;
extern string	BROKER_ID;				// 经纪公司代码
extern string INVESTOR_ID;			// 投资者代码
extern string  PASSWORD;			// 用户密码
extern string	ORDER_REF;	//报单引用
extern int USER_ID;
extern string LOGIN_ID;
extern string MAC_ADDRESS;
extern unordered_map<string, ControlOrderInfo*> controlTimeMap;//control order insert number
extern boost::posix_time::time_period *mkTimePeriod;
extern OrderFieldInfo* cancledOrderInfo;//cancled order info
extern OrderFieldInfo* acceptShLiOrderInfo;//shengli plantform order response info
extern OrderFieldInfo* acceptMarketOrderInfo;//exchange order response info
extern TradeFieldInfo* orderExecInfo;//when order is executed,trade response is sent
extern double realTradePrice;//default value is lastPrice
extern bool isLogout;
extern string systemID;//系统编号，每个产品一个编号
extern boost::thread_group thread_log_group;
extern boost::lockfree::queue<LogMsg*> logqueue;///日志消息队列
//extern CThostFtdcMdApi* mdUserApi;
extern CodeConverter *codeCC;
extern unordered_map<string, MarketData*> instrinfo;//market data
extern vector<double> mkTimeGap ;//tow marketdata time interval
extern list<OrderInfo*> aggOrderList;//aggressive market maker order list
extern list<OrderInfo*> aggTradeList;//aggressive market maker trade list
extern list<HoldPositionDetail*> holdPositionList;//position list
extern list<OrderInfo*> bidList;//order list
extern list<OrderInfo*> askList;//
extern unordered_map<string, InstrumentInfo*> instruments;			//合约信息
extern int amountCanExist;//how many orders can be put in this price
extern HoldPositionInfo* holdInfo;
extern int overVolume;
extern double floatMetric;//use for compare tow double
extern int orderPriceLevel;
extern int volMetric;
extern int pasStopLossTickNums;
extern int pasStopProfitTickNums;
extern int aggStopLossTickNums;
extern int aggStopProfitTickNums;
extern int timeBias;
// USER_API参数
extern TraderDemo* ptradeApi;
//extern CThostFtdcTraderApi* ptradeApi;
extern int notActiveInsertAmount;//不活跃合约重复下单次数
extern int orderInsertInterval;//不活跃合约重复下单次数
extern vector<string> instrumentsList;	//合约列表

extern unordered_map<string, PriceGap*> instr_price_gap;			//价格差
                               // 配置参数
extern char MARKET_FRONT_ADDR[];
extern char TRADE_FRONT_ADDR[];		// 前置地址
//extern char BROKER_ID[];		// 经纪公司代码
//extern char INVESTOR_ID[];		// 投资者代码
//extern char PASSWORD[];			// 用户密码
//char INSTRUMENT_ID[20];	// 合约代码
//extern TThostFtdcPriceType	LIMIT_PRICE;	// 价格
//extern TThostFtdcDirectionType	DIRECTION;	// 买卖方向
extern vector<string> quoteList;	//合约列表
                                    // 请求编号
extern int iRequestID;
//报单引用
extern int iOrderRef;
//extern unordered_map<string, TradeInfo*> willTradeMap;
// 会话参数
//TThostFtdcFrontIDType	FRONT_ID;	//前置编号
//TThostFtdcSessionIDType	SESSION_ID;	//会话编号
//TThostFtdcOrderRefType	ORDER_REF;	//报单引用
                                    //持仓是否已经写入定义字段
bool isPositionDefFieldReady = false;
//成交文件是否已经写入定义字段
bool isTradeDefFieldReady = false;
//用户对冲报单文件是否已经写入定义字段
bool isOrderInsertDefFieldReady = false;
//将成交信息组装成对冲报单
//CThostFtdcInputOrderField assamble(CThostFtdcTradeField *pTrade);
////将投资者对冲报单信息写入文件保存
//void saveInvestorOrderInsertHedge(CThostFtdcInputOrderField *order, string filepath);
//保存报单回报信息
//void saveRspOrderInsertInfo(CThostFtdcInputOrderField *pInputOrder);
//提取投资者报单信息
//string getInvestorOrderInsertInfo(CThostFtdcInputOrderField *order);
//以分隔符方式记录投资者报单委托信息
string getInvestorOrderInsertInfoByDelimater(EES_EnterOrderField *order);
string getOrderAcceptInfo(EES_OrderAcceptField* pAccept);
string getNormalOrderAcceptInfo(OrderFieldInfo* pAccept);
//提取投资者报单信息
//string getOrderActionInfoByDelimater(CThostFtdcInputOrderActionField *order);
//提取委托回报信息
//string getRtnOrderInfoByDelimater(CThostFtdcOrderField *order);
//成交明细
//string getPositionDetail(CThostFtdcInvestorPositionDetailField  *pInvestorPosition);
//提取成交回报信息
//string getRtnTradeInfoByDelimater(CThostFtdcTradeField *order);
//将交易所报单回报响应写入文件保存
//void saveRtnOrder(CThostFtdcOrderField *pOrder);
//获取交易所响应信息
//string getRtnOrder(CThostFtdcOrderField *pOrder);

//int storeInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition);
//void initpst(CThostFtdcInvestorPositionField *pInvestorPosition);
//初始化持仓信息
void initpst(EES_AccountPosition *pInvestorPosition);
void setInitAvaHoldPosition();
void initNormalMMPosition();
bool isNormalTrade(string orderType);
//CThostFtdcRspUserLoginField * loginRef;
extern string tradingDayT;//2010-01-01
//高频参数
extern unordered_map<string, HoldPositionInfo*> normalMMPositionmap;
extern unordered_map<string, HoldPositionInfo*> positionmap;
extern unordered_map<string, list<HoldPositionDetail*>*> positionDetailMap;//持仓明细
extern unordered_map<string, TechMetric*> techMetricMap;
//extern list<HoldPositionDetail*> positionDetailMap;//持仓明细
extern unordered_map<string, vector<string>> instr_map;				//一个合约和哪些合约配对
extern unordered_map<string, unordered_map<string, int64_t>> seq_map_orderref;
extern boost::lockfree::queue<LogMsg*> networkTradeQueue;///报单、成交消息队列,网络通讯使用
extern unordered_map<string, OriginalOrderFieldInfo*> originalOrderMap;//userid send order
extern unordered_map<string, string> exgToOriOrderMap;//order response from exchange;need MarketOrderToken to corresponsding to original order
extern unordered_map<string, string> seq_map_ordersysid;
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
extern boost::recursive_mutex unique_mtx;//unique lock
//extern boost::recursive_mutex arbVolume_mtx;//套利组合单数量的gap
extern int isTwoStartStrategy;//等于2的时候，表示明细和汇总持仓查询完毕，启动系统
extern int isbegin ;//是否启动策略
extern int realLongPstLimit;
extern int realShortPstLimit;
extern int longpstlimit;
//shortpstlimit
extern int shortpstlimit;
//记录时间
extern int long_offset_flag;
extern int short_offset_flag;
//买平标志,1开仓；2平仓
//有空头持仓，可以执行"买平仓"
extern int longPstIsClose;
//有多头持仓，可以执行"卖平仓"
extern int shortPstIsClose;
extern int start_process;
extern int arbVolume;//当前持仓量
extern int arbVolumeMetric;//套利单总共能下多少手，单边
///多个字段组合时使用的分隔符
extern string sep;
extern char tradingDay[12];
void aggStrategy(TradeFieldInfo* realseInfo);
void afterTradeProcessor(TradeFieldInfo* tradeInfo);
void storeTradedOrder(OrderFieldInfo* realseInfo,OriginalOrderFieldInfo* oriOrderField);
TraderDemo::TraderDemo(void){
    m_logonStatus = 0;
}


TraderDemo::~TraderDemo(void)
{
}

void TraderDemo::Run(){
    //InputParam();

    bool ret = Init();
    if (!ret){
        Pause();
        return;
    }
    //DemoSleep(1000);
    //reqOrderInsert(NULL);
    /*
    DemoSleep(1000);
    CxlOrder();

    DemoSleep(1000);
    Close();

    Pause();
*/
}
void TraderDemo::reqInstruments(){
    LOG(INFO) << "-->>>reqInstruments";
    int iResult = m_tradeApi->QuerySymbolList();
    //cerr << "--->>> 请求查询合约: " << ((iResult == 0) ? "成功" : "失败") << endl;
}
void TraderDemo::OnQuerySymbol(EES_SymbolField *pSymbol, bool bFinish){
    if (pSymbol && !bFinish) {
        //cerr << "--->>> " << "OnRspQryInstrument" << endl;
        processRspReqInstrument(pSymbol);
    }else if (bFinish){
        //boost::this_thread::sleep(boost::posix_time::seconds(1));
        isInstrumentInit = true;
        ///请求查询合约
        //start_process = 1;
        //startStrategy();
        //m_tradeApi->DisConnServer();
        //DestroyEESTraderApi(m_tradeApi);
        reqQryInvestorAccount();

    }
}
void TraderDemo::reqQryInvestorPosition(){
    //查询持仓之前，把持仓数据清除
    boost::recursive_mutex::scoped_lock SLock(unique_mtx);
    positionmap.clear();
    //RESULT EESTraderApi::QueryAccountPosition(const char* accountId, int nReqId)
    int iResult = m_tradeApi->QueryAccountPosition(INVESTOR_ID.c_str(),++iOrderRef);
    cerr << "--->>> 请求查询投资者持仓: " << ((iResult == 0) ? "成功" : "失败") << endl;
    //查询结果
    string strRst = ((iResult == 0) ? "成功" : "失败");
    strRst.append("--->>> 请求查询投资者持仓: ");
    LOG(INFO) << (strRst);
}
void TraderDemo::reqQryInvestorAccount(){
    //RESULT EESTraderApi::QueryAccountPosition(const char* accountId, int nReqId)
    int iResult = m_tradeApi->QueryAccountBP(INVESTOR_ID.c_str(),++iOrderRef);
    cerr << "--->>> 请求查询投资者money: " << ((iResult == 0) ? "成功" : "失败") << endl;
    //查询结果
    string strRst = ((iResult == 0) ? "成功" : "失败");
    strRst.append("--->>> 请求查询投资者money: ");
    LOG(INFO) << (strRst);
}
void TraderDemo::OnQueryAccountBP(const char* pAccount, EES_AccountBP* pAccout, int nReqId ){
    cerr << "--->>> " << "OnQueryAccountBP" << endl;
    if(INVESTOR_ID != boost::lexical_cast<string>(pAccount)){
        LOG(ERROR) << "INVESTORID=" + boost::lexical_cast<string>(pAccount) + ",THIS NOT MY MONEY!!!";
        return;
    }
    string msg = "";
    msg += "m_InitialBp=" + boost::lexical_cast<string>(pAccout->m_InitialBp) + ";";
    msg += "m_AvailableBp=" + boost::lexical_cast<string>(pAccout->m_AvailableBp) + ";";
    msg += "m_Margin=" + boost::lexical_cast<string>(pAccout->m_Margin) + ";";
    msg += "m_TotalLiquidPL=" + boost::lexical_cast<string>(pAccout->m_TotalLiquidPL) + ";";
    msg += "m_TotalMarketPL=" + boost::lexical_cast<string>(pAccout->m_TotalMarketPL) + ";";
    cout << msg << endl;
    LOG(INFO) << "INVESTORID=" + boost::lexical_cast<string>(pAccount) + ",MONEY INFO:" + msg;
    reqQryInvestorPosition();
}

void TraderDemo::OnQueryAccountPosition(const char* pAccount, EES_AccountPosition* pAccoutnPosition, int nReqId, bool bFinish){

    cerr << "--->>> " << "OnRspQryInvestorPosition" << endl;
    if(INVESTOR_ID != boost::lexical_cast<string>(pAccount)){
        LOG(ERROR) << "INVESTORID=" + boost::lexical_cast<string>(pAccount) + ",THIS NOT MY POSITION!!!";
        return;
    }
    if (pAccoutnPosition && !bFinish) {
        for(int i = 0,j = instrumentsList.size();i < j;i++){
            if(instrumentsList[i] == boost::lexical_cast<string>(pAccoutnPosition->m_Symbol)){
                break;
            }else{
                return;
            }
        }
        initpst(pAccoutnPosition);
    }else if (bFinish){
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

        tradeParaProcessTwo();
        //setInitAvaHoldPosition();
        //initNormalMMPosition();
        cout << ">>>>>>>>>>>>if ok,please press Enter!<<<<<<<<<<<<"<< endl;
        cout << ">>>>>>>>>>>>Ready,please init!<<<<<<<<<<<<"<< endl;
        //reqQryInvestorAccount();
        //getchar();
        //startStrategy();
    }
}
void initNormalMMPosition(){
    unordered_map<string, HoldPositionInfo*>::iterator tmpit = positionmap.begin();
    for (; tmpit != positionmap.end(); tmpit++) {
        HoldPositionInfo* normalHoldPst = new HoldPositionInfo();
        string str_instrument = tmpit->first;
        HoldPositionInfo* tmppst = tmpit->second;
        normalHoldPst->longAmount = tmppst->longAmount;
        normalHoldPst->longAvaClosePosition = tmppst->longAvaClosePosition;
        normalHoldPst->longHoldAvgPrice = tmppst->longHoldAvgPrice;
        normalHoldPst->longTdPosition = tmppst->longTdPosition;
        normalHoldPst->longTotalPosition = tmppst->longTotalPosition;
        normalHoldPst->longYdPosition = tmppst->longYdPosition;

        normalHoldPst->shortAmount = tmppst->shortAmount;
        normalHoldPst->shortAvaClosePosition = tmppst->shortAvaClosePosition;
        normalHoldPst->shortHoldAvgPrice = tmppst->shortHoldAvgPrice;
        normalHoldPst->shortTdPosition = tmppst->shortTdPosition;
        normalHoldPst->shortTotalPosition = tmppst->shortTotalPosition;
        normalHoldPst->shortYdPosition = tmppst->shortYdPosition;
        normalMMPositionmap[str_instrument] = normalHoldPst;
    }
}
//初始化持仓信息
void initpst(EES_AccountPosition *pInvestorPosition)
{
    boost::recursive_mutex::scoped_lock SLock(unique_mtx);
    ///合约代码
    char	*InstrumentID = pInvestorPosition->m_Symbol;
    string str_instrumentid = string(InstrumentID);
    ///持仓多空方向 多空方向 1：多头 5：空头
    int	dir = pInvestorPosition->m_PosiDirection;
    //ru1801
    //char PosiDirection[] = { dir,'\0' };
    ///投机套保标志
    //char	flag = pInvestorPosition->m_HedgeFlag;
    //char HedgeFlag[] = { flag,'\0' };
    ///上日持仓
    int	ydPosition = pInvestorPosition->m_OvnQty;
    //char YdPosition[100];
    //sprintf(YdPosition, "%d", ydPosition);
    ///今日持仓
    int	todayPosition = pInvestorPosition->m_TodayQty;
    //char Position[100];
    //sprintf(Position, "%d", position);
    string str_dir = boost::lexical_cast<string>(dir);
    double multiplier = getMultipler(str_instrumentid);

    if (positionmap.find(str_instrumentid) == positionmap.end()) {//暂时没有处理，不需要考虑多空方向
        unordered_map<string, int> tmpmap;
        HoldPositionInfo* tmpinfo = new HoldPositionInfo();
        if ("1" == str_dir) {//买  //多头
            tmpinfo->longTotalPosition = todayPosition + ydPosition;
            tmpinfo->longAvaClosePosition = todayPosition + ydPosition;
            tmpinfo->longAmount = pInvestorPosition->m_PositionCost;
            tmpinfo->longHoldAvgPrice = pInvestorPosition->m_PositionCost / (multiplier*tmpinfo->longTotalPosition);
            //tmpmap["longTotalPosition"] = position;
            //空头
            tmpinfo->shortTotalPosition = 0;
            //昨仓
            tmpinfo->longYdPosition = ydPosition;
            //今仓
            tmpinfo->longTdPosition = todayPosition;
        } else if ("5" == str_dir) {//空头
            tmpinfo->longTotalPosition = 0;
            tmpinfo->shortTotalPosition = todayPosition + ydPosition;
            tmpinfo->shortAvaClosePosition = todayPosition + ydPosition;
            tmpinfo->shortAmount = pInvestorPosition->m_PositionCost;
            tmpinfo->shortHoldAvgPrice = pInvestorPosition->m_PositionCost / (multiplier*tmpinfo->shortTotalPosition);

            //昨仓
            tmpinfo->shortYdPosition = ydPosition;
            //今仓
            tmpinfo->shortTdPosition = todayPosition;
        } else {
            //cout << InstrumentID << ";error:持仓类型无法判断PosiDirection=" << str_dir << endl;
            LOG(ERROR) << string(InstrumentID) + ";error:持仓类型无法判断PosiDirection=" + str_dir;
            return;
        }
        positionmap[str_instrumentid] = tmpinfo;
    }else {
        unordered_map<string, HoldPositionInfo*>::iterator tmpmap = positionmap.find(str_instrumentid);
        HoldPositionInfo* tmpinfo = tmpmap->second;
        //对应的反方向应该已经存在，这里后续需要确认
        if ("1" == str_dir) {//多头
            tmpinfo->longTotalPosition = todayPosition + ydPosition;
            tmpinfo->longAvaClosePosition = todayPosition + ydPosition;
            tmpinfo->longAmount = pInvestorPosition->m_PositionCost;
            tmpinfo->longHoldAvgPrice = pInvestorPosition->m_PositionCost / (multiplier*tmpinfo->longTotalPosition);
            //昨仓
            tmpinfo->longYdPosition = ydPosition;
            //今仓
            tmpinfo->longTdPosition = todayPosition;
        } else if ("5" == str_dir) {//空
                                    //空头
            tmpinfo->shortTotalPosition = todayPosition + ydPosition;
            tmpinfo->shortAvaClosePosition = todayPosition + ydPosition;
            tmpinfo->shortAmount = pInvestorPosition->m_PositionCost;
            tmpinfo->shortHoldAvgPrice = pInvestorPosition->m_PositionCost / (multiplier*tmpinfo->shortTotalPosition);

            //昨仓
            tmpinfo->shortYdPosition = ydPosition;
            //今仓
            tmpinfo->shortTdPosition = todayPosition;
        } else {
            //cout << InstrumentID << ";error:持仓类型无法判断PosiDirection=" << str_dir << endl;
            LOG(ERROR) << string(InstrumentID) + ";error:持仓类型无法判断PosiDirection=" + str_dir;
            return;
        }
    }
    //storeInvestorPosition(pInvestorPosition);
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
                    cout << insComKey + " arbComVolMetricAS=" + boost::lexical_cast<string>(pg->arbComVolMetricAS) + ",arbComVolMetricDS=" + boost::lexical_cast<string>(pg->arbComVolMetricDS) <<endl;
                    cout << insComKey + "正套持仓数量=" + boost::lexical_cast<string>(pg->arbComVolumeAS) + ",反套持仓数量=" + boost::lexical_cast<string>(pg->arbComVolumeDS) +",正套持仓均价=" + boost::lexical_cast<string>(TM->asHoldMeanGap) + ",反套持仓均价=" + boost::lexical_cast<string>(TM->dsHoldMeanGap) <<endl;
                }
            }
        }

    }
}
bool TraderDemo::Init()
{

    bool ret = LoadEESTrader();
    if (!ret)
    {
        return false;
    }

    RESULT ret_err = m_tradeApi->ConnServer(tradeServerIP.c_str(), tradeServerPort, this, queryServerIP.c_str(), queryServerPort);
    if (ret_err != NO_ERROR)
    {
        printf("connect to REM server failed!\n");
        return false;
    }else{
        cout<<"start to connect to server."<<endl;
        return true;
    }
    /*
    int waitTime = 0;//等待超时
    while (m_logonStatus != 2 && m_logonStatus != 3)
    {
        DemoSleep(100);
        waitTime++;
        if (waitTime >= 50)//5秒超时
        {
            printf("wait for logon response timeout!\n");
            return false;
        }
    }
    */
}

void TraderDemo::Close()
{
    if (m_tradeApi)
    {
        m_tradeApi->DisConnServer();
    }

    UnloadEESTrader();
}

void TraderDemo::DemoSleep(int nMilliSeconds)
{
#ifdef WIN32

    Sleep(nMilliSeconds);
#else
    boost::this_thread::sleep(boost::posix_time::microseconds(1000* nMilliSeconds));    //microsecond,millisecn
    //sleep(1000* nMilliSeconds);

#endif

}

void TraderDemo::Pause()
{
    string str_temp;

    printf("\npress any key to continue:\n");
    cin >> str_temp;
}

bool TraderDemo::LoadEESTrader()
{
#ifdef WIN32

    return Windows_LoadEESTrader();

#else

    return Linux_LoadEESTrader();

#endif
}

void TraderDemo::UnloadEESTrader()
{
#ifdef WIN32

    return Windows_UnloadEESTrader();

#else

    return Linux_UnloadEESTrader();

#endif
}

bool TraderDemo::Windows_LoadEESTrader()
{
#ifdef WIN32

    m_handle =  LoadLibrary(EES_TRADER_DLL_NAME);
    if (!m_handle)
    {
        printf("load library(%s) failed.\n", EES_TRADER_DLL_NAME);
        return false;
    }

    funcCreateEESTraderApi createFun = (funcCreateEESTraderApi)GetProcAddress(m_handle, CREATE_EES_TRADER_API_NAME);
    if (!createFun)
    {
        printf("get function addresss(%s) failed!\n", CREATE_EES_TRADER_API_NAME);
        return false;
    }

    m_distoryFun = (funcDestroyEESTraderApi)GetProcAddress(m_handle, DESTROY_EES_TRADER_API_NAME);
    if (!createFun)
    {
        printf("get function addresss(%s) failed!\n", DESTROY_EES_TRADER_API_NAME);
        return false;
    }

    m_tradeApi = createFun();
    if (!m_tradeApi)
    {
        printf("create trade API object failed!\n");
        return false;
    }

#endif

    return true;
}

void TraderDemo::Windows_UnloadEESTrader()
{
#ifdef WIN32

    if (m_tradeApi)
    {
        m_distoryFun(m_tradeApi);
        m_tradeApi = NULL;
        m_distoryFun = NULL;
    }

    if (m_handle)
    {
        FreeLibrary(m_handle);
        m_handle = NULL;
    }
#endif
}

bool TraderDemo::Linux_LoadEESTrader()
{
#ifndef WIN32
    m_handle =  dlopen(EES_TRADER_DLL_NAME, RTLD_LAZY);
    if (!m_handle)
    {
        printf("load library(%s) failed.\n", EES_TRADER_DLL_NAME);
        return false;
    }

    funcCreateEESTraderApi createFun = (funcCreateEESTraderApi)dlsym(m_handle, CREATE_EES_TRADER_API_NAME);
    if (!createFun)
    {
        printf("get function addresss(%s) failed!\n", CREATE_EES_TRADER_API_NAME);
        return false;
    }

    m_distoryFun = (funcDestroyEESTraderApi)dlsym(m_handle, DESTROY_EES_TRADER_API_NAME);
    if (!createFun)
    {
        printf("get function addresss(%s) failed!\n", DESTROY_EES_TRADER_API_NAME);
        return false;
    }

    m_tradeApi = createFun();
    if (!m_tradeApi)
    {
        printf("create trade API object failed!\n");
        return false;
    }
#endif

    return true;
}

void TraderDemo::Linux_UnloadEESTrader()
{
#ifndef WIN32
    if (m_tradeApi)
    {
        m_distoryFun(m_tradeApi);
        m_tradeApi = NULL;
        m_distoryFun = NULL;
    }

    if (m_handle)
    {
        dlclose(m_handle);
        m_handle = NULL;
    }
#endif
}
void TraderDemo::Logon(){
    if (!m_tradeApi){
        printf("INVALID api object\n");
        return;
    }
    cout<<"loginid="<<LOGIN_ID << endl;
    cout<<"passwd="<<PASSWORD << endl;
    cout<<"login REM ......" << endl;
    LOG(ERROR) << "request login shengli plantform......";
    //cout<<"investorid="<<INVESTOR_ID << endl;
    int rst = m_tradeApi->UserLogon(LOGIN_ID.c_str(), PASSWORD.c_str(), "TradeDemo", MAC_ADDRESS.c_str());
    if(rst == 0){
         LOG(ERROR) << "request login successfully!";
    }else{
        LOG(ERROR) << "request login failed!";
    }
}

void TraderDemo::reqOrderInsert(EES_EnterOrderField* orderField,AdditionOrderInfo* aoi){
    /************************************************************************/
    /* 每个字段，按照=分隔符进行分割                                        */
    /************************************************************************/
    try {
        ///replace
        RESULT iResult = m_tradeApi->EnterOrder(orderField);
        if(iResult == 0){//save original order info
            storeInitOrders(orderField,aoi);
        }else{
            LOG(ERROR) << ("ReqOrderInsert--->>>failed,failed code="+boost::lexical_cast<string>(iResult));
            string orid = boost::lexical_cast<string>(USER_ID) + boost::lexical_cast<string>(orderField->m_ClientOrderToken);
            boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
            LOG(INFO) << "--->>> reqOrderInsert,orid=" + orid;
            OriginalOrderFieldInfo* oriOrderField;
            unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.find(orid);
            if(it != originalOrderMap.end()){
                oriOrderField = it->second;
                string msg = "find orid=" + orid + ";volume=" + boost::lexical_cast<string>(oriOrderField->volumeTotalOriginal) + ",price=" + boost::lexical_cast<string>(oriOrderField->m_Price) +
                        ",orderType="+ boost::lexical_cast<string>(oriOrderField->orderType) + ",clidentToken="+ boost::lexical_cast<string>(oriOrderField->clientOrderToken);
                cout<<msg<<endl;
                originalOrderMap.erase(it);
            }else{
                LOG(ERROR) <<"ERROR: can't find OriginalOrderFieldInfo,orid=" + boost::lexical_cast<string>(orid);
            }
            LOG(ERROR) << "ERROR:reqOrderInsert.m_ClientOrderToken=" + boost::lexical_cast<string>(orderField->m_ClientOrderToken);

            //cout<<"3"<<endl;
            deleteOriOrder(orderField->m_ClientOrderToken);
            //cout<<"1"<<endl;

        }//[u"报单编号",u"合约名称",u"买卖",u"开平",u"挂单状态",u"信息",u"报单价格",u"报单手数",u"未成交手数",u"成交手数",u"详细状态",u"报单时间",u"成交价格",u"交易所"]
        //委托类操作，使用客户端定义的请求编号格式
        //cout << boost::lexical_cast<string>(iResult);
        //cerr << "--->>> ReqOrderInsert:" << ((iResult == NO_ERROR) ? "成功" : "失败") << endl;
        //记录报单录入信息
        string orderinsertstr = getInvestorOrderInsertInfoByDelimater(orderField);
        LOG(INFO) << ("ReqOrderInsert--->>>" + orderinsertstr);
        string tmpstr = "--->>> ReqOrderInsert: ";
        tmpstr.append(((iResult == 0) ? "success" : "failed"));
        LOG(INFO) << (tmpstr);
    }
    catch (const runtime_error &re) {
        cerr << re.what() << endl;
    }
    catch (exception* e)
    {
        cerr << e->what() << endl;
        LOG(ERROR) << (e->what());
    }

}
///报单操作请求
void TraderDemo::reqOrderAction(EES_CancelOrder* order) {
    RESULT ret = m_tradeApi->CancelOrder(order);
    if (ret != NO_ERROR){
        LOG(ERROR) << "send cancel failed";
        return;

    }else{
        if(isLogout){
            LOG(INFO) << "send cancel successfully";
        }
    }
}
void TraderDemo::CxlOrder()
{
    string str_temp;
    string str_no = "n";
    cout << "send cancel? (y/n) ";
    cin >> str_temp;
    if (str_no == str_temp)
    {
        return;
    }


    EES_CancelOrder  temp;
    memset(&temp, 0, sizeof(EES_CancelOrder));

    strcpy(temp.m_Account, m_account.c_str());
    temp.m_Quantity = m_quantity;
    temp.m_MarketOrderToken = m_marketOrderID;

    RESULT ret = m_tradeApi->CancelOrder(&temp);
    if (ret != NO_ERROR)
    {
        printf("send cancel failed(%d)\n", ret);
        return;
    }

    printf("send cancel successfully\n");
}

void TraderDemo::OnConnection(ERR_NO errNo, const char* pErrStr)
{
    if (errNo != NO_ERROR){
        LOG(INFO) << "ERROR!!connect to rem server failed,errorNum=" + boost::lexical_cast<string>(errNo) + ",errMsg=" + boost::lexical_cast<string>(pErrStr);
        //printf("connect to rem server failed(%d), %s!\n", errNo, pErrStr);
        Init();
        return;
    }else{
        LOG(INFO) << "GOOD!!connect to rem server successfully,errorNum=" + boost::lexical_cast<string>(errNo) + ",errMsg=" + boost::lexical_cast<string>(pErrStr);
    }
    m_logonStatus = 1;
    Logon();
}


void TraderDemo::OnDisConnection(ERR_NO errNo, const char* pErrStr)
{
    LOG(ERROR) << "ERROR!!disconnect from rem server,errorNum=" + boost::lexical_cast<string>(errNo) + ",errMsg=" + boost::lexical_cast<string>(pErrStr);
    //printf("disconnect from rem server(%d), %s!\n", errNo,pErrStr);
    m_logonStatus = 3;
    start_process=0;
    Init();
}


void TraderDemo::OnUserLogon(EES_LogonResponse* pLogon)
{
    //printf("logon result(%d)\n", pLogon->m_Result);
    if (pLogon->m_Result != NO_ERROR)
    {
        m_logonStatus = 3;
        //printf("logon failed, result=%d\n", pLogon->m_Result);
        LOG(ERROR) << "login failed, result code=" + boost::lexical_cast<string>(pLogon->m_Result);
        return;
    }else{
        iOrderRef = pLogon->m_MaxToken;
        ORDER_REF = boost::lexical_cast<string>(iOrderRef);
        USER_ID = pLogon->m_UserId;
        string tmptd = boost::lexical_cast<string>(pLogon->m_TradingDate);
        strcpy(tradingDay,tmptd.c_str());

        string year = boost::lexical_cast<string>(tradingDay[0]) + boost::lexical_cast<string>(tradingDay[1]) + boost::lexical_cast<string>(tradingDay[2]) + boost::lexical_cast<string>(tradingDay[3]);
        string month = boost::lexical_cast<string>(tradingDay[4]) + boost::lexical_cast<string>(tradingDay[5]);
        string day = boost::lexical_cast<string>(tradingDay[6]) + boost::lexical_cast<string>(tradingDay[7]);
        tradingDayT = year + "-" + month + "-" + day;
        LOG(ERROR) << "login successfully!! trading date " + boost::lexical_cast<string>(pLogon->m_TradingDate) + ",max orderRef = " + boost::lexical_cast<string>(pLogon->m_MaxToken)
                      + ",userid = " + boost::lexical_cast<string>(USER_ID) ;
        if(!isInstrumentInit){
            reqInstruments();
        }

    }

}
string getOrderAcceptInfo(EES_OrderAcceptField* pAccept){
    string orderInfo = "";
    orderInfo += "m_AcceptTime=" + boost::lexical_cast<string>(pAccept->m_AcceptTime) + ";";
    orderInfo += "m_Account=" + boost::lexical_cast<string>(pAccept->m_Account) + ";";
    orderInfo += "m_ClientOrderToken=" + boost::lexical_cast<string>(pAccept->m_ClientOrderToken) + ";";
    orderInfo += "m_Exchange=" + boost::lexical_cast<string>(pAccept->m_Exchange) + ";";
    orderInfo += "m_HedgeFlag=" + boost::lexical_cast<string>(pAccept->m_HedgeFlag) + ";";
    orderInfo += "m_MarketOrderToken=" + boost::lexical_cast<string>(pAccept->m_MarketOrderToken) + ";";
    orderInfo += "m_MarketSessionId=" + boost::lexical_cast<string>(pAccept->m_MarketSessionId) + ";";
    orderInfo += "m_MinQty=" + boost::lexical_cast<string>(pAccept->m_MinQty) + ";";
    orderInfo += "m_OrderState=" + boost::lexical_cast<string>(pAccept->m_OrderState) + ";";
    orderInfo += "m_Price=" + boost::lexical_cast<string>(pAccept->m_Price) + ";";
    orderInfo += "m_Qty=" + boost::lexical_cast<string>(pAccept->m_Qty) + ";";
    orderInfo += "m_SecType=" + boost::lexical_cast<string>(pAccept->m_SecType) + ";";
    orderInfo += "m_Side=" + boost::lexical_cast<string>(pAccept->m_Side) + ";";
    orderInfo += "m_Symbol=" + boost::lexical_cast<string>(pAccept->m_Symbol) + ";";
    orderInfo += "m_Tif=" + boost::lexical_cast<string>(pAccept->m_Tif) + ";";
    orderInfo += "m_UserID=" + boost::lexical_cast<string>(pAccept->m_UserID) + ";";
    return orderInfo;
}
string getNormalOrderAcceptInfo(OrderFieldInfo* pAccept){
    string orderInfo = "";
    orderInfo += "clientOrderToken=" + boost::lexical_cast<string>(pAccept->clientOrderToken) + ";";
    orderInfo += "direction=" + boost::lexical_cast<string>(pAccept->Direction) + ";";
    //orderInfo += "clientOrderToken=" + boost::lexical_cast<string>(pAccept->clientOrderToken) + ";";
    orderInfo += "instrumentID=" + boost::lexical_cast<string>(pAccept->InstrumentID) + ";";
    orderInfo += "offsetFlag=" + boost::lexical_cast<string>(pAccept->OffsetFlag) + ";";
    orderInfo += "marketOrderToken=" + boost::lexical_cast<string>(pAccept->marketOrderToken) + ";";
    orderInfo += "orderRef=" + boost::lexical_cast<string>(pAccept->clientOrderToken) + ";";
    orderInfo += "price=" + boost::lexical_cast<string>(pAccept->Price) + ";";
    orderInfo += "volume=" + boost::lexical_cast<string>(pAccept->tradeVolume) + ";";
    orderInfo += "volumeTotalOriginal=" + boost::lexical_cast<string>(pAccept->VolumeTotalOriginal) + ";";
    return orderInfo;
}
bool isMyOrder_OrderAccept(EES_OrderAcceptField* pAccept,OriginalOrderFieldInfo* &oriOrderField){
    //报单结构体
    string orid = boost::lexical_cast<string>(pAccept->m_UserID) + boost::lexical_cast<string>(pAccept->m_ClientOrderToken);
    LOG(INFO) << "orid=" + orid;
    unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.find(orid);
    if(it != originalOrderMap.end()){
        oriOrderField = it->second;
        //keep corresponde to exchange info
        oriOrderField->orderStatus = "a";
        oriOrderField->marketOrderToken = pAccept->m_MarketOrderToken;
        string motID = boost::lexical_cast<string>(pAccept->m_MarketOrderToken);
        exgToOriOrderMap[motID] = orid;
        LOG(INFO) << "find orid=" + orid + ";set orderstatus=a,marketOrderToken=" + boost::lexical_cast<string>(oriOrderField->marketOrderToken);
        //process hold_position,change some info.for cancel
        for(list<HoldPositionDetail*>::iterator hdIT = holdPositionList.begin();hdIT != holdPositionList.end();hdIT++){
            HoldPositionDetail* hpd = *hdIT;
            if(hpd->closeClientOrderToken == oriOrderField->clientOrderToken){
                hpd->closeMarketOrderToken = pAccept->m_MarketOrderToken;
                LOG(INFO) << "set hold postion's closeMarketOrderToken=" + motID;
                break;
            }
        }
        return true;
    }else{
        LOG(ERROR) << "ORDER's useid = " + boost::lexical_cast<string>(pAccept->m_UserID) + ",is not our useid=" + boost::lexical_cast<string>(USER_ID) + ",orid=" + orid + " is not in originalOrderMap.";
        return false;
    }
}
void TraderDemo::OnOrderAccept(EES_OrderAcceptField* pAccept){
    //报单结构体
    OriginalOrderFieldInfo* oriOrderField;
    LOG(INFO) << "--->>> OnOrderAccept: ";
    boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
    if(isMyOrder_OrderAccept(pAccept,oriOrderField)){
        LOG(INFO) << "is my order";
    }else{
        LOG(INFO) << "is not my order";
        return ;
    }

    LOG(INFO) << "SHENGLI:" + getOrderAcceptInfo(pAccept);
    transformFromShengLiPlantformOrder(pAccept,acceptShLiOrderInfo);//transform
    string tmp=getNormalOrderAcceptInfo(acceptShLiOrderInfo);

    LOG(INFO) << "change to normal:" + tmp;
    {//非撤单，修改报单状态
        if(acceptShLiOrderInfo->OffsetFlag != "0"){//only close position will be locked
            LOG(INFO) << "close order,need lock.";
            //未知单状态，锁定可平量
            processHowManyHoldsCanBeClose(acceptShLiOrderInfo, "lock");//锁定持仓
            /*
            if(isNormalTrade(oriOrderField->orderType)){
                processNormalHowManyHoldsCanBeClose(acceptShLiOrderInfo, "lock");//锁定持仓
            }*/
        }else{
            LOG(INFO) << "open order,don't need lock.";
        }
    }
    //m_marketOrderID = pAccept->m_MarketOrderToken;
}
bool isMyOrder_MarketAccept(EES_OrderMarketAcceptField* pAccept,OriginalOrderFieldInfo* oriOrderField){

    string omID = boost::lexical_cast<string>(pAccept->m_MarketOrderToken);
    unordered_map<string, string>::iterator exgIT = exgToOriOrderMap.find(omID);
    if(exgIT != exgToOriOrderMap.end()){//this is my order;
        string orid = exgIT->second;
        unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.find(orid);
        if(it != originalOrderMap.end()){//get order
            oriOrderField = it->second;
            //make up some info
            oriOrderField->orderStatus = "1";
            oriOrderField->orderSysID = pAccept->m_MarketOrderId;
            LOG(INFO) << "SHENGLI,set order status=1";
            LOG(INFO) << "SHENGLI MARKETTOKEN=" + boost::lexical_cast<string>(oriOrderField->marketOrderToken) + ",exhcange return markettoken=" + boost::lexical_cast<string>(pAccept->m_MarketOrderToken);
            return true;
        }else{
            LOG(ERROR) << "miss order!!marketToken = " + omID + " and investorID = " +  boost::lexical_cast<string>(pAccept->m_Account) + ",maybe not our useid=" + boost::lexical_cast<string>(USER_ID) + "'s order!!";
            return false;
        }
    }else{
        LOG(ERROR) << "miss order!!marketToken = " + omID + " and investorID = " + boost::lexical_cast<string>(pAccept->m_Account) + ",maybe not our useid=" + boost::lexical_cast<string>(USER_ID) + "'s order!!";
        return false;
    }
}
void TraderDemo::OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept)
{
    boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
    LOG(INFO) << "--->>> OnOrderMarketAccept: ";
    //报单结构体
    OriginalOrderFieldInfo *oriOrderField;
    string omID = boost::lexical_cast<string>(pAccept->m_MarketOrderToken);
    unordered_map<string, string>::iterator exgIT = exgToOriOrderMap.find(omID);
    if(exgIT != exgToOriOrderMap.end()){//this is my order;
        string orid = exgIT->second;
        unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.find(orid);
        if(it != originalOrderMap.end()){//get order
            oriOrderField = it->second;
            //make up some info
            oriOrderField->orderStatus = "1";
            oriOrderField->orderSysID = pAccept->m_MarketOrderId;
            LOG(INFO) << "SHENGLI,set order status=1";
            LOG(INFO) << "SHENGLI MARKETTOKEN=" + boost::lexical_cast<string>(oriOrderField->marketOrderToken) + ",exhcange return markettoken=" + boost::lexical_cast<string>(pAccept->m_MarketOrderToken);
            LOG(INFO) << "is my market order";
        }else{
            LOG(ERROR) << "miss order!!marketToken = " + omID + " and investorID = " +  boost::lexical_cast<string>(pAccept->m_Account) + ",is not our useid=" + boost::lexical_cast<string>(USER_ID) + "'s order!!";
            LOG(INFO) << "is not my market order";
            return ;
        }
    }else{
        LOG(ERROR) << "miss order!!marketToken = " + omID + " and investorID = " + boost::lexical_cast<string>(pAccept->m_Account) + ",is not our useid=" + boost::lexical_cast<string>(USER_ID) + "'s order!!";
        LOG(INFO) << "is not my market order";
        return ;
    }
}

void TraderDemo::OnOrderReject(EES_OrderRejectField* pReject)
{
    LOG(INFO) << "--->>> OnOrderReject: ";
    char GrammerText[1024]="";
    char RiskText[1024]="";
    //codeCC->convert(pReject->m_GrammerText,strlen(pReject->m_GrammerText),GrammerText,1024);
    codeCC->convert(pReject->m_GrammerText,strlen(pReject->m_RiskText),RiskText,1024);
    printf("---------------------------------------------------------\n");
    printf("OnOrderReject\n");
    printf("---------------------------------------------------------\n");
    printf("ClientOrderToken:     %d\n", pReject->m_ClientOrderToken);
    printf("ReasonCode      :     %d\n", int(pReject->m_ReasonCode));
    printf("m_Userid      :     %d\n", int(pReject->m_Userid));
    printf("GrammerText     :     %s\n", GrammerText);
    printf("RiskText        :     %s\n", RiskText);
    printf("\n");
    OriginalOrderFieldInfo *oriOrderField;
    //报单结构体
    //string orid = boost::lexical_cast<string>(pReject->m_Userid) + boost::lexical_cast<string>(pReject->m_ClientOrderToken);
    string orid = boost::lexical_cast<string>(USER_ID) + boost::lexical_cast<string>(pReject->m_ClientOrderToken);
    boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
    LOG(INFO) << "--->>> OnOrderReject,orid=" + orid;
    unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.find(orid);
    if(it != originalOrderMap.end()){
        oriOrderField = it->second;
        string msg = "find orid=" + orid + ";volume=" + boost::lexical_cast<string>(oriOrderField->volumeTotalOriginal) + ",price=" + boost::lexical_cast<string>(oriOrderField->m_Price) +
                ",orderType="+ boost::lexical_cast<string>(oriOrderField->orderType) + ",clidentToken="+ boost::lexical_cast<string>(oriOrderField->clientOrderToken);
        cout<<msg<<endl;
    }else{
        LOG(ERROR) <<"ERROR: can't find OriginalOrderFieldInfo,orid=" + boost::lexical_cast<string>(orid);
    }


    //transformFromExchangeTrade(oriOrderField,orderExecInfo);
    LOG(ERROR) << "ERROR:OnOrderReject.m_ClientOrderToken=" + boost::lexical_cast<string>(pReject->m_ClientOrderToken) + ",ReasonCode=" + boost::lexical_cast<string>(int(pReject->m_ReasonCode))  +
                  ",GrammerText=" + boost::lexical_cast<string>(GrammerText) + ",RiskText=" + boost::lexical_cast<string>(RiskText);

    //cout<<"3"<<endl;
    deleteOriOrder(pReject->m_ClientOrderToken);
    //cout<<"1"<<endl;
    originalOrderMap.erase(it);
    //cout<<"2"<<endl;
}

void TraderDemo::OnOrderMarketReject(EES_OrderMarketRejectField* pReject)
{
    boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
    LOG(INFO) << "--->>> OnOrderMarketReject: ";
    printf("---------------------------------------------------------\n");
    printf("OnOrderMarketReject\n");
    printf("---------------------------------------------------------\n");
    printf("Exchange OrderSysID  :     %lld\n", pReject->m_MarketOrderToken);
    printf("Reason Text          :     %.100s\n", pReject->m_ReasonText);
    printf("\n");
    string omid = boost::lexical_cast<string>(pReject->m_MarketOrderToken);
    LOG(ERROR) << "ERROR:OnOrderMarketReject.m_MarketOrderToken=" + omid +
                  ",m_ReasonText=" + boost::lexical_cast<string>(pReject->m_ReasonText) ;
    //deleteOriOrder(pReject->m_ClientOrderToken);
    OriginalOrderFieldInfo *oriOrderField;
    unordered_map<string, string>::iterator etomIT = exgToOriOrderMap.find(omid);
    if(etomIT!=exgToOriOrderMap.end()){
        string orid=etomIT->second;
        LOG(INFO) << "--->>> OnOrderMarketReject,orid=" + orid;
        unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.find(orid);
        if(it != originalOrderMap.end()){
            oriOrderField = it->second;
            string msg = "find orid=" + orid + ";volume=" + boost::lexical_cast<string>(oriOrderField->volumeTotalOriginal) + ",price=" + boost::lexical_cast<string>(oriOrderField->m_Price) +
                    ",orderType="+ boost::lexical_cast<string>(oriOrderField->orderType) + ",clidentToken="+ boost::lexical_cast<string>(oriOrderField->clientOrderToken);
            cout<<msg<<endl;
        }else{
            LOG(ERROR) <<"ERROR: can't find OriginalOrderFieldInfo,orid=" + boost::lexical_cast<string>(orid);
        }
        deleteOriOrder(oriOrderField->clientOrderToken);
        originalOrderMap.erase(it);

        transformFromExchangeTrade(oriOrderField,orderExecInfo);

        OrderFieldInfo* realseInfo = new OrderFieldInfo();
        realseInfo->InstrumentID = oriOrderField->instrumentID;
        realseInfo->Direction = orderExecInfo->Direction;
        realseInfo->OffsetFlag = orderExecInfo->OffsetFlag;
        realseInfo->marketOrderToken = orderExecInfo->marketToken;
        realseInfo->clientOrderToken = oriOrderField->clientOrderToken;
        realseInfo->orderType = oriOrderField->orderType;
        realseInfo->tradeVolume = oriOrderField->volumeTotalOriginal;
        realseInfo->openStgType = oriOrderField->openStgType;
        realseInfo->VolumeTotalOriginal = oriOrderField->volumeTotalOriginal;//original order volume
        //i known why there is a processing here!close position will minus some volume,so release position must be done before!
        if(realseInfo->OffsetFlag != "0"){
            processHowManyHoldsCanBeClose(realseInfo,"release");//释放持仓
            //There are two strategies in this code,so two different hold position map exist.Another will be processed bellow.
            /*
            if(isNormalTrade(oriOrderField->orderType)){//
                processNormalHowManyHoldsCanBeClose(realseInfo,"release");
            }*///not used
        }
    }
}
bool isMyTrade(EES_OrderExecutionField* pExec,OriginalOrderFieldInfo* oriOrderField){

    string omID = boost::lexical_cast<string>(pExec->m_MarketOrderToken);
    unordered_map<string, string>::iterator exgIT = exgToOriOrderMap.find(omID);
    if(exgIT != exgToOriOrderMap.end()){//this is my order;
        string orid = exgIT->second;
        unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.find(orid);
        if(it != originalOrderMap.end()){//get order
            oriOrderField = it->second;
            //make up some info
            oriOrderField->orderStatus = "2";
            oriOrderField->realPrice = pExec->m_Price;
            oriOrderField->realVolume = pExec->m_Quantity;
            LOG(INFO) << "trade,set order status=2";
            LOG(INFO) << "trade, realPrice=" + boost::lexical_cast<string>(oriOrderField->realPrice) + ",realVolume=" + boost::lexical_cast<string>(oriOrderField->realVolume);
            return true;
        }else{
            LOG(ERROR) << "miss this trade!!marketToken = " + omID + " and userID = " +  boost::lexical_cast<string>(pExec->m_Userid) + ",can't match original useid=" + boost::lexical_cast<string>(USER_ID) + "'s order!!";
            return false;
        }
    }else{
        LOG(ERROR) << "ommit trade!!marketToken = " + omID + " and userID = " + boost::lexical_cast<string>(pExec->m_Userid) + ",is not our useid=" + boost::lexical_cast<string>(USER_ID) + "'s order!!";
        return false;
    }
}
void TraderDemo::OnOrderExecution(EES_OrderExecutionField* pExec)
{
    boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
    LOG(INFO) << "--->>> OnOrderExecution: ";
    //报单结构体
    OriginalOrderFieldInfo* oriOrderField;
    string omID = boost::lexical_cast<string>(pExec->m_MarketOrderToken);
    unordered_map<string, string>::iterator exgIT = exgToOriOrderMap.find(omID);
    unordered_map<string, OriginalOrderFieldInfo*>::iterator it;
    if(exgIT != exgToOriOrderMap.end()){//this is my order;
        string orid = exgIT->second;
        it = originalOrderMap.find(orid);
        if(it != originalOrderMap.end()){//get order
            oriOrderField = it->second;
            //make up some info
            oriOrderField->orderStatus = "2";
            oriOrderField->realPrice = pExec->m_Price;
            oriOrderField->totalTradeVolume += pExec->m_Quantity;
            oriOrderField->realVolume = pExec->m_Quantity;
            LOG(INFO) << "trade,set order status=2";
            LOG(INFO) << "trade, realPrice=" + boost::lexical_cast<string>(oriOrderField->realPrice) + ",realVolume=" + boost::lexical_cast<string>(oriOrderField->realVolume) +
                          ",marketorderToken=" + omID;
            LOG(INFO) << "is my trade.";
        }else{
            LOG(ERROR) << "miss this trade!!marketToken = " + omID + " and userID = " +  boost::lexical_cast<string>(pExec->m_Userid) + ",can't match original useid=" + boost::lexical_cast<string>(USER_ID) + "'s order!!";
            LOG(INFO) << "is not my trade.";
            return;
        }
    }else{
        LOG(ERROR) << "ommit trade!!marketToken = " + omID + " and userID = " + boost::lexical_cast<string>(pExec->m_Userid) + ",is not our useid=" + boost::lexical_cast<string>(USER_ID) + "'s order!!";
        LOG(INFO) << "is not my trade.";
        return;
    }
    cerr << "--->>> " << "OnRtnTrade" << endl;

    transformFromExchangeTrade(oriOrderField,orderExecInfo);

    OrderFieldInfo* realseInfo = new OrderFieldInfo();
    realseInfo->Price = pExec->m_Price;
    realseInfo->InstrumentID = oriOrderField->instrumentID;
    realseInfo->Direction = orderExecInfo->Direction;
    realseInfo->OffsetFlag = orderExecInfo->OffsetFlag;
    realseInfo->marketOrderToken = orderExecInfo->marketToken;
    realseInfo->clientOrderToken = oriOrderField->clientOrderToken;
    realseInfo->orderType = oriOrderField->orderType;
    realseInfo->tradeVolume = pExec->m_Quantity;
    realseInfo->openStgType = oriOrderField->openStgType;
    realseInfo->VolumeTotalOriginal = oriOrderField->volumeTotalOriginal;//original order volume
    realseInfo->OrderSysID = oriOrderField->orderSysID;
    realseInfo->investorID = boost::lexical_cast<string>(pExec->m_Userid);
    string msg = "businessType=wtm_1001;result=0;updateTime="+currTime+getTradeInfo(realseInfo);
    sendMSG(msg);
    //i known why there is a processing here!close position will minus some volume,so release position must be done before!
    if(realseInfo->OffsetFlag != "0"){
        processHowManyHoldsCanBeClose(realseInfo,"release");//释放持仓
        /*if(oriOrderField->volumeTotalOriginal==oriOrderField->realVolume){
            processHowManyHoldsCanBeClose(realseInfo,"release");//释放持仓
        }else{
            LOG(INFO)<<"volumeTotalOriginal="+boost::lexical_cast<string>(oriOrderField->volumeTotalOriginal)
                        +" is not equal to realVolume="+boost::lexical_cast<string>(oriOrderField->realVolume)
                        +",not release.";
        }*/
        //There are two strategies in this code,so two different hold position map exist.Another will be processed bellow.
        /*
        if(isNormalTrade(oriOrderField->orderType)){//
            processNormalHowManyHoldsCanBeClose(realseInfo,"release");
        }*///not used
    }
    //处理持仓情况
    processtrade(orderExecInfo);/*
    if(isNormalTrade(oriOrderField->orderType)){
        processtradeOfNormal(orderExecInfo);
    }*/
    double priceTick = getPriceTick(oriOrderField->instrumentID);
    string openStgType = oriOrderField->openStgType;
    string orderType = oriOrderField->orderType;
    string mkType = oriOrderField->mkType;
    string function = oriOrderField->function;
    LOG(INFO) << "openStgType="+openStgType+",orderType="+orderType+",mkType=" +mkType + ",function=" +function;
    if(openStgType=="3000"){//first open order
        string tmpsts=techCls.stgStatus;
        testSwitch=true;
        techCls.stgStatus="3";
        LOG(INFO)<<"This is first order execution,set stgStatus from "+tmpsts+" to "+techCls.stgStatus;
        if(!oriOrderField->isFirstOpen){
            oriOrderField->isFirstOpen=true;
            LOG(INFO) << "This is first orders's first execution,set isFirstOpen to "+boost::lexical_cast<string>(oriOrderField->isFirstOpen)
                          +",set firstOpenPrice from "+boost::lexical_cast<string>(techCls.firstOpenPrice)
                          +" to " + boost::lexical_cast<string>(realseInfo->Price);
            techCls.firstOpenPrice=realseInfo->Price;
            LOG(INFO)<<"This is open position order.";
            WaitForCloseInfo* wfcInfo = new WaitForCloseInfo();
            wfcInfo->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo->openStgType = oriOrderField->openStgType;
            wfcInfo->tradeVolume = realseInfo->tradeVolume;
            wfcInfo->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo->direction = realseInfo->Direction;
            wfcInfo->openPrice = realseInfo->Price;
            allTradeList.push_back(wfcInfo);
        }else{
            LOG(INFO) << "This is first orders's other execution.";
            processOtherOpen(realseInfo,&allTradeList);
        }
    }else if(openStgType=="4000"){//close all position immediately
        LOG(INFO)<<"This is close all position directly trade.4000";
        processClose(realseInfo,&longReverseList);
        if(longReverseList.size()==0){
            tmpLongReverseList.clear();
            LOG(INFO)<<"All order has been closed,so close all order's task is over.";
        }
    }
    else if(openStgType=="2011"){//over first open price,stop loss.all position is closed,start a new circle.some parameters should be init here.;
        LOG(INFO)<<"This is init trade.";
        LOG(INFO)<<"This is stop profit trade.2011 for normal stop profit.";
        processClose(realseInfo,&allTradeList);
    }else if(openStgType=="2022"){//2 tick jump trigger stop loss
        string tmpsts=techCls.priceStatus;
        techCls.priceStatus="2";
        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus;
        if(!oriOrderField->isFirstOpen){
            double tmpprice=techCls.stopLossPrice;
            techCls.stopLossPrice=realseInfo->Price - techCls.nJumpTriggerSL*priceTick;
            oriOrderField->isFirstOpen=true;
            LOG(INFO) << "This is order of 2 tick jump trigger stop loss first execution,set stopLossPrice from "+boost::lexical_cast<string>(tmpprice)
                          +" to " + boost::lexical_cast<string>(techCls.stopLossPrice);
            LOG(INFO)<<"This is open position order.";
            WaitForCloseInfo* wfcInfo = new WaitForCloseInfo();
            wfcInfo->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo->openStgType = oriOrderField->openStgType;
            wfcInfo->tradeVolume = realseInfo->tradeVolume;
            wfcInfo->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo->direction = realseInfo->Direction;
            wfcInfo->openPrice = realseInfo->Price;
            allTradeList.push_back(wfcInfo);
        }else{
            LOG(INFO) << "This is 2 tick jump trigger stop loss orders's other execution,not process.";
            processOtherOpen(realseInfo,&allTradeList);
        }
    }else if(openStgType=="2032"){//one sweet range open position
        string tmpsts=techCls.priceStatus;
        techCls.priceStatus="3";
        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus;
        if(!oriOrderField->isFirstOpen){
            oriOrderField->isFirstOpen=true;
            LOG(INFO) << "This is one sweet first execution,not set stopLossPrice.";
            LOG(INFO)<<"This is open position order.";
            WaitForCloseInfo* wfcInfo = new WaitForCloseInfo();
            wfcInfo->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo->openStgType = oriOrderField->openStgType;
            wfcInfo->tradeVolume = realseInfo->tradeVolume;
            wfcInfo->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo->direction = realseInfo->Direction;
            wfcInfo->openPrice = realseInfo->Price;
            allTradeList.push_back(wfcInfo);
        }else{
            LOG(INFO) << "This is one sweet orders's other execution,not process.";
            processOtherOpen(realseInfo,&allTradeList);
        }
    }else if(openStgType=="2042"){//one normal range open position
        string tmpsts=techCls.priceStatus;
        techCls.priceStatus="4";
        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus;
        if(!oriOrderField->isFirstOpen){
            oriOrderField->isFirstOpen=true;
            LOG(INFO) << "This is one normal first execution,should set stopLossPrice.we will be kicked out by hold cost.";
            if(longReverseList.size() > 0){
                LOG(INFO)<<"All order traded before one normal has been moved to reverse list,don't need do again.";
            }else{
                int tmplist=allTradeList.size();

                tmpLongReverseList.clear();
                longReverseList.clear();
                //transfer all order in WaitForCloseInfo to longReverseList
                for(list<WaitForCloseInfo*>::iterator wfIt = allTradeList.begin();wfIt!=allTradeList.end();){
                    //tmpLongReverseList->push_back(*wfIt);
                    longReverseList.push_back(*wfIt);
                    wfIt = allTradeList.erase(wfIt);
                }
                LOG(INFO)<<"When first trade in one normal,all order traded before will be moved to reverselist.before allTradeList="+boost::lexical_cast<string>(tmplist)
                           +",after allTradeList="+boost::lexical_cast<string>(allTradeList.size())
                           +",longReserseList="+boost::lexical_cast<string>(longReverseList.size());
            }

            LOG(INFO)<<"This is open position order.";
            storeTradedOrder(realseInfo,oriOrderField);
            /*WaitForCloseInfo* wfcInfo1 = new WaitForCloseInfo();
            wfcInfo1->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo1->openStgType = oriOrderField->openStgType;
            wfcInfo1->tradeVolume = realseInfo->tradeVolume;
            wfcInfo1->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo1->direction = realseInfo->Direction;
            wfcInfo1->openPrice = realseInfo->Price;
            WaitForCloseInfo* wfcInfo2 = new WaitForCloseInfo();
            wfcInfo2->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo2->openStgType = oriOrderField->openStgType;
            wfcInfo2->tradeVolume = realseInfo->tradeVolume;
            wfcInfo2->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo2->direction = realseInfo->Direction;
            wfcInfo2->openPrice = realseInfo->Price;
            tmpLongReverseList.push_back(wfcInfo1);
            longReverseList.push_back(wfcInfo2);*/
        }else{
            LOG(INFO) << "This is one normal orders's other execution,not process.";
            processOtherOpen(realseInfo,&tmpLongReverseList);
            processOtherOpen(realseInfo,&longReverseList);
        }
    }else if(openStgType=="2052"){//two status
        string tmpsts=techCls.priceStatus;
        techCls.priceStatus="5";
        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus;
        if(!oriOrderField->isFirstOpen){
            oriOrderField->isFirstOpen=true;
            LOG(INFO) << "This is two status first execution,should set stopLossPrice.we will be kicked out by hold cost.";
            LOG(INFO)<<"This is open position order.";
            storeTradedOrder(realseInfo,oriOrderField);
            /*WaitForCloseInfo* wfcInfo1 = new WaitForCloseInfo();
            wfcInfo1->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo1->openStgType = oriOrderField->openStgType;
            wfcInfo1->tradeVolume = realseInfo->tradeVolume;
            wfcInfo1->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo1->direction = realseInfo->Direction;
            wfcInfo1->openPrice = realseInfo->Price;
            WaitForCloseInfo* wfcInfo2 = new WaitForCloseInfo();
            wfcInfo2->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo2->openStgType = oriOrderField->openStgType;
            wfcInfo2->tradeVolume = realseInfo->tradeVolume;
            wfcInfo2->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo2->direction = realseInfo->Direction;
            wfcInfo2->openPrice = realseInfo->Price;
            tmpLongReverseList.push_back(wfcInfo1);
            longReverseList.push_back(wfcInfo2);*/
        }else{
            LOG(INFO) << "This is two status orders's other execution,not process.";
            processOtherOpen(realseInfo,&tmpLongReverseList);
            processOtherOpen(realseInfo,&longReverseList);
        }
    }else if(openStgType=="2052a"){//addition fbna
        techCls.isAddOrderOpen = true;
        LOG(INFO)<<"this is addition fbna orders,not change priceStatus,remain priceStatus= "+techCls.priceStatus+",set isAddOrderOpen = true.";
        if(!oriOrderField->isFirstOpen){
            oriOrderField->isFirstOpen=true;
            LOG(INFO) << "This is addition fbna order's first execution,should set stopLossPrice.we will be kicked out by hold cost.";
            LOG(INFO)<<"This is open position order.";
            storeTradedOrder(realseInfo,oriOrderField);
            /*WaitForCloseInfo* wfcInfo1 = new WaitForCloseInfo();
            wfcInfo1->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo1->openStgType = oriOrderField->openStgType;
            wfcInfo1->tradeVolume = realseInfo->tradeVolume;
            wfcInfo1->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo1->direction = realseInfo->Direction;
            wfcInfo1->openPrice = realseInfo->Price;
            WaitForCloseInfo* wfcInfo2 = new WaitForCloseInfo();
            wfcInfo2->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo2->openStgType = oriOrderField->openStgType;
            wfcInfo2->tradeVolume = realseInfo->tradeVolume;
            wfcInfo2->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo2->direction = realseInfo->Direction;
            wfcInfo2->openPrice = realseInfo->Price;
            tmpLongReverseList.push_back(wfcInfo1);
            longReverseList.push_back(wfcInfo2);*/
        }else{
            LOG(INFO) << "This is addition fbna order's other execution,not process.";
            processOtherOpen(realseInfo,&tmpLongReverseList);
            processOtherOpen(realseInfo,&longReverseList);
        }
    }else if(openStgType=="p"){//
        LOG(INFO)<<"this is protection order.";
        if(!oriOrderField->isFirstOpen){
            oriOrderField->isFirstOpen=true;
            LOG(INFO) << "This is protection first execution.";
            LOG(INFO)<<"This is open position order.";
            WaitForCloseInfo* wfcInfo = new WaitForCloseInfo();
            wfcInfo->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo->openStgType = oriOrderField->openStgType;
            wfcInfo->tradeVolume = realseInfo->tradeVolume;
            wfcInfo->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo->direction = realseInfo->Direction;
            wfcInfo->openPrice = realseInfo->Price;
            wfcInfo->instrumentID = realseInfo->InstrumentID;
            protectList.push_back(wfcInfo);
        }else{
            LOG(INFO) << "This is protection orders's other execution,not process.";
            processOtherOpen(realseInfo,&protectList);
        }
    }
    else if(openStgType=="2062"){//lock
        double tickPrice=getPriceTick(oriOrderField->instrumentID);
        string tmpsts=techCls.priceStatus;
        techCls.priceStatus="6";
        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus;
        if(techCls.lockFirstOpenPrice ==0 ){
            double tmplfop = techCls.lockFirstOpenPrice;
            techCls.lockFirstOpenPrice = realseInfo->Price;
            LOG(INFO)<<"this is first trade price of locking.set lockFirstOpenPrice from "+boost::lexical_cast<string>(tmplfop)
                       +" to "+boost::lexical_cast<string>(techCls.lockFirstOpenPrice);

        }
        if(!oriOrderField->isFirstOpen){
            lockInit();
            oriOrderField->isFirstOpen=true;
            LOG(INFO) << "This is two status's lock execution.";
            LOG(INFO)<<"This is open position order.";
            storeTradedOrder(realseInfo,oriOrderField);

            /*WaitForCloseInfo* wfcInfo1 = new WaitForCloseInfo();
            wfcInfo1->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo1->openStgType = oriOrderField->openStgType;
            wfcInfo1->tradeVolume = realseInfo->tradeVolume;
            wfcInfo1->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo1->direction = realseInfo->Direction;
            wfcInfo1->openPrice = realseInfo->Price;
            WaitForCloseInfo* wfcInfo2 = new WaitForCloseInfo();
            wfcInfo2->marketOrderToken = realseInfo->marketOrderToken;
            wfcInfo2->openStgType = oriOrderField->openStgType;
            wfcInfo2->tradeVolume = realseInfo->tradeVolume;
            wfcInfo2->originalVolume = realseInfo->VolumeTotalOriginal;
            wfcInfo2->direction = realseInfo->Direction;
            wfcInfo2->openPrice = realseInfo->Price;
            tmpLongReverseList.push_back(wfcInfo1);
            longReverseList.push_back(wfcInfo2);*/
        }else{
            LOG(INFO) << "This is two status lock's other execution,not process.";
            processOtherOpen(realseInfo,&tmpLongReverseList);
            processOtherOpen(realseInfo,&longReverseList);
        }
        //relockprice will be set here also.because lock direction is reverse,so direction will change
        if(realseInfo->Direction == "0"){
            setRelockPrice(realseInfo->Price,tickPrice,"1");
        }else{
            setRelockPrice(realseInfo->Price,tickPrice,"0");
        }
    }else if(openStgType=="2071"){//short unlock
        LOG(INFO)<<"This is stop profit trade.2071,short position unlock.";
        string tmpsts=techCls.priceStatus;
        double preUnlp = techCls.unlockPrice;
        //reset lockFirstOpenPrice
        techCls.lockFirstOpenPrice=0;
        techCls.priceStatus="7";
        techCls.unlockPrice=realseInfo->Price;
        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus+",and set unlockPrice from "+boost::lexical_cast<string>(preUnlp)+" to "+boost::lexical_cast<string>(techCls.unlockPrice);
        processClose(realseInfo,&longReverseList);
        processClose(realseInfo,&tmpLongReverseList);
        ///will close protect orders
        closeProtectOrders();
    }else if(openStgType=="2081"){//long unlock
        LOG(INFO)<<"This is stop profit trade.2081,long position unlock.";
        string tmpsts=techCls.priceStatus;
        techCls.priceStatus="8";
        //techCls.unlockPrice=realseInfo->Price;
        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus;
        processClose(realseInfo,&longReverseList);
        processClose(realseInfo,&tmpLongReverseList);
    }else if(openStgType=="closeP"){//close protect
        LOG(INFO)<<"This is close protect trade.closeP.";
        processClose(realseInfo,&protectList);
    }
    else if(openStgType=="2001"){// long reverse stop profit
        ////long reverse stop profit.all position is closed,start a new circle.some parameters should be init here.;
        LOG(INFO)<<"This is stop profit trade.2001";
        processClose(realseInfo,&longReverseList);

    }else{
        LOG(ERROR)<<"ERROR:wrong openStgType="+openStgType;
    }
    //delete init info
    if(oriOrderField->volumeTotalOriginal==oriOrderField->totalTradeVolume){
        LOG(INFO)<<"clean order.";
        deleteOriOrder(pExec->m_ClientOrderToken);
        originalOrderMap.erase(it);
    }else{
        LOG(INFO)<<"volumeTotalOriginal="+boost::lexical_cast<string>(oriOrderField->volumeTotalOriginal)
                    +" is not equal to totalTradeVolume="+boost::lexical_cast<string>(oriOrderField->totalTradeVolume)
                    +",not delete init info.";
    }
    if(allTradeList.size()!=0){
        computeUserHoldPositionInfo(&allTradeList);
    }else if(longReverseList.size()!=0){
        computeUserHoldPositionInfo(&longReverseList);
    }else{
        computeUserHoldPositionInfo(NULL);
    }
    if(openStgType=="2001"||openStgType=="2081"){
        if(longReverseList.size()==0){
            //init
            coverYourAss();
        }
    }else if(openStgType=="2011"){
        if(allTradeList.size()==0){
            coverYourAss();
        }
    }
    /*string stg = "businessType=wtm_1001;instrumentID="+realseInfo->InstrumentID+";longAmount="+boost::lexical_cast<string>(userHoldPst.longAmount)+",shortAmount="+boost::lexical_cast<string>(userHoldPst.shortAmount) +",longTotal="+boost::lexical_cast<string>(userHoldPst.longTotalPosition)+",shortTotal="+boost::lexical_cast<string>(userHoldPst.shortTotalPosition)+",longHoldAvgPrice="
            +boost::lexical_cast<string>(userHoldPst.longHoldAvgPrice)+",shortHoldAvgPrice="+boost::lexical_cast<string>(userHoldPst.shortHoldAvgPrice);
    sendMSG(std);
    LogMsg *logmsg = new LogMsg();
    logmsg->setMsg(stg);
    networkTradeQueue.push(logmsg);*/
}
void storeTradedOrder(OrderFieldInfo* realseInfo,OriginalOrderFieldInfo* oriOrderField){
    WaitForCloseInfo* wfcInfo1 = new WaitForCloseInfo();
    wfcInfo1->marketOrderToken = realseInfo->marketOrderToken;
    wfcInfo1->openStgType = oriOrderField->openStgType;
    wfcInfo1->tradeVolume = realseInfo->tradeVolume;
    wfcInfo1->originalVolume = realseInfo->VolumeTotalOriginal;
    wfcInfo1->direction = realseInfo->Direction;
    wfcInfo1->openPrice = realseInfo->Price;
    WaitForCloseInfo* wfcInfo2 = new WaitForCloseInfo();
    wfcInfo2->marketOrderToken = realseInfo->marketOrderToken;
    wfcInfo2->openStgType = oriOrderField->openStgType;
    wfcInfo2->tradeVolume = realseInfo->tradeVolume;
    wfcInfo2->originalVolume = realseInfo->VolumeTotalOriginal;
    wfcInfo2->direction = realseInfo->Direction;
    wfcInfo2->openPrice = realseInfo->Price;
    tmpLongReverseList.push_back(wfcInfo1);
    longReverseList.push_back(wfcInfo2);
}

bool isNormalTrade(string orderType){
    LOG(INFO) << "isNormalTrade:orderType=" + orderType;
    if(orderType == "50" || orderType == "51" || orderType == "500"|| orderType == "501"|| orderType == "510"|| orderType == "511"){
        return false;//aggressive mode
    }else{
        return true;//normal mode
    }
}

void aggStrategy(TradeFieldInfo* realseInfo){
    LOG(INFO) << "aggStrategy,direction=" + boost::lexical_cast<string>(realseInfo->Direction) + ",offsetFlag=" + boost::lexical_cast<string>(realseInfo->OffsetFlag) +
                  ",orderType=" + realseInfo->orderType;
    string instrumentID = realseInfo->InstrumentID;
    MarketData* mkdata;
    double bidPrice1 = 0;
    double askPrice1 = 0;
    unordered_map<string, MarketData*>::iterator it = instrinfo.find(instrumentID);
    if(it != instrinfo.end()){
        mkdata = it->second;
        bidPrice1 = mkdata->bidPrice;
        askPrice1 = mkdata->askPrice;
    }else{
        return;
    }
    bool isOriginalDelete = true;
    double tick = getPriceTick(instrumentID);
    string direction = realseInfo->Direction;
    string oriOffsetFlag = realseInfo->OffsetFlag;
    double tradePrice = realseInfo->Price;
    if(bidPrice1 > tradePrice || askPrice1 < tradePrice){
        LOG(INFO) << "trade price=" + boost::lexical_cast<string>(tradePrice) + ",bid=" + boost::lexical_cast<string>(bidPrice1)+","+
                      "ask=" + boost::lexical_cast<string>(askPrice1);
    }
    if(direction == "0" &&  bidPrice1 > tradePrice&& isNormalTrade(realseInfo->orderType)){//buy cross spread,start aggressive mode

    //if(direction == "0" &&  bidPrice1 > tradePrice){
    //if(direction == "0"){
        int priceLevel = round((bidPrice1 - tradePrice)/tick) + 1;
        //int priceLevel = 2;
        LOG(INFO) << "long open, trade price=" + boost::lexical_cast<string>(tradePrice) + ",curr bidPrice=" + boost::lexical_cast<string>(bidPrice1) +
                      ",cross spread trade detected.priceLevel=" + boost::lexical_cast<string>(priceLevel) + ",start agg mode.";
        for(int j=0;j<4;j++){
            for(int i = 0;i < priceLevel; i++){
                double aggPrice = bidPrice1 - (i+1)*tick;
                unsigned int newOrderToken = iOrderRef++;
                unsigned char m_side = 0;
                int volume = 1;
                OrderInfo* orderInfo = new OrderInfo();
                orderInfo->userID = USER_ID;
                orderInfo->clientOrderToken = newOrderToken;
                string orderType = "500";//fok of buy open
                string offsetFlag = "0";
                m_side = changeSignalFromNormalToSL(direction,offsetFlag);
                orderInfo->m_Side = m_side;
                orderInfo->offsetFlag = offsetFlag;
                orderInfo->direction = direction;
                orderInfo->price = aggPrice;
                orderInfo->orderType = orderType;
                orderInfo->volume = volume;
                orderInfo->instrumentID = instrumentID;
                //aggOrderList.emplace_back(orderInfo);
                //LOG(INFO) << "down_culculate=" + boost::lexical_cast<string>(down_culculate) + ",up_culculate=" + boost::lexical_cast<string>(up_culculate) + ",buy open.";
                LOG(INFO) << "new add bid list to aggOrderList." + getOrderInfo(orderInfo);
                //shengli
                EES_EnterOrderField orderField;
                memset(&orderField, 0, sizeof(EES_EnterOrderField));
                orderField.m_Tif = EES_OrderTif_IOC;//立即单 EES_OrderTif_Day  EES_OrderTif_IOC
                orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
                strcpy(orderField.m_Account, INVESTOR_ID.c_str());
                strcpy(orderField.m_Symbol, instrumentID.c_str());//modify 2.
                orderField.m_Side = m_side;//modify 1.buy open
                orderField.m_Exchange = EES_ExchangeID_shfe;
                orderField.m_SecType = EES_SecType_fut;
                orderField.m_Price = aggPrice;//modify 3.
                orderField.m_Qty = volume;
                orderField.m_ClientOrderToken = newOrderToken;
                AdditionOrderInfo aoi;
                memset(&aoi, 0, sizeof(AdditionOrderInfo));
                aoi.mkType = "-1";
                aoi.orderType = orderType;
                ptradeApi->reqOrderInsert(&orderField,&aoi);
            }
        }

    }else if(direction == "1" && askPrice1 < tradePrice && isNormalTrade(realseInfo->orderType)){//short cross spread,start aggressive mode
    //  }else if(direction == "1"){//short cross spread,start aggressive mode
        int priceLevel = round((tradePrice-askPrice1)/tick) + 1;
        //int priceLevel = 2;
        LOG(INFO) << "short open, trade price=" + boost::lexical_cast<string>(tradePrice) + ",curr askPrice1=" + boost::lexical_cast<string>(askPrice1) +
                      ",cross spread trade detected.priceLevel=" + boost::lexical_cast<string>(priceLevel) + ",start agg mode.";
        for(int j=0;j<4;j++){
            for(int i = 0;i < priceLevel; i++){
                double aggPrice = askPrice1 + (i+1)*tick;
                unsigned int newOrderToken = iOrderRef++;
                unsigned char m_side = 0;
                int volume = 1;
                OrderInfo* orderInfo = new OrderInfo();
                orderInfo->userID = USER_ID;
                orderInfo->clientOrderToken = newOrderToken;
                string orderType = "510";//fok of sell open
                string offsetFlag = "0";
                m_side = changeSignalFromNormalToSL(direction,offsetFlag);
                orderInfo->m_Side = m_side;
                orderInfo->offsetFlag = offsetFlag;
                orderInfo->direction = direction;
                orderInfo->price = aggPrice;
                orderInfo->orderType = orderType;
                orderInfo->volume = volume;
                orderInfo->instrumentID = instrumentID;
                //aggOrderList.emplace_back(orderInfo);
                //LOG(INFO) << "down_culculate=" + boost::lexical_cast<string>(down_culculate) + ",up_culculate=" + boost::lexical_cast<string>(up_culculate) + ",buy open.";
                LOG(INFO) << "new add ask list to aggOrderList." + getOrderInfo(orderInfo);
                //shengli
                EES_EnterOrderField orderField;
                memset(&orderField, 0, sizeof(EES_EnterOrderField));
                orderField.m_Tif = EES_OrderTif_IOC;//立即单 EES_OrderTif_Day  EES_OrderTif_IOC
                orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
                strcpy(orderField.m_Account, INVESTOR_ID.c_str());
                strcpy(orderField.m_Symbol, instrumentID.c_str());//modify 2.
                orderField.m_Side = m_side;//modify 1.buy open
                orderField.m_Exchange = EES_ExchangeID_shfe;
                orderField.m_SecType = EES_SecType_fut;
                orderField.m_Price = aggPrice;//modify 3.
                orderField.m_Qty = volume;
                orderField.m_ClientOrderToken = newOrderToken;
                AdditionOrderInfo aoi;
                memset(&aoi, 0, sizeof(AdditionOrderInfo));
                aoi.mkType = "-1";
                aoi.orderType = orderType;
                ptradeApi->reqOrderInsert(&orderField,&aoi);
            }
        }

    }else{
        LOG(INFO) << "trade price=" + boost::lexical_cast<string>(tradePrice) + ",between askPrice1=" + boost::lexical_cast<string>(askPrice1) +
                      " and bidPrice1=" + boost::lexical_cast<string>(bidPrice1) + ",ignore agg mode.";
    }

    //process aggressive mode trade,to check if all order is filled!!
    //upper only judge,if start aggressive mode
    if(realseInfo->orderType == "500"){//buy open trade,so we will sell close directly.

        LOG(INFO) << "this is aggressive trade.buy open, trade price=" + boost::lexical_cast<string>(tradePrice) + ",we will send close open order directly.";
        double aggPrice = tradePrice + tick;
        unsigned int newOrderToken = iOrderRef++;
        unsigned char m_side = 0;
        int volume = realseInfo->Volume;//real trade volume
        OrderInfo* orderInfo = new OrderInfo();
        orderInfo->userID = USER_ID;
        orderInfo->clientOrderToken = newOrderToken;
        string dir = "1";
        string orderType = "511";//fok of sell close
        string offsetFlag = getCloseMethod(instrumentID,"buy");
        m_side = changeSignalFromNormalToSL(dir,offsetFlag);//trade close signal
        orderInfo->m_Side = m_side;
        orderInfo->offsetFlag = offsetFlag;
        orderInfo->direction = dir;
        orderInfo->price = aggPrice;//only one tick profit
        orderInfo->orderType = orderType;
        orderInfo->volume = volume;
        orderInfo->instrumentID = instrumentID;
        //aggOrderList.emplace_back(orderInfo);
        LOG(INFO) << "new add sell close list to aggOrderList." + getOrderInfo(orderInfo);
        //shengli
        EES_EnterOrderField orderField;
        memset(&orderField, 0, sizeof(EES_EnterOrderField));
        orderField.m_Tif = EES_OrderTif_IOC;//立即单 EES_OrderTif_Day  EES_OrderTif_IOC
        orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
        strcpy(orderField.m_Account, INVESTOR_ID.c_str());
        strcpy(orderField.m_Symbol, instrumentID.c_str());//modify 2.
        orderField.m_Side = m_side;//modify 1.buy open
        orderField.m_Exchange = EES_ExchangeID_shfe;
        orderField.m_SecType = EES_SecType_fut;
        orderField.m_Price = aggPrice;//modify 3.
        orderField.m_Qty = volume;
        orderField.m_ClientOrderToken = newOrderToken;
        AdditionOrderInfo aoi;
        memset(&aoi, 0, sizeof(AdditionOrderInfo));
        aoi.mkType = "-1";
        aoi.orderType = orderType;
        ptradeApi->reqOrderInsert(&orderField,&aoi);

    }else if(realseInfo->orderType == "510"){//sell open trade,so we will buy close directly.
        LOG(INFO) << "this is aggressive trade.sell open, trade price=" + boost::lexical_cast<string>(tradePrice) + ",we will send buy close order directly.";
        double aggPrice = tradePrice - tick;
        unsigned int newOrderToken = iOrderRef++;
        unsigned char m_side = 0;
        int volume = realseInfo->Volume;//real trade volume
        string dir = "0";
        string orderType = "501";//fok of sell close
        string offsetFlag = getCloseMethod(instrumentID,"sell");
        OrderInfo* orderInfo = new OrderInfo();
        orderInfo->userID = USER_ID;
        orderInfo->clientOrderToken = newOrderToken;
        m_side = changeSignalFromNormalToSL(dir,offsetFlag);//trade close signal
        orderInfo->m_Side = m_side;
        orderInfo->offsetFlag = offsetFlag;
        orderInfo->direction = dir;
        orderInfo->price = aggPrice;//only one tick profit
        orderInfo->orderType = orderType;
        orderInfo->volume = volume;
        orderInfo->instrumentID = instrumentID;
       // aggOrderList.emplace_back(orderInfo);
        LOG(INFO) << "new add buy close list to aggOrderList." + getOrderInfo(orderInfo);
        //shengli
        EES_EnterOrderField orderField;
        memset(&orderField, 0, sizeof(EES_EnterOrderField));
        orderField.m_Tif = EES_OrderTif_IOC;//立即单 EES_OrderTif_Day  EES_OrderTif_IOC
        orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
        strcpy(orderField.m_Account, INVESTOR_ID.c_str());
        strcpy(orderField.m_Symbol, instrumentID.c_str());//modify 2.
        orderField.m_Side = m_side;//modify 1.buy open
        orderField.m_Exchange = EES_ExchangeID_shfe;
        orderField.m_SecType = EES_SecType_fut;
        orderField.m_Price = aggPrice;//modify 3.
        orderField.m_Qty = volume;
        orderField.m_ClientOrderToken = newOrderToken;
        AdditionOrderInfo aoi;
        memset(&aoi, 0, sizeof(AdditionOrderInfo));
        aoi.mkType = "-1";
        aoi.orderType = orderType;
        ptradeApi->reqOrderInsert(&orderField,&aoi);

    }else if(realseInfo->orderType == "501"){//buy close trade,judge if need make up other close trade.for there will be only part of volume is filled.
        LOG(INFO) << "this is aggressive trade.buy close, trade price=" + boost::lexical_cast<string>(tradePrice) + ",we will judge if need make up other close trade.for there will be only part of volume is filled.";

        int oriVolume = realseInfo->VolumeTotalOriginal;
        int realTradeVol = realseInfo->Volume;
        if((oriVolume-realTradeVol) > 0){//part of open volume is filled,need make up,so keep original info
            isOriginalDelete = false;
        }
    }else if(realseInfo->orderType == "511"){//buy close trade,judge if need make up other close trade.for there will be only part of volume is filled.
        LOG(INFO) << "this is aggressive trade.sell close, trade price=" + boost::lexical_cast<string>(tradePrice) + ",we will judge if need make up other close trade.for there will be only part of volume is filled.";

        int oriVolume = realseInfo->VolumeTotalOriginal;
        int realTradeVol = realseInfo->Volume;
        if((oriVolume-realTradeVol) > 0){//part of open volume is filled,need make up orders.
           isOriginalDelete = false;
        }
    }
    if(isOriginalDelete){
        deleteOriOrder(realseInfo->clientOrderToken);
    }

}
void TraderDemo::OnOrderCxled(EES_OrderCxled* pOrder){
    boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
    //开始时间
    boost::posix_time::ptime startTime = getCurrentTimeByBoost();

    //报单结构体
    OriginalOrderFieldInfo* oriOrderField;
    string orid = boost::lexical_cast<string>(pOrder->m_Userid) + boost::lexical_cast<string>(pOrder->m_ClientOrderToken);
    LOG(INFO) << "--->>> OnOrderCxled: orid="+orid;
    unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.find(orid);
    if(it != originalOrderMap.end()){
        oriOrderField = it->second;
    }else{
        LOG(ERROR) << "ORDER's useid = " + boost::lexical_cast<string>(pOrder->m_Userid) + ",is not our useid=" + boost::lexical_cast<string>(USER_ID);
        return;
    }
    //撤单，说明报单未成功;
    //正常撤单都是由frontid+sessionid+orderref选择；但是手动撤单采用ordersysid+brokerorderseq
    //判断是否是不活跃合约
    transformFromExchangeOrder(oriOrderField,cancledOrderInfo);
    if(cancledOrderInfo->OffsetFlag != "0"){
        //cancel use trade volume
        processHowManyHoldsCanBeClose(cancledOrderInfo,"release");//释放持仓
        /*
        if(isNormalTrade(oriOrderField->orderType)){//
            processNormalHowManyHoldsCanBeClose(cancledOrderInfo,"release");
        }*/
    }

    string orderType = oriOrderField->orderType;
    string mkType = oriOrderField->mkType;
    string function = oriOrderField->function;
    string timeFlag = oriOrderField->timeFlag;
    //delete
    deleteOriOrder(oriOrderField->clientOrderToken);//1
    originalOrderMap.erase(it);
    //untraded and elapse time is less than 200ms,go on inserting.
    unordered_map<string, ControlOrderInfo*>::iterator ctmIT = controlTimeMap.find(timeFlag);
    if(ctmIT != controlTimeMap.end()){
        ControlOrderInfo* ctlInfo = ctmIT->second;
        struct timeval dwStart = ctlInfo->timeStart;
        struct timeval dwEnd;
        gettimeofday(&dwEnd,NULL);
        unsigned long dwTime=0;
        dwTime = 1000*(dwEnd.tv_sec-dwStart.tv_sec)+(dwEnd.tv_usec-dwStart.tv_usec)/1000;
        //string msg="--->elapse time is "+num2str(dwTime)+" ms,laugh nums="+num2str(ctlInfo->howManyOrderLaugh);
        //cout<<"--->elapse time is "<<num2str(dwTime)<<endl;
        //LOG(INFO) <<msg;
        if(function=="0"&&dwTime<=timeBias){
            LOG(INFO) << "ioc order not traded and elapse time is "+num2str(dwTime)+" ms" + "go on inserting.";
            //boost::this_thread::sleep(boost::posix_time::microseconds(1000));
            AdditionOrderInfo* addinfo=new AdditionOrderInfo();
            addinfo->timeFlag = timeFlag;
            addinfo->function = "0";
            addNewOrderTrade(oriOrderField->instrumentID,cancledOrderInfo->Direction,cancledOrderInfo->OffsetFlag,oriOrderField->m_Price,oriOrderField->volumeTotalOriginal,"agg",addinfo);
        }
    }else{
        LOG(INFO) << "timeFlag="+ timeFlag +".can't find order insert controlmap.this order may be other type order.";
    }

    unsigned int clientOrderToken = oriOrderField->clientOrderToken;
    //process hold_position,change some info.
    for(list<HoldPositionDetail*>::iterator hdIT = holdPositionList.begin();hdIT != holdPositionList.end();hdIT++){
        HoldPositionDetail* hpd = *hdIT;
        if(hpd->closeClientOrderToken == clientOrderToken){//find hold position
            LOG(INFO) << "find hold position,modify this order'pl from " + hpd->pl + " to 0";
            hpd->pl = "0";
            hpd->closeClientOrderToken=0;
            hpd->closeMarketOrderToken=0;
            if(function=="200"){//stop profit order action,mean time is over or price revert to bad direction,should insert stop loss order.
                double lowerPrice = 0;
                double higherPrice = 0;
                unordered_map<string, InstrumentInfo*>::iterator insIT = instruments.find(oriOrderField->instrumentID);
                if(insIT != instruments.end()){
                    InstrumentInfo* info = insIT->second;
                    lowerPrice = info->LowerLimitPrice;
                    higherPrice = info->UpperLimitPrice;
                }else{
                    LOG(ERROR) << "can't find instrumentID=" + oriOrderField->instrumentID + "'s InstrumentInfo.";
                }
                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                addinfo->function = "100";
                hpd->pl = "100";//may be change this flag in order accept.
                if(orderType == "01"){//buy close
                    addNewOrderTrade(oriOrderField->instrumentID,cancledOrderInfo->Direction,cancledOrderInfo->OffsetFlag,higherPrice,oriOrderField->volumeTotalOriginal,"agg",addinfo);
                }else if(orderType == "11"){//sell close
                    addNewOrderTrade(oriOrderField->instrumentID,cancledOrderInfo->Direction,cancledOrderInfo->OffsetFlag,lowerPrice,oriOrderField->volumeTotalOriginal,"agg",addinfo);
                }
                LOG(INFO) << "modify this order'pl to " + hpd->pl + ",change closeClientOrderToken from " +
                              boost::lexical_cast<string>(hpd->closeClientOrderToken) + " to " + boost::lexical_cast<string>(addinfo->clientOrderToken);
                hpd->closeClientOrderToken = addinfo->clientOrderToken;

            }/*else if(mkType == "pas"&&function=="200"){
                hpd->pl = "0";
                hpd->closeClientOrderToken = 0;
                LOG(INFO) << "modify this order'pl to " + hpd->pl + ",change closeClientOrderToken from " +
                              boost::lexical_cast<string>(hpd->closeClientOrderToken) + " to 0.";

            }else if(mkType == "agg"&&(function=="100"||function=="200")){
                hpd->pl = "0";
                hpd->closeClientOrderToken = 0;
                LOG(INFO) << "modify this order'pl to " + hpd->pl + ",change closeClientOrderToken from " +
                              boost::lexical_cast<string>(hpd->closeClientOrderToken) + " to 0.";
            }*/else{
                LOG(ERROR) << "ERROR:KNOWN function="+function+",mktype="+mkType;
            }
            break;
        }
    }

    //end
    /////////////////////////////////////////////////////////////

    //结束时间
    boost::posix_time::ptime endTime = getCurrentTimeByBoost();
    int seconds = getTimeInterval(startTime, endTime, "t");
    if(isLogout){
        LOG(INFO) << "OnOrderCxled,处理时长=" + boost::lexical_cast<string>(seconds);
    }
}
void afterTradeProcessor(TradeFieldInfo* tradeInfo){
    LOG(INFO) <<"after order execution,do some research!";
    string instrumentID = tradeInfo->InstrumentID;

    double realTradePrice = tradeInfo->Price;
    double lastPrice = realTradePrice;
    double tickPrice = getPriceTick(instrumentID);
    int priceLevel = 1;
    getAvailableClosePosition(instrumentID);
    //passive market maker logistic
    for(int i = 0;i < priceLevel;i ++){
        //buy order
        {
            double newOrderPrice = realTradePrice - (i+1)*tickPrice;
            int orderCount = getPriceExistAmount(instrumentID,"0",newOrderPrice,lastPrice,tickPrice);
            if(orderCount < amountCanExist){
                LOG(INFO) << "bidList:there are "+ boost::lexical_cast<string>(orderCount) + " orders on price=" + boost::lexical_cast<string>(newOrderPrice) + ",not over metrick=" + boost::lexical_cast<string>(amountCanExist) + ",need new order insert.";
                string flag = "0";
                /*
                if(holdInfo->shortTotalPosition >= overVolume){
                    flag = "1";
                    LOG(INFO) << "short position =" + boost::lexical_cast<string>(holdInfo->shortTotalPosition) + " over "  + boost::lexical_cast<string>(overVolume)+ ",buy open change to buy close.";
                }*/
                AdditionOrderInfo* addinfo = new AdditionOrderInfo();
                if((holdInfo->longTotalPosition - holdInfo->shortTotalPosition)<=volMetric){//don't go on opening
                    LOG(INFO) << "longTotalPosition=" + boost::lexical_cast<string>(holdInfo->longTotalPosition) + ",short pst=" +
                                  boost::lexical_cast<string>(holdInfo->shortTotalPosition) +",not over volMetric=" + boost::lexical_cast<string>(volMetric)+",ok.";
                    //addNewOrder(instrumentID,"0",flag,newOrderPrice,amountCanExist-orderCount,"pas",NULL);
                    addNewOrderTrade(instrumentID,"0",flag,newOrderPrice,amountCanExist-orderCount,"pas",addinfo);
                }else{
                    LOG(INFO) << "longTotalPosition=" + boost::lexical_cast<string>(holdInfo->longTotalPosition) + ",short pst=" +
                                  boost::lexical_cast<string>(holdInfo->shortTotalPosition) +",over volMetric=" + boost::lexical_cast<string>(volMetric)+",buy open is baned.";
                }
                //addNewOrderTrade(instrumentID,"0",flag,newOrderPrice,amountCanExist-orderCount,"pas",addinfo);
            }else{
                LOG(INFO) << "bidList:there are "+ boost::lexical_cast<string>(orderCount) + " orders on price=" + boost::lexical_cast<string>(newOrderPrice) + ",over metrick=" + boost::lexical_cast<string>(amountCanExist) + ",no new order insert.";
            }
        }
        {
            double newOrderPrice = realTradePrice + (i+1)*tickPrice;
            int orderCount = getPriceExistAmount(instrumentID,"1",newOrderPrice,lastPrice,tickPrice);
            if(orderCount < amountCanExist){
                LOG(INFO) << "askList:there are "+ boost::lexical_cast<string>(orderCount) + " orders on price=" + boost::lexical_cast<string>(newOrderPrice) + ",not over metrick=" + boost::lexical_cast<string>(amountCanExist) + ",need new order insert.";
                string flag = "0";
                /*
                if(holdInfo->longTotalPosition >= overVolume){
                    flag = "1";
                    LOG(INFO) << "long position =" + boost::lexical_cast<string>(holdInfo->longTotalPosition) + " over "  + boost::lexical_cast<string>(overVolume)+ ",sell open change to sell close.";
                }*/
                AdditionOrderInfo* addinfo = new AdditionOrderInfo();
                if((holdInfo->shortTotalPosition - holdInfo->longTotalPosition)<=volMetric){//don't go on opening
                    LOG(INFO) << "longTotalPosition=" + boost::lexical_cast<string>(holdInfo->longTotalPosition) + ",short pst=" +
                                  boost::lexical_cast<string>(holdInfo->shortTotalPosition) +",not over volMetric=" + boost::lexical_cast<string>(volMetric)+",ok.";
                    //addNewOrder(instrumentID,"1",flag,newOrderPrice,amountCanExist-orderCount,"pas",NULL);
                    addNewOrderTrade(instrumentID,"1",flag,newOrderPrice,amountCanExist-orderCount,"pas",addinfo);
                }else{
                    LOG(INFO) << "longTotalPosition=" + boost::lexical_cast<string>(holdInfo->longTotalPosition) + ",short pst=" +
                                  boost::lexical_cast<string>(holdInfo->shortTotalPosition) +",over volMetric=" + boost::lexical_cast<string>(volMetric)+",buy open is baned.";
                }
                //addNewOrderTrade(instrumentID,"1",flag,newOrderPrice,amountCanExist-orderCount,"pas",addinfo);
            }else{
                LOG(INFO) << "askList:there are "+ boost::lexical_cast<string>(orderCount) + " orders on price=" + boost::lexical_cast<string>(newOrderPrice) + ",over metrick=" + boost::lexical_cast<string>(amountCanExist) + ",no new order insert.";
            }
        }
    }
}

void TraderDemo::OnCxlOrderReject(EES_CxlOrderRej* pReject)
{
    LOG(INFO) << "--->>> OnCxlOrderReject: ";
    char GrammerText[1024];
    char RiskText[1024];
    OriginalOrderFieldInfo* oriOrderField;
    string omID = boost::lexical_cast<string>(pReject->m_MarketOrderToken);
    unordered_map<string, string>::iterator exgIT = exgToOriOrderMap.find(omID);
    unordered_map<string, OriginalOrderFieldInfo*>::iterator it;
    if(exgIT != exgToOriOrderMap.end()){//this is my order;
        string orid = exgIT->second;
        it = originalOrderMap.find(orid);
        if(it != originalOrderMap.end()){//get order
            oriOrderField = it->second;
        }else{
            LOG(ERROR) << "miss this cancel!!orid="+orid+",marketToken = " + omID + " and userID = " +  boost::lexical_cast<string>(pReject->m_account) + ",can't match original useid=" + boost::lexical_cast<string>(USER_ID) + "'s order!!";
            LOG(INFO) << "is not my cancel.";
            return;
        }
    }else{
        LOG(ERROR) << "ommit cancle!!marketToken = " + omID + " and userID = " + boost::lexical_cast<string>(pReject->m_account) + ",is not our useid=" + boost::lexical_cast<string>(USER_ID) + "'s order!!";
        LOG(INFO) << "is not my cancle.";
        return;
    }
    LOG(ERROR) << "ERROR:OnOrderReject.m_MarketOrderToken=" + boost::lexical_cast<string>(pReject->m_MarketOrderToken) + ",ReasonCode=" + boost::lexical_cast<string>(pReject->m_ReasonCode)  +
                  ",reasonText=" + boost::lexical_cast<string>(pReject->m_ReasonText) ;
    deleteOriOrder(oriOrderField->clientOrderToken);
    originalOrderMap.erase(it);
    printf("---------------------------------------------------------\n");
    printf("OnCxlOrderReject\n");
    printf("---------------------------------------------------------\n");
    printf("m_MarketOrderToken:     %lld\n", pReject->m_MarketOrderToken);
    printf("m_ReasonCode      :     %u\n", pReject->m_ReasonCode);
    printf("m_ReasonText    :     %s\n", pReject->m_ReasonText);
    printf("\n");

}
//以分隔符方式记录投资者报单委托信息
string getInvestorOrderInsertInfoByDelimater(EES_EnterOrderField *order){
    string orderinfo = "";
    //char tmp[2]={order->m_Exchange,'\0'};
    orderinfo += "account=" + boost::lexical_cast<string>(order->m_Account) + ";";
    orderinfo += "m_ClientOrderToken=" + boost::lexical_cast<string>(order->m_ClientOrderToken) + ";";
    orderinfo += "m_Exchange=" + string({order->m_Exchange,'\0'}) + ";";
    orderinfo += "m_HedgeFlag=" + string({order->m_HedgeFlag,'\0'}) + ";";
    orderinfo += "m_MarketSessionId=" + string({order->m_MarketSessionId,'\0'}) + ";";
    orderinfo += "m_MinQty=" + boost::lexical_cast<string>(order->m_MinQty) + ";";
    orderinfo += "m_OptExecFlag=" + string({order->m_OptExecFlag,'\0'}) + ";";
    orderinfo += "m_Price=" + boost::lexical_cast<string>(order->m_Price) + ";";
    orderinfo += "m_Qty=" + boost::lexical_cast<string>(order->m_Qty) + ";";
    char s1[2] = {order->m_SecType,'\0'};
    char s2[2] = {order->m_Side,'\0'};
    orderinfo += "m_SecType=" + string(s1) + ";";
    orderinfo += "m_Side=" + string(s2) + ";";
    orderinfo += "m_Symbol=" + boost::lexical_cast<string>(order->m_Symbol) + ";";
    orderinfo += "m_Tif=" + boost::lexical_cast<string>(order->m_Tif) + ";";
    return orderinfo;
}
