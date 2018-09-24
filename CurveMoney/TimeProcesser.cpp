
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



