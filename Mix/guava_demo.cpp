#include <iostream>
#include "guava_demo.h"
#include "property.h"
#include "TimeProcesser.h"


using std::cout;
using std::cin;
using std::endl;
extern unordered_map<string, InstrumentInfo*> instruments;			//合约信息
extern unordered_map<string, MarketData*> instrinfo;//market data
extern char **ppInstrumentID;// 行情订阅列表
extern string slLocalIP;
extern string slGuavaIP;
extern int slGuavaPort;
extern int start_process;
extern Strategy techCls;
guava_demo::guava_demo(void)
{
	m_quit_flag = false;
}


guava_demo::~guava_demo(void)
{
}


void guava_demo::run()
{
	input_param();

	bool ret = init();
	if (!ret)
	{
        LOG(ERROR)<<"ERROR::SHENGLI GUAVA MARKETDATA NOT CONNECT!!!!";

        return;
    }else{
        string msg="INIT SHENGLI GUAVA MARKETDATA OK.";
        LOG(INFO)<<"msg";
        cout<<msg<<endl;
    }
}


void guava_demo::input_param()
{
    string local_ip = slLocalIP;
    cout << "SHENGLI GUAVA LOCALIP: " << local_ip << endl;
    string guava_ip = slGuavaIP;
    int guava_port = slGuavaPort;
    string guava_local_ip = local_ip;
    int guava_local_port = 23501;


    cout << "mk multicast ip: " << guava_ip << endl;
    cout << "mk multicast port: " << guava_port << endl;
    cout << "mk multicast local ip: " << guava_local_ip << endl;
    cout << "mk multicast local port: " << guava_local_port << endl;

	/// add by zhou.hu review 2014/4/24 设置具体的参数
	multicast_info temp;

	/// add by zhou.hu review 2014/4/23 中金行情通道
	memset(&temp, 0, sizeof(multicast_info));
	
    strcpy(temp.m_remote_ip, guava_ip.c_str());
    temp.m_remote_port = guava_port;
    strcpy(temp.m_local_ip, guava_local_ip.c_str());
    temp.m_local_port = guava_local_port;

	m_cffex_info = temp;
}


bool guava_demo::init()
{
	return m_guava.init(m_cffex_info, this);
}

void guava_demo::close()
{
	m_guava.close();
}

