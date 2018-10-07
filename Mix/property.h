#ifndef SALESITEM_H
#define SALESITEM_H
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include<iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif  // _WIND32
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread_pool.hpp>
#include <boost/thread.hpp>
#include <boost/atomic/atomic.hpp>
#include <chrono>
#include <iconv.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <glog/logging.h>
#include <glog/log_severity.h>
#include <boost/locale.hpp>
#include "EESTraderDemo.h"
#include "EESQuoteDefine.h"
#include "Strategy.h"
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
using namespace std;
inline unsigned int PthreadSelf()
{
#ifdef WIN32
    return::GetCurrentThreadId();
#else
    return pthread_self();
#endif
}
class CChineseCode{
public:
    static void UTF_8ToGB2312(string &pOut, char *pText, int pLen);//UTF-8 转为 GB2312
};
//日志信息类
class LogMsg {
public:
    LogMsg() {}
    string& getMsg() {
        return strmsg;
    }
    void setMsg(string msg) {
        strmsg = msg;
    }
    int& GetData(){
        return m_iData;
    }
    int& GetID() {
        return ID;
    }
    void setID(int msgid) {
        ID = msgid;
    }
    ~LogMsg() {}
private:
    int ID = 0;
    int m_iData;
    string m_szDataString;
    string strmsg;
    //char m_szDataString[MAX_DATA_SIZE];
};
// 代码转换操作类
class CodeConverter {
private:
    iconv_t cd;
public:
    // 构造
    CodeConverter(const char *from_charset,const char *to_charset) {
        cd = iconv_open(to_charset,from_charset);
    }
// 析构
    ~CodeConverter() {
    iconv_close(cd);
    }
    // 转换输出
    int convert(char *inbuf,int inlen,char *outbuf,int outlen) {
        char **pin = &inbuf;
        char **pout = &outbuf;
        memset(outbuf,0,outlen);
        return iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
    }
};
/*original order info,
1.created on order inserting
2.add marketToken when receive response from shengli plantform*/
class OriginalOrderFieldInfo{
public:
    string  investorID;						///< 用户代码
    //long  side;							///< 买卖方向
    unsigned char m_Side;
    unsigned char  exchangeID;						///< 交易所
    string  instrumentID;						///< 合约代码
    double              m_Price;						///< 价格
    double realPrice;
    unsigned int        volumeTotalOriginal;							///< 数量
    int totalTradeVolume=0;//
    unsigned int realVolume=0;//current trade
    unsigned int		clientOrderToken;				///< 整型，必须保证，这次比上次的值大，并不一定需要保证连续
    unsigned int		m_Tif;							///< 当需要下FAK/FOK报单时，需要设置为EES_OrderTif_IOC
    unsigned int		marketOrderToken;//response to shengli plantform
    string orderType;//"0":buy open;"1":sell open;"01":buy close;"11":sell close;"2":stop profit;"3":stop loss
    string mkType;//"agg":aggressive mm;"pas":passive mm;"0":
    string orderStatus = "a";//"a":submit to shengli;"1":submit to exchange;"2":traded
    string openStgType;//"3000":first open;priceStatus,"2":step into 2 tick trigger;"1":step into ntickMoveSL.
    bool isFirstOpen=false;//cooperation with openStgType="3000"
    string orderSysID;
    int m_SecType;
    int m_HedgeFlag;
    string function = "0";
    string timeFlag = "";//this will be used for control order insert frequency
};
/*control order insert time period.either by control insert times or control last time.*/
class ControlOrderInfo{
public:
    struct timeval timeStart;//when market data arrive,and at that time begin to insert order.
    int howManyOrderLaugh=0;
};

/*info of order,not info of trade*/
class OrderFieldInfo{
public:
    string OrderLocalID;
    string OrderSubmitStatus;
    int SessionID;
    string InstrumentID;
    string OrderRef;
    int FrontID;
    string OffsetFlag;
    string Direction;
    double VolumeTotalOriginal;
    string OrderStatus;
    string OrderSysID;
    double Price;
    string orderType;//"0":buy open;"1":sell open;"01":buy close;"11":sell close;"2":stop profit;"3":stop loss
    //string mkType;//"agg":aggressive mm;"pas":passive mm;"0":
    unsigned int marketOrderToken;
    unsigned int clientOrderToken;
    string openStgType;
    int tradeVolume;
    string investorID;

};
class InstrumentFieldInfo{
public:
    string ExchangeID;
    double PriceTick;
    string InstrumentID;
    double VolumeMultiple;
};
class DepthMarketDataFieldInfo{
public:
    double LastPrice;
    string InstrumentID;
};

