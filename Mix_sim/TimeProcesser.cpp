
#include "TimeProcesser.h"
#include <string>
#include <unordered_map>
#include "property.h"
#include <glog/logging.h>
using namespace std;
extern CTechMetric ctm;
extern bool isLogout;
extern string systemID;//系统编号，每个产品一个编号
extern unordered_map<string, vector<string>> instr_map;				//一个合约和哪些合约配对
extern char tradingDay[12];
extern string tradingDayT;//2010-01-01
extern unordered_map<string, MarketData*> marketdata_map;
extern unordered_map<string, TechMetric*> techMetricMap;
extern boost::lockfree::queue<MarketData*> techMetricQueue;///技术指标处理列表
extern boost::lockfree::queue<PriceGapMarketData*> detectOpenQueue;///开仓列表
extern boost::lockfree::queue<PriceGapMarketData*> detectCloseQueue;///止盈列表
extern boost::lockfree::queue<PriceGapMarketData*> strategyQueue;///处理列表
extern boost::recursive_mutex techMetric_mtx;//技术指标
extern boost::recursive_mutex mkdata_mtx; //行情锁
extern boost::lockfree::queue<LogMsg*> networkTradeQueue;////报单、成交消息队列,网络通讯使用
extern ATRInfoClass* atrinfo;

extern Strategy techCls;
extern list<WaitForCloseInfo*> protectList;//protect order list
extern list<WaitForCloseInfo*> allTradeList;//before one normal
extern list<WaitForCloseInfo*> longReverseList;//before one normal
extern list<WaitForCloseInfo*> tmpLongReverseList;//before one normal
extern HoldPositionInfo userHoldPst;//not real hold position info
extern unordered_map<string, HoldPositionInfo*> reversePosition;
extern SpecOrderField* sof;
extern OrderInfo orderInfo;
extern char tradingDay[12];
extern string tradingDayT;//2010-01-01
extern string currTime;//=tradingDayT + updateTime
extern boost::recursive_mutex unique_mtx;//unique lock
bool isInTimePeriod(boost::posix_time::time_period* pTimePeriod){
    boost::posix_time::ptime currtime = getCurrentTimeByBoost();
    // 判断当前时间是否在规定的时间区间内;
    if (pTimePeriod->contains(currtime))  {
        // 当前时间在规定的时间区间内;
        // do something;
        LOG(INFO) << "TIME IN PERIOD";
        cout << "TIME IN PERIOD" <<currtime<<endl;
        return true;
    }  else if (pTimePeriod->is_after(currtime))  {
        // 当前时间早于8点50分;
        // do something;
        LOG(INFO) << "TIME BEFORE";
        cout << "TIME IN BEFORE"<<currtime <<endl;
        return true;
    }  else if (pTimePeriod->is_before(currtime))  {
        // 当前时间晚于9点15分;
        // do something;
        LOG(INFO) << "TIME after";
        cout << "TIME IN after"<<currtime <<endl;
        return true;
    }
}


boost::posix_time::ptime getCurrentTimeByBoost() {
    boost::posix_time::ptime time_now;
    // 这里为微秒为单位;这里可以将microsec_clock替换成second_clock以秒为单位;
    time_now = boost::posix_time::microsec_clock::universal_time();
    return time_now;
}

