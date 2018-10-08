#include "EESQuoteDemo.h"
#include <glog/logging.h>
#include "TimeProcesser.h"
#include "property.h"
#include <iostream>
#include "Strategy.h"
using std::cout;
using std::cin;
using std::endl;
extern string marketServerIP ;
extern int start_process;
extern int  marketServerPort;
extern unordered_map<string, HoldPositionInfo*> positionmap;
extern boost::lockfree::queue<MarketData*> techMetricQueue;///技术指标处理列表
extern boost::lockfree::queue<LogMsg*> networkTradeQueue;///报单、成交消息队列,网络通讯使用
// 报单引用
extern int iOrderRef;
extern string  PASSWORD;			// 用户密码
extern int USER_ID;
extern string LOGIN_ID;
extern string INVESTOR_ID;			// 投资者代码
extern unordered_map<string, InstrumentInfo*> instruments;			//合约信息
extern unordered_map<string, MarketData*> instrinfo;//market data
extern vector<string> quoteList;	//合约列表
extern unordered_map<string, MarketData*> marketdata_map;
extern boost::recursive_mutex unique_mtx;//unique lock
extern bool isLogout;
extern string systemID;//系统编号，每个产品一个编号
extern char tradingDay[12];
extern string tradingDayT;//2010-01-01
extern string currTime;//=tradingDayT + updateTime
/**********************************/
//gap list map
extern double realTradePrice;//default value is lastPrice
//extern list<OrderInfo*> orderList;//all order list
extern list<HoldPositionDetail*> holdPositionList;//position list
extern boost::posix_time::time_period *mkTimePeriod;
extern unordered_map<double,vector<double>> map_price_gap;
extern list<OrderInfo*> bidList;//order list
extern list<OrderInfo*> askList;//
extern list<OrderInfo*> longList;// trade list
extern list<OrderInfo*> shortList;//trade list
extern double floatMetric;//use for compare tow double
extern int volSpread;
extern int orderPriceLevel;
extern int overVolume;
extern unordered_map<string, OriginalOrderFieldInfo*> originalOrderMap;//userid send order
extern ATRInfoClass* atrinfo;
extern vector<int> upCulmulateList;
extern vector<int> downCulmulateList;
extern int amountCanExist;//how many orders can be put in this price
char tradeDay[64]={'\0'};

//上涨
extern boost::atomic_int up_culculate;
//下跌
extern boost::atomic_int down_culculate;
//price up to sell
extern boost::atomic_int priceUpToSell;
//price down to buy
extern boost::atomic_int priceDownToBuy;
//上一次价格所处的区间
extern int last_gap;
extern double previous_price;
extern int last_down_cul;
extern int last_up_cul;
extern int eachPriceOrderCount;
//报单触发信号
extern int cul_times;
extern int volMetric;
// USER_API参数
extern TraderDemo* ptradeApi;
extern HoldPositionInfo* holdInfo;
///
//extern int kIndex_15s;
//extern int kIndex_15m;
//extern int INDEX_15s;
//extern int INDEX_15m;
//when market data arrived,judge something;
TempOrderInfo* tempOrderInfo = new TempOrderInfo();
boost::posix_time::ptime HEARTBEAT = getCurrentTimeByBoost();
void addNewOrder(string instrumentID,string direction,string offsetFlag,double orderInsertPrice,int volume,string mkType,AdditionOrderInfo* addinfo);
extern Strategy techCls;
extern list<WaitForCloseInfo*> protectList;//protect order list
extern list<WaitForCloseInfo*> allTradeList;//before one normal
extern list<WaitForCloseInfo*> longReverseList;//before one normal
extern list<WaitForCloseInfo*> tmpLongReverseList;//before one normal
extern HoldPositionInfo userHoldPst;//not real hold position info
extern unordered_map<string, HoldPositionInfo*> reversePosition;
extern SpecOrderField* sof;
extern OrderInfo orderInfo;
void calibratePriceAndVolume(string instrumentID,string direction,double lastPrice,double tickPrice,bool isFollow,TempOrderInfo* &tmpInfo);
string getAggDirection(string aggType);
bool hasAggressiveMMOver();
QuoteDemo::QuoteDemo(void)
{
	m_eesApi = NULL;
	m_handle = NULL;

}


QuoteDemo::~QuoteDemo(void)
{
}

void QuoteDemo::Run()
{
    if(Init()){
        cout << "INIT LOADING MARKET .SO SUCCESSFULLY!!";
        EqsTcpInfo			info;
        vector<EqsTcpInfo>	vec_info;

        memset(&info, 0, sizeof(EqsTcpInfo));
        strcpy(info.m_eqsIp, marketServerIP.c_str());
        info.m_eqsPort = marketServerPort;
        vec_info.push_back(info);
        cout<<"mk ip=" <<string(info.m_eqsIp) << ",port=" << boost::lexical_cast<string>(marketServerPort)<<endl;
        bool ret = m_eesApi->ConnServer(vec_info, this);
        if (!ret){
            cout << "初始化EES行情服API对象失败!!!";
        }else{
            cout << "初始化EES行情服API对象成功!!!";
        }

    }else{
        cout << "INIT LOADING MARKET .SO FAILED!!"<<endl;
    }
}

bool QuoteDemo::Init()
{
	bool ret = LoadEESQuote();
	if (!ret){
		return false;
    }else{
        return true;
    }
}

void QuoteDemo::Close()
{
	if (m_eesApi)
	{
		m_eesApi->DisConnServer();
	}

	UnloadEESQuote();
}


void QuoteDemo::Pause()
{
	string str_temp;

	printf("\n按任意字符继续:\n");
	cin >> str_temp;
}


void QuoteDemo::OnEqsConnected(){
    LOG(ERROR) << "CONNECT TO SHENGLI MARKET SERVER SUCCESSFULLY!! BEGIN TO LOGIN......";
	Logon();
}

void QuoteDemo::OnEqsDisconnected()
{
    LOG(ERROR) << "CONNECT TO SHENGLI MARKET SERVER FAILED!! ";
	//throw std::logic_error("The method or operation is not implemented.");
    Run();
}

void QuoteDemo::OnLoginResponse(bool bSuccess, const char* pReason)
{
	if (!bSuccess){
		printf("登录EES行情服务器失败, %s!\n",pReason);
        LOG(INFO) << "登录EES行情服务器失败......";
		return;
    }else{
        LOG(INFO) << "登录EES行情服务器成功!now begin to subscribe market data......";
        time_t timep;
        time (&timep);
        char tmp[64];
        strftime(tradeDay, sizeof(tradeDay), "%Y%m%d %H:%M:%S",localtime(&timep) );
        cout<<tradeDay<<endl;
        for(int i = 0,j = quoteList.size();i < j;i ++){
            LOG(INFO) << "subscribe instrumentID = " + quoteList[i];
            if(quoteList[i] != ""){
                RegisterSymbol(EQS_FUTURE, quoteList[i].c_str());
            }

        }

    }

    //QueryAllSymbol();
}

void QuoteDemo::OnQuoteUpdated(EesEqsIntrumentType chInstrumentType, EESMarketDepthQuoteData* pDepthQuoteData)
{
	ShowQuote(pDepthQuoteData);
}


void QuoteDemo::OnSymbolRegisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess)
{
	if (bSuccess){
        LOG(INFO) << "合约 = " + boost::lexical_cast<string>(pSymbol) + " 注册成功";
	}
	else{
        LOG(INFO) << "合约 = " + boost::lexical_cast<string>(pSymbol) + " 注册失败";
	}
	
}

void QuoteDemo::OnSymbolUnregisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess)
{
	if (bSuccess)
	{
		printf("合约(%s)注销成功\n", pSymbol);
	}
	else
	{
		printf("合约(%s)注销失败\n", pSymbol);
	}
}

void QuoteDemo::OnSymbolListResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bLast)
{
	string strSymbol = pSymbol;
	if (!strSymbol.empty())
	{
		printf("合约:	%s\n", pSymbol);

		RegisterSymbol(chInstrumentType, pSymbol);
	}

	if (bLast)
	{
		printf("\n开始接收行情，按任意键退出:\n");
	}
}

bool QuoteDemo::LoadEESQuote()
{
#ifdef WIN32
	
	return Windows_LoadEESQuote();

#else
	
	return Linux_LoadEESQuote();

#endif
}

void QuoteDemo::UnloadEESQuote()
{
#ifdef WIN32

	return Windows_UnloadEESQuote();

#else

	return Linux_UnloadEESQuote();

#endif
}

bool QuoteDemo::Windows_LoadEESQuote()
{
#ifdef WIN32

	m_handle =  LoadLibrary(EES_QUOTE_DLL_NAME);
	if (!m_handle)
	{
		printf("加载EES行情动态库(%s)失败\n", EES_QUOTE_DLL_NAME);
		return false;
	}

	funcCreateEESQuoteApi createFun = (funcCreateEESQuoteApi)GetProcAddress(m_handle, CREATE_EES_QUOTE_API_NAME);
	if (!createFun)
	{
		printf("获取EES行情接口函数地址(%s)失败!\n", CREATE_EES_QUOTE_API_NAME);
		return false;
	}

	m_distoryFun = (funcDestroyEESQuoteApi)GetProcAddress(m_handle, DESTROY_EES_QUOTE_API_NAME);
	if (!createFun)
	{
		printf("获取EES行情接口函数地址(%s)失败!\n", DESTROY_EES_QUOTE_API_NAME);
		return false;
	}

	m_eesApi = createFun();
	if (!m_eesApi)
	{
		printf("创建EES行情对象失败!\n");
		return false;
	}

#endif

	return true;
}
void QuoteDemo::Windows_UnloadEESQuote()
{
#ifdef WIN32
	if (m_eesApi)
	{
		m_distoryFun(m_eesApi);
		m_eesApi = NULL;
		m_distoryFun = NULL;
	}

	if (m_handle)
	{
		FreeLibrary(m_handle);
		m_handle = NULL;
	}
#endif
}