/*info of trade,not info of order*/
class TradeFieldInfo{
public:
    string OffsetFlag;
    string Direction;
    string InstrumentID;
    string OrderRef;
    string TradeTime;
    string TradingDay;
    string TradeID;
    int VolumeTotalOriginal;//originao order volume
    double Price;//trade price
    double oriPrice;//original order price
    string orderType;
    string OrderSysID;
    int Volume;//real trade volume
    unsigned int marketToken;
    unsigned int clientOrderToken;
};


/*持仓信息情况*/
class HoldPositionInfo {
public:
    int shortTotalPosition;//空头总持仓
    int longTotalPosition;//多头总持仓
    int shortYdPosition;//空头昨持仓
    int longYdPosition;//多头昨持仓
    int shortTdPosition;//空头今持仓
    int longTdPosition;//多头今持仓
    int shortAvaClosePosition;//空头可平量
    int longAvaClosePosition;//多头可平量
    double longAmount;//多头持仓交易金额
    double shortAmount;//空头持仓交易金额
    double shortHoldAvgPrice;//空头持仓均价
    double longHoldAvgPrice;//多头持仓均价
    string allPstClean="0";//"0":default,not clean;"1": cleaning;"2":all clean

};
/*持仓明细情况*/
class HoldPositionDetail {
public:
    ///合约代码
    string	instrumentID = "";
    ///经纪公司代码
    string	brokerID = "";
    ///投资者代码
    string	investorID = "";
    ///投机套保标志
    string	hedgeFlag = "";
    ///买卖
    string	direction = "";
    ///开仓日期
    string	openDate = "";
    ///成交编号
    string	tradeID = "";
    ///数量
    int	volume = 0;
    ///开仓价
    double	openPrice = 0.0;
    ///交易日
    string	tradingDay = "";
    ///结算编号
    int	settlementID = 0;
    ///成交类型
    string	orderType = "";
    string mkType = "";
    ///交易所代码
    string	exchangeID = "";
    string pl="0";//which close method used for this hold position;profit or loss
    unsigned int closeClientOrderToken;
    unsigned int closeMarketOrderToken;
    struct timeval holdTimeStart;//how long time this position hold;used for evaluate when to close position.
};

