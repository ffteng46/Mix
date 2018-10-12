#include "Strategy.h"
#include "calculate.h"
#include "property.h"
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include<map>
extern Strategy techCls;
extern calculate cal;					//先在主入口实例化，然后引用这个实例
extern bool testSwitch;
extern char tradingDay[12];
extern string currTime;
extern string tradingDayT;//2010-01-01
Strategy::Strategy()
{
    //memset(&tickData, 0, sizeof(CThostFtdcDepthMarketDataField));
    //QueryPerformanceFrequency(&m_nFreq);


    nSec = 0;
    ATR_window = 60;
    volatility = 0;
    kBarNum = 15;				//取15分钟K线？

    kPeriod = 60;
    mainDirection="3";

    INDEX_15s = -10;
    INDEX_30s = -10;
    INDEX_15m = -10;
    INDEX_1h = -10;

    kIndex_15s = -10;						//15 秒
    kIndex_30s = -10;;						//30 秒
    kIndex_15m = -10;;						//1分钟
    kIndex_1h = -10;;

    //trueKData15S = new Kdata();
    //trueKData15M = new Kdata();
    // 技术指标
    //ta_result = {0};
    //result_15 = {0};
    //result_1m = { 0 };
    //result_1h = {0};




}

Strategy::~Strategy()
{

}