bool QuoteDemo::Linux_LoadEESQuote()
{
#ifndef WIN32
	m_handle =  dlopen(EES_QUOTE_DLL_NAME, RTLD_LAZY);
	if (!m_handle)
	{
		printf("加载EES行情动态库(%s)失败\n", EES_QUOTE_DLL_NAME);
		return false;
	}

	funcCreateEESQuoteApi createFun = (funcCreateEESQuoteApi)dlsym(m_handle, CREATE_EES_QUOTE_API_NAME);
	if (!createFun)
	{
		printf("获取EES创建函数地址失败!\n");
		return false;
	}

	m_distoryFun = (funcDestroyEESQuoteApi)dlsym(m_handle, DESTROY_EES_QUOTE_API_NAME);
	if (!createFun)
	{
		printf("获取EES销毁函数地址失败!\n");
		return false;
	}

	m_eesApi = createFun();
	if (!m_eesApi)
	{
		printf("创建EES行情对象失败!\n");
		return false;
	}

#endif

	return true;
}

void QuoteDemo::Linux_UnloadEESQuote()
{
#ifndef WIN32
	if (m_eesApi)
	{
		m_distoryFun(m_eesApi);
		m_eesApi = NULL;
		m_distoryFun = NULL;
	}

	if (m_handle)
	{
		dlclose(m_handle);
		m_handle = NULL;
	}
#endif
}


void QuoteDemo::Logon(){
	EqsLoginParam temp;

    strcpy(temp.m_loginId, LOGIN_ID.c_str());
    strcpy(temp.m_password, PASSWORD.c_str());

	if (!m_eesApi)
	{
		printf("无效的EES对象\n");
		return;
	}
    LOG(INFO) << "LOGIN MARKET SERVER INFO:LOGIN_ID=" + LOGIN_ID + ",PASSWORD=" + PASSWORD;
	m_eesApi->LoginToEqs(temp);
}

void QuoteDemo::QueryAllSymbol()
{
	if (!m_eesApi)
	{
		printf("无效的EES对象\n");
		return;
	}

	m_eesApi->QuerySymbolList();
}

