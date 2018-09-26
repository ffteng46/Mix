#include <iostream>
#include "EESTraderDemo.h"
#include "EESQuoteDemo.h"
#include <glog/logging.h>
#include <mutex>
#include "property.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "tcpServer.h"
#include <fstream>
#include <list>
#include <boost/lexical_cast.hpp>
#include "TimeProcesser.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "Strategy.h"
#include "calculate.h"
//using namespace std;

#define BUFFER_SIZE 1024
using namespace std;
//CTechMetric ctm;
Strategy techCls;
calculate cal;
HoldPositionInfo userHoldPst;//not real hold position info
list<WaitForCloseInfo*> protectList;//protect order list
list<WaitForCloseInfo*> allTradeList;//before one normal
list<WaitForCloseInfo*> longReverseList;//before one normal
list<WaitForCloseInfo*> tmpLongReverseList;//before one normal
unordered_map<string, HoldPositionInfo*> reversePosition;
bool testSwitch=false;
bool isInstrumentInit=false;
/************************market maker*/
vector<double> mkTimeGap ;//tow marketdata time interval
list<OrderInfo*> aggOrderList;//aggressive market maker order list
list<OrderInfo*> aggTradeList;//aggressive market maker trade list
HoldPositionInfo* holdInfo = new HoldPositionInfo();
unordered_map<string, MarketData*> instrinfo;//market data
vector<double> simPriceList;//save some simulated trade price
boost::posix_time::time_period *mkTimePeriod;
unordered_map<string, HoldPositionInfo*> positionmap;
unordered_map<string, ControlOrderInfo*> controlTimeMap;//control order insert number
unordered_map<string, HoldPositionInfo*> normalMMPositionmap;
int amountCanExist = 1;//how many orders can be put in this price

//gap list map
unordered_map<double,vector<double>> map_price_gap;
//list<OrderInfo*> orderList;//all order list
list<HoldPositionDetail*> holdPositionList;//position list
double realTradePrice = 0;//default value is lastPrice
list<OrderInfo*> bidList;//order list
list<OrderInfo*> askList;//
list<OrderInfo*> longList;// trade list
list<OrderInfo*> shortList;//trade list
double floatMetric = 0.0001;//use for compare tow double
int volSpread = 3;
int orderPriceLevel = 6;
int overVolume = 10;//over this metric,open will be changed to close
int volMetric = 3;

//上涨
boost::atomic_int up_culculate(0);
//下跌
boost::atomic_int down_culculate(0);
//price up to sell
boost::atomic_int priceUpToSell;
//price down to buy
boost::atomic_int priceDownToBuy;
//上一次价格所处的区间
int last_gap = -1;
//报单触发信号
int cul_times;