void Strategy::RunMarketData(EESMarketDepthQuoteData *pDepthMarketData)
{
    string instrumentID=pDepthMarketData->InstrumentID;
    memcpy(&tickData, pDepthMarketData, sizeof(EESMarketDepthQuoteData));
    //QueryPerformanceCounter(&currentTime);
    //cout << "start cal !!!!!!!!!!!" << endl;

    int h1, m1, s1;
    sscanf(pDepthMarketData->UpdateTime, "%d:%d:%d", &h1, &m1, &s1);
    UpdateTime = h1 * 10000 + m1 * 100 + s1 * 1;
    //time_condition(UpdateTime,timecondition);
    LOG(INFO)<<"BEFORE:kIndex_15s="+boost::lexical_cast<string>(kIndex_15s)+",kIndex_15m="+boost::lexical_cast<string>(kIndex_15m);
    kIndex_15s = cal_Kindex(h1, m1, s1, techCls.periodSecOne);			//15秒k
    kIndex_15m = cal_Kindex(h1, m1, s1, 60*techCls.periodMinOne);		//15分钟
    if(kIndex_15s==-1||kIndex_15m==-1){
        LOG(ERROR)<<"ERROR:input time is error.!!!";
        return;
    }
    LOG(INFO)<<"AFTER:kIndex_15s="+boost::lexical_cast<string>(kIndex_15s)+",kIndex_15m="+boost::lexical_cast<string>(kIndex_15m);
    LOG(INFO)<<"BEFORE:INDEX_15s="+boost::lexical_cast<string>(INDEX_15s)+",INDEX_15m="+boost::lexical_cast<string>(INDEX_15m);
    //kIndex_180 = cal_Kindex(h1, m1, s1, 3600);		//1小时
    //LOG(INFO) << "kIndex_15m="+boost::lexical_cast<string>(kIndex_15m)
    //              +",INDEX_15m="+boost::lexical_cast<string>(INDEX_15m);
    //////////////////////all 15s k line////////////////////
    ///////////////
    LOG(INFO)<<"begin .15s k line.";
    if (kIndex_15s == INDEX_15s){
        update_kline(&tickData, allK15sList,false);
        Kdata* tmpk = &allK15sList[allK15sList.size() - 1];
        genKLine_15S=false;
        string msg="businessType=wtm_5001;logTime="+currTime + ";ma5="+boost::lexical_cast<string>(tmpk->ma5)+";"
                + "ma10="+boost::lexical_cast<string>(tmpk->ma10)+";"
                + "ma20="+boost::lexical_cast<string>(tmpk->ma20)+";"
                + "tr="+boost::lexical_cast<string>(tmpk->TR)+";"
                + "atr="+boost::lexical_cast<string>(tmpk->ATR)+";"
                + "macd_diff="+boost::lexical_cast<string>(tmpk->macd_diff)+";"
                + "macd_dea="+boost::lexical_cast<string>(tmpk->macd_dea)+";"
                + "closePrice="+boost::lexical_cast<string>(tmpk->closePrice)+";"
                + "openPrice="+boost::lexical_cast<string>(tmpk->openPrice)+";"
                + "highPrice="+boost::lexical_cast<string>(tmpk->highPrice)+";"
                + "lowPrice="+boost::lexical_cast<string>(tmpk->lowPrice)+";"
                + "tradingDay="+tradingDay+";"
                + "kIndex="+boost::lexical_cast<string>(kIndex_15s)+";"
                + "index="+boost::lexical_cast<string>(INDEX_15s)+";"
                + "updateTime="+boost::lexical_cast<string>(pDepthMarketData->UpdateTime)+";"
                + "timeType=second;"
                + "opType=u;"
                + "second=15";
        sendMSG(msg);
    }else if (kIndex_15s != INDEX_15s){
        genKLine_15S=true;//a new k line
        INDEX_15s = kIndex_15s;
        creat_Kline(&tickData, kIndex_15s, allK15sList);
        run_tech_lib(allK15sList);

        if(allK15sList.size()>=2){
            vector<Kdata >::iterator it=allK15sList.end()-2;
            trueKData15S=&(*it);
            KData_15s.push_back(allK15sList[allK15sList.size()-1]);
            if(!beginK15s){
                doKcleanS(KData_15s);
                //Kdata tmp=techCls.KData_15s.back();
                //techCls.KData_15s.clear();
                //techCls.KData_15s.push_back(tmp);
            }
            string msg="businessType=wtm_5001;logTime="+currTime + ";ma5="+boost::lexical_cast<string>(trueKData15S->ma5)+";"
                    + "ma10="+boost::lexical_cast<string>(trueKData15S->ma10)+";"
                    + "ma20="+boost::lexical_cast<string>(trueKData15S->ma20)+";"
                    + "tr="+boost::lexical_cast<string>(trueKData15S->TR)+";"
                    + "atr="+boost::lexical_cast<string>(trueKData15S->ATR)+";"
                    + "macd_diff="+boost::lexical_cast<string>(trueKData15S->macd_diff)+";"
                    + "macd_dea="+boost::lexical_cast<string>(trueKData15S->macd_dea)+";"
                    + "closePrice="+boost::lexical_cast<string>(trueKData15S->closePrice)+";"
                    + "openPrice="+boost::lexical_cast<string>(trueKData15S->openPrice)+";"
                    + "highPrice="+boost::lexical_cast<string>(trueKData15S->highPrice)+";"
                    + "lowPrice="+boost::lexical_cast<string>(trueKData15S->lowPrice)+";"
                    + "tradingDay="+tradingDay+";"
                    + "kIndex="+boost::lexical_cast<string>(kIndex_15s)+";"
                    + "index="+boost::lexical_cast<string>(INDEX_15s)+";"
                    + "updateTime="+boost::lexical_cast<string>(pDepthMarketData->UpdateTime)+";"
                    + "timeType=second;"
                    + "opType=c;"
                    + "second=15";
            sendMSG(msg);
        }
    }/*
    if(beginK15s){
        LOG(INFO)<<"begin .15s k line.";
        if (kIndex_15s == INDEX_15s){
            update_kline(&tickData, KData_15s,false);
            genKLine_15S=false;
        }else if (kIndex_15s != INDEX_15s){
            genKLine_15S=true;//a new k line
            INDEX_15s = kIndex_15s;
            creat_Kline(&tickData, kIndex_15s, KData_15s);
            if(KData_15s.size()>=2){
                vector<Kdata >::iterator it=KData_15s.end()-2;
                trueKData15S=&(*it);
                if(!isK15sFirstItem){
                    LOG(INFO) << "this is first k 15 line for strongly signal!";
                    isK15sFirstItem=true;//should be reset
                }else{
                    LOG(INFO) << "the first k 15 line for strongly signal has been set!";
                }
            }
        }
    }*/
    if (kIndex_15m == INDEX_15m)
    {
        update_kline(&tickData, KData_15m,true);
        genKLine_15m=false;
        Kdata* tmpk = &KData_15m[KData_15m.size() - 1];
        string msg="businessType=wtm_5001;logTime="+currTime + ";ma5="+boost::lexical_cast<string>(tmpk->ma5)+";"
                + "ma10="+boost::lexical_cast<string>(tmpk->ma10)+";"
                + "ma20="+boost::lexical_cast<string>(tmpk->ma20)+";"
                + "tr="+boost::lexical_cast<string>(tmpk->TR)+";"
                + "atr="+boost::lexical_cast<string>(tmpk->ATR)+";"
                + "macd_diff="+boost::lexical_cast<string>(tmpk->macd_diff)+";"
                + "macd_dea="+boost::lexical_cast<string>(tmpk->macd_dea)+";"
                + "closePrice="+boost::lexical_cast<string>(tmpk->closePrice)+";"
                + "openPrice="+boost::lexical_cast<string>(tmpk->openPrice)+";"
                + "highPrice="+boost::lexical_cast<string>(tmpk->highPrice)+";"
                + "lowPrice="+boost::lexical_cast<string>(tmpk->lowPrice)+";"
                + "tradingDay="+tradingDay+";"
                + "kIndex="+boost::lexical_cast<string>(kIndex_15m)+";"
                + "index="+boost::lexical_cast<string>(INDEX_15m)+";"
                + "updateTime="+boost::lexical_cast<string>(pDepthMarketData->UpdateTime)+";"
                + "timeType=minute;"
                + "opType=u;"
                + "minute=15";
        sendMSG(msg);
        //LOG(INFO)<<msg;
    }else if (kIndex_15m != INDEX_15m){
        genKLine_15m=true;
        INDEX_15m = kIndex_15m;
        creat_Kline(&tickData, kIndex_15m, KData_15m);
        run_tech_lib(KData_15m);
        newestData15M=&(KData_15m[KData_15m.size()-1]);
        if(KData_15m.size()>=2){
            LOG(INFO) << "this is just k 15m line!";
            vector<Kdata >::iterator it=KData_15m.end()-2;
            trueKData15M=&(*it);
            string msg="businessType=wtm_5001;logTime="+currTime + ";ma5="+boost::lexical_cast<string>(trueKData15M->ma5)+";"
                    + "ma10="+boost::lexical_cast<string>(trueKData15M->ma10)+";"
                    + "ma20="+boost::lexical_cast<string>(trueKData15M->ma20)+";"
                    + "tr="+boost::lexical_cast<string>(trueKData15S->TR)+";"
                    + "atr="+boost::lexical_cast<string>(trueKData15S->ATR)+";"
                    + "macd_diff="+boost::lexical_cast<string>(trueKData15M->macd_diff)+";"
                    + "macd_dea="+boost::lexical_cast<string>(trueKData15M->macd_dea)+";"
                    + "closePrice="+boost::lexical_cast<string>(trueKData15M->closePrice)+";"
                    + "openPrice="+boost::lexical_cast<string>(trueKData15M->openPrice)+";"
                    + "highPrice="+boost::lexical_cast<string>(trueKData15M->highPrice)+";"
                    + "lowPrice="+boost::lexical_cast<string>(trueKData15M->lowPrice)+";"
                    + "tradingDay="+tradingDay+";"
                    + "kIndex="+boost::lexical_cast<string>(kIndex_15m)+";"
                    + "index="+boost::lexical_cast<string>(INDEX_15m)+";"
                    + "updateTime="+boost::lexical_cast<string>(pDepthMarketData->UpdateTime)+";"
                    + "timeType=minute;"
                    + "opType=c;"
                    + "minute=15";
            sendMSG(msg);
            LOG(INFO)<<msg;
        }
        //write_record(KData_15m, UpdateTime, kdata15min);
    }
    //mainDirection="0";
    LOG(INFO)<<"AFTER:INDEX_15s="+boost::lexical_cast<string>(INDEX_15s)+",INDEX_15m="+boost::lexical_cast<string>(INDEX_15m);
    if(techCls.isTestInviron){
        mainDirection="0";
        beginK15s=true;
        return;
    }

    if(genKLine_15m&&trueKData15M){
        string msg="businessType=wtm_6001;tradingDay="+string(tradingDay)+";logTime="+currTime + ";logType=1;log=diff="+boost::lexical_cast<string>(trueKData15M->macd_diff) +","
                +"dea="+boost::lexical_cast<string>(trueKData15M->macd_dea) +","
                +"ma5="+boost::lexical_cast<string>(trueKData15M->ma5) +","
                +"ma10="+boost::lexical_cast<string>(trueKData15M->ma10) +","
                +"ma20="+boost::lexical_cast<string>(trueKData15M->ma20);
        LOG(INFO) << msg;
        sendMSG(msg);
        if(trueKData15M->macd_dea==0||trueKData15M->ma20==0||trueKData15M->ma10==0){
            return;
        }
        if(trueKData15M->macd_diff>trueKData15M->macd_dea
                &&trueKData15M->ma5>trueKData15M->ma20
                &&trueKData15M->ma10>trueKData15M->ma20){//main direction is long
            msg="businessType=wtm_6001;tradingDay="+string(tradingDay)+";logTime="+currTime + ";"+"logType=1;log=match long strongly condition!current main direction="+mainDirection;
            LOG(INFO) << msg;
            sendMSG(msg);
            if(mainDirection=="02"){//strong->watch->strong
                Strategy::Kdata* lastTwoK;
                if(techCls.priceStatus=="10"||techCls.priceStatus=="11"){
                    msg="businessType=wtm_6001;tradingDay="+string(tradingDay)+";logTime="+currTime + ";logType=1;log=In last watch status,not find direction.when main direction is set,reset parameter.";
                    LOG(INFO)<<msg;
                    sendMSG(msg);
                    coverYourAss();
                }
                if(KData_15m.size()>=3){
                    vector<Strategy::Kdata >::iterator it=KData_15m.end()-3;
                    lastTwoK=&(*it);
                    if(trueKData15M->closePrice > lastTwoK->closePrice){//Kn+1>Kn,resume to long
                        mainDirection="0";
                        msg="businessType=wtm_6001;tradingDay="+string(tradingDay)+";logTime="+currTime + ";logType=1;log=before mainDirection =02,this k's close price="+boost::lexical_cast<string>(trueKData15M->closePrice)
                                +" is bigger than last k's close price="+boost::lexical_cast<string>(lastTwoK->closePrice)
                                +",change maindir to 0!";
                        LOG(INFO) <<msg;
                        sendMSG(msg);
                    }else{
                        msg="businessType=wtm_6001;tradingDay="+string(tradingDay)+";logTime="+currTime + ";logType=1;log=before mainDirection =02,this k's close price="+boost::lexical_cast<string>(trueKData15M->closePrice)
                                +" is not bigger than last k's close price="+boost::lexical_cast<string>(lastTwoK->closePrice)
                                //+",remain watching!";
                               +",reset direction to 3!";
                        LOG(INFO) << msg;
                        sendMSG(msg);
                        mainDirection="3";
                        resetK15sData();
                    }
                }else{
                    msg="businessType=wtm_6001;tradingDay="+string(tradingDay)+";logTime="+currTime + ";logType=1;log=15m k line is less than 2,don't process! size="+boost::lexical_cast<string>(KData_15m.size());
                    LOG(INFO) << msg;
                    sendMSG(msg);
                }

            }else if(mainDirection=="3"){
                mainDirection="0";
                beginK15s=true;
                doKcleanS(KData_15s);
                //KData_15s.clear();
                msg="businessType=wtm_6001;tradingDay="+string(tradingDay)+";logTime="+currTime + ";logType=1;log=change main direciton from 3 to 0.begin to compute 15s k line,current 15s k line size is "
                        +boost::lexical_cast<string>(KData_15s.size());
                LOG(INFO) <<msg;
                sendMSG(msg);
            }else{
                if(mainDirection == "12"){
                    msg="businessType=wtm_6001;tradingDay="+string(tradingDay)+";logTime="+currTime + ";logType=1;log=direction change from 12 to 0 directly.should close all position,and start from 0 point?";
                    LOG(INFO)<<msg;
                    sendMSG(msg);
                    mainDirection="0";
                    beginK15s=true;
                    doKcleanS(KData_15s);
                    //KData_15s.clear();
                    msg="businessType=wtm_6001;tradingDay="+string(tradingDay)+";logTime="+currTime + ";logType=1;log=change main direciton from 12 to 0;begin to compute 15s k line,current 15s k line size is "
                            +boost::lexical_cast<string>(KData_15s.size());
                    LOG(INFO) << msg;
                    sendMSG(msg);
                }else{
                    msg="businessType=wtm_6001;tradingDay="+string(tradingDay)+";logTime="+currTime + ";logType=1;log=change main direciton from "+mainDirection+"to 0!";
                    LOG(INFO) << msg;
                    sendMSG(msg);
                    mainDirection="0";
                }

            }
        }else if(trueKData15M->macd_diff < trueKData15M->macd_dea
                 &&trueKData15M->ma5 < trueKData15M->ma20
                 &&trueKData15M->ma10 < trueKData15M->ma20){
            LOG(INFO) << "match short strongly condition!"+msg;
            return;
            if(mainDirection=="12"){//strong->watch->strong
                Strategy::Kdata* lastTwoK;
                if(techCls.priceStatus=="10"||techCls.priceStatus=="11"){
                    LOG(INFO)<<"In last watch status,not find direction.when main direction is set,reset parameter.";
                    coverYourAss();
                }
                if(KData_15m.size()>=3){
                    vector<Strategy::Kdata >::iterator it=KData_15m.end()-3;
                    lastTwoK=&(*it);
                    if(trueKData15M->closePrice < lastTwoK->closePrice){//Kn+1<Kn,resume to short
                        mainDirection="1";

                        LOG(INFO) << "before mainDirection =12,this k's close price="+boost::lexical_cast<string>(trueKData15M->closePrice)
                                      +" is smaller than last k's close price="+boost::lexical_cast<string>(lastTwoK->closePrice)
                                      +",change maindir to "+mainDirection+"!";
                    }else{
                        LOG(INFO) << "before mainDirection =12,this k's close price="+boost::lexical_cast<string>(trueKData15M->closePrice)
                                      +" is not smaller than last k's close price="+boost::lexical_cast<string>(lastTwoK->closePrice)
                                     //+",remain watching!";
                                    +",reset direction to 3!";
                       mainDirection="3";
                       resetK15sData();
                    }
                }else{
                    LOG(INFO) << "15m k line is less than 2,don't process! size="+boost::lexical_cast<string>(KData_15m.size());
                }

            }else if(mainDirection=="3"){
                mainDirection="1";
                beginK15s=true;
                doKcleanS(KData_15s);
                //KData_15s.clear();
                LOG(INFO) << "change main direciton from 3 to 1;begin to compute 15s k line,current 15s k line size is "
                              +boost::lexical_cast<string>(KData_15s.size());
            }else{
                LOG(INFO) << "change main direciton from "+mainDirection+"to 1!";
                mainDirection="1";
            }
        }else{//not match strongly condition
            LOG(INFO) << "not match strongly condition!"+msg;
            if(mainDirection=="3"){
                LOG(INFO) << "remain mainDirection="+mainDirection;
            }else if(mainDirection=="02"||mainDirection=="12"){
                LOG(INFO) << "last 15m k is at watching status,this line not match strongly condition,reset to default.";
                mainDirection = "3";
                if(stgStatus=="11"){
                    LOG(INFO)<<"In watching status,don't find direction by dual trust.stgStatus reset to 0.";
                    stgStatus="0";
                }
                resetK15sData();
            }else if(mainDirection=="0"){
                mainDirection="02";
                techCls.priceStatus="0";
                techCls.stgStatus="10";
                LOG(INFO) << "change main dir from 0 to "+mainDirection+",begin to watch.stgStatus set to "+techCls.stgStatus;

                //all untrade action
                LOG(INFO)<<"just watching status,all untrade order will be action.";
                tryAllOrderAction(instrumentID);
                //all trade close when next marketdata.
                //tmpLongReverseList.clear();
                //longReverseList.clear();
                //transfer all order in WaitForCloseInfo to longReverseList
                /*
                if(allTradeList.size() > 0){
                    for(list<WaitForCloseInfo*>::iterator wfIt = allTradeList.begin();wfIt!=allTradeList.end();){
                        //tmpLongReverseList->push_back(*wfIt);
                        longReverseList.push_back(*wfIt);
                        wfIt = allTradeList.erase(wfIt);
                    }
                }
                //all untrade action

                //all trade close
                double tmpMax=max(abs(trueKData15M->highPrice - trueKData15M->closePrice),abs(trueKData15M->closePrice - trueKData15M->lowPrice));
                techCls.limit[0]=trueKData15M->closePrice+0.7*tmpMax;
                techCls.limit[1]=trueKData15M->closePrice-1*tmpMax;
                techCls.priceStatus="0";
                techCls.stgStatus="10";
                techCls.minPrice=0;
                techCls.maxPrice=0;
                techCls.firstOpenKLineType="0";
                techCls.KData_15s.clear();*/
            }else if(mainDirection=="1"){
                mainDirection="12";
                LOG(INFO) << "change main dir from 1 to "+mainDirection+",begin to watch.";
            }
        }
        string tmpmsg="businessType=wtm_1003;tradingDay="+string(tradingDay)+";logTime="+currTime + ";mainDirection="+techCls.mainDirection;
        sendMSG(tmpmsg);
    }
}