/*根据行情判断当前的动作*/
class MarketPriceAction {
public:
    double lastInstrInsertPrice = 0;//近月合约下单价格
    double forwardInstrInsertPrice = 0;//远月合约下单价格
    string orderType = "0";//报单类型
    string happyID = "";//止盈对应的套利单标识
    double hopeGap = 0;//期望gap
};
/*组合报单信息*/
class TradeInfo {
public:
    string tradingDay = "";
    string tradeTime = "";
    boost::atomic_int32_t orderSeq;//组合报单序号
    string systemID = "";//系统编号，每个产品一个编号
    string stopProfitStatus = "100";//100，等待中，可以下单平仓;200，止盈中，不能继续下单
    string insComKey = "";//组合合约标志
    string happyID = "";//组合套利单全部成交后建立，用于后续止盈时的配对查找标志.
    string closeID = "";//used for close
    double badCloseGap = 0;//stop loss
    double hopeOpenGap = 0;//触发下单时候的gap
    double realOpenGap = 0;//实际成交时候的gap
    double hopeCloseGap = 0;//触发下单时候的止盈gap
    double realCloseGap = 0;//止盈gap
    string orderType = "as";//as表示正套;ds反套
    //bool isRtnOrderOk = false;
    //100,110,0这些状态都不能继续撤单.100避免回报未回来延时，所导致撤单查找不到
    string activeOrderActionStatus = "100";//活跃合约撤单状态：0,表示成交；100，初始,发送活跃合约报单，还未收到回报；110，撤单中；120，已撤单，追单中;其他，表示收到报单的回报状态
    int followTimes = 0;//追单次数
    int notActiveInsertAmount = 0;//不活跃合约FOK报单下单次数，超过次数组合会被删除.重复下单次数
    int sessionID;
    int frontID;
    unsigned int notActiveMarketOrderToken = 0;//not active id
    unsigned int notActiveClientOrderToken = 0;//not active id
    EES_SecType notActiveSecType;
    EES_SideType notActiveTradeSide;//shengli direction and offsetFlag
    string notActiveInstrumentid;
    string notActiveDirection;
    double notActiveOrderInsertPrice = 0;
    double notActiveRealOpenPrice = 0;//不活跃合约实际开仓成交价格
    double notActiveRealClosePrice = 0;//不活跃合约实际平仓成交价格
    bool notActiveIsTraded;
    string notActiveHedgeFlag;
    string notActiveOffsetFlag;
    string notActiveOrderRef;
    int notActiveVolume;
    ///报单提交状态
    //TThostFtdcOrderSubmitStatusType	notActiveOrderSubmitStatus;
    string notActiveOrderSubmitStatus;
    ///报单状态
    //TThostFtdcOrderStatusType	notActiveOrderStatus;
    string notActiveOrderStatus;
    ///本地报单编号
    //TThostFtdcOrderLocalIDType	notActiveOrderLocalID;
    string notActiveOrderLocalID;
    ///报单编号
    //TThostFtdcOrderSysIDType	notActiveOrderSysID;
    string notActiveOrderSysID;
    ///成交编号
    //TThostFtdcTradeIDType	notActiveTradeID;
    string notActiveTradeID;

    unsigned int activeMarketOrderToken = 0;// active id
    unsigned int activeClientOrderToken = 0;// active id
    EES_SecType activeSecType;
    EES_SideType activeTradeSide;//shengli direction and offsetFlag
    string activeInstrumentid;
    string activeDirection;
    double activeOrderInsertPrice = 0;
    double activeRealOpenPrice = 0;//活跃合约实际开仓成交价格
    double activeRealClosePrice = 0;//活跃合约实际平仓成交价格
    bool activeIsTraded;
    string activeHedgeFlag;
    string activeOffsetFlag;
    string activeOrderRef;
    int activeVolume;
    ///报单提交状态
    //TThostFtdcOrderSubmitStatusType	activeOrderSubmitStatus;
    string activeOrderSubmitStatus;
    ///报单状态
    //TThostFtdcOrderStatusType	activeOrderStatus;
    string activeOrderStatus;
    ///本地报单编号
    //TThostFtdcOrderLocalIDType	activeOrderLocalID;
    string activeOrderLocalID;
    ///报单编号
    //TThostFtdcOrderSysIDType	activeOrderSysID;
    string activeOrderSysID;
    ///成交编号
    //TThostFtdcTradeIDType	activeTradeID;
    string activeTradeID;
    string toString();
};
class InstrumentInfo {
public:
   // TThostFtdcExchangeIDType ExchangeID;
    string ExchangeID;
    ///合约数量乘数
    //TThostFtdcVolumeMultipleType	VolumeMultiple;
    double VolumeMultiple;
    ///最小变动价位
    double	PriceTick;
    ///多头保证金率
    //TThostFtdcRatioType	LongMarginRatio;
    double LongMarginRatio;
    ///空头保证金率
    //TThostFtdcRatioType	ShortMarginRatio;
    double ShortMarginRatio;
    string instrumentID;
    ///涨停板价
    double	UpperLimitPrice =0;
    ///跌停板价
    double	LowerLimitPrice = 0;
    bool isPriceInit = false;
};
/*judge order price and volume*/
class TempOrderInfo{
public:
    int volume = 0;
    double orderInsertPrice = 0;
    double spreadTickPrice = 0;
};
/*报单信息*/
class OrderInfo {
public:
    string tradingDay = "";
    string tradeTime = "";
    boost::atomic_int32_t orderSeq;//组合报单序号
    string systemID = "";//系统编号，每个产品一个编号
    string direction;
    string offsetFlag;
    string orderType;//"0":buy open;"1":sell open;"01":buy close;"11":sell close;"2":stop profit;"3":stop loss
    //"2001":used for reverse stop profit,base on hold position cost.
    //"2011":used for one sweet metric stop profit
    string openStgType;//"3000":first open;priceStatus,"2":step into 2 tick trigger;"1":step into ntickMoveSL.
    string mkType;//"agg":aggressive mm;"pas":passive mm;"0":
    int sessionID;
    int frontID;
    string orderRef;
    string orderSysID;
    string brokerOrderSeq;
    double price;
    int volume;
    string instrumentID;
    int begin_up_cul = 0;
    int begin_down_cul = 0;
    int userID;
    unsigned int clientOrderToken;
    unsigned char m_Side;
    unsigned char m_SecType;
    string status= "0";//0:normal;1:now action
    string function = "0";//0:normal order;100:stop loss order;200:stop profit order
};
class MarketData {
public:
    string instrumentID;
    int volume;
    double bidPrice;
    double askPrice;
    double lastPrice;
    double simPrice;//turnover/volume/multiply
    double	priceTick;
    double highestPrice;
    double lowestPrice;
    double turnover;
    string updateTime;
    char updateTime_char[30];
};
class ATRInfoClass{
public:
    double maxPrice;
    double minPrice;
    double lastClosePrice;
    double currATR;
    double lastATR;
    double TR;
};