void QuoteDemo::RegisterSymbol(EesEqsIntrumentType chInstrumentType, const char* pSymbol)
{
	if (!m_eesApi)
	{
		printf("无效的EES对象\n");
		return;
	}

	m_eesApi->RegisterSymbol(chInstrumentType, pSymbol);
}
bool aggMMIsTriggerd = false;
AdditionOrderInfo* addinfo = new AdditionOrderInfo();
int pasStopLossTickNums = 2;
int pasStopProfitTickNums = 2;
int aggStopLossTickNums = 2;
int aggStopProfitTickNums = 3;
LogMsg *mklogmsg = new LogMsg();
LogMsg *statelogmsg = new LogMsg();
extern double openTick;
void QuoteDemo::ShowQuote(EESMarketDepthQuoteData* pDepthMarketData){


    //mili-sec haomiao
    //mico-sec weimiao
    //nano-sec namiao
    currTime = tradingDayT + " " + boost::lexical_cast<string>(pDepthMarketData->UpdateTime)+" "+boost::lexical_cast<string>(pDepthMarketData->UpdateMillisec);
    if (start_process == 1) {
        return;
    }
    //开始时间
    boost::posix_time::ptime startTime = getCurrentTimeByBoost();
    //mkTimePeriod = new boost::posix_time::time_period(startTime,boost::posix_time::milliseconds(490));
    //LOG(INFO) << "new md time period:" + boost::lexical_cast<string>(mkTimePeriod->begin()) + "," + boost::lexical_cast<string>(mkTimePeriod->last());
    //lock_guard<recursive_mutex> locker(g_lock_ti);
    //decompose();
    //4 she 5 ru
    char buf[20]={'\0'};
    memset(buf,0,20);
    if (abs(pDepthMarketData->AskPrice1) > 10000000000 || abs(pDepthMarketData->LastPrice) > 10000000000 ) {
        LOG(ERROR) << (boost::lexical_cast<string>(pDepthMarketData->InstrumentID) + "初始化行情出现问题;");
        return;
    }
    if (pDepthMarketData->InstrumentID == NULL||pDepthMarketData->BidPrice1 == NULL) {
        LOG(ERROR) << (boost::lexical_cast<string>(pDepthMarketData->InstrumentID) +"行情数据为空！！！！！");
        return;
    }
    string tmpstr = "instrumentID=" +  boost::lexical_cast<string>(pDepthMarketData->InstrumentID) + ";lastprice=" + boost::lexical_cast<string>(pDepthMarketData->LastPrice) +
        ";bidprice=" + boost::lexical_cast<string>(pDepthMarketData->BidPrice1) + ";askprice=" + boost::lexical_cast<string>(pDepthMarketData->AskPrice1);
    cout << "marketdata=" + tmpstr <<endl;
    sprintf(buf, "%.2f", pDepthMarketData->LastPrice);
    sscanf(buf, "%lf", &pDepthMarketData->LastPrice);
    memset(buf, 0, 20);

    sprintf(buf, "%.2f", pDepthMarketData->BidPrice1);
    sscanf(buf, "%lf", &pDepthMarketData->BidPrice1);
    memset(buf, 0, 20);
    sprintf(buf, "%.2f", pDepthMarketData->AskPrice1);
    sscanf(buf, "%lf", &pDepthMarketData->AskPrice1);

    string instrumentID = string(pDepthMarketData->InstrumentID);
    unordered_map<string, InstrumentInfo*>::iterator insIT = instruments.find(instrumentID);
    if(insIT != instruments.end()){
        InstrumentInfo* info = insIT->second;
        if(!info->isPriceInit){
            info->LowerLimitPrice = pDepthMarketData->LowerLimitPrice;
            info->UpperLimitPrice = pDepthMarketData->UpperLimitPrice;
            cout<<"init instrumentid" <<endl;
            //initPriceGap(boost::lexical_cast<string>(instrumentID));
            info->isPriceInit = true;
        }

    }
    double lastPrice = pDepthMarketData->LastPrice;
    realTradePrice = lastPrice;
    double currentPrice = pDepthMarketData->LastPrice;
    //instr_price_map[pDepthMarketData->InstrumentID] = currentPrice;
    int volume = pDepthMarketData->Volume;//成交量
    double turnover = pDepthMarketData->Turnover;
    double multiply = getMultipler(instrumentID);
    double tickPrice = getPriceTick(instrumentID);
    //cout<<"openTick="<<boost::lexical_cast<string>(openTick)<<",tickPRice="<<boost::lexical_cast<string>(tickPrice)<<endl;
    //return;
    MarketData* marketdatainfo;
    unordered_map<string, MarketData*>::iterator it = instrinfo.find(instrumentID);
    if(it == instrinfo.end()){
        marketdatainfo = new MarketData();//保存当前行情数据
        marketdatainfo->updateTime = string(pDepthMarketData->UpdateTime);
        strcpy(marketdatainfo->updateTime_char,pDepthMarketData->UpdateTime);
        marketdatainfo->instrumentID = instrumentID;
        marketdatainfo->volume = volume;
        marketdatainfo->bidPrice = pDepthMarketData->BidPrice1;
        marketdatainfo->askPrice = pDepthMarketData->AskPrice1;
        marketdatainfo->lastPrice = currentPrice;

        marketdatainfo->turnover = turnover;
        if(volume !=0 && multiply != 0){
            marketdatainfo->simPrice = turnover/volume/multiply;
            //marketdatainfo->simPrice = turnover/volume;
        }else{
            LOG(ERROR) << "OnRtnDepthMarketData:volume or tickPrice is zero!!!";
            return;
        }
        instrinfo[instrumentID] = marketdatainfo;
    }else{
        marketdatainfo = it->second;
        int prevolume = marketdatainfo->volume;
        double preTurnover = marketdatainfo->turnover;
        int tmpVolume = volume - prevolume;
        double tmpTurnover = turnover - preTurnover;
        marketdatainfo->volume = volume;
        marketdatainfo->turnover = turnover;
        marketdatainfo->updateTime = string(pDepthMarketData->UpdateTime);
        strcpy(marketdatainfo->updateTime_char,pDepthMarketData->UpdateTime);
        marketdatainfo->bidPrice = pDepthMarketData->BidPrice1;
        marketdatainfo->askPrice = pDepthMarketData->AskPrice1;
        marketdatainfo->lastPrice = currentPrice;
        if(multiply != 0&&tmpVolume != 0){
            //marketdatainfo->simPrice = tmpTurnover/tmpVolume;//multiply needed in shfe
            marketdatainfo->simPrice = tmpTurnover/tmpVolume/multiply;
        }
    }
    techCls.RunMarketData(pDepthMarketData);
    //return;
    string msg="businessType=wtm_6001;tradingDay="+boost::lexical_cast<string>(tradingDay)+";logTime="+currTime + ";logType=2;mainDirection="+techCls.mainDirection+";stgStatus="+techCls.stgStatus+";priceStatus="+techCls.priceStatus+";lastPrice="+boost::lexical_cast<string>(lastPrice);
    sendMSG(msg);
    LOG(INFO) << "mainDirection="+techCls.mainDirection+",stgStatus="+techCls.stgStatus+",priceStatus="+techCls.priceStatus+",lastPrice="+boost::lexical_cast<string>(lastPrice)+",15s k line size="+boost::lexical_cast<string>(techCls.KData_15s.size());
    //strategy
    boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
    if(techCls.mainDirection=="0"||techCls.mainDirection=="3"||techCls.mainDirection=="02"){
        //if((techCls.stgStatus=="0"||techCls.stgStatus=="1"||techCls.stgStatus=="2")&&!techCls.newestData15M->min3_last){
        if((techCls.stgStatus=="0"||techCls.stgStatus=="1"||techCls.stgStatus=="2")){
            if(techCls.KData_15s.size() <= 1){
                LOG(INFO) << "the first k 15 line has not generated,not process!";
                //continue;
            }else if(techCls.KData_15s.size() == 2){
                LOG(INFO) << "for long condition,the first k 15 line of signal has appeared!";
                if(techCls.stgStatus=="0"){
                    techCls.stgStatus="1";
                    LOG(INFO) << "This is the just tick price to generate a 15s k line,not process!but init shadow and sun line";
                    initSunOrShadowLine(techCls.mainDirection);
                }else if(techCls.stgStatus=="1"){
                    techCls.stgStatus="2";
                    LOG(INFO) << "This is the just first tick price of a newly generated 15s k line,and compute dual trust parameters!";
                    computeDualTrustPara(pDepthMarketData->LastPrice);
                }else if(techCls.stgStatus=="2"){//step into first open status
                    LOG(INFO) << "this is the second 15s k line's first open status.firstOpenKLineType="+techCls.firstOpenKLineType;
                    if(techCls.firstOpenKLineType=="1"){//sun line
                        if(lastPrice>=techCls.limit[1]){
                            if(existUntradeOrder("3000",NULL)){
                                LOG(INFO) << "There are orders not fully traded,waiting.";
                            }else{
                                LOG(INFO) << "Begin to order insert first open orders.range=["+boost::lexical_cast<string>(techCls.limit[0])+","
                                        +boost::lexical_cast<string>(techCls.limit[1])+"],lastPrice="+boost::lexical_cast<string>(lastPrice);
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="3000";
                                addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
                            }
                        }else{
                            LOG(INFO)<<"sun line,but lastprice="+boost::lexical_cast<string>(lastPrice)+" not over upper limit="+boost::lexical_cast<string>(techCls.limit[1]);
                        }
                    }else if(techCls.firstOpenKLineType=="2"){//shadow line
                        LOG(INFO) << "This is the first 15s k shadow line,not process.";
                    }else if(techCls.firstOpenKLineType=="3"){//flat line
                        LOG(INFO) << "This is a flat 15s k line,not process.";
                    }else{
                        LOG(ERROR)<<"ERROR:wrong firstOpenKLineTyp="+techCls.firstOpenKLineType;
                    }
                }
            }else if(techCls.KData_15s.size() == 3){//second 15s k line
                if(techCls.stgStatus=="2"){//though step into first open,but no order traded.
                    //OrderInfo* orderInfo;
                    if(existUntradeOrder("3000",&orderInfo)){
                        if(orderInfo.status=="0"){
                            LOG(INFO) << "There are untrade order,and not action.So we will execute order action.";
                            tryOrderAction(instrumentID,&orderInfo,"1");
                            orderInfo.status="1";
                        }
                    }else{
                        if(techCls.firstOpenKLineType=="1"){//sun line
                            techCls.stgStatus="1";
                            vector<Strategy::Kdata>::iterator d15it=techCls.KData_15s.begin();
                            techCls.KData_15s.erase(d15it);
                            LOG(INFO) << "sun line.This is the just tick price to generate a 15s k line.because first open not happen in last 15s k line,so not process!but init shadow and sun line again,and reset stgStatus to 1.delete first k 15s line item.after delete size="+
                                          boost::lexical_cast<string>(techCls.KData_15s.size())+"(hope size is 2)";
                            initSunOrShadowLine(techCls.mainDirection);
                        }else if(techCls.firstOpenKLineType=="2"){//shadow line
                            LOG(INFO) << "current's 15s k closePrice="+
                                          boost::lexical_cast<string>(techCls.trueKData15S->closePrice)+",limit=["+
                                          boost::lexical_cast<string>(techCls.limit[0])+","+
                                          boost::lexical_cast<string>(techCls.limit[1])+"]."  ;
                            if(techCls.trueKData15S->closePrice > techCls.limit[1]){
                                LOG(INFO) << "Last 15s k is a shadow line,and current's closePrice="+
                                              boost::lexical_cast<string>(techCls.trueKData15S->closePrice)+" is bigger than upper edge="+
                                              boost::lexical_cast<string>(techCls.limit[1])+" of dual limit.Begin to order insert first open orders.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="3000";
                                addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);

                            }else{
                                techCls.stgStatus="1";
                                vector<Strategy::Kdata>::iterator d15it=techCls.KData_15s.begin();
                                techCls.KData_15s.erase(d15it);
                                string msg="shadow line.This is the just tick price to generate a 15s k line.because first open not happen in last 15s k line,so not process!but init shadow and sun line again,and reset stgStatus to 1.delete first k 15s line item.after delete size="+boost::lexical_cast<string>(techCls.KData_15s.size())+"(hope size is 2)";
                                LOG(INFO) << msg;
                                initSunOrShadowLine(techCls.mainDirection);
                            }
                        }

                    }
                }
            }else{
                LOG(ERROR)<<"KData_15s.size() is "+boost::lexical_cast<string>(techCls.KData_15s.size())+", not define when status=0.";
            }
        }else if(techCls.stgStatus=="3"){//now first open is ok,not care 15s k line.
            if(techCls.priceStatus=="0"){//just first open happen.
                double priceTick=getPriceTick(instrumentID);
                if(lastPrice >= techCls.firstOpenPrice&&lastPrice<(techCls.firstOpenPrice+techCls.nTickMoveSL*priceTick)){
                    LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["+
                                  boost::lexical_cast<string>(techCls.firstOpenPrice)+","+
                                  boost::lexical_cast<string>(techCls.firstOpenPrice+techCls.nTickMoveSL*priceTick)+"].This is the first adjust range,not process.";
                }else if(lastPrice >= (techCls.firstOpenPrice+techCls.nTickMoveSL*priceTick)&&lastPrice<(techCls.firstOpenPrice+2*techCls.nTickMoveSL*priceTick)){
                    techCls.priceStatus="1";
                    LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                  +boost::lexical_cast<string>(techCls.firstOpenPrice+techCls.nTickMoveSL*priceTick)+","
                                  +boost::lexical_cast<string>(techCls.firstOpenPrice+2*techCls.nTickMoveSL*priceTick)+"].This is the second adjust range,move up one tick for stop loss price,change stop loss price from "
                                  +boost::lexical_cast<string>(techCls.stopLossPrice)+" to "
                                  +boost::lexical_cast<string>(techCls.firstOpenPrice+priceTick*techCls.stopLossPriceTick);
                    techCls.stopLossPrice=techCls.firstOpenPrice+priceTick*techCls.stopLossPriceTick;
                }else if(lastPrice>=(techCls.firstOpenPrice+2*techCls.nTickMoveSL*priceTick)){
                    if(existUntradeOrder("2022",NULL)){
                        LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                      +boost::lexical_cast<string>(techCls.firstOpenPrice+2*techCls.nTickMoveSL*priceTick)+","
                                      +"].There are untrade order,not process.";
                    }else{
                        /*LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                      +boost::lexical_cast<string>(techCls.firstOpenPrice+2*techCls.nTickMoveSL*priceTick)+","
                                      +"].This is the '2 tick trigger stop loss' range,change stop loss price from "
                                      +boost::lexical_cast<string>(techCls.stopLossPrice)+" to "
                                      +boost::lexical_cast<string>(lastPrice-techCls.nJumpTriggerSL*priceTick);*/
                        LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                      +boost::lexical_cast<string>(techCls.firstOpenPrice+2*techCls.nTickMoveSL*priceTick)+","
                                      +"].This is the '2 tick trigger stop loss' range.add new order.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2022";
                        //addNewOrderTrade(instrumentID,"0","0",lastPrice,ceil(techCls.firstMetricVolume/3.0),"0",addinfo);
                        addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
                    }
                }else {//if price between one sweet range
                    LOG(INFO)<<"Price may step into one sweet range.lastPrice="+boost::lexical_cast<string>(lastPrice)+",firstToSweet=["+
                               boost::lexical_cast<string>(techCls.firstOpenPrice)+","+boost::lexical_cast<string>(techCls.firstOpenPrice - techCls.firstToSweetTickNums*tickPrice)+").";
                    WaitForCloseInfo* wfc_lastOpen = allTradeList.back();
                    if(wfc_lastOpen->openPrice < techCls.firstOpenPrice){
                        LOG(INFO)<<"There is order traded in one sweet range,showing that price trace is sweet->first->sweet,set priceStatus=3.";
                        techCls.priceStatus="3";
                    }else{
                        double simFirstOpenPrice=techCls.firstOpenPrice - techCls.firstToSweetTickNums*tickPrice;
                        if(lastPrice > simFirstOpenPrice){
                            LOG(INFO)<<"price not touch sweet.";
                        }else if(lastPrice >= (simFirstOpenPrice - techCls.oneSweetGrade*techCls.oneSweetGap*tickPrice)){
                            LOG(INFO)<<"price in sweet range.";
                            for(int i=0;i<techCls.oneSweetGrade;i++){
                                if(lastPrice<=simFirstOpenPrice - (i+1)*techCls.oneSweetGap*tickPrice
                                        &&lastPrice > simFirstOpenPrice - (i+2)*techCls.oneSweetGap*tickPrice){
                                    LOG(INFO) << "now lastPrice step into one sweet range's "+boost::lexical_cast<string>(i+1)
                                                  +" and "+boost::lexical_cast<string>(i+2)+" jump.";

                                    if(lastPrice < wfc_lastOpen->openPrice&&(int((simFirstOpenPrice - lastPrice)/tickPrice))%techCls.oneSweetGap==0){
                                        LOG(INFO)<<"This price is at one sweet gap,judge if add new order.";
                                        sof->instrumentID=instrumentID;
                                        sof->direction="0";
                                        sof->offsetFlag="0";
                                        sof->lastPrice=lastPrice;
                                        sof->volume=techCls.oneSweetVolume;
                                        sof->orderType="0";
                                        sof->openStgType="2032";
                                        doSpecOrder(sof);/*
                                        OrderInfo orderInfo;
                                        if(existUntradeOrder("2032",&orderInfo)){
                                            if(orderInfo.price==lastPrice){
                                                LOG(INFO) << "There are untrade order at this price,not process.";
                                            }else{
                                                LOG(INFO) << "There are untrade order exist,but not at this price,add new order.";
                                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                                addinfo->openStgType="2032";
                                                addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
                                            }
                                        }else{
                                            LOG(INFO) << "There are no untrade order exist,add new order.";
                                            AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                            addinfo->openStgType="2032";
                                            addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
                                        }*/
                                    }else{
                                        LOG(INFO)<<"This price is not at one sweet gap,not process.";
                                    }

                                }
                            }
                        }else{
                            LOG(ERROR)<<"ERROR:step into one normal or two status,NEED TO PROCESS?lastPrice="
                                        +boost::lexical_cast<string>(lastPrice)+",one sweet limit price="
                                        +boost::lexical_cast<string>(techCls.firstOpenPrice-techCls.oneSweetGrade*techCls.oneSweetGap*tickPrice);
                            techCls.priceStatus="3";
                            LOG(INFO)<<"warning:an unbelievable price fluctuation appears.set priceStatus=3 directly.";
                        }

                    }
                }
            }else if(techCls.priceStatus=="1"||techCls.priceStatus=="2"){
                LOG(INFO)<<"lastPrice="+boost::lexical_cast<string>(lastPrice)+ ",priceStatus="+techCls.priceStatus+",price step into between firstOpenPrice and 2 tick jump stop loss.stopLossPrice="
                           +boost::lexical_cast<string>(techCls.stopLossPrice)+",open new order price="
                           +boost::lexical_cast<string>(techCls.firstOpenPrice+2*techCls.nTickMoveSL*tickPrice);
                if(lastPrice <= techCls.stopLossPrice){//
                    //OrderInfo orderInfo;
                    if(existUntradeOrder("2011",&orderInfo)){
                        LOG(INFO)<<"LastPirce ="+boost::lexical_cast<string>(lastPrice)+" trigger stopLossPrice="
                                   +boost::lexical_cast<string>(techCls.stopLossPrice)+",there are untraded order,not process.";
                    }else{
                        LOG(INFO)<<"LastPirce ="+boost::lexical_cast<string>(lastPrice)+" trigger stopLossPrice="
                                   +boost::lexical_cast<string>(techCls.stopLossPrice)+",there are not untraded order,add new order.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2011";
                        addNewOrderTrade(instrumentID,"1","1",lastPrice,userHoldPst.longTotalPosition,"0",addinfo);
                    }
                }else if(lastPrice>=(techCls.firstOpenPrice+2*techCls.nTickMoveSL*tickPrice)){
                    if(int((lastPrice - (techCls.firstOpenPrice+2*techCls.nTickMoveSL*tickPrice))/tickPrice)%2==0){
                        LOG(INFO)<<"price over double nTickMoveSL and satisfy 2 tick jump stop loss.lastPrice="
                                   +boost::lexical_cast<string>(lastPrice)+",firstOpenPrice="
                                   +boost::lexical_cast<string>(techCls.firstOpenPrice)+",double nTickMoveSL="
                                   +boost::lexical_cast<string>(techCls.firstOpenPrice+2*techCls.nTickMoveSL*tickPrice);
                        WaitForCloseInfo* wfc_lastOpen = allTradeList.back();
                        if(lastPrice > wfc_lastOpen->openPrice){
                            if(existUntradeOrder("2022",NULL)){
                                LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                              +boost::lexical_cast<string>(techCls.firstOpenPrice+2*techCls.nTickMoveSL*tickPrice)+","
                                              +"].There are untrade order,not process.";
                            }else{
                                LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                              +boost::lexical_cast<string>(techCls.firstOpenPrice+2*techCls.nTickMoveSL*tickPrice)+","
                                              +"].This is the '2 tick trigger stop loss' range.add new order.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2022";
                                //addNewOrderTrade(instrumentID,"0","0",lastPrice,ceil(techCls.firstMetricVolume/3.0),"0",addinfo);
                                addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
                            }
                        }else{
                            LOG(INFO)<<"There are orders traded at lastPrice="+boost::lexical_cast<string>(lastPrice)+",not process.";
                        }
                    }else{
                        LOG(INFO)<<"price not satisfy 2 tick jump stop loss,not process.lastPrice="
                                   +boost::lexical_cast<string>(lastPrice)+",firstOpenPrice="
                                   +boost::lexical_cast<string>(techCls.firstOpenPrice)+",double nTickMoveSL="
                                   +boost::lexical_cast<string>(techCls.firstOpenPrice+2*techCls.nTickMoveSL*tickPrice);
                    }

                }
            }else if(techCls.priceStatus=="3"){//move in one sweet range
                //LOG(INFO)<<"Price may step in on";
                double simFirstOpenPrice=techCls.firstOpenPrice - techCls.firstToSweetTickNums*tickPrice;
                if(techCls.firstOpenPrice > lastPrice&&lastPrice > simFirstOpenPrice){
                    LOG(INFO)<<"Price in middle range of first and sweet.not process.";
                }else if(lastPrice >= (simFirstOpenPrice-techCls.oneSweetGrade*techCls.oneSweetGap*tickPrice)
                        &&lastPrice <= simFirstOpenPrice){
                    WaitForCloseInfo* wfc_lastOpen = allTradeList.back();
                    for(int i=0;i<techCls.oneSweetGrade;i++){
                        if(lastPrice<=simFirstOpenPrice - (i+1)*techCls.oneSweetGap*tickPrice
                                &&lastPrice > simFirstOpenPrice - (i+2)*techCls.oneSweetGap*tickPrice){
                            LOG(INFO) << "now lastPrice step into one sweet range's "+boost::lexical_cast<string>(i+1)
                                          +" and "+boost::lexical_cast<string>(i+2)+" jump.";

                            if(lastPrice < wfc_lastOpen->openPrice&&(int((simFirstOpenPrice - lastPrice)/tickPrice))%techCls.oneSweetGap==0){
                                LOG(INFO)<<"This price is at one sweet gap,judge if add new order.";
                                sof->instrumentID=instrumentID;
                                sof->direction="0";
                                sof->offsetFlag="0";
                                sof->lastPrice=lastPrice;
                                sof->volume=techCls.oneSweetVolume;
                                sof->orderType="0";
                                sof->openStgType="2032";
                                doSpecOrder(sof);
                            }else{
                                LOG(INFO)<<"This price is not at one sweet gap,not process.";
                            }

                        }
                    }
                }else if(lastPrice > techCls.firstOpenPrice){//step into range of over first open.
                    LOG(INFO)<<"step into range of over first open,cancell all one sweet orders.";
                    cancelSpecTypeOrder(instrumentID,"2032");
                    string tmpsts = techCls.priceStatus;
                    techCls.priceStatus = "0";
                    LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus+",return to status of first open.";
                }else if(lastPrice < (simFirstOpenPrice-techCls.oneSweetGrade*techCls.oneSweetGap*tickPrice - techCls.sweetToNormalTickNums*tickPrice)){
                    LOG(INFO)<<"step into range of one normal.";
                    if(existUntradeOrder("2042",NULL)){
                        LOG(INFO) << "There are untrade order,not process.";
                    }else{
                        LOG(INFO) << "There are no untrade order exist,add new order.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2042";
                        addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.oneNormalVolume,"0",addinfo);
                    }
                }else{
                    LOG(INFO)<<"Where am i?lastPrice="+boost::lexical_cast<string>(lastPrice);
                }
            }else if(techCls.priceStatus=="4"){//one normal
                if(stopProfit("0",lastPrice,instrumentID)){//
                    LOG(INFO)<<"long reverse is stopping profit,not process.";
                }else{
                    //stop profit order not all traded,then first order action.must.
                    cancelSpecTypeOrder(instrumentID,"2001");
                    WaitForCloseInfo* wfc_firstOpen;//the first order
                    WaitForCloseInfo* wfc_lastOpen;//the last order
                    WaitForCloseInfo* wfc_lastSecondOpen;//the last second order
                    if(tmpLongReverseList.size()>0){
                        list<WaitForCloseInfo*>::iterator atIT=tmpLongReverseList.begin();
                        wfc_firstOpen=*atIT;
                    }
                    wfc_lastOpen=tmpLongReverseList.back();
                    if(tmpLongReverseList.size() < techCls.oneNormalGrade){
                        LOG(INFO)<<"long逆向加仓处于第一阈值内,each grade加仓量="+boost::lexical_cast<string>(techCls.oneNormalVolume);
                        if(int((wfc_firstOpen->openPrice-lastPrice)/tickPrice)%techCls.oneNormalGap==0){
                            LOG(INFO)<<"this price is at one normal gap,judge if add new order.lastPrice="+boost::lexical_cast<string>(lastPrice);
                            if(lastPrice < wfc_lastOpen->openPrice){
                                LOG(INFO)<<"lastPrice is lower than the last order price,add new order,加仓量="+boost::lexical_cast<string>(techCls.oneNormalVolume);
                                //OrderInfo orderInfo;
                                if(existUntradeOrder("2042",&orderInfo)){
                                    if(orderInfo.price==lastPrice){
                                        LOG(INFO) << "There are untrade order at this price,not process.";
                                    }else{
                                        LOG(INFO) << "There are untrade order exist,but not at this price,add new order.";
                                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                        addinfo->openStgType="2042";
                                        addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.oneNormalVolume,"0",addinfo);
                                    }
                                }else{
                                    LOG(INFO) << "There are no untrade order exist,add new order.";
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="2042";
                                    addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.oneNormalVolume,"0",addinfo);
                                }
                            }else{
                                LOG(INFO)<<"There are already orders at this price,not process.";
                            }
                        }else{
                            LOG(INFO)<<"This is not order insert price,not process.";
                        }
                    }else if(tmpLongReverseList.size() >= techCls.oneNormalGrade
                             &&lastPrice < wfc_lastOpen->openPrice){//Fibonacci
                        LOG(INFO)<<"Price step into two status:Fibonacci.";
                        int existGrades = (tmpLongReverseList.size() - techCls.oneNormalGrade);
                        int tmpGrade=0;
                        for(list<WaitForCloseInfo*>::iterator wfit=tmpLongReverseList.begin();wfit != tmpLongReverseList.end();wfit++){
                            tmpGrade += 1;
                            if(tmpGrade == techCls.oneNormalGrade){
                                wfc_firstOpen = *wfit;
                                break;
                            }
                        }
                        if(existGrades < techCls.twoGrade){
                            if(int((wfc_firstOpen->openPrice - lastPrice)/tickPrice)%techCls.twoGap==0){
                                LOG(INFO)<<"This price is at two gap,judge if add new order.";
                                if(lastPrice < wfc_lastOpen->openPrice){
                                    LOG(INFO)<<"lastPrice is lower than the last order price,add new order";
                                    int tmpR=0;
                                    for(list<WaitForCloseInfo*>::reverse_iterator wfit=tmpLongReverseList.rbegin();wfit != tmpLongReverseList.rend();wfit++){
                                        tmpR += 1;
                                        if(tmpR == 2){
                                            wfc_lastSecondOpen = *wfit;
                                            break;
                                        }
                                    }
                                    int hopeVolume=wfc_lastOpen->tradeVolume+wfc_lastSecondOpen->tradeVolume;
                                    LOG(INFO)<<"long逆向加仓处于第二阈值内,最后两次加仓数量为:lastSecOrder="
                                               +boost::lexical_cast<string>(wfc_lastSecondOpen->tradeVolume)+",lastOrder.volume="
                                               +boost::lexical_cast<string>(wfc_lastOpen->tradeVolume)+",当前加仓数量:hopeVolume="
                                               +boost::lexical_cast<string>(hopeVolume);
                                    //OrderInfo orderInfo;
                                    if(existUntradeOrder("2052",&orderInfo)){
                                        if(orderInfo.price==lastPrice){
                                            LOG(INFO) << "There are untrade order at this price,not process.";
                                        }else{
                                            LOG(INFO) << "There are untrade order exist,but not at this price,add new order.";
                                            AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                            addinfo->openStgType="2052";
                                            addNewOrderTrade(instrumentID,"0","0",lastPrice,hopeVolume,"0",addinfo);
                                        }
                                    }else{
                                        LOG(INFO) << "There are no untrade order exist,add new order.";
                                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                        addinfo->openStgType="2052";
                                        addNewOrderTrade(instrumentID,"0","0",lastPrice,hopeVolume,"0",addinfo);
                                    }
                                }else{
                                    LOG(INFO)<<"There are already orders at this price,not process.";
                                }

                            }
                        }

                    }else{

                    }
                }
            }else if(techCls.priceStatus=="5"){//tow status
                LOG(INFO)<<"Price step into two status:Fibonacci.";
                if(stopProfit("0",lastPrice,instrumentID)){//
                    LOG(INFO)<<"long reverse is stopping profit,not process.";
                }else{
                    //stop profit order not all traded,then first order action
                    cancelSpecTypeOrder(instrumentID,"2001");
                    WaitForCloseInfo* wfc_firstOpen;//the first order
                    WaitForCloseInfo* wfc_lastOpen;//the last order
                    WaitForCloseInfo* wfc_lastSecondOpen;//the last second order
                    int existGrades = (tmpLongReverseList.size() - techCls.oneNormalGrade);
                    LOG(INFO)<<"Current grade for two is "+boost::lexical_cast<string>(existGrades)
                               +",and the limit grade is "+boost::lexical_cast<string>(techCls.twoGrade);
                    if(existGrades >= techCls.gradeToProtect){
                        LOG(INFO)<<"In two status,over the "+boost::lexical_cast<string>(techCls.gradeToProtect)+" grade will trigger protection order for short.";
                        if(protectList.size() == 0){
                            //OrderInfo orderInfo;
                            if(existUntradeOrder("p",&orderInfo)){
                                if(orderInfo.price==lastPrice){
                                    LOG(INFO) << "There are untrade order at this price,not process.";
                                }else{
                                    LOG(INFO) << "There are untrade order exist,but not at this price,add new order.";
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="p";
                                    addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.protectVolume,"0",addinfo);
                                }
                            }else{
                                LOG(INFO) << "There are no untrade order exist,add new order.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="p";
                                addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.protectVolume,"0",addinfo);
                            }
                        }else{
                            WaitForCloseInfo* p_lastOpen = protectList.back();//the last order
                            if(lastPrice < p_lastOpen->openPrice){
                                LOG(INFO)<<"For protection,lastPrice is lower than the last order price,add new order.";
                                //OrderInfo orderInfo;
                                if(existUntradeOrder("p",&orderInfo)){
                                    if(orderInfo.price==lastPrice){
                                        LOG(INFO) << "There are untrade order at this price,not process.";
                                    }else{
                                        LOG(INFO) << "There are untrade order exist,but not at this price,add new order.";
                                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                        addinfo->openStgType="p";
                                        addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.protectVolume,"0",addinfo);
                                    }
                                }else{
                                    LOG(INFO) << "There are no untrade order exist,add new order.";
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="p";
                                    addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.protectVolume,"0",addinfo);
                                }
                            }else{
                                LOG(INFO)<<"There are orders existed at this price,not process.";
                            }
                        }
                    }
                    int tmpGrade=0;
                    for(list<WaitForCloseInfo*>::iterator wfit=tmpLongReverseList.begin();wfit != tmpLongReverseList.end();wfit++){
                        tmpGrade += 1;
                        if(tmpGrade == techCls.oneNormalGrade){
                            wfc_firstOpen = *wfit;
                            break;
                        }
                    }
                    wfc_lastOpen=tmpLongReverseList.back();
                    if(existGrades < techCls.twoGrade){
                        if(int((wfc_firstOpen->openPrice - lastPrice)/tickPrice)%techCls.twoGap==0){
                            LOG(INFO)<<"This price is at two gap,judge if add new order.";

                            if(lastPrice < wfc_lastOpen->openPrice){
                                LOG(INFO)<<"lastPrice is lower than the last order price,add new order.";
                                int tmpR=0;
                                for(list<WaitForCloseInfo*>::reverse_iterator wfit=tmpLongReverseList.rbegin();wfit != tmpLongReverseList.rend();wfit++){
                                    tmpR += 1;
                                    if(tmpR == 2){
                                        wfc_lastSecondOpen = *wfit;
                                        break;
                                    }
                                }
                                int hopeVolume=wfc_lastOpen->tradeVolume+wfc_lastSecondOpen->tradeVolume;
                                LOG(INFO)<<"long逆向加仓处于第二阈值内,最后两次加仓数量为:lastSecOrder="
                                           +boost::lexical_cast<string>(wfc_lastSecondOpen->tradeVolume)+",lastOrder.volume="
                                           +boost::lexical_cast<string>(wfc_lastOpen->tradeVolume)+",当前加仓数量:hopeVolume="
                                           +boost::lexical_cast<string>(hopeVolume);
                                //OrderInfo orderInfo;
                                if(existUntradeOrder("2052",&orderInfo)){
                                    if(orderInfo.price==lastPrice){
                                        LOG(INFO) << "There are untrade order at this price,not process.";
                                    }else{
                                        LOG(INFO) << "There are untrade order exist,but not at this price,add new order.lastPrice="+boost::lexical_cast<string>(lastPrice)
                                                     +",untradePrice="+boost::lexical_cast<string>(orderInfo.price);
                                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                        addinfo->openStgType="2052";
                                        addNewOrderTrade(instrumentID,"0","0",lastPrice,hopeVolume,"0",addinfo);
                                    }
                                }else{
                                    LOG(INFO) << "There are no untrade order exist,add new order.";
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="2052";
                                    addNewOrderTrade(instrumentID,"0","0",lastPrice,hopeVolume,"0",addinfo);
                                }
                            }else{
                                LOG(INFO)<<"There are already orders at this price,not process.lastPrice="+boost::lexical_cast<string>(lastPrice)+",lastOpenPrice="
                                           +boost::lexical_cast<string>(wfc_lastOpen->openPrice);
                            }

                        }
                    }else if(existGrades >= techCls.twoGrade&&lastPrice < wfc_lastOpen->openPrice){//lock position
                        LOG(INFO)<<"Warning!!!!!Lock position!!!!Current grade for two is "+boost::lexical_cast<string>(existGrades)
                                   +",and the limit grade is "+boost::lexical_cast<string>(techCls.twoGrade)
                                   +",lastPrice="+boost::lexical_cast<string>(lastPrice)+" is lower than the last open price="
                                   +boost::lexical_cast<string>( wfc_lastOpen->openPrice);
                        sof->instrumentID=instrumentID;
                        sof->direction="1";
                        sof->offsetFlag="0";
                        sof->lastPrice=lastPrice;
                        sof->volume=userHoldPst.longTotalPosition;
                        sof->orderType="0";
                        sof->openStgType="2062";
                        doSpecOrder(sof);
                        //OrderInfo orderInfo;
                        /*
                        if(existUntradeOrder("2062",&orderInfo)){
                            if(orderInfo.price == lastPrice){
                                LOG(INFO) << "There are untrade order at this price,not process.";
                            }else{
                                LOG(INFO) << "There are untrade order for 2062,but not at this price,reinsert.";
                                cancelSpecTypeOrder(instrumentID,"2062");
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2062";
                                addNewOrderTrade(instrumentID,"1","0",lastPrice,userHoldPst.longTotalPosition,"0",addinfo);
                            }

                        }else{
                            LOG(INFO) << "There are no untrade order exist,add new order.";
                            AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                            addinfo->openStgType="2062";
                            addNewOrderTrade(instrumentID,"1","0",lastPrice,userHoldPst.longTotalPosition,"0",addinfo);
                        }*/
                    }
                }

            }else if(techCls.priceStatus=="6"){
                WaitForCloseInfo* wfc_lastOpen=tmpLongReverseList.back();
                double ATR15S_60C=1.5*tickPrice;//get this techmetric from lib
                if(ATR15S_60C == 0){
                    //init 60 circle 15s atr
                }
                LOG(INFO)<<"ATR="+boost::lexical_cast<string>(ATR15S_60C);
                if(lastPrice <= (wfc_lastOpen->openPrice - techCls.watchUnlockATRNums*ATR15S_60C)){//price go down
                    LOG(INFO)<<"WATCHING:price goes down to "+boost::lexical_cast<string>(techCls.watchUnlockATRNums)+" atr lower.lastPrice="+boost::lexical_cast<string>(lastPrice)+",lastOpenPrice="
                               +boost::lexical_cast<string>(wfc_lastOpen->openPrice);
                    if(techCls.minPrice == 0){
                        techCls.minPrice = lastPrice;
                        LOG(INFO)<<"begin to initialize minPrice="+boost::lexical_cast<string>(techCls.minPrice);
                    }else if(techCls.minPrice > lastPrice){
                        double tmprice=techCls.minPrice;
                        techCls.minPrice = lastPrice;
                        LOG(INFO)<<"Price become even more lower,set minPrice from "+boost::lexical_cast<string>(tmprice)+" to "
                                   +boost::lexical_cast<string>( techCls.minPrice);
                    }else{
                        LOG(INFO)<<"It's seems price may reverse to go up.minPrice="+boost::lexical_cast<string>(techCls.minPrice)+",lastPrice="
                                   +boost::lexical_cast<string>(lastPrice);
                        if((lastPrice - techCls.minPrice) >= techCls.afterWatchUnlockATRNums*ATR15S_60C){
                            LOG(INFO)<<"Unlock condition one:Price triggered short direction stop loss,begin to unlock.";
                            sof->instrumentID=instrumentID;
                            sof->direction="0";
                            sof->offsetFlag="1";
                            sof->lastPrice=lastPrice;
                            sof->volume=userHoldPst.shortTotalPosition;
                            sof->orderType="0";
                            sof->openStgType="2071";
                            doSpecOrder(sof);
                            //OrderInfo orderInfo;
                            /*
                            if(existUntradeOrder("2071",&orderInfo)){
                                if(orderInfo.price == lastPrice){
                                    LOG(INFO) << "There are untrade order at this price,not process.";
                                }else{
                                    LOG(INFO) << "There are untrade order for 2071,but not at this price,reinsert.";
                                    cancelSpecTypeOrder(instrumentID,"2071");
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="2071";
                                    addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                                }
                            }else{
                                LOG(INFO) << "There are no untrade order exist,add new order.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2071";
                                addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                            }*/
                        }
                    }
                }else if(lastPrice > (wfc_lastOpen->openPrice - techCls.watchUnlockATRNums*ATR15S_60C)&&lastPrice <= wfc_lastOpen->openPrice){
                    //LOG(INFO)<<"";
                    if(techCls.minPrice == 0){
                        LOG(INFO)<<"After lock position,price not touch the lowest point of "+boost::lexical_cast<string>(techCls.watchUnlockATRNums)+" atr.maybe the shake.";
                        if((wfc_lastOpen->openPrice - lastPrice) >= ATR15S_60C){
                            //LOG(INFO)<<"lastPrice="+boost::lexical_cast<string>();
                            LOG(INFO)<<"price has been shaked.";
                            techCls.shake = true;
                        }
                    }else if(techCls.minPrice != 0){
                        LOG(INFO)<<"Price touched the lowest point of "+boost::lexical_cast<string>(techCls.watchUnlockATRNums)+" atr,and return back.";
                        if((lastPrice - techCls.minPrice) >= techCls.afterWatchUnlockATRNums*ATR15S_60C){
                            LOG(INFO)<<"Unlock condition two:Price triggered short direction stop loss,begin to unlock.";

                            sof->instrumentID=instrumentID;
                            sof->direction="0";
                            sof->offsetFlag="1";
                            sof->lastPrice=lastPrice;
                            sof->volume=userHoldPst.shortTotalPosition;
                            sof->orderType="0";
                            sof->openStgType="2071";
                            doSpecOrder(sof);
                            //OrderInfo orderInfo;
                            /*
                            if(existUntradeOrder("2071",&orderInfo)){
                                if(orderInfo.price == lastPrice){
                                    LOG(INFO) << "There are untrade order at this price,not process.";
                                }else{
                                    LOG(INFO) << "There are untrade order for 2071,but not at this price,reinsert.";
                                    cancelSpecTypeOrder(instrumentID,"2071");
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="2071";
                                    addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                                }
                            }else{
                                LOG(INFO) << "There are no untrade order exist,add new order.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2071";
                                addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                            }*/
                        }
                    }
                }else if(lastPrice > wfc_lastOpen->openPrice){
                    if(techCls.minPrice == 0){
                        LOG(INFO)<<"After lock position,price not touch the lowest point of "+boost::lexical_cast<string>(techCls.watchUnlockATRNums)+" atr.";
                        if(techCls.shake){
                            LOG(INFO)<<"after lock,the price has been shaked.";
                            LOG(INFO)<<"Unlock condition three:Price triggered short direction stop loss,begin to unlock.";

                            sof->instrumentID=instrumentID;
                            sof->direction="0";
                            sof->offsetFlag="1";
                            sof->lastPrice=lastPrice;
                            sof->volume=userHoldPst.shortTotalPosition;
                            sof->orderType="0";
                            sof->openStgType="2071";
                            doSpecOrder(sof);
                            //OrderInfo orderInfo;
                            /*
                            if(existUntradeOrder("2071",&orderInfo)){
                                if(orderInfo.price == lastPrice){
                                    LOG(INFO) << "There are untrade order at this price,not process.";
                                }else{
                                    LOG(INFO) << "There are untrade order for 2071,but not at this price,reinsert.";
                                    cancelSpecTypeOrder(instrumentID,"2071");
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="2071";
                                    addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                                }
                            }else{
                                LOG(INFO) << "There are no untrade order exist,add new order.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2071";
                                addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                            }*/
                        }else if(lastPrice > (wfc_lastOpen->openPrice + 2*tickPrice)){//this type of unlock need to be disgrude.
                            LOG(INFO)<<"after lock,the price has never been shaked.";
                            LOG(INFO)<<"Unlock condition four:Price triggered short direction stop loss,begin to unlock.";
                            sof->instrumentID=instrumentID;
                            sof->direction="0";
                            sof->offsetFlag="1";
                            sof->lastPrice=lastPrice;
                            sof->volume=userHoldPst.shortTotalPosition;
                            sof->orderType="0";
                            sof->openStgType="2071";
                            doSpecOrder(sof);
                            /*
                            if(existUntradeOrder("2071",NULL)){
                                LOG(INFO) << "There are untrade order at this price,not process.";
                            }else{
                                LOG(INFO) << "There are no untrade order exist,add new order.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2071";
                                addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                            }*/
                        }
                    }else{
                        LOG(INFO)<<"lastPrice="+boost::lexical_cast<string>(lastPrice)+">lastOpenPrice="+boost::lexical_cast<string>(wfc_lastOpen->openPrice)
                                   +",and minPrice="+boost::lexical_cast<string>(techCls.minPrice)+",judge if unlock.";
                        if((lastPrice - techCls.minPrice) >= techCls.afterWatchUnlockATRNums*ATR15S_60C){
                            LOG(INFO)<<"Unlock condition xxx:Price triggered short direction stop loss,begin to unlock.";
                            sof->instrumentID=instrumentID;
                            sof->direction="0";
                            sof->offsetFlag="1";
                            sof->lastPrice=lastPrice;
                            sof->volume=userHoldPst.shortTotalPosition;
                            sof->orderType="0";
                            sof->openStgType="2071";
                            doSpecOrder(sof);
                            /*
                            if(existUntradeOrder("2071",NULL)){
                                LOG(INFO) << "There are untrade order at this price,not process.";
                            }else{
                                LOG(INFO) << "There are no untrade order exist,add new order.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2071";
                                addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                            }*/
                        }
                        //LOG(ERROR)<<"ERROR:minPrice="+boost::lexical_cast<string>(techCls.minPrice)+",never take process this price.";
                    }
                }
            }else if(techCls.priceStatus == "7"){//watch high price
                LOG(INFO)<<"now has been unlock,begin to watch high price ready to long unlock.";
                double ATR15S_60C=1.5*tickPrice;//get this techmetric from lib
                if(ATR15S_60C == 0){
                    //init 60 circle 15s atr
                }
                LOG(INFO)<<"ATR="+boost::lexical_cast<string>(ATR15S_60C)+",current maxPrice="+boost::lexical_cast<string>(techCls.maxPrice);
                if(lastPrice >= (techCls.unlockPrice + techCls.watchUnlockAnotherATRNums*ATR15S_60C)){
                    if(techCls.maxPrice == 0){
                        techCls.maxPrice = lastPrice;
                    }else if(techCls.maxPrice < lastPrice){
                        techCls.maxPrice = lastPrice;
                    }else{
                        if((techCls.maxPrice - lastPrice) >= techCls.afterWatchUnlockOtherATRNums*ATR15S_60C){
                            LOG(INFO)<<"Unlock condition five:Price triggered long direction stop loss,begin to unlock.";
                            sof->instrumentID=instrumentID;
                            sof->direction="1";
                            sof->offsetFlag="1";
                            sof->lastPrice=lastPrice;
                            sof->volume=userHoldPst.longTotalPosition;
                            sof->orderType="0";
                            sof->openStgType="2081";
                            doSpecOrder(sof);
                            /*
                            if(existUntradeOrder("2081",NULL)){
                                LOG(INFO) << "There are untrade order at this price,not process.";
                            }else{
                                LOG(INFO) << "There are no untrade order exist,add new order.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2081";
                                addNewOrderTrade(instrumentID,"1","0",lastPrice,userHoldPst.longTotalPosition,"0",addinfo);
                            }*/
                        }
                    }
                }else{
                    LOG(INFO)<<"Not trigger lastPrice="+boost::lexical_cast<string>(lastPrice) +" >= (techCls.unlockPrice + n*ATR15S_60C)="
                               +boost::lexical_cast<string>(techCls.unlockPrice)+"+"
                               +boost::lexical_cast<string>(ATR15S_60C*techCls.watchUnlockAnotherATRNums)+")";
                }
                if((lastPrice - techCls.unlockPrice) >= techCls.timesOfStopLoss*techCls.twoGap*tickPrice){
                    LOG(INFO)<<"Unlock condition six:Price triggered long direction stop loss,begin to unlock.price over "+boost::lexical_cast<string>(techCls.timesOfStopLoss)+" times * twogap.";
                    sof->instrumentID=instrumentID;
                    sof->direction="1";
                    sof->offsetFlag="1";
                    sof->lastPrice=lastPrice;
                    sof->volume=userHoldPst.longTotalPosition;
                    sof->orderType="0";
                    sof->openStgType="2081";
                    doSpecOrder(sof);/*
                    if(existUntradeOrder("2081",NULL)){
                        LOG(INFO) << "There are untrade order at this price,not process.";
                    }else{
                        LOG(INFO) << "There are no untrade order exist,add new order.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2081";
                        addNewOrderTrade(instrumentID,"1","0",lastPrice,userHoldPst.longTotalPosition,"0",addinfo);
                    }*/
                }else if((techCls.unlockPrice - lastPrice) >= techCls.relockATRNums*ATR15S_60C){
                    LOG(INFO)<<"WARNING:relock position!!!!After unlock,price goes down again,and trigger relock condition.unlockPrice("+boost::lexical_cast<string>(techCls.unlockPrice)+")-lastPrice("
                               +boost::lexical_cast<string>(lastPrice)+")<=relockATRNums*ATR15S_60C("+boost::lexical_cast<string>(techCls.relockATRNums*ATR15S_60C)+").";
                    LOG(INFO)<<"before relock,check if all of last unlock order is traded.if not then action.";
                    if(existUntradeOrder("2071",NULL)){
                        tryAllOrderAction(instrumentID);
                    }else{
                        sof->instrumentID=instrumentID;
                        sof->direction="1";
                        sof->offsetFlag="0";
                        sof->lastPrice=lastPrice;
                        sof->volume=userHoldPst.longTotalPosition-userHoldPst.shortTotalPosition;
                        sof->orderType="0";
                        sof->openStgType="2062";
                        doSpecOrder(sof);/*
                        if(existUntradeOrder("2062",NULL)){
                            LOG(INFO) << "There are untrade order at this price,not process.";
                        }else{
                            LOG(INFO) << "There are no untrade order exist,add new order.";
                            AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                            addinfo->openStgType="2062";
                            addNewOrderTrade(instrumentID,"1","0",lastPrice,userHoldPst.longTotalPosition-userHoldPst.shortTotalPosition,"0",addinfo);
                        }*/
                    }

                }else{
                    LOG(INFO)<<"Not trigger (lastPrice="+boost::lexical_cast<string>(lastPrice)+"-unlockPrice"+boost::lexical_cast<string>(techCls.unlockPrice) +") >= (4*techCls.twoGap*tickPrice)=(4*"
                               +boost::lexical_cast<string>(techCls.twoGap)+"*"
                               +boost::lexical_cast<string>(tickPrice)+")";
                }
            }else{
                LOG(INFO)<<"priceStatus="+techCls.priceStatus+",this kind of status not define.";
            }
        }else if(techCls.stgStatus == "10"){
            LOG(INFO)<<"First step into long direction watching status,we will transfer all position to reverseList.allTradeList="+boost::lexical_cast<string>(allTradeList.size())+",reserveList="
                       +boost::lexical_cast<string>(longReverseList.size());
            if(allTradeList.size() > 0){
                for(list<WaitForCloseInfo*>::iterator wfIt = allTradeList.begin();wfIt!=allTradeList.end();){
                    //tmpLongReverseList->push_back(*wfIt);
                    longReverseList.push_back(*wfIt);
                    wfIt = allTradeList.erase(wfIt);
                }
            }
            double tmpMax=max(abs(techCls.trueKData15M->highPrice - techCls.trueKData15M->closePrice),abs(techCls.trueKData15M->closePrice - techCls.trueKData15M->lowPrice));
            //techCls.limit[1]=lastPrice+0.7*tmpMax;
            //techCls.limit[0]=lastPrice-1*tmpMax;
            techCls.limit[0]=lastPrice;
            techCls.limit[1]=lastPrice;
            LOG(INFO)<<"The first tick of long direction watching status,compute dual parameters.limit=["+boost::lexical_cast<string>(techCls.limit[0])+","
                    +boost::lexical_cast<string>(techCls.limit[1])+"]";
            techCls.stgStatus="11";
            techCls.minPrice=0;
            techCls.maxPrice=0;
            techCls.firstOpenKLineType="0";
            techCls.KData_15s.clear();
            techCls.beginK15s = true;//from here start 15s k line.
            LOG(INFO)<<"After transfer all position to reverseList.allTradeList="+boost::lexical_cast<string>(allTradeList.size())+",reserveList="
                       +boost::lexical_cast<string>(longReverseList.size())+",begin to close all position.";
            if(existUntradeOrder("4000",NULL)){
                LOG(INFO) << "There are orders not fully traded,waiting.";
            }else{
                LOG(INFO) << "There are not orders untraded,Begin Close all.";
                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                addinfo->openStgType="4000";
                if(userHoldPst.longTotalPosition>0){
                    addNewOrderTrade(instrumentID,"1","1",lastPrice,userHoldPst.longTotalPosition,"0",addinfo);
                }
                if(userHoldPst.shortTotalPosition>0){
                    addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                }
            }
        }else if(techCls.stgStatus == "11"){
            LOG(INFO)<<"First step into long direction watching status,next broken will be true direction.limit=["+boost::lexical_cast<string>(techCls.limit[0])+","
                    +boost::lexical_cast<string>(techCls.limit[1])+"]";
            if(lastPrice >= techCls.limit[1]){
                string tmpdir = techCls.mainDirection;
                techCls.mainDirection = "0";
                LOG(INFO)<<"current price="+boost::lexical_cast<string>(lastPrice)+",over upper limit.limit=["+boost::lexical_cast<string>(techCls.limit[0])+","
                        +boost::lexical_cast<string>(techCls.limit[1])+"],reset main direction from "+tmpdir+" to "+techCls.mainDirection;
                if(existUntradeOrder("3000",NULL)){
                    LOG(INFO) << "There are orders not fully traded,waiting.";
                }else{
                    LOG(INFO) << "Begin to order insert first open orders.range=["+boost::lexical_cast<string>(techCls.limit[0])+","
                            +boost::lexical_cast<string>(techCls.limit[1])+"],lastPrice="+boost::lexical_cast<string>(lastPrice);
                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                    addinfo->openStgType="3000";
                    addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
                }
            }else if(lastPrice <= techCls.limit[0]){
                string tmpdir = techCls.mainDirection;
                techCls.mainDirection = "1";
                LOG(INFO)<<"current price="+boost::lexical_cast<string>(lastPrice)+",over lower limit.limit=["+boost::lexical_cast<string>(techCls.limit[0])+","
                        +boost::lexical_cast<string>(techCls.limit[1])+"],reset main direction from "+tmpdir+" to "+techCls.mainDirection;
                if(existUntradeOrder("3000",NULL)){
                    LOG(INFO) << "There are orders not fully traded,waiting.";
                }else{
                    LOG(INFO) << "Begin to order insert first open orders.range=["+boost::lexical_cast<string>(techCls.limit[0])+","
                            +boost::lexical_cast<string>(techCls.limit[1])+"],lastPrice="+boost::lexical_cast<string>(lastPrice);
                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                    addinfo->openStgType="3000";
                    //addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
                }
            }
        }

    }
    //metricProcesserForSingleThread(marketdatainfo);
    //techMetricQueue.push(marketdatainfo);//compute tech metric
    //getAvailableClosePosition(instrumentID);
    ///////////
    //当前行情
    double simPrice = marketdatainfo->simPrice;
    //int intPart = round(simPrice/tickPrice);//price//self.tickPrice
    double tmpLastPrice = lastPrice;
    /*
    if(abs(upRange - simPrice) > abs(simPrice - downRange)){//距离下边界近
        lastPrice = downRange;
    }else{
        lastPrice = upRange;
    }*/
    string tmpmkdata=
            "businessType=0;tradingDay="+boost::lexical_cast<string>(tradeDay)+
            ";instrumentID="+instrumentID+
            ";updateTime="+marketdatainfo->updateTime+
            ";bidPrice1="+boost::lexical_cast<string>(pDepthMarketData->BidPrice1)+
            ";bidVolume1="+boost::lexical_cast<string>(pDepthMarketData->BidVolume1)+
            ";askPrice1="+boost::lexical_cast<string>(pDepthMarketData->AskPrice1)+
            ";askVolume1="+boost::lexical_cast<string>(pDepthMarketData->AskVolume1)+
            ";lastPrice=" + boost::lexical_cast<string>(tmpLastPrice) +
            ";volume="+boost::lexical_cast<string>(pDepthMarketData->Volume)+
            ";turnover="+boost::lexical_cast<string>(pDepthMarketData->Turnover)+
            ";avgprice=" + boost::lexical_cast<string>(pDepthMarketData->AveragePrice) +
            ";simPrice=" + boost::lexical_cast<string>(simPrice);
    //sendMSG(tmpmkdata);
    //mklogmsg->setMsg(tmpmkdata);
    //networkTradeQueue.push(mklogmsg);
    //LOG(INFO) << tmpmkdata;
    int heatBeatSecond = getTimeInterval(HEARTBEAT, startTime, "s");
    if (heatBeatSecond >= 10) {
        //LOG(ERROR) << "marketdata work fine!";
        HEARTBEAT = startTime;
    }
}