int getTimeInterval(boost::posix_time::ptime timeBegin, boost::posix_time::ptime timeEnd, string timeType) {
    boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse;
    //时间间隔
    time_elapse = timeEnd - timeBegin;
    if (timeType == "t") {
        // 类似GetTickCount，只是这边得到的是2个时间的ticket值的差，以微秒为单位;
        int ticks = time_elapse.ticks();
        return ticks;
    } else if (timeType == "s") {
        // 得到两个时间间隔的秒数;
        int sec = time_elapse.total_seconds();
        return sec;
    }
}
boost::posix_time::ptime changeStringToTime(string strTime) {
    //把字符串转换为boost 时间类对象 :
    //ptime p1 = from_iso_string("20121101T202020");
    //return  boost::posix_time::time_from_string("2012-3-5 01:00:00");
    return  boost::posix_time::time_from_string(strTime);
}
string getDes(PriceGapMarketData* pg) {
    string str;
    str += "lastIns=" + pg->lastInstrumentID + ";lastBid=" + boost::lexical_cast<string>(pg->lastInsBidPrice) + ";lastAsk=" + boost::lexical_cast<string>(pg->lastInsAskPrice) + ";lastVolume=" + boost::lexical_cast<string>(pg->lastInsVolume) +
        ";fwdIns=" + pg->forwardInstrumentID + ";fwdBid=" + boost::lexical_cast<string>(pg->forwardInsBidPrice) + ";fwdAsk=" + boost::lexical_cast<string>(pg->forwardInsAskPrice) + ";fwdVolume=" + boost::lexical_cast<string>(pg->forwardInsVolume);
    return str;
}
void metricProcesser() {


}
void longDirectionTrade(MarketData *mkData){
    double lastPrice = mkData->lastPrice;
    string instrumentID = mkData->instrumentID;
    double tickPrice = getPriceTick(instrumentID);
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
                    computeDualTrustPara(mkData->lastPrice);
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
                    sof->instrumentID=instrumentID;
                    sof->direction="1";
                    sof->offsetFlag="1";
                    sof->lastPrice=lastPrice;
                    sof->volume=userHoldPst.longTotalPosition;
                    sof->orderType="0";
                    sof->openStgType="2011";
                    doSpecOrder(sof);
                    //OrderInfo orderInfo;
                    /*
                    if(existUntradeOrder("2011",&orderInfo)){
                        LOG(INFO)<<"LastPirce ="+boost::lexical_cast<string>(lastPrice)+" trigger stopLossPrice="
                                   +boost::lexical_cast<string>(techCls.stopLossPrice)+",there are untraded order,not process.";
                    }else{
                        LOG(INFO)<<"LastPirce ="+boost::lexical_cast<string>(lastPrice)+" trigger stopLossPrice="
                                   +boost::lexical_cast<string>(techCls.stopLossPrice)+",there are not untraded order,add new order.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2011";
                        addNewOrderTrade(instrumentID,"1","1",lastPrice,userHoldPst.longTotalPosition,"0",addinfo);
                    }*/
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
                    LOG(INFO)<<"step into range of one normal.firstOpenPriceNormal="+boost::lexical_cast<string>(techCls.firstOpenPriceNormal);//until here,below how
                    WaitForCloseInfo* wfc_lastOpen=allTradeList.back();//the last order
                    if(techCls.firstOpenPriceNormal ==0 && lastPrice < wfc_lastOpen->openPrice){//not step into one normal before
                        if(existUntradeOrder("2042",NULL)){
                            LOG(INFO) << "There are untrade order,not process.";
                        }else{
                            LOG(INFO) << "There are no untrade order exist,add new order.";
                            AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                            addinfo->openStgType="2042";
                            addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.oneNormalVolume,"0",addinfo);
                        }
                    }else{
                        string tmpsts=techCls.priceStatus;
                        techCls.priceStatus="4";
                        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus;
                        int tmplist=allTradeList.size();
                        longReverseList.clear();
                        //transfer all order in WaitForCloseInfo to longReverseList
                        for(list<WaitForCloseInfo*>::iterator wfIt = allTradeList.begin();wfIt!=allTradeList.end();){
                            //tmpLongReverseList->push_back(*wfIt);
                            longReverseList.push_back(*wfIt);
                            wfIt = allTradeList.erase(wfIt);
                        }
                        LOG(INFO)<<"When step into one normal range again,all order traded before will be moved to reverselist.before allTradeList="+boost::lexical_cast<string>(tmplist)
                                   +",after allTradeList="+boost::lexical_cast<string>(allTradeList.size())
                                   +",longReserseList="+boost::lexical_cast<string>(longReverseList.size());
                    }

                }else{
                    LOG(INFO)<<"Where am i?lastPrice="+boost::lexical_cast<string>(lastPrice);
                }
            }else if(techCls.priceStatus=="4"){//one normal
                //if(stopProfit("0",lastPrice,instrumentID)){//
                if(false){//
                    LOG(INFO)<<"long reverse is stopping profit,not process.";
                }else{
                    //stop profit order not all traded,then first order action.must.
                    //cancelSpecTypeOrder(instrumentID,"2001");
                    WaitForCloseInfo* wfc_firstOpen;//the first order
                    WaitForCloseInfo* wfc_lastOpen;//the last order
                    WaitForCloseInfo* wfc_lastSecondOpen;//the last second order
                    if(tmpLongReverseList.size()>0){
                        list<WaitForCloseInfo*>::iterator atIT=tmpLongReverseList.begin();
                        wfc_firstOpen=*atIT;
                    }
                    wfc_lastOpen=tmpLongReverseList.back();
                    /*
                    if(lastPrice >= wfc_lastOpen->openPrice){
                        LOG(INFO)<<"although in one normal state,but will not insert order,so if there are stop profit orders not traded,not care.";
                    }else{
                        //stop profit order not all traded,then first order action
                        cancelSpecTypeOrder(instrumentID,"2001");
                    }*/
                    //price will jump to sweet
                    double simFirstOpenPrice=techCls.firstOpenPrice - techCls.firstToSweetTickNums*tickPrice;
                    if(lastPrice >= (simFirstOpenPrice-techCls.oneSweetGrade*techCls.oneSweetGap*tickPrice - techCls.sweetToNormalTickNums*tickPrice)){
                        LOG(INFO)<<"step into range of one sweet.cancell all one normal orders.";
                        cancelSpecTypeOrder(instrumentID,"2042");
                        string tmpsts = techCls.priceStatus;
                        techCls.priceStatus = "3";
                        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus+",return to status of one sweet.";


                        //transfer all order in longReverseList to allTradeList
                        int tmplist=longReverseList.size();
                        allTradeList.clear();
                        for(list<WaitForCloseInfo*>::iterator wfIt = longReverseList.begin();wfIt!=longReverseList.end();){
                            allTradeList.push_back(*wfIt);
                            wfIt = longReverseList.erase(wfIt);
                        }
                        LOG(INFO)<<"When from one normal to sweet,all order traded before will be moved to allTradeList.before longReverseList="+boost::lexical_cast<string>(tmplist)
                                   +",after longReverseList="+boost::lexical_cast<string>(longReverseList.size())
                                   +",allTradeList="+boost::lexical_cast<string>(allTradeList.size());
                    }else if(tmpLongReverseList.size() < techCls.oneNormalGrade){
                        LOG(INFO)<<"long逆向加仓处于第一阈值内,each grade加仓量="+boost::lexical_cast<string>(techCls.oneNormalVolume);
                        if(wfc_firstOpen&&int((wfc_firstOpen->openPrice-lastPrice)/tickPrice)%techCls.oneNormalGap==0){
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
                        LOG(INFO)<<"Current grade for two is "+boost::lexical_cast<string>(existGrades)
                                   +",and the limit grade is "+boost::lexical_cast<string>(techCls.twoGrade);
                        setStageTick("5",existGrades);
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
                        LOG(INFO)<<"price in one normal,not touch any trigger,not process.";
                    }
                }
            }else if(techCls.priceStatus=="5"){//tow status
                LOG(INFO)<<"Price step into two status:Fibonacci.";
                //judge if we will stop profit.at the last tow grade,can insert limit close order.
                if(techCls.stageTick == 1){
                    stopProfit("0",lastPrice,instrumentID);
                }
                //if(stopProfit("0",lastPrice,instrumentID)){//
                if(stopLoss("0",lastPrice,mkData->bidPrice,mkData->askPrice,instrumentID)){
                    LOG(INFO)<<"long reverse is stopping profit,not process.";
                }else{
                    WaitForCloseInfo* wfc_firstOpen;//the first order
                    WaitForCloseInfo* wfc_lastOpen;//the last order
                    WaitForCloseInfo* wfc_lastSecondOpen;//the last second order
                    if(tmpLongReverseList.size()==0){
                        return;
                    }
                    wfc_lastOpen=tmpLongReverseList.back();
                    if(lastPrice >= wfc_lastOpen->openPrice){
                        LOG(INFO)<<"although in Fibonacci state,but will not insert order,so if there are stop profit orders not traded,not care.";
                    }else{
                        //stop profit order not all traded,then first order action
                        cancelSpecTypeOrder(instrumentID,"2001");
                    }
                    int existGrades = (tmpLongReverseList.size() - techCls.oneNormalGrade);
                    LOG(INFO)<<"Current grade for two is "+boost::lexical_cast<string>(existGrades)
                               +",and the limit grade is "+boost::lexical_cast<string>(techCls.twoGrade);
                    setStageTick("5",existGrades);
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
                    //wfc_lastOpen=tmpLongReverseList.back();
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
                                        addinfo->flag="fbna";//not use for now
                                        addNewOrderTrade(instrumentID,"0","0",lastPrice,hopeVolume,"0",addinfo);
                                    }
                                }else{
                                    LOG(INFO) << "There are no untrade order exist,add new order.";
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="2052";
                                    addinfo->flag="fbna";//not use for now
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
                        LOG(INFO)<<"lock change to close all trade.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2077";
                        if(userHoldPst.longTotalPosition>0){
                            addNewOrderTrade(instrumentID,"1","1",mkData->bidPrice,userHoldPst.longTotalPosition,"agg",addinfo);
                        }
                        if(userHoldPst.shortTotalPosition>0){
                            addNewOrderTrade(instrumentID,"0","1",mkData->askPrice,userHoldPst.shortTotalPosition,"agg",addinfo);
                        }
                        ///before here is lock pst
                        /*sof->instrumentID=instrumentID;
                        sof->direction="1";
                        sof->offsetFlag="0";
                        if(techCls.isTestInviron){
                            sof->lastPrice=mkData->bidPrice;
                        }else{
                            InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                            sof->lastPrice=instInfo->LowerLimitPrice;
                            //sof->lastPrice=mkData->lowestPrice;
                        }
                        sof->volume=userHoldPst.longTotalPosition-userHoldPst.shortTotalPosition;
                        sof->orderType="0";
                        sof->openStgType="2062";
                        doSpecOrder(sof);*/
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
                //setRelockPrice(lastPrice,tickPrice,techCls.mainDirection);//?
                //setAddMinPrice(lastPrice,techCls.mainDirection);//for fbnq
                //WaitForCloseInfo* wfc_lastOpen=tmpLongReverseList.back();
                //double lockPrice = userHoldPst.shortHoldAvgPrice;//?
                //change lockPrice from avgHoldPrice to first open price
                if(techCls.lockFirstOpenPrice==0){
                    LOG(ERROR)<<"lock trade order not processed?waiting......";
                    return;
                }
                double lockPrice = techCls.lockFirstOpenPrice;
                ///change unlock logic
                ///
                ///
                ///
                ///
                if(lastPrice <= (lockPrice-techCls.lockWatchTickNums*tickPrice)){
                    LOG(INFO)<<"WATCHING:price goes down to "+boost::lexical_cast<string>(techCls.lockWatchTickNums)+" tick lower.lastPrice="+boost::lexical_cast<string>(lastPrice)+",lockPrice="
                               +boost::lexical_cast<string>(lockPrice);
                    //setRelockPrice(lastPrice,tickPrice,techCls.mainDirection);
                    if(techCls.minPrice == 0){
                        techCls.minPrice = lastPrice;
                        LOG(INFO)<<"begin to initialize minPrice="+boost::lexical_cast<string>(techCls.minPrice);
                        setDrawbackPrice(lockPrice,tickPrice,techCls.mainDirection);
                    }else if(techCls.minPrice > lastPrice){
                        double tmprice=techCls.minPrice;
                        techCls.minPrice = lastPrice;
                        setDrawbackPrice(lockPrice,tickPrice,techCls.mainDirection);
                        LOG(INFO)<<"Price become even more lower,set minPrice from "+boost::lexical_cast<string>(tmprice)+" to "
                                   +boost::lexical_cast<string>( techCls.minPrice);
                    }else{
                        LOG(INFO)<<"It's seems price may reverse to go up.minPrice="+boost::lexical_cast<string>(techCls.minPrice)+",lastPrice="
                                   +boost::lexical_cast<string>(lastPrice);
                        //int drawbackTickNums = round(((lockPrice-techCls.minPrice)*1.0/tickPrice)/techCls.drawbackTickRate);
                        //techCls.drawbackPrice = techCls.minPrice + drawbackTickNums*tickPrice;
                        //LOG(INFO)<<"drawbackTickNums="+boost::lexical_cast<string>(drawbackTickNums)+",drawbackPrice="+boost::lexical_cast<string>(techCls.drawbackPrice);
                        if(techCls.drawbackPrice!=0 && lastPrice >= techCls.drawbackPrice){
                            LOG(INFO)<<"Unlock condition one:Price triggered short direction stop loss,begin to unlock.orderPrice="+boost::lexical_cast<string>(mkData->highestPrice);
                            sof->instrumentID=instrumentID;
                            sof->direction="0";
                            sof->offsetFlag="1";
                            if(techCls.isTestInviron){
                                sof->lastPrice=mkData->askPrice;
                            }else{
                                InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                                sof->lastPrice=instInfo->UpperLimitPrice;
                            }
                            sof->volume=userHoldPst.shortTotalPosition;
                            sof->orderType="agg";
                            sof->openStgType="2071";
                            doSpecOrder(sof);
                        }else{
                            LOG(INFO)<<"drawbackPrice="+boost::lexical_cast<string>(techCls.drawbackPrice)+",not satisfy unlock condition,not process.";
                        }
                    }
                }else if(lastPrice > (lockPrice-techCls.lockWatchTickNums*tickPrice)&&lastPrice<=lockPrice){

                    if(techCls.minPrice == 0){
                        LOG(INFO)<<"After lock position,price not touch the lowest point of "+boost::lexical_cast<string>(techCls.lockWatchTickNums)+" tick lower.not process.";
                    }else if(techCls.minPrice != 0){
                        LOG(INFO)<<"Price touched the lowest point of "+boost::lexical_cast<string>(techCls.lockWatchTickNums)+" tick lower,and return back.";
                        if(techCls.drawbackPrice!=0 && lastPrice >= techCls.drawbackPrice){
                            LOG(INFO)<<"Unlock condition two:Price triggered short direction stop loss,begin to unlock.orderPrice="+boost::lexical_cast<string>(mkData->highestPrice);
                            sof->instrumentID=instrumentID;
                            sof->direction="0";
                            sof->offsetFlag="1";
                            if(techCls.isTestInviron){
                                sof->lastPrice=mkData->askPrice;
                            }else{
                                InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                                sof->lastPrice=instInfo->UpperLimitPrice;
                                //sof->lastPrice=mkData->highestPrice;
                            }
                            sof->volume=userHoldPst.shortTotalPosition;
                            sof->orderType="agg";
                            sof->openStgType="2071";
                            doSpecOrder(sof);
                        }else{
                            LOG(INFO)<<"drawbackPrice="+boost::lexical_cast<string>(techCls.drawbackPrice)+",not satisfy unlock condition,not process.";
                        }
                    }
                }else if(lastPrice <= techCls.lockFirstOpenPrice&&lastPrice>=lockPrice){
                    LOG(INFO)<<"price between lockFirstOpenPrice="+boost::lexical_cast<string>(techCls.lockFirstOpenPrice)+" and lockPrice="+boost::lexical_cast<string>(lockPrice)+",not process.";
                }else if(lastPrice > techCls.lockFirstOpenPrice&&techCls.lockFirstOpenPrice < mkData->bidPrice){
                    LOG(INFO)<<"Unlock condition three:price step over lockFirstOpenPrice="+boost::lexical_cast<string>(techCls.lockFirstOpenPrice)+",bidPrice="+boost::lexical_cast<string>(mkData->bidPrice)+",stop loss.";
                    sof->instrumentID=instrumentID;
                    sof->direction="0";
                    sof->offsetFlag="1";
                    if(techCls.isTestInviron){
                        sof->lastPrice=mkData->askPrice;
                    }else{
                        InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                        sof->lastPrice=instInfo->UpperLimitPrice;
                        //sof->lastPrice=mkData->highestPrice;
                    }
                    sof->volume=userHoldPst.shortTotalPosition;
                    sof->orderType="0";
                    sof->openStgType="2071e";
                    doSpecOrder(sof);
                }else{
                    LOG(INFO)<<"NOT PROCESS.";
                }
            }else if(techCls.priceStatus == "7"){//watch high price
                LOG(INFO)<<"now has been unlock,begin to watch high price ready to long unlock.minPrice="+boost::lexical_cast<string>(techCls.minPrice)+",relockPrice="
                           +boost::lexical_cast<string>(techCls.relockPrice)+",addMinPrice="+boost::lexical_cast<string>(techCls.additionMinPrice);
                double ATR15S_60C=techCls.trueKData15S->ATR*tickPrice;//get this techmetric from lib
                if(ATR15S_60C == 0){
                    ATR15S_60C=1.5*tickPrice;
                }
                ///if add addition fbna order
                /*
                if(techCls.additionMinPrice !=0 && lastPrice <= techCls.additionMinPrice){
                    LOG(INFO)<<"duration in unlock,price go down again and touch the minimal additionMinPrice="+boost::lexical_cast<string>(techCls.additionMinPrice)+",add additional fbna orders.";
                    if(techCls.isAddOrderOpen){
                        LOG(INFO)<<"additional FBNA order has been traded,not add new orders.";
                    }else{
                        int hopeVolume=getFBNAOrderVolume(tmpLongReverseList,"0");
                        LOG(INFO)<<"当前加仓数量:hopeVolume="
                                   +boost::lexical_cast<string>(hopeVolume);
                        //OrderInfo orderInfo;
                        if(existUntradeOrder("2052a",&orderInfo)){
                            if(orderInfo.price==lastPrice){
                                LOG(INFO) << "There are untrade order at this price,not process.";
                            }else{
                                LOG(INFO) << "There are untrade order exist,but not at this price,add new order.lastPrice="+boost::lexical_cast<string>(lastPrice)
                                             +",untradePrice="+boost::lexical_cast<string>(orderInfo.price);
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2052a";
                                addinfo->flag="fbna";
                                addNewOrderTrade(instrumentID,"0","0",lastPrice,hopeVolume,"0",addinfo);
                            }
                        }else{
                            LOG(INFO) << "There are no untrade order exist,add new order.";
                            AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                            addinfo->openStgType="2052a";
                            addinfo->flag="fbna";
                            addNewOrderTrade(instrumentID,"0","0",lastPrice,hopeVolume,"0",addinfo);
                        }
                    }
                }*/
                double metricPrice=(techCls.unlockPrice+6+techCls.rawPstStopLossTickNums*tickPrice+(techCls.lockTimes-1)*techCls.twoGap);
                //LOG(INFO)<<"ATR="+boost::lexical_cast<string>(ATR15S_60C)+",current maxPrice="+boost::lexical_cast<string>(techCls.maxPrice);
                LOG(INFO)<<"unlockprice="+boost::lexical_cast<string>(techCls.unlockPrice)+",rawPstTicks="+boost::lexical_cast<string>(techCls.rawPstStopLossTickNums)+",lockTimes="
                           +boost::lexical_cast<string>(techCls.lockTimes)+",towGap="+boost::lexical_cast<string>(techCls.twoGap)+";metric="
                           +boost::lexical_cast<string>(metricPrice);
                if(lastPrice >= metricPrice){
                    double tmpRawSLP=mkData->bidPrice - tickPrice;
                    if(techCls.rawStopLossPrice==0||(techCls.rawStopLossPrice < tmpRawSLP)){
                        double preRawslp = techCls.rawStopLossPrice;
                        techCls.rawStopLossPrice=tmpRawSLP;
                        LOG(INFO)<<"lastprice has over metric,set raw stop losss price from "+boost::lexical_cast<string>(preRawslp)+ " to "+boost::lexical_cast<string>(techCls.rawStopLossPrice);
                    }
                }else if(lastPrice < metricPrice&&lastPrice > techCls.relockPrice){
                    LOG(INFO)<<"Not trigger,price="+boost::lexical_cast<string>(lastPrice)+" is still between stop loss price="+boost::lexical_cast<string>(metricPrice)+" and relockPrice="+
                               boost::lexical_cast<string>(techCls.relockPrice)+"rawStopLossPrice="+boost::lexical_cast<string>(techCls.rawStopLossPrice)+",not process.";
                }else if(lastPrice <= techCls.relockPrice){
                    LOG(INFO)<<"WARNING:relock position!!!!After unlock,price goes down again,and trigger relock condition.lastPrice("+boost::lexical_cast<string>(lastPrice)+") <= relockPrice("
                               +boost::lexical_cast<string>(techCls.relockPrice)+").";
                    LOG(INFO)<<"before relock,check if all of last unlock order is traded.if not then action.";
                    if(existUntradeOrder("2071",NULL)){
                        tryAllOrderAction(instrumentID);
                    }
                    //add max lock times
                    if(techCls.lockTimes >= 2){
                        LOG(INFO)<<"lock times over metric 2,close all trade.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2077";
                        if(userHoldPst.longTotalPosition>0){
                            addNewOrderTrade(instrumentID,"1","1",mkData->bidPrice,userHoldPst.longTotalPosition,"agg",addinfo);
                        }
                        if(userHoldPst.shortTotalPosition>0){
                            addNewOrderTrade(instrumentID,"0","1",mkData->askPrice,userHoldPst.shortTotalPosition,"agg",addinfo);
                        }
                    }else{
                        sof->instrumentID=instrumentID;
                        sof->direction="1";
                        sof->offsetFlag="0";
                        if(techCls.isTestInviron){
                            sof->lastPrice=mkData->bidPrice;
                        }else{
                            InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                            sof->lastPrice=instInfo->LowerLimitPrice;
                            //sof->lastPrice=mkData->lowestPrice;
                        }
                        sof->volume=userHoldPst.longTotalPosition-userHoldPst.shortTotalPosition;
                        sof->orderType="0";
                        sof->openStgType="2062";
                        doSpecOrder(sof);
                    }

                }
                ///seprate judge
                if(techCls.rawStopLossPrice != 0 && lastPrice <= techCls.rawStopLossPrice){
                    LOG(INFO)<<"Unlock condition six:Price triggered long direction stop loss,begin to unlock.price over rawStopLossPrice="+boost::lexical_cast<string>(techCls.rawStopLossPrice);
                    sof->instrumentID=instrumentID;
                    sof->direction="1";
                    sof->offsetFlag="1";
                    if(techCls.isTestInviron){
                        sof->lastPrice=mkData->bidPrice;
                    }else{
                        InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                        sof->lastPrice=instInfo->LowerLimitPrice;
                        //sof->lastPrice=mkData->lowestPrice;
                    }
                   // sof->lastPrice=lastPrice;
                    sof->volume=userHoldPst.longTotalPosition;
                    sof->orderType="agg";
                    sof->openStgType="2081";
                    doSpecOrder(sof);
                }
                ///seprate judge


                ////
                /*
                if(lastPrice >= (techCls.unlockPrice + techCls.watchUnlockAnotherATRNums*ATR15S_60C)){
                    if(techCls.maxPrice == 0){
                        techCls.maxPrice = lastPrice;
                        LOG(INFO)<<"begin to initialize maxPrice="+boost::lexical_cast<string>(techCls.maxPrice);
                    }else if(techCls.maxPrice < lastPrice){
                        double tmprice = techCls.maxPrice;
                        techCls.maxPrice = lastPrice;
                        LOG(INFO)<<"Price become even more higher,set maxPrice from "+boost::lexical_cast<string>(tmprice)+" to "
                                   +boost::lexical_cast<string>( techCls.maxPrice);
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
                        }else{
                            LOG(INFO)<<"price go back to "+boost::lexical_cast<string>(lastPrice)+",not satisfy unlock condition,not process.";
                        }
                    }
                }else{
                    LOG(INFO)<<"Not trigger lastPrice="+boost::lexical_cast<string>(lastPrice) +" >= (techCls.unlockPrice + n*ATR15S_60C)="
                               +boost::lexical_cast<string>(techCls.unlockPrice)+"+"
                               +boost::lexical_cast<string>(ATR15S_60C*techCls.watchUnlockAnotherATRNums)+")";
                }
                ///add when price not over atr
                if(lastPrice < (techCls.unlockPrice + techCls.watchUnlockAnotherATRNums*ATR15S_60C)){

                    if(techCls.maxPrice!=0){
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
                        }else{
                            LOG(INFO)<<"price go back to "+boost::lexical_cast<string>(lastPrice)+",not satisfy unlock condition,not process.";
                        }
                    }
                }
                ///another method to be kicked out
                if((lastPrice - techCls.unlockPrice) >= techCls.timesOfStopLoss*techCls.twoGap*tickPrice){
                    LOG(INFO)<<"Unlock condition six:Price triggered long direction stop loss,begin to unlock.price over "+boost::lexical_cast<string>(techCls.timesOfStopLoss)+" times * twogap.";
                    sof->instrumentID=instrumentID;
                    sof->direction="1";
                    sof->offsetFlag="1";
                    sof->lastPrice=lastPrice;
                    sof->volume=userHoldPst.longTotalPosition;
                    sof->orderType="0";
                    sof->openStgType="2081";
                    doSpecOrder(sof);
                }

                else if((techCls.unlockPrice - lastPrice) >= techCls.relockATRNums*ATR15S_60C){
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
                        doSpecOrder(sof);
                    }

                }*/


            }else{
                LOG(INFO)<<"priceStatus="+techCls.priceStatus+",this kind of status not define.";
            }
        }

    }
}
void shortDirectionTrade(MarketData *mkData){
    double lastPrice = mkData->lastPrice;
    string instrumentID = mkData->instrumentID;
    double tickPrice = getPriceTick(instrumentID);
    if(techCls.mainDirection=="1"||techCls.mainDirection=="3"||techCls.mainDirection=="12"){
        if((techCls.stgStatus=="0"||techCls.stgStatus=="1"||techCls.stgStatus=="2")){
            if(techCls.KData_15s.size() <= 1){
                LOG(INFO) << "the first k 15 line has not generated,not process!";
            }else if(techCls.KData_15s.size() == 2){
                LOG(INFO) << "for short condition,the first k 15 line of signal has appeared!";
                if(techCls.stgStatus=="0"){
                    techCls.stgStatus="1";
                    LOG(INFO) << "This is the just tick price to generate a 15s k line,not process!but init shadow and sun line";
                    initSunOrShadowLine(techCls.mainDirection);
                }else if(techCls.stgStatus=="1"){
                    techCls.stgStatus="2";
                    LOG(INFO) << "This is the just first tick price of a newly generated 15s k line,and compute dual trust parameters!";
                    computeDualTrustPara(mkData->lastPrice);
                }else if(techCls.stgStatus=="2"){//step into first open status
                    LOG(INFO) << "this is the second 15s k line's first open status.firstOpenKLineType="+techCls.firstOpenKLineType;
                    if(techCls.firstOpenKLineType=="2"){//shadow line
                        if(lastPrice<=techCls.limit[0]){
                            if(existUntradeOrder("3000",NULL)){
                                LOG(INFO) << "There are orders not fully traded,waiting.";
                            }else{
                                LOG(INFO) << "Begin to order insert first open orders.range=["+boost::lexical_cast<string>(techCls.limit[0])+","
                                        +boost::lexical_cast<string>(techCls.limit[1])+"],lastPrice="+boost::lexical_cast<string>(lastPrice);
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="3000";
                                addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
                            }
                        }else{
                            LOG(INFO)<<"shadow line,but lastprice="+boost::lexical_cast<string>(lastPrice)+" not over lower limit="+boost::lexical_cast<string>(techCls.limit[0]);
                        }
                    }else if(techCls.firstOpenKLineType=="1"){//sun line
                        LOG(INFO) << "This is the first 15s k sun line,not process.";
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
                        if(techCls.firstOpenKLineType=="2"){//shadow line
                            techCls.stgStatus="1";
                            vector<Strategy::Kdata>::iterator d15it=techCls.KData_15s.begin();
                            techCls.KData_15s.erase(d15it);
                            LOG(INFO) << "shadow line.This is the just tick price to generate a 15s k line.because first open not happen in last 15s k line,so not process!but init shadow and sun line again,and reset stgStatus to 1.delete first k 15s line item.after delete size="+
                                          boost::lexical_cast<string>(techCls.KData_15s.size())+"(hope size is 2)";
                            initSunOrShadowLine(techCls.mainDirection);
                        }else if(techCls.firstOpenKLineType=="1"){//sun line
                            LOG(INFO) << "current's 15s k closePrice="+
                                          boost::lexical_cast<string>(techCls.trueKData15S->closePrice)+",limit=["+
                                          boost::lexical_cast<string>(techCls.limit[0])+","+
                                          boost::lexical_cast<string>(techCls.limit[1])+"]."  ;
                            if(techCls.trueKData15S->closePrice < techCls.limit[0]){
                                LOG(INFO) << "Last 15s k is a sun line,and current's closePrice="+
                                              boost::lexical_cast<string>(techCls.trueKData15S->closePrice)+" is lower than lower edge="+
                                              boost::lexical_cast<string>(techCls.limit[0])+" of dual limit.Begin to order insert first open orders.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="3000";
                                addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);

                            }else{
                                techCls.stgStatus="1";
                                vector<Strategy::Kdata>::iterator d15it=techCls.KData_15s.begin();
                                techCls.KData_15s.erase(d15it);
                                string msg="sun line.This is the just tick price to generate a 15s k line.because first open not happen in last 15s k line,so not process!but init shadow and sun line again,and reset stgStatus to 1.delete first k 15s line item.after delete size="+boost::lexical_cast<string>(techCls.KData_15s.size())+"(hope size is 2)";
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
                if(lastPrice <= techCls.firstOpenPrice&&lastPrice>(techCls.firstOpenPrice-techCls.nTickMoveSL*priceTick)){
                    LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["+
                                  boost::lexical_cast<string>(techCls.firstOpenPrice)+","+
                                  boost::lexical_cast<string>(techCls.firstOpenPrice-techCls.nTickMoveSL*priceTick)+"].This is the first adjust range,not process.";
                }else if(lastPrice <= (techCls.firstOpenPrice-techCls.nTickMoveSL*priceTick)&&lastPrice>(techCls.firstOpenPrice-2*techCls.nTickMoveSL*priceTick)){
                    techCls.priceStatus="1";
                    LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                  +boost::lexical_cast<string>(techCls.firstOpenPrice-techCls.nTickMoveSL*priceTick)+","
                                  +boost::lexical_cast<string>(techCls.firstOpenPrice-2*techCls.nTickMoveSL*priceTick)+"].This is the second adjust range,move up one tick for stop loss price,change stop loss price from "
                                  +boost::lexical_cast<string>(techCls.stopLossPrice)+" to "
                                  +boost::lexical_cast<string>(techCls.firstOpenPrice-priceTick*techCls.stopLossPriceTick);
                    techCls.stopLossPrice=techCls.firstOpenPrice-priceTick*techCls.stopLossPriceTick;
                }else if(lastPrice<=(techCls.firstOpenPrice-2*techCls.nTickMoveSL*priceTick)){
                    if(existUntradeOrder("2022",NULL)){
                        LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                      +boost::lexical_cast<string>(techCls.firstOpenPrice-2*techCls.nTickMoveSL*priceTick)+","
                                      +"].There are untrade order,not process.";
                    }else{
                        /*LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                      +boost::lexical_cast<string>(techCls.firstOpenPrice+2*techCls.nTickMoveSL*priceTick)+","
                                      +"].This is the '2 tick trigger stop loss' range,change stop loss price from "
                                      +boost::lexical_cast<string>(techCls.stopLossPrice)+" to "
                                      +boost::lexical_cast<string>(lastPrice-techCls.nJumpTriggerSL*priceTick);*/
                        LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                      +boost::lexical_cast<string>(techCls.firstOpenPrice-2*techCls.nTickMoveSL*priceTick)+","
                                      +"].This is the '2 tick trigger stop loss' range.add new order.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2022";
                        //addNewOrderTrade(instrumentID,"0","0",lastPrice,ceil(techCls.firstMetricVolume/3.0),"0",addinfo);
                        addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
                    }
                }else {//if price between one sweet range
                    LOG(INFO)<<"Price may step into one sweet range.lastPrice="+boost::lexical_cast<string>(lastPrice)+",firstToSweet=["+
                               boost::lexical_cast<string>(techCls.firstOpenPrice)+","+boost::lexical_cast<string>(techCls.firstOpenPrice + techCls.firstToSweetTickNums*tickPrice)+").";
                    WaitForCloseInfo* wfc_lastOpen = allTradeList.back();
                    if(wfc_lastOpen->openPrice > techCls.firstOpenPrice){
                        LOG(INFO)<<"There is order traded in one sweet range,showing that price trace is sweet->first->sweet,set priceStatus=3.";
                        techCls.priceStatus="3";
                    }else{
                        double simFirstOpenPrice=techCls.firstOpenPrice + techCls.firstToSweetTickNums*tickPrice;
                        if(lastPrice < simFirstOpenPrice){
                            LOG(INFO)<<"price not touch sweet.";
                        }else if(lastPrice <= (simFirstOpenPrice + techCls.oneSweetGrade*techCls.oneSweetGap*tickPrice)){
                            LOG(INFO)<<"price in sweet range.";
                            for(int i=0;i<techCls.oneSweetGrade;i++){
                                if(lastPrice>=simFirstOpenPrice + (i+1)*techCls.oneSweetGap*tickPrice
                                        &&lastPrice < simFirstOpenPrice + (i+2)*techCls.oneSweetGap*tickPrice){
                                    LOG(INFO) << "now lastPrice step into one sweet range's "+boost::lexical_cast<string>(i+1)
                                                  +" and "+boost::lexical_cast<string>(i+2)+" jump.";

                                    if(lastPrice > wfc_lastOpen->openPrice&&(int((lastPrice - simFirstOpenPrice)/tickPrice))%techCls.oneSweetGap==0){
                                        LOG(INFO)<<"This price is at one sweet gap,judge if add new order.";
                                        sof->instrumentID=instrumentID;
                                        sof->direction="1";
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
                                        +boost::lexical_cast<string>(techCls.firstOpenPrice+techCls.oneSweetGrade*techCls.oneSweetGap*tickPrice);
                            techCls.priceStatus="3";
                            LOG(INFO)<<"warning:an unbelievable price fluctuation appears.set priceStatus=3 directly.";
                        }

                    }
                }
            }else if(techCls.priceStatus=="1"||techCls.priceStatus=="2"){
                LOG(INFO)<<"lastPrice="+boost::lexical_cast<string>(lastPrice)+ ",priceStatus="+techCls.priceStatus+",price step into between firstOpenPrice and 2 tick jump stop loss.stopLossPrice="
                           +boost::lexical_cast<string>(techCls.stopLossPrice)+",open new order price="
                           +boost::lexical_cast<string>(techCls.firstOpenPrice-2*techCls.nTickMoveSL*tickPrice);
                if(lastPrice >= techCls.stopLossPrice){//
                    sof->instrumentID=instrumentID;
                    sof->direction="0";
                    sof->offsetFlag="1";
                    sof->lastPrice=lastPrice;
                    sof->volume=userHoldPst.shortTotalPosition;
                    sof->orderType="0";
                    sof->openStgType="2011";
                    doSpecOrder(sof);
                    //OrderInfo orderInfo;
                    /*
                    if(existUntradeOrder("2011",&orderInfo)){
                        LOG(INFO)<<"LastPirce ="+boost::lexical_cast<string>(lastPrice)+" trigger stopLossPrice="
                                   +boost::lexical_cast<string>(techCls.stopLossPrice)+",there are untraded order,not process.";
                    }else{
                        LOG(INFO)<<"LastPirce ="+boost::lexical_cast<string>(lastPrice)+" trigger stopLossPrice="
                                   +boost::lexical_cast<string>(techCls.stopLossPrice)+",there are not untraded order,add new order.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2011";
                        addNewOrderTrade(instrumentID,"0","1",lastPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                    }*/
                }else if(lastPrice<=(techCls.firstOpenPrice-2*techCls.nTickMoveSL*tickPrice)){
                    if(int(((techCls.firstOpenPrice-2*techCls.nTickMoveSL*tickPrice)-lastPrice)/tickPrice)%2==0){
                        LOG(INFO)<<"price over double nTickMoveSL and satisfy 2 tick jump stop loss.lastPrice="
                                   +boost::lexical_cast<string>(lastPrice)+",firstOpenPrice="
                                   +boost::lexical_cast<string>(techCls.firstOpenPrice)+",double nTickMoveSL="
                                   +boost::lexical_cast<string>(techCls.firstOpenPrice-2*techCls.nTickMoveSL*tickPrice);
                        WaitForCloseInfo* wfc_lastOpen = allTradeList.back();
                        if(lastPrice < wfc_lastOpen->openPrice){
                            if(existUntradeOrder("2022",NULL)){
                                LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of [,"
                                              +boost::lexical_cast<string>(techCls.firstOpenPrice-2*techCls.nTickMoveSL*tickPrice)
                                              +"].There are untrade order,not process.";
                            }else{
                                LOG(INFO) << "now lastPrice="+boost::lexical_cast<string>(lastPrice)+" is in the range of ["
                                              +boost::lexical_cast<string>(techCls.firstOpenPrice-2*techCls.nTickMoveSL*tickPrice)+","
                                              +"].This is the '2 tick trigger stop loss' range.add new order.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2022";
                                //addNewOrderTrade(instrumentID,"0","0",lastPrice,ceil(techCls.firstMetricVolume/3.0),"0",addinfo);
                                addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
                            }
                        }else{
                            LOG(INFO)<<"There are orders traded at lastPrice="+boost::lexical_cast<string>(lastPrice)+",not process.";
                        }
                    }else{
                        LOG(INFO)<<"price not satisfy 2 tick jump stop loss,not process.lastPrice="
                                   +boost::lexical_cast<string>(lastPrice)+",firstOpenPrice="
                                   +boost::lexical_cast<string>(techCls.firstOpenPrice)+",double nTickMoveSL="
                                   +boost::lexical_cast<string>(techCls.firstOpenPrice-2*techCls.nTickMoveSL*tickPrice);
                    }

                }
            }else if(techCls.priceStatus=="3"){//move in one sweet range
                //LOG(INFO)<<"Price may step in on";
                double simFirstOpenPrice=techCls.firstOpenPrice + techCls.firstToSweetTickNums*tickPrice;
                if(techCls.firstOpenPrice < lastPrice&&lastPrice < simFirstOpenPrice){
                    LOG(INFO)<<"Price in middle range of first and sweet.not process.";
                }else if(lastPrice <= (simFirstOpenPrice+techCls.oneSweetGrade*techCls.oneSweetGap*tickPrice)
                        &&lastPrice >= simFirstOpenPrice){
                    WaitForCloseInfo* wfc_lastOpen = allTradeList.back();
                    for(int i=0;i<techCls.oneSweetGrade;i++){
                        if(lastPrice>=simFirstOpenPrice + (i+1)*techCls.oneSweetGap*tickPrice
                                &&lastPrice < simFirstOpenPrice + (i+2)*techCls.oneSweetGap*tickPrice){
                            LOG(INFO) << "now lastPrice step into one sweet range's "+boost::lexical_cast<string>(i+1)
                                          +" and "+boost::lexical_cast<string>(i+2)+" jump.";

                            if(lastPrice > wfc_lastOpen->openPrice&&(int((lastPrice-simFirstOpenPrice)/tickPrice))%techCls.oneSweetGap==0){
                                LOG(INFO)<<"This price is at one sweet gap,judge if add new order.";
                                sof->instrumentID=instrumentID;
                                sof->direction="1";
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
                }else if(lastPrice < techCls.firstOpenPrice){//step into range of over first open.
                    LOG(INFO)<<"step into range of over first open,cancell all one sweet orders.";
                    cancelSpecTypeOrder(instrumentID,"2032");
                    string tmpsts = techCls.priceStatus;
                    techCls.priceStatus = "0";
                    LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus+",return to status of first open.";
                }else if(lastPrice > (simFirstOpenPrice+techCls.oneSweetGrade*techCls.oneSweetGap*tickPrice + techCls.sweetToNormalTickNums*tickPrice)){
                    LOG(INFO)<<"step into range of one normal.firstOpenPriceNormal="+boost::lexical_cast<string>(techCls.firstOpenPriceNormal);//until here,below how
                    WaitForCloseInfo* wfc_lastOpen=allTradeList.back();//the last order
                    if(techCls.firstOpenPriceNormal ==0 && lastPrice > wfc_lastOpen->openPrice){//not step into one normal before
                        if(existUntradeOrder("2042",NULL)){
                            LOG(INFO) << "There are untrade order,not process.";
                        }else{
                            LOG(INFO) << "There are no untrade order exist,add new order.";
                            AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                            addinfo->openStgType="2042";
                            addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.oneNormalVolume,"0",addinfo);
                        }
                    }else{
                        string tmpsts=techCls.priceStatus;
                        techCls.priceStatus="4";
                        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus;
                        int tmplist=allTradeList.size();
                        longReverseList.clear();
                        //transfer all order in WaitForCloseInfo to longReverseList
                        for(list<WaitForCloseInfo*>::iterator wfIt = allTradeList.begin();wfIt!=allTradeList.end();){
                            //tmpLongReverseList->push_back(*wfIt);
                            longReverseList.push_back(*wfIt);
                            wfIt = allTradeList.erase(wfIt);
                        }
                        LOG(INFO)<<"When step into one normal range again,all order traded before will be moved to reverselist.before allTradeList="+boost::lexical_cast<string>(tmplist)
                                   +",after allTradeList="+boost::lexical_cast<string>(allTradeList.size())
                                   +",longReserseList="+boost::lexical_cast<string>(longReverseList.size());
                    }

                }else{
                    LOG(INFO)<<"Where am i?lastPrice="+boost::lexical_cast<string>(lastPrice);
                }
            }else if(techCls.priceStatus=="4"){//one normal
                //if(stopProfit("0",lastPrice,instrumentID)){//
                if(false){//
                    LOG(INFO)<<"long reverse is stopping profit,not process.";
                }else{
                    //stop profit order not all traded,then first order action.must.
                    //cancelSpecTypeOrder(instrumentID,"2001");
                    WaitForCloseInfo* wfc_firstOpen;//the first order
                    WaitForCloseInfo* wfc_lastOpen;//the last order
                    WaitForCloseInfo* wfc_lastSecondOpen;//the last second order
                    if(tmpLongReverseList.size()>0){
                        list<WaitForCloseInfo*>::iterator atIT=tmpLongReverseList.begin();
                        wfc_firstOpen=*atIT;
                    }
                    wfc_lastOpen=tmpLongReverseList.back();
                    /*
                    if(lastPrice >= wfc_lastOpen->openPrice){
                        LOG(INFO)<<"although in one normal state,but will not insert order,so if there are stop profit orders not traded,not care.";
                    }else{
                        //stop profit order not all traded,then first order action
                        cancelSpecTypeOrder(instrumentID,"2001");
                    }*/
                    //price will jump to sweet
                    double simFirstOpenPrice=techCls.firstOpenPrice + techCls.firstToSweetTickNums*tickPrice;
                    if(lastPrice <= (simFirstOpenPrice+techCls.oneSweetGrade*techCls.oneSweetGap*tickPrice + techCls.sweetToNormalTickNums*tickPrice)){
                        LOG(INFO)<<"step into range of one sweet.cancell all one normal orders.";
                        cancelSpecTypeOrder(instrumentID,"2042");
                        string tmpsts = techCls.priceStatus;
                        techCls.priceStatus = "3";
                        LOG(INFO)<<"set priceStatus from "+tmpsts+" to "+techCls.priceStatus+",return to status of one sweet.";


                        //transfer all order in longReverseList to allTradeList
                        int tmplist=longReverseList.size();
                        allTradeList.clear();
                        for(list<WaitForCloseInfo*>::iterator wfIt = longReverseList.begin();wfIt!=longReverseList.end();){
                            allTradeList.push_back(*wfIt);
                            wfIt = longReverseList.erase(wfIt);
                        }
                        LOG(INFO)<<"When from one normal to sweet,all order traded before will be moved to allTradeList.before longReverseList="+boost::lexical_cast<string>(tmplist)
                                   +",after longReverseList="+boost::lexical_cast<string>(longReverseList.size())
                                   +",allTradeList="+boost::lexical_cast<string>(allTradeList.size());
                    }else if(tmpLongReverseList.size() < techCls.oneNormalGrade){
                        LOG(INFO)<<"long逆向加仓处于第一阈值内,each grade加仓量="+boost::lexical_cast<string>(techCls.oneNormalVolume);
                        if(wfc_firstOpen&&int((lastPrice-wfc_firstOpen->openPrice)/tickPrice)%techCls.oneNormalGap==0){
                            LOG(INFO)<<"this price is at one normal gap,judge if add new order.lastPrice="+boost::lexical_cast<string>(lastPrice);
                            if(lastPrice > wfc_lastOpen->openPrice){
                                LOG(INFO)<<"lastPrice is higher than the last order price,add new order,加仓量="+boost::lexical_cast<string>(techCls.oneNormalVolume);
                                //OrderInfo orderInfo;
                                if(existUntradeOrder("2042",&orderInfo)){
                                    if(orderInfo.price==lastPrice){
                                        LOG(INFO) << "There are untrade order at this price,not process.";
                                    }else{
                                        LOG(INFO) << "There are untrade order exist,but not at this price,add new order.";
                                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                        addinfo->openStgType="2042";
                                        addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.oneNormalVolume,"0",addinfo);
                                    }
                                }else{
                                    LOG(INFO) << "There are no untrade order exist,add new order.";
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="2042";
                                    addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.oneNormalVolume,"0",addinfo);
                                }
                            }else{
                                LOG(INFO)<<"There are already orders at this price,not process.";
                            }
                        }else{
                            LOG(INFO)<<"This is not order insert price,not process.";
                        }
                    }else if(tmpLongReverseList.size() >= techCls.oneNormalGrade
                             &&lastPrice > wfc_lastOpen->openPrice){//Fibonacci
                        LOG(INFO)<<"Price step into two status:Fibonacci.";
                        int existGrades = (tmpLongReverseList.size() - techCls.oneNormalGrade);
                        LOG(INFO)<<"Current grade for two is "+boost::lexical_cast<string>(existGrades)
                                   +",and the limit grade is "+boost::lexical_cast<string>(techCls.twoGrade);
                        setStageTick("5",existGrades);
                        int tmpGrade=0;
                        for(list<WaitForCloseInfo*>::iterator wfit=tmpLongReverseList.begin();wfit != tmpLongReverseList.end();wfit++){
                            tmpGrade += 1;
                            if(tmpGrade == techCls.oneNormalGrade){
                                wfc_firstOpen = *wfit;
                                break;
                            }
                        }
                        if(existGrades < techCls.twoGrade){
                            if(int((lastPrice - wfc_firstOpen->openPrice)/tickPrice)%techCls.twoGap==0){
                                LOG(INFO)<<"This price is at two gap,judge if add new order.";
                                if(lastPrice > wfc_lastOpen->openPrice){
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
                                            addNewOrderTrade(instrumentID,"1","0",lastPrice,hopeVolume,"0",addinfo);
                                        }
                                    }else{
                                        LOG(INFO) << "There are no untrade order exist,add new order.";
                                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                        addinfo->openStgType="2052";
                                        addNewOrderTrade(instrumentID,"1","0",lastPrice,hopeVolume,"0",addinfo);
                                    }
                                }else{
                                    LOG(INFO)<<"There are already orders at this price,not process.";
                                }

                            }
                        }

                    }else{
                        LOG(INFO)<<"price in one normal,not touch any trigger,not process.";
                    }
                }
            }else if(techCls.priceStatus=="5"){//tow status
                LOG(INFO)<<"Price step into two status:Fibonacci.";
                //judge if we will stop profit.at the last tow grade,can insert limit close order.
                if(techCls.stageTick == 1){
                    stopProfit("1",lastPrice,instrumentID);
                }
                //if(stopProfit("0",lastPrice,instrumentID)){//
                if(stopLoss("1",lastPrice,mkData->bidPrice,mkData->askPrice,instrumentID)){
                    LOG(INFO)<<"short reverse is stopping profit,not process.";
                }else{
                    WaitForCloseInfo* wfc_firstOpen;//the first order
                    WaitForCloseInfo* wfc_lastOpen;//the last order
                    WaitForCloseInfo* wfc_lastSecondOpen;//the last second order
                    if(tmpLongReverseList.size()==0){
                        return;
                    }
                    wfc_lastOpen=tmpLongReverseList.back();
                    if(lastPrice <= wfc_lastOpen->openPrice){
                        LOG(INFO)<<"although in Fibonacci state,but will not insert order,so if there are stop profit orders not traded,not care.";
                    }else{
                        //stop profit order not all traded,then first order action
                        cancelSpecTypeOrder(instrumentID,"2001");
                    }
                    int existGrades = (tmpLongReverseList.size() - techCls.oneNormalGrade);
                    LOG(INFO)<<"Current grade for two is "+boost::lexical_cast<string>(existGrades)
                               +",and the limit grade is "+boost::lexical_cast<string>(techCls.twoGrade);
                    setStageTick("5",existGrades);
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
                                    addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.protectVolume,"0",addinfo);
                                }
                            }else{
                                LOG(INFO) << "There are no untrade order exist,add new order.";
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="p";
                                addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.protectVolume,"0",addinfo);
                            }
                        }else{
                            WaitForCloseInfo* p_lastOpen = protectList.back();//the last order
                            if(lastPrice > p_lastOpen->openPrice){
                                LOG(INFO)<<"For protection,lastPrice is lower than the last order price,add new order.";
                                //OrderInfo orderInfo;
                                if(existUntradeOrder("p",&orderInfo)){
                                    if(orderInfo.price==lastPrice){
                                        LOG(INFO) << "There are untrade order at this price,not process.";
                                    }else{
                                        LOG(INFO) << "There are untrade order exist,but not at this price,add new order.";
                                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                        addinfo->openStgType="p";
                                        addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.protectVolume,"0",addinfo);
                                    }
                                }else{
                                    LOG(INFO) << "There are no untrade order exist,add new order.";
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="p";
                                    addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.protectVolume,"0",addinfo);
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
                    //wfc_lastOpen=tmpLongReverseList.back();
                    if(existGrades < techCls.twoGrade){
                        if(int((lastPrice - wfc_firstOpen->openPrice)/tickPrice)%techCls.twoGap==0){
                            LOG(INFO)<<"This price is at two gap,judge if add new order.";
                            if(lastPrice > wfc_lastOpen->openPrice){
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
                                        addinfo->flag="fbna";//not use for now
                                        addNewOrderTrade(instrumentID,"1","0",lastPrice,hopeVolume,"0",addinfo);
                                    }
                                }else{
                                    LOG(INFO) << "There are no untrade order exist,add new order.";
                                    AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                    addinfo->openStgType="2052";
                                    addinfo->flag="fbna";//not use for now
                                    addNewOrderTrade(instrumentID,"1","0",lastPrice,hopeVolume,"0",addinfo);
                                }
                            }else{
                                LOG(INFO)<<"There are already orders at this price,not process.lastPrice="+boost::lexical_cast<string>(lastPrice)+",lastOpenPrice="
                                           +boost::lexical_cast<string>(wfc_lastOpen->openPrice);
                            }

                        }
                    }else if(existGrades >= techCls.twoGrade&&lastPrice > wfc_lastOpen->openPrice){//lock position
                        LOG(INFO)<<"Warning!!!!!Lock position!!!!Current grade for two is "+boost::lexical_cast<string>(existGrades)
                                   +",and the limit grade is "+boost::lexical_cast<string>(techCls.twoGrade)
                                   +",lastPrice="+boost::lexical_cast<string>(lastPrice)+" is higher than the last open price="
                                   +boost::lexical_cast<string>( wfc_lastOpen->openPrice);
                        LOG(INFO)<<"lock change to close all trade.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2077";
                        if(userHoldPst.longTotalPosition>0){
                            addNewOrderTrade(instrumentID,"1","1",mkData->bidPrice,userHoldPst.longTotalPosition,"agg",addinfo);
                        }
                        if(userHoldPst.shortTotalPosition>0){
                            addNewOrderTrade(instrumentID,"0","1",mkData->askPrice,userHoldPst.shortTotalPosition,"agg",addinfo);
                        }

                        ///below here not use
                        /*sof->instrumentID=instrumentID;
                        sof->direction="0";
                        sof->offsetFlag="0";
                        if(techCls.isTestInviron){
                            sof->lastPrice=mkData->bidPrice;
                        }else{
                            InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                            sof->lastPrice=instInfo->UpperLimitPrice;
                            //sof->lastPrice=mkData->lowestPrice;
                        }
                        sof->volume=userHoldPst.shortTotalPosition - userHoldPst.longTotalPosition;
                        sof->orderType="0";
                        sof->openStgType="2062";
                        doSpecOrder(sof);*/
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
                //setRelockPrice(lastPrice,tickPrice,techCls.mainDirection);//?
                //setAddMinPrice(lastPrice,techCls.mainDirection);//for fbnq
                //WaitForCloseInfo* wfc_lastOpen=tmpLongReverseList.back();
                //double lockPrice = userHoldPst.shortHoldAvgPrice;//?
                //change lockPrice from avgHoldPrice to first open price
                if(techCls.lockFirstOpenPrice==0){
                    LOG(ERROR)<<"lock trade order not processed?waiting......";
                    return;
                }
                double lockPrice = techCls.lockFirstOpenPrice;
                ///change unlock logic
                ///
                ///
                ///
                ///
                if(lastPrice >= (lockPrice+techCls.lockWatchTickNums*tickPrice)){
                    LOG(INFO)<<"WATCHING:price goes up to "+boost::lexical_cast<string>(techCls.lockWatchTickNums)+" tick higher.lastPrice="+boost::lexical_cast<string>(lastPrice)+",lockPrice="
                               +boost::lexical_cast<string>(lockPrice);
                    //setRelockPrice(lastPrice,tickPrice,techCls.mainDirection);
                    if(techCls.maxPrice == 0){
                        techCls.maxPrice = lastPrice;
                        LOG(INFO)<<"begin to initialize maxPrice="+boost::lexical_cast<string>(techCls.maxPrice);
                        //setDrawbackPrice(lockPrice,tickPrice,"1");
                        setDrawbackPrice(lockPrice,tickPrice,techCls.mainDirection);
                    }else if(techCls.maxPrice < lastPrice){
                        double tmprice=techCls.maxPrice;
                        techCls.maxPrice = lastPrice;
                        setDrawbackPrice(lockPrice,tickPrice,techCls.mainDirection);
                        LOG(INFO)<<"Price become even more higher,set maxPrice from "+boost::lexical_cast<string>(tmprice)+" to "
                                   +boost::lexical_cast<string>( techCls.maxPrice);
                    }else{
                        LOG(INFO)<<"It's seems price may reverse to go down.maxPrice="+boost::lexical_cast<string>(techCls.maxPrice)+",lastPrice="
                                   +boost::lexical_cast<string>(lastPrice);
                        //int drawbackTickNums = round(((lockPrice-techCls.minPrice)*1.0/tickPrice)/techCls.drawbackTickRate);
                        //techCls.drawbackPrice = techCls.minPrice + drawbackTickNums*tickPrice;
                        //LOG(INFO)<<"drawbackTickNums="+boost::lexical_cast<string>(drawbackTickNums)+",drawbackPrice="+boost::lexical_cast<string>(techCls.drawbackPrice);
                        if(techCls.drawbackPrice!=0 && lastPrice <= techCls.drawbackPrice){
                            LOG(INFO)<<"Unlock condition one:Price triggered long direction stop loss,begin to unlock.orderPrice="+boost::lexical_cast<string>(mkData->highestPrice);
                            sof->instrumentID=instrumentID;
                            sof->direction="1";
                            sof->offsetFlag="1";
                            if(techCls.isTestInviron){
                                sof->lastPrice=mkData->bidPrice;
                            }else{
                                InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                                sof->lastPrice=instInfo->LowerLimitPrice;
                            }
                            sof->volume=userHoldPst.longTotalPosition;
                            sof->orderType="agg";
                            sof->openStgType="2071";
                            doSpecOrder(sof);
                        }else{
                            LOG(INFO)<<"drawbackPrice="+boost::lexical_cast<string>(techCls.drawbackPrice)+",not satisfy unlock condition,not process.";
                        }
                    }
                }else if(lastPrice < (lockPrice+techCls.lockWatchTickNums*tickPrice)&&lastPrice>=lockPrice){

                    if(techCls.maxPrice == 0){
                        LOG(INFO)<<"After lock position,price not touch the highest point of "+boost::lexical_cast<string>(techCls.lockWatchTickNums)+" tick.not process.";
                    }else if(techCls.maxPrice != 0){
                        LOG(INFO)<<"Price touched the highest point of "+boost::lexical_cast<string>(techCls.lockWatchTickNums)+" tick lower,and return back.";
                        if(techCls.drawbackPrice!=0 && lastPrice <= techCls.drawbackPrice){
                            LOG(INFO)<<"Unlock condition two:Price triggered long direction stop loss,begin to unlock.orderPrice="+boost::lexical_cast<string>(mkData->highestPrice);
                            sof->instrumentID=instrumentID;
                            sof->direction="1";
                            sof->offsetFlag="1";
                            if(techCls.isTestInviron){
                                sof->lastPrice=mkData->bidPrice;
                            }else{
                                InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                                sof->lastPrice=instInfo->LowerLimitPrice;
                                //sof->lastPrice=mkData->highestPrice;
                            }
                            sof->volume=userHoldPst.longTotalPosition;
                            sof->orderType="agg";
                            sof->openStgType="2071";
                            doSpecOrder(sof);
                        }else{
                            LOG(INFO)<<"drawbackPrice="+boost::lexical_cast<string>(techCls.drawbackPrice)+",not satisfy unlock condition,not process.";
                        }
                    }
                }else if(lastPrice >= techCls.lockFirstOpenPrice&&lastPrice<=lockPrice){
                    LOG(INFO)<<"price between lockFirstOpenPrice="+boost::lexical_cast<string>(techCls.lockFirstOpenPrice)+" and lockPrice="+boost::lexical_cast<string>(lockPrice)+",not process.";
                }else if(lastPrice < techCls.lockFirstOpenPrice&&techCls.lockFirstOpenPrice > mkData->askPrice){
                    LOG(INFO)<<"Unlock condition three:price step over lockFirstOpenPrice="+boost::lexical_cast<string>(techCls.lockFirstOpenPrice)+",askPrice="+boost::lexical_cast<string>(mkData->askPrice)+",stop loss.";
                    sof->instrumentID=instrumentID;
                    sof->direction="1";
                    sof->offsetFlag="1";
                    if(techCls.isTestInviron){
                        sof->lastPrice=mkData->bidPrice;
                    }else{
                        InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                        sof->lastPrice=instInfo->LowerLimitPrice;
                        //sof->lastPrice=mkData->highestPrice;
                    }
                    sof->volume=userHoldPst.longTotalPosition;
                    sof->orderType="0";
                    sof->openStgType="2071e";
                    doSpecOrder(sof);
                }else{
                    LOG(INFO)<<"NOT PROCESS.";
                }
                ///below has not modify,until here.
            }else if(techCls.priceStatus == "7"){//watch low price
                LOG(INFO)<<"now has been unlock,begin to watch low price ready to short unlock.minPrice="+boost::lexical_cast<string>(techCls.minPrice)+",relockPrice="
                           +boost::lexical_cast<string>(techCls.relockPrice)+",addMinPrice="+boost::lexical_cast<string>(techCls.additionMinPrice);
                double ATR15S_60C=techCls.trueKData15S->ATR*tickPrice;//get this techmetric from lib
                if(ATR15S_60C == 0){
                    ATR15S_60C=1.5*tickPrice;
                }
                ///if add addition fbna order
                /*
                if(techCls.additionMinPrice !=0 && lastPrice <= techCls.additionMinPrice){
                    LOG(INFO)<<"duration in unlock,price go down again and touch the minimal additionMinPrice="+boost::lexical_cast<string>(techCls.additionMinPrice)+",add additional fbna orders.";
                    if(techCls.isAddOrderOpen){
                        LOG(INFO)<<"additional FBNA order has been traded,not add new orders.";
                    }else{
                        int hopeVolume=getFBNAOrderVolume(tmpLongReverseList,"0");
                        LOG(INFO)<<"当前加仓数量:hopeVolume="
                                   +boost::lexical_cast<string>(hopeVolume);
                        //OrderInfo orderInfo;
                        if(existUntradeOrder("2052a",&orderInfo)){
                            if(orderInfo.price==lastPrice){
                                LOG(INFO) << "There are untrade order at this price,not process.";
                            }else{
                                LOG(INFO) << "There are untrade order exist,but not at this price,add new order.lastPrice="+boost::lexical_cast<string>(lastPrice)
                                             +",untradePrice="+boost::lexical_cast<string>(orderInfo.price);
                                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                                addinfo->openStgType="2052a";
                                addinfo->flag="fbna";
                                addNewOrderTrade(instrumentID,"0","0",lastPrice,hopeVolume,"0",addinfo);
                            }
                        }else{
                            LOG(INFO) << "There are no untrade order exist,add new order.";
                            AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                            addinfo->openStgType="2052a";
                            addinfo->flag="fbna";
                            addNewOrderTrade(instrumentID,"0","0",lastPrice,hopeVolume,"0",addinfo);
                        }
                    }
                }*/
                double metricPrice=(techCls.unlockPrice-6-techCls.rawPstStopLossTickNums*tickPrice-(techCls.lockTimes-1)*techCls.twoGap);
                //LOG(INFO)<<"ATR="+boost::lexical_cast<string>(ATR15S_60C)+",current maxPrice="+boost::lexical_cast<string>(techCls.maxPrice);
                LOG(INFO)<<"unlockprice="+boost::lexical_cast<string>(techCls.unlockPrice)+",rawPstTicks="+boost::lexical_cast<string>(techCls.rawPstStopLossTickNums)+",lockTimes="
                           +boost::lexical_cast<string>(techCls.lockTimes)+",towGap="+boost::lexical_cast<string>(techCls.twoGap)+";metric="
                           +boost::lexical_cast<string>(metricPrice);
                if(lastPrice <= metricPrice){
                    double tmpRawSLP=mkData->askPrice + tickPrice;
                    if(techCls.rawStopLossPrice==0||(techCls.rawStopLossPrice > tmpRawSLP)){
                        double preRawslp = techCls.rawStopLossPrice;
                        techCls.rawStopLossPrice=tmpRawSLP;
                        LOG(INFO)<<"lastprice has over metric,set raw stop losss price from "+boost::lexical_cast<string>(preRawslp)+ " to "+boost::lexical_cast<string>(techCls.rawStopLossPrice);
                    }
                }else if(lastPrice > metricPrice&&lastPrice < techCls.relockPrice){
                    LOG(INFO)<<"Not trigger,price="+boost::lexical_cast<string>(lastPrice)+" is still between stop loss price="+boost::lexical_cast<string>(metricPrice)+" and relockPrice="+
                               boost::lexical_cast<string>(techCls.relockPrice)+"rawStopLossPrice="+boost::lexical_cast<string>(techCls.rawStopLossPrice)+",not process.";
                }else if(lastPrice >= techCls.relockPrice&&techCls.relockPrice != 0){
                    LOG(INFO)<<"WARNING:relock position!!!!After unlock,price goes up again,and trigger relock condition.lastPrice("+boost::lexical_cast<string>(lastPrice)+") >= relockPrice("
                               +boost::lexical_cast<string>(techCls.relockPrice)+").";
                    LOG(INFO)<<"before relock,check if all of last unlock order is traded.if not then action.";
                    if(existUntradeOrder("2071",NULL)){
                        tryAllOrderAction(instrumentID);
                    }
                    //add max lock times
                    if(techCls.lockTimes >= 2){
                        LOG(INFO)<<"lock times over metric 2,close all trade.";
                        AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                        addinfo->openStgType="2077";
                        if(userHoldPst.longTotalPosition>0){
                            addNewOrderTrade(instrumentID,"1","1",mkData->bidPrice,userHoldPst.longTotalPosition,"agg",addinfo);
                        }
                        if(userHoldPst.shortTotalPosition>0){
                            addNewOrderTrade(instrumentID,"0","1",mkData->askPrice,userHoldPst.shortTotalPosition,"agg",addinfo);
                        }
                    }else{
                        sof->instrumentID=instrumentID;
                        sof->direction="0";
                        sof->offsetFlag="0";
                        if(techCls.isTestInviron){
                            sof->lastPrice=mkData->askPrice;
                        }else{
                            InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                            sof->lastPrice=instInfo->UpperLimitPrice;
                            //sof->lastPrice=mkData->lowestPrice;
                        }
                        sof->volume=userHoldPst.shortTotalPosition-userHoldPst.longTotalPosition;
                        sof->orderType="agg";
                        sof->openStgType="2062";
                        doSpecOrder(sof);
                    }

                }
                ///seprate judge
                if(techCls.rawStopLossPrice != 0 && lastPrice >= techCls.rawStopLossPrice){
                    LOG(INFO)<<"Unlock condition six:Price triggered short direction stop loss,begin to unlock.price over rawStopLossPrice="+boost::lexical_cast<string>(techCls.rawStopLossPrice);
                    sof->instrumentID=instrumentID;
                    sof->direction="0";
                    sof->offsetFlag="1";
                    if(techCls.isTestInviron){
                        sof->lastPrice=mkData->askPrice;
                    }else{
                        InstrumentInfo* instInfo =getInstrumentInfo(instrumentID);
                        sof->lastPrice=instInfo->UpperLimitPrice;
                        //sof->lastPrice=mkData->lowestPrice;
                    }
                   // sof->lastPrice=lastPrice;
                    sof->volume=userHoldPst.shortTotalPosition;
                    sof->orderType="0";
                    sof->openStgType="2081";
                    doSpecOrder(sof);
                }
                ///seprate judge


                ////
                /*
                if(lastPrice >= (techCls.unlockPrice + techCls.watchUnlockAnotherATRNums*ATR15S_60C)){
                    if(techCls.maxPrice == 0){
                        techCls.maxPrice = lastPrice;
                        LOG(INFO)<<"begin to initialize maxPrice="+boost::lexical_cast<string>(techCls.maxPrice);
                    }else if(techCls.maxPrice < lastPrice){
                        double tmprice = techCls.maxPrice;
                        techCls.maxPrice = lastPrice;
                        LOG(INFO)<<"Price become even more higher,set maxPrice from "+boost::lexical_cast<string>(tmprice)+" to "
                                   +boost::lexical_cast<string>( techCls.maxPrice);
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
                        }else{
                            LOG(INFO)<<"price go back to "+boost::lexical_cast<string>(lastPrice)+",not satisfy unlock condition,not process.";
                        }
                    }
                }else{
                    LOG(INFO)<<"Not trigger lastPrice="+boost::lexical_cast<string>(lastPrice) +" >= (techCls.unlockPrice + n*ATR15S_60C)="
                               +boost::lexical_cast<string>(techCls.unlockPrice)+"+"
                               +boost::lexical_cast<string>(ATR15S_60C*techCls.watchUnlockAnotherATRNums)+")";
                }
                ///add when price not over atr
                if(lastPrice < (techCls.unlockPrice + techCls.watchUnlockAnotherATRNums*ATR15S_60C)){

                    if(techCls.maxPrice!=0){
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
                        }else{
                            LOG(INFO)<<"price go back to "+boost::lexical_cast<string>(lastPrice)+",not satisfy unlock condition,not process.";
                        }
                    }
                }
                ///another method to be kicked out
                if((lastPrice - techCls.unlockPrice) >= techCls.timesOfStopLoss*techCls.twoGap*tickPrice){
                    LOG(INFO)<<"Unlock condition six:Price triggered long direction stop loss,begin to unlock.price over "+boost::lexical_cast<string>(techCls.timesOfStopLoss)+" times * twogap.";
                    sof->instrumentID=instrumentID;
                    sof->direction="1";
                    sof->offsetFlag="1";
                    sof->lastPrice=lastPrice;
                    sof->volume=userHoldPst.longTotalPosition;
                    sof->orderType="0";
                    sof->openStgType="2081";
                    doSpecOrder(sof);
                }

                else if((techCls.unlockPrice - lastPrice) >= techCls.relockATRNums*ATR15S_60C){
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
                        doSpecOrder(sof);
                    }

                }*/


            }else{
                LOG(INFO)<<"priceStatus="+techCls.priceStatus+",this kind of status not define.";
            }
        }

    }
}
void waitForConfirm(MarketData *pDepthMarketData){
    double lastPrice = pDepthMarketData->lastPrice;
    string instrumentID = pDepthMarketData->instrumentID;
    if(techCls.stgStatus == "10"){
        LOG(INFO)<<"First step into main direction watching status,we will transfer all position to reverseList.allTradeList="+boost::lexical_cast<string>(allTradeList.size())+",reserveList="
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
        techCls.limit[1]=lastPrice+1*tmpMax;
        techCls.limit[0]=lastPrice-1*tmpMax;
        LOG(INFO)<<"The first tick of main direction watching status.lastPrice="+boost::lexical_cast<string>(lastPrice)+",maxtr="+boost::lexical_cast<string>(tmpMax)+",compute dual parameters.limit=["+boost::lexical_cast<string>(techCls.limit[0])+","
                +boost::lexical_cast<string>(techCls.limit[1])+"]";
        techCls.stgStatus="11";
        techCls.beginK15s = true;//from here start 15s k line.
        LOG(INFO)<<"After transfer all position to reverseList.allTradeList="+boost::lexical_cast<string>(allTradeList.size())+",reserveList="
                   +boost::lexical_cast<string>(longReverseList.size())+",begin to close all position.";
        if(existUntradeOrder("4000",NULL)){
            LOG(INFO) << "There are orders not fully traded,waiting.";
        }else{
            LOG(INFO) << "There are not orders untraded,Begin Close all.";
            AdditionOrderInfo* addinfo=new AdditionOrderInfo();
            addinfo->openStgType="4000";
            InstrumentInfo* strInfo = getInstrumentInfo(instrumentID);
            if(userHoldPst.longTotalPosition>0){
                double insertPrice=0;
                if(techCls.isTestInviron){
                    insertPrice = pDepthMarketData->bidPrice - strInfo->PriceTick;
                }else{
                    insertPrice = strInfo->LowerLimitPrice;
                }
                addNewOrderTrade(instrumentID,"1","1",insertPrice,userHoldPst.longTotalPosition,"0",addinfo);
            }
            if(userHoldPst.shortTotalPosition>0){
                double insertPrice=0;
                if(techCls.isTestInviron){
                    insertPrice = pDepthMarketData->askPrice + strInfo->PriceTick;
                }else{
                    insertPrice = strInfo->UpperLimitPrice;
                }
                addNewOrderTrade(instrumentID,"0","1",insertPrice,userHoldPst.shortTotalPosition,"0",addinfo);
            }
        }
        //like coverYourAss()
        {
            userHoldPst.allPstClean="2";
            LOG(INFO)<<"All long position has been cleaned.do something of reseting.";
            tmpLongReverseList.clear();
            longReverseList.clear();
            //tryAllOrderAction("");
            techCls.stageTick=-1;
            techCls.stageStopLossPrice=0;
            techCls.priceStatus="0";
            //techCls.stgStatus="0";
            techCls.firstOpenPrice=0;
            techCls.minPrice=0;
            techCls.maxPrice=0;
            techCls.firstOpenKLineType="0";
            techCls.lockTimes=0;
            Strategy::Kdata tmp=techCls.KData_15s.back();
            techCls.KData_15s.clear();
            techCls.KData_15s.push_back(tmp);
            LOG(INFO)<<"current k 15s line size="+boost::lexical_cast<string>(techCls.KData_15s.size());
            closeProtectOrders();

            cleanRealInfo();
            techCls.unlockPL=0;
            techCls.stopLossPrice=0;
            techCls.firstOpenPriceNormal=0;

        }
    }else if(techCls.stgStatus == "11"){
        LOG(INFO)<<"First step into main direction watching status,next broken will be true direction.limit=["+boost::lexical_cast<string>(techCls.limit[0])+","
                +boost::lexical_cast<string>(techCls.limit[1])+"]";
        double tickPrice = getPriceTick(pDepthMarketData->instrumentID);
        if(lastPrice >= techCls.limit[1] && lastPrice >= (techCls.trueKData15M->ma5 + techCls.overMATickNums*tickPrice)){
            string tmpdir = techCls.mainDirection;
            techCls.mainDirection = "0";
            LOG(INFO)<<"current price="+boost::lexical_cast<string>(lastPrice)+",over upper limit.limit=["+boost::lexical_cast<string>(techCls.limit[0])+","
                    +boost::lexical_cast<string>(techCls.limit[1])+"];ma5 +n*tick="+boost::lexical_cast<string>(techCls.trueKData15M->ma5 + techCls.overMATickNums*tickPrice)+",reset main direction from "+tmpdir+" to "+techCls.mainDirection;
            if(existUntradeOrder("3000",NULL)){
                LOG(INFO) << "There are orders not fully traded,waiting.";
            }else{
                LOG(INFO) << "Begin to order insert first open orders.range=["+boost::lexical_cast<string>(techCls.limit[0])+","
                        +boost::lexical_cast<string>(techCls.limit[1])+"],lastPrice="+boost::lexical_cast<string>(lastPrice);
                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                addinfo->openStgType="3000";
                addNewOrderTrade(instrumentID,"0","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
            }
        }else if(lastPrice <= techCls.limit[0] && lastPrice <= (techCls.trueKData15M->ma5 - techCls.overMATickNums*tickPrice)){
            string tmpdir = techCls.mainDirection;
            techCls.mainDirection = "1";
            LOG(INFO)<<"current price="+boost::lexical_cast<string>(lastPrice)+",over lower limit.limit=["+boost::lexical_cast<string>(techCls.limit[0])+","
                    +boost::lexical_cast<string>(techCls.limit[1])+"];ma5 +n*tick="+boost::lexical_cast<string>(techCls.trueKData15M->ma5 + techCls.overMATickNums*tickPrice)+",reset main direction from "+tmpdir+" to "+techCls.mainDirection;
            if(existUntradeOrder("3000",NULL)){
                LOG(INFO) << "There are orders not fully traded,waiting.";
            }else{
                LOG(INFO) << "Begin to order insert first open orders.range=["+boost::lexical_cast<string>(techCls.limit[0])+","
                        +boost::lexical_cast<string>(techCls.limit[1])+"],lastPrice="+boost::lexical_cast<string>(lastPrice);
                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                addinfo->openStgType="3000";
                addNewOrderTrade(instrumentID,"1","0",lastPrice,techCls.firstMetricVolume,"0",addinfo);
            }
        }else{
            LOG(INFO)<<"price still between dual trust limit=["+boost::lexical_cast<string>(techCls.limit[0])+","
                    +boost::lexical_cast<string>(techCls.limit[1])+"],lastPrice="+boost::lexical_cast<string>(lastPrice);
        }
    }else if(techCls.stgStatus == "120"||techCls.stgStatus == "121"){
        if(allTradeList.size() > 0){
            for(list<WaitForCloseInfo*>::iterator wfIt = allTradeList.begin();wfIt!=allTradeList.end();){
                //tmpLongReverseList->push_back(*wfIt);
                longReverseList.push_back(*wfIt);
                wfIt = allTradeList.erase(wfIt);
            }
        }
        LOG(INFO)<<"main direction change too much,close all position.";
        if(techCls.stgStatus == "120"){
            if(existUntradeOrder("4010",NULL)){
                LOG(INFO) << "There are orders not fully traded,waiting.";
            }else{
                LOG(INFO) << "There are not orders untraded,Begin Close all.";
                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                addinfo->openStgType="4010";
                InstrumentInfo* strInfo = getInstrumentInfo(instrumentID);
                if(userHoldPst.longTotalPosition>0){
                    double insertPrice=0;
                    if(techCls.isTestInviron){
                        insertPrice = pDepthMarketData->bidPrice - strInfo->PriceTick;
                    }else{
                        insertPrice = strInfo->LowerLimitPrice;
                    }
                    addNewOrderTrade(instrumentID,"1","1",insertPrice,userHoldPst.longTotalPosition,"0",addinfo);
                    //addNewOrderTrade(instrumentID,"1","1",strInfo->LowerLimitPrice,userHoldPst.longTotalPosition,"0",addinfo);
                }
                if(userHoldPst.shortTotalPosition>0){
                    double insertPrice=0;
                    if(techCls.isTestInviron){
                        insertPrice = pDepthMarketData->askPrice + strInfo->PriceTick;
                    }else{
                        insertPrice = strInfo->UpperLimitPrice;
                    }
                    addNewOrderTrade(instrumentID,"0","1",insertPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                    //addNewOrderTrade(instrumentID,"0","1",strInfo->UpperLimitPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                }
            }
        }else if(techCls.stgStatus == "121"){
            if(existUntradeOrder("4011",NULL)){
                LOG(INFO) << "There are orders not fully traded,waiting.";
            }else{
                LOG(INFO) << "There are not orders untraded,Begin Close all.";
                AdditionOrderInfo* addinfo=new AdditionOrderInfo();
                addinfo->openStgType="4011";
                InstrumentInfo* strInfo = getInstrumentInfo(instrumentID);
                if(userHoldPst.longTotalPosition>0){
                    double insertPrice=0;
                    if(techCls.isTestInviron){
                        insertPrice = pDepthMarketData->bidPrice - strInfo->PriceTick;
                    }else{
                        insertPrice = strInfo->LowerLimitPrice;
                    }
                    addNewOrderTrade(instrumentID,"1","1",insertPrice,userHoldPst.longTotalPosition,"0",addinfo);
                    //addNewOrderTrade(instrumentID,"1","1",strInfo->LowerLimitPrice,userHoldPst.longTotalPosition,"0",addinfo);
                }
                if(userHoldPst.shortTotalPosition>0){
                    double insertPrice=0;
                    if(techCls.isTestInviron){
                        insertPrice = pDepthMarketData->askPrice + strInfo->PriceTick;
                    }else{
                        insertPrice = strInfo->UpperLimitPrice;
                    }
                    addNewOrderTrade(instrumentID,"0","1",insertPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                    //addNewOrderTrade(instrumentID,"0","1",strInfo->UpperLimitPrice,userHoldPst.shortTotalPosition,"0",addinfo);
                }
            }
        }


    }
}

//LogMsg *mklogmsg = new LogMsg();
void metricProcesserForSingleThread(MarketData *mkData) {
    try {
        //开始时间
        boost::posix_time::ptime time1 = getCurrentTimeByBoost();
        //boost::recursive_mutex::scoped_lock SLock3(techMetric_mtx);
        double lastPrice = mkData->lastPrice;
        if(mkData->updateTime=="20:59:00"){
            return;
        }
        //开始时间
        //boost::posix_time::ptime startTime = getCurrentTimeByBoost();
        techCls.RunMarketData(mkData);
        //boost::posix_time::ptime endTime = getCurrentTimeByBoost();
        //int heatBeatSecond = getTimeInterval(startTime,endTime,  "t");
        //cout<<"RunMarketData="<<heatBeatSecond<<endl;


        string msg="businessType=wtm_6001;tradingDay="+boost::lexical_cast<string>(tradingDay)+";logTime="+currTime + ";logType=2;mainDirection="+techCls.mainDirection+";stgStatus="+techCls.stgStatus+";priceStatus="+techCls.priceStatus+";lastPrice="+boost::lexical_cast<string>(lastPrice);

        sendMSG(msg);
        //return;
        LOG(INFO) <<"time=" +currTime+",mainDirection="+techCls.mainDirection+",stgStatus="+techCls.stgStatus+",priceStatus="+techCls.priceStatus+",lastPrice="+boost::lexical_cast<string>(lastPrice)+",15s k line size="+boost::lexical_cast<string>(techCls.KData_15s.size());
        //strategy
        if(techCls.mainDirection == "0"){
            //开始时间
            //boost::posix_time::ptime startTime2 = getCurrentTimeByBoost();
            longDirectionTrade(mkData);
            //boost::posix_time::ptime endTime2 = getCurrentTimeByBoost();
            //int heatBeatSecond = getTimeInterval(startTime2,endTime2,  "t");
            //cout<<"longDirectionTrade="<<heatBeatSecond<<endl;

        }else if(techCls.mainDirection == "1"){
            //开始时间
            //boost::posix_time::ptime startTime2 = getCurrentTimeByBoost();
            shortDirectionTrade(mkData);
            //boost::posix_time::ptime endTime2 = getCurrentTimeByBoost();
            //int heatBeatSecond = getTimeInterval(startTime2,endTime2,  "t");
            //cout<<"shortDirectionTrade="<<heatBeatSecond<<endl;

        }else if(techCls.mainDirection == "4"){
            //开始时间
            //boost::posix_time::ptime startTime2 = getCurrentTimeByBoost();
            waitForConfirm(mkData);
            //boost::posix_time::ptime endTime2 = getCurrentTimeByBoost();
            //int heatBeatSecond = getTimeInterval(startTime2,endTime2,  "t");
            //cout<<"waitForConfirm="<<heatBeatSecond<<endl;

        }else{
            LOG(ERROR)<<"ERROR::undefined main direction type.maindir="+techCls.mainDirection;
        }
        //boost::recursive_mutex::scoped_lock SLock4(unique_mtx);//锁定
        //结束时间
        boost::posix_time::ptime time2 = getCurrentTimeByBoost();
        int seconds = getTimeInterval(time1, time2, "t");
        /*
        //时间间隔
        if(isLogout){
            string t1 = tradingDayT + " " + frontPrice->updateTime;
            string t2 = tradingDayT + " " + mkData->updateTime;
            boost::posix_time::ptime int1 = changeStringToTime(t1);
            boost::posix_time::ptime int2 = changeStringToTime(t2);
            int elapseTime = getTimeInterval(int1, int2, "s");
            LOG(INFO) << "当前数列长度=" + boost::lexical_cast<string>(pgDataSeq->size()) + ",时间间隔=" + boost::lexical_cast<string>(elapseTime) + ",最大值as=" + boost::lexical_cast<string>(tm->asHighestGap) + ",ds=" + boost::lexical_cast<string>(tm->dsHighestGap)
                + ";最小值as=" + boost::lexical_cast<string>(tm->asLowestGap) + ",ds=" + boost::lexical_cast<string>(tm->dsLowestGap) + ",平均值as=" + boost::lexical_cast<string>(tm->asMAGap) + ",ds=" + boost::lexical_cast<string>(tm->dsMAGap)
                + ";处理时长=" + boost::lexical_cast<string>(seconds);

            stg = "businessType=wtm_4;insComKey=" + insComKey + ";" + "tradingDay=" + tradingDayT
                + ";" + "updateTime=" + boost::lexical_cast<string>(updateTime)
                + ";processTime=" + boost::lexical_cast<string>(seconds) + ";type=metric;seq=" + boost::lexical_cast<string>(time2)+";systemID=" + systemID;
            logmsg->setMsg(stg);
            networkTradeQueue.push(logmsg);
        }*/
    }catch (const runtime_error &re) {
        cerr << re.what() << endl;
        LOG(ERROR) << re.what();
    }
    catch (exception* e) {
        cerr << e->what() << endl;
        LOG(ERROR) << e->what();
    }

}