/*组合合约的价格gap
用于计算技术指标*/
class PriceGapMarketData {
public:
    string closeID;
    string insComKey;
    double gapPrice;
    string updateTime = "";
    //double asGapPrice;//正套价差,不用
    //double dsGapPrice;//反套价差，不用
    double asOpenGap;
    double asCloseGap;
    double dsOpenGap;
    double dsCloseGap;
    double asHighestGap;
    double asLowestGap;
    double dsHighestGap;
    double dsLowestGap;
    string lastInstrumentID;//近月合约
    string forwardInstrumentID;//远月合约
    double lastInsVolume;
    double forwardInsVolume;
    double lastInsBidPrice;
    double forwardInsBidPrice;
    double lastInsAskPrice;
    double forwardInsAskPrice;
    double priceTick;
    string des;

};
/*close position info*/
class WaitForCloseInfo{
public:
    string updateTime;
    string orderType;
    string insComKey;
    string closeID;
    string systemID;
    double stopProfitGap;//hope profit gap
    double stopLossGap;//hope loss gap
    double openGap;//real open gap
    double closeGap;//real close positon gap
    double ginGap;//real profit or loss gap
    string status;//0:created;1:closed
    //new
    int originalVolume;
    double openPrice;
    int tradeVolume;
    unsigned int marketOrderToken;
    string openStgType;
    string direction;
    string instrumentID;
};
class SpecOrderField{
public:
    string instrumentID;
    string direction;
    string offsetFlag;
    double lastPrice;
    int volume;
    string orderType;
    string openStgType;
};

/*组合成交量以及可成交阈值都在此进行设置*/
class PriceGap {
public:
    double asOpenMetric = 100000;
    double dsOpenMetric = 100000;