double previous_price = 0;
int last_down_cul;
int last_up_cul;
int eachPriceOrderCount = 1;
/***************fromTradeSpi*/
///多个字段组合时使用的分隔符
string sep = ";";
char tradingDay[12] = { "\0" };
// 会话参数
string MAC_ADDRESS = "00-01-02-03-04-05";
int	FRONT_ID;	//前置编号
int	SESSION_ID;	//会话编号
string	ORDER_REF;	//报单引用
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
/***************/
unordered_map<string, MarketData*> marketdata_map;
// UserApi对象
//CThostFtdcMdApi* mdUserApi;
CodeConverter *codeCC;
//交易对象
boost::thread_group thread_log_group;
// UserApi对象
TraderDemo* ptradeApi;
QuoteDemo* pQuoteApi;
// 配置参数
//char FRONT_ADDR[] = "tcp://asp-sim2-md1.financial-trading-platform.com:26213";		// 前置地址
//180.168.146.181:10100
string tradingDayT = "";//2010-01-01
//存放行情消息队列
list<string> mkdata;
string tradeServerIP = "";
int tradeServerPort = 0;
string queryServerIP = "";
int queryServerPort = 0;
string marketServerIP = "";
int  marketServerPort = 0;
char TRADE_FRONT_ADDR[] = "tcp://180.168.146.181:10100";
char MARKET_FRONT_ADDR[] = "tcp://180.168.146.181:10100";
string	BROKER_ID = "0077";				// 经纪公司代码
string INVESTOR_ID = "00042";			// 投资者代码
string  PASSWORD = "0";			// 用户密码
int USER_ID = 0;
string LOGIN_ID = "";
int remoteTradeServerPort = 0;//交易端口
string hedgeFlag = "1";//账号类型，组合投机套保标 投机 '1'套保 '3'
int defaultVolume = 1;//默认下单手数
char **ppInstrumentID;// 行情订阅列表
vector<string> instrumentsList;	//合约列表
int iInstrumentID = 1;									// 行情订阅数量
int mkdatasrvport = 0;
OrderFieldInfo* cancledOrderInfo = new OrderFieldInfo();//cancled order info
OrderFieldInfo* acceptShLiOrderInfo = new OrderFieldInfo();//shengli plantform order response info
OrderFieldInfo* acceptMarketOrderInfo = new OrderFieldInfo();//exchange order response info
TradeFieldInfo* orderExecInfo = new TradeFieldInfo();//when order is executed,trade response is sent
///////时间函数处理
unordered_map<string, TechMetric*> techMetricMap;
vector<string> quoteList;	//合约列表											//价格变动单位
unordered_map<string, vector<string>> instr_map;				//一个合约和哪些合约配对
boost::lockfree::queue<LogMsg*> logqueue(1000);///日志消息队列
boost::lockfree::queue<LogMsg*> networkTradeQueue(1000);///报单、成交消息队列,网络通讯使用
boost::lockfree::queue<PriceGapMarketData*> detectOpenQueue(1000);///开仓行情处理列表
boost::lockfree::queue<PriceGapMarketData*> strategyQueue(1000);///处理列表
boost::lockfree::queue<PriceGapMarketData*> detectCloseQueue(1000);///止盈行情处理列表
boost::lockfree::queue<MarketData*> techMetricQueue(1000);///技术指标处理列表
boost::recursive_mutex unique_mtx;//unique lock
//触发套利单时，保存套利信息
unordered_map<string, list<TradeInfo*>*> willTradeMap;
unordered_map<string, unordered_map<string, int64_t>> seq_map_orderref;
unordered_map<string, string> seq_map_ordersysid;
list<WaitForCloseInfo*> allASTradeList;//long
list<WaitForCloseInfo*> allDSTradeList;//long
double asTradeListMinGap;
double asTradeListMaxGap;
double dsTradeListMinGap;
double dsTradeListMaxGap;
double currAtr = 0;
double lastAtr = 0;
ATRInfoClass* atrinfo = new ATRInfoClass();
vector<int> upCulmulateList;
vector<int> downCulmulateList;

