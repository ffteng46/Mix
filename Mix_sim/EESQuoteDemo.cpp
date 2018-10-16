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

}

void QuoteDemo::ShowQuoteSim(EESMarketDepthQuoteData* pDepthMarketData){
    return;

    //mili-sec haomiao
    //mico-sec weimiao
    //nano-sec namiao
    currTime = string(tradingDay) + " " + boost::lexical_cast<string>(pDepthMarketData->UpdateTime)+" "+boost::lexical_cast<string>(pDepthMarketData->UpdateMillisec);
    if (start_process == 0) {
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
        marketdatainfo->highestPrice = pDepthMarketData->UpperLimitPrice;
        marketdatainfo->lowestPrice = pDepthMarketData->LowerLimitPrice;
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
        marketdatainfo->highestPrice = pDepthMarketData->UpperLimitPrice;
        marketdatainfo->lowestPrice = pDepthMarketData->LowerLimitPrice;
        if(multiply != 0&&tmpVolume != 0){
            //marketdatainfo->simPrice = tmpTurnover/tmpVolume;//multiply needed in shfe
            marketdatainfo->simPrice = tmpTurnover/tmpVolume/multiply;
        }
    }
    techCls.RunMarketData(marketdatainfo);

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
