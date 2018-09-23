//#include "MdSpi.h"
//#include "ctpapi/ThostFtdcTraderApi.h"
//#include "TraderSpi.h"
#include "property.h"
#include "EESTraderDemo.h"
#include "TimeProcesser.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <mutex>
#include <glog/logging.h>
#include <boost/thread/recursive_mutex.hpp>
using namespace std;
extern HoldPositionInfo userHoldPst;//not real hold position info
extern list<WaitForCloseInfo*> allTradeList;//before one normal
extern list<WaitForCloseInfo*> longReverseList;//before one normal
extern list<WaitForCloseInfo*> tmpLongReverseList;//before one normal
extern Strategy techCls;
extern unordered_map<string, HoldPositionInfo*> reversePosition;
/*****************************marketdata*/
//gap list map
extern unordered_map<string, HoldPositionInfo*> normalMMPositionmap;
extern unordered_map<double,vector<double>> map_price_gap;
extern double profitTickNums;//盈利数值
extern double lossTickNums;
extern HoldPositionInfo* holdInfo;
extern list<OrderInfo*> bidList;//order list
extern list<OrderInfo*> askList;//
extern list<OrderInfo*> longList;// trade list
extern list<OrderInfo*> shortList;//trade list
extern int volMetric;
extern int overVolume;
extern double floatMetric;//use for compare tow double
extern int orderPriceLevel;

extern int positionHoldTime;//how long positon hold,count by second.
extern bool isLogout;
extern string tradingDayT;
extern string hedgeFlag;//账号类型，组合投机套保标 投机 '1'套保 '3'
extern int defaultVolume;//默认下单手数
extern double profitValue;
extern int gapCanBeOpenNums;//每个gap上面可以开仓的数量
extern int USER_ID;
extern string INVESTOR_ID ;			// 投资者代码
extern char	FRONT_ID;	//前置编号
extern int	SESSION_ID;	//会话编号
extern int start_process;
extern recursive_mutex g_lock_ti;//tradeinfo lock
extern recursive_mutex g_lock_log;//log lock
extern unordered_map<string, TechMetric*> techMetricMap;//技术指标
extern unordered_map<string, MarketData*> marketdata_map;
extern unordered_map<string, list<TradeInfo*>*> willTradeMap;			//触发套利单时，保存套利信息
extern unordered_map<string, list<TradeInfo*>*> alreadyTradeMapAS;			//保存已成交套利单信息,正套
extern unordered_map<string, list<TradeInfo*>*> alreadyTradeMapDS;			//保存已成交套利单信息,反套
extern unordered_map<string, bool> holdPstIsLocked;			//保存已经锁仓的报单，防止多次未知单回报导致的多次锁仓情况。
extern unordered_map<string, list<TradeInfo*>*> stopProfitWillTradeMap;//止盈报单
extern unordered_map<string, OriginalOrderFieldInfo*> originalOrderMap;//userid send order
extern unordered_map<string, vector<string>> instr_map;				//一个合约和哪些合约配对
extern boost::lockfree::queue<LogMsg*> logqueue;///日志消息队列
extern boost::lockfree::queue<LogMsg*> networkTradeQueue;///报单、成交消息队列,发送到客户端线程使用
extern boost::lockfree::queue<PriceGapMarketData*> detectOpenQueue;///开仓行情处理列表
extern boost::lockfree::queue<PriceGapMarketData*> detectCloseQueue;///止盈行情处理列表
extern boost::lockfree::queue<PriceGapMarketData*> strategyQueue;///处理列表

                                                                    //extern unordered_map<string, double> instr_price_map;					//合约的价格
extern unordered_map<string, PriceGap*> instr_price_gap;			//价格差
extern unordered_map<string, InstrumentInfo*> instruments;			//合约信息
extern unordered_map<string, list<HoldPositionDetail*>*> positionDetailMap;//持仓明细
extern unordered_map<string, list<HoldPositionDetail*>*> pstDetailFromDB;//持仓明细
extern unordered_map<string, ControlOrderInfo*> controlTimeMap;//control order insert number
//extern list<string> loglist;		///日志消息队列
extern boost::atomic_int32_t orderSeq;//组合报单序号
extern int notActiveInsertAmount;//不活跃合约重复下单次数
//extern double stopLossTickNums;//价差往不利方向损失多少tick时候，组合止损
extern int arbVolumeMetric;//套利单总共能下多少手，单边
extern int biasTickNums;//价格偏移多少个tick进行追单
extern int arbVolume;//当前持仓量
extern int maxUntradeNums;//最大未成交套利单笔数(非手数，手数=maxUntradeNums*defaultVolume)
extern int maxFollowTimes;//最大追单次数
//extern int overMAGapTickNums;//超过均值之上几个tick才开仓
                          //extern unordered_map<string, unordered_map<string, int>> positionmap;
extern unordered_map<string, HoldPositionInfo*> positionmap;
extern unordered_map<string, unordered_map<string, int64_t>> seq_map_orderref;
extern unordered_map<string, string> seq_map_ordersysid;
extern list<HoldPositionDetail*> holdPositionList;//position list
extern int realLongPstLimit;
extern int realShortPstLimit;
extern int lastABSSpread;
extern int firstGap;
extern int secondGap;
extern string systemID ;//系统编号，每个产品一个编号
extern double openTick ;//开仓时候每次增加的tick值5
extern double closeTick;//平仓时每次增加的tick值15
extern double extreamTick; //: 偏离到达极值之后每次增加的tick值30
extern double extreamPriceGap;//当前价差最大值
extern double maPriceGap;//当前价差平均值
//boost::atomic_int32_t orderID(0);
//boost::atomic_int32_t logLength(0);
//delayed ask order map
unordered_map<double, unordered_map<string, int32_t>> delayedAskOrderMap;
unordered_map<double, unordered_map<string, int32_t>> delayedBidOrderMap;
extern list<WaitForCloseInfo*> allASTradeList;//longex
extern list<WaitForCloseInfo*> allDSTradeList;//long
extern double asTradeListMinGap;
extern double asTradeListMaxGap;
extern double dsTradeListMinGap;
extern double dsTradeListMaxGap;
//all delayed order
//unordered_map<int, CThostFtdcInputOrderField*> allDelayedOrder;
//交易对象
extern TraderDemo* ptradeApi;
//extern CTraderSpi* pTradeUserSpi;
// 请求编号
extern int iRequestID;
// 报单引用
extern int iOrderRef;

//买平标志,1开仓；2平仓
extern int longPstIsClose;
extern int shortPstIsClose;
///////////////锁//////////////////
//positionmap可重入锁
boost::recursive_mutex pst_mtx;
//orderinsertkey与ordersysid对应关系锁
boost::recursive_mutex order_mtx;
//action order lock
boost::recursive_mutex actionOrderMTX;
//套利组合单列表锁
boost::recursive_mutex willTrade_mtx;
//盈利组合单列表锁
boost::recursive_mutex stopProfit_mtx;
//等待止盈套利组合单列表锁
boost::recursive_mutex alreadyTrade_mtx;
//查询组合持仓的gap
extern boost::recursive_mutex priceGap_mtx;
//extern boost::recursive_mutex arbVolume_mtx;//套利组合单数量的gap
extern boost::recursive_mutex queryPst_mtx;//查询各种持仓
extern boost::recursive_mutex techMetric_mtx;//技术指标
//行情锁
boost::recursive_mutex mkdata_mtx;
extern boost::recursive_mutex pstDetail_mtx;//持仓明细
extern boost::recursive_mutex pstDetailDB_mtx;//持仓明细
extern int isTwoStartStrategy;//等于2的时候，表示明细和汇总持仓查询完毕，启动系统
//longpstlimit
extern int longpstlimit;
//shortpstlimit
extern int shortpstlimit;
//记录时间
extern int long_offset_flag;
extern int short_offset_flag;
//日志保存路径
string logPath = "tradelog.txt";
string getComInstrumentKey(string instr_s1, string instr_s2) {
    string ins_com_key = "";
    //价格差初始化
    if (instr_s1 > instr_s2) {//Key按照近月-远月的格式
        ins_com_key = instr_s2 + "-" + instr_s1;
    } else {
        ins_com_key = instr_s1 + "-" + instr_s2;
    }
    return ins_com_key;
}
void defineMainDirection(){


}

/*
根据传入的合约ID判断是套利报单的活跃还是不活跃合约.手动撤单需要验证一下，是否一定能够根据sessionID_frontID+orderRef匹配到.现在撤单都是根据该标识查找
手动撤单，基本都是活跃合约的；
回报处理使用
"1":不活跃合约-开仓
"2":活跃合约-开仓
"3":不活跃合约-止盈平仓
"4":活跃合约-止盈平仓
"0":未查找到套利配对
*/
int* decideOrderType(OrderFieldInfo *pOrder) {
    //lock_guard<recursive_mutex> locker(g_lock_ti);
    string instrumentid = pOrder->InstrumentID;
    string orderRef = pOrder->OrderRef;
    unsigned int clientToken = pOrder->clientOrderToken;
    LOG(INFO) << ("-->>decideOrderType:clientToken= " + boost::lexical_cast<string>(clientToken));
    int* temp = new int[2];
    int rst = 0;
    int insertAmount = 0;//不活跃合约持续报单次数,需要用到
    unordered_map<string, vector<string>>::iterator vec_it = instr_map.find(instrumentid);
    if (vec_it == instr_map.end()) {
        cout << "can't find instrument list" << endl;
        LOG(INFO) << ("processNotActiveOrder:can't find instrumentid= " + instrumentid);
        rst = 0;
    } else {
        vector<string> tmp_instr_vec = vec_it->second;//找到对应的配对合约
        for (unsigned int i = 0; i < tmp_instr_vec.size(); i++) {
            string pd_instrument = tmp_instr_vec[i];
            string com_ins_key = getComInstrumentKey(pd_instrument, instrumentid);
            /*从开仓序列和止盈序列中，分别找对应的报单列表*/
            //一、开仓单中查找
            unordered_map<string, list<TradeInfo*>*>::iterator untrade_it = willTradeMap.find(com_ins_key);
            if (untrade_it != willTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = untrade_it->second;
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); tmpit++) {
                    TradeInfo* tradeinfo = *tmpit;
                    if (tradeinfo->notActiveInstrumentid == instrumentid && clientToken == tradeinfo->notActiveClientOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        rst = 1;
                        insertAmount = tradeinfo->notActiveInsertAmount;
                        break;
                    } else if (tradeinfo->activeInstrumentid == instrumentid && tradeinfo->activeClientOrderToken == clientToken && !tradeinfo->activeIsTraded) {//活跃合约的报单
                        rst = 2;
                        break;
                    }
                }
            }else{
                LOG(INFO) << "from willTradeMap can't find comInstrumentKey=" + com_ins_key;
            }
            //二、止盈单中查找
            unordered_map<string, list<TradeInfo*>*>::iterator stopprofit_untrade_it = stopProfitWillTradeMap.find(com_ins_key);
            if (stopprofit_untrade_it != stopProfitWillTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tradeinfo = stopprofit_untrade_it->second;
                for (list<TradeInfo*>::iterator tmpit = tradeinfo->begin(); tmpit != tradeinfo->end(); tmpit++) {
                    TradeInfo* tradeinfo = *tmpit;
                    if (tradeinfo->notActiveInstrumentid == instrumentid && clientToken == tradeinfo->notActiveClientOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        rst = 3;
                        insertAmount = tradeinfo->notActiveInsertAmount;
                        break;
                    } else if (tradeinfo->activeInstrumentid == instrumentid && tradeinfo->activeClientOrderToken == clientToken && !tradeinfo->activeIsTraded) {//活跃合约的报单
                        rst = 4;
                        break;
                    }
                }/*
                if (tradeinfo->notActiveInstrumentid == instrumentid && orderRef == tradeinfo->notActiveOrderRef && pOrder->SessionID == tradeinfo->sessionID && pOrder->FrontID == tradeinfo->frontID && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                    rst = 3;
                    insertAmount = tradeinfo->notActiveInsertAmount;
                } else if (tradeinfo->activeInstrumentid == instrumentid && tradeinfo->activeOrderRef == orderRef && pOrder->SessionID == tradeinfo->sessionID && pOrder->FrontID == tradeinfo->frontID && !tradeinfo->activeIsTraded) {//活跃合约的报单
                    rst = 4;
                }*/
            }else{
                LOG(INFO) << "from stopProfitWillTradeMap can't find comInstrumentKey=" + com_ins_key;
            }
        }
    }
    temp[0] = rst;
    temp[1] = insertAmount;
    string tmpmsg = "";
    if (rst == 0) {
        tmpmsg = "(手动撤单),断线重连，无法找到对应报单";
        LOG(INFO) << "根据撤单回报，查找合约组合。" +  tmpmsg;
    } else {
        LOG(INFO) << "根据撤单回报，查找到合约组合。当前组合中不活跃合约尝试下单次数当前为:" + boost::lexical_cast<string>(insertAmount) + ";处理撤单结果=" + boost::lexical_cast<string>(rst);
    }

    return temp;
}
string resetOrderRef(OrderFieldInfo *pOrder, unsigned int oldClientToken, unsigned int newClientToken) {
    //lock_guard<recursive_mutex> locker(g_lock_ti);
    string instrumentID = pOrder->InstrumentID;
    string rst = "0";
    unordered_map<string, vector<string>>::iterator vec_it = instr_map.find(instrumentID);
    if (vec_it == instr_map.end()) {
        cout << "can't find instrument list." << endl;
        LOG(INFO) << ("resetOrderRef:can't find instrumentid= " + instrumentID);
        rst = "0";
    } else {
        vector<string> tmp_instr_vec = vec_it->second;//找到对应的配对合约
        for (unsigned int i = 0; i < tmp_instr_vec.size(); i++) {
            string pd_instrument = tmp_instr_vec[i];
            string com_ins_key = getComInstrumentKey(pd_instrument, instrumentID);
            //分两类，套利单和止盈单
            unordered_map<string, list<TradeInfo*>*>::iterator untrade_it = willTradeMap.find(com_ins_key);
            if (untrade_it != willTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = untrade_it->second;
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); tmpit++) {
                    TradeInfo* tradeinfo = *tmpit;
                    if (tradeinfo->notActiveInstrumentid == instrumentID && oldClientToken == tradeinfo->notActiveClientOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        tradeinfo->notActiveClientOrderToken = newClientToken;
                        tradeinfo->notActiveMarketOrderToken = 0;
                        LOG(INFO) << ("resetOrderRef:change open notActiveClientOrderToken from " + boost::lexical_cast<string>(oldClientToken) + " to " + boost::lexical_cast<string>(newClientToken));
                        rst = "1";
                        break;
                    } else if (tradeinfo->activeInstrumentid == instrumentID && tradeinfo->activeClientOrderToken == oldClientToken && !tradeinfo->activeIsTraded) {//活跃合约的报单
                        tradeinfo->activeClientOrderToken = newClientToken;
                        tradeinfo->activeMarketOrderToken = 0;
                        LOG(INFO) << ("resetOrderRef:change open activeClientOrderToken from " + boost::lexical_cast<string>(oldClientToken) + " to " + boost::lexical_cast<string>(newClientToken));
                        rst = "1";
                        break;
                    }
                }
            }
            //止盈单
            unordered_map<string, list<TradeInfo*>*>::iterator stop_untrade_it = stopProfitWillTradeMap.find(com_ins_key);
            if (stop_untrade_it != stopProfitWillTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = stop_untrade_it->second;
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); tmpit++) {
                    TradeInfo* tradeinfo = *tmpit;
                    if (tradeinfo->notActiveInstrumentid == instrumentID && oldClientToken == tradeinfo->notActiveClientOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        tradeinfo->notActiveClientOrderToken = newClientToken;
                        tradeinfo->notActiveMarketOrderToken = 0;
                        LOG(INFO) << ("resetOrderRef:change close notActiveClientOrderToken from " + boost::lexical_cast<string>(oldClientToken) + " to " + boost::lexical_cast<string>(newClientToken));
                        rst = "1";
                        break;
                    } else if (tradeinfo->activeInstrumentid == instrumentID && tradeinfo->activeClientOrderToken == oldClientToken && !tradeinfo->activeIsTraded) {//活跃合约的报单
                        tradeinfo->activeClientOrderToken = newClientToken;
                        tradeinfo->activeMarketOrderToken = 0;
                        LOG(INFO) << ("resetOrderRef:change close activeClientOrderToken from " + boost::lexical_cast<string>(oldClientToken) + " to " + boost::lexical_cast<string>(newClientToken));
                        rst = "1";
                        break;
                    }
                }/*
                TradeInfo* tradeinfo = stop_untrade_it->second;
                if (tradeinfo->notActiveInstrumentid == instrumentID && oldOrderRef == tradeinfo->notActiveOrderRef && pOrder->SessionID == tradeinfo->sessionID && pOrder->FrontID == tradeinfo->frontID && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                    tradeinfo->notActiveOrderRef = newOrderRef;
                    LOG(INFO) << ("resetOrderRef:change close notActiveOrderRef from " + oldOrderRef + " to " + newOrderRef);
                    rst = "1";
                } else if (tradeinfo->activeInstrumentid == instrumentID && tradeinfo->activeOrderRef == oldOrderRef && pOrder->SessionID == tradeinfo->sessionID && pOrder->FrontID == tradeinfo->frontID && !tradeinfo->activeIsTraded) {//活跃合约的报单
                    tradeinfo->activeOrderRef = newOrderRef;
                    LOG(INFO) << ("resetOrderRef:change close activeOrderRef from " + oldOrderRef + " to " + newOrderRef);
                    rst = "1";
                }*/
            }
        }
    }
    return rst;
}
void closeOrderInfo() {
    //下单情况
    string orderinfo;
    for (unordered_map<string, list<TradeInfo*>*>::iterator untradeinfo = stopProfitWillTradeMap.begin(); untradeinfo != stopProfitWillTradeMap.end(); untradeinfo++) {
        string key = untradeinfo->first;
        list<TradeInfo*>* tmplist = untradeinfo->second;
        for (list<TradeInfo*>::iterator it = tmplist->begin(); it != tmplist->end();it++) {
            TradeInfo* tradeInfo = *it;
            string notActiveInstrumentid = tradeInfo->notActiveInstrumentid;
            string notActiveClientOrderToken = boost::lexical_cast<string>(tradeInfo->notActiveClientOrderToken);
            string activeClientOrderToken = boost::lexical_cast<string>(tradeInfo->activeClientOrderToken);
            //string notActiveSessionID = tradeInfo->notact
            string activeInstrumentid = tradeInfo->activeInstrumentid;
            string activeOrderInsertPrice = boost::lexical_cast<string>(tradeInfo->activeOrderInsertPrice);
            string notActiveOrderInsertPrice = boost::lexical_cast<string>(tradeInfo->notActiveOrderInsertPrice);
            string activeTradeSide = boost::lexical_cast<string>(tradeInfo->activeTradeSide);
            string notActiveTradeSide = boost::lexical_cast<string>(tradeInfo->notActiveTradeSide);
            string notActiveOffsetFlag = tradeInfo->notActiveOffsetFlag;
            string activeOffsetFlag = tradeInfo->activeOffsetFlag;
            string activeIsTraded = boost::lexical_cast<string>(tradeInfo->activeIsTraded);
            string notActiveIsTraded = boost::lexical_cast<string>(tradeInfo->notActiveIsTraded);
            orderinfo += "notActiveInstrumentid=" + notActiveInstrumentid + ";notActiveOrderInsertPrice=" + notActiveOrderInsertPrice + ";"  +
                "notActiveIsTraded=" + notActiveIsTraded + ";notActiveClientOrderToken=" + notActiveClientOrderToken;
            orderinfo += "activeInstrumentid=" + activeInstrumentid + ";activeOrderInsertPrice=" + activeOrderInsertPrice + ";"  +
                "activeIsTraded=" + activeIsTraded + "activeClientOrderToken=" + activeClientOrderToken;
        }


    }
    LOG(INFO) << ("当前正在处理的止盈情况;" + orderinfo);
}
string processArbRtnOrder(OrderFieldInfo *pOrder) {
    if (start_process == 0) {
        return "";
    }
    string instrumentID = pOrder->InstrumentID;
    string rst = "0";
    unordered_map<string, vector<string>>::iterator vec_it = instr_map.find(instrumentID);
    closeOrderInfo();
    if (vec_it == instr_map.end()) {
        cout << "can't find instrument list." << endl;
        LOG(INFO) << ("processArbRtnOrder:can't find instrumentid= " + instrumentID);
        rst = "0";
    } else {
        vector<string> tmp_instr_vec = vec_it->second;//找到对应的配对合约
        for (unsigned int i = 0; i < tmp_instr_vec.size(); i++) {
            string pd_instrument = tmp_instr_vec[i];
            string com_ins_key = getComInstrumentKey(pd_instrument, instrumentID);
            //套利单
            unordered_map<string, list<TradeInfo*>*>::iterator untrade_it = willTradeMap.find(com_ins_key);
            if (untrade_it != willTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = untrade_it->second;//当前处理报单，只以sessionID+frontID+orderRef为标志；后续添加以orderLocalID+BrokerOrderSeq
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); tmpit++) {
                    TradeInfo* tradeinfo = *tmpit;
                    if (tradeinfo->notActiveInstrumentid == instrumentID && pOrder->clientOrderToken == tradeinfo->notActiveClientOrderToken &&  !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        OriginalOrderFieldInfo* oriOrderField;
                        for(unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.begin();it != originalOrderMap.end();it++){
                            oriOrderField = it->second;
                            if(oriOrderField->clientOrderToken == tradeinfo->notActiveClientOrderToken){
                                tradeinfo->notActiveOrderStatus = oriOrderField->orderStatus;
                                LOG(INFO) << "set clientOrderToken=" + boost::lexical_cast<string>(tradeinfo->notActiveClientOrderToken) + "'s order status =" + boost::lexical_cast<string>(oriOrderField->orderStatus);
                                break;
                            }

                        }
                        if(tradeinfo->notActiveMarketOrderToken == 0){
                            tradeinfo->notActiveMarketOrderToken = pOrder->marketOrderToken;
                            rst = "1";
                            LOG(INFO) << "找到对应的套利不活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",notActiveMarketOrderToken set to " + boost::lexical_cast<string>(tradeinfo->notActiveMarketOrderToken);
                            break;
                        }else{
                            rst = "1";
                            LOG(INFO) << "找到对应的套利不活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",notActiveMarketOrderToken has been set to " + boost::lexical_cast<string>(tradeinfo->notActiveMarketOrderToken) + ",current info MarketOrderToken=" + boost::lexical_cast<string>(pOrder->marketOrderToken) + ",no need to do.";
                            break;
                        }

                    } else if (tradeinfo->activeInstrumentid == instrumentID && tradeinfo->activeClientOrderToken == pOrder->clientOrderToken && !tradeinfo->activeIsTraded) {//活跃合约的报单
                        OriginalOrderFieldInfo* oriOrderField;
                        for(unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.begin();it != originalOrderMap.end();it++){
                            oriOrderField = it->second;
                            if(oriOrderField->clientOrderToken == tradeinfo->activeClientOrderToken){
                                tradeinfo->activeOrderStatus = oriOrderField->orderStatus;
                                LOG(INFO) << "set clientOrderToken=" + boost::lexical_cast<string>(tradeinfo->activeClientOrderToken) + "'s order status =" + boost::lexical_cast<string>(oriOrderField->orderStatus);
                                break;
                            }

                        }
                        tradeinfo->activeOrderActionStatus = boost::lexical_cast<string>(oriOrderField->orderStatus);
                        LOG(INFO) << "找到对应的套利活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",activeOrderActionStatus=" + tradeinfo->activeOrderActionStatus;
                        if(tradeinfo->activeMarketOrderToken == 0){
                            tradeinfo->activeMarketOrderToken = pOrder->marketOrderToken;
                            //tradeinfo->notActiveBrokerOrderSeq = pOrder->BrokerOrderSeq;
                            //LOG(INFO)<<("processRtnOrder:change notActiveOrderRef from " + oldOrderRef + " to " + newOrderRef);
                            //if (pOrder->OrderStatus == "2") {//成交信息
                            //    tradeinfo->notActiveIsTraded = true;
                            //}
                            rst = "1";
                            LOG(INFO) << "找到对应的套利活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",activeMarketOrderToken set to " + boost::lexical_cast<string>(tradeinfo->activeMarketOrderToken);
                            break;
                        }else{
                            rst = "1";
                            LOG(INFO) << "找到对应的套利活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",activeMarketOrderToken has been set to " + boost::lexical_cast<string>(tradeinfo->activeMarketOrderToken) + ",current info MarketOrderToken=" + boost::lexical_cast<string>(pOrder->marketOrderToken) + ",no need to do.";
                            break;
                        }
                        //tradeinfo->activeOrderActionStatus = boost::lexical_cast<string>(pOrder->OrderStatus);
                        //rst = "1";
                        //LOG(INFO) << "找到对应的套利活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",OrderStatus=" + pOrder->OrderStatus;
                        //break;
                    }
                }
            } else {
                LOG(INFO) << "未查找到套利单信息,当前套利单信息：" + getAllUntradeInfo();
            }
            //止盈单
            unordered_map<string, list<TradeInfo*>*>::iterator stop_untrade_it = stopProfitWillTradeMap.find(com_ins_key);
            if (stop_untrade_it != stopProfitWillTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = stop_untrade_it->second;
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); tmpit++) {
                    TradeInfo* tradeinfo = *tmpit;//当前处理报单，只以sessionID+frontID+orderRef为标志；后续添加以orderLocalID+BrokerOrderSeq
                    if (tradeinfo->notActiveInstrumentid == instrumentID && pOrder->clientOrderToken == tradeinfo->notActiveClientOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        OriginalOrderFieldInfo* oriOrderField;
                        for(unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.begin();it != originalOrderMap.end();it++){
                            oriOrderField = it->second;
                            if(oriOrderField->clientOrderToken == tradeinfo->notActiveClientOrderToken){
                                tradeinfo->notActiveOrderStatus = oriOrderField->orderStatus;
                                LOG(INFO) << "set clientOrderToken=" + boost::lexical_cast<string>(tradeinfo->notActiveClientOrderToken) + "'s order status =" + boost::lexical_cast<string>(oriOrderField->orderStatus);
                                break;
                            }

                        }
                        //tradeinfo->notActiveOrderLocalID = pOrder->OrderLocalID;
                        //tradeinfo->notActiveOrderStatus = pOrder->OrderStatus;
                        //tradeinfo->notActiveOrderSubmitStatus = pOrder->OrderSubmitStatus;
                        //tradeinfo->notActiveOrderSysID = pOrder->OrderSysID;
                        if(tradeinfo->notActiveMarketOrderToken == 0){
                            tradeinfo->notActiveMarketOrderToken = pOrder->marketOrderToken;
                            //tradeinfo->notActiveBrokerOrderSeq = pOrder->BrokerOrderSeq;
                            //LOG(INFO)<<("processRtnOrder:change notActiveOrderRef from " + oldOrderRef + " to " + newOrderRef);
                            //if (pOrder->OrderStatus == "2") {//成交信息
                            //    tradeinfo->notActiveIsTraded = true;
                            //}
                            rst = "1";
                            LOG(INFO) << "找到对应的止盈不活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",notActiveMarketOrderToken set to " + boost::lexical_cast<string>(tradeinfo->notActiveMarketOrderToken);
                            break;
                        }else{
                            rst = "1";
                            LOG(INFO) << "找到对应的止盈不活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",notActiveMarketOrderToken has been set to " + boost::lexical_cast<string>(tradeinfo->notActiveMarketOrderToken) + ",current info MarketOrderToken=" + boost::lexical_cast<string>(pOrder->marketOrderToken) + ",no need to do.";
                            break;
                        }

                        //tradeinfo->notActiveMarketOrderToken = pOrder->marketOrderToken;
                        //tradeinfo->notActiveBrokerOrderSeq = pOrder->BrokerOrderSeq;
                        //LOG(INFO)<<("processRtnOrder:change notActiveOrderRef from " + oldOrderRef + " to " + newOrderRef);
                        //if (pOrder->OrderStatus == "2") {//成交信息
                        //    tradeinfo->notActiveIsTraded = true;
                        //}
                        ////rst = "1";
                        //LOG(INFO) << "找到对应的止盈不活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",OrderStatus=" + pOrder->OrderStatus;
                        //break;
                    } else if (tradeinfo->activeInstrumentid == instrumentID && tradeinfo->activeClientOrderToken == pOrder->clientOrderToken && !tradeinfo->activeIsTraded) {//活跃合约的报单
                        OriginalOrderFieldInfo* oriOrderField;
                        for(unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.begin();it != originalOrderMap.end();it++){
                            oriOrderField = it->second;
                            if(oriOrderField->clientOrderToken == tradeinfo->activeClientOrderToken){
                                tradeinfo->activeOrderStatus = oriOrderField->orderStatus;
                                LOG(INFO) << "set clientOrderToken=" + boost::lexical_cast<string>(tradeinfo->activeClientOrderToken) + "'s order status =" + boost::lexical_cast<string>(oriOrderField->orderStatus);
                                break;
                            }

                        }
                        tradeinfo->activeOrderActionStatus = boost::lexical_cast<string>(oriOrderField->orderStatus);
                        LOG(INFO) << "找到对应的套利活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",activeOrderActionStatus=" + tradeinfo->activeOrderActionStatus;
                        if(tradeinfo->activeMarketOrderToken == 0){
                            tradeinfo->activeMarketOrderToken = pOrder->marketOrderToken;
                            //tradeinfo->notActiveBrokerOrderSeq = pOrder->BrokerOrderSeq;
                            //LOG(INFO)<<("processRtnOrder:change notActiveOrderRef from " + oldOrderRef + " to " + newOrderRef);
                            //if (pOrder->OrderStatus == "2") {//成交信息
                            //    tradeinfo->notActiveIsTraded = true;
                            //}
                            rst = "1";
                            LOG(INFO) << "找到对应的止盈活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",activeMarketOrderToken set to " + boost::lexical_cast<string>(tradeinfo->activeMarketOrderToken);
                            break;
                        }else{
                            rst = "1";
                            LOG(INFO) << "找到对应的止盈活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",activeMarketOrderToken has been set to " + boost::lexical_cast<string>(tradeinfo->activeMarketOrderToken) + ",current info MarketOrderToken=" + boost::lexical_cast<string>(pOrder->marketOrderToken) + ",no need to do.";
                            break;
                        }
                        //tradeinfo->activeMarketOrderToken = pOrder->marketOrderToken;
                        //tradeinfo->activeBrokerOrderSeq = pOrder->BrokerOrderSeq;
                        //LOG(INFO)<<("resetOrderRef:change activeOrderRef from " + oldOrderRef + " to " + newOrderRef);
                        //if (pOrder->OrderStatus == "2") {//成交信息
                        //    tradeinfo->activeIsTraded = true;
                        //}
                        //tradeinfo->activeOrderActionStatus = boost::lexical_cast<string>(pOrder->OrderStatus);
                        //rst = "1";
                        //LOG(INFO) << "找到对应的止盈活跃合约报单，处理报单结束.instrumentID=" + instrumentID + ",OrderStatus=" + pOrder->OrderStatus;
                        //break;
                    }
                }
            } else {
                LOG(INFO) << "未查找到止盈单信息,当前止盈单信息：" + getAllStopProfitInfo();
            }
        }
    }
    return rst;
}
string processArbRtnTrade(TradeFieldInfo *pTrade) {
    //lock_guard<recursive_mutex> locker(g_lock_ti);
    string instrumentID = pTrade->InstrumentID;
    string rst = "0";
    unordered_map<string, vector<string>>::iterator vec_it = instr_map.find(instrumentID);
    //closeOrderInfo();//记录组合单情况
    cout << "processArbRtnTrade1" << endl;
    if (vec_it == instr_map.end()) {
        cout << "can't find instrument list." << endl;
        LOG(INFO) << ("processArbRtnTrade:can't find instrumentid= " + instrumentID);
        rst = "0";
    } else {
        vector<string> tmp_instr_vec = vec_it->second;//找到对应的配对合约
        for (unsigned int i = 0; i < tmp_instr_vec.size(); i++) {
            string pd_instrument = tmp_instr_vec[i];
            string com_ins_key = getComInstrumentKey(pd_instrument, instrumentID);
            PriceGap* pg = getPriceGap(com_ins_key);
            //int arbComVolume = pg->arbComVolume;//组合单手数上限
            //int arbComVolMetric = pg->arbComVolMetric;//
            cout << "processArbRtnTrade2" << endl;
            //套利单
            unordered_map<string, list<TradeInfo*>*>::iterator untrade_it = willTradeMap.find(com_ins_key);
            if (untrade_it != willTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = untrade_it->second;
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); ) {
                    TradeInfo* tradeinfo = *tmpit;
                    //if (tradeinfo->notActiveInstrumentid == instrumentID && strcmp(tradeinfo->notActiveOrderSysID, pTrade->OrderSysID) == 0 && strcmp(tradeinfo->notActiveOrderLocalID, pTrade->OrderLocalID) == 0) {//不活跃合约的成交报单
                    if (tradeinfo->notActiveInstrumentid == instrumentID && tradeinfo->notActiveMarketOrderToken == pTrade->marketToken ) {//不活跃合约的成交报单
                        tradeinfo->notActiveIsTraded = true;
                        tradeinfo->notActiveRealOpenPrice = pTrade->Price;
                        //tradeinfo->notActiveTradeID = pTrade->TradeID;
                        string orderType = tradeinfo->orderType;
                        if (orderType == "as") {
                            pg->arbComVolumeAS += tradeinfo->notActiveVolume;
                            LOG(INFO) << com_ins_key + ",正套套利合约下单组合次数=" + boost::lexical_cast<string>(pg->arbComVolumeAS) + ",阈值=" + boost::lexical_cast<string>(pg->arbComVolMetricAS);
                        } else if (orderType == "ds") {
                            pg->arbComVolumeDS += tradeinfo->notActiveVolume;
                            LOG(INFO) << com_ins_key + ",反套套利合约下单组合次数=" + boost::lexical_cast<string>(pg->arbComVolumeDS) + ",阈值=" + boost::lexical_cast<string>(pg->arbComVolMetricDS);
                        }
                        pg->holdPositionGap = abs(pg->arbComVolumeAS - pg->arbComVolumeDS);
                        LOG(INFO) << com_ins_key + ",合约持仓差值=" + boost::lexical_cast<string>(pg->holdPositionGap);
                        //不活跃合约成交后，需要下套利活跃合约的限价单
                        if (tradeinfo->activeIsTraded) {//已经成交，不需要再次下单
                            tradeinfo->tradingDay = boost::lexical_cast<string>(pTrade->TradingDay);
                            tradeinfo->tradeTime = boost::lexical_cast<string>(pTrade->TradeTime);
                            addArbitrageOrders(tradeinfo);//添加组合套利单
                            tmpit = tmplist->erase(tmpit);
                            LOG(INFO) << "不活跃合约成交后，需要下套利活跃合约的限价单;活跃合约已经成交，不需要再次下单";
                            break;
                        } else {
                            tmpit++;
                        }
                        //
                        EES_EnterOrderField orderField;
                        memset(&orderField, 0, sizeof(EES_EnterOrderField));
                        //temp.m_Tif = EES_OrderTif_Day;EES_OrderTif_IOC
                        orderField.m_Tif = EES_OrderTif_Day;

                        orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;

                        strcpy(orderField.m_Account, INVESTOR_ID.c_str());
                        strcpy(orderField.m_Symbol, tradeinfo->activeInstrumentid.c_str());
                        orderField.m_Side = tradeinfo->activeTradeSide;
                        //orderField.m_Exchange = EES_ExchangeID_shfe;
                        orderField.m_SecType = tradeinfo->activeSecType;
                        orderField.m_Price = tradeinfo->activeOrderInsertPrice;
                        orderField.m_Qty = tradeinfo->activeVolume;
                        orderField.m_ClientOrderToken = ++iOrderRef;
                        //ptradeApi->reqOrderInsert(&orderField,"100");
                        ptradeApi->reqOrderInsert(&orderField,NULL);
                        //set init orderref
                        tradeinfo->activeOrderRef = boost::lexical_cast<string>(orderField.m_ClientOrderToken);
                        tradeinfo->activeClientOrderToken = orderField.m_ClientOrderToken;
                        LOG(INFO) << ("processArbRtnTrade:open,not active instrument is traded;active instrumentID now orderInsert!");
                        rst = "1";
                        break;
                    //} else if (tradeinfo->activeInstrumentid == instrumentID && strcmp(tradeinfo->activeOrderSysID, pTrade->OrderSysID) == 0 && strcmp(tradeinfo->activeOrderLocalID, pTrade->OrderLocalID) == 0) {//活跃合约的报单
                    } else if (tradeinfo->activeInstrumentid == instrumentID && tradeinfo->activeMarketOrderToken == pTrade->marketToken) {//活跃合约的报单
                        tradeinfo->activeIsTraded = true;
                        tradeinfo->activeRealOpenPrice = pTrade->Price;
                        tradeinfo->activeTradeID = pTrade->TradeID;
                        if (!tradeinfo->notActiveIsTraded) {
                            LOG(ERROR) << ("processArbRtnTrade:open,active instrument is traded.but not active instrument is not traded.something is error!!!!");
                        } else {
                            LOG(INFO) << ("processArbRtnTrade:open,active instrument is traded.");
                            tradeinfo->tradingDay = boost::lexical_cast<string>(pTrade->TradingDay);
                            tradeinfo->tradeTime = boost::lexical_cast<string>(pTrade->TradeTime);
                            addArbitrageOrders(tradeinfo);//添加组合套利单
                            tmpit = tmplist->erase(tmpit);
                        }
                        rst = "1";
                        break;
                    } else {
                        tmpit++;
                    }
                }
            } else {
                LOG(INFO) << ("processArbRtnTrade:未查询到组合套利单:" + com_ins_key);
            }
            cout << "processArbRtnTrade3" << endl;
            //止盈单
            unordered_map<string, list<TradeInfo*>*>::iterator stop_untrade_it = stopProfitWillTradeMap.find(com_ins_key);
            if (stop_untrade_it != stopProfitWillTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = stop_untrade_it->second;
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); ) {
                    TradeInfo* tradeinfo = *tmpit;
                    //if (tradeinfo->notActiveInstrumentid == instrumentID && strcmp(tradeinfo->notActiveOrderSysID, pTrade->OrderSysID) == 0 && strcmp(tradeinfo->notActiveOrderLocalID, pTrade->OrderLocalID) == 0) {//不活跃合约的成交报单
                    if (tradeinfo->notActiveInstrumentid == instrumentID && tradeinfo->notActiveMarketOrderToken == pTrade->marketToken) {//不活跃合约的成交报单
                        tradeinfo->notActiveIsTraded = true;
                        tradeinfo->notActiveRealClosePrice = pTrade->Price;
                        string orderType = tradeinfo->orderType;
                        if(orderType == "as"){
                            pg->arbComVolumeAS -= tradeinfo->notActiveVolume;
                            //arbVolume -= tradeinfo->notActiveVolume;
                            LOG(INFO) << com_ins_key + ",正套止盈之后,套利组合手数=" + boost::lexical_cast<string>(pg->arbComVolumeAS) + ",阈值=" + boost::lexical_cast<string>(pg->arbComVolMetricAS);
                        } else if (orderType == "ds") {
                            pg->arbComVolumeDS -= tradeinfo->notActiveVolume;
                            //arbVolume -= tradeinfo->notActiveVolume;
                            LOG(INFO) << com_ins_key + ",反套止盈之后,套利组合手数=" + boost::lexical_cast<string>(pg->arbComVolumeDS) + ",阈值=" + boost::lexical_cast<string>(pg->arbComVolMetricDS);
                        }
                        pg->holdPositionGap = abs(pg->arbComVolumeAS - pg->arbComVolumeDS);
                        LOG(INFO) << com_ins_key + ",合约持仓差值=" + boost::lexical_cast<string>(pg->holdPositionGap);
                        //不活跃合约成交后，需要下套利活跃合约的限价单
                        if (tradeinfo->activeIsTraded) {//已经成交，不需要再次下单,并删除等待止盈组合列表
                            tradeinfo->tradingDay = boost::lexical_cast<string>(pTrade->TradingDay);
                            tradeinfo->tradeTime = boost::lexical_cast<string>(pTrade->TradeTime);
                            deleteArbitrageOrders(tradeinfo);
                            //stopProfitWillTradeMap.erase(stop_untrade_it);//止盈组合成交，删除报单序列
                            tmpit = tmplist->erase(tmpit);
                            break;
                        }else{
                            tmpit++;
                        }
                        cout << "processArbRtnTrade4" << endl;
                        EES_EnterOrderField orderField;
                        memset(&orderField, 0, sizeof(EES_EnterOrderField));
                        //temp.m_Tif = EES_OrderTif_Day;EES_OrderTif_IOC
                        orderField.m_Tif = EES_OrderTif_Day;
                        orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;

                        strcpy(orderField.m_Account, INVESTOR_ID.c_str());
                        strcpy(orderField.m_Symbol, tradeinfo->activeInstrumentid.c_str());
                        orderField.m_Side = tradeinfo->activeTradeSide;
                        //orderField.m_Exchange = EES_ExchangeID_shfe;
                        orderField.m_SecType = tradeinfo->activeSecType;
                        orderField.m_Price = tradeinfo->activeOrderInsertPrice;
                        orderField.m_Qty = tradeinfo->activeVolume;
                        orderField.m_ClientOrderToken = ++iOrderRef;
                        //ptradeApi->reqOrderInsert(&orderField,"100");
                        ptradeApi->reqOrderInsert(&orderField,NULL);
                        //set init orderref
                        tradeinfo->activeOrderRef = boost::lexical_cast<string>(orderField.m_ClientOrderToken);
                        tradeinfo->activeClientOrderToken = orderField.m_ClientOrderToken;
                        //pTradeUserSpi->ReqOrderInsert(orderstr);
                        //tradeinfo->activeOrderRef = activeOrderRef;
                        LOG(INFO) << ("processArbRtnTrade:close,not active instrument is traded;active instrumentID now orderInsert!");
                        rst = "1";
                        break;
                        //} else if (tradeinfo->activeInstrumentid == instrumentID && strcmp(tradeinfo->activeOrderSysID, pTrade->OrderSysID) == 0 && strcmp(tradeinfo->activeOrderLocalID, pTrade->OrderLocalID) == 0) {//活跃合约的报单
                    } else if (tradeinfo->activeInstrumentid == instrumentID && tradeinfo->activeMarketOrderToken == pTrade->marketToken) {//活跃合约的报单
                        tradeinfo->activeIsTraded = true;
                        tradeinfo->activeRealClosePrice = pTrade->Price;
                        if (!tradeinfo->notActiveIsTraded) {
                            LOG(ERROR) << ("processArbRtnTrade:close,active instrument is traded.but not active instrument is not traded.something is error!!!!");
                        } else {
                            LOG(INFO) << ("processArbRtnTrade:close,active instrument is traded.");
                            tradeinfo->tradingDay = boost::lexical_cast<string>(pTrade->TradingDay);
                            tradeinfo->tradeTime = boost::lexical_cast<string>(pTrade->TradeTime);
                            deleteArbitrageOrders(tradeinfo);//并删除等待止盈组合列表
                            tmpit = tmplist->erase(tmpit);
                        }
                        rst = "1";
                        break;
                    } else {
                        tmpit++;
                    }
                }

            } else {
                LOG(INFO) << ("processArbRtnTrade:未查询到组合止盈单:" + com_ins_key);
            }

        }
    }
    return rst;
}
string manualOrderActioin(OrderFieldInfo *pOrder) {
    //lock_guard<recursive_mutex> locker(g_lock_ti);
    string instrumentID = pOrder->InstrumentID;
    unsigned int marketToken = pOrder->marketOrderToken;
    string rst = "0";
    unordered_map<string, vector<string>>::iterator vec_it = instr_map.find(instrumentID);
    if (vec_it == instr_map.end()) {
        cout << "can't find instrument list." << endl;
        LOG(INFO) << ("processArbRtnOrder:can't find instrumentid= " + instrumentID);
        rst = "0";
    } else {
        vector<string> tmp_instr_vec = vec_it->second;//找到对应的配对合约
        for (unsigned int i = 0; i < tmp_instr_vec.size(); i++) {
            string pd_instrument = tmp_instr_vec[i];
            string com_ins_key = getComInstrumentKey(pd_instrument, instrumentID);
            //套利单
            unordered_map<string, list<TradeInfo*>*>::iterator untrade_it = willTradeMap.find(com_ins_key);
            if (untrade_it != willTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = untrade_it->second;//以OrderSysID + BrokerOrderSeq为标志
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); ) {
                    TradeInfo* tradeinfo = *tmpit;
                    if (tradeinfo->notActiveInstrumentid == instrumentID && marketToken == tradeinfo->notActiveMarketOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        LOG(INFO) << ("manualOrderActioin:MarketOrderToken报单编号=" + boost::lexical_cast<string>(tradeinfo->notActiveMarketOrderToken) + "的订单已经被手动撤销.");
                        tradeinfo->notActiveIsTraded = true;
                        rst = "1";
                    } else if (tradeinfo->activeInstrumentid == instrumentID && marketToken == tradeinfo->activeMarketOrderToken && !tradeinfo->activeIsTraded) {//活跃合约的报单
                        LOG(INFO) << ("manualOrderActioin:MarketOrderToken报单编号=" + boost::lexical_cast<string>(tradeinfo->activeMarketOrderToken) + "的订单已经被手动撤销.");
                        tradeinfo->activeIsTraded = true;
                        rst = "1";
                    }
                    //都已经撤销或者成交，删除组合
                    if (tradeinfo->notActiveIsTraded && tradeinfo->activeIsTraded) {
                        tmpit = tmplist->erase(tmpit);
                        //willTradeMap.erase(untrade_it);
                    } else {
                        tmpit++;
                    }
                }
            }
            //止盈单
            unordered_map<string, list<TradeInfo*>*>::iterator stop_untrade_it = stopProfitWillTradeMap.find(com_ins_key);
            if (stop_untrade_it != stopProfitWillTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = untrade_it->second;//以OrderSysID + BrokerOrderSeq为标志
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); ) {
                    TradeInfo* tradeinfo = *tmpit;//以OrderSysID + BrokerOrderSeq为标志
                    if (tradeinfo->notActiveInstrumentid == instrumentID && marketToken == tradeinfo->notActiveMarketOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        LOG(INFO) << ("manualOrderActioin:MarketOrderToken报单编号=" + boost::lexical_cast<string>(tradeinfo->notActiveMarketOrderToken) + "的订单已经被手动撤销.");
                        tradeinfo->notActiveIsTraded = true;
                        rst = "1";
                    } else if (tradeinfo->activeInstrumentid == instrumentID && marketToken == tradeinfo->activeMarketOrderToken && !tradeinfo->activeIsTraded) {//活跃合约的报单
                        LOG(INFO) << ("manualOrderActioin:MarketOrderToken报单编号=" + boost::lexical_cast<string>(tradeinfo->activeMarketOrderToken) + "的订单已经被手动撤销.");
                        tradeinfo->activeIsTraded = true;
                        rst = "1";
                    }
                    //都已经撤销或者成交，删除组合
                    if (tradeinfo->notActiveIsTraded && tradeinfo->activeIsTraded) {
                        //stopProfitWillTradeMap.erase(stop_untrade_it);
                        tmpit = tmplist->erase(tmpit);
                    } else {
                        tmpit++;
                    }
                }

            }
        }
    }
    return rst;
}
string setNotActiveOrderInsertAmount(OrderFieldInfo *pOrder) {
    //lock_guard<recursive_mutex> locker(g_lock_ti);
    string instrumentID = pOrder->InstrumentID;
    string rst = "0";
    unordered_map<string, vector<string>>::iterator vec_it = instr_map.find(instrumentID);
    if (vec_it == instr_map.end()) {
        cout << "can't find instrument list." << endl;
        LOG(INFO) << ("setNotActiveOrderInsertAmount:can't find instrumentid= " + instrumentID);
        rst = "0";
    } else {
        vector<string> tmp_instr_vec = vec_it->second;//找到对应的配对合约
        for (unsigned int i = 0; i < tmp_instr_vec.size(); i++) {
            string pd_instrument = tmp_instr_vec[i];
            string com_ins_key = getComInstrumentKey(pd_instrument, instrumentID);
            //分别对应套利单和平仓单
            unordered_map<string, list<TradeInfo*>*>::iterator untrade_it = willTradeMap.find(com_ins_key);
            if (untrade_it != willTradeMap.end()) {//查找到合约对应的组合套利单列表
                list<TradeInfo*>* tmplist = untrade_it->second;//以OrderSysID + BrokerOrderSeq为标志
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); ) {
                    TradeInfo* tradeinfo = *tmpit;
                    if (tradeinfo->notActiveInstrumentid == instrumentID && pOrder->clientOrderToken == tradeinfo->notActiveClientOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        int bf = tradeinfo->notActiveInsertAmount;
                        tradeinfo->notActiveInsertAmount += 1;
                        LOG(INFO) << ("setNotActiveOrderInsertAmount:change open instrest notActiveInsertAmount from " + boost::lexical_cast<string>(bf) + " to " + boost::lexical_cast<string>(tradeinfo->notActiveInsertAmount));
                        if (tradeinfo->notActiveInsertAmount > notActiveInsertAmount) {//超过限制，删除套利组合单
                            LOG(INFO) << "不活跃合约下单次数超过notActiveInsertAmount=" + boost::lexical_cast<string>(notActiveInsertAmount) + ",删除套利组合单";
                            LOG(INFO) << "tradeinfo:" + getArbDetail(tradeinfo);
                            tmpit = tmplist->erase(tmpit);
                            //willTradeMap.erase(untrade_it);
                            rst = "1";
                            //下单情况
                            string orderinfo = getAllUntradeInfo();
                            LOG(INFO) << ("删除套利组合单之后的套利情况：size=" + boost::lexical_cast<string>(tmplist->size()) + ";" + orderinfo);
                        } else {
                            tmpit++;
                        }
                    } else {
                        tmpit++;
                    }
                }
            }
            //止盈单
            unordered_map<string, list<TradeInfo*>*>::iterator stopprofit_untrade_it = stopProfitWillTradeMap.find(com_ins_key);
            if (stopprofit_untrade_it != stopProfitWillTradeMap.end()) {//查找到合约对应的组合止盈单
                list<TradeInfo*>* tmplist = stopprofit_untrade_it->second;//以OrderSysID + BrokerOrderSeq为标志
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); ) {
                    TradeInfo* tradeinfo = *tmpit;
                    if (tradeinfo->notActiveInstrumentid == instrumentID && pOrder->clientOrderToken == tradeinfo->notActiveClientOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        int bf = tradeinfo->notActiveInsertAmount;
                        tradeinfo->notActiveInsertAmount += 1;
                        LOG(INFO) << ("setNotActiveOrderInsertAmount:change stop profit notActiveInsertAmount from " + boost::lexical_cast<string>(bf) + " to " + boost::lexical_cast<string>(tradeinfo->notActiveInsertAmount));
                        if (tradeinfo->notActiveInsertAmount > notActiveInsertAmount) {//超过限制，删除止盈组合单,并修改等待止盈单状态
                            //修改等待组合套利单状态为可触发
                            changeAlreadyTradedOrderStatus(tradeinfo, "100");
                            LOG(INFO) << "不活跃合约下单次数超过notActiveInsertAmount=" + boost::lexical_cast<string>(notActiveInsertAmount) + ",删除止盈组合单";
                            LOG(INFO) << "tradeinfo:" + getArbDetail(tradeinfo);
                            tmpit = tmplist->erase(tmpit);
                            //stopProfitWillTradeMap.erase(stopprofit_untrade_it);
                            rst = "1";
                            //下单情况
                            string orderinfo;
                            /*
                            for (unordered_map<string, TradeInfo*>::iterator untradeinfo = stopProfitWillTradeMap.begin(); untradeinfo != stopProfitWillTradeMap.end(); untradeinfo++) {
                                string key = untradeinfo->first;
                                TradeInfo* tradeInfo = untradeinfo->second;
                                string notActiveInstrumentid = tradeInfo->notActiveInstrumentid;
                                string activeInstrumentid = tradeInfo->activeInstrumentid;
                                string activeOrderInsertPrice = boost::lexical_cast<string>(tradeInfo->activeOrderInsertPrice);
                                string notActiveOrderInsertPrice = boost::lexical_cast<string>(tradeInfo->notActiveOrderInsertPrice);
                                string activeDirection = tradeInfo->activeDirection;
                                string notActiveDirection = tradeInfo->notActiveDirection;
                                string notActiveOffsetFlag = tradeInfo->notActiveOffsetFlag;
                                string activeOffsetFlag = tradeInfo->activeOffsetFlag;
                                string activeIsTraded = boost::lexical_cast<string>(tradeInfo->activeIsTraded);
                                string notActiveIsTraded = boost::lexical_cast<string>(tradeInfo->notActiveIsTraded);
                                orderinfo += "notActiveInstrumentid=" + notActiveInstrumentid + ";notActiveOrderInsertPrice=" + notActiveOrderInsertPrice + ";" + "notActiveDirection=" + notActiveDirection + ";notActiveOffsetFlag=" + notActiveOffsetFlag + ";" +
                                    "notActiveIsTraded=" + notActiveIsTraded;
                                orderinfo += "activeInstrumentid=" + activeInstrumentid + ";activeOrderInsertPrice=" + activeOrderInsertPrice + ";" + "activeDirection=" + activeDirection + ";activeOffsetFlag=" + activeOffsetFlag + ";" +
                                    "activeIsTraded=" + activeIsTraded;
                            }*/
                            LOG(INFO) << ("删除止盈组合单之后的报单情况：size=" + boost::lexical_cast<string>(stopProfitWillTradeMap.size()) + orderinfo);
                        } else {
                            tmpit++;
                        }
                    } else {
                        tmpit++;
                    }
                }

            }
        }
    }
    return rst;
}
string getDBPositionDetail(HoldPositionDetail* detail) {
    string info;
    info.append("instrumentID=" + boost::lexical_cast<string>(detail->instrumentID) + ";");
    info.append("direction=" + boost::lexical_cast<string>(detail->direction) + ";");
    info.append("hedgeFlag=" + boost::lexical_cast<string>(detail->hedgeFlag) + ";");
    info.append("volume=" + boost::lexical_cast<string>(detail->volume) + ";");
    info.append("openDate=" + boost::lexical_cast<string>(detail->openDate) + ";");
    info.append("tradeID=" + boost::lexical_cast<string>(detail->tradeID) + ";");
    return info;
}
string getArbDetail(TradeInfo* info) {
    string addData;
    addData.append("insComKey=" + info->insComKey + ";");
    addData.append("systemID=" + info->systemID + ";");
    addData.append("tradingDay=" + info->tradingDay + ";");
    addData.append("tradeTime=" + info->tradeTime + ";");
    addData.append("hopeOpenGap=" + boost::lexical_cast<string>(info->hopeOpenGap) + ";");
    addData.append("realOpenGap=" + boost::lexical_cast<string>(info->realOpenGap) + ";");
    addData.append("hopeCloseGap=" + boost::lexical_cast<string>(info->hopeCloseGap) + ";");
    addData.append("realCloseGap=" + boost::lexical_cast<string>(info->realCloseGap) + ";");
    addData.append("orderType=" + info->orderType + ";");
    //不活跃合约信息
    addData.append("happyID=" + info->happyID + ";");
    addData.append("sessionID=" + boost::lexical_cast<string>(info->sessionID) + ";");
    addData.append("frontID=" + boost::lexical_cast<string>(info->frontID) + ";");
    addData.append("notActiveClientOrderToken=" + boost::lexical_cast<string>(info->notActiveClientOrderToken) + ";");
    addData.append("notActiveMarketOrderToken=" + boost::lexical_cast<string>(info->notActiveMarketOrderToken) + ";");
    addData.append("notActiveInstrumentid=" + info->notActiveInstrumentid + ";");
    addData.append("notActiveDirection=" + info->notActiveDirection + ";");
    addData.append("notActiveOrderInsertPrice=" + boost::lexical_cast<string>(info->notActiveOrderInsertPrice) + ";");
    addData.append("notActiveHedgeFlag=" + info->notActiveHedgeFlag + ";");
    addData.append("notActiveOffsetFlag=" + info->notActiveOffsetFlag + ";");
    addData.append("notActiveOrderRef=" + info->notActiveOrderRef + ";");
    addData.append("notActiveVolume=" + boost::lexical_cast<string>(info->notActiveVolume) + ";");
    addData.append("notActiveOrderSubmitStatus=" + boost::lexical_cast<string>(info->notActiveOrderSubmitStatus) + ";");
    addData.append("notActiveOrderStatus=" + boost::lexical_cast<string>(info->notActiveOrderStatus) + ";");
    addData.append("notActiveOrderLocalID=" + boost::lexical_cast<string>(info->notActiveOrderLocalID) + ";");
    addData.append("notActiveOrderSysID=" + boost::lexical_cast<string>(info->notActiveOrderSysID) + ";");
    addData.append("notActiveTradeID=" + boost::lexical_cast<string>(info->notActiveTradeID) + ";");
    //活跃合约信息
    addData.append("activeClientOrderToken=" + boost::lexical_cast<string>(info->activeClientOrderToken) + ";");
    addData.append("activeInstrumentid=" + info->activeInstrumentid + ";");
    addData.append("activeDirection=" + info->activeDirection + ";");
    addData.append("activeOrderInsertPrice=" + boost::lexical_cast<string>(info->activeOrderInsertPrice) + ";");
    addData.append("activeHedgeFlag=" + info->activeHedgeFlag + ";");
    addData.append("activeOffsetFlag=" + info->activeOffsetFlag + ";");
    addData.append("activeOrderRef=" + info->activeOrderRef + ";");
    addData.append("activeVolume=" + boost::lexical_cast<string>(info->activeVolume) + ";");
    addData.append("activeOrderSubmitStatus=" + boost::lexical_cast<string>(info->activeOrderSubmitStatus) + ";");
    addData.append("activeOrderStatus=" + boost::lexical_cast<string>(info->activeOrderStatus) + ";");
    addData.append("activeOrderLocalID=" + boost::lexical_cast<string>(info->activeOrderLocalID) + ";");
    addData.append("activeOrderSysID=" + boost::lexical_cast<string>(info->activeOrderSysID) + ";");
    addData.append("activeTradeID=" + boost::lexical_cast<string>(info->activeTradeID) + ";");
    /*
    string orderinfo;
    string notActiveInstrumentid = tradeInfo->notActiveInstrumentid;
    string activeInstrumentid = tradeInfo->activeInstrumentid;
    string activeOrderInsertPrice = boost::lexical_cast<string>(tradeInfo->activeOrderInsertPrice);
    string notActiveOrderInsertPrice = boost::lexical_cast<string>(tradeInfo->notActiveOrderInsertPrice);
    string activeDirection = tradeInfo->activeDirection;
    string notActiveDirection = tradeInfo->notActiveDirection;
    string notActiveOffsetFlag = tradeInfo->notActiveOffsetFlag;
    string activeOffsetFlag = tradeInfo->activeOffsetFlag;
    string activeIsTraded = boost::lexical_cast<string>(tradeInfo->activeIsTraded);
    string notActiveIsTraded = boost::lexical_cast<string>(tradeInfo->notActiveIsTraded);
    string notActiveInsertAmount = boost::lexical_cast<string>(tradeInfo->notActiveInsertAmount);
    string activeOrderActionStatus = boost::lexical_cast<string>(tradeInfo->activeOrderActionStatus);
    string notActiveOrderRef = tradeInfo->notActiveOrderRef;
    string activeOrderRef = tradeInfo->activeOrderRef;
    orderinfo += "notActiveInstrumentid=" + notActiveInstrumentid + ";notActiveOrderInsertPrice=" + notActiveOrderInsertPrice + ";" + "notActiveDirection=" + notActiveDirection + ";notActiveOffsetFlag=" + notActiveOffsetFlag + ";" +
        "notActiveIsTraded=" + notActiveIsTraded + ";notActiveInsertAmount=" + notActiveInsertAmount + ";notActiveOrderRef=" + notActiveOrderRef + ";";
    orderinfo += "activeInstrumentid=" + activeInstrumentid + ";activeOrderInsertPrice=" + activeOrderInsertPrice + ";" + "activeDirection=" + activeDirection + ";activeOffsetFlag=" + activeOffsetFlag + ";" +
        "activeIsTraded=" + activeIsTraded + ";activeOrderRef=" + activeOrderRef + ",activeOrderActionStatus=" + activeOrderActionStatus;
    return orderinfo;
    */
    return addData;
}
string processRspReqInstrument(EES_SymbolField *pInstrument) {
    InstrumentInfo* info = new InstrumentInfo();
    info->ExchangeID = pInstrument->m_ExchangeID;
    info->PriceTick = pInstrument->m_PriceTick;
    info->instrumentID = string(pInstrument->m_symbol);
    info->VolumeMultiple = pInstrument->m_VolumeMultiple;
    instruments[string(pInstrument->m_symbol)] = info;
    LOG(INFO) << "查询到合约信息:" + info->instrumentID + ";priceTick=" + boost::lexical_cast<string>(info->PriceTick) + ";volumeMultiple=" + boost::lexical_cast<string>(info->VolumeMultiple);
    return "";
}
void initPriceGap(string instrumentID){
    cerr << "--->>> " << "initPriceGap"  << endl;
    //int gaps = 2*(max_price - min_price);
    unordered_map<string, InstrumentInfo*>::iterator it = instruments.find(instrumentID);
    double min_price = 0;
    double max_price = 0;
    double tick = 0;
    if(it != instruments.end()){
        InstrumentInfo* info = it->second;
        min_price = info->LowerLimitPrice;
        max_price = info->UpperLimitPrice;
        tick = info->PriceTick;
    }
    double tmp_price = min_price;
    while(true){
        if(tmp_price <= max_price){
            //double down_gap = (tmp_price - min_price)*2;
            //double up_gap = down_gap + 2*tick;

            double down_gap = (tmp_price - min_price);
            double up_gap = down_gap + tick - 1;
            //double up_gap = down_gap + tick;

            vector<double> gap_list;
            gap_list.push_back(down_gap);
            gap_list.push_back(up_gap);
//            gap_list[0] = down_gap;
//            gap_list[1] = up_gap;
            map_price_gap[tmp_price] = gap_list;
            char c_tmp[100];
            sprintf(c_tmp,"price=%f down=%f up=%f",tmp_price,gap_list[0],gap_list[1]);
            //cout<<c_tmp<<endl;
            LOG(INFO) << c_tmp;
        }else{
            break;
        }
        tmp_price += tick;
    }
}
string getAllUntradeInfo() {
    string orderinfo;
    for (unordered_map<string, list<TradeInfo*>*>::iterator untradeinfo = willTradeMap.begin(); untradeinfo != willTradeMap.end(); untradeinfo++) {
        string key = untradeinfo->first;
        list<TradeInfo*>* tilist = untradeinfo->second;
        for (list<TradeInfo*>::iterator it = tilist->begin(); it != tilist->end(); it++) {
            TradeInfo* tradeInfo = *it;
            string notActiveInstrumentid = tradeInfo->notActiveInstrumentid;
            string activeInstrumentid = tradeInfo->activeInstrumentid;
            string activeOrderInsertPrice = boost::lexical_cast<string>(tradeInfo->activeOrderInsertPrice);
            string notActiveOrderInsertPrice = boost::lexical_cast<string>(tradeInfo->notActiveOrderInsertPrice);
            string activeDirection = tradeInfo->activeDirection;
            string notActiveDirection = tradeInfo->notActiveDirection;
            string notActiveOffsetFlag = tradeInfo->notActiveOffsetFlag;
            string activeOffsetFlag = tradeInfo->activeOffsetFlag;
            string activeIsTraded = boost::lexical_cast<string>(tradeInfo->activeIsTraded);
            string notActiveIsTraded = boost::lexical_cast<string>(tradeInfo->notActiveIsTraded);
            orderinfo += "notActiveInstrumentid=" + notActiveInstrumentid + ";notActiveOrderInsertPrice=" + notActiveOrderInsertPrice + ";" + "notActiveDirection=" + notActiveDirection + ";notActiveOffsetFlag=" + notActiveOffsetFlag + ";" +
                "notActiveIsTraded=" + notActiveIsTraded + ";";
            orderinfo += "activeInstrumentid=" + activeInstrumentid + ";activeOrderInsertPrice=" + activeOrderInsertPrice + ";" + "activeDirection=" + activeDirection + ";activeOffsetFlag=" + activeOffsetFlag + ";" +
                "activeIsTraded=" + activeIsTraded + ";";
        }
    }
    return orderinfo;
}
string getAllAlreadyTradedInfo() {
    string orderinfo;
    for (unordered_map<string, list<TradeInfo*>*>::iterator listIt = alreadyTradeMapAS.begin(); listIt != alreadyTradeMapAS.end();listIt++) {
        string insComKey = listIt->first;
        list<TradeInfo*>* tradeList = listIt->second;
        for (list<TradeInfo*>::iterator tradeIt = tradeList->begin(); tradeIt != tradeList->end(); tradeIt++) {
            TradeInfo* tdInfo = *tradeIt;
            orderinfo += getArbDetail(tdInfo);
        }
    }
    return orderinfo;
}
string TradeInfo::toString() {
    string orderinfo;
    string notActiveInstrumentid = notActiveInstrumentid;
    string activeInstrumentid = activeInstrumentid;
    string activeOrderInsertPrice = boost::lexical_cast<string>(activeOrderInsertPrice);
    string notActiveOrderInsertPrice = boost::lexical_cast<string>(notActiveOrderInsertPrice);
    string activeDirection = activeDirection;
    string notActiveDirection = notActiveDirection;
    string notActiveOffsetFlag = notActiveOffsetFlag;
    string activeOffsetFlag =activeOffsetFlag;
    string activeIsTraded = boost::lexical_cast<string>(activeIsTraded);
    string notActiveIsTraded = boost::lexical_cast<string>(notActiveIsTraded);
    orderinfo += "notActiveInstrumentid=" + notActiveInstrumentid + ";notActiveOrderInsertPrice=" + notActiveOrderInsertPrice + ";" + "notActiveDirection=" + notActiveDirection + ";notActiveOffsetFlag=" + notActiveOffsetFlag + ";" +
        "notActiveIsTraded=" + notActiveIsTraded + ";";
    orderinfo += "activeInstrumentid=" + activeInstrumentid + ";activeOrderInsertPrice=" + activeOrderInsertPrice + ";" + "activeDirection=" + activeDirection + ";activeOffsetFlag=" + activeOffsetFlag + ";" +
        "activeIsTraded=" + activeIsTraded + ";";
    orderinfo += "insComKey=" + insComKey + ";happyID=" + happyID;
    return orderinfo;
}
string getAllStopProfitInfo() {
    string orderinfo;
    for (unordered_map<string, list<TradeInfo*>*>::iterator untradeinfo = stopProfitWillTradeMap.begin(); untradeinfo != stopProfitWillTradeMap.end(); untradeinfo++) {
        string key = untradeinfo->first;
        list<TradeInfo*>* tmplist = untradeinfo->second;
        for (list<TradeInfo*>::iterator it = tmplist->begin(); it != tmplist->end(); it++) {
            TradeInfo* tradeInfo = *it;
            string notActiveInstrumentid = tradeInfo->notActiveInstrumentid;
            string activeInstrumentid = tradeInfo->activeInstrumentid;
            string activeOrderInsertPrice = boost::lexical_cast<string>(tradeInfo->activeOrderInsertPrice);
            string notActiveOrderInsertPrice = boost::lexical_cast<string>(tradeInfo->notActiveOrderInsertPrice);
            string activeDirection = tradeInfo->activeDirection;
            string notActiveDirection = tradeInfo->notActiveDirection;
            string notActiveOffsetFlag = tradeInfo->notActiveOffsetFlag;
            string activeOffsetFlag = tradeInfo->activeOffsetFlag;
            string activeIsTraded = boost::lexical_cast<string>(tradeInfo->activeIsTraded);
            string notActiveIsTraded = boost::lexical_cast<string>(tradeInfo->notActiveIsTraded);
            orderinfo += "notActiveInstrumentid=" + notActiveInstrumentid + ";notActiveOrderInsertPrice=" + notActiveOrderInsertPrice + ";" + "notActiveDirection=" + notActiveDirection + ";notActiveOffsetFlag=" + notActiveOffsetFlag + ";" +
                "notActiveIsTraded=" + notActiveIsTraded + ";";
            orderinfo += "activeInstrumentid=" + activeInstrumentid + ";activeOrderInsertPrice=" + activeOrderInsertPrice + ";" + "activeDirection=" + activeDirection + ";activeOffsetFlag=" + activeOffsetFlag + ";" +
                "activeIsTraded=" + activeIsTraded + ";";
        }

    }
    return orderinfo;
}
TradeInfo*  getOriginalTradeInfo(OrderFieldInfo *pOrder) {
    string instrumentid = pOrder->InstrumentID;
    unsigned int orderClientToken = pOrder->clientOrderToken;
    TradeInfo* tradeinfo;
    unordered_map<string, vector<string>>::iterator vec_it = instr_map.find(instrumentid);
    if (vec_it == instr_map.end()) {
        cout << "can't find instrument list" << endl;
        LOG(INFO) << ("getOriginalTradeInfo:can't find instrumentid= " + instrumentid);
        return NULL;
    } else {
        vector<string> tmp_instr_vec = vec_it->second;//找到对应的配对合约
        for (unsigned int i = 0; i < tmp_instr_vec.size(); i++) {
            string pd_instrument = tmp_instr_vec[i];
            string com_ins_key = getComInstrumentKey(pd_instrument, instrumentid);
            /*从开仓序列和止盈序列中，分别找对应的报单列表*/
            //一、开仓单中查找
            unordered_map<string, list<TradeInfo*>*>::iterator untrade_it = willTradeMap.find(com_ins_key);
            if (untrade_it != willTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = untrade_it->second;
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); tmpit++) {
                    tradeinfo = *tmpit;
                    if (tradeinfo->notActiveInstrumentid == instrumentid && orderClientToken == tradeinfo->notActiveClientOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        return tradeinfo;
                    } else if (tradeinfo->activeInstrumentid == instrumentid && tradeinfo->activeClientOrderToken == orderClientToken && !tradeinfo->activeIsTraded) {//活跃合约的报单
                        return tradeinfo;
                    }
                }
            }
            //二、止盈单中查找
            unordered_map<string, list<TradeInfo*>*>::iterator stopprofit_untrade_it = stopProfitWillTradeMap.find(com_ins_key);
            if (stopprofit_untrade_it != stopProfitWillTradeMap.end()) {//查找到合约对应的组合套利单
                list<TradeInfo*>* tmplist = stopprofit_untrade_it->second;
                for (list<TradeInfo*>::iterator tmpit = tmplist->begin(); tmpit != tmplist->end(); tmpit++) {
                    tradeinfo = *tmpit;
                    if (tradeinfo->notActiveInstrumentid == instrumentid && orderClientToken == tradeinfo->notActiveClientOrderToken && !tradeinfo->notActiveIsTraded) {//不活跃合约的报单
                        return tradeinfo;
                    } else if (tradeinfo->activeInstrumentid == instrumentid && tradeinfo->activeClientOrderToken == orderClientToken && !tradeinfo->activeIsTraded) {//活跃合约的报单
                        return tradeinfo;
                    }
                }

            }
        }
    }
    LOG(ERROR) << "ERROR:根据撤单回报，查找不到合约组合。";
    return tradeinfo;
}
double getMultipler(string instrumentID) {
    //合约乘数
    unordered_map<string, InstrumentInfo*>::iterator ins_it = instruments.find(instrumentID);
    if (ins_it == instruments.end()) {
        LOG(ERROR) << "无法查找到合约信息.instrumentID=" + instrumentID;
        return 0;
    }
    InstrumentInfo* insinfo = ins_it->second;
    double multiplyFactor = insinfo->VolumeMultiple;
    return multiplyFactor;
}
bool isOverTradeTwo(PriceGapMarketData* pgmd, string orderType) {//不判断
    int openCount = 0;
    string insComKey = pgmd->insComKey;
    double willGap = 0;
    unordered_map<string, list<TradeInfo*>*>::iterator atmIt;
    if (orderType == "as") {
        willGap = pgmd->asOpenGap;
        atmIt = alreadyTradeMapAS.find(insComKey);
        if (atmIt == alreadyTradeMapAS.end()) {
            LOG(ERROR) << "isOverTradeTwo正套列表不存在,可以开仓";
            return true;
        }
    }else if(orderType == "ds"){
        willGap = pgmd->dsOpenGap;
        atmIt = alreadyTradeMapDS.find(insComKey);
        if (atmIt == alreadyTradeMapDS.end()) {
            LOG(ERROR) << "isOverTradeTwo反套列表不存在,可以开仓";
            return true;
        }
    }
    list<TradeInfo*>* tradeList = atmIt->second;
    for (list<TradeInfo*>::iterator tlIt = tradeList->begin(); tlIt != tradeList->end(); tlIt++) {
        TradeInfo* tInfo = *tlIt;
        if (tInfo->realOpenGap == willGap) {
            openCount += tInfo->activeVolume;
        }
    }
    if (openCount >= gapCanBeOpenNums) {//每个gap最多持仓限制
        if(isLogout){
            LOG(INFO) << "在gap=" + boost::lexical_cast<string>(willGap) + " 上面已经开仓的数量为" + boost::lexical_cast<string>(openCount) + ",超过阈值=" + boost::lexical_cast<string>(gapCanBeOpenNums);
        }
        return false;
    } else {
        if(isLogout){
            LOG(INFO) << "在gap=" + boost::lexical_cast<string>(willGap) + " 上面已经开仓的数量为" + boost::lexical_cast<string>(openCount) + ",不超过阈值=" + boost::lexical_cast<string>(gapCanBeOpenNums);
        }
        return true;
    }
}
bool isHoldPositionOverGap(PriceGap* priceGap, string orderType) {
    if (orderType == "as") {
        int pstGap = priceGap->arbComVolumeAS - priceGap->arbComVolumeDS;
        if (pstGap >= 10) {
            if(isLogout){
                LOG(INFO) << "正套持仓差=" + boost::lexical_cast<string>(pstGap) + " 已经达到阈值=" + boost::lexical_cast<string>(10) + ",不能继续开仓";
            }

            return false;
        } else {
            return true;
        }
    }else if(orderType == "ds"){
        int pstGap = priceGap->arbComVolumeDS - priceGap->arbComVolumeAS;
        if (pstGap >= 10) {
            if(isLogout){
                LOG(INFO) << "反套持仓差=" + boost::lexical_cast<string>(pstGap) + " 已经达到阈值=" + boost::lexical_cast<string>(10) + ",不能继续开仓";
            }

            return false;
        } else {
            return true;
        }
    }
}
bool isOverTrade(string pd_instr_key,string orderType ) {
    unordered_map<string, list<TradeInfo*>*>::iterator untradeinfoList = willTradeMap.find(pd_instr_key);
    PriceGap* pg = getPriceGap(pd_instr_key);
    int arbComVolume = 0;//组合单手数上限
    int arbComVolMetric = 0;//
    if(orderType == "as") {
        arbComVolume = pg->arbComVolumeAS;//组合单手数上限
        arbComVolMetric = pg->arbComVolMetricAS;//
    } else if (orderType == "ds") {
        arbComVolume = pg->arbComVolumeDS;//组合单手数上限
        arbComVolMetric = pg->arbComVolMetricDS;//
    }
    //如果已经存在套利合约单，需要进一步判断价格是否偏离
    if (untradeinfoList != willTradeMap.end()) {//说明存在套利合约的下单信息
        list<TradeInfo*>* untradeinfolist = untradeinfoList->second;
        int untradeCount = 0;//未成交笔数
        int untradeVols = 0;//未成交的手数

        string tmpstr = "";
        for (list<TradeInfo*>::iterator it = untradeinfolist->begin(); it != untradeinfolist->end(); it++) {
            TradeInfo* tmpTradeInfo = *it;
            if (orderType == tmpTradeInfo->orderType) {//报单类型相同
                untradeVols += tmpTradeInfo->notActiveVolume;//
                untradeCount += 1;
                tmpstr += getArbDetail(tmpTradeInfo) + ";";
            }
        }
        //第一个是笔数，第二个是手数
        if (untradeCount >= maxUntradeNums) {//超过最大未成交组合的数量，则不能继续下单，防止由于未收到成交回报等原因导致的报单报入超限
            if(isLogout){
                LOG(ERROR) << "行情触发套利条件，但是套利单未成交数量:" + boost::lexical_cast<string>(untradeCount) + " 超过最大未成交组合的数量:" + boost::lexical_cast<string>(maxUntradeNums) + "，则不能继续下单，防止由于未收到成交回报等原因导致的报单报入超限" + ",等待成交套利单=" + tmpstr;
            }
            return false;
        } else if ((arbComVolume + untradeVols) >= arbComVolMetric) {
            if(isLogout){
                LOG(ERROR) << ("行情触发套利条件,但是已经超过开仓限制,不能下单。当前持仓手数=" + boost::lexical_cast<string>(arbComVolume) + ",未成交手数=" + boost::lexical_cast<string>(untradeVols) + ",阈值=" + boost::lexical_cast<string>(arbComVolMetric));
            }
            return false;
        } else {
            if(isLogout){
                LOG(INFO) << ("行情触发套利条件,未超过开仓限制,可以下单。当前持仓=" + boost::lexical_cast<string>(arbComVolume) + ",未成交=" + boost::lexical_cast<string>(untradeVols) + ",阈值=" + boost::lexical_cast<string>(arbComVolMetric));
            }
            return true;
        }
    } else if(arbComVolume >= arbComVolMetric){
        if(isLogout){
            LOG(ERROR) << ("行情触发套利条件,但是已经超过开仓限制,不能下单。当前持仓=" + boost::lexical_cast<string>(arbComVolume) + ",未成交=0,阈值=" + boost::lexical_cast<string>(arbComVolMetric));
        }
        return false;
    } else {
        return true;
    }
}
bool isOrderInsertPriceBiased(string pd_instr_key) {
    unordered_map<string, list<TradeInfo*>*>::iterator untradeinfoList = willTradeMap.find(pd_instr_key);
    //如果已经存在套利合约单，需要进一步判断价格是否偏离
    if (untradeinfoList != willTradeMap.end()) {//说明存在套利合约的下单信息
        list<TradeInfo*>* untradeinfolist = untradeinfoList->second;
        //最后一个套利组合是最新的套利组合
        int tmpSize = untradeinfolist->size();
        if (tmpSize > 0) {//说明有未成交组合,取最后一个判断
            if (tmpSize >= maxUntradeNums) {//超过最大未成交组合的数量，则不能继续下单，防止由于未收到成交回报等原因导致的报单报入超限
                LOG(INFO) << "行情触发套利条件，但是套利单未成交数量:" + boost::lexical_cast<string>(tmpSize) + " 超过最大未成交组合的数量:" + boost::lexical_cast<string>(maxUntradeNums) + "，则不能继续下单，防止由于未收到成交回报等原因导致的报单报入超限";
                return false;
            }/*
            TradeInfo* tmpinfo = untradeinfolist->back();
            if (tmpinfo->followTimes >= maxFollowTimes) {//追单超过几次，才允许下单。追单就表示价格已经偏离，偏离了就重新下单。
                                                         //允许下单
                LOG(INFO) << ("行情触发套利条件,但是已经开始下单,且追单次数大于：" + boost::lexical_cast<string>(maxFollowTimes) + ",价格已经偏离，可以继续下套利单.最近一笔已经存在的套利单:" + getArbDetail(tmpinfo));
                return true;
            } else {
                //不允许下单
                LOG(INFO) << ("行情触发套利条件,但是已经开始下单,且追单次数不大于：" + boost::lexical_cast<string>(maxFollowTimes) + ",价格没有偏离，不需要重复下单.最近一笔已经存在的套利单:" + getArbDetail(tmpinfo));
                return false;
            }*/
        } else {
            LOG(INFO) << "untradeinfolist->size() == 0;当前无套利单，可以直接下单";
            return true;
        }
    } else {
        LOG(INFO) << "当前无套利单，可以直接下单";
        return true;
    }
}
double getPriceTick(string instrumentID) {
    //合约priceTick
    unordered_map<string, InstrumentInfo*>::iterator ins_it = instruments.find(instrumentID);
    if (ins_it == instruments.end()) {
        LOG(ERROR) << "无法查找到合约信息.instrumentID=" + instrumentID;
        return 0;
    }
    InstrumentInfo* insinfo = ins_it->second;
    double priceTick = insinfo->PriceTick;
    return priceTick;
}
int processtrade(TradeFieldInfo *pTrade)
{
    LOG(INFO)<<"===>processtrade";
    if (start_process == 0) {
        return 0;
    }
    ///买卖方向
    string	direction = pTrade->Direction;
    //char Direction[] = { direction,'\0' };
    //sprintf(Direction,"%s",direction);
    ///开平标志
    string	offsetFlag = pTrade->OffsetFlag;
    //char OffsetFlag[] = { offsetFlag,'\0' };
    ///合约代码
    string instrumentID = pTrade->InstrumentID;
    //string str_inst = string(InstrumentID);
    int volume = pTrade->Volume;
    //买卖方向
    //string str_dir = string(Direction);
    //开平方向
    //string str_offset = string(OffsetFlag);
    //成交价格
    double tradePrice = pTrade->Price;
    //合约乘数
    unordered_map<string, InstrumentInfo*>::iterator ins_it = instruments.find(instrumentID);
    if (ins_it == instruments.end()) {
        LOG(ERROR) << "处理成交信息时,无法查找到合约信息.instrumentID=" + instrumentID;
        return 0;
    }
    InstrumentInfo* insinfo = ins_it->second;
    double multiplyFactor = insinfo->VolumeMultiple;
    HoldPositionInfo* hpstinfo;
    //unordered_map<string, unordered_map<string, int>>::iterator map_iterator = positionmap.find(str_inst);
    unordered_map<string, HoldPositionInfo*>::iterator map_iterator = positionmap.find(instrumentID);
    //新开仓
    if (map_iterator == positionmap.end()) {
        //unordered_map<string, int> tmpmap;
        HoldPositionInfo* tmpmap = new HoldPositionInfo();
        hpstinfo = tmpmap;
        if (direction == "0") {//买
                             //多头
            tmpmap->longTdPosition = pTrade->Volume;
            tmpmap->longYdPosition = 0;
            tmpmap->longTotalPosition = pTrade->Volume;
            tmpmap->longAvaClosePosition = pTrade->Volume;
            tmpmap->longHoldAvgPrice = tradePrice;
            tmpmap->longAmount = tradePrice*pTrade->Volume*multiplyFactor;
            //空头
            tmpmap->shortTdPosition = 0;
            tmpmap->shortYdPosition = 0;
            tmpmap->shortTotalPosition = 0;
            tmpmap->shortHoldAvgPrice = 0;
            tmpmap->shortAmount = 0;
        } else if (direction == "1") {//卖
                                    //空头
            tmpmap->shortTdPosition = pTrade->Volume;
            tmpmap->shortYdPosition = 0;
            tmpmap->shortTotalPosition = pTrade->Volume;
            tmpmap->shortAvaClosePosition = pTrade->Volume;
            tmpmap->shortHoldAvgPrice = tradePrice;
            tmpmap->shortAmount = tradePrice*pTrade->Volume*multiplyFactor;
            //多头
            tmpmap->longTdPosition = 0;
            tmpmap->longYdPosition = 0;
            tmpmap->longTotalPosition = 0;
            tmpmap->longHoldAvgPrice = 0;
            tmpmap->longAmount = 0;
        }
        positionmap[instrumentID] = tmpmap;
    } else {
        ///平仓
        //        #define USTP_FTDC_OF_Close '1'
        //        ///强平
        //        #define USTP_FTDC_OF_ForceClose '2'
        //        ///平今
        //        #define USTP_FTDC_OF_CloseToday '3'
        //        ///平昨
        //        #define USTP_FTDC_OF_CloseYesterday '4'
        HoldPositionInfo* tmpinfo = map_iterator->second;
        hpstinfo = tmpinfo;
        if (direction == "0") {//买
            double tmp_shortHoldAvgPrice = tmpinfo->shortHoldAvgPrice;//空头持仓均价
            int tmp_totalPst = tmpinfo->shortTotalPosition;//原空头持仓量
            double tmp_totalAmount = tmpinfo->shortAmount;//原空头交易金额
            if (offsetFlag == "0") {//买开仓,多头增加
                tmpinfo->longTdPosition = tmpinfo->longTdPosition + pTrade->Volume;
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                double tmp_longHoldAvgPrice = tmpinfo->longHoldAvgPrice;//多头持仓均价
                int tmp_totalPst = tmpinfo->longTotalPosition;//原多头持仓量
                double tmp_totalAmount = tmpinfo->longAmount;//原多头交易金额
                realLongPstLimit = tmp_tdpst + tmp_ydpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
                tmpinfo->longAvaClosePosition = tmpinfo->longAvaClosePosition + volume;
                tmpinfo->longHoldAvgPrice = (tmp_longHoldAvgPrice*tmp_totalPst + tradePrice*pTrade->Volume) / (realLongPstLimit);//当前多头持仓均价
                tmpinfo->longAmount = tmp_totalAmount + tradePrice*pTrade->Volume*multiplyFactor;//当前多头交易金额
            } else if (offsetFlag == "1") {//买平仓,空头减少
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                //int tmp_num = map_iterator->second["shortTotalPosition"];
                if (tmp_tdpst > 0) {
                    if (tmp_tdpst <= pTrade->Volume) {
                        tmp_ydpst = tmp_ydpst - (pTrade->Volume - tmp_tdpst);
                        tmp_tdpst = 0;
                    } else {
                        tmp_tdpst = tmp_tdpst - pTrade->Volume;
                    }
                } else if (tmp_tdpst == 0) {
                    tmp_ydpst = tmp_ydpst - pTrade->Volume;
                } else {
                    cout << "tdposition is error!!!" << endl;
                    LOG(ERROR) << "tdposition is error!!!";
                }
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->shortTdPosition = tmp_tdpst;
                tmpinfo->shortYdPosition = tmp_ydpst;
                tmpinfo->shortTotalPosition = realShortPstLimit;
            } else if (offsetFlag == "3") {//平今
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                tmp_tdpst = tmp_tdpst - pTrade->Volume;
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->shortTdPosition = tmp_tdpst;
                tmpinfo->shortTotalPosition = realShortPstLimit;
            } else if (offsetFlag == "4") {//平昨
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                if (tmp_ydpst == 0) {
                    char c_err[100];
                    sprintf(c_err, "shortYdPosition is zero!!!,please check this rtn trade.");
                    cout << c_err << endl;
                    LOG(ERROR) << c_err;
                }
                tmp_ydpst = tmp_ydpst - pTrade->Volume;
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->shortYdPosition = tmp_ydpst;
                tmpinfo->shortTotalPosition = realShortPstLimit;
            }
            //买平仓处理空头平均价格;买开仓不需要单独处理
            if (offsetFlag != "0") {
                tmpinfo->shortHoldAvgPrice = (tmp_shortHoldAvgPrice*tmp_totalPst - tradePrice*pTrade->Volume) / (realShortPstLimit);//当前空头持仓均价
                tmpinfo->shortAmount = tmp_totalAmount - tradePrice*pTrade->Volume*multiplyFactor;//当前空头交易金额
                tmpinfo->shortAvaClosePosition = tmpinfo->shortAvaClosePosition - volume;//当前空头可平量
            }
        } else if (direction == "1") {//卖
            double tmp_longHoldAvgPrice = tmpinfo->longHoldAvgPrice;//原多头持仓均价
            int tmp_totalPst = tmpinfo->longTotalPosition;//原多头持仓量
            double tmp_totalAmount = tmpinfo->longAmount;//原多头交易金额
            if (offsetFlag == "0") {//卖开仓,空头增加
                tmpinfo->shortTdPosition = tmpinfo->shortTdPosition + pTrade->Volume;
                double tmp_shortHoldAvgPrice = tmpinfo->shortHoldAvgPrice;//原空头持仓均价
                int tmp_totalPst = tmpinfo->shortTotalPosition;//原空头持仓量
                double tmp_totalAmount = tmpinfo->shortAmount;//原空头交易金额
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                realShortPstLimit = tmp_tdpst + tmp_ydpst;
                tmpinfo->shortTotalPosition = realShortPstLimit;
                tmpinfo->shortAvaClosePosition = tmpinfo->shortAvaClosePosition + volume;
                tmpinfo->shortHoldAvgPrice = (tmp_shortHoldAvgPrice*tmp_totalPst + tradePrice*pTrade->Volume) / (realShortPstLimit);//当前空头持仓均价
                tmpinfo->shortAmount = tmp_totalAmount + tradePrice*pTrade->Volume*multiplyFactor;//当前空头交易金额
            } else if (offsetFlag == "1") {//卖平仓,多头减少
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                if (tmp_tdpst > 0) {
                    if (tmp_tdpst <= pTrade->Volume) {
                        tmp_ydpst = tmp_ydpst - (pTrade->Volume - tmp_tdpst);
                        tmp_tdpst = 0;
                    } else {
                        tmp_tdpst = tmp_tdpst - pTrade->Volume;
                    }
                } else if (tmp_tdpst == 0) {
                    tmp_ydpst = tmp_ydpst - pTrade->Volume;
                } else {
                    cout << "tdposition is error!!!" << endl;
                    LOG(ERROR) << "tdposition is error!!!";
                }
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->longTdPosition = tmp_tdpst;
                tmpinfo->longYdPosition = tmp_ydpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
            } else if (offsetFlag == "3") {//平今
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                tmp_tdpst = tmp_tdpst - pTrade->Volume;
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->longTdPosition = tmp_tdpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
            } else if (offsetFlag == "4") {//平昨
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                if (tmp_ydpst == 0) {
                    char c_err[100];
                    sprintf(c_err, "longYdPosition is zero!!!,please check this rtn trade.");
                    cout << c_err << endl;
                    LOG(ERROR) << c_err;
                }
                tmp_ydpst = tmp_ydpst - pTrade->Volume;
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->longYdPosition = tmp_ydpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
            }
            //卖平仓处理多头平均价格;卖开仓不需要单独处理
            if (offsetFlag != "0") {
                tmpinfo->longHoldAvgPrice = (tmp_longHoldAvgPrice*tmp_totalPst - tradePrice*pTrade->Volume) / (realLongPstLimit);//当前多头持仓均价
                tmpinfo->longAmount = tmp_totalAmount - tradePrice*pTrade->Volume*multiplyFactor;//当前多头交易金额
                tmpinfo->longAvaClosePosition = tmpinfo->longAvaClosePosition - volume;
            }
        }
    }
    //processAverageGapGprice(pTrade);
    tradeParaProcessTwo();
    string tmpmsg;
    for (unordered_map<string, HoldPositionInfo*>::iterator it = positionmap.begin(); it != positionmap.end(); it++) {
        HoldPositionInfo* tmpinfo = it->second;
        tmpmsg.append(it->first).append("持仓情况:");
        char char_tmp_pst[10] = { '\0' };
        char char_longyd_pst[10] = { '\0' };
        char char_longtd_pst[10] = { '\0' };
        sprintf(char_tmp_pst, "%d", tmpinfo->longTotalPosition);
        sprintf(char_longyd_pst, "%d", tmpinfo->longYdPosition);
        sprintf(char_longtd_pst, "%d", tmpinfo->longTdPosition);
        char str_avgBuy[25], str_avgSell[25];
        int sig = 2;
        gcvt(tmpinfo->longHoldAvgPrice, sig, str_avgBuy);
        tmpmsg.append("多头数量=");
        tmpmsg.append(char_tmp_pst);
        tmpmsg.append(";今仓数量=");
        tmpmsg.append(char_longtd_pst);
        tmpmsg.append(";昨仓数量=");
        tmpmsg.append(char_longyd_pst);
        tmpmsg.append(";可平数量=" + boost::lexical_cast<string>(tmpinfo->longAvaClosePosition));
        tmpmsg.append(";持仓均价=");
        tmpmsg.append(boost::lexical_cast<string>(tmpinfo->longHoldAvgPrice));
        tmpmsg.append(";持仓金额=" + boost::lexical_cast<string>(tmpinfo->longAmount));
        char char_tmp_pst2[10] = { '\0' };
        char char_shortyd_pst[10] = { '\0' };
        char char_shorttd_pst[10] = { '\0' };
        gcvt(tmpinfo->shortHoldAvgPrice, sig, str_avgSell);
        sprintf(char_tmp_pst2, "%d", tmpinfo->shortTotalPosition);
        sprintf(char_shortyd_pst, "%d", tmpinfo->shortYdPosition);
        sprintf(char_shorttd_pst, "%d", tmpinfo->shortTdPosition);
        tmpmsg.append("空头数量=");
        tmpmsg.append(char_tmp_pst2);
        tmpmsg.append(";今仓数量=");
        tmpmsg.append(char_shorttd_pst);
        tmpmsg.append(";昨仓数量=");
        tmpmsg.append(char_shortyd_pst);
        tmpmsg.append(";可平数量=" + boost::lexical_cast<string>(tmpinfo->shortAvaClosePosition));
        tmpmsg.append(";持仓均价=");
        tmpmsg.append(boost::lexical_cast<string>(tmpinfo->shortHoldAvgPrice));
        tmpmsg.append(";持仓金额=" + boost::lexical_cast<string>(tmpinfo->shortAmount) + ";#");
    }
    cout << tmpmsg << endl;
    LOG(INFO) << tmpmsg;
    return 0;
}
//normal market maker trade
int processtradeOfNormal(TradeFieldInfo *pTrade)
{
    LOG(INFO)<<"===>processtradeOfNormal";
    if (start_process == 0) {
        return 0;
    }
    ///买卖方向
    string	direction = pTrade->Direction;
    //char Direction[] = { direction,'\0' };
    //sprintf(Direction,"%s",direction);
    ///开平标志
    string	offsetFlag = pTrade->OffsetFlag;
    //char OffsetFlag[] = { offsetFlag,'\0' };
    ///合约代码
    string instrumentID = pTrade->InstrumentID;
    //string str_inst = string(InstrumentID);
    int volume = pTrade->Volume;
    //买卖方向
    //string str_dir = string(Direction);
    //开平方向
    //string str_offset = string(OffsetFlag);
    //成交价格
    double tradePrice = pTrade->Price;
    //合约乘数
    unordered_map<string, InstrumentInfo*>::iterator ins_it = instruments.find(instrumentID);
    if (ins_it == instruments.end()) {
        LOG(ERROR) << "处理成交信息时,无法查找到合约信息.instrumentID=" + instrumentID;
        return 0;
    }
    InstrumentInfo* insinfo = ins_it->second;
    double multiplyFactor = insinfo->VolumeMultiple;
    HoldPositionInfo* hpstinfo;
    unordered_map<string, HoldPositionInfo*>::iterator map_iterator = normalMMPositionmap.find(instrumentID);
    //新开仓
    if (map_iterator == normalMMPositionmap.end()) {
        HoldPositionInfo* tmpmap = new HoldPositionInfo();
        hpstinfo = tmpmap;
        if (direction == "0") {//买
                             //多头
            tmpmap->longTdPosition = pTrade->Volume;
            tmpmap->longYdPosition = 0;
            tmpmap->longTotalPosition = pTrade->Volume;
            tmpmap->longAvaClosePosition = pTrade->Volume;
            tmpmap->longHoldAvgPrice = tradePrice;
            tmpmap->longAmount = tradePrice*pTrade->Volume*multiplyFactor;
            //空头
            tmpmap->shortTdPosition = 0;
            tmpmap->shortYdPosition = 0;
            tmpmap->shortTotalPosition = 0;
            tmpmap->shortHoldAvgPrice = 0;
            tmpmap->shortAmount = 0;
        } else if (direction == "1") {//卖
                                    //空头
            tmpmap->shortTdPosition = pTrade->Volume;
            tmpmap->shortYdPosition = 0;
            tmpmap->shortTotalPosition = pTrade->Volume;
            tmpmap->shortAvaClosePosition = pTrade->Volume;
            tmpmap->shortHoldAvgPrice = tradePrice;
            tmpmap->shortAmount = tradePrice*pTrade->Volume*multiplyFactor;
            //多头
            tmpmap->longTdPosition = 0;
            tmpmap->longYdPosition = 0;
            tmpmap->longTotalPosition = 0;
            tmpmap->longHoldAvgPrice = 0;
            tmpmap->longAmount = 0;
        }
        normalMMPositionmap[instrumentID] = tmpmap;
    } else {
        ///平仓
        //        #define USTP_FTDC_OF_Close '1'
        //        ///强平
        //        #define USTP_FTDC_OF_ForceClose '2'
        //        ///平今
        //        #define USTP_FTDC_OF_CloseToday '3'
        //        ///平昨
        //        #define USTP_FTDC_OF_CloseYesterday '4'
        HoldPositionInfo* tmpinfo = map_iterator->second;
        int realShortPstLimit = 0;
        int realLongPstLimit = 0;
        hpstinfo = tmpinfo;
        if (direction == "0") {//买
            double tmp_shortHoldAvgPrice = tmpinfo->shortHoldAvgPrice;//空头持仓均价
            int tmp_totalPst = tmpinfo->shortTotalPosition;//原空头持仓量
            double tmp_totalAmount = tmpinfo->shortAmount;//原空头交易金额
            if (offsetFlag == "0") {//买开仓,多头增加
                tmpinfo->longTdPosition = tmpinfo->longTdPosition + pTrade->Volume;
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                double tmp_longHoldAvgPrice = tmpinfo->longHoldAvgPrice;//多头持仓均价
                int tmp_totalPst = tmpinfo->longTotalPosition;//原多头持仓量
                double tmp_totalAmount = tmpinfo->longAmount;//原多头交易金额
                realLongPstLimit = tmp_tdpst + tmp_ydpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
                tmpinfo->longAvaClosePosition = tmpinfo->longAvaClosePosition + volume;
                tmpinfo->longHoldAvgPrice = (tmp_longHoldAvgPrice*tmp_totalPst + tradePrice*pTrade->Volume) / (realLongPstLimit);//当前多头持仓均价
                tmpinfo->longAmount = tmp_totalAmount + tradePrice*pTrade->Volume*multiplyFactor;//当前多头交易金额
            } else if (offsetFlag == "1") {//买平仓,空头减少
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                //int tmp_num = map_iterator->second["shortTotalPosition"];
                if (tmp_tdpst > 0) {
                    if (tmp_tdpst <= pTrade->Volume) {
                        tmp_ydpst = tmp_ydpst - (pTrade->Volume - tmp_tdpst);
                        tmp_tdpst = 0;
                    } else {
                        tmp_tdpst = tmp_tdpst - pTrade->Volume;
                    }
                } else if (tmp_tdpst == 0) {
                    tmp_ydpst = tmp_ydpst - pTrade->Volume;
                } else {
                    cout << "tdposition is error!!!" << endl;
                    LOG(ERROR) << "tdposition is error!!!";
                }
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->shortTdPosition = tmp_tdpst;
                tmpinfo->shortYdPosition = tmp_ydpst;
                tmpinfo->shortTotalPosition = tmp_ydpst + tmp_tdpst;
            } else if (offsetFlag == "3") {//平今
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                tmp_tdpst = tmp_tdpst - pTrade->Volume;
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->shortTdPosition = tmp_tdpst;
                tmpinfo->shortTotalPosition = tmp_ydpst + tmp_tdpst;
            } else if (offsetFlag == "4") {//平昨
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                if (tmp_ydpst == 0) {
                    char c_err[100];
                    sprintf(c_err, "shortYdPosition is zero!!!,please check this rtn trade.");
                    cout << c_err << endl;
                    LOG(ERROR) << c_err;
                }
                tmp_ydpst = tmp_ydpst - pTrade->Volume;
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->shortYdPosition = tmp_ydpst;
                tmpinfo->shortTotalPosition = tmp_ydpst + tmp_tdpst;
            }
            //买平仓处理空头平均价格;买开仓不需要单独处理
            if (offsetFlag != "0") {
                tmpinfo->shortHoldAvgPrice = (tmp_shortHoldAvgPrice*tmp_totalPst - tradePrice*pTrade->Volume) / (realShortPstLimit);//当前空头持仓均价
                tmpinfo->shortAmount = tmp_totalAmount - tradePrice*pTrade->Volume*multiplyFactor;//当前空头交易金额
                tmpinfo->shortAvaClosePosition = tmpinfo->shortAvaClosePosition - volume;//当前空头可平量
            }
        } else if (direction == "1") {//卖
            double tmp_longHoldAvgPrice = tmpinfo->longHoldAvgPrice;//原多头持仓均价
            int tmp_totalPst = tmpinfo->longTotalPosition;//原多头持仓量
            double tmp_totalAmount = tmpinfo->longAmount;//原多头交易金额
            if (offsetFlag == "0") {//卖开仓,空头增加
                tmpinfo->shortTdPosition = tmpinfo->shortTdPosition + pTrade->Volume;
                double tmp_shortHoldAvgPrice = tmpinfo->shortHoldAvgPrice;//原空头持仓均价
                int tmp_totalPst = tmpinfo->shortTotalPosition;//原空头持仓量
                double tmp_totalAmount = tmpinfo->shortAmount;//原空头交易金额
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                realShortPstLimit = tmp_tdpst + tmp_ydpst;
                tmpinfo->shortTotalPosition = realShortPstLimit;
                tmpinfo->shortAvaClosePosition = tmpinfo->shortAvaClosePosition + volume;
                tmpinfo->shortHoldAvgPrice = (tmp_shortHoldAvgPrice*tmp_totalPst + tradePrice*pTrade->Volume) / (realShortPstLimit);//当前空头持仓均价
                tmpinfo->shortAmount = tmp_totalAmount + tradePrice*pTrade->Volume*multiplyFactor;//当前空头交易金额
            } else if (offsetFlag == "1") {//卖平仓,多头减少
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                if (tmp_tdpst > 0) {
                    if (tmp_tdpst <= pTrade->Volume) {
                        tmp_ydpst = tmp_ydpst - (pTrade->Volume - tmp_tdpst);
                        tmp_tdpst = 0;
                    } else {
                        tmp_tdpst = tmp_tdpst - pTrade->Volume;
                    }
                } else if (tmp_tdpst == 0) {
                    tmp_ydpst = tmp_ydpst - pTrade->Volume;
                } else {
                    cout << "tdposition is error!!!" << endl;
                    LOG(ERROR) << "tdposition is error!!!";
                }
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->longTdPosition = tmp_tdpst;
                tmpinfo->longYdPosition = tmp_ydpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
            } else if (offsetFlag == "3") {//平今
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                tmp_tdpst = tmp_tdpst - pTrade->Volume;
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->longTdPosition = tmp_tdpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
            } else if (offsetFlag == "4") {//平昨
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                if (tmp_ydpst == 0) {
                    char c_err[100];
                    sprintf(c_err, "longYdPosition is zero!!!,please check this rtn trade.");
                    cout << c_err << endl;
                    LOG(ERROR) << c_err;
                }
                tmp_ydpst = tmp_ydpst - pTrade->Volume;
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->longYdPosition = tmp_ydpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
            }
            //卖平仓处理多头平均价格;卖开仓不需要单独处理
            if (offsetFlag != "0") {
                tmpinfo->longHoldAvgPrice = (tmp_longHoldAvgPrice*tmp_totalPst - tradePrice*pTrade->Volume) / (realLongPstLimit);//当前多头持仓均价
                tmpinfo->longAmount = tmp_totalAmount - tradePrice*pTrade->Volume*multiplyFactor;//当前多头交易金额
                tmpinfo->longAvaClosePosition = tmpinfo->longAvaClosePosition - volume;
            }
        }
    }
    /* not follow
    if(true){
        LOG(INFO) << "onrtntrade,follow orders judge......";
        if((hpstinfo->shortTotalPosition - hpstinfo->longTotalPosition) > volMetric){
            int closeVol = hpstinfo->shortTotalPosition - hpstinfo->longTotalPosition- volMetric;
            double tmpPrice = insinfo->UpperLimitPrice;
            LOG(INFO) << "short pst=" + boost::lexical_cast<string>(hpstinfo->shortTotalPosition) + ",long pst=" + boost::lexical_cast<string>(hpstinfo->longTotalPosition) +
                          ",over volMetric=" + boost::lexical_cast<string>(volMetric) + ",do follow!buy price=" + boost::lexical_cast<string>(tmpPrice) + ",volume=" + boost::lexical_cast<string>(closeVol);
            addNewOrderTrade(instrumentID,"0","0",tmpPrice,closeVol,"-1");
            //addNewOrder(instrumentID,"0",flag,newOrderPrice,voln-orderCount);
        }else if((hpstinfo->longTotalPosition - hpstinfo->shortTotalPosition) > volMetric){
            int closeVol = hpstinfo->longTotalPosition - hpstinfo->shortTotalPosition- volMetric;
            double tmpPrice = insinfo->LowerLimitPrice;
            LOG(INFO) << "long pst=" + boost::lexical_cast<string>(hpstinfo->longTotalPosition) + ",short pst=" + boost::lexical_cast<string>(hpstinfo->shortTotalPosition) +
                          ",over volMetric=" + boost::lexical_cast<string>(volMetric) + ",do follow!sell price=" + boost::lexical_cast<string>(tmpPrice) + ",volume=" + boost::lexical_cast<string>(closeVol);
            addNewOrderTrade(instrumentID,"1","0",tmpPrice,closeVol,"-1");
        }
    }*/
    string tmpmsg;
    for (unordered_map<string, HoldPositionInfo*>::iterator it = normalMMPositionmap.begin(); it != normalMMPositionmap.end(); it++) {
        HoldPositionInfo* tmpinfo = it->second;
        tmpmsg.append(it->first).append("持仓情况:");
        char char_tmp_pst[10] = { '\0' };
        char char_longyd_pst[10] = { '\0' };
        char char_longtd_pst[10] = { '\0' };
        sprintf(char_tmp_pst, "%d", tmpinfo->longTotalPosition);
        sprintf(char_longyd_pst, "%d", tmpinfo->longYdPosition);
        sprintf(char_longtd_pst, "%d", tmpinfo->longTdPosition);
        char str_avgBuy[25], str_avgSell[25];
        int sig = 2;
        gcvt(tmpinfo->longHoldAvgPrice, sig, str_avgBuy);
        tmpmsg.append("多头数量=");
        tmpmsg.append(char_tmp_pst);
        tmpmsg.append(";今仓数量=");
        tmpmsg.append(char_longtd_pst);
        tmpmsg.append(";昨仓数量=");
        tmpmsg.append(char_longyd_pst);
        tmpmsg.append(";可平数量=" + boost::lexical_cast<string>(tmpinfo->longAvaClosePosition));
        tmpmsg.append(";持仓均价=");
        tmpmsg.append(boost::lexical_cast<string>(tmpinfo->longHoldAvgPrice));
        tmpmsg.append(";持仓金额=" + boost::lexical_cast<string>(tmpinfo->longAmount));
        char char_tmp_pst2[10] = { '\0' };
        char char_shortyd_pst[10] = { '\0' };
        char char_shorttd_pst[10] = { '\0' };
        gcvt(tmpinfo->shortHoldAvgPrice, sig, str_avgSell);
        sprintf(char_tmp_pst2, "%d", tmpinfo->shortTotalPosition);
        sprintf(char_shortyd_pst, "%d", tmpinfo->shortYdPosition);
        sprintf(char_shorttd_pst, "%d", tmpinfo->shortTdPosition);
        tmpmsg.append("空头数量=");
        tmpmsg.append(char_tmp_pst2);
        tmpmsg.append(";今仓数量=");
        tmpmsg.append(char_shorttd_pst);
        tmpmsg.append(";昨仓数量=");
        tmpmsg.append(char_shortyd_pst);
        tmpmsg.append(";可平数量=" + boost::lexical_cast<string>(tmpinfo->shortAvaClosePosition));
        tmpmsg.append(";持仓均价=");
        tmpmsg.append(boost::lexical_cast<string>(tmpinfo->shortHoldAvgPrice));
        tmpmsg.append(";持仓金额=" + boost::lexical_cast<string>(tmpinfo->shortAmount) + ";#");
    }
    cout << tmpmsg << endl;
    LOG(INFO) << tmpmsg;
    return 0;
}
int getPriceExistAmount(string instrumentID,string direction,double newOrderPrice,double lastPrice,double tickPrice){
    int orderCount = 0;
    //list<OrderInfo*> tmplist;
    string info = "";
    string allOrderlsit;
    //double lastPrice = marketData->lastPrice;
    if(direction == "0"){
        info = "买";
        for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();it++){
            OrderInfo* orderInfo = *it;
            allOrderlsit += getOrderInfo(orderInfo);
            if(abs(newOrderPrice - orderInfo->price) < floatMetric){//how many orders on newOrderPrice
                orderCount += orderInfo->volume;
            }
            if(orderInfo->status == "0"&&((lastPrice - orderInfo->price) >= orderPriceLevel*tickPrice || (orderInfo->price < newOrderPrice&&orderInfo->volume>1))){
                LOG(INFO) << "OrderAction:long order price=" + boost::lexical_cast<string>(orderInfo->price) + ",lastPrice=" + boost::lexical_cast<string>(lastPrice) + ",over " +
                              boost::lexical_cast<string>(orderPriceLevel) +" tickprice.volume=" + boost::lexical_cast<string>(orderInfo->volume) +",need order action.";
                tryOrderAction(instrumentID,orderInfo,"1");
                orderInfo->status="1";
            }
        }
    }else if(direction == "1"){
        info = "卖";
        for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();it++){
            OrderInfo* orderInfo = *it;
            allOrderlsit += getOrderInfo(orderInfo);
            if(abs(newOrderPrice - orderInfo->price) < floatMetric){
                orderCount += orderInfo->volume;
            }
            if(orderInfo->status == "0"&&((orderInfo->price -lastPrice) >= orderPriceLevel*tickPrice|| (orderInfo->price > newOrderPrice&&orderInfo->volume>1))){
                LOG(INFO) << "OrderAction:short order price=" + boost::lexical_cast<string>(orderInfo->price) + ",lastPrice=" + boost::lexical_cast<string>(lastPrice) + ",over "+
                              boost::lexical_cast<string>(orderPriceLevel) +" tickprice.volume=" + boost::lexical_cast<string>(orderInfo->volume) +",need order action.";
                tryOrderAction(instrumentID,orderInfo,"1");
                orderInfo->status="1";
            }
        }
    }

    LOG(INFO) << "当前价格" + boost::lexical_cast<string>(newOrderPrice) + "挂单" + info +",数量=" + boost::lexical_cast<string>(orderCount) + ";" + allOrderlsit;
    return orderCount;
}

void stopLossProcess(MarketData *mkData) {
    try {
        double lastPrice = mkData->lastPrice;
        double tickPrice = getPriceTick(mkData->instrumentID);
        string instrumentID = mkData->instrumentID;
        //process hold_position,change some info.
        for(list<HoldPositionDetail*>::iterator hdIT = holdPositionList.begin();hdIT != holdPositionList.end();hdIT++){
            HoldPositionDetail* hpd = *hdIT;
            //if(hpd->closeClientOrderToken!=0&&hpd->instrumentID==instrumentID){
            //all position compute hold time only,no matter which type
            if(hpd->closeClientOrderToken!=0){
                struct timeval holdEndTime;
                gettimeofday(&holdEndTime,NULL);
                unsigned long holdTime=0;
                holdTime = (holdEndTime.tv_sec-hpd->holdTimeStart.tv_sec)+(holdEndTime.tv_usec-hpd->holdTimeStart.tv_usec)/1000000;
                if(holdTime>=positionHoldTime){
                /*if((hpd->direction == "0"&&(hpd->openPrice-lastPrice)>=tickPrice)||//long position
                        (hpd->direction == "1"&&(lastPrice-hpd->openPrice)>=tickPrice)){//short position*/
                    //1、撤单
                    EES_CancelOrder  clOrder;
                    memset(&clOrder, 0, sizeof(EES_CancelOrder));
                    strcpy(clOrder.m_Account, INVESTOR_ID.c_str());
                    clOrder.m_Quantity = hpd->volume;
                    clOrder.m_MarketOrderToken = hpd->closeMarketOrderToken;
                    ptradeApi->reqOrderAction(&clOrder);
                    string tmporder = getCancleOrderInfo(&clOrder);
                    LOG(INFO) << "position direction="+hpd->direction+",openprice="+ num2str(hpd->openPrice)+",lastPrice="+num2str(lastPrice)+
                                  ",already holdTime="+num2str(holdTime)+" seconds, default positionHoldTime="+num2str(positionHoldTime)+" s, need to action.order action=" + tmporder;
                }
            }
        }
    }catch (const runtime_error &re) {
        cerr << re.what() << endl;
        LOG(ERROR) << re.what();
    }
    catch (exception* e) {
        cerr << e->what() << endl;
        LOG(ERROR) << e->what();
    }

}
//when order price over gap,execute order action
//actionType:"1",order action directly;"2",judge
void tryOrderAction(string instrumentID,OrderInfo* orderInfo,string actionType){
    unsigned int marketOrderToken = 0;
    //报单结构体
    OriginalOrderFieldInfo* oriOrderField;
    string orid = boost::lexical_cast<string>(orderInfo->userID) + boost::lexical_cast<string>(orderInfo->clientOrderToken);
    LOG(INFO) << "action orid=" + orid;
    unordered_map<string, OriginalOrderFieldInfo*>::iterator it = originalOrderMap.find(orid);
    if(it != originalOrderMap.end()){
        oriOrderField = it->second;
        marketOrderToken = oriOrderField->marketOrderToken;
        LOG(INFO) << "find orid=" + orid + ";res marketOrderToken=" + boost::lexical_cast<string>(marketOrderToken);
    }else{
        LOG(ERROR) << "ORDER's useid = " + boost::lexical_cast<string>(orderInfo->userID) + ",is not our useid=" + boost::lexical_cast<string>(USER_ID) + ",orid=" + orid + " is not in originalOrderMap.";
        return;
    }
    if(actionType == "1"){//direction orderAction
        //1、撤单
        EES_CancelOrder  clOrder;
        memset(&clOrder, 0, sizeof(EES_CancelOrder));
        strcpy(clOrder.m_Account, INVESTOR_ID.c_str());
        clOrder.m_Quantity = orderInfo->volume;
        clOrder.m_MarketOrderToken = marketOrderToken;
        ptradeApi->reqOrderAction(&clOrder);
        string tmporder = getCancleOrderInfo(&clOrder);
        LOG(INFO) << "direct action;,need to action.order action=" + tmporder;
    }else if(actionType == "2"){
        double tickPrice = getPriceTick(instrumentID);
        int overNums = 40;
        //only open price will be action;close price do'nt action
        /*
        if((orderInfo->orderType == "0" || orderInfo->orderType == "1") && abs(marketData->lastPrice - orderInfo->price)/tickPrice >= overNums){//bias to long ,action
            //1、撤单
            EES_CancelOrder  clOrder;
            memset(&clOrder, 0, sizeof(EES_CancelOrder));
            strcpy(clOrder.m_Account, INVESTOR_ID.c_str());
            clOrder.m_Quantity = orderInfo->volume;
            clOrder.m_MarketOrderToken = marketOrderToken;
            ptradeApi->reqOrderAction(&clOrder);
            string tmporder = getCancleOrderInfo(&clOrder);
            LOG(INFO) << "lastPrice=" + boost::lexical_cast<string>(marketData->lastPrice) + ",order price=" + boost::lexical_cast<string>(orderInfo->price) +
                         ";bias over "  + boost::lexical_cast<string>(overNums) +  "ticks,action.orderinfo:" + tmporder;
        }*/
    }

}
string getCancleOrderInfo(EES_CancelOrder  *clOrder){
    string msg = "";
    msg += "account=" + boost::lexical_cast<string>(clOrder->m_Account) + ";";
    msg += "m_Quantity=" + boost::lexical_cast<string>(clOrder->m_Quantity) + ";";
    msg += "m_MarketOrderToken=" + boost::lexical_cast<string>(clOrder->m_MarketOrderToken) + ";";
    return msg;
}
//addition info used for feed back
void addNewOrderTrade(string instrumentID,string direction,string offsetFlag,double orderInsertPrice,int volume,string mkType, AdditionOrderInfo* addinfo){
    //string instrumentID = boost::lexical_cast<string>(pDepthMarketData->InstrumentID);
    unsigned int newOrderToken = ++iOrderRef;
    double price = orderInsertPrice;
    unsigned char m_side = 0;
    string orderType = "0";
    OrderInfo* orderInfo = new OrderInfo();
    if(direction == "0"){
        //wait for trade,used for orderAction
        orderInfo->userID = USER_ID;
        orderInfo->clientOrderToken = newOrderToken;
        orderType = "0";
        if(offsetFlag == "1"){//choose close today or close yestoday
            orderType = "0" + offsetFlag;
            offsetFlag = getCloseMethod(instrumentID,"sell");
            LOG(INFO) << "buy close,orderType=" + orderType;
        }else{
            LOG(INFO) << "buy open,orderType=" + orderType;
        }
        m_side = changeSignalFromNormalToSL(direction,offsetFlag);
        orderInfo->m_Side = m_side;
        orderInfo->offsetFlag = offsetFlag;
        orderInfo->direction = direction;
        orderInfo->price = price;
        orderInfo->orderType = orderType;
        orderInfo->mkType = mkType;
        orderInfo->volume = volume;
        orderInfo->instrumentID = instrumentID;
        if(addinfo){
            orderInfo->openStgType=addinfo->openStgType;
        }
        //orderInfo->begin_down_cul = down_culculate;
        bidList.emplace_back(orderInfo);
        //LOG(INFO) << "down_culculate=" + boost::lexical_cast<string>(down_culculate) + ",up_culculate=" + boost::lexical_cast<string>(up_culculate) + ",buy open.";
        LOG(INFO) << "new add bid list." + getOrderInfo(orderInfo);
    }else{
        //wait for trade,used for orderAction
        orderInfo->userID = USER_ID;
        orderInfo->clientOrderToken = newOrderToken;
        orderInfo->direction = direction;
        orderType = "1";
        if(offsetFlag == "1"){//choose close today or close yestoday
            orderType = "1" + offsetFlag;
            offsetFlag = getCloseMethod(instrumentID,"buy");
            LOG(INFO) << "sell close,orderType=" + orderType;
        }else{
            LOG(INFO) << "sell open,orderType=" + orderType;
        }
        m_side = changeSignalFromNormalToSL(direction,offsetFlag);
        orderInfo->m_Side = m_side;
        orderInfo->offsetFlag = offsetFlag;
        orderInfo->price = price;
        orderInfo->orderType = orderType;
        orderInfo->mkType = mkType;
        orderInfo->volume = volume;
        orderInfo->instrumentID = instrumentID;
        if(addinfo){
            orderInfo->openStgType=addinfo->openStgType;
        }
        //orderInfo->begin_up_cul = up_culculate;
        askList.emplace_back(orderInfo);

        //LOG(INFO) << "down_culculate=" + boost::lexical_cast<string>(down_culculate) + ",up_culculate=" + boost::lexical_cast<string>(up_culculate) + ",sell open.";
        LOG(INFO) << "new add ask list." + getOrderInfo(orderInfo);
    }
    if(addinfo){
        addinfo->orderType = orderType;
        addinfo->mkType = mkType;
        addinfo->clientOrderToken = newOrderToken;
    }
    //shengli
    EES_EnterOrderField orderField;
    //memset(&orderField, 0, sizeof(EES_EnterOrderField));
    if(mkType=="pas" || mkType=="0"){
        orderField.m_Tif = EES_OrderTif_Day;//立即单 EES_OrderTif_Day  EES_OrderTif_IOC
    }else if(mkType == "agg"){
        orderField.m_Tif = EES_OrderTif_IOC;//立即单 EES_OrderTif_Day  EES_OrderTif_IOC
    }

    orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
    strcpy(orderField.m_Account, INVESTOR_ID.c_str());
    strcpy(orderField.m_Symbol, instrumentID.c_str());//modify 2.
    orderField.m_Side = m_side;//modify 1.buy open
    orderField.m_Exchange = EES_ExchangeID_shfe;
    orderField.m_SecType = EES_SecType_fut;
    orderField.m_Price = price;//modify 3.
    orderField.m_Qty = volume;
    orderField.m_ClientOrderToken = newOrderToken;
    AdditionOrderInfo aoi;
    //memset(&aoi, 0, sizeof(AdditionOrderInfo));
    aoi.mkType = mkType;
    aoi.orderType = orderType;
    if(addinfo){
        aoi.function = addinfo->function;
        aoi.timeFlag = addinfo->timeFlag;
        aoi.openStgType = addinfo->openStgType;
    }
    ptradeApi->reqOrderInsert(&orderField,&aoi);
    string tmporder = "";
    LOG(INFO) << tmporder;

}
void processAverageGapGprice(TradeFieldInfo *pTrade) {
    ///买卖方向
    string	direction = pTrade->Direction;
    //char Direction[] = { direction,'\0' };
    //sprintf(Direction,"%s",direction);
    ///开平标志
    string	offsetFlag = pTrade->OffsetFlag;
    //char OffsetFlag[] = { offsetFlag,'\0' };
    ///合约代码
    string instrumentID = pTrade->InstrumentID;
    //string str_inst = string(InstrumentID);
    unordered_map<string, HoldPositionInfo*>::iterator map_iterator = positionmap.find(instrumentID);
    if (map_iterator == positionmap.end()) {
        LOG(ERROR) << "processAverageGapGprice,无法查找到持仓信息：" + instrumentID;
        return;
    }
    HoldPositionInfo* currHoldPst = map_iterator->second;
    unordered_map<string, vector<string>>::iterator vec_it = instr_map.find(instrumentID);
    if (vec_it == instr_map.end()) {
        cout << "can't find instrument list" << endl;
        LOG(ERROR) << ("processAverageGapGprice:can't find instrumentid= " + instrumentID);
        return;
    } else {
        vector<string> tmp_instr_vec = vec_it->second;//找到对应的配对合约
        for (unsigned int i = 0; i < tmp_instr_vec.size(); i++) {
            string pd_instrument = tmp_instr_vec[i];
            string ins_com_key = getComInstrumentKey(pd_instrument, instrumentID);
            //技术指标
            TechMetric* tm;
            unordered_map<string, TechMetric*>::iterator tmIt = techMetricMap.find(ins_com_key);
            if (tmIt == techMetricMap.end()) {
                LOG(ERROR) << "processAverageGapGprice,无法查找到" + ins_com_key + "的技术指标";
                return;
            } else {
                tm = tmIt->second;
            }
            //配对合约的持仓信息
            unordered_map<string, HoldPositionInfo*>::iterator mpd_iterator = positionmap.find(pd_instrument);
            if (mpd_iterator == positionmap.end()) {
                LOG(ERROR) << "processAverageGapGprice,无法查找到持仓信息：" + pd_instrument;
                return;
            }
            HoldPositionInfo* pdHoldPst = mpd_iterator->second;
            double tmpMeanas = 0;
            double tmpMeands = 0;
            string activeInstrumentID = tm->activeInstrumentID;
            if (activeInstrumentID == instrumentID) {
                LOG(INFO) << "活跃合约成交，计算平均持仓均价";
                if (instrumentID > pd_instrument) {//正套为pd_instrument的多头持仓均价-str_inst的空头持仓持仓;
                    tmpMeanas = pdHoldPst->longHoldAvgPrice - currHoldPst->shortHoldAvgPrice;

                    tmpMeands = pdHoldPst->shortHoldAvgPrice - currHoldPst->longHoldAvgPrice;
                } else {//str_inst为近月
                    tmpMeanas = currHoldPst->longHoldAvgPrice - pdHoldPst->shortHoldAvgPrice;
                    tmpMeands = currHoldPst->shortHoldAvgPrice - pdHoldPst->longHoldAvgPrice;
                }
                tm->asHoldMeanGap = tmpMeanas;
                tm->dsHoldMeanGap = tmpMeands;
                LOG(INFO) << ins_com_key + " processAverageGapGprice正套平均持仓均价：" + boost::lexical_cast<string>(tmpMeanas);
                LOG(INFO) << ins_com_key + " processAverageGapGprice反套平均持仓均价：" + boost::lexical_cast<string>(tmpMeands);
            }
        }
    }
}
void tradeParaProcessTwo() {
    for (unordered_map<string, HoldPositionInfo*>::iterator map_iterator = positionmap.begin(); map_iterator != positionmap.end(); map_iterator++) {
        string tmpmsg;
        HoldPositionInfo* tmpinfo = map_iterator->second;
        realShortPstLimit = tmpinfo->shortTotalPosition;
        realLongPstLimit = tmpinfo->longTotalPosition;
        int shortYdPst = tmpinfo->shortYdPosition;
        int longYdPst = tmpinfo->longYdPosition;
        if (longYdPst > 0) {
            shortPstIsClose = 2;
            short_offset_flag = 4;
        } else {
            short_offset_flag = 3;//无昨仓，则平今
        }
        if (shortYdPst > 0) {
            longPstIsClose = 2;
            long_offset_flag = 4;
        } else {
            long_offset_flag = 3;//无昨仓，则平今
        }
        //复位操作
        if (realShortPstLimit == 0) {
            tmpinfo->shortAmount = 0;
            tmpinfo->shortHoldAvgPrice = 0;
        }
        if (realLongPstLimit == 0) {
            tmpinfo->longAmount = 0;
            tmpinfo->longHoldAvgPrice = 0;
        }
        /*
        // buy or open judge
        if (realLongPstLimit > longpstlimit) { //多头超过持仓限额，且必须空头有持仓才能多头平仓
        char char_limit[10] = { '\0' };
        sprintf(char_limit, "%d", realLongPstLimit);
        longPstIsClose = 11;//long can not to open new position
        tmpmsg.append("多头持仓量=");
        tmpmsg.append(char_limit).append("大于longpstlimit,long can not to open new position");
        }
        else if (realShortPstLimit > shortpstlimit) {//空头开平仓判断
        char char_limit[10] = { '\0' };
        sprintf(char_limit, "%d", realShortPstLimit);
        shortPstIsClose = 11;
        tmpmsg.append("空头持仓量=");
        tmpmsg.append(char_limit).append("大于shortpstlimit,short can not to open new position");
        }

        cout << tmpmsg << endl;
        LOG(INFO) << tmpmsg;
        */
        //spread set
        int bidAkdSpread = abs(realShortPstLimit - realLongPstLimit);
        /*
        if (bidAkdSpread >= firstGap && bidAkdSpread < secondGap && realShortPstLimit  > realLongPstLimit) {
        bidCulTimes += 2;
        if (down_culculate >= bidCulTimes) {
        down_culculate = (4 * down_culculate) / 5;
        }
        }
        else if (bidAkdSpread >= secondGap && realShortPstLimit > realLongPstLimit) {
        bidCulTimes += 4;
        if (down_culculate >= bidCulTimes) {
        down_culculate = (4 * down_culculate) / 5;
        }
        }
        else if (bidAkdSpread >= firstGap && bidAkdSpread < secondGap && realShortPstLimit < realLongPstLimit) {
        askCulTimes += 2;
        if (up_culculate >= askCulTimes) {
        up_culculate = (4 * up_culculate) / 5;
        }
        }
        else if (bidAkdSpread >= secondGap && realShortPstLimit < realLongPstLimit) {
        askCulTimes += 4;
        if (up_culculate >= askCulTimes) {
        up_culculate = (4 * up_culculate) / 5;
        }
        }
        else {
        bidCulTimes = cul_times;
        askCulTimes = cul_times;
        }
        lastABSSpread = bidAkdSpread;
        */
    }
}
vector<string> split(string str, string pattern) {
    str += pattern;
    vector<string> strvev;
    int lenstr = str.size();
    for (int i = 0; i<lenstr; i++) {
        int pos = str.find(pattern, i);
        if (pos	< lenstr) {

            string findstr = str.substr(i, pos - i);
            strvev.push_back(findstr);
            i = pos + pattern.size() - 1;
        }
    }
    return strvev;
}
void processFollow(MarketData *pDepthMarketData) {
    /*
    boost::recursive_mutex::scoped_lock SLock(willTrade_mtx);//锁定
    boost::recursive_mutex::scoped_lock SLock3(stopProfit_mtx);
    boost::recursive_mutex::scoped_lock SLock2(pst_mtx);//锁定
    */
    double lastPrice = pDepthMarketData->lastPrice;
    string instrumentID = boost::lexical_cast<string>(pDepthMarketData->instrumentID);
    double multiplier = getMultipler(instrumentID);
    double tick = getPriceTick(instrumentID);
    //套利单追单
    for(unordered_map<string, list<TradeInfo*>*>::iterator untrade_it = willTradeMap.begin(); untrade_it != willTradeMap.end();untrade_it++) {
        list<TradeInfo*>* tradeList = untrade_it->second;
        for (list<TradeInfo*>::iterator it = tradeList->begin(); it != tradeList->end();it++) {
            TradeInfo* tmpInfo = *it;
            //不活跃合约已经成交，活跃合约没有成交，且行情为活跃合约,判断是否价格偏移
            if (tmpInfo->notActiveIsTraded && !tmpInfo->activeIsTraded && tmpInfo->activeInstrumentid == instrumentID) {
                double activeOrderInsertPrice = tmpInfo->activeOrderInsertPrice;
                if (abs(lastPrice - activeOrderInsertPrice) - biasTickNums*tick >= 0) {//价格偏移，且未追单或者已追单，那么撤单
                    if ((tmpInfo->activeOrderActionStatus == "100"|| tmpInfo->activeOrderActionStatus == "2" || tmpInfo->activeOrderActionStatus == "1"|| tmpInfo->activeOrderActionStatus == "a")) {//刚发送追单中的撤单请求=100；撤单中(接受到撤单回报=5)；追单已经成交=0时，不能继续撤单
                        if(isLogout){
                            LOG(INFO) << "activeOrderActionStatus=" + boost::lexical_cast<string>(tmpInfo->activeOrderActionStatus) + ",不能追单";
                        }
                        continue;
                    }
                    if(isLogout){
                        LOG(INFO) << "价格偏移:活跃合约 " + instrumentID + "当前行情=" + boost::lexical_cast<string>(lastPrice) + ",下单价格=" + boost::lexical_cast<string>(activeOrderInsertPrice) + ",价格偏移超过" + boost::lexical_cast<string>(biasTickNums*tick) +
                            ",追单状态=" + boost::lexical_cast<string>(tmpInfo->activeOrderActionStatus);
                    }
                    tmpInfo->activeOrderActionStatus = "100";//初始状态
                    double avgBuy = 0;//多头持仓均价
                    double avgSell = 0;//空头持仓均价
                    double maxGap = 0;
                    double minGap = 0;
                    double tmpgap = 0;

                    if (true) {//总体处于套利范围，并且追单次数低于预设值,执行追单
                        if(isLogout){
                            LOG(INFO) << "价格由" + boost::lexical_cast<string>(activeOrderInsertPrice) + "偏移到" + boost::lexical_cast<string>(lastPrice) + ";买均价=" + boost::lexical_cast<string>(avgBuy) + ",卖均价=" + boost::lexical_cast<string>(avgSell) + ",均价gap=" + boost::lexical_cast<string>(tmpgap) +
                                "处于" + boost::lexical_cast<string>(minGap) + "和" + boost::lexical_cast<string>(maxGap) + "之间，执行追单操作:撤单";
                        }
                        if (tmpInfo->activeTradeSide == 1 && tmpInfo->activeTradeSide == 3 && tmpInfo->activeTradeSide == 5) {
                            if(isLogout){
                                LOG(INFO) << "总体处于套利范围，并且追单次数低于预设值,执行追单.买开仓价格由" + boost::lexical_cast<string>(tmpInfo->activeOrderInsertPrice) + "上调到" + boost::lexical_cast<string>(lastPrice) + ".追单次数由" + boost::lexical_cast<string>(tmpInfo->followTimes) + "基础上加1";
                            }

                        } else {
                            if(isLogout){
                                 LOG(INFO) << "总体处于套利范围，并且追单次数低于预设值,执行追单.卖开仓价格由" + boost::lexical_cast<string>(tmpInfo->activeOrderInsertPrice) + "下调到" + boost::lexical_cast<string>(lastPrice) + ".追单次数由" + boost::lexical_cast<string>(tmpInfo->followTimes) + "基础上加1";
                            }

                        }
                        tmpInfo->activeOrderInsertPrice = lastPrice;
                        tmpInfo->followTimes += 1;
                        //1、撤单
                        /*
                        list<string> paras;
                        paras.push_back("InstrumentID=" + instrumentID);
                        paras.push_back("OrderRef=" + string(tmpInfo->activeOrderRef));
                        paras.push_back("FrontID=" + boost::lexical_cast<string>(FRONT_ID));
                        paras.push_back("SessionID=" + boost::lexical_cast<string>(SESSION_ID));
                        */
                        EES_CancelOrder  clOrder;
                        memset(&clOrder, 0, sizeof(EES_CancelOrder));
                        strcpy(clOrder.m_Account, INVESTOR_ID.c_str());
                        clOrder.m_Quantity = tmpInfo->activeVolume;
                        clOrder.m_MarketOrderToken = tmpInfo->activeMarketOrderToken;
                        ptradeApi->reqOrderAction(&clOrder);
                    } else {
                        if(isLogout){
                            LOG(INFO) << "价格由" + boost::lexical_cast<string>(activeOrderInsertPrice) + "偏移到" + boost::lexical_cast<string>(lastPrice) + ";买均价=" + boost::lexical_cast<string>(avgBuy) + ",卖均价=" + boost::lexical_cast<string>(avgSell) + ",均价gap=" + boost::lexical_cast<string>(tmpgap) +
                                ",套利区间:" + boost::lexical_cast<string>(minGap) + "到" + boost::lexical_cast<string>(maxGap) + "之间;当前追单次数=" + boost::lexical_cast<string>(tmpInfo->followTimes) +",不执行追单操作";
                        }
                    }

                }
            }
        }
    }
    //止盈单追单
    for (unordered_map<string, list<TradeInfo*>*>::iterator untrade_it = stopProfitWillTradeMap.begin(); untrade_it != stopProfitWillTradeMap.end(); untrade_it++) {
        list<TradeInfo*>* tradeList = untrade_it->second;
        for (list<TradeInfo*>::iterator it = tradeList->begin(); it != tradeList->end(); it++) {
            TradeInfo* tmpInfo = *it;
            //不活跃合约已经成交，活跃合约没有成交，且行情为活跃合约,判断是否价格偏移
            if (tmpInfo->notActiveIsTraded && !tmpInfo->activeIsTraded && tmpInfo->activeInstrumentid == instrumentID) {
                double activeOrderInsertPrice = tmpInfo->activeOrderInsertPrice;
                if (abs(lastPrice - activeOrderInsertPrice) - biasTickNums*tick >= 0) {//价格偏移，且未追单或者已追单，那么撤单
                    if ((tmpInfo->activeOrderActionStatus == "100" || tmpInfo->activeOrderActionStatus == "1" || tmpInfo->activeOrderActionStatus == "2" || tmpInfo->activeOrderActionStatus == "a")) {//刚发送追单中的撤单请求=100；撤单中(接受到撤单回报=5)；追单已经成交=0时，不能继续撤单
                        if(isLogout){
                            LOG(INFO) << "activeOrderActionStatus=" + boost::lexical_cast<string>(tmpInfo->activeOrderActionStatus) + ",不能追单";
                        }
                        continue;
                    }
                    if(isLogout){
                        LOG(INFO) << "止盈价格偏移:活跃合约 " + instrumentID + "当前行情=" + boost::lexical_cast<string>(lastPrice) + ",下单价格=" + boost::lexical_cast<string>(activeOrderInsertPrice) + ",价格偏移超过" + boost::lexical_cast<string>(biasTickNums*tick) +
                            ",追单状态=" + boost::lexical_cast<string>(tmpInfo->activeOrderActionStatus);
                    }

                    tmpInfo->activeOrderActionStatus = boost::lexical_cast<string>(100);//初始状态
                    double avgBuy = 0;//多头持仓均价
                    double avgSell = 0;//空头持仓均价
                    double maxGap = 0;
                    double minGap = 0;
                    double tmpgap = 0;
                    /*
                                                                                        //获得活跃合约的持仓信息
                    unordered_map<string, HoldPositionInfo*>::iterator map_iterator = positionmap.find(instrumentID);//活跃合约
                    if (map_iterator == positionmap.end()) {//无持仓信息
                        LOG(INFO) << "processFollow:无法查找到合约的持仓信息：" + instrumentID;
                        continue;
                    }
                    //获得不活跃合约的持仓信息
                    unordered_map<string, HoldPositionInfo*>::iterator notActive_map_iterator = positionmap.find(tmpInfo->notActiveInstrumentid);
                    if (notActive_map_iterator == positionmap.end()) {//无持仓信息
                        LOG(INFO) << "processFollow:无法查找到合约的持仓信息：" + tmpInfo->notActiveInstrumentid;
                        continue;
                    }
                    HoldPositionInfo* tmpHoldInfo_active = map_iterator->second;
                    HoldPositionInfo* tmpHoldInfo_notactive = notActive_map_iterator->second;
                    string comKey = getComInstrumentKey(instrumentID, tmpInfo->notActiveInstrumentid);
                    unordered_map<string, PriceGap*>::iterator instr_price_gap_it = instr_price_gap.find(comKey);//找到配对合约的预警值																						   //double price_gap_pre_set = 0;
                    PriceGap* gapinfo;
                    if (instr_price_gap_it != instr_price_gap.end()) {
                        gapinfo = instr_price_gap_it->second;//找到预设价差
                    } else {
                        LOG(ERROR) << "processFollow:没有找到预设价差:" + comKey;
                        continue;
                    }
                    double maxGap = gapinfo->maxGap;
                    double minGap = gapinfo->minGap;
                    double avgBuy = 0;//多头持仓均价
                    double avgSell = 0;//空头持仓均价
                    double tmpgap = 0;
                    if (tmpInfo->activeDirection == "0") {//说明活跃合约是多头下单，不活跃合约是空头下单
                        //实时持仓均价
                        avgBuy = (tmpHoldInfo_active->longAmount + lastPrice*tmpInfo->activeVolume*multiplier) / (multiplier*(tmpHoldInfo_active->longTotalPosition + tmpInfo->activeVolume));
                        //实时持仓均价
                        avgSell = tmpHoldInfo_notactive->shortHoldAvgPrice;

                    } else if (tmpInfo->activeDirection == "1") {//活跃合约是空头下单，不活跃合约是多头下单
                        //实时持仓均价
                        avgSell = (tmpHoldInfo_active->shortAmount + lastPrice*tmpInfo->activeVolume*multiplier) / (multiplier*(tmpHoldInfo_active->shortTotalPosition + tmpInfo->activeVolume));
                        //实时持仓均价
                        avgBuy = tmpHoldInfo_notactive->longHoldAvgPrice;
                    }
                    tmpgap = avgBuy - avgSell;
                    char str_avgBuy[25], str_avgSell[25], str_tmpgap[25];
                    int sig = 2;
                    gcvt(avgBuy, sig, str_avgBuy);
                    gcvt(avgSell, sig, str_avgSell);
                    gcvt(tmpgap, sig, str_tmpgap);
                    */
                    //if (tmpgap <= maxGap && tmpgap >= minGap && tmpInfo->followTimes < maxFollowTimes) {//总体处于套利范围，并且追单次数低于预设值,执行追单
                    //if ( tmpInfo->followTimes < maxFollowTimes) {//总体处于套利范围，并且追单次数低于预设值,执行追单
                    //if (tmpgap > maxGap || tmpgap < minGap) {//总体处于套利范围，并且追单次数低于预设值,执行追单
                    if (true) {//总体处于套利范围，并且追单次数低于预设值,执行追单
                        if(isLogout){
                            LOG(INFO) << "止盈价格由" + boost::lexical_cast<string>(activeOrderInsertPrice) + "偏移到" + boost::lexical_cast<string>(lastPrice) + ";买均价=" + boost::lexical_cast<string>(avgBuy) + ",卖均价=" + boost::lexical_cast<string>(avgSell) + ",均价gap=" + boost::lexical_cast<string>(tmpgap) +
                                "处于" + boost::lexical_cast<string>(minGap) + "和" + boost::lexical_cast<string>(maxGap) + "之间，执行追单操作:撤单";
                        }

                        if(tmpInfo->activeTradeSide == 1 && tmpInfo->activeTradeSide == 3 && tmpInfo->activeTradeSide == 5)  {
                            if(isLogout){
                                LOG(INFO) << "总体处于套利范围，并且追单次数低于预设值,执行追单.买开仓价格由" + boost::lexical_cast<string>(tmpInfo->activeOrderInsertPrice) + "上调到" + boost::lexical_cast<string>(lastPrice) + ".追单次数由" + boost::lexical_cast<string>(tmpInfo->followTimes) + "基础上加1";
                            }

                        } else {
                            if(isLogout){
                                LOG(INFO) << "总体处于套利范围，并且追单次数低于预设值,执行追单.卖开仓价格由" + boost::lexical_cast<string>(tmpInfo->activeOrderInsertPrice) + "下调到" + boost::lexical_cast<string>(lastPrice) + ".追单次数由" + boost::lexical_cast<string>(tmpInfo->followTimes) + "基础上加1";
                            }

                        }
                        tmpInfo->activeOrderInsertPrice = lastPrice;
                        tmpInfo->followTimes += 1;
                        //1、撤单
                        /*
                        list<string> paras;
                        paras.push_back("InstrumentID=" + instrumentID);
                        paras.push_back("OrderRef=" + string(tmpInfo->activeOrderRef));
                        paras.push_back("FrontID=" + boost::lexical_cast<string>(FRONT_ID));
                        paras.push_back("SessionID=" + boost::lexical_cast<string>(SESSION_ID));
                        ptradeApi->ReqOrderActionTmp(paras);
                        */
                        EES_CancelOrder  clOrder;
                        memset(&clOrder, 0, sizeof(EES_CancelOrder));
                        strcpy(clOrder.m_Account, INVESTOR_ID.c_str());
                        clOrder.m_Quantity = tmpInfo->activeVolume;
                        clOrder.m_MarketOrderToken = tmpInfo->activeMarketOrderToken;
                        ptradeApi->reqOrderAction(&clOrder);
                    } else {
                        if(isLogout){
                            LOG(INFO) << "价格由" + boost::lexical_cast<string>(activeOrderInsertPrice) + "偏移到" + boost::lexical_cast<string>(lastPrice) + ";买均价=" + boost::lexical_cast<string>(avgBuy) + ",卖均价=" + boost::lexical_cast<string>(avgSell) + ",均价gap=" + boost::lexical_cast<string>(tmpgap) +
                                ",套利区间:" + boost::lexical_cast<string>(minGap) + "到" + boost::lexical_cast<string>(maxGap) + "之间;当前追单次数=" + boost::lexical_cast<string>(tmpInfo->followTimes) + ",不执行追单操作";
                        }

                    }

                }
            }
        }
    }
}
unsigned char changeSignalFromNormalToSL(string direction,string offsetFlag){
    /*
#define EES_SideType_open_long                  1		///< =买单（开今）
#define EES_SideType_close_today_long           2		///< =卖单（平今）
#define EES_SideType_close_today_short          3		///< =买单（平今）
#define EES_SideType_open_short                 4		///< =卖单（开今）
#define EES_SideType_close_ovn_short            5		///< =买单（平昨）
#define EES_SideType_close_ovn_long             6		///< =卖单（平昨）
#define EES_SideType_force_close_ovn_short      7		///< =买单 （强平昨）
#define EES_SideType_force_close_ovn_long       8		///< =卖单 （强平昨）
#define EES_SideType_force_close_today_short    9		///< =买单 （强平今）
#define EES_SideType_force_close_today_long     10		///< =卖单 （强平今）
#define EES_SideType_opt_exec					11		///< =期权行权*/
    if(direction == "0" && offsetFlag == "0"){//买单（开今）
        return EES_SideType_open_long;
    }else if(direction == "1" && offsetFlag == "3"){//卖单（平今）
        return EES_SideType_close_today_long;
    }else if(direction == "0" && offsetFlag == "3"){//买单（平今）
        return EES_SideType_close_today_short;
    }else if(direction == "1" && offsetFlag == "0"){//卖单（开今）
        return EES_SideType_open_short;
    }else if(direction == "0" && offsetFlag == "4"){//买单（平昨）
        return EES_SideType_close_ovn_short;
    }else if(direction == "1" && offsetFlag == "4"){//卖单（平昨）
        return EES_SideType_close_ovn_long;
    }else{
        LOG(ERROR) << "can't find signal:direction=" + direction +",offsetFlag=" + offsetFlag;
    }
    /*
    if(pOrder->m_Side == 1){//买单（开今）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "0";
    }else if(pOrder->m_Side == 2){//卖单（平今）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "3";
    }else if(pOrder->m_Side == 3){//买单（平今）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "3";
    }else if(pOrder->m_Side == 4){//卖单（开今）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "0";
    }else if(pOrder->m_Side == 5){//买单（平昨）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "4";
    }else if(pOrder->m_Side == 6){//卖单（平昨）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "4";
    }*/
}
string getOrderInfo(OrderInfo* info){
    string msg = "";
    msg += "direction=" + info->direction + ";";
    msg += "offsetFlag=" + info->offsetFlag + ";";
    msg += "clientOrderToken=" + boost::lexical_cast<string>(info->clientOrderToken) + ";";
    msg += "instrumentID=" + info->instrumentID + ";";
    msg += "function=" + info->function + ";";
    msg += "orderSysID=" + info->orderSysID + ";";
    msg += "mkType=" + info->mkType + ";";
    msg += "orderType=" + info->orderType + ";";
    msg += "openStgType=" + info->openStgType + ";";
    msg += "price=" + boost::lexical_cast<string>(info->price) + ";";
    msg += "volume=" + boost::lexical_cast<string>(info->volume) + ";";
    return msg;


}
InstrumentInfo* getInstrumentInfo(string instrumentID) {
    //合约priceTick
    unordered_map<string, InstrumentInfo*>::iterator ins_it = instruments.find(instrumentID);
    if (ins_it == instruments.end()) {
        LOG(ERROR) << "无法查找到合约信息.instrumentID=" + instrumentID;
        return NULL;
    }
    InstrumentInfo* insinfo = ins_it->second;
    return insinfo;
}
int getAvailableClosePosition(string instrumentID){
    unordered_map<string, HoldPositionInfo*>::iterator it = normalMMPositionmap.find(instrumentID);
    if (it == normalMMPositionmap.end()) {
        LOG(ERROR) << "合约" + instrumentID + " 无持仓信息操作错误!!";
        return 0;
    }
    HoldPositionInfo* tmpholdInfo = it->second;
    if(isLogout){
        LOG(INFO) << "getAvailableClosePosition:合约" + instrumentID + "short 可平量=" + boost::lexical_cast<string>(tmpholdInfo->shortAvaClosePosition);
        LOG(INFO) << "getAvailableClosePosition:合约" + instrumentID + "long 可平量=" + boost::lexical_cast<string>(tmpholdInfo->longAvaClosePosition);
    }
    holdInfo->shortAvaClosePosition = tmpholdInfo->shortAvaClosePosition;
    holdInfo->shortTotalPosition = tmpholdInfo->shortTotalPosition;
    holdInfo->longAvaClosePosition = tmpholdInfo->longAvaClosePosition;
    holdInfo->longTotalPosition = tmpholdInfo->longTotalPosition;
    /*
    if (direction == "0") {//买平仓，锁定空头可平量
        unordered_map<string, HoldPositionInfo*>::iterator it = positionmap.find(instrumentID);
        if (it == positionmap.end()) {
            LOG(ERROR) << "合约" + instrumentID + " 无持仓信息操作错误!!";
            return 0;
        }
        HoldPositionInfo* tmpholdInfo = it->second;
        if(isLogout){
            LOG(INFO) << "合约" + instrumentID + "可平量=" + boost::lexical_cast<string>(tmpholdInfo->shortAvaClosePosition);
        }
        holdInfo->shortAvaClosePosition = tmpholdInfo->shortAvaClosePosition;
        holdInfo->shortTotalPosition = tmpholdInfo->shortTotalPosition;
        holdInfo->longAvaClosePosition = tmpholdInfo->longAvaClosePosition;
        holdInfo->longTotalPosition = tmpholdInfo->longTotalPosition;
        return tmpholdInfo->shortAvaClosePosition;
    } else if (direction == "1" ) {//卖平仓，锁定多头可平量
        unordered_map<string, HoldPositionInfo*>::iterator it = positionmap.find(instrumentID);
        if (it == positionmap.end()) {
            LOG(ERROR) << "合约" + instrumentID + " 无持仓信息操作错误!!";
            return 0;
        }
        HoldPositionInfo* tmpholdInfo = it->second;
        if(isLogout){
            LOG(INFO) << "合约" + instrumentID + "可平量=" + boost::lexical_cast<string>(tmpholdInfo->longAvaClosePosition);
        }
        holdInfo->shortAvaClosePosition = tmpholdInfo->shortAvaClosePosition;
        holdInfo->shortTotalPosition = tmpholdInfo->shortTotalPosition;
        holdInfo->longAvaClosePosition = tmpholdInfo->longAvaClosePosition;
        holdInfo->longTotalPosition = tmpholdInfo->longTotalPosition;
        return tmpholdInfo->longAvaClosePosition;
    }*/
}

void transformFromCancleOrder(EES_EnterOrderField* pOrder,OrderFieldInfo* third_part_info){
    if(pOrder->m_Side == 1){//买单（开今）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "0";
    }else if(pOrder->m_Side == 2){//卖单（平今）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "3";
    }else if(pOrder->m_Side == 3){//买单（平今）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "3";
    }else if(pOrder->m_Side == 4){//卖单（开今）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "0";
    }else if(pOrder->m_Side == 5){//买单（平昨）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "4";
    }else if(pOrder->m_Side == 6){//卖单（平昨）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "4";
    }
    third_part_info->InstrumentID = boost::lexical_cast<string>(pOrder->m_Symbol);
    third_part_info->OrderRef = boost::lexical_cast<string>(pOrder->m_ClientOrderToken);
    third_part_info->VolumeTotalOriginal = pOrder->m_Qty;
}
void transformFromShengLiPlantformOrder(EES_OrderAcceptField* pOrder,OrderFieldInfo* third_part_info){
    if(pOrder->m_Side == 1){//买单（开今）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "0";
    }else if(pOrder->m_Side == 2){//卖单（平今）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "3";
    }else if(pOrder->m_Side == 3){//买单（平今）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "3";
    }else if(pOrder->m_Side == 4){//卖单（开今）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "0";
    }else if(pOrder->m_Side == 5){//买单（平昨）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "4";
    }else if(pOrder->m_Side == 6){//卖单（平昨）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "4";
    }
    third_part_info->InstrumentID = boost::lexical_cast<string>(pOrder->m_Symbol);
    third_part_info->OrderRef = boost::lexical_cast<string>(pOrder->m_ClientOrderToken);
    third_part_info->clientOrderToken = pOrder->m_ClientOrderToken;
    third_part_info->VolumeTotalOriginal = pOrder->m_Qty;
    third_part_info->OrderStatus = boost::lexical_cast<string>(pOrder->m_OrderState);
    third_part_info->marketOrderToken = pOrder->m_MarketOrderToken;
}
void transformFromExchangeOrder(OriginalOrderFieldInfo* pOrder,OrderFieldInfo* third_part_info){
    if(pOrder->m_Side == 1){//买单（开今）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "0";
    }else if(pOrder->m_Side == 2){//卖单（平今）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "3";
    }else if(pOrder->m_Side == 3){//买单（平今）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "3";
    }else if(pOrder->m_Side == 4){//卖单（开今）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "0";
    }else if(pOrder->m_Side == 5){//买单（平昨）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "4";
    }else if(pOrder->m_Side == 6){//卖单（平昨）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "4";
    }
    third_part_info->InstrumentID = pOrder->instrumentID;
    third_part_info->clientOrderToken = pOrder->clientOrderToken;
    third_part_info->VolumeTotalOriginal = pOrder->volumeTotalOriginal;
    third_part_info->marketOrderToken = pOrder->marketOrderToken;
}
void transformFromExchangeTrade(OriginalOrderFieldInfo* pOrder,TradeFieldInfo* third_part_info){
    if(pOrder->m_Side == 1){//买单（开今）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "0";
    }else if(pOrder->m_Side == 2){//卖单（平今）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "3";
    }else if(pOrder->m_Side == 3){//买单（平今）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "3";
    }else if(pOrder->m_Side == 4){//卖单（开今）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "0";
    }else if(pOrder->m_Side == 5){//买单（平昨）
        third_part_info->Direction = "0";
        third_part_info->OffsetFlag = "4";
    }else if(pOrder->m_Side == 6){//卖单（平昨）
        third_part_info->Direction = "1";
        third_part_info->OffsetFlag = "4";
    }
    third_part_info->InstrumentID = pOrder->instrumentID;
    third_part_info->Price = pOrder->realPrice;
    third_part_info->Volume = pOrder->realVolume;
    third_part_info->marketToken = pOrder->marketOrderToken;
}
void storeInitOrders(EES_EnterOrderField* orderField,AdditionOrderInfo* aoi){
    OriginalOrderFieldInfo* oriOrderFiled = new OriginalOrderFieldInfo();
    oriOrderFiled->m_Side = orderField->m_Side;
    oriOrderFiled->m_HedgeFlag = orderField->m_HedgeFlag;
    oriOrderFiled->instrumentID = orderField->m_Symbol;
    oriOrderFiled->investorID = INVESTOR_ID;
    oriOrderFiled->m_Price = orderField->m_Price;
    oriOrderFiled->m_Tif = orderField->m_Tif;
    oriOrderFiled->clientOrderToken = orderField->m_ClientOrderToken;
    oriOrderFiled->volumeTotalOriginal = orderField->m_Qty;
    oriOrderFiled->m_SecType = orderField->m_SecType;
    //oriOrderFiled->volumeTotalOriginal = orderField->m_Qty;
    oriOrderFiled->exchangeID = orderField->m_Exchange;
    oriOrderFiled->orderType = aoi->orderType;
    oriOrderFiled->openStgType = aoi->openStgType;
    oriOrderFiled->mkType = aoi->mkType;
    oriOrderFiled->function = aoi->function;

    string orid = boost::lexical_cast<string>(USER_ID) + boost::lexical_cast<string>(orderField->m_ClientOrderToken);
    originalOrderMap[orid] = oriOrderFiled;
    if(aoi->timeFlag=="0"){
        LOG(INFO)<<"order init timeFlag="+orid;
        oriOrderFiled->timeFlag = orid;
        ControlOrderInfo* ctlInfo=new ControlOrderInfo();
        gettimeofday(&ctlInfo->timeStart,NULL);
        ctlInfo->howManyOrderLaugh+=1;
        controlTimeMap[oriOrderFiled->timeFlag]=ctlInfo;
    }else{
        LOG(INFO)<<"orid="+orid+",order timeFlag="+aoi->timeFlag;
        oriOrderFiled->timeFlag = aoi->timeFlag;
    }

}
//
bool existUntradeOrder(string type,OrderInfo* untradeOrder){
    for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();){
        OrderInfo* orderInfo = *it;
        string openStgType = orderInfo->openStgType;
        if(openStgType==type){
            LOG(INFO) << "bidlist:find order,openStgType="+type;
            if(untradeOrder){
                untradeOrder=orderInfo;
            }
            return true;
        }else{
            it++;
        }
    }
    for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();){
        OrderInfo* orderInfo = *it;
        string openStgType = orderInfo->openStgType;
        if(openStgType==type){
            LOG(INFO) << "asklist:find order,openStgType="+type;
            if(untradeOrder){
                untradeOrder=orderInfo;
            }
            return true;
        }else{
            it++;
        }
    }
    LOG(INFO) << "in bid ask list:not find order,openStgType="+type;
    return false;
}

//only stop profit order action will be process,other order action will be deleted directly
void deleteOriOrder(unsigned int  clientOrderToken){
    unsigned int  cldClientOrderToken = clientOrderToken;
    LOG(INFO) << "delete ori orderinfo:before cancle order,bidlist size=" + boost::lexical_cast<string>(bidList.size());
    for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();){
        OrderInfo* orderInfo = *it;
        int userID = orderInfo->userID;
        unsigned int clientOrderToken = orderInfo->clientOrderToken;
        //string orderType = orderInfo->orderType;
        //string oriOffsetFlag = orderInfo->offsetFlag;
        if(clientOrderToken == cldClientOrderToken){
            it = bidList.erase(it);
            LOG(INFO) << "bidlist:find cancle order,delete.";
            break;
        }else{
            it++;
        }
    }
    LOG(INFO) << "after cancle order,bidlist size=" + boost::lexical_cast<string>(bidList.size());

    LOG(INFO) << "delete ori orderinfo:before cancle order,asklist size=" + boost::lexical_cast<string>(askList.size());
    for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();){
        OrderInfo* orderInfo = *it;
        int userID = orderInfo->userID;
        unsigned int clientOrderToken = orderInfo->clientOrderToken;
        if(clientOrderToken == cldClientOrderToken){
            it = askList.erase(it);
            LOG(INFO) << "askList:find cancle order,delete.";
            break;
        }else{
            it++;
        }
    }
    LOG(INFO) << "after cancle order,askList size=" + boost::lexical_cast<string>(askList.size());
}
void processHowManyHoldsCanBeClose(OrderFieldInfo *pOrder,string type) {
    string instrumentID = pOrder->InstrumentID;
    string offsetFlag = pOrder->OffsetFlag;
    string lockID = boost::lexical_cast<string>(USER_ID) + boost::lexical_cast<string>(pOrder->clientOrderToken) + boost::lexical_cast<string>(pOrder->marketOrderToken);
    if ("lock" == type) {//锁仓
        unordered_map<string, bool>::iterator lockIT = holdPstIsLocked.find(lockID);
        if (lockIT == holdPstIsLocked.end()) {//未锁定
            holdPstIsLocked[lockID] = true;
            LOG(INFO) << "lockID=" + lockID + " not locked,begin to lock.";
        } else {//已经锁定，不再锁定
            LOG(INFO) << "lockID=" + lockID + " has been locked,no need be locked,return.";
            return;
        }
        if (pOrder->Direction == "0" && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//买平仓，锁定空头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = positionmap.find(instrumentID);
            if (it == positionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，买平仓锁仓操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                LOG(INFO) << "锁仓，合约" + instrumentID + "short可平量从" + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition) + "到" + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition - pOrder->VolumeTotalOriginal);
            }

            holdInfo->shortAvaClosePosition = holdInfo->shortAvaClosePosition - pOrder->tradeVolume;

        } else if (pOrder->Direction == "1" && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//卖平仓，锁定多头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = positionmap.find(instrumentID);
            if (it == positionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，卖平仓锁仓操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                LOG(INFO) << "锁仓，合约" + instrumentID + "long可平量从" + boost::lexical_cast<string>(holdInfo->longAvaClosePosition) + "到" + boost::lexical_cast<string>(holdInfo->longAvaClosePosition - pOrder->VolumeTotalOriginal);
            }
            holdInfo->longAvaClosePosition = holdInfo->longAvaClosePosition - pOrder->tradeVolume;
        }
    } else if ("release" == type) {//释放持仓
        unordered_map<string, bool>::iterator lockIT = holdPstIsLocked.find(lockID);
        if (lockIT != holdPstIsLocked.end()) {//未锁定
            holdPstIsLocked.erase(lockIT);
            LOG(INFO) << "when realse,lockID=" + lockID + " found,delete this lock.";
        } else {//已经锁定，不再锁定
            LOG(INFO) << "lockID=" + lockID + " not found,some thing is wrong?.";
        }
        if (pOrder->Direction == "0" && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//买平仓，释放空头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = positionmap.find(instrumentID);
            if (it == positionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，买平仓释放操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                LOG(INFO) << "放仓，合约" + instrumentID + "short可平量从" + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition) + "到" + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition + pOrder->VolumeTotalOriginal);
            }
            holdInfo->shortAvaClosePosition = holdInfo->shortAvaClosePosition + pOrder->tradeVolume;
        } else if (pOrder->Direction == "1" && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//卖平仓，释放多头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = positionmap.find(instrumentID);
            if (it == positionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，卖平仓释放操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                LOG(INFO) << "放仓,合约" + instrumentID + "long可平量从" + boost::lexical_cast<string>(holdInfo->longAvaClosePosition) + "到" + boost::lexical_cast<string>(holdInfo->longAvaClosePosition + pOrder->VolumeTotalOriginal);
            }
            holdInfo->longAvaClosePosition = holdInfo->longAvaClosePosition + pOrder->tradeVolume;
        }
    }

}
void processNormalHowManyHoldsCanBeClose(OrderFieldInfo *pOrder,string type) {
    LOG(INFO) << "processNormalHowManyHoldsCanBeClose";
    string instrumentID = pOrder->InstrumentID;
    string offsetFlag = pOrder->OffsetFlag;
    if ("lock" == type) {//锁仓
        if (pOrder->Direction == "0" && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//买平仓，锁定空头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = normalMMPositionmap.find(instrumentID);
            if (it == normalMMPositionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，买平仓锁仓操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                LOG(INFO) << "锁仓，合约" + instrumentID + "可平量从" + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition) + "到" + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition - pOrder->VolumeTotalOriginal);
            }

            holdInfo->shortAvaClosePosition = holdInfo->shortAvaClosePosition - pOrder->tradeVolume;

        } else if (pOrder->Direction == "1" && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//卖平仓，锁定多头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = normalMMPositionmap.find(instrumentID);
            if (it == normalMMPositionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，卖平仓锁仓操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                LOG(INFO) << "锁仓，合约" + instrumentID + "可平量从" + boost::lexical_cast<string>(holdInfo->longAvaClosePosition) + "到" + boost::lexical_cast<string>(holdInfo->longAvaClosePosition - pOrder->VolumeTotalOriginal);
            }
            holdInfo->longAvaClosePosition = holdInfo->longAvaClosePosition - pOrder->tradeVolume;
        }
    } else if ("release" == type) {//释放持仓
        if (pOrder->Direction == "0" && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//买平仓，释放空头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = normalMMPositionmap.find(instrumentID);
            if (it == normalMMPositionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，买平仓释放操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                LOG(INFO) << "放仓，合约" + instrumentID + "可平量从" + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition) + "到" + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition + pOrder->VolumeTotalOriginal);
            }
            holdInfo->shortAvaClosePosition = holdInfo->shortAvaClosePosition + pOrder->tradeVolume;
        } else if (pOrder->Direction == "1" && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//卖平仓，释放多头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = normalMMPositionmap.find(instrumentID);
            if (it == normalMMPositionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，卖平仓释放操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                LOG(INFO) << "放仓,合约" + instrumentID + "可平量从" + boost::lexical_cast<string>(holdInfo->longAvaClosePosition) + "到" + boost::lexical_cast<string>(holdInfo->longAvaClosePosition + pOrder->VolumeTotalOriginal);
            }
            holdInfo->longAvaClosePosition = holdInfo->longAvaClosePosition + pOrder->tradeVolume;
        }
    }

}
int processTradeNew(TradeFieldInfo *pTrade) {
    string direction = boost::lexical_cast<string>(pTrade->Direction);
    string offsetflag = boost::lexical_cast<string>(pTrade->OffsetFlag);
    double price = pTrade->Price;
    int vol = pTrade->Volume;
    double mkprice = 0;
    if (direction == "0") {//买开仓

    }
    return 1;
}
// 获取系统的当前时间，单位微秒(us)
int64_t GetSysTimeMicros()
{
#ifdef _WIN32
    // 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
#define EPOCHFILETIME   (116444736000000000UL)
    FILETIME ft;
    LARGE_INTEGER li;
    int64_t tt = 0;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    // 从1970年1月1日0:0:0:000到现在的微秒数(UTC时间)
    tt = (li.QuadPart - EPOCHFILETIME) / 10;
    return tt;
#else
    timeval tv;
    gettimeofday(&tv, 0);
    return (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_usec;
#endif // _WIN32
    return 0;
}
char* GetDiffTime(int64_t start, int64_t end) {
    char char_diff[10] = { '\0' };
    int64_t diff = end - start;
    sprintf(char_diff, "%d", diff);
    return char_diff;
}
////获取当前系统时间
string getCurrentSystemTime() {
    time_t tmp_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm* tm_time = localtime(&tmp_time);
    auto auto_time = put_time(tm_time, "%F %H:%M:%S");
    stringstream ss;
    ss << auto_time;
    string rtn = string(ss.str());
    cout << rtn;
    return rtn;
}
void logEngine() {
    //cout<<"启动进程"<<endl;
    //cout << boosttoolsnamespace::CBoostTools::gbktoutf8("启动进程") << endl;
    ofstream in;
    in.open(logPath, ios::app); //ios::trunc表示在打开文件前将文件清空,由于是写入,文件不存在则创建
    LogMsg *pData;
    int loglength = 0;
    while (1){
        if (logqueue.empty()) {
            this_thread::yield();
        } else if (logqueue.pop(pData)) {
            loglength += 1;
            //cout<<pData->getMsg()<<" "<<logqueue.empty()<<endl;
            string info;
            char cw[2048] = { '\0' };
            info = pData->getMsg();
            //            if(info.size() == 0){
            //                continue;
            //            }
            info = getCurrentSystemTime() + " " + info;
            cout << "运行日志：" << info << ";size=" << info.size() << ";cap=" << endl;
            info.copy(cw, info.size(), 0);
            //cout<<"日志："<<cw<<";size="<<strlen(cw)<<endl;
            in << cw << endl;
            if (loglength%20 == 0) {
                in.flush();
            }
        }
        //cout<<"yigong="<<c<<endl;
    }
    in.close();//关闭文件
}
/*套利组合单发送到保存进程*/
void addArbitrageOrders(TradeInfo* info) {
    //合约的tickPrice
    unordered_map<string, InstrumentInfo*>::iterator insIt = instruments.find(info->notActiveInstrumentid);
    if (insIt == instruments.end()) {
        LOG(ERROR) << "无法查找到合约=" + info->notActiveInstrumentid + "对应的tickPrice";
        return;
    }
    InstrumentInfo* insInfo = insIt->second;
    double priceTick = insInfo->PriceTick;
    string ins_com_key = getComInstrumentKey(info->notActiveInstrumentid, info->activeInstrumentid);
    info->insComKey = ins_com_key;
    info->happyID = generateHappyID(info);
    PriceGap* tmpGap = getPriceGap(ins_com_key);
    info->systemID = tmpGap->systemID;
    string orderType = info->orderType;
    WaitForCloseInfo* wfcInfo ;
    //价格差初始化
    if (info->notActiveInstrumentid > info->activeInstrumentid) {//Key按照近月-远月的格式。不活跃合约为远月合约
        info->realOpenGap = info->activeRealOpenPrice - info->notActiveRealOpenPrice ;
    } else {
        info->realOpenGap = info->notActiveRealOpenPrice - info->activeRealOpenPrice;
    }
    //修改组合预警值.使用期望开仓gap
    LOG(INFO) << "套利单实际开仓成交预警值=" + boost::lexical_cast<string>(info->realOpenGap);
    //string tpinfo = ins_com_key + "组合开仓预警值，由" + boost::lexical_cast<string>(tmpGap->maxGap) + "调整为" + boost::lexical_cast<string>(info->hopeOpenGap - openTick*priceTick) + ",盈利gap调整为" + boost::lexical_cast<string>(info->realOpenGap + profitTickNums*priceTick);
    //LOG(INFO) << tpinfo;


    //技术指标
    TechMetric* tm;
    unordered_map<string, TechMetric*>::iterator tmIt = techMetricMap.find(ins_com_key);
    if (tmIt == techMetricMap.end()) {
        LOG(ERROR) << "addArbitrageOrders,无法查找到" + ins_com_key + "的技术指标";
        return;
    } else {
        tm = tmIt->second;
    }
    if (orderType == "as") {
        LOG(INFO) << "as open,asOpenMetric change from " + boost::lexical_cast<string>(tmpGap->asOpenMetric) + " to " + boost::lexical_cast<string>(tmpGap->asOpenMetric - openTick*priceTick);
        tmpGap->asOpenMetric -= openTick*priceTick;
        info->hopeCloseGap = info->realOpenGap + profitTickNums*priceTick;
        info->badCloseGap = info->realOpenGap - lossTickNums*priceTick;
        //prepare for close
        wfcInfo = new WaitForCloseInfo();
        wfcInfo->closeID = info->happyID;
        wfcInfo->stopLossGap = info->badCloseGap;
        wfcInfo->stopProfitGap =  info->hopeCloseGap;
        wfcInfo->openGap = info->realOpenGap;
        wfcInfo->insComKey = ins_com_key;
        wfcInfo->orderType = "as";
        wfcInfo->status = "0";
        wfcInfo->systemID = tmpGap->systemID;
        wfcInfo->updateTime = info->tradingDay + " " + info->tradeTime;
        allASTradeList.emplace_back(wfcInfo);
        //
        unordered_map<string, list<TradeInfo*>*>::iterator asATMIt = alreadyTradeMapAS.find(ins_com_key);
        if (asATMIt != alreadyTradeMapAS.end()) {
            list<TradeInfo*>* asList = asATMIt->second;
            asList->emplace_back(info);
        } else {
            list<TradeInfo*>* tmplist = new list<TradeInfo*>();
            tmplist->emplace_back(info);
            alreadyTradeMapAS[ins_com_key] = tmplist;
        }
        //计算持仓平均gap值
        tm->asGapTotalAmount += info->realOpenGap*info->activeVolume;
        tm->asTotalVolume += info->activeVolume;
        double tmpmean = tm->asGapTotalAmount / tm->asTotalVolume;
        //tm->asMeanGap = tm->asGapTotalAmount / tm->asTotalVolume;
        LOG(INFO) << ins_com_key + ",addArbitrageOrders,正套持仓平均值=" + boost::lexical_cast<string>(tmpmean);
    } else if (orderType == "ds") {
        LOG(INFO) << "ds open ,dsOpenMetric change from " + boost::lexical_cast<string>(tmpGap->dsOpenMetric) + " to " + boost::lexical_cast<string>(tmpGap->dsOpenMetric + openTick*priceTick);
        tmpGap->dsOpenMetric += openTick*priceTick;
        info->hopeCloseGap = info->realOpenGap - profitTickNums*priceTick;
        info->badCloseGap = info->realOpenGap + lossTickNums*priceTick;
        //prepare for close
        wfcInfo = new WaitForCloseInfo();
        wfcInfo->closeID = info->happyID;
        wfcInfo->stopLossGap = info->badCloseGap;
        wfcInfo->stopProfitGap =  info->hopeCloseGap;
        wfcInfo->openGap = info->realOpenGap;
        wfcInfo->insComKey = ins_com_key;
        wfcInfo->orderType = "ds";
        wfcInfo->status = "0";
        wfcInfo->systemID = tmpGap->systemID;
        wfcInfo->updateTime = info->tradingDay + " " + info->tradeTime;
        allDSTradeList.emplace_back(wfcInfo);
        unordered_map<string, list<TradeInfo*>*>::iterator dsATMIt = alreadyTradeMapDS.find(ins_com_key);
        if (dsATMIt != alreadyTradeMapDS.end()) {
            list<TradeInfo*>* asList = dsATMIt->second;
            asList->emplace_back(info);
        } else {
            list<TradeInfo*>* tmplist = new list<TradeInfo*>();
            tmplist->emplace_back(info);
            alreadyTradeMapDS[ins_com_key] = tmplist;
        }
        //计算持仓平均gap值
        tm->dsGapTotalAmount += info->realOpenGap*info->activeVolume;
        tm->dsTotalVolume += info->activeVolume;
        double tmpmean = tm->dsGapTotalAmount / tm->dsTotalVolume;
        //tm->dsMeanGap = tm->dsGapTotalAmount / tm->dsTotalVolume;
        LOG(INFO) << ins_com_key + ",addArbitrageOrders,反套持仓平均值=" + boost::lexical_cast<string>(tmpmean);
    }
    /*
    //添加到alreadyTradeMap
    unordered_map<string, list<TradeInfo*>*>::iterator tmpit = alreadyTradeMap.find(ins_com_key);
    if (tmpit == alreadyTradeMap.end()) {//新建
        list<TradeInfo*>* tmplist = new list<TradeInfo*>();
        tmplist->push_back(info);
        alreadyTradeMap[ins_com_key] = tmplist;
    } else {
        list<TradeInfo*>* tmplist = tmpit->second;
        tmplist->push_back(info);
    }*/
    //LOG(INFO) << "当前等待套利单信息：" + getAllAlreadyTradedInfo();
    LogMsg *logmsg = new LogMsg();
    cout << "add" <<endl;
    string msg = "businessType=wtm_5;" + getWaitForCloseInfoDetail(wfcInfo);
    logmsg->setMsg(msg);
    networkTradeQueue.push(logmsg);


    string addData = "businessType=wtm_1;";//增加套利单组合
    string tmpstr = getArbDetail(info);
    addData.append(tmpstr);
    logmsg = new LogMsg();
    logmsg->setMsg(addData);
    networkTradeQueue.push(logmsg);
    LOG(INFO) << "套利组合单发送到保存进程" + addData;

    computeGapRange();
    /*
    string storagePrice = "businessType=wtm_10;insComKey=" + ins_com_key + ";maxGap=" + boost::lexical_cast<string>(tmpGap->maxGap) + ";minGap=" + boost::lexical_cast<string>(tmpGap->minGap) + ";profitGap=" +
        boost::lexical_cast<string>(tmpGap->profitGap) + ";systemID=" + tmpGap->systemID;
    logmsg = new LogMsg();
    logmsg->setMsg(storagePrice);
    networkTradeQueue.push(logmsg);*/
}
/*套利组合单发送到保存进程*/
void deleteArbitrageOrders(TradeInfo* info) {
    //合约的tickPrice
    double priceTick = getPriceTick(info->notActiveInstrumentid);
    string ins_com_key = getComInstrumentKey(info->notActiveInstrumentid, info->activeInstrumentid);
    /*
    info->insComKey = ins_com_key;
    info->happyID = generateHappyID(info);
    PriceGap* tmpGap = getPriceGap(ins_com_key);
    info->systemID = tmpGap->systemID;
    string orderType = info->orderType;
    */
    PriceGap* tmpGap = getPriceGap(ins_com_key);
    string closeID = info->closeID;
    string orderType = info->orderType;
    TradeInfo* delTinfo;
    WaitForCloseInfo* wfcInfo;
    //技术指标
    TechMetric* tm;
    unordered_map<string, TechMetric*>::iterator tmIt = techMetricMap.find(ins_com_key);
    if (tmIt == techMetricMap.end()) {
        LOG(ERROR) << "addArbitrageOrders,无法查找到" + ins_com_key + "的技术指标";
        return;
    } else {
        tm = tmIt->second;
    }
    if (orderType == "as") {
        //close option
        if(closeID == ""){
            LOG(ERROR) << "ERROR:as closeid is null,something is wrong!";
        }else{
            LOG(INFO) << "as close ,asOpenMetric change from " + boost::lexical_cast<string>(tmpGap->asOpenMetric) + " to " + boost::lexical_cast<string>(tmpGap->asOpenMetric + openTick*priceTick);
            tmpGap->asOpenMetric += openTick*priceTick;
            bool thingFound = false;
            for(list<WaitForCloseInfo*>::iterator wfcIT = allASTradeList.begin();wfcIT != allASTradeList.end();){
                wfcInfo = *wfcIT;
                if(wfcInfo->closeID == closeID){//delete
                    LOG(INFO) << "as stop loss or profit list found,will be ereased!!";
                    wfcIT = allASTradeList.erase(wfcIT);
                    thingFound = true;
                    break;
                }else{
                    wfcIT ++;
                }
            }
            if(!thingFound){
                LOG(ERROR) << "ERROR:as can't find closeID= " + closeID + " from allASTradeList!";
            }
        }

        unordered_map<string, list<TradeInfo*>*>::iterator asATMIt = alreadyTradeMapAS.find(ins_com_key);
        if (asATMIt != alreadyTradeMapAS.end()) {//平仓删除最早的记录
            list<TradeInfo*>* asList = asATMIt->second;
            delTinfo = asList->front();
            asList->pop_front();
            LOG(INFO) << "allASTradeList size is " + boost::lexical_cast<string>(allASTradeList.size()) + ";alreadyTradeMapAS size is " + boost::lexical_cast<string>(asList->size());
            //计算持仓平均gap值
            tm->asGapTotalAmount -= delTinfo->realOpenGap*delTinfo->activeVolume;
            tm->asTotalVolume -= delTinfo->activeVolume;
            double tmpmean = tm->asGapTotalAmount / tm->asTotalVolume;
            //tm->asMeanGap = tm->asGapTotalAmount / tm->asTotalVolume;
            /*
            //计算持仓平均gap值
            double tmpAmount = 0;
            int tmpVolume = 0;
            for (list<TradeInfo*>::iterator ltiIt = asList->begin(); ltiIt != asList->end();ltiIt++) {
                TradeInfo* asTi = *ltiIt;
                tmpAmount += asTi->realOpenGap;
                tmpVolume += asTi->activeVolume;
            }
            tm->asGapTotalAmount = tmpAmount;
            tm->asTotalVolume = tmpVolume;
            tm->asMeanGap = tm->asGapTotalAmount / tm->asTotalVolume;*/
            LOG(INFO) << ins_com_key + ",deleteArbitrageOrders,正套持仓平均值=" + boost::lexical_cast<string>(tmpmean);
        } else {
            LOG(ERROR) << "deleteArbitrageOrders,alreadyTradeMapAS无法查找到" + ins_com_key + "的持仓信息";
            return;
        }

    } else if (orderType == "ds") {
        //close option
        if(closeID == ""){
            LOG(ERROR) << "ERROR:ds closeid is null,something is wrong!";
        }else{
            LOG(INFO) << "ds close ,dsOpenMetric change from " + boost::lexical_cast<string>(tmpGap->dsOpenMetric) + " to " + boost::lexical_cast<string>(tmpGap->dsOpenMetric - openTick*priceTick);
            tmpGap->dsOpenMetric -= openTick*priceTick;
            bool thingFound = false;
            for(list<WaitForCloseInfo*>::iterator wfcIT = allDSTradeList.begin();wfcIT != allDSTradeList.end();){
                wfcInfo = *wfcIT;
                if(wfcInfo->closeID == closeID){//delete
                    LOG(INFO) << "ds stop loss or profit list found,will be ereased!!";
                    wfcIT = allDSTradeList.erase(wfcIT);
                    thingFound = true;
                    break;
                }else{
                    wfcIT ++;
                }
            }
            if(!thingFound){
                LOG(ERROR) << "ERROR:ds can't find closeID= " + closeID + " from allDSTradeList!";
            }
        }
        unordered_map<string, list<TradeInfo*>*>::iterator dsATMIt = alreadyTradeMapDS.find(ins_com_key);
        if (dsATMIt != alreadyTradeMapDS.end()) {
            list<TradeInfo*>* dsList = dsATMIt->second;
            delTinfo = dsList->front();
            dsList->pop_front();
            LOG(INFO) << "allDSTradeList size is " + boost::lexical_cast<string>(allDSTradeList.size()) + ";alreadyTradeMapDS size is " + boost::lexical_cast<string>(dsList->size());
            //计算持仓平均gap值
            //计算持仓平均gap值
            cout << "delTinfo->realOpenGap=" << boost::lexical_cast<string>(delTinfo->realOpenGap)<<";" <<endl;
            cout << "delTinfo->activeVolume"<< boost::lexical_cast<string>(delTinfo->activeVolume)<<";" <<endl;
            cout << "tm->dsGapTotalAmount"<< boost::lexical_cast<string>(tm->dsGapTotalAmount)<<";" <<endl;
            tm->dsGapTotalAmount -= delTinfo->realOpenGap*delTinfo->activeVolume;
            tm->dsTotalVolume -= delTinfo->activeVolume;
            double tmpmean = tm->dsGapTotalAmount / tm->dsTotalVolume;
            //tm->dsMeanGap = tm->dsGapTotalAmount / tm->dsTotalVolume;
            /*
            double tmpAmount = 0;
            int tmpVolume = 0;
            for (list<TradeInfo*>::iterator ltiIt = dsList->begin(); ltiIt != dsList->end(); ltiIt++) {
                TradeInfo* dsTi = *ltiIt;
                tmpAmount += dsTi->realOpenGap;
                tmpVolume += dsTi->activeVolume;
            }
            tm->dsGapTotalAmount = tmpAmount;
            tm->dsTotalVolume = tmpVolume;
            tm->dsMeanGap = tm->dsGapTotalAmount / tm->dsTotalVolume;*/
            LOG(INFO) << ins_com_key + ",deleteArbitrageOrders,反套持仓平均值=" + boost::lexical_cast<string>(tmpmean);
        } else {
            LOG(ERROR) << "deleteArbitrageOrders,alreadyTradeMapDS无法查找到" + ins_com_key + "的持仓信息";
            return;
        }
    }
    /*
    //添加到alreadyTradeMap
    unordered_map<string, list<TradeInfo*>*>::iterator tmpit = alreadyTradeMap.find(ins_com_key);
    if (tmpit == alreadyTradeMap.end()) {//新建
    list<TradeInfo*>* tmplist = new list<TradeInfo*>();
    tmplist->push_back(info);
    alreadyTradeMap[ins_com_key] = tmplist;
    } else {
    list<TradeInfo*>* tmplist = tmpit->second;
    tmplist->push_back(info);
    }*/
    //LOG(INFO) << "当前等待套利单信息：" + getAllAlreadyTradedInfo();
    //价格差初始化
    if (delTinfo->notActiveInstrumentid > delTinfo->activeInstrumentid) {//Key按照近月-远月的格式。不活跃合约为远月合约
        delTinfo->realCloseGap = info->activeRealClosePrice - info->notActiveRealClosePrice;
    } else {
        delTinfo->realCloseGap = info->notActiveRealClosePrice - info->activeRealClosePrice;
    }
    wfcInfo->status = "1";
    wfcInfo->closeGap = delTinfo->realCloseGap;
    LogMsg *logmsg = new LogMsg();
    cout << "delete" <<endl;
    string msg = "businessType=wtm_5;" + getWaitForCloseInfoDetail(wfcInfo);
    logmsg->setMsg(msg);
    networkTradeQueue.push(logmsg);

    delTinfo->tradingDay = info->tradingDay;
    delTinfo->tradeTime = info->tradeTime;
    string addData = "businessType=wtm_2;";//删除数据库中套利单组合
    addData.append(getArbDetail(delTinfo));
    logmsg = new LogMsg();
    logmsg->setMsg(addData);
    networkTradeQueue.push(logmsg);
    LOG(INFO) << "删除等待止盈套利组合单发送到保存进程：" + addData;

    computeGapRange();
    /*
    string storagePrice = "businessType=wtm_10;insComKey=" + ins_com_key + ";maxGap=" + boost::lexical_cast<string>(tmpGap->maxGap) + ";minGap=" + boost::lexical_cast<string>(tmpGap->minGap) + ";profitGap=" +
        boost::lexical_cast<string>(tmpGap->profitGap) + ";systemID=" + tmpGap->systemID;
    logmsg = new LogMsg();
    logmsg->setMsg(storagePrice);
    networkTradeQueue.push(logmsg);*/
}
void computeGapRange(){
    double asMinGap;
    double asMaxGap;
    double dsMinGap;
    double dsMaxGap;
    bool asBegin = true;
    bool dsBegin = true;
    for(list<WaitForCloseInfo*>::iterator it = allASTradeList.begin();it != allASTradeList.end();it ++){
        WaitForCloseInfo* wfcInfo = *it;
        if(asBegin){
           asMinGap = wfcInfo->stopLossGap;
           asMaxGap = wfcInfo->stopProfitGap;
           asBegin = false;
        }else{
            if(wfcInfo->stopLossGap < asMinGap){
                asMinGap = wfcInfo->stopLossGap;
            }
            if(wfcInfo->stopProfitGap > asMaxGap){
                asMaxGap = wfcInfo->stopProfitGap;
            }
        }
    }

    for(list<WaitForCloseInfo*>::iterator it = allDSTradeList.begin();it != allDSTradeList.end();it ++){
        WaitForCloseInfo* wfcInfo = *it;
        if(dsBegin){
           dsMinGap = wfcInfo->stopLossGap;
           dsMaxGap = wfcInfo->stopProfitGap;
           dsBegin = false;
        }else{
            if(wfcInfo->stopLossGap < dsMinGap){
                dsMinGap = wfcInfo->stopLossGap;
            }
            if(wfcInfo->stopProfitGap > dsMaxGap){
                dsMaxGap = wfcInfo->stopProfitGap;
            }
        }
    }
    if(!asBegin){
        asTradeListMinGap = asMinGap;
        asTradeListMaxGap = asMaxGap;
    }
    if(!dsBegin){
        dsTradeListMinGap = dsMinGap;
        dsTradeListMaxGap = dsMaxGap;
    }
    LOG(INFO) << "as range is in [" + boost::lexical_cast<string>(asTradeListMinGap) + "," + boost::lexical_cast<string>(asTradeListMaxGap)+"]";
    LOG(INFO) << "ds range is in [" + boost::lexical_cast<string>(dsTradeListMinGap) + "," + boost::lexical_cast<string>(dsTradeListMaxGap)+"]";
}

string getWaitForCloseInfoDetail(WaitForCloseInfo* wfcInfo){
    string str = "closeID=" + boost::lexical_cast<string>(wfcInfo->closeID) + ";";
    str = str + "stopProfitGap=" + boost::lexical_cast<string>(wfcInfo->stopProfitGap) + ";";
    str = str + "stopLossGap=" + boost::lexical_cast<string>(wfcInfo->stopLossGap) + ";";
    str = str + "openGap=" + boost::lexical_cast<string>(wfcInfo->openGap) + ";";
    str = str + "closeGap=" + boost::lexical_cast<string>(wfcInfo->closeGap) + ";";
    str = str + "ginGap=" + boost::lexical_cast<string>(wfcInfo->ginGap) + ";";
    str = str + "updateTime=" + wfcInfo->updateTime + ";";
    str = str + "orderType=" + wfcInfo->orderType + ";";
    str = str + "insComKey=" + wfcInfo->insComKey + ";";
    str = str + "status=" + wfcInfo->status + ";";
    str = str + "systemID=" + wfcInfo->systemID + ";";
    return str;
}
string generateHappyID(TradeInfo* info) {
    return  boost::lexical_cast<string>(info->notActiveClientOrderToken) + boost::lexical_cast<string>(info->activeClientOrderToken) + boost::lexical_cast<string>(USER_ID);
}
void changeAlreadyTradedOrderStatus(TradeInfo* info, string status) {
    string orderType = info->orderType;
    if (orderType == "as") {
        string insComKey = getComInstrumentKey(info->activeInstrumentid, info->notActiveInstrumentid);
        unordered_map<string, list<TradeInfo*>*>::iterator it = alreadyTradeMapAS.find(insComKey);
        if (it != alreadyTradeMapAS.end()) {
            list<TradeInfo*>* tradelist = it->second;
            bool isGet = false;
            for (list<TradeInfo*>::iterator lit = tradelist->begin(); lit != tradelist->end(); lit++) {
                TradeInfo* tmpinfo = *lit;
                if (tmpinfo->happyID == info->happyID) {//修改报单状态
                    LOG(INFO) << "已经成交等待止盈的组合单状态由" + tmpinfo->stopProfitStatus + ",修改成" + status;
                    tmpinfo->stopProfitStatus = status;
                    isGet = true;
                    break;
                }
            }
            if (!isGet) {
                LOG(ERROR) << "未查找到等待止盈的组合单:" + getArbDetail(info);
            }
        } else {
            LOG(ERROR) << "未查找到等待止盈的组合单:" + getArbDetail(info);
        }
    } else if (orderType == "ds") {
        string insComKey = getComInstrumentKey(info->activeInstrumentid, info->notActiveInstrumentid);
        unordered_map<string, list<TradeInfo*>*>::iterator it = alreadyTradeMapDS.find(insComKey);
        if (it != alreadyTradeMapDS.end()) {
            list<TradeInfo*>* tradelist = it->second;
            bool isGet = false;
            for (list<TradeInfo*>::iterator lit = tradelist->begin(); lit != tradelist->end(); lit++) {
                TradeInfo* tmpinfo = *lit;
                if (tmpinfo->happyID == info->happyID) {//修改报单状态
                    LOG(INFO) << "已经成交等待止盈的组合单状态由" + tmpinfo->stopProfitStatus + ",修改成" + status;
                    tmpinfo->stopProfitStatus = status;
                    isGet = true;
                    break;
                }
            }
            if (!isGet) {
                LOG(ERROR) << "未查找到等待止盈的组合单:" + getArbDetail(info);
            }
        } else {
            LOG(ERROR) << "未查找到等待止盈的组合单:" + getArbDetail(info);
        }
    }


}
void initGapPriceData(list<string> comOrdersList) {
    /************************************************************************/
    /* 每个字段，按照=分隔符进行分割                                        */
    /************************************************************************/
    try {
        string insComKey = "";
        double gapPrice = 0;
        double asOpenGap = 0;
        double asCloseGap = 0;
        double dsOpenGap = 0;
        double dsCloseGap = 0;
        for (list<string>::iterator beg = comOrdersList.begin(); beg != comOrdersList.end(); beg++) {
            string tmpstr = *beg;
            vector<string> vec = split(tmpstr, "=");
            if ("insComKey" == vec[0]) {
                insComKey = vec[1];
            }
            if ("gapPrice" == vec[0]) {
                gapPrice = boost::lexical_cast<double>(vec[1]);
            }
            if ("asOpenGap" == vec[0]) {
                asOpenGap = boost::lexical_cast<double>(vec[1]);
            }
            if ("asCloseGap" == vec[0]) {
                asCloseGap = boost::lexical_cast<double>(vec[1]);
            }
            if ("dsOpenGap" == vec[0]) {
                dsOpenGap = boost::lexical_cast<double>(vec[1]);
            }
            if ("dsCloseGap" == vec[0]) {
                dsCloseGap = boost::lexical_cast<double>(vec[1]);
            }
        }
        TechMetric* tm;
        list<PriceGapMarketData*>* pgDataSeq;
        unordered_map<string, TechMetric*>::iterator it = techMetricMap.find(insComKey);
        if (it == techMetricMap.end()) {
            tm = new TechMetric();
            pgDataSeq = new list<PriceGapMarketData*>();
            tm->pgDataSeq = pgDataSeq;
            techMetricMap[insComKey] = tm;
        } else {
            tm = it->second;
            pgDataSeq = tm->pgDataSeq;
        }
        PriceGapMarketData* pgmd = new PriceGapMarketData();
        pgmd->insComKey = insComKey;
        pgmd->gapPrice = gapPrice;
        pgmd->asOpenGap = asOpenGap;
        pgmd->asCloseGap = asCloseGap;
        pgmd->dsOpenGap = dsOpenGap;
        pgmd->dsCloseGap = dsCloseGap;
        pgDataSeq->emplace_back(pgmd);
    }
    catch (const runtime_error &re) {
        cerr << re.what() << endl;
    }
    catch (exception* e) {
        cerr << e->what() << endl;
        LogMsg *logmsg = new LogMsg();
        logmsg->setMsg(e->what());
        logqueue.push(logmsg);
    }
}
void initArbgComOrders(list<string> comOrdersList) {
    /************************************************************************/
    /* 每个字段，按照=分隔符进行分割                                        */
    /************************************************************************/
    try {
        TradeInfo* tradeInfo = new TradeInfo();
        for (list<string>::iterator beg = comOrdersList.begin(); beg != comOrdersList.end(); beg++) {
            string tmpstr = *beg;
            vector<string> vec = split(tmpstr, "=");
            if ("insComKey" == vec[0]) {
                tradeInfo->insComKey = vec[1];
            }
            if ("systemID" == vec[0]) {
                tradeInfo->systemID = vec[1];
            }
            if ("hopeOpenGap" == vec[0]) {
                tradeInfo->hopeOpenGap = boost::lexical_cast<double>(vec[1]);
            }
            if ("realOpenGap" == vec[0]) {
                tradeInfo->realOpenGap = boost::lexical_cast<double>(vec[1]);
            }
            if ("hopeCloseGap" == vec[0]) {
                tradeInfo->hopeCloseGap = boost::lexical_cast<double>(vec[1]);
            }
            if ("orderType" == vec[0]) {
                tradeInfo->orderType =vec[1];
            }
            if ("happyID" == vec[0]) {
                tradeInfo->happyID =vec[1];
            }
            if ("sessionID" == vec[0]) {
                tradeInfo->sessionID = boost::lexical_cast<int>(vec[1]);
            }
            if ("frontID" == vec[0]) {
                tradeInfo->frontID = boost::lexical_cast<int>(vec[1]);
            }
            if ("notActiveInstrumentid" == vec[0]) {
                tradeInfo->notActiveInstrumentid = vec[1];
            }
            if ("notActiveDirection" == vec[0]) {
                tradeInfo->notActiveDirection = vec[1];
            }
            if ("notActiveOrderInsertPrice" == vec[0]) {
                tradeInfo->notActiveOrderInsertPrice = boost::lexical_cast<double>(vec[1]);
            }
            if ("notActiveHedgeFlag" == vec[0]) {
                tradeInfo->notActiveHedgeFlag = vec[1];
            }
            if ("notActiveOrderRef" == vec[0]) {
                tradeInfo->notActiveOrderRef = vec[1];
            }
            if ("notActiveVolume" == vec[0]) {
                tradeInfo->notActiveVolume = boost::lexical_cast<int>(vec[1]);
            }
            if ("notActiveOrderSysID" == vec[0]) {
                tradeInfo->notActiveOrderSysID = vec[1];
            }
            //活跃合约信息
            if ("activeInstrumentid" == vec[0]) {
                tradeInfo->activeInstrumentid =vec[1];
            }
            if ("activeDirection" == vec[0]) {
                tradeInfo->activeDirection = vec[1];
            }
            if ("activeOrderInsertPrice" == vec[0]) {
                tradeInfo->activeOrderInsertPrice = boost::lexical_cast<double>(vec[1]);
            }
            if ("activeHedgeFlag" == vec[0]) {
                tradeInfo->activeHedgeFlag = vec[1];
            }
            if ("activeOffsetFlag" == vec[0]) {
                tradeInfo->activeOffsetFlag = vec[1];
            }
            if ("activeOrderRef" == vec[0]) {
                tradeInfo->activeOrderRef = vec[1];
            }
            if ("activeVolume" == vec[0]) {
                tradeInfo->activeVolume = boost::lexical_cast<int>(vec[1]);
            }
            if ("activeOrderSysID" == vec[0]) {
                tradeInfo->activeOrderSysID = vec[1];
                //strcpy(tradeInfo->activeOrderSysID, vec[1].c_str());
            }
        }
        boost::recursive_mutex::scoped_lock SLock4(alreadyTrade_mtx);//锁定
        //保存到unordered_map<string, list<TradeInfo*>*> alreadyTradeMap;			//保存已成交套利单信息
        if (tradeInfo->orderType == "as") {
            unordered_map<string, list<TradeInfo*>*>::iterator it = alreadyTradeMapAS.find(tradeInfo->insComKey);
            if (it == alreadyTradeMapAS.end()) {//系统启动初始化,暂时没有成交，列表为空
                list<TradeInfo*>* tradeList = new list<TradeInfo*>();
                tradeList->push_back(tradeInfo);
                alreadyTradeMapAS[tradeInfo->insComKey] = tradeList;
                LOG(INFO) << "存放等待止盈套利组合单成功.当前等待止盈组合数量=" + boost::lexical_cast<string>(tradeList->size()) + "数据库初始化报单=" + getArbDetail(tradeInfo);
            } else { //已经存在列表的情况
                list<TradeInfo*>* tradeList = it->second;
                bool isFind = false;
                for (list<TradeInfo*>::iterator tlIt = tradeList->begin(); tlIt != tradeList->end(); tlIt++) {
                    TradeInfo* tmpinfo = *tlIt;
                    if (tmpinfo->happyID == tradeInfo->happyID) {//存在相同的套利组合单，那么不操作
                        isFind = true;
                        LOG(ERROR) << "系统中已经存在相同的等待止盈套利组合单.当前等待止盈组合数量=" + boost::lexical_cast<string>(tradeList->size()) + "系统中的报单=" + getArbDetail(tmpinfo) + ";数据库初始化报单=" + getArbDetail(tradeInfo);
                        break;
                    }
                }
                if (!isFind) {//没有初始化，则放入队列
                    tradeList->push_back(tradeInfo);
                    LOG(INFO) << "存放等待止盈套利组合单成功.当前等待止盈组合数量=" + boost::lexical_cast<string>(tradeList->size()) + "数据库初始化报单=" + getArbDetail(tradeInfo);
                }
            }
        } else if (tradeInfo->orderType == "ds") {
            unordered_map<string, list<TradeInfo*>*>::iterator it = alreadyTradeMapDS.find(tradeInfo->insComKey);
            if (it == alreadyTradeMapDS.end()) {//系统启动初始化,暂时没有成交，列表为空
                list<TradeInfo*>* tradeList = new list<TradeInfo*>();
                tradeList->push_back(tradeInfo);
                alreadyTradeMapDS[tradeInfo->insComKey] = tradeList;
                LOG(INFO) << "存放等待止盈套利组合单成功.当前等待止盈组合数量=" + boost::lexical_cast<string>(tradeList->size()) + "数据库初始化报单=" + getArbDetail(tradeInfo);
            } else { //已经存在列表的情况
                list<TradeInfo*>* tradeList = it->second;
                bool isFind = false;
                for (list<TradeInfo*>::iterator tlIt = tradeList->begin(); tlIt != tradeList->end(); tlIt++) {
                    TradeInfo* tmpinfo = *tlIt;
                    if (tmpinfo->happyID == tradeInfo->happyID) {//存在相同的套利组合单，那么不操作
                        isFind = true;
                        LOG(ERROR) << "系统中已经存在相同的等待止盈套利组合单.当前等待止盈组合数量=" + boost::lexical_cast<string>(tradeList->size()) + "系统中的报单=" + getArbDetail(tmpinfo) + ";数据库初始化报单=" + getArbDetail(tradeInfo);
                        break;
                    }
                }
                if (!isFind) {//没有初始化，则放入队列
                    tradeList->push_back(tradeInfo);
                    LOG(INFO) << "存放等待止盈套利组合单成功.当前等待止盈组合数量=" + boost::lexical_cast<string>(tradeList->size()) + "数据库初始化报单=" + getArbDetail(tradeInfo);
                }
            }
        }

    }catch (const runtime_error &re) {
        cerr << re.what() << endl;
    }catch (exception* e){
        cerr << e->what() << endl;
        LogMsg *logmsg = new LogMsg();
        logmsg->setMsg(e->what());
        logqueue.push(logmsg);
    }
}
void completeInitArbgComOrders() {
    boost::recursive_mutex::scoped_lock SLock(pstDetail_mtx);
    boost::recursive_mutex::scoped_lock SLock2(alreadyTrade_mtx);
    //正套
    for (unordered_map<string, list<TradeInfo*>*>::iterator dbIt = alreadyTradeMapAS.begin(); dbIt != alreadyTradeMapAS.end(); dbIt++) {
        //持仓明细
        string instrumentID = dbIt->first;
        list<TradeInfo*>* dbList = dbIt->second;
        int storgedInDB = dbList->size();
        //check from exchange
        PriceGap* pg = getPriceGap(instrumentID);
        if(!pg){
            string msg = "ERROR!!!!正套初始化的" + instrumentID + "组合持仓数据量=" + boost::lexical_cast<string>(storgedInDB) + ",is not matched from exchange = 0";
            LOG(ERROR) <<msg;
            //cout<< msg<< endl;
        }else if(storgedInDB == pg->arbComVolumeAS){//match
            string msg = "OK!正套初始化的" + instrumentID + "组合持仓数据量=" + boost::lexical_cast<string>(storgedInDB) + ",matched from exchange = " + boost::lexical_cast<string>(pg->arbComVolumeAS);
            LOG(ERROR) <<msg;
            //cout<< msg<< endl;
        }else{
            string msg = "ERROR!!!!正套初始化的" + instrumentID + "组合持仓数据量=" + boost::lexical_cast<string>(storgedInDB) + ",is not matched from exchange = " + boost::lexical_cast<string>(pg->arbComVolumeAS);
            LOG(ERROR) <<msg;
            //cout<< msg<< endl;
        }

    }
    //反套
    for (unordered_map<string, list<TradeInfo*>*>::iterator dbIt = alreadyTradeMapDS.begin(); dbIt != alreadyTradeMapDS.end(); dbIt++) {
        //持仓明细
        string instrumentID = dbIt->first;
        list<TradeInfo*>* dbList = dbIt->second;
        //LOG(INFO) << "反套初始化的" + instrumentID + "组合持仓数据量=" + boost::lexical_cast<string>(dbList->size());
        //cout << "反套初始化的" + instrumentID + "组合持仓数据量=" + boost::lexical_cast<string>(dbList->size()) << endl;
        int storgedInDB = dbList->size();
        //check from exchange
        PriceGap* pg = getPriceGap(instrumentID);
        if(!pg){
            string msg = "ERROR!!!!反套初始化的" + instrumentID + "组合持仓数据量=" + boost::lexical_cast<string>(storgedInDB) + ",is not matched from exchange = 0";
            LOG(ERROR) <<msg;
            //cout<< msg<< endl;
        }else if(storgedInDB == pg->arbComVolumeDS){//match
            string msg = "OK!反套初始化的" + instrumentID + "组合持仓数据量=" + boost::lexical_cast<string>(storgedInDB) + ",matched from exchange = " + boost::lexical_cast<string>(pg->arbComVolumeDS);
            LOG(ERROR) <<msg;
            //cout<< msg<< endl;
        }else{
            string msg = "ERROR!!!!反套初始化的" + instrumentID + "组合持仓数据量=" + boost::lexical_cast<string>(storgedInDB) + ",is not matched from exchange = " + boost::lexical_cast<string>(pg->arbComVolumeDS);
            LOG(ERROR) <<msg;
            //cout<< msg<< endl;
        }
    }
    for (unordered_map<string, list<HoldPositionDetail*>*>::iterator realIt = positionDetailMap.begin(); realIt != positionDetailMap.end();realIt++) {
        //持仓明细
        string instrumentID = realIt->first;
        list<HoldPositionDetail*>* realList = realIt->second;
        LOG(INFO) << "查询的" + instrumentID + "持仓数据量=" + boost::lexical_cast<string>(realList->size());
        cout << "查询的" + instrumentID + "持仓数据量=" + boost::lexical_cast<string>(realList->size()) << endl;
        /*
        unordered_map<string, list<TradeInfo*>*>::iterator dbIt = alreadyTradeMap.find(instrumentID);
        if (dbIt == alreadyTradeMap.end()) {
            LOG(INFO) << "#################初始化数据错误#################";
            LOG(INFO) << "无法从数据库中初始化等待止盈持仓组合数据";
        } else {

            list<TradeInfo*>* dbList = dbIt->second;
            if (realList->size() > dbList->size()) {
                LOG(INFO) << "查询的持仓数据=" + boost::lexical_cast<string>(realList->size()) + "大于数据库保存数据=" + boost::lexical_cast<string>(dbList->size()) + "，请确认。";
            } else if (realList->size() < dbList->size()) {
                LOG(INFO) << "查询的持仓数据=" + boost::lexical_cast<string>(realList->size()) + "小于数据库保存数据=" + boost::lexical_cast<string>(dbList->size()) + "，请确认。";
            } else {
                LOG(INFO) << "合约" + instrumentID + "持仓初始化正确。";
            }*/
            /*
            list<HoldPositionDetail*>* realList = realIt->second;
            list<HoldPositionDetail*>* dbList = dbIt->second;
            int pdCount = 0;
            for (list<HoldPositionDetail*>::iterator tmpRealIt = realList->begin(); tmpRealIt != realList->end();) {
                HoldPositionDetail* tmpRealOrder = *tmpRealIt;
                bool isFind = false;
                for (list<HoldPositionDetail*>::iterator tmpDBIt = dbList->begin(); tmpDBIt != dbList->end();) {
                    HoldPositionDetail* tmpDBOrder = *tmpDBIt;
                    if (tmpDBOrder->tradeID == tmpRealOrder->tradeID) {//找对对应的配对持仓，删除
                        pdCount++;
                        tmpDBIt = dbList->erase(tmpDBIt);
                        isFind = true;
                        break;
                    } else {
                        tmpDBIt++;
                    }
                }
                if (isFind) {//查找到，删除查询的持仓合约
                    tmpRealIt = realList->erase(tmpRealIt);
                } else {
                    tmpRealIt++;
                }
            }
            LOG(INFO) << "合约" + instrumentID + "清理掉持仓纪录" + boost::lexical_cast<string>(pdCount) +"笔";
            if (dbList->size() > 0) {
                LOG(INFO) << "合约" + instrumentID + "的数据库记录有误，无法完全清理。剩余持仓纪录：";
                for (list<HoldPositionDetail*>::iterator eit = dbList->begin(); eit != dbList->end();eit++) {
                    LOG(INFO) << getDBPositionDetail(*eit);
                }
            }
            if (realList->size() > 0) {
                LOG(INFO) << "合约" + instrumentID + "持仓明细查询记录无法完全清理。剩余持仓纪录：";
                for (list<HoldPositionDetail*>::iterator eit = realList->begin(); eit != realList->end(); eit++) {
                    LOG(INFO) << getDBPositionDetail(*eit);
                }
            }
            if (dbList->size() ==0 && realList->size() == 0) {
                LOG(INFO) << "合约" + instrumentID + "持仓初始化正确。";
            }*/
        //}
    }
    for (unordered_map<string, TechMetric*>::iterator tmit = techMetricMap.begin(); tmit != techMetricMap.end();tmit++) {
        TechMetric* tm = tmit->second;

        list<PriceGapMarketData*>* pgDataSeq = tm->pgDataSeq;
        double totalGapPrice = 0;
        double asTotalGap = 0;
        double dsTotalGap = 0;
        for (list<PriceGapMarketData*>::iterator tmpPGIt = pgDataSeq->begin(); tmpPGIt != pgDataSeq->end(); tmpPGIt++) {
            PriceGapMarketData* tmpComdata = *tmpPGIt;
            double tmpGapPrice = tmpComdata->gapPrice;
            double tmpASGap = tmpComdata->asOpenGap;//均值的计算，统一使用一种计算方式
            double tmpDSGap = tmpComdata->dsOpenGap;
            asTotalGap += tmpASGap;
            dsTotalGap += tmpDSGap;
            totalGapPrice += tmpGapPrice;
        }
        double maGapPrice = totalGapPrice / pgDataSeq->size();
        double maGapPriceAS = asTotalGap / pgDataSeq->size();
        double maGapPriceDS = dsTotalGap / pgDataSeq->size();
        string tmpstr = tmit->first +",maGapPrice=" + boost::lexical_cast<string>(maGapPrice);
        cout << tmpstr << endl;
        LOG(INFO) << tmpstr;
    }
    startStrategy();
}
void startStrategy() {
    int isbegin = 0;
    cout << "是否启动策略程序?0 否，1是" << endl;
    cin >> isbegin;
    //isbeginmk = 1;
    if (isbegin == 1) {
        start_process = 1;
        cout<< "start successfully!start_process=" + boost::lexical_cast<string>(start_process);
    }else{
        cout<< "start failed!start_process=" + boost::lexical_cast<string>(start_process);
    }
}
PriceGap* getPriceGap(string insComKey) {
    unordered_map<string, PriceGap*>::iterator instr_price_gap_it = instr_price_gap.find(insComKey);//找到配对合约的预警值																						   //double price_gap_pre_set = 0;
    //PriceGap* gapinfo;
    if (instr_price_gap_it != instr_price_gap.end()) {
        return instr_price_gap_it->second;//找到预设价差
    } else {
        LOG(ERROR) << "没有找到预设价差:" + insComKey;
        return NULL;
    }
}
void setPriceGap(string insComKey, PriceGap* gap) {
    //boost::recursive_mutex::scoped_lock SLock(priceGap_mtx);
    unordered_map<string, PriceGap*>::iterator instr_price_gap_it = instr_price_gap.find(insComKey);//找到配对合约的预警值																						   //double price_gap_pre_set = 0;																	//PriceGap* gapinfo;
    if (instr_price_gap_it != instr_price_gap.end()) {
        PriceGap* orgGap = instr_price_gap_it->second;//找到预设价差
        if (gap->maxGap != 0) {
            LOG(INFO) << "ins_com_key组合开仓预警值，由" + boost::lexical_cast<string>(orgGap->maxGap) + "调整为" + boost::lexical_cast<string>(gap->maxGap);
            orgGap->maxGap = gap->maxGap;
        }
        if (gap->minGap != 0) {
            orgGap->minGap = gap->minGap;
        }
        if (gap->profitGap != 0) {
            LOG(INFO) << "ins_com_key组合平仓预警值，由" + boost::lexical_cast<string>(orgGap->profitGap) + "调整为" + boost::lexical_cast<string>(gap->profitGap);
            orgGap->profitGap = gap->profitGap;
        }
    } else {
        LOG(INFO) << "######################setPriceGap无法查找到ins_com_key" + insComKey + "的预警值";

    }
}
bool asIsCanBeTrade(PriceGapMarketData* mkdata,double gapPrice){
    for(list<WaitForCloseInfo*>::iterator tlIt = allASTradeList.begin();tlIt != allASTradeList.end();tlIt ++){
        WaitForCloseInfo* closeInfo = *tlIt;
        if(mkdata->insComKey != closeInfo->insComKey){
            continue;
        }
        mkdata->closeID = closeInfo->closeID;
        if(closeInfo->stopProfitGap <= gapPrice){//stop profit
            if(isLogout){
                LOG(INFO) << "正套止盈,gapPrice=" + boost::lexical_cast<string>(gapPrice) + ">=大于" + boost::lexical_cast<string>(closeInfo->stopProfitGap) ;
            }
            return true;
        }/*else if(closeInfo->stopLossGap >= gapPrice){//stop loss
            if(isLogout){
                LOG(INFO) << "正套止损。gapPrice=" + boost::lexical_cast<string>(gapPrice) + "<=，止损点设为= " + boost::lexical_cast<string>(closeInfo->stopLossGap);
            }
            return true;
        }*/
    }
    LOG(INFO) << "as has hold position =" + boost::lexical_cast<string>(allASTradeList.size());
    return false;
}
bool dsIsCanBeTrade(PriceGapMarketData* mkdata,double gapPrice){
    for(list<WaitForCloseInfo*>::iterator tlIt = allDSTradeList.begin();tlIt != allDSTradeList.end();tlIt ++){
        WaitForCloseInfo* closeInfo = *tlIt;
        if(mkdata->insComKey != closeInfo->insComKey){
            continue;
        }
        mkdata->closeID = closeInfo->closeID;
        if(closeInfo->stopProfitGap >= gapPrice){//stop profit
            if(isLogout){
                LOG(INFO) << "反套止盈,gapPrice=" + boost::lexical_cast<string>(gapPrice) + ">=大于" + boost::lexical_cast<string>(closeInfo->stopProfitGap) ;
            }
            return true;
        }/*else if(closeInfo->stopLossGap <= gapPrice){//stop loss
            if(isLogout){
                LOG(INFO) << "反套止损。gapPrice=" + boost::lexical_cast<string>(gapPrice) + "<=，止损点设为= " + boost::lexical_cast<string>(closeInfo->stopLossGap);
            }
            return true;
        }*/
    }
    LOG(INFO) << "ds has hold position =" + boost::lexical_cast<string>(allDSTradeList.size());
    return false;
}
void processStrategyForSingleThread(PriceGapMarketData * mkDataGap) {
    try {
        //boost::posix_time::ptime HEARTBEAT = getCurrentTimeByBoost();
        //开始时间
        boost::posix_time::ptime startTime = getCurrentTimeByBoost();
        boost::posix_time::ptime endTime;
        string insComKey = (mkDataGap->insComKey);
        //获得技术指标
        unordered_map<string, TechMetric*>::iterator tmmIt = techMetricMap.find(insComKey);
        if (tmmIt == techMetricMap.end()) {
            LOG(ERROR) << "processStrategy:无法查找到InstrumentID=" + insComKey + "对应的技术指标";
            return ;
        }
        TechMetric* tm = tmmIt->second;
        string lastInstrumentID = mkDataGap->lastInstrumentID;//近月
        int lastVolume = mkDataGap->lastInsVolume;//当前合约成交量
        /*
        double fwdBidPrice = mkDataGap->forwardInsBidPrice;
        double fwdAskPrice = mkDataGap->forwardInsAskPrice;
        */
        int fwdVolume = mkDataGap->forwardInsVolume;
        double asOpenGap = mkDataGap->asOpenGap;
        double dsOpenGap = mkDataGap->dsOpenGap;
        double asCloseGap = mkDataGap->asCloseGap;
        double dsCloseGap = mkDataGap->dsCloseGap;

        if(isLogout){
            LOG(INFO) << insComKey + " tm->asMAGap=" + boost::lexical_cast<string>(tm->asMAGap) + ",asOpenGap=" + boost::lexical_cast<string>(asOpenGap) + ",dsOpenGap=" + boost::lexical_cast<string>(dsOpenGap)+ ",overMAGapTickNums=" + boost::lexical_cast<string>(tm->overMAGapTickNums) + ",downMAGapTickNums=" + boost::lexical_cast<string>(tm->downMAGapTickNums) +
                ",stopLossTickNums=" + boost::lexical_cast<string>(tm->stopLossTickNums);
        }
        if (tm->asMAGap == 0 || asOpenGap == 0 || dsOpenGap == 0) {//数据未开始接受
            return;
        }
        double priceTick = getPriceTick(lastInstrumentID);
        if (priceTick == 0) {
            LOG(ERROR) << "processStrategy:无法查找到InstrumentID=" + lastInstrumentID + "对应的TICK";
            return;
        }
        //LOG(ERROR) << "processStrategy 4";
        PriceGap* tmpPriceGap = getPriceGap(insComKey);
        //init open gap price
        if(tmpPriceGap->asOpenMetric == 100000){
            tmpPriceGap->asOpenMetric = mkDataGap->gapPrice;
            tmpPriceGap->dsOpenMetric = mkDataGap->gapPrice;
        }
        if(tmpPriceGap->arbComVolumeAS == 0 ){
            tmpPriceGap->asOpenMetric = mkDataGap->gapPrice;
        }else if(tmpPriceGap->arbComVolumeDS == 0){
            tmpPriceGap->dsOpenMetric = mkDataGap->gapPrice;
        }
        //double asOpenGapMetric = tm->asMAGap - tm->overMAGapTickNums*priceTick;//开仓阈值为均值之上overMAGapTickNums个tick.正套
       // double dsOpenGapMetric = tm->dsMAGap + tm->overMAGapTickNums*priceTick;//开仓阈值为均值之下overMAGapTickNums个tick.反套
        //double dsCloseGapMetric = tm->dsMAGap - (tm->downMAGapTickNums-1)*priceTick;//平仓阈值为均值之上overMAGapTickNums个tick.反套
        //double asCloseGapMetric = tm->asMAGap + (tm->downMAGapTickNums-1)*priceTick;//平仓阈值为均值之下overMAGapTickNums个tick.正套
        //double asOpenGapMetric = tm->maGap - tm->overMAGapTickNums*priceTick;//开仓阈值为均值之上overMAGapTickNums个tick.正套
        //double dsOpenGapMetric = tm->maGap + tm->overMAGapTickNums*priceTick;//开仓阈值为均值之下overMAGapTickNums个tick.反套
        //double dsCloseGapMetric = tm->maGap - (tm->downMAGapTickNums-1)*priceTick;//平仓阈值为均值之上overMAGapTickNums个tick.反套
        //double asCloseGapMetric = tm->maGap + (tm->downMAGapTickNums-1)*priceTick;//平仓阈值为均值之下overMAGapTickNums个tick.正套
        //double asHoldMeanGap = tm->asHoldMeanGap;//正套持仓均值
        //double dsHoldMeanGap = tm->dsHoldMeanGap;//反套持仓均值
        /*
        if(isLogout){
            LOG(INFO) << "asOpenGap=" + boost::lexical_cast<string>(asOpenGap) + ",asOpenGapMetric=" + boost::lexical_cast<string>(asOpenGapMetric) + ";dsOpenGap=" + boost::lexical_cast<string>(dsOpenGap) + ",dsOpenGapMetric=" + boost::lexical_cast<string>(dsOpenGapMetric) +
                "asCloseGap=" + boost::lexical_cast<string>(asCloseGap) + ",asCloseGapMetric=" + boost::lexical_cast<string>(asCloseGapMetric) + ";dsCloseGap=" + boost::lexical_cast<string>(dsCloseGap) + ",dsCloseGapMetric=" + boost::lexical_cast<string>(dsCloseGapMetric) +
                ".当前均值asMAGap=" + boost::lexical_cast<string>(tm->maGap) + ",dsMAGap=" + boost::lexical_cast<string>(tm->maGap) + ".持仓均值asHoldMeanGap=" + boost::lexical_cast<string>(asHoldMeanGap) + ",dsHoldMeanGap=" + boost::lexical_cast<string>(dsHoldMeanGap);
        }*/
        LOG(INFO) << "asCloseGap=" + boost::lexical_cast<string>(asCloseGap) + ",as range is in [" + boost::lexical_cast<string>(asTradeListMinGap) + "," + boost::lexical_cast<string>(asTradeListMaxGap)+"]";
        LOG(INFO) << "dsCloseGap=" + boost::lexical_cast<string>(dsCloseGap) + ",ds range is in [" + boost::lexical_cast<string>(dsTradeListMinGap) + "," + boost::lexical_cast<string>(dsTradeListMaxGap)+"]";

        //不存在下单信息，那么直接以市价单下单
        //一、统一近月-远月的合约价差进行比较;套利时候，买近月，卖远月
        //二、市价可以成交的，不必考虑canbetrade或者canbeclose
        if (fwdVolume >= lastVolume) {//远月合约为活跃合约,近月合约为非活跃合约.
            //近月不活跃,正套
            if (asOpenGap < tmpPriceGap->asOpenMetric && isOverTrade(insComKey, "as") && isOverTradeTwo(mkDataGap, "as") && isHoldPositionOverGap(tmpPriceGap,"as")) {//正套
                if(isLogout){
                    LOG(INFO) << "正套开仓,近月不活跃.asOpenGap=" + boost::lexical_cast<string>(asOpenGap) + "小于" + boost::lexical_cast<string>(tmpPriceGap->asOpenMetric)  + "," + mkDataGap->des;
                }

                openIntrestProcesserTwo(mkDataGap);
            } else if (dsOpenGap > tmpPriceGap->dsOpenMetric && isOverTrade(insComKey, "ds") && isOverTradeTwo(mkDataGap, "ds") && isHoldPositionOverGap(tmpPriceGap, "ds")) {//反套
                if(isLogout){
                    LOG(INFO) << "反套开仓,近月不活跃.dsOpenGap=" + boost::lexical_cast<string>(dsOpenGap) + "大于" + boost::lexical_cast<string>(tmpPriceGap->dsOpenMetric) + "," + mkDataGap->des;
                }

                openIntrestProcesserFour(mkDataGap);
            }
            ////##########################################
            //平仓处理
            //if (asHoldMeanGap - priceTick*tm->stopLossTickNums >= asCloseGap && canBeClose(insComKey, "as")) {//正套价差扩大到开仓均价的priceTick*stopLossTickNums之外，止损平仓
            if (asIsCanBeTrade(mkDataGap, asCloseGap) && canBeClose(insComKey, "as")) {
                stopProfitProcesserThree(mkDataGap);
            }
            if (dsIsCanBeTrade(mkDataGap, dsCloseGap) && canBeClose(insComKey, "ds")) {//反套价差继续缩小到止损线，止损平仓
                stopProfitProcesserFour(mkDataGap);
            }
            //#############################################
        } else {//远月合约为不活跃合约
                //远月不活跃,正套
            if (asOpenGap <= tmpPriceGap->asOpenMetric && isOverTrade(insComKey, "as") && isOverTradeTwo(mkDataGap, "as") && isHoldPositionOverGap(tmpPriceGap, "as")) {//正套
                if(isLogout){
                    LOG(INFO) << "正套开仓,远月不活跃.asOpenGap=" + boost::lexical_cast<string>(asOpenGap) + "小于" + boost::lexical_cast<string>(tmpPriceGap->asOpenMetric) + "," + mkDataGap->des;

                }
                openIntrestProcesserOne(mkDataGap);
            } else if (dsOpenGap >= tmpPriceGap->dsOpenMetric && isOverTrade(insComKey, "ds") && isOverTradeTwo(mkDataGap, "ds") && isHoldPositionOverGap(tmpPriceGap, "ds")) {//反套,//远月不活跃，反套
                if(isLogout){
                    LOG(INFO) << "反套开仓,远月不活跃.dsOpenGap=" + boost::lexical_cast<string>(dsOpenGap) + "大于" + boost::lexical_cast<string>(tmpPriceGap->dsOpenMetric) + "," + mkDataGap->des;

                }
                openIntrestProcesserThree(mkDataGap);
            }
            //###########################################
            //平仓处理
            if (asIsCanBeTrade(mkDataGap, asCloseGap) && canBeClose(insComKey, "as")) {
                //mkDataGap->closeID = *closeID;
                stopProfitProcesserOne(mkDataGap);
            }

            if (dsIsCanBeTrade(mkDataGap, dsCloseGap) && canBeClose(insComKey, "ds")) {//反套价差继续缩小到止损线，止损平仓
                //mkDataGap->closeID = *closeID;
                stopProfitProcesserTwo(mkDataGap);
            }
            //###########################################
        }
        //结束时间
        endTime = getCurrentTimeByBoost();
        int seconds = getTimeInterval(startTime, endTime, "t");
        //LOG(INFO) << "processStrategy,处理时长=" + boost::lexical_cast<string>(seconds);
        string stg = "businessType=wtm_4;insComKey=" + insComKey + ";" + "tradingDay=" + tradingDayT
            + ";" + "processTime=" + boost::lexical_cast<string>(seconds) + ";type=strategy;seq=" + boost::lexical_cast<string>(endTime)+";systemID=" + systemID;
        LogMsg* logmsg = new LogMsg();
        logmsg->setMsg(stg);
        networkTradeQueue.push(logmsg);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
void processStrategy() {
    cout << "启动进程processStrategy" << endl;
    //cout << boosttoolsnamespace::CBoostTools::gbktoutf8("启动进程") << endl;
    try {
        PriceGapMarketData * mkDataGap;
        boost::posix_time::ptime HEARTBEAT = getCurrentTimeByBoost();;
        while (1) {
            if (strategyQueue.empty()) {
                this_thread::yield();
            } else if (strategyQueue.pop(mkDataGap)) {
                //开始时间
                boost::posix_time::ptime startTime = getCurrentTimeByBoost();
                boost::posix_time::ptime endTime;
                //boost::recursive_mutex::scoped_lock SLock1(mkdata_mtx);//加锁处理
                boost::recursive_mutex::scoped_lock SLock1(willTrade_mtx);//加锁处理
                boost::recursive_mutex::scoped_lock SLock2(priceGap_mtx);//锁定
                boost::recursive_mutex::scoped_lock SLock3(stopProfit_mtx);//加锁处理
                boost::recursive_mutex::scoped_lock SLock4(pst_mtx);//加锁处理
                boost::recursive_mutex::scoped_lock SLock5(alreadyTrade_mtx);
                boost::recursive_mutex::scoped_lock SLock6(techMetric_mtx);
                string insComKey = (mkDataGap->insComKey);
                //获得技术指标
                unordered_map<string, TechMetric*>::iterator tmmIt = techMetricMap.find(insComKey);
                if (tmmIt == techMetricMap.end()) {
                    LOG(ERROR) << "processStrategy:无法查找到InstrumentID=" + insComKey + "对应的技术指标";
                    continue;
                }
                //LOG(ERROR) << "processStrategy 3";

                TechMetric* tm = tmmIt->second;

                int tradeItl = getTimeInterval(tm->tradeInterval,startTime,"s");
                if (tradeItl >= 20) {
                    LOG(ERROR) << insComKey + ",processStrategy work fine!";
                    tm->tradeInterval = startTime;
                } else {
                    //continue;
                }
                int heatBeatSecond = getTimeInterval(HEARTBEAT, startTime, "s");
                if (heatBeatSecond >= 20) {
                    LOG(ERROR) << "processStrategy work fine!";
                    HEARTBEAT = startTime;
                }
                int ticks;
                //LOG(ERROR) << "processStrategy 2";
                endTime = getCurrentTimeByBoost();
                ticks = getTimeInterval(startTime, endTime, "t");
                //LOG(INFO) << "processStrategy,获取锁处理时长=" + boost::lexical_cast<string>(ticks);
                string stg1 = "businessType=wtm_4;insComKey=" + insComKey + ";" + "tradingDay=" + tradingDayT
                    + ";" + "processTime=" + boost::lexical_cast<string>(ticks) + ";type=strategyGetLock;seq=" + boost::lexical_cast<string>(endTime);
                LogMsg *logmsg = new LogMsg();
                logmsg->setMsg(stg1);
                networkTradeQueue.push(logmsg);
                string info = getCurrentSystemTime() + " ";

                string lastInstrumentID = mkDataGap->lastInstrumentID;//近月
                string forwardInstrumentID = mkDataGap->forwardInstrumentID;
                double lastPriceTick = 0;
                double forwardPriceTick = 0;//配对最小变动
                                            //double currLastPrice = mkData->lastPrice;
                double lastBidPrice = mkDataGap->lastInsBidPrice;
                double lastAskPrice = mkDataGap->lastInsAskPrice;
                int lastVolume = mkDataGap->lastInsVolume;//当前合约成交量

                double fwdBidPrice = mkDataGap->forwardInsBidPrice;
                double fwdAskPrice = mkDataGap->forwardInsAskPrice;
                int fwdVolume = mkDataGap->forwardInsVolume;
                double asOpenGap = mkDataGap->asOpenGap;
                double dsOpenGap = mkDataGap->dsOpenGap;
                double asCloseGap = mkDataGap->asCloseGap;
                double dsCloseGap = mkDataGap->dsCloseGap;

                if(isLogout){
                    LOG(INFO) << insComKey + " tm->asMAGap=" + boost::lexical_cast<string>(tm->asMAGap) + ",asOpenGap=" + boost::lexical_cast<string>(asOpenGap) + ",dsOpenGap=" + boost::lexical_cast<string>(dsOpenGap)+ ",overMAGapTickNums=" + boost::lexical_cast<string>(tm->overMAGapTickNums) + ",downMAGapTickNums=" + boost::lexical_cast<string>(tm->downMAGapTickNums) +
                        ",stopLossTickNums=" + boost::lexical_cast<string>(tm->stopLossTickNums);
                }
                if (tm->asMAGap == 0 || asOpenGap == 0 || dsOpenGap == 0) {//数据未开始接受
                    continue;
                }
                double priceTick = getPriceTick(lastInstrumentID);
                if (priceTick == 0) {
                    LOG(ERROR) << "processStrategy:无法查找到InstrumentID=" + lastInstrumentID + "对应的TICK";
                    continue;
                }
                //LOG(ERROR) << "processStrategy 4";
                PriceGap* tmpPriceGap = getPriceGap(insComKey);
                double asOpenGapMetric = tm->asMAGap - tm->overMAGapTickNums*priceTick;//开仓阈值为均值之上overMAGapTickNums个tick.正套
                double dsOpenGapMetric = tm->dsMAGap + tm->overMAGapTickNums*priceTick;//开仓阈值为均值之下overMAGapTickNums个tick.反套
                double dsCloseGapMetric = tm->dsMAGap - (tm->downMAGapTickNums-1)*priceTick;//平仓阈值为均值之上overMAGapTickNums个tick.反套
                double asCloseGapMetric = tm->asMAGap + (tm->downMAGapTickNums-1)*priceTick;//平仓阈值为均值之下overMAGapTickNums个tick.正套
                //double asOpenGapMetric = tm->maGap - tm->overMAGapTickNums*priceTick;//开仓阈值为均值之上overMAGapTickNums个tick.正套
                //double dsOpenGapMetric = tm->maGap + tm->overMAGapTickNums*priceTick;//开仓阈值为均值之下overMAGapTickNums个tick.反套
                //double dsCloseGapMetric = tm->maGap - (tm->downMAGapTickNums-1)*priceTick;//平仓阈值为均值之上overMAGapTickNums个tick.反套
                //double asCloseGapMetric = tm->maGap + (tm->downMAGapTickNums-1)*priceTick;//平仓阈值为均值之下overMAGapTickNums个tick.正套
                double asHoldMeanGap = tm->asHoldMeanGap;//正套持仓均值
                double dsHoldMeanGap = tm->dsHoldMeanGap;//反套持仓均值
                if(isLogout){
                    LOG(INFO) << "asOpenGap=" + boost::lexical_cast<string>(asOpenGap) + ",asOpenGapMetric=" + boost::lexical_cast<string>(asOpenGapMetric) + ";dsOpenGap=" + boost::lexical_cast<string>(dsOpenGap) + ",dsOpenGapMetric=" + boost::lexical_cast<string>(dsOpenGapMetric) +
                        "asCloseGap=" + boost::lexical_cast<string>(asCloseGap) + ",asCloseGapMetric=" + boost::lexical_cast<string>(asCloseGapMetric) + ";dsCloseGap=" + boost::lexical_cast<string>(dsCloseGap) + ",dsCloseGapMetric=" + boost::lexical_cast<string>(dsCloseGapMetric) +
                        ".当前均值asMAGap=" + boost::lexical_cast<string>(tm->maGap) + ",dsMAGap=" + boost::lexical_cast<string>(tm->maGap) + ".持仓均值asHoldMeanGap=" + boost::lexical_cast<string>(asHoldMeanGap) + ",dsHoldMeanGap=" + boost::lexical_cast<string>(dsHoldMeanGap);
                }

                //不存在下单信息，那么直接以市价单下单
                //一、统一近月-远月的合约价差进行比较;套利时候，买近月，卖远月
                //二、市价可以成交的，不必考虑canbetrade或者canbeclose
                if (fwdVolume >= lastVolume) {//远月合约为活跃合约,近月合约为非活跃合约.
                    //近月不活跃,正套
                    if (asOpenGap <= asOpenGapMetric && isOverTrade(insComKey, "as") && isOverTradeTwo(mkDataGap, "as") && isHoldPositionOverGap(tmpPriceGap,"as")) {//正套
                        if(isLogout){
                            LOG(INFO) << "正套开仓,近月不活跃.asOpenGap=" + boost::lexical_cast<string>(asOpenGap) + "小于" + boost::lexical_cast<string>(asOpenGapMetric)  + "," + mkDataGap->des;
                        }

                        openIntrestProcesserTwo(mkDataGap);
                    } else if (dsOpenGap >= dsOpenGapMetric && isOverTrade(insComKey, "ds") && isOverTradeTwo(mkDataGap, "ds") && isHoldPositionOverGap(tmpPriceGap, "ds")) {//反套
                        if(isLogout){
                            LOG(INFO) << "反套开仓,近月不活跃.dsOpenGap=" + boost::lexical_cast<string>(dsOpenGap) + "大于" + boost::lexical_cast<string>(dsOpenGapMetric) + "," + mkDataGap->des;
                        }

                        openIntrestProcesserFour(mkDataGap);
                    }
                    ////##########################################
                    //平仓处理
                    //止损平仓
                    //正套价差扩大到开仓均价的priceTick*stopLossTickNums之外，止损平仓
                    if (asHoldMeanGap - priceTick*tm->stopLossTickNums >= asCloseGap && canBeClose(insComKey, "as")) {//正套价差扩大到开仓均价的priceTick*stopLossTickNums之外，止损平仓
                        if(isLogout){
                            LOG(INFO) << "正套止损。近月不活跃.asHoldMeanGap=" + boost::lexical_cast<string>(asHoldMeanGap) + "，止损点设为= " + boost::lexical_cast<string>(asHoldMeanGap - priceTick*tm->stopLossTickNums) + ".当前价差扩大到" + boost::lexical_cast<string>(asCloseGap) + ",平仓止损," + mkDataGap->des;
                        }
                        stopProfitProcesserThree(mkDataGap);
                    } else if (dsHoldMeanGap + priceTick*tm->stopLossTickNums <= dsCloseGap && canBeClose(insComKey, "ds")) {//反套价差继续缩小到止损线，止损平仓
                        if(isLogout){
                            LOG(INFO) << "反套止损,近月不活跃.dsHoldMeanGap=" + boost::lexical_cast<string>(dsHoldMeanGap) + "，止损点设为=" + boost::lexical_cast<string>(dsHoldMeanGap + priceTick*tm->stopLossTickNums) + ".当前价差缩小到=" + boost::lexical_cast<string>(dsCloseGap) + ",平仓止损," + mkDataGap->des;
                        }
                        stopProfitProcesserFour(mkDataGap);
                    }
                    //止盈平仓
                    //近月不活跃,正套平仓
                    if (asCloseGap >= asCloseGapMetric && canBeClose(insComKey, "as")) {//正套
                        if(isLogout){
                            LOG(INFO) << "正套止盈,近月不活跃.asGap=" + boost::lexical_cast<string>(asCloseGap) + "大于" + boost::lexical_cast<string>(asCloseGapMetric) + ",当前均值=" + boost::lexical_cast<string>(tm->asMAGap) + "," + mkDataGap->des;

                        }
                        stopProfitProcesserThree(mkDataGap);
                    } else if (dsCloseGap <= dsCloseGapMetric && canBeClose(insComKey, "ds")) {//反套
                        if(isLogout){
                            LOG(INFO) << "反套止盈,近月不活跃.dsGap=" + boost::lexical_cast<string>(dsCloseGap) + "小于" + boost::lexical_cast<string>(dsCloseGapMetric) + ",当前均值=" + boost::lexical_cast<string>(tm->dsMAGap) + "," + mkDataGap->des;

                        }
                        stopProfitProcesserFour(mkDataGap);
                    }
                    //LOG(ERROR) << "processStrategy 5";
                    //#############################################
                } else {//远月合约为不活跃合约
                        //远月不活跃,正套
                    if (asOpenGap <= asOpenGapMetric && isOverTrade(insComKey, "as") && isOverTradeTwo(mkDataGap, "as") && isHoldPositionOverGap(tmpPriceGap, "as")) {//正套
                        if(isLogout){
                            LOG(INFO) << "正套开仓,远月不活跃.asOpenGap=" + boost::lexical_cast<string>(asOpenGap) + "小于" + boost::lexical_cast<string>(asOpenGapMetric) + "," + mkDataGap->des;

                        }
                        openIntrestProcesserOne(mkDataGap);
                    } else if (dsOpenGap >= dsOpenGapMetric && isOverTrade(insComKey, "ds") && isOverTradeTwo(mkDataGap, "ds") && isHoldPositionOverGap(tmpPriceGap, "ds")) {//反套,//远月不活跃，反套
                        if(isLogout){
                            LOG(INFO) << "反套开仓,远月不活跃.dsOpenGap=" + boost::lexical_cast<string>(dsOpenGap) + "大于" + boost::lexical_cast<string>(dsOpenGapMetric) + "," + mkDataGap->des;

                        }
                        openIntrestProcesserThree(mkDataGap);
                    }
                    //###########################################
                    //平仓处理
                    //止损平仓
                    if (asHoldMeanGap - priceTick*tm->stopLossTickNums >= asCloseGap && canBeClose(insComKey, "as")) {//正套价差扩大到开仓均价的priceTick*stopLossTickNums之外，止损平仓
                        if(isLogout){
                            LOG(INFO) << "正套止损。远月不活跃.asHoldMeanGap=" + boost::lexical_cast<string>(asHoldMeanGap) + "，止损线= " + boost::lexical_cast<string>(asHoldMeanGap - priceTick*tm->stopLossTickNums) + ",当前价差扩大到" + boost::lexical_cast<string>(asCloseGap) + ",平仓止损," + mkDataGap->des;

                        }
                        stopProfitProcesserOne(mkDataGap);
                    } else if (dsHoldMeanGap + priceTick*tm->stopLossTickNums <= dsCloseGap && canBeClose(insComKey, "ds")) {//反套价差继续缩小到止损线，止损平仓
                        if(isLogout){
                            LOG(INFO) << "反套止损,远月不活跃.dsHoldMeanGap=" + boost::lexical_cast<string>(dsHoldMeanGap) + "，止损线=" + boost::lexical_cast<string>(dsHoldMeanGap + priceTick*tm->stopLossTickNums) + ",当前价差缩小到=" + boost::lexical_cast<string>(dsCloseGap) + ",平仓止损," + mkDataGap->des;

                        }
                        stopProfitProcesserTwo(mkDataGap);
                    }
                    //远月不活跃,正套平仓
                    if (asCloseGap >= asCloseGapMetric && canBeClose(insComKey, "as")) {//正套
                        if(isLogout){
                            LOG(INFO) << "正套止盈,远月不活跃.asGap=" + boost::lexical_cast<string>(asCloseGap) + "大于" + boost::lexical_cast<string>(asCloseGapMetric) + ",当前均值=" + boost::lexical_cast<string>(tm->asMAGap) + "," + mkDataGap->des;

                        }
                        stopProfitProcesserOne(mkDataGap);
                    } else if (dsCloseGap <= dsCloseGapMetric && canBeClose(insComKey, "ds")) {//反套,//远月不活跃，反套
                        if(isLogout){
                            LOG(INFO) << "反套止盈,远月不活跃.dsGap=" + boost::lexical_cast<string>(dsCloseGap) + "小于" + boost::lexical_cast<string>(dsCloseGapMetric) + ",当前均值=" + boost::lexical_cast<string>(tm->dsMAGap) + "," + mkDataGap->des;

                        }
                        stopProfitProcesserTwo(mkDataGap);
                    }
                    //LOG(ERROR) << "processStrategy 6";
                    //###########################################
                }
                //结束时间
                endTime = getCurrentTimeByBoost();
                int seconds = getTimeInterval(startTime, endTime, "t");
                //LOG(INFO) << "processStrategy,处理时长=" + boost::lexical_cast<string>(seconds);
                string stg = "businessType=wtm_4;insComKey=" + insComKey + ";" + "tradingDay=" + tradingDayT
                    + ";" + "processTime=" + boost::lexical_cast<string>(seconds) + ";type=strategy;seq=" + boost::lexical_cast<string>(endTime);
                logmsg = new LogMsg();
                logmsg->setMsg(stg);
                networkTradeQueue.push(logmsg);
                //Sleep(1);
                //LOG(ERROR) << "processStrategy 7";
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
void openProcesser() {
    cout<<"启动进程openProcesser"<<endl;
    //cout << boosttoolsnamespace::CBoostTools::gbktoutf8("启动进程") << endl;
    try{
        PriceGapMarketData * mkDataGap;
        while (1) {
            if (detectOpenQueue.empty()) {
                this_thread::yield();
            } else if (detectOpenQueue.pop(mkDataGap)) {
                //开始时间
                boost::posix_time::ptime startTime = getCurrentTimeByBoost();
                boost::posix_time::ptime endTime;
                int ticks;
                //boost::recursive_mutex::scoped_lock SLock1(mkdata_mtx);//加锁处理
                boost::recursive_mutex::scoped_lock SLock2(willTrade_mtx);//加锁处理
//				boost::recursive_mutex::scoped_lock SLock4(arbVolume_mtx);//加锁处理
                boost::recursive_mutex::scoped_lock SLock5(priceGap_mtx);//锁定
                boost::recursive_mutex::scoped_lock SLock6(alreadyTrade_mtx);
                boost::recursive_mutex::scoped_lock SLock3(techMetric_mtx);
                endTime = getCurrentTimeByBoost();
                ticks = getTimeInterval(startTime, endTime, "t");
                LOG(INFO) << "openProcesser,获取锁处理时长=" + boost::lexical_cast<string>(ticks);
                string info = getCurrentSystemTime() + " ";
                string insComKey = (mkDataGap->insComKey);
                string lastInstrumentID = mkDataGap->lastInstrumentID;//近月
                string forwardInstrumentID = mkDataGap->forwardInstrumentID;
                double lastPriceTick = 0;
                double forwardPriceTick = 0;//配对最小变动
                //double currLastPrice = mkData->lastPrice;
                double lastBidPrice = mkDataGap->lastInsBidPrice;
                double lastAskPrice = mkDataGap->lastInsAskPrice;
                int lastVolume = mkDataGap->lastInsVolume;//当前合约成交量

                double fwdBidPrice = mkDataGap->forwardInsBidPrice;
                double fwdAskPrice = mkDataGap->forwardInsAskPrice;
                int fwdVolume = mkDataGap->forwardInsVolume;
                double asGap = mkDataGap->asOpenGap;
                double dsGap = mkDataGap->dsOpenGap;
                //获得技术指标
                unordered_map<string, TechMetric*>::iterator tmmIt = techMetricMap.find(insComKey);
                if (tmmIt == techMetricMap.end()) {
                    LOG(INFO) << "无法查找到InstrumentID=" + insComKey + "对应的技术指标";
                    return ;
                }
                TechMetric* tm = tmmIt->second;
                if (tm->maGap == 0 || asGap == 0 || dsGap == 0) {//数据未开始接受
                    return ;
                }
                double priceTick = getPriceTick(lastInstrumentID);
                if (priceTick == 0) {
                    LOG(ERROR) << "无法查找到InstrumentID=" + lastInstrumentID + "对应的TICK";
                    return;
                }
                double openMaxGap = tm->asMAGap - tm->overMAGapTickNums*priceTick;//开仓阈值为均值之上overMAGapTickNums个tick.正套
                double openMinGap = tm->dsMAGap + tm->overMAGapTickNums*priceTick;//开仓阈值为均值之下overMAGapTickNums个tick.反套
                LOG(INFO) << "asGap=" + boost::lexical_cast<string>(asGap) + ",openMaxGap=" + boost::lexical_cast<string>(openMaxGap) + ",dsGap=" + boost::lexical_cast<string>(dsGap) + ",openMinGap=" + boost::lexical_cast<string>(openMinGap) + ",当前均值as=" + boost::lexical_cast<string>(tm->asMAGap) +",ds=" + boost::lexical_cast<string>(tm->dsMAGap) ;
                //不存在下单信息，那么直接以市价单下单
                //一、统一近月-远月的合约价差进行比较;套利时候，买近月，卖远月
                //二、市价可以成交的，不必考虑canbetrade或者canbeclose
                if (fwdVolume >= lastVolume) {//远月合约为活跃合约,近月合约为非活跃合约.
                    //近月不活跃,正套
                    if (asGap <= openMaxGap && isOverTrade(insComKey,"as") && isOverTradeTwo(mkDataGap, "as")) {//正套
                        LOG(INFO) << "正套开仓,近月不活跃.asGap=" + boost::lexical_cast<string>(asGap) + "小于" + boost::lexical_cast<string>(openMaxGap) + ",当前均值=" + boost::lexical_cast<string>(tm->asMAGap) + "," + mkDataGap->des;
                        openIntrestProcesserTwo(mkDataGap);
                    }else if (dsGap >= openMinGap && isOverTrade(insComKey,"ds") && isOverTradeTwo(mkDataGap, "ds")) {//反套
                        LOG(INFO) << "反套开仓,近月不活跃.dsGap=" + boost::lexical_cast<string>(dsGap) + "大于" + boost::lexical_cast<string>(openMinGap) + ",当前均值=" + boost::lexical_cast<string>(tm->dsMAGap) + "," + mkDataGap->des;
                        openIntrestProcesserFour(mkDataGap);
                    }
                } else {//远月合约为不活跃合约
                    //远月不活跃,正套
                    if (asGap <= openMaxGap && isOverTrade(insComKey,"as") && isOverTradeTwo(mkDataGap, "as")) {//正套
                        LOG(INFO) << "正套开仓,远月不活跃.asGap=" + boost::lexical_cast<string>(asGap) + "小于" + boost::lexical_cast<string>(openMaxGap) + ",当前均值=" + boost::lexical_cast<string>(tm->asMAGap) + "," + mkDataGap->des;
                        openIntrestProcesserOne(mkDataGap);
                    }else if (dsGap >= openMinGap && isOverTrade(insComKey,"ds") && isOverTradeTwo(mkDataGap, "ds")) {//反套,//远月不活跃，反套
                        LOG(INFO) << "反套开仓,远月不活跃.dsGap=" + boost::lexical_cast<string>(dsGap) + "大于" + boost::lexical_cast<string>(openMinGap) + ",当前均值=" + boost::lexical_cast<string>(tm->dsMAGap) + "," + mkDataGap->des;
                        openIntrestProcesserThree(mkDataGap);
                    }
                }
                //结束时间
                endTime = getCurrentTimeByBoost();
                int seconds = getTimeInterval(startTime, endTime, "t");
                LOG(INFO) << "openProcesser,处理时长=" + boost::lexical_cast<string>(seconds);
                //Sleep(1);
            }
        }
    }catch (std::exception& e){
        std::cerr << e.what() << std::endl;
    }
}
void closeProcesser() {
    cout<<"启动进程closeProcesser"<<endl;
    //cout << boosttoolsnamespace::CBoostTools::gbktoutf8("启动进程") << endl;
    try {
        PriceGapMarketData * mkDataGap;
        while (1) {
            if (detectOpenQueue.empty()) {
                this_thread::yield();
            } else if (detectOpenQueue.pop(mkDataGap)) {
                //开始时间
                boost::posix_time::ptime startTime = getCurrentTimeByBoost();
                boost::posix_time::ptime endTime ;
                int ticks;
                //boost::recursive_mutex::scoped_lock SLock1(mkdata_mtx);//加锁处理
                //boost::recursive_mutex::scoped_lock SLock2(willTrade_mtx);//加锁处理
                //boost::recursive_mutex::scoped_lock SLock1(arbVolume_mtx);//加锁处理
                boost::recursive_mutex::scoped_lock SLock5(priceGap_mtx);//锁定
                boost::recursive_mutex::scoped_lock SLock2(stopProfit_mtx);
                boost::recursive_mutex::scoped_lock SLock(pst_mtx);//锁定
                boost::recursive_mutex::scoped_lock SLock4(alreadyTrade_mtx);//锁定
                boost::recursive_mutex::scoped_lock SLock3(techMetric_mtx);
                //结束时间
                endTime = getCurrentTimeByBoost();
                ticks = getTimeInterval(startTime, endTime, "t");
                LOG(INFO) << "closeProcesser,获取锁处理时长=" + boost::lexical_cast<string>(ticks);
                string info = getCurrentSystemTime() + " ";
                string insComKey = (mkDataGap->insComKey);
                string lastInstrumentID = mkDataGap->lastInstrumentID;//近月
                string forwardInstrumentID = mkDataGap->forwardInstrumentID;
                double lastPriceTick = 0;
                double forwardPriceTick = 0;//配对最小变动

                double lastBidPrice = mkDataGap->lastInsBidPrice;
                double lastAskPrice = mkDataGap->lastInsAskPrice;
                int lastVolume = mkDataGap->lastInsVolume;//当前合约成交量

                double fwdBidPrice = mkDataGap->forwardInsBidPrice;
                double fwdAskPrice = mkDataGap->forwardInsAskPrice;
                int fwdVolume = mkDataGap->forwardInsVolume;
                double asGap = mkDataGap->asCloseGap;
                double dsGap = mkDataGap->dsCloseGap;
                //获得技术指标
                unordered_map<string, TechMetric*>::iterator tmmIt = techMetricMap.find(insComKey);
                if (tmmIt == techMetricMap.end()) {
                    LOG(INFO) << "无法查找到InstrumentID=" + insComKey + "对应的技术指标";
                    return;
                }
                TechMetric* tm = tmmIt->second;
                if (tm->maGap == 0) {//数据未开始接受
                    return;
                }
                double priceTick = getPriceTick(lastInstrumentID);
                if (priceTick == 0) {
                    LOG(ERROR) << "无法查找到InstrumentID=" + lastInstrumentID + "对应的TICK";
                    return;
                }
                double closeMaxGap = tm->dsMAGap - tm->overMAGapTickNums*priceTick;//平仓阈值为均值之上overMAGapTickNums个tick.反套
                double closeMinGap = tm->asMAGap + tm->downMAGapTickNums*priceTick;//平仓阈值为均值之下overMAGapTickNums个tick.正套
                double asHoldMeanGap = tm->asHoldMeanGap;//正套持仓均值
                double dsHoldMeanGap = tm->dsHoldMeanGap;//反套持仓均值
                                                                            //不存在下单信息，那么直接以市价单下单
                                                                            //一、统一近月-远月的合约价差进行比较;套利时候，买近月，卖远月
                                                                            //二、市价可以成交的，不必考虑canbetrade或者canbeclose
                LOG(INFO) << "asGap=" + boost::lexical_cast<string>(asGap) + "需要大于closeMinGap=" + boost::lexical_cast<string>(closeMinGap) + "才能止盈平仓,dsCloseGap=" + boost::lexical_cast<string>(dsGap) + "需要小于closeMaxGap=" + boost::lexical_cast<string>(closeMaxGap) + "才能止盈平仓。当前均值as=" + boost::lexical_cast<string>(tm->asMAGap)+",ds="+ boost::lexical_cast<string>(tm->dsMAGap) +
                    ",正套持仓均值=" + boost::lexical_cast<string>(asHoldMeanGap) + ",反套持仓均值=" + boost::lexical_cast<string>(dsHoldMeanGap) + ",行情信息=" + mkDataGap->des;
                if (fwdVolume >= lastVolume) {//远月合约为活跃合约,近月合约为非活跃合约.
                    if (asHoldMeanGap - priceTick*tm->stopLossTickNums >= asGap) {//正套价差扩大到开仓均价的priceTick*stopLossTickNums之外，止损平仓
                        LOG(INFO) << "正套止损。近月不活跃.asHoldMeanGap=" + boost::lexical_cast<string>(asHoldMeanGap) + "，止损线= " + boost::lexical_cast<string>(asHoldMeanGap - priceTick*tm->stopLossTickNums) +",当前价差扩大到" + boost::lexical_cast<string>(asGap) + ",平仓止损," + mkDataGap->des;
                        stopProfitProcesserThree(mkDataGap);
                    } else if (dsHoldMeanGap + priceTick*tm->stopLossTickNums <= dsGap) {//反套价差继续缩小到止损线，止损平仓
                        LOG(INFO) << "反套止损,近月不活跃.dsHoldMeanGap=" + boost::lexical_cast<string>(dsHoldMeanGap) + "，止损线=" + boost::lexical_cast<string>(dsHoldMeanGap + priceTick*tm->stopLossTickNums) + ",当前价差缩小到=" + boost::lexical_cast<string>(dsGap) + ",平仓止损," + mkDataGap->des;
                        stopProfitProcesserFour(mkDataGap);
                    }
                    //近月不活跃,正套平仓
                    if (asGap >= closeMinGap && canBeClose(insComKey,"as")) {//正套
                        LOG(INFO) << "正套止盈,近月不活跃.asGap=" + boost::lexical_cast<string>(asGap) + "大于" + boost::lexical_cast<string>(closeMinGap) + ",当前均值=" + boost::lexical_cast<string>(tm->asMAGap) + "," + mkDataGap->des;
                        stopProfitProcesserThree(mkDataGap);
                    } else if (dsGap <= closeMaxGap && canBeClose(insComKey, "ds")) {//反套
                        LOG(INFO) << "反套止盈,近月不活跃.dsGap=" + boost::lexical_cast<string>(dsGap) + "小于" + boost::lexical_cast<string>(closeMaxGap) + ",当前均值=" + boost::lexical_cast<string>(tm->dsMAGap) + "," + mkDataGap->des;
                        stopProfitProcesserFour(mkDataGap);
                    }
                } else {//远月合约为不活跃合约
                    if (asHoldMeanGap - priceTick*tm->stopLossTickNums >= asGap) {//正套价差扩大到开仓均价的priceTick*stopLossTickNums之外，止损平仓
                        LOG(INFO) << "正套止损。远月不活跃.asHoldMeanGap=" + boost::lexical_cast<string>(asHoldMeanGap) + "，止损线= " + boost::lexical_cast<string>(asHoldMeanGap - priceTick*tm->stopLossTickNums) + ",当前价差扩大到" + boost::lexical_cast<string>(asGap) + ",平仓止损," + mkDataGap->des;
                        stopProfitProcesserOne(mkDataGap);
                    } else if (dsHoldMeanGap + priceTick*tm->stopLossTickNums <= dsGap) {//反套价差继续缩小到止损线，止损平仓
                        LOG(INFO) << "反套止损,远月不活跃.dsHoldMeanGap=" + boost::lexical_cast<string>(dsHoldMeanGap) + "，止损线=" + boost::lexical_cast<string>(dsHoldMeanGap + priceTick*tm->stopLossTickNums) + ",当前价差缩小到=" + boost::lexical_cast<string>(dsGap) + ",平仓止损," + mkDataGap->des;
                        stopProfitProcesserTwo(mkDataGap);
                    }
                        //远月不活跃,正套平仓
                    if (asGap >= closeMinGap && canBeClose(insComKey, "as")) {//正套
                        LOG(INFO) << "正套止盈,远月不活跃.asGap=" + boost::lexical_cast<string>(asGap) + "大于" + boost::lexical_cast<string>(closeMinGap) + ",当前均值=" + boost::lexical_cast<string>(tm->asMAGap) + "," + mkDataGap->des;
                        stopProfitProcesserOne(mkDataGap);
                    } else if (dsGap <= closeMaxGap && canBeClose(insComKey, "ds")) {//反套,//远月不活跃，反套
                        LOG(INFO) << "反套止盈,远月不活跃.dsGap=" + boost::lexical_cast<string>(dsGap) + "小于" + boost::lexical_cast<string>(closeMaxGap) + ",当前均值=" + boost::lexical_cast<string>(tm->dsMAGap) + "," + mkDataGap->des;
                        stopProfitProcesserTwo(mkDataGap);
                    }
                }
                //结束时间
                endTime = getCurrentTimeByBoost();
                int seconds = getTimeInterval(startTime, endTime, "t");
                LOG(INFO) << "closeProcesser,处理时长=" + boost::lexical_cast<string>(seconds);
                //Sleep(1);
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
string getMarketData(MarketData* mkdata) {
    double currLastPrice = mkdata->lastPrice;
    double currBidPrice = mkdata->bidPrice;
    double currAskPrice = mkdata->askPrice;
    int currVolume = mkdata->volume;
    double currPriceTick = mkdata->priceTick;
    string currInstrumentID = mkdata->instrumentID;
    string datainfo = "instrumentID=" + currInstrumentID + ";lastPrice=" + boost::lexical_cast<string>(currLastPrice) + ";bidPrice=" + boost::lexical_cast<string>(currBidPrice) + ";askPrice=" +
        boost::lexical_cast<string>(currAskPrice) + ";priceTick=" + boost::lexical_cast<string>(currPriceTick) + ";volume=" + boost::lexical_cast<string>(currVolume);
    return datainfo;
}
MarketPriceAction* priceDefineOpen(PriceGapMarketData* pgMkData, bool isLastActive) {
    double* temp = new double[3];
    MarketPriceAction* mpa = new MarketPriceAction();
    temp[0] = 0;
    temp[1] = 0;
    temp[2] = 0;
    //近月
    //double currLastPrice = lastMarketData->lastPrice;
    double currBidPrice = pgMkData->lastInsBidPrice;
    double currAskPrice = pgMkData->lastInsAskPrice;
    //int currVolume = lastMarketData->volume;
    double currPriceTick = pgMkData->priceTick;
    string currInstrumentID = pgMkData->lastInstrumentID;
    //string cstr = getMarketData(lastMarketData);
    //远月
    double f_lastPrice = pgMkData->forwardInsBidPrice;
    double f_bidPrice = pgMkData->forwardInsBidPrice;
    double f_askPrice = pgMkData->forwardInsAskPrice;
    //int f_volume = forwardMarketData->volume;
    double f_priceTick = pgMkData->priceTick;;
    string f_instrumentID = pgMkData->forwardInstrumentID;;
    //string fstr = getMarketData(forwardMarketData);

    //预设价差
    //double openMinGap = preSetGapInfo->minGap;//-20
    //double openMaxGap = preSetGapInfo->maxGap;//-15
    //double closeGap = preSetGapInfo->profitGap;//-10
    string insComKey = pgMkData->insComKey;
    //获得技术指标
    unordered_map<string, TechMetric*>::iterator tmmIt = techMetricMap.find(insComKey);
    if (tmmIt == techMetricMap.end()) {
        LOG(INFO) << "无法查找到InstrumentID=" + insComKey + "对应的技术指标";
        return mpa;
    }
    TechMetric* tm = tmmIt->second;
    if (tm->maGap == 0) {//数据未开始接受
        return mpa;
    }
    double openMaxGap = tm->maGap - tm->overMAGapTickNums*currPriceTick;//开仓阈值为均值之上2个tick
    //string datainfo = cstr + ";" + fstr;
    //价格形成机制：买入近月合约，卖出远月合约，正套
    if (isLastActive) {//近月合约为活跃
        double gap1 = pgMkData->gapPrice;//市价可以成交gap
        if (gap1 <= openMaxGap) {
            LOG(INFO) << "近月合约:" + currInstrumentID + "为活跃,市价可以成交,设置市价成交套利组合,gap=" + boost::lexical_cast<string>(gap1) + ",阈值为" + boost::lexical_cast<string>(openMaxGap) + ",均值为" + boost::lexical_cast<string>(tm->maGap)
                + ";";
            temp[0] = currAskPrice;
            temp[1] = f_bidPrice;
            temp[2] = 11;
            mpa->lastInstrInsertPrice = currAskPrice;
            mpa->forwardInstrInsertPrice = f_bidPrice;
            mpa->hopeGap = gap1;
            mpa->orderType = "11";
            return mpa;
        }
    } else {//远月为活跃合约
        double gap1 = currAskPrice - f_bidPrice;//市价可以成交gap
                                                //double midprice = round((currBidPrice + currAskPrice) / 2/currPriceTick)*currPriceTick;
        double gap2 = currAskPrice - f_askPrice;//普通gap
        //if (gap1 <= openMaxGap && gap1 >= openMinGap) {
        if (gap1 <= openMaxGap) {
            LOG(INFO) << "远月合约:" + f_instrumentID + "为活跃,市价可以成交,设置市价成交套利组合.gap=" + boost::lexical_cast<string>(gap1) + ",阈值为" + boost::lexical_cast<string>(openMaxGap) + ",均值为" + boost::lexical_cast<string>(tm->maGap) + ";" ;
            temp[0] = currAskPrice;
            temp[1] = f_bidPrice;
            temp[2] = 11;
            mpa->lastInstrInsertPrice = currAskPrice;
            mpa->forwardInstrInsertPrice = f_bidPrice;
            mpa->hopeGap = gap1;
            mpa->orderType = "11";
            return mpa;
        }
    }
    return mpa;

}
MarketPriceAction* priceDefineClose(MarketData* lastMarketData, MarketData* forwardMarketData, PriceGap* preSetGapInfo, bool isLastActive) {
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
    //double openMinGap = preSetGapInfo->minGap;//-20
    //double openMaxGap = preSetGapInfo->maxGap;//-15
    //double closeGap = preSetGapInfo->profitGap;//-10
    string insComKey = getComInstrumentKey(currInstrumentID, f_instrumentID);
    string datainfo = cstr + ";" + fstr;
    //价格形成机制：买入近月合约，卖出远月合约，正套
    if (isLastActive) {//近月合约为活跃
         //平仓合约价格:卖出近月合约，买入远月合约(远月不活跃,先交易)
        double closeGap1 = currBidPrice - f_askPrice;//市价可以成交
        //gap1较大，gap2较小
        //以下为盈利离场///////////////////
        //第二次修改时，需要市价成交才下单
        //第三次修改成，每次独立判断，市价成交才下单
        unordered_map<string, list<TradeInfo*>*>::iterator it = alreadyTradeMapAS.find(insComKey);
        if (it != alreadyTradeMapAS.end()) {
            list<TradeInfo*>* tradeList = it->second;
            for (list<TradeInfo*>::iterator tradeIt = tradeList->begin(); tradeIt != tradeList->end(); tradeIt++) {
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
    } else {//远月为活跃合约
        double closeGap1 = currBidPrice - f_askPrice;//市价可以成交
         //以下为止盈离场
         //第二次修改时，需要市价成交才下单
         //第三次修改成，每次独立判断，市价成交才下单
        unordered_map<string, list<TradeInfo*>*>::iterator it = alreadyTradeMapAS.find(insComKey);
        if (it != alreadyTradeMapAS.end()) {
            list<TradeInfo*>* tradeList = it->second;
            for (list<TradeInfo*>::iterator tradeIt = tradeList->begin(); tradeIt != tradeList->end(); tradeIt++) {
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
    }
    return mpa;
}
/*远月为不活跃合约，先开远月，卖远月
正套*/
void openIntrestProcesserOne(PriceGapMarketData* defineInfo) {
    //定义好元近月的报单价格
    string currInstrumentID = defineInfo->lastInstrumentID;
    string fwdInstrumentID = defineInfo->forwardInstrumentID;
    string insComKey = defineInfo->insComKey;
    list<string> orderstr;
    /*
    //double tmpPrice = currBidPrice;//比当前卖的价格的价格还低
    orderstr.push_back("InstrumentID=" + fwdInstrumentID);
    orderstr.push_back("RequestID=" + boost::lexical_cast<string>(iRequestID++));
    orderstr.push_back("Direction=1");//组合投机套保标 投机 '1'套保 '3'
    orderstr.push_back("HedgeFlag=" + hedgeFlag);
    orderstr.push_back("OffsetFlag=0");
    orderstr.push_back("Price=" + boost::lexical_cast<string>(defineInfo->forwardInsBidPrice));//tick暂时硬编码
    orderstr.push_back("Volume=" + boost::lexical_cast<string>(defaultVolume));
    string notActiveOrderRef = boost::lexical_cast<string>(iOrderRef++);
    orderstr.push_back("OrderRef=" + notActiveOrderRef);
    orderstr.push_back("priceType=2");
    orderstr.push_back("timeCondition=1");//立即单
    */
    //shengli
    EES_EnterOrderField orderField;
    memset(&orderField, 0, sizeof(EES_EnterOrderField));
    orderField.m_Tif = EES_OrderTif_IOC;//立即单
    orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
    strcpy(orderField.m_Account, INVESTOR_ID.c_str());
    strcpy(orderField.m_Symbol, fwdInstrumentID.c_str());//modify 2.
    orderField.m_Side = EES_SideType_open_short;//modify 1.buy open
    //orderField.m_Exchange = EES_ExchangeID_shfe;
    orderField.m_SecType = EES_SecType_fut;
    orderField.m_Price = defineInfo->forwardInsBidPrice;//modify 3.
    orderField.m_Qty = defaultVolume;
    orderField.m_ClientOrderToken = ++iOrderRef;
    //ptradeApi->reqOrderInsert(&orderField,"100");
    ptradeApi->reqOrderInsert(&orderField,NULL);
    TradeInfo* tmpinfo = new TradeInfo();
    tmpinfo->hopeOpenGap = defineInfo->lastInsAskPrice - defineInfo->forwardInsBidPrice;
    tmpinfo->insComKey = insComKey;
    tmpinfo->orderType = "as";
    //tmpinfo->frontID = FRONT_ID;
    //tmpinfo->sessionID = SESSION_ID;
    tmpinfo->notActiveSecType = EES_SecType_fut;
    tmpinfo->notActiveInstrumentid = fwdInstrumentID;
    //tmpinfo->notActiveDirection = "1";
    //tmpinfo->notActiveOffsetFlag = "0";
    tmpinfo->notActiveTradeSide = orderField.m_Side;
    tmpinfo->notActiveClientOrderToken = orderField.m_ClientOrderToken;
    tmpinfo->notActiveOrderInsertPrice = defineInfo->forwardInsBidPrice;
    tmpinfo->notActiveIsTraded = false;
    //tmpinfo->notActiveOrderRef = notActiveOrderRef;
    tmpinfo->notActiveVolume = orderField.m_Qty ;
    tmpinfo->notActiveHedgeFlag = hedgeFlag;
    tmpinfo->notActiveInsertAmount = 1;

    tmpinfo->activeSecType = EES_SecType_fut;
    tmpinfo->activeInstrumentid = currInstrumentID;
    //tmpinfo->activeDirection = "0";
    //tmpinfo->activeOffsetFlag = "0";
    tmpinfo->activeTradeSide = EES_SideType_open_long;//buy open
    tmpinfo->activeOrderInsertPrice = defineInfo->lastInsAskPrice;
    tmpinfo->activeIsTraded = false;
    tmpinfo->activeVolume = orderField.m_Qty ;
    tmpinfo->activeHedgeFlag = hedgeFlag;
    unordered_map<string, list<TradeInfo*>*>::iterator it = willTradeMap.find(insComKey);
    if (it == willTradeMap.end()) {//新添加
        list<TradeInfo*>* tmpList = new list<TradeInfo*>();
        tmpList->push_back(tmpinfo);//最新的组合添加到最后
        willTradeMap[insComKey] = tmpList;
    } else {
        list<TradeInfo*>* tmpList = it->second;
        tmpList->push_back(tmpinfo);
    }
}
/*近月为不活跃合约，先开近月。买入近月
正套*/
void openIntrestProcesserTwo(PriceGapMarketData* defineInfo) {
    //定义好元近月的报单价格
    string currInstrumentID = defineInfo->lastInstrumentID;
    string fwdInstrumentID = defineInfo->forwardInstrumentID;
    string insComKey = defineInfo->insComKey;
    /*
    list<string> orderstr;
    orderstr.push_back("InstrumentID=" + currInstrumentID);
    orderstr.push_back("RequestID=" + boost::lexical_cast<string>(iRequestID++));
    orderstr.push_back("Direction=0");//组合投机套保标 投机 '1'套保 '3'
    orderstr.push_back("HedgeFlag=" + hedgeFlag);
    orderstr.push_back("OffsetFlag=0");
    orderstr.push_back("Price=" + boost::lexical_cast<string>(defineInfo->lastInsAskPrice));//tick暂时硬编码
    orderstr.push_back("Volume=" + boost::lexical_cast<string>(defaultVolume));
    //string notActiveOrderRef = boost::lexical_cast<string>(iOrderRef++);
    //string notActiveClientToken = boost::lexical_cast<string>(iOrderRef++);
    orderstr.push_back("OrderRef=" + notActiveOrderRef);
    orderstr.push_back("priceType=2");
    orderstr.push_back("timeCondition=1");//立即单
    */
    //shengli
    EES_EnterOrderField orderField;
    memset(&orderField, 0, sizeof(EES_EnterOrderField));
    orderField.m_Tif = EES_OrderTif_IOC;//立即单
    orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
    strcpy(orderField.m_Account, INVESTOR_ID.c_str());
    strcpy(orderField.m_Symbol, currInstrumentID.c_str());
    orderField.m_Side = EES_SideType_open_long;//buy open
    //orderField.m_Exchange = EES_ExchangeID_shfe;
    orderField.m_SecType = EES_SecType_fut;
    orderField.m_Price = defineInfo->lastInsAskPrice;
    orderField.m_Qty = defaultVolume;
    orderField.m_ClientOrderToken = ++iOrderRef;
    //ptradeApi->reqOrderInsert(&orderField,"100");
    ptradeApi->reqOrderInsert(&orderField,NULL);

    TradeInfo* tmpinfo = new TradeInfo();
    tmpinfo->hopeOpenGap = defineInfo->lastInsAskPrice - defineInfo->forwardInsBidPrice;
    tmpinfo->insComKey = insComKey;
    tmpinfo->orderType = "as";
    //tmpinfo->frontID = FRONT_ID;
    //tmpinfo->sessionID = SESSION_ID;
    tmpinfo->notActiveSecType = EES_SecType_fut;
    tmpinfo->notActiveInstrumentid = currInstrumentID;
    tmpinfo->notActiveTradeSide = orderField.m_Side ;
    //tmpinfo->notActiveDirection = "0";
    //tmpinfo->notActiveOffsetFlag = "0";
    tmpinfo->notActiveOrderInsertPrice = defineInfo->lastInsAskPrice;
    tmpinfo->notActiveIsTraded = false;
    //tmpinfo->notActiveOrderRef = notActiveOrderRef;
    tmpinfo->notActiveClientOrderToken = orderField.m_ClientOrderToken;
    tmpinfo->notActiveVolume = orderField.m_Qty;
    tmpinfo->notActiveHedgeFlag = hedgeFlag;

    tmpinfo->activeSecType = EES_SecType_fut;
    tmpinfo->activeInstrumentid = fwdInstrumentID;
    //tmpinfo->activeDirection = "1";
    //tmpinfo->activeOffsetFlag = "0";
    tmpinfo->activeTradeSide = EES_SideType_open_short;//sell open
    tmpinfo->activeOrderInsertPrice = defineInfo->forwardInsBidPrice;
    tmpinfo->activeIsTraded = false;
    tmpinfo->activeVolume = orderField.m_Qty;
    tmpinfo->activeHedgeFlag = hedgeFlag;
    cout<< "activeTRadeside=" + boost::lexical_cast<string>(tmpinfo->activeTradeSide);
    unordered_map<string, list<TradeInfo*>*>::iterator it = willTradeMap.find(insComKey);
    if (it == willTradeMap.end()) {//新添加
        list<TradeInfo*>* tmpList = new list<TradeInfo*>();
        tmpList->push_back(tmpinfo);//最新的组合添加到最后
        willTradeMap[insComKey] = tmpList;
    } else {
        list<TradeInfo*>* tmpList = it->second;
        tmpList->push_back(tmpinfo);
    }
}
/*远月为不活跃合约，先开远月。买入远月
反套*/
void openIntrestProcesserThree(PriceGapMarketData* defineInfo) {
    //定义好元近月的报单价格
    string currInstrumentID = defineInfo->lastInstrumentID;
    string fwdInstrumentID = defineInfo->forwardInstrumentID;
    string insComKey = defineInfo->insComKey;
    /*
    list<string> orderstr;
    //double tmpPrice = currAskPrice;
    orderstr.push_back("InstrumentID=" + fwdInstrumentID);
    orderstr.push_back("RequestID=" + boost::lexical_cast<string>(iRequestID++));
    orderstr.push_back("Direction=0");//组合投机套保标 投机 '1'套保 '3'
    orderstr.push_back("HedgeFlag=" + hedgeFlag);
    orderstr.push_back("OffsetFlag=0");
    orderstr.push_back("Price=" + boost::lexical_cast<string>(defineInfo->forwardInsAskPrice));//tick暂时硬编码
    orderstr.push_back("Volume=" + boost::lexical_cast<string>(defaultVolume));
    string notActiveOrderRef = boost::lexical_cast<string>(iOrderRef++);
    orderstr.push_back("OrderRef=" + notActiveOrderRef);
    orderstr.push_back("priceType=2");
    orderstr.push_back("timeCondition=1");//立即单
    */
    //shengli
    EES_EnterOrderField orderField;
    memset(&orderField, 0, sizeof(EES_EnterOrderField));
    orderField.m_Tif = EES_OrderTif_IOC;//立即单
    orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
    strcpy(orderField.m_Account, INVESTOR_ID.c_str());
    strcpy(orderField.m_Symbol, fwdInstrumentID.c_str());//modify 2.
    orderField.m_Side = EES_SideType_open_long;//modify 1.buy open
    //orderField.m_Exchange = EES_ExchangeID_shfe;
    orderField.m_SecType = EES_SecType_fut;
    orderField.m_Price = defineInfo->forwardInsAskPrice;//modify 3.
    orderField.m_Qty = defaultVolume;
    orderField.m_ClientOrderToken = ++iOrderRef;
    //ptradeApi->reqOrderInsert(&orderField,"100");
    ptradeApi->reqOrderInsert(&orderField,NULL);

    //pTradeUserSpi->ReqOrderInsert(orderstr);
    TradeInfo* tmpinfo = new TradeInfo();
    tmpinfo->hopeOpenGap = defineInfo->lastInsBidPrice - defineInfo->forwardInsAskPrice;
    tmpinfo->insComKey = insComKey;
    tmpinfo->orderType = "ds";
    //tmpinfo->frontID = FRONT_ID;
    //tmpinfo->sessionID = SESSION_ID;
    tmpinfo->notActiveSecType = EES_SecType_fut;
    tmpinfo->notActiveInstrumentid = fwdInstrumentID;
    //tmpinfo->notActiveDirection = "0";
    //tmpinfo->notActiveOffsetFlag = "0";
    tmpinfo->notActiveTradeSide = orderField.m_Side;
    tmpinfo->notActiveClientOrderToken = orderField.m_ClientOrderToken;
    tmpinfo->notActiveOrderInsertPrice = defineInfo->forwardInsAskPrice;
    tmpinfo->notActiveIsTraded = false;
    //tmpinfo->notActiveOrderRef = notActiveOrderRef;
    tmpinfo->notActiveVolume = orderField.m_Qty;
    tmpinfo->notActiveHedgeFlag = hedgeFlag;

    tmpinfo->activeSecType = EES_SecType_fut;
    tmpinfo->activeInstrumentid = currInstrumentID;
    //tmpinfo->activeDirection = "1";
    //tmpinfo->activeOffsetFlag = "0";
    tmpinfo->activeTradeSide = EES_SideType_open_short;//sell open
    tmpinfo->activeOrderInsertPrice = defineInfo->lastInsBidPrice;
    tmpinfo->activeIsTraded = false;
    tmpinfo->activeVolume = orderField.m_Qty;
    tmpinfo->activeHedgeFlag = hedgeFlag;
    unordered_map<string, list<TradeInfo*>*>::iterator it = willTradeMap.find(insComKey);
    if (it == willTradeMap.end()) {//新添加
        list<TradeInfo*>* tmpList = new list<TradeInfo*>();
        tmpList->push_back(tmpinfo);//最新的组合添加到最后
        willTradeMap[insComKey] = tmpList;
    } else {
        list<TradeInfo*>* tmpList = it->second;
        tmpList->push_back(tmpinfo);
    }
}
/*近月为不活跃合约，先开近月。卖出近月
反套*/
void openIntrestProcesserFour(PriceGapMarketData* defineInfo) {
    //定义好元近月的报单价格
    string currInstrumentID = defineInfo->lastInstrumentID;
    string fwdInstrumentID = defineInfo->forwardInstrumentID;
    string insComKey = defineInfo->insComKey;
/*
    list<string> orderstr;
    //double tmpPrice = pd_bidPrice;
    orderstr.push_back("InstrumentID=" + currInstrumentID);
    orderstr.push_back("RequestID=" + boost::lexical_cast<string>(iRequestID++));
    orderstr.push_back("Direction=1");//组合投机套保标 投机 '1'套保 '3'
    orderstr.push_back("HedgeFlag=" + hedgeFlag);
    orderstr.push_back("OffsetFlag=0");
    orderstr.push_back("Price=" + boost::lexical_cast<string>(defineInfo->lastInsBidPrice));//tick暂时硬编码
    orderstr.push_back("Volume=" + boost::lexical_cast<string>(defaultVolume));
    string notActiveOrderRef = boost::lexical_cast<string>(iOrderRef++);
    orderstr.push_back("OrderRef=" + notActiveOrderRef);
    orderstr.push_back("priceType=2");
    orderstr.push_back("timeCondition=1");//立即单
    */
    //shengli
    EES_EnterOrderField orderField;
    memset(&orderField, 0, sizeof(EES_EnterOrderField));
    orderField.m_Tif = EES_OrderTif_IOC;//立即单
    orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
    strcpy(orderField.m_Account, INVESTOR_ID.c_str());
    strcpy(orderField.m_Symbol, currInstrumentID.c_str());//modify 2.
    orderField.m_Side = EES_SideType_open_short;//modify 1.sell open
    //orderField.m_Exchange = EES_ExchangeID_shfe;
    orderField.m_SecType = EES_SecType_fut;
    orderField.m_Price = defineInfo->lastInsBidPrice;//modify 3.
    orderField.m_Qty = defaultVolume;
    orderField.m_ClientOrderToken = ++iOrderRef;
    //ptradeApi->reqOrderInsert(&orderField,"100");
    ptradeApi->reqOrderInsert(&orderField,NULL);

    TradeInfo* tmpinfo = new TradeInfo();
    tmpinfo->hopeOpenGap = defineInfo->lastInsBidPrice - defineInfo->forwardInsAskPrice;
    tmpinfo->insComKey = insComKey;
    tmpinfo->orderType = "ds";
    //tmpinfo->frontID = FRONT_ID;
    //tmpinfo->sessionID = SESSION_ID;
    tmpinfo->notActiveSecType = EES_SecType_fut;
    tmpinfo->notActiveInstrumentid = currInstrumentID;
    //tmpinfo->notActiveDirection = "1";
    //tmpinfo->notActiveOffsetFlag = "0";
    tmpinfo->notActiveTradeSide = orderField.m_Side;
    tmpinfo->notActiveClientOrderToken = orderField.m_ClientOrderToken;
    tmpinfo->notActiveOrderInsertPrice = defineInfo->lastInsBidPrice;
    tmpinfo->notActiveIsTraded = false;
    //tmpinfo->notActiveOrderRef = notActiveOrderRef;
    tmpinfo->notActiveVolume =  orderField.m_Qty;
    tmpinfo->notActiveHedgeFlag = hedgeFlag;

    tmpinfo->activeSecType = EES_SecType_fut;
    tmpinfo->activeInstrumentid = fwdInstrumentID;
    //tmpinfo->activeDirection = "0";
    //tmpinfo->activeOffsetFlag = "0";
    tmpinfo->activeTradeSide = EES_SideType_open_long;//buy open
    tmpinfo->activeOrderInsertPrice = defineInfo->forwardInsAskPrice;
    tmpinfo->activeIsTraded = false;
    tmpinfo->activeVolume =  orderField.m_Qty;
    tmpinfo->activeHedgeFlag = hedgeFlag;
    unordered_map<string, list<TradeInfo*>*>::iterator it = willTradeMap.find(insComKey);
    if (it == willTradeMap.end()) {//新添加
        list<TradeInfo*>* tmpList = new list<TradeInfo*>();
        tmpList->push_back(tmpinfo);//最新的组合添加到最后
        willTradeMap[insComKey] = tmpList;
    } else {
        list<TradeInfo*>* tmpList = it->second;
        tmpList->push_back(tmpinfo);
    }
}
/*套利组合前提:买近月，卖远月
正套平仓
近月合约为活跃合约，操作：卖平仓
远月合约为不活跃合约，操作:买平仓
*/
void stopProfitProcesserOne(PriceGapMarketData* defineInfo) {
    //定义好元近月的报单价格
    string currInstrumentID = defineInfo->lastInstrumentID;
    string fwdInstrumentID = defineInfo->forwardInstrumentID;
    string insComKey = defineInfo->insComKey;
    /*
    //string happyID = defineInfo->happyID;
    list<string> orderstr;//先买平仓远月非活跃合约
    orderstr.push_back("InstrumentID=" + fwdInstrumentID);
    orderstr.push_back("RequestID=" + boost::lexical_cast<string>(iRequestID++));
    orderstr.push_back("Direction=0");
    orderstr.push_back("HedgeFlag=" + hedgeFlag);//组合投机套保标 投机 '1'套保 '3'
    string notActiveCloseOffset = getCloseMethod(fwdInstrumentID, "sell");
    orderstr.push_back("OffsetFlag=" + notActiveCloseOffset);//平今平昨标志
    orderstr.push_back("Price=" + boost::lexical_cast<string>(defineInfo->forwardInsAskPrice));//tick暂时硬编码
    orderstr.push_back("Volume=" + boost::lexical_cast<string>(defaultVolume));
    string notActiveOrderRef = boost::lexical_cast<string>(iOrderRef++);
    orderstr.push_back("OrderRef=" + notActiveOrderRef);
    orderstr.push_back("priceType=2");
    orderstr.push_back("timeCondition=1");//立即单IOC
    */
    //shengli
    EES_EnterOrderField orderField;
    memset(&orderField, 0, sizeof(EES_EnterOrderField));
    orderField.m_Tif = EES_OrderTif_IOC;//立即单
    orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
    strcpy(orderField.m_Account, INVESTOR_ID.c_str());
    strcpy(orderField.m_Symbol, fwdInstrumentID.c_str());//modify 2.
    string notActiveCloseOffset = getCloseMethod(fwdInstrumentID, "sell");
    EES_SideType shengliTradeSide = getShengliMethod("0",notActiveCloseOffset);//modify
    orderField.m_Side = shengliTradeSide;//modify 1.sell open
    //orderField.m_Exchange = EES_ExchangeID_shfe;
    orderField.m_SecType = EES_SecType_fut;
    orderField.m_Price = defineInfo->forwardInsAskPrice;//modify 3.
    orderField.m_Qty = defaultVolume;
    orderField.m_ClientOrderToken = ++iOrderRef;
    //ptradeApi->reqOrderInsert(&orderField,"100");
    ptradeApi->reqOrderInsert(&orderField,NULL);

    TradeInfo* tmpinfo = new TradeInfo();
    //tmpinfo->happyID = happyID;
    tmpinfo->stopProfitStatus = "200";//正在被处理
    //tmpinfo->hopeCloseGap = defineInfo->hopeGap;
    tmpinfo->insComKey = insComKey;
    tmpinfo->orderType = "as";
    //tmpinfo->frontID = FRONT_ID;
    //tmpinfo->sessionID = SESSION_ID;
    tmpinfo->notActiveSecType = EES_SecType_fut;
    tmpinfo->notActiveInstrumentid = fwdInstrumentID;
    //tmpinfo->notActiveDirection = "0";
    //tmpinfo->notActiveOffsetFlag = notActiveCloseOffset;
    tmpinfo->notActiveOrderInsertPrice = defineInfo->forwardInsAskPrice;
    tmpinfo->notActiveIsTraded = false;
    //tmpinfo->notActiveOrderRef = notActiveOrderRef;
    tmpinfo->notActiveVolume = orderField.m_Qty;
    tmpinfo->notActiveHedgeFlag = hedgeFlag;
    tmpinfo->notActiveInsertAmount = 1;
    tmpinfo->notActiveClientOrderToken = orderField.m_ClientOrderToken;
    tmpinfo->notActiveTradeSide = orderField.m_Side ;

    tmpinfo->activeSecType = EES_SecType_fut;
    tmpinfo->activeInstrumentid = currInstrumentID;
    //tmpinfo->activeDirection = "1";
    //tmpinfo->activeOffsetFlag = getCloseMethod(currInstrumentID, "buy");
    tmpinfo->activeTradeSide = getShengliMethod("1",getCloseMethod(currInstrumentID, "buy"));
    tmpinfo->activeOrderInsertPrice = defineInfo->lastInsBidPrice;
    tmpinfo->activeIsTraded = false;
    tmpinfo->activeVolume = orderField.m_Qty;
    tmpinfo->activeHedgeFlag = hedgeFlag;
    //processHowManyHoldsCanBeClose(currInstrumentID, "0", notActiveCloseOffset, defaultVolume);
    //processHowManyHoldsCanBeClose(pd_instrument, "1", tmpinfo->activeOffsetFlag, defaultVolume);
    unordered_map<string, list<TradeInfo*>*>::iterator it = stopProfitWillTradeMap.find(insComKey);
    if (it == stopProfitWillTradeMap.end()) {//新添加
        list<TradeInfo*>* tmpList = new list<TradeInfo*>();
        tmpList->push_back(tmpinfo);//最新的组合添加到最后
        stopProfitWillTradeMap[insComKey] = tmpList;
    } else {
        list<TradeInfo*>* tmpList = it->second;
        tmpList->push_back(tmpinfo);
    }
    //stopProfitWillTradeMap[insComKey] = (tmpinfo);
}
/*反套平仓
卖出远月，买入近月
远月为不活跃合约
*/
void stopProfitProcesserTwo(PriceGapMarketData* defineInfo) {
    //定义好元近月的报单价格
    string currInstrumentID = defineInfo->lastInstrumentID;
    string fwdInstrumentID = defineInfo->forwardInstrumentID;
    string insComKey = defineInfo->insComKey;
    /*
    //string happyID = defineInfo->happyID;
    list<string> orderstr;
    //double tmpPrice = pd_askPrice;
    orderstr.push_back("InstrumentID=" + fwdInstrumentID);
    orderstr.push_back("RequestID=" + boost::lexical_cast<string>(iRequestID++));
    orderstr.push_back("Direction=1");//组合投机套保标 投机 '1'套保 '3'
    orderstr.push_back("HedgeFlag=" + hedgeFlag);
    string notActiveCloseOffset = getCloseMethod(fwdInstrumentID, "buy");
    orderstr.push_back("OffsetFlag=" + notActiveCloseOffset);
    orderstr.push_back("Price=" + boost::lexical_cast<string>(defineInfo->forwardInsBidPrice));//tick暂时硬编码
    orderstr.push_back("Volume=" + boost::lexical_cast<string>(defaultVolume));
    string notActiveOrderRef = boost::lexical_cast<string>(iOrderRef++);
    orderstr.push_back("OrderRef=" + notActiveOrderRef);
    orderstr.push_back("priceType=2");
    orderstr.push_back("timeCondition=1");//立即单
    */
    //shengli
    EES_EnterOrderField orderField;
    memset(&orderField, 0, sizeof(EES_EnterOrderField));
    orderField.m_Tif = EES_OrderTif_IOC;//立即单
    orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
    strcpy(orderField.m_Account, INVESTOR_ID.c_str());
    strcpy(orderField.m_Symbol, fwdInstrumentID.c_str());//modify 2.
    string notActiveCloseOffset = getCloseMethod(fwdInstrumentID, "buy");//modify
    EES_SideType shengliTradeSide = getShengliMethod("1",notActiveCloseOffset);//modify
    orderField.m_Side = shengliTradeSide;//modify
    //orderField.m_Exchange = EES_ExchangeID_shfe;
    orderField.m_SecType = EES_SecType_fut;
    orderField.m_Price = defineInfo->forwardInsBidPrice;//modify 3.
    orderField.m_Qty = defaultVolume;
    orderField.m_ClientOrderToken = ++iOrderRef;
    //ptradeApi->reqOrderInsert(&orderField,"100");
    ptradeApi->reqOrderInsert(&orderField,NULL);

    TradeInfo* tmpinfo = new TradeInfo();
    //tmpinfo->happyID = happyID;
    tmpinfo->stopProfitStatus = "200";//正在被处理
    //tmpinfo->hopeCloseGap = defineInfo->hopeGap;
    tmpinfo->insComKey = insComKey;
    tmpinfo->orderType = "ds";
    //tmpinfo->frontID = FRONT_ID;
    //tmpinfo->sessionID = SESSION_ID;
    tmpinfo->notActiveSecType = EES_SecType_fut;
    tmpinfo->notActiveInstrumentid = fwdInstrumentID;
    //tmpinfo->notActiveDirection = "1";
    //tmpinfo->notActiveOffsetFlag = notActiveCloseOffset;
    tmpinfo->notActiveOrderInsertPrice = defineInfo->forwardInsBidPrice;
    tmpinfo->notActiveIsTraded = false;
    //tmpinfo->notActiveOrderRef = notActiveOrderRef;
    tmpinfo->notActiveVolume = orderField.m_Qty;
    tmpinfo->notActiveHedgeFlag = hedgeFlag;
    tmpinfo->notActiveClientOrderToken = orderField.m_ClientOrderToken;
    tmpinfo->notActiveTradeSide = orderField.m_Side ;

    tmpinfo->activeSecType = EES_SecType_fut;
    tmpinfo->activeInstrumentid = currInstrumentID;
    //tmpinfo->activeDirection = "0";
    //tmpinfo->activeOffsetFlag = getCloseMethod(currInstrumentID, "sell");
    tmpinfo->activeTradeSide = getShengliMethod("0",getCloseMethod(currInstrumentID, "sell"));
    tmpinfo->activeOrderInsertPrice = defineInfo->lastInsAskPrice;
    tmpinfo->activeIsTraded = false;
    tmpinfo->activeVolume = orderField.m_Qty;
    tmpinfo->activeHedgeFlag = hedgeFlag;
    //processHowManyHoldsCanBeClose(currInstrumentID, "0", tmpinfo->activeOffsetFlag, defaultVolume);
    //processHowManyHoldsCanBeClose(pd_instrument, "1", notActiveCloseOffset, defaultVolume);
    unordered_map<string, list<TradeInfo*>*>::iterator it = stopProfitWillTradeMap.find(insComKey);
    if (it == stopProfitWillTradeMap.end()) {//新添加
        list<TradeInfo*>* tmpList = new list<TradeInfo*>();
        tmpList->push_back(tmpinfo);//最新的组合添加到最后
        stopProfitWillTradeMap[insComKey] = tmpList;
    } else {
        list<TradeInfo*>* tmpList = it->second;
        tmpList->push_back(tmpinfo);
    }
    //stopProfitWillTradeMap[insComKey] = (tmpinfo);
}
/*正套平仓
远月合约为活跃合约，操作：买平仓
近月合约为不活跃合约，操作:卖平仓*/
void stopProfitProcesserThree(PriceGapMarketData* defineInfo) {
    //定义好元近月的报单价格
    string currInstrumentID = defineInfo->lastInstrumentID;
    string fwdInstrumentID = defineInfo->forwardInstrumentID;
    string insComKey = defineInfo->insComKey;
    //string happyID = defineInfo->happyID;
    /*
    list<string> orderstr;
    //double tmpPrice = currAskPrice;
    orderstr.push_back("InstrumentID=" + currInstrumentID);
    orderstr.push_back("RequestID=" + boost::lexical_cast<string>(iRequestID++));
    orderstr.push_back("Direction=1");//组合投机套保标 投机 '1'套保 '3'
    orderstr.push_back("HedgeFlag=" + hedgeFlag);
    string notActiveCloseOffset = getCloseMethod(currInstrumentID, "buy");
    orderstr.push_back("OffsetFlag=" + notActiveCloseOffset);
    orderstr.push_back("Price=" + boost::lexical_cast<string>(defineInfo->lastInsBidPrice));//tick暂时硬编码
    orderstr.push_back("Volume=" + boost::lexical_cast<string>(defaultVolume));
    string notActiveOrderRef = boost::lexical_cast<string>(iOrderRef++);
    orderstr.push_back("OrderRef=" + notActiveOrderRef);
    orderstr.push_back("priceType=2");
    orderstr.push_back("timeCondition=1");//立即单
    */
    //shengli
    EES_EnterOrderField orderField;
    memset(&orderField, 0, sizeof(EES_EnterOrderField));
    orderField.m_Tif = EES_OrderTif_IOC;//立即单
    orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
    strcpy(orderField.m_Account, INVESTOR_ID.c_str());
    strcpy(orderField.m_Symbol, currInstrumentID.c_str());//modify 2.
    string notActiveCloseOffset = getCloseMethod(currInstrumentID, "buy");//modify
    EES_SideType shengliTradeSide = getShengliMethod("1",notActiveCloseOffset);//modify
    orderField.m_Side = shengliTradeSide;//modify
    //orderField.m_Exchange = EES_ExchangeID_shfe;
    orderField.m_SecType = EES_SecType_fut;
    orderField.m_Price = defineInfo->lastInsBidPrice;//modify 3.
    orderField.m_Qty = defaultVolume;
    orderField.m_ClientOrderToken = ++iOrderRef;
    //ptradeApi->reqOrderInsert(&orderField,"100");
    ptradeApi->reqOrderInsert(&orderField,NULL);

    TradeInfo* tmpinfo = new TradeInfo();
    //tmpinfo->happyID = happyID;
    tmpinfo->stopProfitStatus = "200";//正在被处理
    //tmpinfo->hopeCloseGap = defineInfo->hopeGap;
    tmpinfo->insComKey = insComKey;
    tmpinfo->orderType = "as";
    tmpinfo->notActiveSecType = EES_SecType_fut;
    tmpinfo->notActiveInstrumentid = currInstrumentID;
    tmpinfo->notActiveOrderInsertPrice = defineInfo->lastInsBidPrice;
    tmpinfo->notActiveIsTraded = false;
    tmpinfo->notActiveVolume = orderField.m_Qty;
    tmpinfo->notActiveHedgeFlag = hedgeFlag;
    tmpinfo->notActiveClientOrderToken = orderField.m_ClientOrderToken;
    tmpinfo->notActiveTradeSide = orderField.m_Side ;

    tmpinfo->activeSecType = EES_SecType_fut;
    tmpinfo->activeInstrumentid = fwdInstrumentID;
    //tmpinfo->activeDirection = "0";
    //tmpinfo->activeOffsetFlag = getCloseMethod(fwdInstrumentID, "sell");
    tmpinfo->activeTradeSide = getShengliMethod("0",getCloseMethod(fwdInstrumentID, "sell"));
    tmpinfo->activeOrderInsertPrice = defineInfo->forwardInsAskPrice;
    tmpinfo->activeIsTraded = false;
    tmpinfo->activeVolume = orderField.m_Qty;
    tmpinfo->activeHedgeFlag = hedgeFlag;
    //processHowManyHoldsCanBeClose(currInstrumentID, "1", notActiveCloseOffset, defaultVolume);
    //processHowManyHoldsCanBeClose(pd_instrument, "0", tmpinfo->activeOffsetFlag, defaultVolume);
    unordered_map<string, list<TradeInfo*>*>::iterator it = stopProfitWillTradeMap.find(insComKey);
    if (it == stopProfitWillTradeMap.end()) {//新添加
        list<TradeInfo*>* tmpList = new list<TradeInfo*>();
        tmpList->push_back(tmpinfo);//最新的组合添加到最后
        stopProfitWillTradeMap[insComKey] = tmpList;
    } else {
        list<TradeInfo*>* tmpList = it->second;
        tmpList->push_back(tmpinfo);
    }
    //stopProfitWillTradeMap[insComKey] = (tmpinfo);
}
/*反套平仓
卖出远月，买入近月
近月为不活跃合约
*/
void stopProfitProcesserFour(PriceGapMarketData* defineInfo) {
    //定义好元近月的报单价格
    string currInstrumentID = defineInfo->lastInstrumentID;
    string fwdInstrumentID = defineInfo->forwardInstrumentID;
    string insComKey = defineInfo->insComKey;
    //string happyID = defineInfo->happyID;
    /*
    list<string> orderstr;
    //double tmpPrice = pd_bidPrice;
    orderstr.push_back("InstrumentID=" + currInstrumentID);
    orderstr.push_back("RequestID=" + boost::lexical_cast<string>(iRequestID++));
    orderstr.push_back("Direction=0");//组合投机套保标 投机 '1'套保 '3'
    orderstr.push_back("HedgeFlag=" + hedgeFlag);
    string notActiveCloseOffset = getCloseMethod(currInstrumentID, "sell");
    orderstr.push_back("OffsetFlag=" + notActiveCloseOffset);
    orderstr.push_back("Price=" + boost::lexical_cast<string>(defineInfo->lastInsAskPrice));//tick暂时硬编码
    orderstr.push_back("Volume=" + boost::lexical_cast<string>(defaultVolume));
    string notActiveOrderRef = boost::lexical_cast<string>(iOrderRef++);
    orderstr.push_back("OrderRef=" + notActiveOrderRef);
    orderstr.push_back("priceType=2");
    orderstr.push_back("timeCondition=1");//立即单
    */
    //shengli
    EES_EnterOrderField orderField;
    memset(&orderField, 0, sizeof(EES_EnterOrderField));
    orderField.m_Tif = EES_OrderTif_IOC;//立即单
    orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
    strcpy(orderField.m_Account, INVESTOR_ID.c_str());
    strcpy(orderField.m_Symbol, currInstrumentID.c_str());//modify 2.
    string notActiveCloseOffset = getCloseMethod(currInstrumentID, "sell");//modify
    EES_SideType shengliTradeSide = getShengliMethod("0",notActiveCloseOffset);//modify
    orderField.m_Side = shengliTradeSide;//modify
    //orderField.m_Exchange = EES_ExchangeID_shfe;
    orderField.m_SecType = EES_SecType_fut;
    orderField.m_Price = defineInfo->lastInsAskPrice;//modify 3.
    orderField.m_Qty = defaultVolume;
    orderField.m_ClientOrderToken = ++iOrderRef;
    //ptradeApi->reqOrderInsert(&orderField,"100");
    ptradeApi->reqOrderInsert(&orderField,NULL);

    TradeInfo* tmpinfo = new TradeInfo();
    //tmpinfo->happyID = happyID;
    tmpinfo->stopProfitStatus = "200";//正在被处理
    //tmpinfo->hopeCloseGap = defineInfo->hopeGap;
    tmpinfo->insComKey = insComKey;
    tmpinfo->orderType = "ds";
    //tmpinfo->frontID = FRONT_ID;
    //tmpinfo->sessionID = SESSION_ID;
    tmpinfo->notActiveSecType = EES_SecType_fut;
    tmpinfo->notActiveInstrumentid = currInstrumentID;
    //tmpinfo->notActiveDirection = "0";
    //tmpinfo->notActiveOffsetFlag = notActiveCloseOffset;
    tmpinfo->notActiveOrderInsertPrice = defineInfo->lastInsAskPrice;
    tmpinfo->notActiveIsTraded = false;
    //tmpinfo->notActiveOrderRef = notActiveOrderRef;
    tmpinfo->notActiveVolume = orderField.m_Qty;
    tmpinfo->notActiveHedgeFlag = hedgeFlag;
    tmpinfo->notActiveClientOrderToken = orderField.m_ClientOrderToken;
    tmpinfo->notActiveTradeSide = orderField.m_Side ;

    tmpinfo->activeSecType = EES_SecType_fut;
    tmpinfo->activeInstrumentid = fwdInstrumentID;
    //tmpinfo->activeDirection = "1";
    //tmpinfo->activeOffsetFlag = getCloseMethod(fwdInstrumentID, "buy");
    tmpinfo->activeTradeSide = getShengliMethod("1",getCloseMethod(fwdInstrumentID, "buy"));
    tmpinfo->activeOrderInsertPrice = defineInfo->forwardInsBidPrice;
    tmpinfo->activeIsTraded = false;
    tmpinfo->activeVolume = orderField.m_Qty;
    tmpinfo->activeHedgeFlag = hedgeFlag;
    //processHowManyHoldsCanBeClose(currInstrumentID, "1", tmpinfo->activeOffsetFlag, defaultVolume);
    //processHowManyHoldsCanBeClose(pd_instrument, "0", notActiveCloseOffset, defaultVolume);
    unordered_map<string, list<TradeInfo*>*>::iterator it = stopProfitWillTradeMap.find(insComKey);
    if (it == stopProfitWillTradeMap.end()) {//新添加
        list<TradeInfo*>* tmpList = new list<TradeInfo*>();
        tmpList->push_back(tmpinfo);//最新的组合添加到最后
        stopProfitWillTradeMap[insComKey] = tmpList;
    } else {
        list<TradeInfo*>* tmpList = it->second;
        tmpList->push_back(tmpinfo);
    }
    //stopProfitWillTradeMap[insComKey] = (tmpinfo);
}
/*是否可以下止盈单
正套*/
bool canBeClose(string pd_instr_key,string orderType) {
    vector<string> ins = split(pd_instr_key, "-");
    //有可平量才下单.key按照近月-远月的格式;同时近月都是买持仓，远月都是卖持仓
    unordered_map<string, HoldPositionInfo*>::iterator it = positionmap.find(ins[0]);
    if (it != positionmap.end()) {
        HoldPositionInfo* tmpinfo = it->second;
        if (orderType == "as") {//正套的持仓，近月都是买持仓，远月都是卖持仓
            if (tmpinfo->longAvaClosePosition < defaultVolume) {
                //可平持仓不足
                if(isLogout){
                    LOG(ERROR) << "合约:" + ins[0] + "买持仓可平量=" + boost::lexical_cast<string>(tmpinfo->longAvaClosePosition) + "小于平仓量，不能止盈平仓.";
                }
                return false;
            } else {
                if(isLogout){
                    LOG(INFO) << "合约:" + ins[0] + "买持仓可平量=" + boost::lexical_cast<string>(tmpinfo->longAvaClosePosition);
                }

            }
        } else {//反套的持仓，近月都是卖持仓，远月都是买持仓
            if (tmpinfo->shortAvaClosePosition < defaultVolume) {
                //可平持仓不足
                if(isLogout){
                    LOG(ERROR) << "合约:" + ins[0] + "卖持仓可平量=" + boost::lexical_cast<string>(tmpinfo->shortAvaClosePosition) + "小于平仓量，不能止盈平仓.";
                }

                return false;
            } else {
                if(isLogout){
                    LOG(INFO) << "合约:" + ins[0] + "卖持仓可平量=" + boost::lexical_cast<string>(tmpinfo->shortAvaClosePosition);
                }

            }
        }
    } else {
        //无持仓数据
        LOG(INFO) << "合约:" + ins[0] + "当前无持仓.";
        return false;
    }
    it = positionmap.find(ins[1]);
    if (it != positionmap.end()) {
        HoldPositionInfo* tmpinfo = it->second;
        if (orderType == "as") {//正套的持仓，近月都是买持仓，远月都是卖持仓
            if (tmpinfo->shortAvaClosePosition < defaultVolume) {
                //可平持仓不足
                if(isLogout){
                    LOG(ERROR) << "合约:" + ins[1] + "卖持仓可平量=" + boost::lexical_cast<string>(tmpinfo->shortAvaClosePosition) + "小于平仓量，不能止盈平仓.";
                }

                return false;
            } else {
                if(isLogout){
                    LOG(INFO) << "合约:" + ins[1] + "卖持仓可平量=" + boost::lexical_cast<string>(tmpinfo->shortAvaClosePosition);
                }
            }
        } else {//反套的持仓，近月都是卖持仓，远月都是买持仓
            if (tmpinfo->longAvaClosePosition < defaultVolume) {
                //可平持仓不足
                if(isLogout){
                    LOG(ERROR) << "合约:" + ins[1] + "买持仓可平量=" + boost::lexical_cast<string>(tmpinfo->longAvaClosePosition) + "小于平仓量，不能止盈平仓.";
                }
                return false;
            } else {
                if(isLogout){
                    LOG(INFO) << "合约:" + ins[1] + "买持仓可平量=" + boost::lexical_cast<string>(tmpinfo->longAvaClosePosition);
                }

            }
        }

    } else {
        //无持仓数据
        LOG(INFO) << "合约:" + ins[1] + "当前无持仓.";
        return false;
    }
    PriceGap* pg = getPriceGap(pd_instr_key);
    int arbComVolume = 0;//组合单手数上限
    int arbComVolMetric = 0;//
    if (orderType == "as") {
        arbComVolume = pg->arbComVolumeAS;//组合单手数上限
        arbComVolMetric = pg->arbComVolMetricAS;//
    } else if (orderType == "ds") {
        arbComVolume = pg->arbComVolumeDS;//组合单手数上限
        arbComVolMetric = pg->arbComVolMetricDS;//
    }
    unordered_map<string, list<TradeInfo*>*>::iterator untradeinfo_stopprofit = stopProfitWillTradeMap.find(pd_instr_key);
    if (untradeinfo_stopprofit != stopProfitWillTradeMap.end()) {//说明存在未成交止盈单，不需要重复下单
        list<TradeInfo*>* stopList = untradeinfo_stopprofit->second;
        int untradeCount = stopList->size();//未成交笔数
        int untradeVols = 0;//未成交的手数
        string tmpstr = "";
        for (list<TradeInfo*>::iterator it = stopList->begin(); it != stopList->end(); it++) {
            TradeInfo* tmpTradeInfo = *it;
            untradeVols += tmpTradeInfo->notActiveVolume;//
            tmpstr += getArbDetail(tmpTradeInfo) + ";";
        }
        //第一个是笔数，第二个是手数
        if (untradeCount >= maxUntradeNums) {//超过最大未成交组合的数量，则不能继续下单，防止由于未收到成交回报等原因导致的报单报入超限
            if(isLogout){
                LOG(INFO) << "行情触发止盈条件，但是止盈单未成交数量:" + boost::lexical_cast<string>(untradeCount) + " 超过最大未成交组合的数量:" + boost::lexical_cast<string>(maxUntradeNums) + "，则不能继续下单，防止由于未收到成交回报等原因导致的报单报入超限" + ",等待成交止盈单=" + tmpstr;
            }
            return false;
        } else if (untradeVols >= arbComVolume) {
            if(isLogout){
                LOG(INFO) << "行情触发止盈条件,但是已经超过持仓限制,不能下单。当前持仓=" + boost::lexical_cast<string>(arbComVolume) + ",未成交手数=" + boost::lexical_cast<string>(untradeVols);
            }

            return false;
        } else {
            if(isLogout){
                LOG(INFO) << "行情触发止盈条件,未超过持仓限制,可以下单。当前持仓=" + boost::lexical_cast<string>(arbComVolume) + ",未成交手数=" + boost::lexical_cast<string>(untradeVols);
            }
            return true;
        }
    } else {
        if(isLogout){
            LOG(INFO) << "止盈单已经成交或者无止盈单组合，继续下单.当前止盈列表情况; " + getAllStopProfitInfo();
        }
        //查看当前是否有可下单持仓
        return true;
    }
}

/*执行平仓操作，需要查询持仓情况。做出平今平昨决定。只针对上期所品种.
参数：instrumentID 要查询对手的合约
type 持仓类型.执行卖平仓操作时,输入buy,表示要查询多头的持仓情况.执行买平仓操作时,输入sell，表示要查询空头的持仓情况.
返回值:开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'*/
string getCloseMethod(string instrumentID, string type) {
    //boost::recursive_mutex::scoped_lock SLock(pst_mtx);
    unordered_map<string, HoldPositionInfo*>::iterator map_iterator = positionmap.find(instrumentID);
    if (map_iterator != positionmap.end()) {//查找到合约持仓
        HoldPositionInfo* hold = map_iterator->second;
        string tmpmsg;
        int realShortPstLimit = hold->shortTotalPosition;
        int realLongPstLimit = hold->longTotalPosition;
        int avlpstLong = hold->longAvaClosePosition;
        int avlpstShort = hold->shortAvaClosePosition;
        int shortYdPst = hold->shortYdPosition;
        int longYdPst = hold->longYdPosition;
        if (type == "buy") {
            //calculate
            int untradeYdVolume = 0;
            int untradeVolume = 0;
            for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();it++){
                OrderInfo* orderInfo = *it;
                if(orderInfo->direction == "1" && orderInfo->offsetFlag == "4"){//close yesterday,but not traded now
                    untradeYdVolume += orderInfo->volume;
                }
                if(orderInfo->direction == "1"){
                    untradeVolume += orderInfo->volume;
                }
            }
            LOG(INFO) << "untradeVolume in askList is " + boost::lexical_cast<string>(untradeVolume) + "untradeYdVolume in askList is " + boost::lexical_cast<string>(untradeYdVolume) + ",longYdPst=" + boost::lexical_cast<string>(longYdPst) +
                          ",avlpstLong=" + boost::lexical_cast<string>(avlpstLong);
            if (longYdPst - untradeYdVolume > 0) {
                return "4";//平昨
            } else if (avlpstLong > 0) {
            //} else if (realLongPstLimit > 0) {
                return "3";//平今，上期所品种
            } else {
                LOG(ERROR) << "合约无买持仓数据，无法判断平仓方式.instrumentID=" + instrumentID+",change close to open.";
                return "0";//
                //boost::this_thread::sleep(boost::posix_time::seconds(3));
                //Sleep(1000);
                //return "-1";
            }
        } else if (type == "sell") {
            //calculate
            int untradeYdVolume = 0;
            int untradeVolume = 0;
            for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();it++){
                OrderInfo* orderInfo = *it;
                if(orderInfo->direction == "0" && orderInfo->offsetFlag == "4"){//close yesterday,but not traded now
                    untradeYdVolume += orderInfo->volume;
                }
                if(orderInfo->direction == "0"){
                    untradeVolume += orderInfo->volume;
                }
            }
            LOG(INFO) << "untradeVolume in bidList is " + boost::lexical_cast<string>(untradeVolume) + "untradeYdVolume in bidList is " + boost::lexical_cast<string>(untradeYdVolume) + ",shortYdPst=" + boost::lexical_cast<string>(shortYdPst) +
                          ",avlpstShort=" + boost::lexical_cast<string>(avlpstShort);
            if (shortYdPst - untradeYdVolume > 0) {
                return "4";//平昨
            } else if (avlpstShort > 0) {
                return "3";//平今，上期所品种
            } else {
                LOG(ERROR) << "合约无卖持仓数据，无法判断平仓方式.instrumentID=" + instrumentID + ",change close to open.";
                return "0";//
                //boost::this_thread::sleep(boost::posix_time::seconds(3));
                //Sleep(1000);
                //return "-1";
            }
        }
    } else {
        LOG(ERROR) << "查找不到合约的持仓信息:instrumentID=" + instrumentID;
    }
}
void getBidAskOrderListInfo(unsigned int clientOrderToken,OrderInfo* rtnOrderInfo){
    unsigned int  cldClientOrderToken = clientOrderToken;
    for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();){
        OrderInfo* orderInfo = *it;
        unsigned int clientOrderToken = orderInfo->clientOrderToken;
        if(clientOrderToken == cldClientOrderToken){
            rtnOrderInfo->direction = orderInfo->direction;
            rtnOrderInfo->offsetFlag = orderInfo->offsetFlag;
            rtnOrderInfo->volume = orderInfo->volume;
            rtnOrderInfo->price = orderInfo->price;
            rtnOrderInfo->clientOrderToken = orderInfo->clientOrderToken;
            LOG(INFO) << "bidlist:find order,return.";
            return;
        }else{
            it++;
        }
    }
    for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();){
        OrderInfo* orderInfo = *it;
        unsigned int clientOrderToken = orderInfo->clientOrderToken;
        if(clientOrderToken == cldClientOrderToken){
            rtnOrderInfo->direction = orderInfo->direction;
            rtnOrderInfo->offsetFlag = orderInfo->offsetFlag;
            rtnOrderInfo->volume = orderInfo->volume;
            rtnOrderInfo->price = orderInfo->price;
            rtnOrderInfo->clientOrderToken = orderInfo->clientOrderToken;
            LOG(INFO) << "askList:find order,return.";
            break;
        }else{
            it++;
        }
    }
}

/*error return 0*/
EES_SideType getShengliMethod(string direction,string offsetFlag){
    //开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'*
    /*
    typedef unsigned char EES_SideType;						///< 买卖方向
    #define EES_SideType_open_long                  1		///< =买单（开今）
    #define EES_SideType_close_today_long           2		///< =卖单（平今）
    #define EES_SideType_close_today_short          3		///< =买单（平今）
    #define EES_SideType_open_short                 4		///< =卖单（开今）
    #define EES_SideType_close_ovn_short            5		///< =买单（平昨）
    #define EES_SideType_close_ovn_long             6		///< =卖单（平昨）
    #define EES_SideType_force_close_ovn_short      7		///< =买单 （强平昨）
    #define EES_SideType_force_close_ovn_long       8		///< =卖单 （强平昨）
    #define EES_SideType_force_close_today_short    9		///< =买单 （强平今）
    #define EES_SideType_force_close_today_long     10		///< =卖单 （强平今）
    #define EES_SideType_opt_exec					11		///< =期权行权
    */
    if(direction == "0"){//buy
        if(offsetFlag == "0"){// open
            return EES_SideType_open_long;
        }else if(offsetFlag == "3"){//close today
            return EES_SideType_close_today_short;
        }else if(offsetFlag == "4"){//close yesteday
            return EES_SideType_close_ovn_short;
        }else{
            LOG(ERROR) << "offsetFlag=" + offsetFlag + ",not identified this flag!!!";
            return 0;
        }
    }else if(direction == "1"){//sell
        if(offsetFlag == "0"){// open
            return EES_SideType_open_short;
        }else if(offsetFlag == "3"){//close today
            return EES_SideType_close_today_long;
        }else if(offsetFlag == "4"){//close yesteday
            return EES_SideType_close_ovn_long;
        }else{
            LOG(ERROR) << "offsetFlag=" + offsetFlag + ",not identified this flag!!!";
            return 0;
        }
    }else{
        LOG(ERROR) << "direction=" + direction + ",not identified this flag!!!";
        return 0;
    }
}
string num2str(double i)
{
        stringstream ss;
        ss<<i;
        return ss.str();
}
void computeDualTrustPara(double lastPrice){
    //max(max-close,close-min)
    double range=max(techCls.trueKData15S->highPrice-techCls.trueKData15S->closePrice,techCls.trueKData15S->closePrice-techCls.trueKData15S->lowPrice);
    range=0;
    techCls.limit[0]=lastPrice-techCls.K2*range;
    techCls.limit[1]=lastPrice+techCls.K1*range;
    LOG(INFO) << "Init dual trust parameters.range="+num2str(range)+",low limit="+num2str(techCls.limit[0])+",up limit="+num2str(techCls.limit[1]);
}

void initOverTickNums(TechMetric* tm,string insComKey){
    //组合持仓开平参数设置
    if (insComKey == "cu1802-cu1803") {
        tm->overMAGapTickNums = 1;
        tm->downMAGapTickNums = 1;
    } else if (insComKey == "pp1801-pp1805") {
        tm->overMAGapTickNums = 5.5;
        tm->downMAGapTickNums = 5.5;
    } else if (insComKey == "ru1805-ru1809") {
        tm->overMAGapTickNums = 1;
        tm->downMAGapTickNums = 1;
    } else if (insComKey == "rb1801-rb1805") {
        tm->overMAGapTickNums = 1;
        tm->downMAGapTickNums = 1;
    } else if (insComKey == "i1801-i1805") {
        tm->overMAGapTickNums = 4;
        tm->downMAGapTickNums = 4;
    } else if (insComKey == "MA801-MA805") {
        tm->overMAGapTickNums = 4.3;
        tm->downMAGapTickNums = 4.3;
    } else if (insComKey == "hc1801-hc1805") {
        tm->overMAGapTickNums = 3;
        tm->downMAGapTickNums = 3.3;
    } else if (insComKey == "ni1805-ni1809") {
        tm->overMAGapTickNums = 1;
        tm->downMAGapTickNums = 1;
    } else if (insComKey == "jm1801-jm1805") {
        tm->overMAGapTickNums = 3.8;
        tm->downMAGapTickNums = 3.8;
    }else if (insComKey == "TA801-TA805") {
        tm->overMAGapTickNums = 3;
        tm->downMAGapTickNums = 3;
    }else if (insComKey == "CF801-CF805") {
        tm->overMAGapTickNums = 1.8;
        tm->downMAGapTickNums = 1.8;
    }else if (insComKey == "m1801-m1805") {
        tm->overMAGapTickNums = 1.5;
        tm->downMAGapTickNums = 1.5;
    }else if (insComKey == "j1801-j1805") {
        tm->overMAGapTickNums = 5.4;
        tm->downMAGapTickNums = 5.5;
    }
}
void initSunOrShadowLine(string direction){
    //init shadow and sun line
    if(techCls.trueKData15S->closePrice > techCls.trueKData15S->openPrice){
        techCls.firstOpenKLineType="1";
        LOG(INFO) << "this 15s k line is a sun line!closePrice="+boost::lexical_cast<string>(techCls.trueKData15S->closePrice)+
                      ",openPrice="+boost::lexical_cast<string>(techCls.trueKData15S->openPrice);
    }else if(techCls.trueKData15S->closePrice < techCls.trueKData15S->openPrice){
        techCls.firstOpenKLineType="2";
        LOG(INFO) << "this 15s k line is a shadow line!closePrice="+boost::lexical_cast<string>(techCls.trueKData15S->closePrice)+
                      ",openPrice="+boost::lexical_cast<string>(techCls.trueKData15S->openPrice);
    }else{
        if(direction=="0"){
            techCls.firstOpenKLineType="1";
        }else{
            techCls.firstOpenKLineType="2";
        }
        LOG(INFO) << "this 15s k line is a flat line!closePrice="+boost::lexical_cast<string>(techCls.trueKData15S->closePrice)+
                      ",openPrice="+boost::lexical_cast<string>(techCls.trueKData15S->openPrice);
    }
}
//判断加仓是否止盈出局,base on hold position hold cost.
bool stopProfit(string direction,double lastPrice,string instrumentID){
    /*HoldPositionInfo* tmpinfo;
    unordered_map<string, HoldPositionInfo*>::iterator it=reversePosition.find(instrumentID);
    if(it == reversePosition.end()){
        LOG(INFO)<<"can't find instrumentID="+instrumentID+" reverse position.";
        return false;
    }else{
        tmpinfo= it->second;
    }*/
    double tickPrice=getPriceTick(instrumentID);
    if(direction=="0"){//多头逆向加仓
        if((lastPrice-userHoldPst.longHoldAvgPrice)>=techCls.lrsptn*tickPrice){
            LOG(INFO)<<"lastPrice="+boost::lexical_cast<string>(lastPrice)
                        +"-longHoldAvgPrice="+boost::lexical_cast<string>(userHoldPst.longHoldAvgPrice)
                        +">="+boost::lexical_cast<string>(techCls.lrsptn*tickPrice);
            OrderInfo orderInfo;
            if(existUntradeOrder("2001",&orderInfo)){
                LOG(INFO)<<"There are untrade order for long reverse stop profit,not process.";
                return true;
            }else{
                LOG(INFO)<<"多头逆向止盈出局,下平仓单";
                userHoldPst.allPstClean="1";
                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                addinfo->openStgType="2001";
                addNewOrderTrade(instrumentID,"1","1",lastPrice,userHoldPst.longTotalPosition,"0",addinfo);
                return true;
            }
        }else{
            return false;
        }
    }else if(direction=="1"){//空头逆向加仓
        if((userHoldPst.shortHoldAvgPrice-lastPrice)>=techCls.srsptn*tickPrice){
            LOG(INFO)<<"shortHoldAvgPrice="+boost::lexical_cast<string>(userHoldPst.shortHoldAvgPrice)
                        +"-lastPrice="+boost::lexical_cast<string>(lastPrice)
                        +">="+boost::lexical_cast<string>(techCls.srsptn*tickPrice);
            OrderInfo orderInfo;
            if(existUntradeOrder("1001",&orderInfo)){
                LOG(INFO)<<"There are untrade order for short reverse stop profit,not process.";
                return true;
            }else{
                LOG(INFO)<<"空头逆向止盈出局,下平仓单";
                userHoldPst.allPstClean="1";
                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                addinfo->openStgType="1001";
                addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                return true;
            }
        }else{
            return false;
        }
    }else{
        LOG(ERROR)<<"错误的加仓类型,direction="+direction;
        return false;
    }
}
void processOtherOpen(OrderFieldInfo* realseInfo,list<WaitForCloseInfo*>* userPstList){
    bool thingFound = false;
    WaitForCloseInfo* wfcInfo;
    for(list<WaitForCloseInfo*>::iterator wfcIT = userPstList->begin();wfcIT != userPstList->end();){
        wfcInfo = *wfcIT;
        if(wfcInfo->marketOrderToken == realseInfo->marketOrderToken){//
            LOG(INFO) << "Find first order,change volume from "+boost::lexical_cast<string>(wfcInfo->tradeVolume)+" to "
                              +boost::lexical_cast<string>(wfcInfo->tradeVolume+realseInfo->tradeVolume);
            wfcInfo->tradeVolume += realseInfo->tradeVolume;
            thingFound = true;
            break;
        }else{
            wfcIT ++;
        }
    }
    if(!thingFound){
        LOG(ERROR) << "ERROR:as can't find marketOrderToken= " + boost::lexical_cast<string>(realseInfo->marketOrderToken) + " from allTradeList!";
    }
}
void processClose(OrderFieldInfo* realseInfo,list<WaitForCloseInfo*>* userPstList){
    int reserve = realseInfo->tradeVolume;
    for(list<WaitForCloseInfo*>::iterator wfcIT = userPstList->begin();wfcIT != userPstList->end();){
        WaitForCloseInfo* wfcInfo = *wfcIT;
        if(wfcInfo->tradeVolume > reserve){
            wfcInfo->tradeVolume -= reserve;
            break;
        }else if(wfcInfo->tradeVolume == reserve){
            userPstList->erase(wfcIT);
            break;
        }else if(wfcInfo->tradeVolume < reserve){
            reserve -= wfcInfo->tradeVolume;
            wfcIT = userPstList->erase(wfcIT);
        }
    }
}

void processUserHoldPosition(OrderFieldInfo* realseInfo,list<WaitForCloseInfo*>* userPstList){
    bool thingFound = false;
    WaitForCloseInfo* wfcInfo;
    for(list<WaitForCloseInfo*>::iterator wfcIT = userPstList->begin();wfcIT != userPstList->end();){
        wfcInfo = *wfcIT;
        if(wfcInfo->marketOrderToken == realseInfo->marketOrderToken){//
            if(realseInfo->OffsetFlag=="0"){//open
                LOG(INFO) << "This is first orders's other execution,change volume from "+boost::lexical_cast<string>(wfcInfo->tradeVolume)+" to "
                              +boost::lexical_cast<string>(wfcInfo->tradeVolume+realseInfo->tradeVolume);
                wfcInfo->tradeVolume += realseInfo->tradeVolume;

            }else {//close
                LOG(INFO) << "This is first orders's other execution,change volume from "+boost::lexical_cast<string>(wfcInfo->tradeVolume)+" to "
                              +boost::lexical_cast<string>(wfcInfo->tradeVolume-realseInfo->tradeVolume);
                wfcInfo->tradeVolume -= realseInfo->tradeVolume;
                if(wfcInfo->tradeVolume == 0){
                    LOG(INFO) << "All order has been closed,delete this record.";
                    userPstList->erase(wfcIT);
                }else{
                    LOG(INFO) << "Not all order has been closed.originalVolume"+boost::lexical_cast<string>(wfcInfo->originalVolume)
                                  +",tradeVolume="+boost::lexical_cast<string>(wfcInfo->tradeVolume);
                }
            }
            thingFound = true;
            break;
        }else{
            wfcIT ++;
        }
    }
    if(!thingFound){
        LOG(ERROR) << "ERROR:as can't find marketOrderToken= " + boost::lexical_cast<string>(realseInfo->marketOrderToken) + " from allTradeList!";
    }
}
void cancelSpecTypeOrder(string instrumentID,string type){
    for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();){
        OrderInfo* orderInfo = *it;
        string openStgType = orderInfo->openStgType;
        if(openStgType==type){
            LOG(INFO) << "bidlist:find order,openStgType="+type;
            if(orderInfo->status=="0"){
                LOG(INFO) << "There are untrade order,and not action.So we will execute order action.";
                tryOrderAction(instrumentID,orderInfo,"1");
                orderInfo->status="1";
            }
        }else{
            it++;
        }
    }
    for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();){
        OrderInfo* orderInfo = *it;
        string openStgType = orderInfo->openStgType;
        if(openStgType==type){
            LOG(INFO) << "asklist:find order,openStgType="+type;
            if(orderInfo->status=="0"){
                LOG(INFO) << "There are untrade order,and not action.So we will execute order action.";
                tryOrderAction(instrumentID,orderInfo,"1");
                orderInfo->status="1";
            }
        }else{
            it++;
        }
    }
}
void computeUserHoldPositionInfo(list<WaitForCloseInfo*> *sourList){
    for(list<WaitForCloseInfo*>::iterator wfcIT = sourList->begin();wfcIT != sourList->end();wfcIT ++){
        WaitForCloseInfo* wfcInfo = *wfcIT;
        if(wfcInfo->direction == "0"){//long position
            userHoldPst.longTotalPosition += wfcInfo->tradeVolume;
            userHoldPst.longAmount += wfcInfo->tradeVolume*wfcInfo->openPrice;
            userHoldPst.longHoldAvgPrice = userHoldPst.longAmount/(userHoldPst.longTotalPosition);
        }else if(wfcInfo->direction == "1"){//short position
            userHoldPst.shortTotalPosition += wfcInfo->tradeVolume;
            userHoldPst.shortAmount += wfcInfo->tradeVolume*wfcInfo->openPrice;
            userHoldPst.shortHoldAvgPrice = userHoldPst.shortAmount/(userHoldPst.shortTotalPosition);
        }else{
            LOG(ERROR)<<"ERROR:wrong type diretion="+wfcInfo->direction;
        }

    }

}
void coverYourAss(){
    userHoldPst.allPstClean="2";
    LOG(INFO)<<"All long position has been cleaned.do something of reseting.";
    techCls.priceStatus="0";
    techCls.stgStatus="0";
    techCls.minPrice=0;
    techCls.maxPrice=0;
    techCls.firstOpenKLineType="0";
    Strategy::Kdata tmp=techCls.KData_15s.back();
    techCls.KData_15s.clear();
    techCls.KData_15s.push_back(tmp);
    LOG(INFO)<<"current k 15s line size="+boost::lexical_cast<string>(techCls.KData_15s.size());
}

void resetK15sData(){
    if(userHoldPst.allPstClean=="2"){
        techCls.beginK15s = false;
        techCls.KData_15s.clear();
    }else if(userHoldPst.longTotalPosition==0&&userHoldPst.shortTotalPosition==0){
        techCls.beginK15s = false;
        techCls.KData_15s.clear();
    }

}