void addNewOrder(string instrumentID,string direction,string offsetFlag,double orderInsertPrice,int volume,string mkType,AdditionOrderInfo* addinfo){
//void addNewOrder(AdditionOrderInfo* addinfo){

    //string instrumentID = boost::lexical_cast<string>(pDepthMarketData->InstrumentID);
    unsigned int newOrderToken = iOrderRef++;
    if(addinfo){
        addinfo->clientOrderToken=newOrderToken;
    }
    double price = orderInsertPrice;
    unsigned char m_side = 0;
    string orderType = "-1";
    if(direction == "0"){
        //wait for trade,used for orderAction
        OrderInfo* orderInfo = new OrderInfo();
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
        orderInfo->volume = volume;
        orderInfo->instrumentID = instrumentID;
        orderInfo->begin_down_cul = down_culculate;
        orderInfo->mkType = mkType;
        orderInfo->function = addinfo->function;
        bidList.emplace_back(orderInfo);
        //LOG(INFO) << "down_culculate=" + boost::lexical_cast<string>(down_culculate) + ",up_culculate=" + boost::lexical_cast<string>(up_culculate) + ",buy open.";
        LOG(INFO) << "new add bid list." + getOrderInfo(orderInfo);
    }else{
        //wait for trade,used for orderAction
        OrderInfo* orderInfo = new OrderInfo();
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
        orderInfo->volume = volume;
        orderInfo->instrumentID = instrumentID;
        orderInfo->begin_up_cul = up_culculate;
        orderInfo->mkType = mkType;
        orderInfo->function = addinfo->function;
        askList.emplace_back(orderInfo);
        //LOG(INFO) << "down_culculate=" + boost::lexical_cast<string>(down_culculate) + ",up_culculate=" + boost::lexical_cast<string>(up_culculate) + ",sell open.";
        LOG(INFO) << "new add ask list." + getOrderInfo(orderInfo);
    }
    //shengli
    EES_EnterOrderField orderField;
    memset(&orderField, 0, sizeof(EES_EnterOrderField));
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
    aoi.function = addinfo->function;
    aoi.timeFlag = "0";
    ptradeApi->reqOrderInsert(&orderField,&aoi);
    string tmporder = "";
    LOG(INFO) << tmporder;

}