    double maxGap = 0.0;
    double minGap = 0.0;//在区间之间，开仓
    double profitGap = 0.0;//超过此区间，平仓
    string systemID = "";
    int timeInterval_ma = 0;//计算简单移动平均线的时间间隔,单位为秒
    int arbComVolumeAS = 0;//正套组合持仓量
    int arbComVolumeDS = 0;//反套组合持仓量
    int arbComVolMetricAS = 10;//正套组合单阈值
    int arbComVolMetricDS = 10;//反套组合单阈值
    int holdPositionGap = 0;//组合合约的持仓差值
};
/*技术指标,组合合约的*/
class TechMetric {
public:
    double totalGapPrice = 0;//compute gap平均值de total value
    double asTotalGap = 0;//正套总gap,for move average gap
    double dsTotalGap = 0;//反套总gap,for move average gap
    string activeInstrumentID;//组合合约中的活跃ID
    boost::posix_time::ptime lastTime;
    double asHighestGap;
    double asLowestGap;
    double dsHighestGap;
    double dsLowestGap;
    double maGap = 0;//简单移动平均价格
    double maGapAmount = 0;//use for computing std of maGap
    bool isSTDVOverFLow = false;//diviation is over flow
    double maGapSTDEV = 0;//STD of maGap
    double asMAGap = 0;//正套移动均值
    double dsMAGap = 0;//反套移动均值
    string strTime = "";
    list<PriceGapMarketData*>* pgDataSeq;//realTime gap data
    list<double>* MAGapSeq;//move average gap seq
    double asGapTotalAmount = 0;//正套gap总值
    int  asTotalVolume = 0;//正套总持仓
    double asHoldMeanGap = 0;//正套持仓平均gap
    double dsGapTotalAmount = 0;//反套gap总值
    int dsTotalVolume = 0;//反套总持仓
    double dsHoldMeanGap = 0;//反套持仓平均gap
    double overMAGapTickNums = 0;//大于均值之上overMAGapTickNums个tick开仓
    double downMAGapTickNums = 0;//小于均值之下downMAGapTickNums个tick平仓
    double stopLossTickNums = 80;//损失达到多少个tick止损
    double stopTradeNums = 10;//止损超过多少次，今天不再进行此品种的交易
    boost::posix_time::ptime tradeInterval;
};
/*技术指标*/
class TechMetric_two{
public:
    double maxPrice = 0;//
    double minPrice = 0;//
    double openPrice = 0;//
    string instrumentID;//
    boost::posix_time::ptime lastTime;
    double asHighestGap;
    double asLowestGap;
    string strTime = "";
    list<PriceGapMarketData*>* pgDataSeq;//realTime gap data
    list<double>* MAGapSeq;//move average gap seq易
    boost::posix_time::ptime tradeInterval;
};
class CTechMetric{
public:
    list<TechMetric_two*> kLine15s_List;//K Line list 15s
    list<TechMetric_two*> kLine15m_List;//K Line list 15m
    int kIndex_15s;
    int mirrorIndex15s;
    int kIndex_15m;
    int mirrorIndex15m;
};

string getComInstrumentKey(string ins1, string ins2);//配对合约，只有一个组合标志
/*
根据传入的合约ID判断是套利报单的活跃还是不活跃合约.手动撤单需要验证一下，是否一定能够根据sessionID_frontID+orderRef匹配到.现在撤单都是根据该标识查找
手动撤单，基本都是活跃合约的；
"1":不活跃合约-开仓
 "2":活跃合约-开仓
 "3":不活跃合约-止盈平仓
 "4":活跃合约-止盈平仓
"0":未查找到套利配对
*/
int* decideOrderType(OrderFieldInfo *pOrder);
/*将制定合约的报单引用修改为新的报单引用
"1":修改成功
"0":失败
*/
string resetOrderRef(OrderFieldInfo *pOrder, unsigned int oldClientToken, unsigned int newClientToken);
/*处理报单回报信息,在rtnOrder期间即可处理成交，修改是否成交标识
"1":修改成功
"0":失败
*/
string processArbRtnOrder(OrderFieldInfo *pOrder);
/*处理成交回报信息，同时下单对应的活跃合约限价单
*/
string processArbRtnTrade( TradeFieldInfo *pTrade);
/*手动撤销报单处理*/
string manualOrderActioin(OrderFieldInfo *pOrder);
/*计算当前不活跃报单的次数,如果超过次数，则删除报单序列
回报处理使用
返回值：0 没有超过下单次数，不删除保单组合
        1 超过下单次数，删除报单组合*/