int Strategy::cal_Kindex(int h1, int m1, int s1, int black_window)
{
    /*int send_nums;
    send_nums = 0;*/


    if (h1 >= 21 && h1 <= 24)
    {
        nSec = (h1 - 21) * 3600 + m1 * 60 + s1+ preTotalSeconds;

    }
    else if (h1 >= 0 && h1 <= 2)
    {
        nSec = 10800 + h1 * 3600 + m1 * 60 + s1+ preTotalSeconds;
        //UpdateTime = (h1==0) ?(): h1 * 10000 + m1 * 100 + s1 * 1 ;
    }
    else if (h1 >= 9 && h1<=15)
    {
        //夜盘和白天的收盘价K线连续
        /*if (nightendtime != 0)
        {
        if (h1 == 9 || (h1 == 10 && m1 <= 15))
        {
        nSec = (h1 - 9) * 3600 + m1 * 60 + s1 + totalNightSeconds;
        }
        else if ((h1 == 10 && m1 >= 30) || h1 == 11)
        {
        nSec = (h1 - 9) * 3600 + m1 * 60 + s1 - 900 + totalNightSeconds;
        }
        else if (h1 >= 13 && h1 < 15)
        {
        nSec = (h1 - 9) * 3600 + m1 * 60 + s1 - 900 - 2 * 3600 + totalNightSeconds;
        }
        }*/

        if (h1 == 9 || (h1 == 10 && m1 <= 15))
        {
            nSec = (h1 - 9) * 3600 + m1 * 60 + s1 + preTotalSeconds + tonightSecs;
        }
        else if ((h1 == 10 && m1 >= 30) || h1 == 11)
        {
            nSec = (h1 - 9) * 3600 + m1 * 60 + s1 - 900 + preTotalSeconds + tonightSecs;
        }
        else if (h1 >= 13 && h1 <= 15)
        {
            nSec = (h1 - 9) * 3600 + m1 * 60 + s1 - 900 - 2 * 3600 +preTotalSeconds + tonightSecs;
        }

    }

    else
    {
        return -1;
    }

    int tem_index{ 0 };
    tem_index = nSec / black_window;

    return tem_index;

}
void Strategy::setPreTotalSecs(int totalDates){
    int totalSeconds=totalDates*(2+1.5+2.5-0.25)*3600;
    LOG(INFO)<<"totalSeconds="+boost::lexical_cast<string>(totalSeconds);
    cout<<"totalSeconds="+boost::lexical_cast<string>(totalSeconds);
    preTotalSeconds=totalSeconds;
}