void guava_demo::pause()
{
	string str_temp;

	printf("\n按任意字符继续(输入q将退出):\n");
	cin >> str_temp;
	if (str_temp == "q")
	{
		m_quit_flag = true;
	}
}
double guavaUpperPrice=0;
double guavaLowerPrice=0;
int guavaamount=0;
void guava_demo::on_receive_nomal(guava_udp_normal* marketdata)
{
    if (start_process == 0) {
        return;
    }
    guavaamount+=1;
    if(guavaamount%100==0){
        LOG(INFO)<<"guavaamount="+boost::lexical_cast<string>(guavaamount);
    }
    //assemble marketdata
    string instrumentID=boost::lexical_cast<string>(marketdata->m_symbol);
    int volume = marketdata->m_last_share;//成交量
    double turnover = marketdata->m_total_value;
    double multiply = getMultipler(instrumentID);
    string updateTime=string(marketdata->m_update_time);
    int millisecond=marketdata->m_millisecond;
    double bidPrice=marketdata->m_bid_px;
    double askPrice=marketdata->m_ask_px;
    double lastPrice=marketdata->m_last_px;
    //double tickPrice = getPriceTick(instrumentID);
    if(guavaUpperPrice < lastPrice){
        guavaUpperPrice = lastPrice;
    }
    if(guavaLowerPrice > lastPrice){
        guavaLowerPrice = lastPrice;
    }
    MarketData* marketdatainfo;
    unordered_map<string, MarketData*>::iterator it = instrinfo.find(instrumentID);
    if(it == instrinfo.end()){
        marketdatainfo = new MarketData();//保存当前行情数据
        marketdatainfo->updateTime = updateTime;
        //strcpy(marketdatainfo->updateTime_char,marketdata->UpdateTime);
        marketdatainfo->instrumentID = instrumentID;
        marketdatainfo->volume = volume;
        marketdatainfo->bidPrice = bidPrice;
        marketdatainfo->askPrice = askPrice;
        marketdatainfo->lastPrice = lastPrice;
        marketdatainfo->highestPrice = guavaUpperPrice;
        marketdatainfo->lowestPrice = guavaLowerPrice;
        marketdatainfo->turnover = turnover;
        if(volume !=0 && multiply != 0){
            marketdatainfo->simPrice = turnover/volume/multiply;
            //marketdatainfo->simPrice = turnover/volume;
        }else{
            LOG(ERROR) << "OnRtnDepthMarketData:volume or tickPrice is zero!!!instrumentID="+instrumentID;
            return ;
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
        marketdatainfo->updateTime = updateTime;
        //strcpy(marketdatainfo->updateTime_char,marketdata->UpdateTime);
        marketdatainfo->bidPrice = bidPrice;
        marketdatainfo->askPrice = askPrice;
        marketdatainfo->lastPrice = lastPrice;
        marketdatainfo->highestPrice = guavaUpperPrice;
        marketdatainfo->lowestPrice = guavaLowerPrice;
        if(multiply != 0&&tmpVolume != 0){
            //marketdatainfo->simPrice = tmpTurnover/tmpVolume;//multiply needed in shfe
            marketdatainfo->simPrice = tmpTurnover/tmpVolume/multiply;
        }
    }
    string fastJ=marketdatainfo->updateTime+boost::lexical_cast<string>(millisecond);
    string tmpmkdata=
            "ga;instrumentID="+marketdatainfo->instrumentID+";"+
            fastJ+
            ";bidPrice1="+boost::lexical_cast<string>(marketdatainfo->bidPrice)+
            ";bidVolume1="+boost::lexical_cast<string>(marketdata->m_bid_share)+
            ";askPrice1="+boost::lexical_cast<string>(marketdatainfo->askPrice)+
            ";askVolume1="+boost::lexical_cast<string>(marketdata->m_ask_share)+
            ";lastPrice=" + boost::lexical_cast<string>(marketdatainfo->lastPrice) +
            ";volume="+boost::lexical_cast<string>(marketdatainfo->volume)+
            ";turnover="+boost::lexical_cast<string>(marketdatainfo->turnover)+
            ";uptime=" + boost::lexical_cast<string>(millisecond);

    //cout<<tmpmkdata<<endl;
    if(techCls.isTestInviron){
        LOG(INFO)<<tmpmkdata;
        cout<<tmpmkdata<<endl;
    }

    if(whichMarketDataFast("slGuava",fastJ)){
        if(techCls.isTestInviron){
            return;
        }
        metricProcesserForSingleThread(marketdatainfo);
    }
    //string str_body = to_string(data);

    //cout << "receive nomal msg: " << str_body << endl;
}

string guava_demo::to_string(guava_udp_normal* marketdata)
{
	char buff[8192];

	/// 针对于大商所做出的修改   lisen  2016/10/17
    long long	dd_pos_temp = *(long long*)(&marketdata->m_total_pos);
	double		dd_pos		= static_cast<double>(dd_pos_temp);
    /*
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%u,%d,%d,%s,%s,%d,%d,%.4f,%d,%.4f,%.4f,%.4f,%d,%.4f,%d"
        ,marketdata->m_sequence
        ,(int)(marketdata->m_exchange_id)
        ,(int)(marketdata->m_channel_id)
        ,marketdata->m_symbol
        ,marketdata->m_update_time
        , marketdata->m_millisecond
        , (int)(marketdata->m_quote_flag)

        ,marketdata->m_last_px
        ,marketdata->m_last_share
        ,marketdata->m_total_value
		//,ptr->m_total_pos
		,dd_pos
        ,marketdata->m_bid_px
        ,marketdata->m_bid_share
        ,marketdata->m_ask_px
        ,marketdata->m_ask_share
		);
*/
    /**
     * @brief str
     */


	string str = buff;
	return str;
}