string setNotActiveOrderInsertAmount(OrderFieldInfo *pOrder);
/*查询持仓明细情况*/
//string getDBPositionDetail(HoldPositionDetail* detail);
/*查询当前套利单所有的详细情况*/
string getArbDetail(TradeInfo* tradeInfo);
/*获得当前所有未成交的套利单报单情况*/
string getAllUntradeInfo();
/*根据报单回报，查询对应的原始下单信息*/
TradeInfo*  getOriginalTradeInfo(OrderFieldInfo *pOrder);
/*获得当前所有未成交的止盈单报单情况*/
string getAllStopProfitInfo();
/*获得当前所有已经成交的等待止盈的套利组合单情况*/
string getAllAlreadyTradedInfo();
/*查询合约信息，结果保存到instruments中。主要是priceTick，exchangeID,等信息*/
string processRspReqInstrument(EES_SymbolField *pInstrument);
void closeOrderInfo();
/*根据持仓情况，判断是平今还是平昨。针对上期所合约*/
void tradeParaProcessTwo();
/*计算平均持仓gap均价，用于止损*/
void processAverageGapGprice(TradeFieldInfo *pTrade);
/*根据成交信息处理当前持仓情况
老方法，后续替换掉*/
int processtrade(TradeFieldInfo *pTrade);
/*是否追单*/
void processFollow(MarketData *pDepthMarketData);
/*根据合约代码查询对应的合约乘数*/
double getMultipler(string instrumentID);
/*根据合约代码查询对应的priceTick*/
double getPriceTick(string instrumentID);
vector<string> split(string str, string pattern);
/*平仓锁定可平数量,判断多少持仓可以用于平仓
回报处理使用
参数：type: lock,锁定持仓
            release,释放持仓*/