void Strategy::creat_Kline(EESMarketDepthQuoteData* tickData, int current_index, vector<Kdata > &vectorKData)
{	///////////////////////////////////////////////////////////////////////////////////////

    //新产生的K线creat_Kline
    Kdata newKata;
    newKata.updatetimes = UpdateTime;
    newKata.openPrice = tickData->LastPrice;
    newKata.closePrice = tickData->LastPrice;
    newKata.highPrice = tickData->LastPrice;
    newKata.lowPrice = tickData->LastPrice;
    newKata.volume = tickData->Volume;					//累积成交量
    //newKata.atr = 0;
    newKata.nIndex = current_index;
    //技术指标
    newKata.macd_diff= 0;
    newKata.macd_dea = 0;
    newKata.macd_sign = 0;
    newKata.OBV = 0;
    newKata.TR=0;
    newKata.ATR=0;
    newKata.ma5=0;
    newKata.ma10=0;
    newKata.ma20=0;

    vectorKData.push_back(newKata);


            //输出K线
            //if (KData_15.size() >= 2)
            //{
                //kdatalog << updatetime << "," << LastPrice << "," << KData_15[KData_15.size() - 2].openPrice << "," << vectorKData[vectorKData.size() - 2].highPrice << ","
                //	<< vectorKData[vectorKData.size() - 2].lowPrice << "," << vectorKData[vectorKData.size() - 2].closePrice << "," << vectorKData[vectorKData.size() - 2].nIndex << "," << endl;
                //<< current_macd << "," << results_RSI << "," << stok << "," << stod << "," << bulling << "," << tem_std << endl;
            //}
            //cout << "计算新K" << endl;
    //cout << "初始化K线  : " << current_index << "; " << vectorKData.size() << endl;
    //当前K线没走完，但是只要更新了，就会影响具体的技术指标
}


