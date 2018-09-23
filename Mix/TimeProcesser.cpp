
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

void metricProcesserForSingleThread(MarketData *mkData) {
    try {
        PriceGap* priceGap;
        list<PriceGapMarketData*>* pgDataSeq;
        //开始时间
        boost::posix_time::ptime time1 = getCurrentTimeByBoost();
        //boost::recursive_mutex::scoped_lock SLock3(techMetric_mtx);
        unordered_map<string, vector<string>>::iterator insIt = instr_map.find(mkData->instrumentID);
        if (insIt == instr_map.end()) {
            LOG(INFO) << "metricProcesser，未查询到合约对应的合约列表,instrumentID=" + mkData->instrumentID;
            return;
        }
        if(ctm.kIndex_15s==ctm.mirrorIndex15s){

        }
        vector<string> insVec = insIt->second;
        for (unsigned int i = 0, j = insVec.size(); i < j;i++) {
            string pdInstrumentID = insVec[i];
            string insComKey = getComInstrumentKey(mkData->instrumentID,pdInstrumentID);
            //获得priceGap
            priceGap = getPriceGap(insComKey);

            unordered_map<string, TechMetric*>::iterator tit = techMetricMap.find(insComKey);
            if (tit == techMetricMap.end()) {
                LOG(INFO) << "metricProcesser，未查询到合约技术指标信息,新建信息,insComKey=" + insComKey;
                list<PriceGapMarketData*>* tmpkdseq = new list<PriceGapMarketData*>();
                list<double>* tmpmaseq = new list<double>();
                TechMetric* tm = new TechMetric();
                //tmpkdseq->emplace_back(mkData);
                tm->pgDataSeq = tmpkdseq;
                tm->MAGapSeq = tmpmaseq;
                techMetricMap[insComKey] = tm;
                initOverTickNums(tm, insComKey);
                return;
            }
            TechMetric* tm = tit->second;
            //当前合约信息
            int volume = mkData->volume;
            string updateTime = mkData->updateTime;
            double lastPrice = mkData->lastPrice;
            double bidPrice = mkData->bidPrice;
            double askPrice = mkData->askPrice;
            //double turnover = mkData->turnover;
            //double highestPrice = mkData->highestPrice;
            //double lowestPrice = mkData->lowestPrice;
            //配对合约信息
            double pd_lastPrice = 0;
            double pd_bidPrice = 0;
            double pd_askPrice = 0;
            int pd_volume = 0;
            {
                //boost::recursive_mutex::scoped_lock SLock1(mkdata_mtx);//加锁处理
                unordered_map<string, MarketData*>::iterator mkdata_it = marketdata_map.find(pdInstrumentID);//找到对应配对合约的行情价格
                if (mkdata_it != marketdata_map.end()) {
                    MarketData* pd_mkdata = mkdata_it->second;
                    pd_lastPrice = pd_mkdata->lastPrice;
                    pd_bidPrice = pd_mkdata->bidPrice;
                    pd_askPrice = pd_mkdata->askPrice;
                    pd_volume = pd_mkdata->volume;
                    if (pd_lastPrice == 0) {
                        return;
                    }
                } else {
                    LOG(ERROR) << pdInstrumentID + "合约价格信息不存在";
                    return;
                }
            }
            PriceGapMarketData* tmpPGData = new PriceGapMarketData();//保存，用于后续计算
            double tmpGap = 0;
            //double asGap = 0;//正套
            //double dsGap = 0;//反套
            double asOpenGap = 0;//正套
            double dsOpenGap = 0;//反套
            double asCloseGap = 0;//正套
            double dsCloseGap = 0;//反套
            if (mkData->instrumentID > pdInstrumentID) {//当前合约为远月合约
                //tmpGap = pd_askPrice - bidPrice;//近月-远月
                tmpGap = pd_lastPrice - lastPrice;//近月-远月
                asOpenGap = pd_askPrice - bidPrice;//买近月，卖远月
                asCloseGap = pd_bidPrice - askPrice;//卖近月，买远月
                dsOpenGap = pd_bidPrice - askPrice;//买远月，卖近月
                dsCloseGap = pd_askPrice - bidPrice;//卖远月，买近月
                tmpPGData->lastInstrumentID = pdInstrumentID;
                tmpPGData->lastInsAskPrice = pd_askPrice;
                tmpPGData->lastInsBidPrice = pd_bidPrice;
                tmpPGData->lastInsVolume = pd_volume;

                tmpPGData->forwardInstrumentID = mkData->instrumentID;
                tmpPGData->forwardInsAskPrice = askPrice;
                tmpPGData->forwardInsBidPrice = bidPrice;
                tmpPGData->forwardInsVolume = volume;

            } else {
                //tmpGap = askPrice - pd_bidPrice;//近月-远月
                tmpGap = lastPrice - pd_lastPrice;//近月-远月
                asOpenGap = askPrice - pd_bidPrice;//买近月，卖远月
                dsOpenGap = bidPrice - pd_askPrice;//买远月，卖近月
                asCloseGap = bidPrice - pd_askPrice;//买近月，卖远月
                dsCloseGap = askPrice - pd_bidPrice;//买远月，卖近月
                tmpPGData->forwardInstrumentID = pdInstrumentID;
                tmpPGData->forwardInsAskPrice = pd_askPrice;
                tmpPGData->forwardInsBidPrice = pd_bidPrice;
                tmpPGData->forwardInsVolume = pd_volume;

                tmpPGData->lastInstrumentID = mkData->instrumentID;
                tmpPGData->lastInsAskPrice = askPrice;
                tmpPGData->lastInsBidPrice = bidPrice;
                tmpPGData->lastInsVolume = volume;
            }
            if (volume >= pd_volume) {//远月为活跃合约
                tm->activeInstrumentID = mkData->instrumentID;
            } else {
                tm->activeInstrumentID = pdInstrumentID;
            }
            //tmpPGData->asGapPrice = asGap;
            //tmpPGData->dsGapPrice = dsGap;
            tmpPGData->asOpenGap = asOpenGap;
            tmpPGData->asCloseGap = asCloseGap;
            tmpPGData->dsOpenGap = dsOpenGap;
            tmpPGData->dsCloseGap = dsCloseGap;
            tmpPGData->gapPrice = tmpGap;
            tmpPGData->insComKey = insComKey;
            tmpPGData->des = getDes(tmpPGData);
            tmpPGData->updateTime = updateTime;
            //数据列表
            pgDataSeq = tm->pgDataSeq;
            if (pgDataSeq->size() >= priceGap->timeInterval_ma) {
                if(isLogout){
                    LOG(INFO) << "priceGap->timeInterval_ma=" + boost::lexical_cast<string>(priceGap->timeInterval_ma) + ";asOpenGap=" + boost::lexical_cast<string>(asOpenGap) + ",asCloseGap=" + boost::lexical_cast<string>(asCloseGap) + ",dsOpenGap=" + boost::lexical_cast<string>(dsOpenGap)
                                                + ",dsCloseGap=" + boost::lexical_cast<string>(dsCloseGap) + ",lastInstrumentID=" + boost::lexical_cast<string>(tmpPGData->lastInstrumentID) +",lastInsBidPrice=" + boost::lexical_cast<string>(tmpPGData->lastInsBidPrice) + ",lastInsAskPrice=" + boost::lexical_cast<string>(tmpPGData->lastInsAskPrice)
                                                + ",forwardInstrumentID=" + boost::lexical_cast<string>(tmpPGData->forwardInstrumentID) + ",forwardInsBidPrice=" + boost::lexical_cast<string>(tmpPGData->forwardInsBidPrice) + ",forwardInsAskPrice=" + boost::lexical_cast<string>(tmpPGData->forwardInsAskPrice);
                }
                //直接根据gap进行开平仓计算
                //replace queue for processStrategyForSingleThread
                //strategyQueue.push(tmpPGData);
                processStrategyForSingleThread(tmpPGData);
            }
            //保存数据,所有的行情数据
            string stg = "businessType=wtm_3;insComKey=" + insComKey + ";" + "gapPrice=" + boost::lexical_cast<string>(tmpPGData->gapPrice)
                + ";" + "tradingDay=" + boost::lexical_cast<string>(tradingDay)
                + ";" + "updateTime=" + boost::lexical_cast<string>(updateTime)
                + ";" + "asOpenGap=" + boost::lexical_cast<string>(tmpPGData->asOpenGap)
                + ";" + "dsOpenGap=" + boost::lexical_cast<string>(tmpPGData->dsOpenGap)
                + ";" + "asCloseGap=" + boost::lexical_cast<string>(tmpPGData->asCloseGap)
                + ";" + "dsCloseGap=" + boost::lexical_cast<string>(tmpPGData->dsCloseGap);
            LogMsg *logmsg = new LogMsg();
            logmsg->setMsg(stg);
            networkTradeQueue.push(logmsg);
            ///////////////////////////////////////////////////
            ///均值计算只在不同时间的时候计算，减少计算量
            ///////////////////////////////////////////////////
            ///
            /*
            if (tm->strTime == mkData->updateTime) {
                //LOG(INFO) << "时间重复，不计入平均值计算";
                continue;
            } else {
               // LOG(INFO) << "前一次时间记录=" + tm->strTime + ",当前时间=" + mkData->updateTime;
                //tm->strTime = mkData->updateTime;

            }
            */
            PriceGapMarketData* frontPrice;
            if (pgDataSeq->size() >= priceGap->timeInterval_ma) {//2倍的间隔数量，1秒2个tick
                frontPrice = pgDataSeq->front();
                pgDataSeq->pop_front();//删除第一个
                pgDataSeq->emplace_back(tmpPGData);
                tm->totalGapPrice -= frontPrice->gapPrice;
                tm->asTotalGap -= frontPrice->asOpenGap;
                tm->dsTotalGap -= frontPrice->dsOpenGap;
            } else {
                pgDataSeq->emplace_back(tmpPGData);
                frontPrice = pgDataSeq->front();
            }
            //计算时间价格

            ///////////////////////////////以下为指标计算/////////////////////////////
            /// \brief totalGapPrice
            ///

            tm->totalGapPrice += tmpPGData->gapPrice;
            tm->asTotalGap += tmpPGData->asOpenGap;
            tm->dsTotalGap += tmpPGData->dsOpenGap;

            //
            double tmpMaGapPrice = tm->totalGapPrice / pgDataSeq->size();
            double tmpmaGapPriceAS = tm->asTotalGap / pgDataSeq->size();
            double tmpmaGapPriceDS = tm->dsTotalGap / pgDataSeq->size();

            tm->maGap = tmpMaGapPrice;
            tm->asMAGap = tmpmaGapPriceAS;
            tm->dsMAGap = tmpmaGapPriceDS;
            //tm->asHighestGap = maxGapAS;
            //tm->asLowestGap = minGapAS;
            //tm->dsHighestGap = maxGapDS;
            //tm->dsLowestGap = minGapDS;
            /*
            if(insComKey == "jm1801-jm1805"){
                //compute ma std
                list<double>* tmpMAGapSeq = tm->MAGapSeq;
                if(tmpMAGapSeq->size() >= priceGap->timeInterval_ma){
                    double tmpMaGap = tmpMAGapSeq->front();
                    tmpMAGapSeq->pop_front();
                    tmpMAGapSeq->emplace_back(maGapPrice);
                    tm->maGapAmount -= tmpMaGap;
                    tm->maGapAmount += maGapPrice;
                }else{
                    tmpMAGapSeq->emplace_back(maGapPrice);
                    tm->maGapAmount += maGapPrice;
                }
                double maGapMean = tm->maGapAmount/tmpMAGapSeq->size();
                double sumOfGapMean = 0;
                for(list<double>::iterator it = tmpMAGapSeq->begin();it != tmpMAGapSeq->end();it++){
                    sumOfGapMean += (*it - maGapMean)*(*it - maGapMean);
                }
                double stdev = sqrt(sumOfGapMean/(tmpMAGapSeq->size()-1));
                tm->maGapSTDEV = stdev;
                //if product is jm
                if(stdev <= 0.3){
                    if(tm->isSTDVOverFLow){
                        tm->overMAGapTickNums = 1.8*2;
                        tm->downMAGapTickNums = 1.8*2;
                    }else{
                        tm->overMAGapTickNums = 2.5;
                        tm->downMAGapTickNums = 2.5;
                    }
                }else if(stdev > 0.3 && stdev <= 0.6){
                    if(tm->isSTDVOverFLow){
                        tm->overMAGapTickNums = 2.1*2;
                        tm->downMAGapTickNums = 2.1*2;
                    }else{
                        tm->overMAGapTickNums = 3.2;
                        tm->downMAGapTickNums = 3.2;
                    }
                }else if (stdev > 0.6 && stdev <= 0.9){
                    if(tm->isSTDVOverFLow){
                        tm->overMAGapTickNums = 2.5*2;
                        tm->downMAGapTickNums = 2.5*2;
                    }else{
                        tm->overMAGapTickNums = 4.1;
                        tm->downMAGapTickNums = 4.1;
                    }
                }else if(stdev > 0.9){
                    tm->overMAGapTickNums = 5.5;
                    tm->downMAGapTickNums = 5.5;
                    tm->isSTDVOverFLow = true;
                }
                LOG(INFO) << "JM gap std is " + boost::lexical_cast<string>(tm->maGapSTDEV) + ",over tick nums is "  +
                             boost::lexical_cast<string>(tm->overMAGapTickNums) + ",down tick nums is "  +
                             boost::lexical_cast<string>(tm->downMAGapTickNums) + ",len is "  + boost::lexical_cast<string>(tmpMAGapSeq->size()) +
                              ",flow is "  + boost::lexical_cast<string>(tm->isSTDVOverFLow);
            }*/
            //tm->lastTime
            //结束时间
            boost::posix_time::ptime time2 = getCurrentTimeByBoost();
            int seconds = getTimeInterval(time1, time2, "t");
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
            }

            stg = "businessType=wtm_4;insComKey=" + insComKey + ";" + "tradingDay=" + tradingDayT
                + ";" + "updateTime=" + boost::lexical_cast<string>(updateTime)
                + ";processTime=" + boost::lexical_cast<string>(seconds) + ";type=metric;seq=" + boost::lexical_cast<string>(time2)+";systemID=" + systemID;
            logmsg->setMsg(stg);
            networkTradeQueue.push(logmsg);
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