void processHowManyHoldsCanBeClose(OrderFieldInfo *pOrder, string type);
/*处理开仓，止盈，止损策略*/
void processStrategy();
//void processHowManyHoldsCanBeClose(string instrumentID, string direction, string offsetFlag, int volume);
/*开仓是否超过限制
根据实现成交组合手数arbVolume+暂时未成交组合手数是否>=Metric判断是否超限
*/
bool isOverTrade(string pd_instr_key, string orderType);
/*开仓是否超过限制
在相同gap上能够开仓的数量
*/
bool isOverTradeTwo(PriceGapMarketData* pgmd, string orderType);
/*报单价格是否偏移
超过最大未成交组合的数量，则不能继续下单，防止由于未收到成交回报等原因导致的报单报入超限
如果可以市价成交，立刻下单；（这一条在行情判断中实现，不在本函数实现）
如果价格没有偏移(即没有超过预定的追单次数),则不下单；
超过次数，表示价格已经偏离，可以下单*/
bool isOrderInsertPriceBiased(string pd_instr_key);
/*处理持仓盈亏等信息，暂时没有完成*/
//int processTradeNew(CThostFtdcTradeField *pTrade);
// 获取系统的当前时间，单位微秒(us)
int64_t GetSysTimeMicros();
char* GetDiffTime(int64_t start, int64_t end);//处理时间差
//获取当前系统时间YYYY-MM-DD HH:MI:SS
string getCurrentSystemTime();
/*增加套利单，套利组合单发送到保存进程
1、添加到alreadyTradeMap中
2、保存到数据库，用于下次初始化
*/
void addArbitrageOrders(TradeInfo* info);
/*删除套利单，套利组合单发送到保存进程.在止盈组合单成交之后执行
1、alreadyTradeMap中删除
2、数据库中删除
*/
void deleteArbitrageOrders(TradeInfo* info);
string generateHappyID(TradeInfo* info);
/*设置等待止盈报单的状态*/
void changeAlreadyTradedOrderStatus(TradeInfo* info,string status);
/*初始化套利组合持仓数据*/
void initArbgComOrders(list<string> comOrdersList);
/*初始化套利组合持仓完毕*/
void completeInitArbgComOrders();
/*启动策略程序*/
void startStrategy();
/*获得开仓gap*/
PriceGap* getPriceGap(string insComKey);
/*修改开仓预警值*/
void setPriceGap(string insComKey,PriceGap* gap);
/*根据行情判断是否开仓*/
void openProcesser();
/*根据行情判断是否平仓*/
void closeProcesser();
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
int processtradeOfNormal(TradeFieldInfo *pTrade);
MarketPriceAction* priceDefineOpen(PriceGapMarketData* pgMkData, bool isLastActive);
MarketPriceAction* priceDefineClose(MarketData* lastMarketData, MarketData* forwardMarketData, PriceGap* preSetGapInfo, bool isLastActive);
string getMarketData(MarketData* mkdata);
void openIntrestProcesserOne(PriceGapMarketData* defineInfo);
void openIntrestProcesserTwo(PriceGapMarketData* defineInfo);
void openIntrestProcesserThree(PriceGapMarketData* defineInfo);
void openIntrestProcesserFour(PriceGapMarketData* defineInfo);
/*处理止盈过程*/
void stopProfitProcesserOne(PriceGapMarketData* defineInfo);
void stopProfitProcesserTwo(PriceGapMarketData* defineInfo);
void stopProfitProcesserThree(PriceGapMarketData* defineInfo);
void stopProfitProcesserFour(PriceGapMarketData* defineInfo);
void stopLossProcess(MarketData *mkData);//when market data arrive,process hold position
string getCloseMethod(string instrumentID, string type);
bool canBeClose(string pd_instr_key,string orderType);
bool isHoldPositionOverGap(PriceGap* priceGap,string orderType);
void initGapPriceData(list<string> comOrdersList);
void initOverTickNums(TechMetric* tm,string insComKey);
void processStrategyForSingleThread(PriceGapMarketData * mkDataGap) ;
void processNormalHowManyHoldsCanBeClose(OrderFieldInfo *pOrder,string type) ;
//GBK编码转换到UTF8编码
int GBKToUTF8(unsigned char * lpGBKStr,unsigned char * lpUTF8Str,int nUTF8StrLen);
void transformFromCancleOrder(EES_EnterOrderField* pOrder,OrderFieldInfo* third_part_info);
void transformFromShengLiPlantformOrder(EES_OrderAcceptField* pOrder,OrderFieldInfo* third_part_info);
void transformFromExchangeOrder(OriginalOrderFieldInfo* pOrder,OrderFieldInfo* third_part_info);
void transformFromExchangeTrade(OriginalOrderFieldInfo* pOrder,TradeFieldInfo* third_part_info);
EES_SideType getShengliMethod(string direction,string offsetFlag);
void storeInitOrders(EES_EnterOrderField* orderField,AdditionOrderInfo* aoi);
string getWaitForCloseInfoDetail(WaitForCloseInfo* wfcInfo);
void initWaitForCloseData(list<string> wfcList);
bool asIsCanBeTrade(PriceGapMarketData* mkdata,double gapPrice);
bool dsIsCanBeTrade(PriceGapMarketData* mkdata,double gapPrice);
void computeGapRange();
void initPriceGap(string instrumentID);
int getAvailableClosePosition(string instrumentID);
void getBidAskOrderListInfo(unsigned int clientOrderToken,OrderInfo* orderInfo);
void deleteOriOrder(unsigned int  clientOrderToken);
unsigned char changeSignalFromNormalToSL(string direction,string offsetFlag);
InstrumentInfo* getInstrumentInfo(string instrumentID);
//void processOrderAction(CThostFtdcOrderField *pOrder);
string getOrderInfo(OrderInfo* info);
string getCancleOrderInfo(EES_CancelOrder  *clOrder);
int getPriceExistAmount(string instrumentID,string direction,double price,double lastPrice,double tickPrice);
void tryOrderAction(string instrumentID,OrderInfo* orderInfo,string actionType);
//all spec instrumentID will be action.
void tryAllOrderAction(string instrumentID);
string num2str(double i);
void computeDualTrustPara(double lastPrice);
bool existUntradeOrder(string type,OrderInfo* untradeOrder);
//init shadow and sun line
void initSunOrShadowLine(string direction);
bool stopProfit(string direction,double lastPrice,string instrumentID);
void processUserHoldPosition(OrderFieldInfo* realseInfo,list<WaitForCloseInfo*>* userPstList);
void processOtherOpen(OrderFieldInfo* realseInfo,list<WaitForCloseInfo*>* userPstList);
void processClose(OrderFieldInfo* realseInfo,list<WaitForCloseInfo*>* userPstList);
void computeUserHoldPositionInfo(list<WaitForCloseInfo*> *sourList);
void cancelSpecTypeOrder(string instrumentID,string type);
void resetK15sData();
void coverYourAss();
void doSpecOrder(SpecOrderField* sof);
void lockInit();
void sendMSG(string msg);
string getTradeInfo(OrderFieldInfo* realseInfo);
void addNewOrderTrade(string instrumentID,string direction,string offsetFlag,double orderInsertPrice,int volume,string mkType,AdditionOrderInfo* addinfo);
#endif