//止盈报单
unordered_map<string, list<TradeInfo*>*> stopProfitWillTradeMap;
unordered_map<string, list<TradeInfo*>*> alreadyTradeMapAS;			//保存已成交套利单信息,正套
unordered_map<string, list<TradeInfo*>*> alreadyTradeMapDS;			//保存已成交套利单信息,反套
unordered_map<string, bool> holdPstIsLocked;			//保存已经锁仓的报单，防止多次未知单回报导致的多次锁仓情况。
unordered_map<string, PriceGap*> instr_price_gap;			//价格差
//unordered_map<string, double> instr_price_gap_profit;			//价格差,价差减小,盈利
unordered_map<string, InstrumentInfo*> instruments;			//合约信息
unordered_map<string, list<HoldPositionDetail*>*> positionDetailMap;//持仓明细
unordered_map<string, list<HoldPositionDetail*>*> pstDetailFromDB;//持仓明细
unordered_map<string, OriginalOrderFieldInfo*> originalOrderMap;//userid send order
unordered_map<string, string> exgToOriOrderMap;//order response from exchange;need MarketOrderToken to corresponsding to original order
//list<HoldPositionDetail*> positionDetailMap;//持仓明细
boost::atomic_int32_t orderSeq(0);//组合报单序号
int gapCanBeOpenNums = 4;//每个gap上面可以开仓的数量
//double stopLossTickNums = 10;//价差往不利方向损失多少tick时候，组合止损
int notActiveInsertAmount = 1;//不活跃合约重复下单次数
int orderInsertInterval = 1000;//下单间隔
int arbVolumeMetric = 0;//套利单总共能下多少手，单边
int biasTickNums = 2;//价格偏移多少个tick进行追单
int arbVolume = 0;//当前持仓量
int maxFollowTimes = 2;//最大追单次数
int maxUntradeNums = 3;//最大未成交套利单笔数(非手数，手数=maxUntradeNums*defaultVolume)
//int maxUntradeCount = 3;//最大未成交笔数(非手数，手数=maxUntradeNums*defaultVolume)
string systemID = "";//系统编号，每个产品一个编号
int timeBias=20;
int positionHoldTime=5;//how long positon hold,count by second.
double openTick = 5.0;//开仓时候每次增加的tick值5
double closeTick = 15.0;//平仓时每次增加的tick值15
double extreamTick = 30.0; //: 偏离到达极值之后每次增加的tick值30
double profitValue = 20.0;//盈利数值
double extreamPriceGap = -490.0;//当前价差最大值,超过此值表示超过极值,负值
double maPriceGap = -455.0;//当前价差平均值,当前为无风险差值。负值trad
//int overMAGapTickNums = 1;//超过均值之上几个tick才开仓
int timeInterval = 100;//时间间隔,按照秒来算时间间隔。比如1分钟间隔，timeInterval=60
boost::recursive_mutex pstDetail_mtx;//持仓明细
boost::recursive_mutex pstDetailDB_mtx;//持仓明细
boost::recursive_mutex queryPst_mtx;//查询各种持仓
boost::recursive_mutex priceGap_mtx;//查询组合持仓的gap
//boost::recursive_mutex arbVolume_mtx;//套利组合单数量的gap
boost::recursive_mutex techMetric_mtx;//技术指标
bool isLogout = true;
double profitTickNums = 20.0;//盈利数值
double lossTickNums = 5;
int isTwoStartStrategy = 0;//等于2的时候，表示明细和汇总持仓查询完毕，启动系统
double tick = 0.5;
// 请求编号
int iRequestID = 0;
//报单引用
int iOrderRef = 0;
void datainit();
void mkdataInit();
void testlist();
recursive_mutex g_lock_ti;//tradeinfo lock
recursive_mutex g_lock_log;//log lock
//vector<string> split(string str, string pattern);
//DWORD WINAPI sendByClient(LPVOID lpparameter);          //客户端方式发送
//DWORD WINAPI sendByServer(LPVOID lpparameter);          //客户端方式发送
void startSendMDThread(int sendtype);//发送类型 0：以客户端方式发送；1，以服务端方式发送
void tradeinit();
void tsocket();
int main(){
    google::InitGoogleLogging("");
    google::SetLogDestination(google::GLOG_INFO, "./Logs");
    //google::SetStderrLogging(2);
    google::SetLogFilenameExtension("log_");
    codeCC = new CodeConverter("gb2312","utf-8");
    //mkTimePeriod = new boost::posix_time::time_period(getCurrentTimeByBoost(),boost::posix_time::milliseconds(490));
    //cout<<"-->"<<mkTimePeriod->begin()<<endl;
    //cout<<"en-->"<<mkTimePeriod->last()<<endl;
    for(int i = 0;i < 10;i++){

        //isInTimePeriod(mkTimePeriod);
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
    datainit();
    //testlist();
    //getchar();
    //system("pause");
    //tradeinit();//交易
    startSendMDThread(1);
    mkdataInit();//行情
    //boost::this_thread::sleep(boost::posix_time::seconds(2));
    ptradeApi = new TraderDemo();
    //TraderDemo tmd;
    ptradeApi->Run();
    // 初始化UserApi

    while(1){
        boost::this_thread::sleep(boost::posix_time::seconds(1000));    //microsecond,millisecn
    }
    return 0;
}
/************************************************************************/
/* 初始化参数列表                                                                     */
/************************************************************************/

void datainit() {
    std::ifstream myfile("config/global.properties");
    if (!myfile) {
        cerr << "读取global.properties文件失败" << endl;
    }
    string str;
    while (getline(myfile, str)) {
        int pos = str.find("#");
        if (pos == 0) {
            //cout << "注释:" << str << endl;
        }
        else {
            vector<string> vec = split(str, "=");
            //cout << str << endl;
            //cout<<vec[0]<<"=="<<vec[1]<<endl;
            if ("investorid" == vec[0]) {
                INVESTOR_ID = vec[1];
                //strcpy(INVESTOR_ID, vec[1].c_str());
            }
            else if ("marketServerIP" == vec[0]) {
                marketServerIP = vec[1];
            }
            else if ("marketServerPort" == vec[0]) {
                marketServerPort = boost::lexical_cast<int>(vec[1]);
            }else if ("tradeServerIP" == vec[0]) {
                tradeServerIP = vec[1];
            }else if ("lossTickNums" == vec[0]) {
                lossTickNums = boost::lexical_cast<double>(vec[1]);
            }
            else if ("tradeServerPort" == vec[0]) {
                tradeServerPort = boost::lexical_cast<int>(vec[1]);
            }else if ("profitTickNums" == vec[0]) {
                profitTickNums = boost::lexical_cast<double>(vec[1]);
            }
            else if ("queryServerPort" == vec[0]) {
                queryServerPort = boost::lexical_cast<int>(vec[1]);
            }
            else if ("queryServerIP" == vec[0]) {
                queryServerIP = vec[1];
            }
            else if ("timeInterval" == vec[0]) {
                timeInterval = boost::lexical_cast<int>(vec[1]);
            }
            else if ("gapCanBeOpenNums" == vec[0]) {
                gapCanBeOpenNums = boost::lexical_cast<int>(vec[1]);
            }/*
            else if ("overMAGapTickNums" == vec[0]) {
                overMAGapTickNums = boost::lexical_cast<int>(vec[1]);
            }*/
            else if ("brokerid" == vec[0]) {
                BROKER_ID = vec[1];
                //strcpy(BROKER_ID, vec[1].c_str());
            }
            else if ("systemID" == vec[0]) {
                systemID = vec[1];
            }
            else if ("openTick" == vec[0]) {
                openTick = boost::lexical_cast<double>(vec[1]);
            }else if ("timeBias" == vec[0]) {
                timeBias = boost::lexical_cast<int>(vec[1]);
            }
            else if ("closeTick" == vec[0]) {
                closeTick = boost::lexical_cast<double>(vec[1]);
            }
            else if ("extreamTick" == vec[0]) {
                extreamTick = boost::lexical_cast<double>(vec[1]);
            }
            else if ("extreamPriceGap" == vec[0]) {
                extreamPriceGap = boost::lexical_cast<double>(vec[1]);
            }
            else if ("maPriceGap" == vec[0]) {
                maPriceGap = boost::lexical_cast<double>(vec[1]);
            }
            else if ("profitValue" == vec[0]) {
                profitValue = boost::lexical_cast<double>(vec[1]);
            }
            else if ("remoteTradeServerPort" == vec[0]) {
                remoteTradeServerPort = boost::lexical_cast<int>(vec[1]);
            }
            else if ("remoteMkdataServerPort" == vec[0]) {
                mkdatasrvport = boost::lexical_cast<int>(vec[1]);
            }
            else if ("password" == vec[0]) {
                PASSWORD = vec[1];
                //strcpy(PASSWORD, vec[1].c_str());
            }else if ("loginid" == vec[0]) {
                LOGIN_ID = vec[1];
            }
            else if ("notActiveInsertAmount" == vec[0]) {
                notActiveInsertAmount = boost::lexical_cast<int>(vec[1]);
            }
            else if ("arbVolumeMetric" == vec[0]) {
                arbVolumeMetric = boost::lexical_cast<int>(vec[1]);
            }
            else if ("arbVolume" == vec[0]) {
                arbVolume = boost::lexical_cast<int>(vec[1]);
            }
            else if ("orderInsertInterval" == vec[0]) {
                orderInsertInterval = boost::lexical_cast<int>(vec[1]);
            }
            else if ("maxFollowTimes" == vec[0]) {
                maxFollowTimes = boost::lexical_cast<int>(vec[1]);
            }
            else if ("maxUntradeNums" == vec[0]) {
                maxUntradeNums = boost::lexical_cast<int>(vec[1]);
            }
            else if ("biasTickNums" == vec[0]) {
                biasTickNums = boost::lexical_cast<int>(vec[1]);
            }
            else if ("mdFrontAddr" == vec[0]) {
                strcpy(MARKET_FRONT_ADDR, vec[1].c_str());
            }else if ("volSpread" == vec[0]) {
                volSpread = boost::lexical_cast<int>(vec[1]);
            }else if ("orderPriceLevel" == vec[0]) {
                orderPriceLevel = boost::lexical_cast<int>(vec[1]);
            }else if ("overVolume" == vec[0]) {
                overVolume = boost::lexical_cast<int>(vec[1]);
            }else if ("volMetric" == vec[0]) {
                volMetric = boost::lexical_cast<int>(vec[1]);
            }
            else if ("tradeFrontAddr" == vec[0]) {
                strcpy(TRADE_FRONT_ADDR, vec[1].c_str());
            }
            else if ("tick" == vec[0]) {
                tick = boost::lexical_cast<double>(vec[1]);
            }else if ("instrumentList" == vec[0]) {
                /************************************************************************/
                /* 如果读到      instrumentList，则保存到本程序中                                                               */
                /************************************************************************/
                const char *expr = vec[1].c_str();
                //cout<<expr<<endl;
                char *inslist = new char[strlen(expr) + 1];
                strcpy(inslist, expr);
                //cout<<inslist<<endl;
                const char * splitlt = ","; //分割符号
                char *plt = 0;
                plt = strtok(inslist, splitlt);
                while (plt != NULL) {
                    quoteList.push_back(plt);
                    //cout<<plt<<endl;
                    plt = strtok(NULL, splitlt); //指向下一个指针
                }
                //动态分配字符数组
                ppInstrumentID = new char*[quoteList.size()];
                for (int i = 0, j = quoteList.size(); i < j; i++) {
                    const char * tt2 = quoteList[i].c_str();
                    char* pid = new char[strlen(tt2) + 1];
                    strcpy(pid, tt2);
                    ppInstrumentID[i] = pid;
                    instrumentsList.push_back(boost::lexical_cast<string>(pid));
                    //cout<<ppInstrumentID[i]<<endl;
                }
                iInstrumentID = quoteList.size();
            }
            else if ("spreadList" == vec[0]) {
                string spreadList = vec[1];
                vector<string> tmp_splists = split(spreadList,",");
                for (unsigned int i = 0; i < tmp_splists.size();i++) {
                    string tmp_str = tmp_splists[i];
                    vector<string> price_gap_info_vec = split(tmp_str,"~");
                    //1是开仓价差范围，2是平仓价差
                    vector<string> instr_s = split(price_gap_info_vec[0],"-");
                    string ins_com_key = "";
                    ins_com_key = getComInstrumentKey(instr_s[0], instr_s[1]);
                    /*
                    //价格差初始化
                    if (instr_s[0] > instr_s[1]) {//Key按照近月-远月的格式
                        ins_com_key = instr_s[1] + "-" + instr_s[0];
                    }
                    else {
                        ins_com_key = instr_s[0] + "-" + instr_s[1];
                    }*/
                    //开仓价差范围
                    PriceGap* priceGap = new PriceGap();
                    vector<string> priceGapVec = split(price_gap_info_vec[1],"$");//开仓区间
                    priceGap->minGap = boost::lexical_cast<double>(priceGapVec[0]);
                    priceGap->maxGap = boost::lexical_cast<double>(priceGapVec[1]);
                    priceGap->profitGap = boost::lexical_cast<double>(price_gap_info_vec[2]);
                    priceGap->systemID = systemID;
                    //组合持仓开平参数设置
                    if (ins_com_key == "cu1802-cu1803") {
                        priceGap->timeInterval_ma = timeInterval;
                    } else if (ins_com_key == "pp1801-pp1805") {
                        priceGap->timeInterval_ma = timeInterval;
                    } else if (ins_com_key == "ru1805-ru1809") {
                        priceGap->timeInterval_ma = timeInterval;
                    } else if (ins_com_key == "rb1801-rb1805") {
                        priceGap->timeInterval_ma = timeInterval;
                    } else if (ins_com_key == "i1801-i1805") {
                        priceGap->timeInterval_ma = timeInterval;
                    } else if (ins_com_key == "MA801-MA805") {
                        priceGap->timeInterval_ma = timeInterval;
                    } else if (ins_com_key == "hc1801-hc1805") {
                        priceGap->timeInterval_ma = timeInterval;
                    } else if (ins_com_key == "ni1805-ni1809") {
                        priceGap->timeInterval_ma = timeInterval;
                    } else if (ins_com_key == "jm1801-jm1805") {
                        priceGap->timeInterval_ma = 220;
                    }else if (ins_com_key == "TA801-TA805") {
                        priceGap->timeInterval_ma = timeInterval;
                    }else if (ins_com_key == "CF801-CF805") {
                        priceGap->timeInterval_ma = timeInterval;
                    }else if (ins_com_key == "m1801-m1805") {
                        priceGap->timeInterval_ma = timeInterval;
                    }else if (ins_com_key == "j1801-j1805") {
                        priceGap->timeInterval_ma = timeInterval;
                    }
                    instr_price_gap[ins_com_key] = priceGap;//保存价差数据
                    //instr_price_gap[instr_s[1] + "-" + instr_s[0]] = boost::lexical_cast<double>(price_gap_info_vec[1]);
                    //价格初始化
                    //instr_price_map[instr_s[0]] = 0;
                    //instr_price_map[instr_s[1]] = 0;
                    //第一个合约的配对关系,以第一个合约为key，第二个合约需要放到list里面
                    unordered_map<string, vector<string>>::iterator it_map_strs = instr_map.find(instr_s[0]);//合约配对信息
                    if (it_map_strs == instr_map.end()) {//未建立配对关系,需新建一个vector
                        vector<string> tmp_vec;
                        tmp_vec.push_back(instr_s[1]);
                        instr_map[instr_s[0]] = tmp_vec;
                    }
                    else {
                        vector<string> tmp_vec = it_map_strs->second;
                        bool is_exist_instr = false;
                        for (unsigned int k = 0; k < tmp_vec.size();k ++) {
                            if (tmp_vec[k] == instr_s[1]) {
                                is_exist_instr = true;
                                break;
                            }
                        }
                        if(!is_exist_instr){
                            tmp_vec.push_back(instr_s[1]);
                            it_map_strs->second = tmp_vec;
                        }
                    }
                    //第二个合约的配对关系,以第二个合约为key，第一个合约需要放到list里面
                    it_map_strs = instr_map.find(instr_s[1]);//合约配对信息
                    if (it_map_strs == instr_map.end()) {//未建立配对关系,需新建一个vector
                        vector<string> tmp_vec;
                        tmp_vec.push_back(instr_s[0]);
                        instr_map[instr_s[1]] = tmp_vec;
                    }
                    else {
                        vector<string> tmp_vec = it_map_strs->second;
                        bool is_exist_instr = false;
                        for (unsigned int k = 0; k < tmp_vec.size(); k++) {
                            if (tmp_vec[k] == instr_s[0]) {
                                is_exist_instr = true;
                                break;
                            }
                        }
                        if (!is_exist_instr) {
                            tmp_vec.push_back(instr_s[0]);
                            it_map_strs->second = tmp_vec;
                        }
                    }
                }


            }
        }

    }
}
void testlist() {
    LOG(INFO) << "TRADE_FRONT_ADDR=" + string(TRADE_FRONT_ADDR);
    LOG(INFO) << "MARKET_FRONT_ADDR=" + string(MARKET_FRONT_ADDR);
    LOG(INFO) << "BROKER_ID=" + string(BROKER_ID);
    LOG(INFO) << "INVESTOR_ID=" + string(INVESTOR_ID);
    LOG(INFO) << "PASSWORD=" + string(PASSWORD);
    LOG(INFO) << "hedgeFlag=" + string(hedgeFlag);
    LOG(INFO) << "默认下单手数defaultVolume=" + boost::lexical_cast<string>(defaultVolume);
    LOG(INFO) << "行情端口mkdatasrvport=" + boost::lexical_cast<string>(mkdatasrvport);
    LOG(INFO) << "重复下单次数notActiveInsertAmount=" + boost::lexical_cast<string>(notActiveInsertAmount);
    LOG(INFO) << "重复下单间隔orderInsertInterval=" + boost::lexical_cast<string>(orderInsertInterval);
    LOG(INFO) << "成交限制arbVolumeMetric=" + boost::lexical_cast<string>(arbVolumeMetric);


    LOG(INFO) << "timeInterval=" + boost::lexical_cast<string>(timeInterval);
    //LOG(INFO) << "overMAGapTickNums=" + boost::lexical_cast<string>(overMAGapTickNums);
    LOG(INFO) << "biasTickNums=" + boost::lexical_cast<string>(biasTickNums);
    LOG(INFO) << "maxFollowTimes=" + boost::lexical_cast<string>(maxFollowTimes);
    LOG(INFO) << "maxUntradeNums=" + boost::lexical_cast<string>(maxFollowTimes);
    LOG(INFO) << "systemID=" + systemID;
    LOG(INFO) << "openTick=" + boost::lexical_cast<string>(openTick);
    LOG(INFO) << "closeTick=" + boost::lexical_cast<string>(closeTick);
    LOG(INFO) << "extreamTick=" + boost::lexical_cast<string>(extreamTick);
    LOG(INFO) << "profitValue=" + boost::lexical_cast<string>(profitValue);
    LOG(INFO) << "extreamPriceGap=" + boost::lexical_cast<string>(extreamPriceGap);
    LOG(INFO) << "maPriceGap=" + boost::lexical_cast<string>(maPriceGap);
    LOG(INFO) << "volSpread=" + boost::lexical_cast<string>(volSpread);
    LOG(INFO) << "orderPriceLevel=" + boost::lexical_cast<string>(orderPriceLevel);
    LOG(INFO) << "overVolume=" + boost::lexical_cast<string>(overVolume);
    LOG(INFO) << "volMetric=" + boost::lexical_cast<string>(volMetric);
}
void mkdataInit() {
    cout << "start to init mdApi" << endl;
    pQuoteApi = new QuoteDemo();
    pQuoteApi->Run();
    //mdUserApi = CThostFtdcMdApi::CreateFtdcMdApi("log2");			// 创建UserApi
    //CThostFtdcMdSpi* pUserSpi = new CMdSpi();
    //mdUserApi->RegisterSpi(pUserSpi);						// 注册事件类
    //mdUserApi->RegisterFront(MARKET_FRONT_ADDR);					// connect
    //mdUserApi->Init();
    //cout << "end init mdApi" << endl;
}
void startSendMDThread(int sendtype)
{
    //TraderDemo temp;
    //temp.m_queryServerIp = boost::lexical_cast<string>(TRADE_FRONT_ADDR);
    thread_log_group.create_thread(&startTCPServer);
    //thread_log_group.create_thread(processStrategy);
    //thread_log_group.create_thread(metricProcesser);
}



void tsocket(){
    string ips = "localhost";
    const char* ip = ips.c_str();
    int port = 61000;
    int backlog = 3;

    std::cout << "ip=" << ip << " port="<<port << " backlog=" << backlog  << std::endl;

    int fd;
    int check_ret;

    fd = socket(PF_INET,SOCK_DGRAM , 0);
    assert(fd >= 0);

    struct sockaddr_in address;
    bzero(&address,sizeof(address));

    //转换成网络地址
    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    //地址转换
    inet_pton(AF_INET, ip, &address.sin_addr);

    //绑定ip和端口
    check_ret = bind(fd,(struct sockaddr*)&address,sizeof(address));
    assert(check_ret >= 0);


    cout<<"start udp server ok"<<endl;
    while(1){

        char buffer[BUFFER_SIZE];
        struct sockaddr_in addressClient;
        socklen_t clientLen = sizeof(addressClient);
        memset(buffer, '\0', BUFFER_SIZE);
        //获取信息
        if(recvfrom(fd, buffer, BUFFER_SIZE-1,0,(struct sockaddr*)&addressClient, &clientLen) == -1)
        {
           perror("Receive Data Failed:");
           exit(1);
        }
        printf("buffer=%s\n", buffer);
    }
    close(fd);
}
//
//  main.cpp
//  linux_socket_api_client
//
//  Created by bikang on 16/11/2.
//  Copyright (c) 2016年 bikang. All rights reserved.
//

void tserver(){
    std::cout << "t server" << std::endl;
    string sip="180.166.6.245";
    const char* ip = sip.c_str();
    int port = 62000;
    int backlog = 3;

    std::cout << "ip=" << ip << " port="<<port << " backlog=" << backlog  << std::endl;

    int fd;
    int check_ret;

    fd = socket(PF_INET,SOCK_DGRAM , 0);
    assert(fd >= 0);

    struct sockaddr_in address;
    bzero(&address,sizeof(address));

    //转换成网络地址
    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    //地址转换
    inet_pton(AF_INET, ip, &address.sin_addr);
    //发送数据
    const char* normal_data = "my boy!";
    while(1){
        sleep(1);
    }
    if(sendto(fd, normal_data, strlen(normal_data),0,(struct sockaddr*)&address,sizeof(address)) < 0)
    {
      perror("Send File Name Failed:");
      exit(1);
    }
    close(fd);
}