void calibratePriceAndVolume(string instrumentID,string direction,double lastPrice,double tickPrice,bool isFollow,TempOrderInfo* &tmpInfo){
    double orderInsertPrice = 0;
    double spreadTickPrice = 0;
    int volume = 1;
    if(direction == "1"){//卖出.超过阈值，则以阈值倍数的差价下单
        orderInsertPrice = lastPrice + tickPrice;
        double beforePrice = orderInsertPrice;
        spreadTickPrice = tickPrice;
        if((holdInfo->shortTotalPosition - holdInfo->longTotalPosition)>= volSpread){
            orderInsertPrice = lastPrice + (holdInfo->shortTotalPosition - holdInfo->longTotalPosition)*tickPrice;
            LOG(INFO) << "卖出时，空头=" + boost::lexical_cast<string>(holdInfo->shortTotalPosition) + ",多头="+ boost::lexical_cast<string>(holdInfo->longTotalPosition) + ",空头比多头多"+
                          boost::lexical_cast<string>(holdInfo->shortTotalPosition - holdInfo->longTotalPosition) + "手，调整报单价格由" + boost::lexical_cast<string>(beforePrice) +"->" +
                          boost::lexical_cast<string>(orderInsertPrice);
            spreadTickPrice = (holdInfo->shortTotalPosition - holdInfo->longTotalPosition)*tickPrice;
            if(isFollow){
                LOG(INFO) << "follow orders judge......";
                if((holdInfo->shortTotalPosition - holdInfo->longTotalPosition) >= volMetric){
                    int closeVol = holdInfo->shortTotalPosition - holdInfo->longTotalPosition- volMetric;
                    InstrumentInfo* inst = getInstrumentInfo(instrumentID);
                    double tmpPrice = inst->UpperLimitPrice;
                    LOG(INFO) << "short pst=" + boost::lexical_cast<string>(holdInfo->shortTotalPosition) + ",long pst=" + boost::lexical_cast<string>(holdInfo->longTotalPosition) +
                                  ",over volMetric=" + boost::lexical_cast<string>(volMetric) + ",do follow!buy price=" + boost::lexical_cast<string>(tmpPrice) + ",volume=" + boost::lexical_cast<string>(closeVol);
                    addNewOrder(instrumentID,"0","0",tmpPrice,closeVol,"-1",NULL);
                }
            }
        }else if((holdInfo->longAvaClosePosition - holdInfo->shortAvaClosePosition) >= volSpread){
            volume = 2;
        }
    }else if(direction == "0"){//:#买入开仓
        orderInsertPrice = lastPrice - tickPrice;
        double beforePrice = orderInsertPrice;
        spreadTickPrice = tickPrice;
        if((holdInfo->longTotalPosition - holdInfo->shortTotalPosition)>=volSpread){
            orderInsertPrice = lastPrice - (holdInfo->longTotalPosition - holdInfo->shortTotalPosition)*tickPrice;
            LOG(INFO) << "买入时，多头=" + boost::lexical_cast<string>(holdInfo->longTotalPosition) + ",空头="+ boost::lexical_cast<string>(holdInfo->shortTotalPosition) + ",多头比空头多"+
                                      boost::lexical_cast<string>(holdInfo->longTotalPosition - holdInfo->shortTotalPosition) + "手，调整报单价格由" + boost::lexical_cast<string>(beforePrice) +"->" +
                                      boost::lexical_cast<string>(orderInsertPrice);
            spreadTickPrice = (holdInfo->longTotalPosition - holdInfo->shortTotalPosition)*tickPrice;
            if(isFollow){
                if((holdInfo->longTotalPosition - holdInfo->shortTotalPosition) >= volMetric){
                    int closeVol = holdInfo->longTotalPosition - holdInfo->shortTotalPosition- volMetric;
                    InstrumentInfo* inst = getInstrumentInfo(instrumentID);
                    double tmpPrice = inst->LowerLimitPrice;
                    LOG(INFO) << "long pst=" + boost::lexical_cast<string>(holdInfo->longTotalPosition) + ",short pst=" + boost::lexical_cast<string>(holdInfo->shortTotalPosition) +
                                  ",over volMetric=" + boost::lexical_cast<string>(volMetric) + ",do follow!sell price=" + boost::lexical_cast<string>(tmpPrice) + ",volume=" + boost::lexical_cast<string>(closeVol);
                    addNewOrder(instrumentID,"1","0",tmpPrice,closeVol,"-1",NULL);
                }
            }
        }else if((holdInfo->shortAvaClosePosition - holdInfo->longAvaClosePosition) >= volSpread){
            volume = 2;
        }
    }
    tmpInfo->orderInsertPrice = orderInsertPrice;
    tmpInfo->volume = volume;
    tmpInfo->spreadTickPrice = spreadTickPrice;
}