void Strategy::update_kline(EESMarketDepthQuoteData* tickData, vector<Kdata > &vectorKData,bool is_15min)
{
    if (vectorKData.size()<1)
    {
        cout << "请先初始化K线" << endl;

    }

    else
    {
        vectorKData[vectorKData.size() - 1].closePrice = tickData->LastPrice;
        if (tickData->LastPrice> vectorKData[vectorKData.size() - 1].highPrice)
        {
            vectorKData[vectorKData.size() - 1].highPrice = tickData->LastPrice;
        }
        if (tickData->LastPrice < vectorKData[vectorKData.size() - 1].lowPrice)
        {
            vectorKData[vectorKData.size() - 1].lowPrice = tickData->LastPrice;
        }
    }
    if (is_15min)
    {
        int curr_sec{ 0 };
        curr_sec = UpdateTime - INDEX_15m * 60;
        if (false&&curr_sec>12 * 60)
        {
            string msg="alarm:3 minites before 15m k line.";
            LOG(ERROR)<<msg;
            vectorKData[vectorKData.size() - 1].min3_last = true;
            OrderInfo* orderInfo;
            if(existUntradeOrder("3000",orderInfo)){
                if(orderInfo->status=="0"){
                    //boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
                    LOG(INFO) << "There are untrade order,and not action.So we will execute order action.";
                    tryOrderAction(orderInfo->instrumentID,orderInfo,"1");
                    orderInfo->status="1";
                }
            }
        }
    }
        //vectorKData[vectorKData.size() - 1].atr = vectorKData[vectorKData.size() - 1].highPrice - vectorKData[vectorKData.size() - 1].lowPrice;

}

void Strategy::run_tech_lib(vector<Kdata > &vectorKData)
{
    //##########################计算MACD值
    cal.MACD(vectorKData, fast, middle, slows);

    //##########################计算RSI值    result[3]
    //results_RSI = cal.RSI(vectorKData, RSI_window);
    //cout << "RSI results: " << results_RSI << endl;



    //##########################计算kdj值
    //stok = cal.KDJ(vectorKData, kdj_window);
    //stoks.push_back(stok);
    /*if (stoks.size()> kdj_window)
    {
        stod = cal.double_ema(vectorKData, kdj_window);
    }*/
    //cout << "KDJ results: " << stok << " #  " << stod << endl;


    //##########################计算布林带
    //cal.Bulling(vectorKData, bulling_window,bulling_set);


    //
    cal.ATR(vectorKData, ATR_window);
    cal.MA(vectorKData);
    /*
    cal.OBV(vectorKData);
    cal.MA(vectorKData);

    cal.dural_trust(vectorKData,ks);
    */
    cout << "MACD diff: " << vectorKData[vectorKData.size() - 2].macd_diff << " ;; dea: " << vectorKData[vectorKData.size() - 2].macd_dea << " :;sign: " << vectorKData[vectorKData.size() - 2].macd_sign
        << " uprange " << vectorKData[vectorKData.size() - 1].up_range << "  down_range  " << vectorKData[vectorKData.size() - 1].down_range << endl;

}
/*
void Strategy::update_listcontrol(int updatetime)
{
    if (updatetime %5 ==0)
    {
        for (int i = 0; i < pDlg->dlg_list_tasklist.GetItemCount(); i++)
        {
            CString str2 = pDlg->dlg_list_tasklist.GetItemText(i, 0);

            int taskid = _ttoi(str2);

            for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
            {
                if ((*it)->nTask == taskid)
                {
                    CString str;
                    int nRow = i;


                    str.Format(_T("%.2f"), tickData.BidPrice1);							//  pDepthMarketData->BidPrice1
                    pDlg->dlg_list_tasklist.SetItemText(nRow, 2, str);


                    //str.Format(_T("%.2f"), bench_lin);							//  pDepthMarketData->BidPrice1
                    //pDlg->dlg_list_tasklist.SetItemText(nRow, 3, str);

                    //str.Format(_T("%.2f"), tickPrice*grid_num);							//  pDepthMarketData->BidPrice1
                    //pDlg->dlg_list_tasklist.SetItemText(nRow, 4, str);

                    break;

                }
            }
        }

    }
}*/

