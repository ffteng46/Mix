#pragma once
#ifdef WIN32
#include <Windows.h>
typedef HMODULE		T_DLL_HANDLE;
#else
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "DataProcessor.h"
#include "property.h"
#include "TraderSpi.h"

//#include "src/ergti/adapter/ctpex_proxy_driver.h"

//#include "src/ergti/util/ctpex_proxy_util.h"
//#include "src/ergti/types/ctpex_proxy_ftdc.h"
using namespace std;
// 请求编号
extern int iRequestID;

DataInitInstance::DataInitInstance(void){
}
DataInitInstance::~DataInitInstance(void)
{
}
void DataInitInstance::initExgTraderApi(){
    string logdir="./log";
    cshfeTraderApi=CShfeFtdcTraderApi::CreateFtdcTraderApi(logdir.c_str());
    cshfeTraderSpi=new CSHFETraderSpi();
    cshfeTraderApi->RegisterSpi(cshfeTraderSpi);
    cshfeTraderApi->SetHeartbeatTimeout(10);
    //cshfeTraderApi->SubscribePublicTopic(TERT_RESTART);
    cshfeTraderApi->SubscribePrivateTopic(TERT_RESTART);//TERT_QUICK
    char tmpfnt[60];
    strcpy(tmpfnt,exgTradeFrontIPCSHFE.c_str());
    cshfeTraderApi->RegisterFront(tmpfnt);
    cout<<"exchange front ip="+exgTradeFrontIPCSHFE<<endl;
    if(cshfeTraderApi->OpenRequestLog("./log/cshfe.log")==0){
        LOG(ERROR)<<"open request info file OK.";
    }else{
        LOG(ERROR)<<"open request file failed!";
    }
    if(cshfeTraderApi->OpenResponseLog("./log/cshfe_rsp.log")==0){
        LOG(ERROR)<<"open response info file OK.";
    }else{
        LOG(ERROR)<<"open response file failed!";
    }
    cshfeTraderApi->Init();
}

/*
void DataInitInstance::initExgTraderApi(){
    string dir="./log";
    ctpex_proxy_driver_log_set log_set;
    log_set.b_log_stdout=true;
    log_set.b_request_log=true;
    log_set.b_response_log=true;
    log_set.b_log_file=true;
    string user_id=exgTraderIDCSHFE;
    string participant_id=exgParticipantIDCSHFE;
    string password=exgTraderPasswdCSHFE;
    string user_product_info="";
    string trader_private_topic_resume_type=exgFlowType;
    int data_center_id=1;
    string fnt=exgTradeFrontIPCSHFE;
    ctpex_proxy_trader_cshfe* mytrader=new ctpex_proxy_trader_cshfe();
    ctpex_proxy_trader_spi* myspi=new ctpex_proxy_trader_spi();
    mytrader->init(dir,user_id,participant_id,password,user_product_info,trader_private_topic_resume_type,data_center_id,log_set);
    mytrader->register_spi(myspi);
    mytrader->add_front_server(fnt);
    mytrader->open();
    cout << "Hello World!" << endl;
}*/

void DataInitInstance::beginDataInit(){
    std::ifstream myfile("config/global.properties");
    if (!myfile) {
        cerr << "读取global.properties文件失败" << endl;
    }
    string str;
    while (getline(myfile, str)) {
        int pos = str.find("#");
        if (pos == 0) {
            cout << "注释:" << str << endl;
        }
        else {
            vector<string> vec = UniverseTools::split(str, "=");
            if(vec.size()!=2){
                continue;
            }
            //cout << str << endl;
            cout<<vec[0]<<"="<<vec[1]<<endl;
            if ("investorid" == vec[0]) {
                //INVESTOR_ID = vec[1];
                //strcpy(INVESTOR_ID, vec[1].c_str());
            }else if ("environment" == vec[0]) {
                environment = vec[1];
                //strcpy(BROKER_ID, vec[1].c_str());
            }
            else if ("marketServerIP" == vec[0]) {
                marketServerIP = vec[1];
            }
            else if ("marketServerPort" == vec[0]) {
                marketServerPort = boost::lexical_cast<int>(vec[1]);
            }else if ("tradeServerIP" == vec[0]) {
                tradeServerIP = vec[1];
            }
            else if ("tradeServerPort" == vec[0]) {
                tradeServerPort = boost::lexical_cast<int>(vec[1]);
            }
            else if ("queryServerPort" == vec[0]) {
                queryServerPort = boost::lexical_cast<int>(vec[1]);
            }
            else if ("queryServerIP" == vec[0]) {
                queryServerIP = vec[1];
            }else if ("exgTradeFrontIPCSHFE" == vec[0]) {
                exgTradeFrontIPCSHFE = vec[1];
            }else if ("exgParticipantIDCSHFE" == vec[0]) {
                exgParticipantIDCSHFE = vec[1];
            }else if ("brokerid" == vec[0]) {
                BROKER_ID = vec[1];
                //strcpy(BROKER_ID, vec[1].c_str());
            }else if ("exgTraderIDCSHFE" == vec[0]) {
                exgTraderIDCSHFE = vec[1];
            }else if ("exgTraderPasswdCSHFE" == vec[0]) {
                exgTraderPasswdCSHFE = vec[1];
            }
            else if ("exgFlowType" == vec[0]) {
                exgFlowType = vec[1];
            }
            else if ("profitValue" == vec[0]) {
                //profitValue = boost::lexical_cast<double>(vec[1]);
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
            }
            else if ("tradeFrontAddr" == vec[0]) {
                strcpy(TRADE_FRONT_ADDR, vec[1].c_str());
            }else if ("followTimes" == vec[0]) {
                followTimes=boost::lexical_cast<int>(vec[1]);
            }else if ("follow" == vec[0]) {
                cout<<vec[1]<<endl;
                string spreadList = vec[1];
                vector<string> tmp_splists = UniverseTools::split(spreadList,",");
                for (unsigned int i = 0; i < tmp_splists.size();i++) {
                    string tmp_str = tmp_splists[i];
                    //many nb account
                    vector<string> followAccountStr = UniverseTools::split(tmp_str,"~");
                    //account concat by $
                    vector<string> tmpacc = UniverseTools::split(followAccountStr[0],"%");
                    UserAccountInfo* ba=new UserAccountInfo();
                    ba->investorID=tmpacc[0];
                    ba->password=tmpacc[1];
                    //account concat by $
                    vector<string> instr_s = UniverseTools::split(followAccountStr[1],"$");
                    ba->nbman=instr_s;
                    unordered_map<string, BaseAccount*>::iterator it_map_strs = this->followNBAccountMap.find(tmpacc[0]);//合约配对信息
                    if (it_map_strs == this->followNBAccountMap.end()) {//未建立配对关系,需新建一个vector
                        //vector<string> tmp_vec;
                        //tmp_vec.push_back(instr_s[0]);
                        this->followNBAccountMap[tmpacc[0]] = ba;
                        allAccountMap[tmpacc[0]]=ba;
                        /*isUserLogin=false;
                        loginInvestorID=tmpacc[0];
                        CTPInterface* pUserApi=initTradeApi("");
                        tradeApiMap[tmpacc[0]]=pUserApi;
                        while(1){
                            if(isUserLogin){
                                loginInvestorID="";
                                break;
                            }else{
                                boost::this_thread::sleep(boost::posix_time::seconds(1));    //microsecond,millisecn
                            }
                        }*/
                    }else {}

                }/*
                for( unordered_map<string, BaseAccount*>::iterator iter=this->followNBAccountMap.begin();iter!=this->followNBAccountMap.end();iter++ ){
                       cout<<"key="<<iter->first<<endl;
                       UserAccountInfo* ua=(UserAccountInfo*)iter->second;
                       cout<<"pwd="<<ua->password<<endl;
                }*/

            }else if ("nbman" == vec[0]) {
                cout<<vec[1]<<endl;
                string spreadList = vec[1];
                vector<string> tmp_splists = UniverseTools::split(spreadList,",");
                for (unsigned int i = 0; i < tmp_splists.size();i++) {
                    string tmp_str = tmp_splists[i];
                    //many nb account
                    vector<string> followAccountStr = UniverseTools::split(tmp_str,"~");
                    //account

                    vector<string> tmpacc = UniverseTools::split(followAccountStr[0],"%");
                    UserAccountInfo* ba=new UserAccountInfo();
                    ba->investorID=tmpacc[0];
                    ba->password=tmpacc[1];
                    //account concat by $
                    vector<string> instr_s = UniverseTools::split(followAccountStr[1],"$");
                    ba->nbman=instr_s;
                    //第一个合约的配对关系,以第一个合约为key，第二个合约需要放到list里面
                    unordered_map<string, BaseAccount*>::iterator it_map_strs = this->NBAccountMap.find(tmpacc[0]);//合约配对信息
                    if (it_map_strs == NBAccountMap.end()) {//未建立配对关系,需新建一个vector
                        //vector<string> tmp_vec;
                        //tmp_vec.push_back(instr_s[0]);
                        NBAccountMap[tmpacc[0]] = ba;
                        allAccountMap[tmpacc[0]]=ba;
                    }else {}

                }
                for( unordered_map<string, BaseAccount*>::iterator iter=NBAccountMap.begin();iter!=NBAccountMap.end();iter++ ){
                       cout<<"key="<<iter->first<<endl;
                }

            }/*else if ("followinfo" == vec[0]) {
                cout<<vec[1]<<endl;
                string spreadList = vec[1];
                vector<string> tmp_splists = UniverseTools::split(spreadList,",");
                for (unsigned int i = 0; i < tmp_splists.size();i++) {
                    string tmp_str = tmp_splists[i];
                    //many nb account
                    vector<string> followAccountStr = UniverseTools::split(tmp_str,"~");
                    //account concat by $
                    vector<string> instr_s = UniverseTools::split(followAccountStr[1],"$");
                    //第一个合约的配对关系,以第一个合约为key，第二个合约需要放到list里面
                    unordered_map<string, vector<string>>::iterator it_map_strs = this->NBAccountMap.find(followAccountStr[0]);//合约配对信息
                    if (it_map_strs == NBAccountMap.end()) {//未建立配对关系,需新建一个vector
                        //vector<string> tmp_vec;
                        //tmp_vec.push_back(instr_s[0]);
                        NBAccountMap[followAccountStr[0]] = instr_s;
                    }else {}

                }
                for( unordered_map<string, vector<string>>::iterator iter=NBAccountMap.begin();iter!=NBAccountMap.end();iter++ ){
                       cout<<"key="<<iter->first<<endl;
                }

            }*/else if ("instrumentList" == vec[0]) {
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
                    cout<<ppInstrumentID[i]<<endl;
                }
                iInstrumentID = quoteList.size();
            }
        }

    }
}
bool DataInitInstance::isNBMANTrade(CThostFtdcTradeField *pTrade){
    string clientid=boost::lexical_cast<string>(pTrade->InvestorID);
    unordered_map<string, BaseAccount*>::iterator nbIT=NBAccountMap.find(clientid);
    if(nbIT==NBAccountMap.end()){//not find
        return false;
    }else{
        return true;
    }
}
bool DataInitInstance::isNBMAN(string investorID){
    unordered_map<string, BaseAccount*>::iterator nbIT=NBAccountMap.find(investorID);
    if(nbIT==NBAccountMap.end()){//not find
        cout<<"investorID="+investorID+" is not naman."<<endl;
        return false;
    }else{
        cout<<"investorID="+investorID+" is naman."<<endl;
        return true;
    }
}
bool DataInitInstance::isNBMANOrder(CThostFtdcOrderField *pOrder){
    string clientid=boost::lexical_cast<string>(pOrder->InvestorID);
    unordered_map<string, BaseAccount*>::iterator nbIT=NBAccountMap.find(clientid);
    if(nbIT==NBAccountMap.end()){//not find
        return false;
    }else if(pOrder->OrderStatus == '0'){
        string userid=boost::lexical_cast<string>(pOrder->UserID);
        //string clientid=boost::lexical_cast<string>(pOrder->ClientID);
        string direction=boost::lexical_cast<string>(pOrder->Direction);
        string offsetflag=boost::lexical_cast<string>(pOrder->CombOffsetFlag);
        string instrumentid=boost::lexical_cast<string>(pOrder->InstrumentID);
        string msg="tradingDay="+boost::lexical_cast<string>(pOrder->TradingDay)+";"
                +"userid="+userid+";"
                +"clientID="+clientid+";"
                +"instrumentID="+instrumentid+";"
                +"orderStatus="+boost::lexical_cast<string>(pOrder->OrderStatus)+";"
                +"OrderSysID="+boost::lexical_cast<string>(pOrder->OrderSysID)+";"
                +"direction="+direction+";"
                +"offsetflag="+offsetflag+";"
                +"price="+boost::lexical_cast<string>(pOrder->LimitPrice)+";"
                +"volume="+boost::lexical_cast<string>(pOrder->VolumeTotalOriginal)+";";
        cout<<msg<<endl;
        LOG(INFO)<<msg;
        UserAccountInfo* bacc=(UserAccountInfo*)nbIT->second;
        vector<string> follows=bacc->nbman;//this is subscripted account
        for(vector<string>::iterator flwIT=follows.begin();flwIT!=follows.end();flwIT++){
            string followAcc=(*flwIT);
            //find follow account info order inserting
            unordered_map<string, BaseAccount*>::iterator iter=followNBAccountMap.find(followAcc);
            if(iter!=followNBAccountMap.end()){
                int totalVolume=(int)pOrder->VolumeTotalOriginal*followTimes;
                string flwmsg="";
                flwmsg="account="+followAcc+" is following clientid="+clientid+",follow times is "+boost::lexical_cast<string>(followTimes)+","
                        +",order:"+msg;
                LOG(INFO)<<flwmsg;
                string ivestorID=iter->first;
                UserAccountInfo* uai=(UserAccountInfo*)iter->second;
                //CTPInterface* interface=dii->getTradeApi(ivestorID);
                //CTraderSpi *userSpi=(CTraderSpi*)interface->pUserSpi;
                cout<<"add order investor="<<iter->first<<";uai="<<uai->investorID<<endl;
                /*******************assume order volume more than 1.**********************/

                for(int i=0;i<totalVolume;i++){
                    UserOrderField* userOrderField = new UserOrderField();
                    userOrderField->brokerID=BROKER_ID;
                    userOrderField->direction=direction;
                    userOrderField->frontID=uai->frontID;
                    userOrderField->sessionID=uai->sessionID;
                    userOrderField->hedgeFlag=uai->hedgeFlag;
                    userOrderField->instrumentID=instrumentid;
                    userOrderField->investorID=ivestorID;
                    userOrderField->offsetFlag=offsetflag;
                    userOrderField->orderInsertPrice=pOrder->LimitPrice;
                    string priceType="2";
                    strcpy(userOrderField->orderPriceType,priceType.c_str());
                    userOrderField->orderRef=uai->orderRef++;
                    userOrderField->volume=1;
                    addNewOrderInsert(userOrderField);
                }

            }
        }
        return true;
    }
}
void DataInitInstance::delTraderApi(string investorID){
    unordered_map<string, CTPInterface*>::iterator iter=tradeApiMap.find(investorID);
    if(iter!=tradeApiMap.end()){
        cout<<"delTraderApi:delete investorID="+investorID+"'s trader api"<<endl;
        tradeApiMap.erase(iter);
    }
}

void DataInitInstance::initTradeApi(unordered_map<string, BaseAccount*> accMAP){
    for( unordered_map<string, BaseAccount*>::iterator iter=accMAP.begin();iter!=accMAP.end();iter++ ){
           cout<<"key="<<iter->first<<endl;
           BaseAccount* ba=iter->second;
           isUserLogin=false;
           loginInvestorID=ba->investorID;
           CTPInterface* interface=instanceTradeApi(ba->investorID);
           interface->investorID=ba->investorID;
           tradeApiMap[ba->investorID]=interface;
           loginOK[ba->investorID]=false;
           while(1){
               if(isUserLogin){
                   //loginInvestorID="";
                   break;
               }else{
                   this_thread::yield();
                   //boost::this_thread::sleep(boost::posix_time::seconds(1));    //microsecond,millisecn
               }
           }
    }
}
CTPInterface* DataInitInstance::instanceTradeApi(string investorID){
    string prefix=investorID+getTime();
    // 初始化UserApi
    CThostFtdcTraderApi* pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi(prefix.c_str());			// 创建UserApi
    CTraderSpi* pUserSpi = new CTraderSpi();
    pUserApi->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi);			// 注册事件类
    pUserApi->SubscribePublicTopic(THOST_TERT_RESUME);					// 注册公有流
    pUserApi->SubscribePrivateTopic(THOST_TERT_RESUME);					// 注册私有流
    cout<<"mk="<<MARKET_FRONT_ADDR<<endl;
    cout<<"trade="<<TRADE_FRONT_ADDR<<endl;
    string msg="mk front="+boost::lexical_cast<string>(MARKET_FRONT_ADDR)+";"+"trade="+boost::lexical_cast<string>(TRADE_FRONT_ADDR);
    LOG(INFO)<<msg;
    pUserApi->RegisterFront(TRADE_FRONT_ADDR);
    pUserApi->Init();
    CTPInterface *interface=new CTPInterface();
    interface->pUserApi=pUserApi;
    interface->pUserSpi=pUserSpi;
    return interface;
}
CTPInterface* DataInitInstance::getTradeApi(string investorID){
    cout<<"insid="+investorID<<endl;
    unordered_map<string, CTPInterface*>::iterator famiter = tradeApiMap.find(investorID);
    if (famiter == tradeApiMap.end()) {//未建立配对关系,需新建一个vector
        string msg="getTradeApi:investorID="+investorID+",can't find tradeApi in tradeApiMap.";
        cerr<<msg<<endl;
        LOG(ERROR) <<msg;
        return NULL;
    }else{
        CTPInterface* interface=famiter->second;
        return interface;
    }
}
double DataInitInstance::getPriceTick(string instrumentID) {
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
double DataInitInstance::getTickMetric(string instrumentID) {
    //合约priceTick
    unordered_map<string, InstrumentInfo*>::iterator ins_it = instruments.find(instrumentID);
    if (ins_it == instruments.end()) {
        LOG(ERROR) << "无法查找到合约信息.instrumentID=" + instrumentID;
        return 0;
    }
    InstrumentInfo* insinfo = ins_it->second;
    double priceTick = insinfo->PriceTick;
    double multiplyFactor = insinfo->VolumeMultiple;
    return priceTick*multiplyFactor;
}
double DataInitInstance::getMultipler(string instrumentID) {
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

/*
UserAccountInfo* DataInitInstance::getUserAccount(string investorID){
    unordered_map<string, UserAccount*>::iterator usIT = followNBAccountMap.find(investorID);
    if (usIT != followNBAccountMap.end()) {//未建立配对关系,需新建一个vector
        string msg="getUserAccount:investorID="+investorID+"'s account is existed.";
        LOG(INFO)<<msg;
        return (UserAccountInfo*)usIT->second;
    }else {
        string msg="getUserAccount:investorID="+investorID+"'s account is not existed.";
        return NULL;
    }
}
void DataInitInstance::setUserAccount(UserAccount* ua){
    userAccMap[boost::lexical_cast<string>(ua->pTradingAccount->AccountID)]=ua;

}*/
void DataInitInstance::addNewOrderAction(OrderInfo* orderInfo){
    //1.order action

    CThostFtdcInputOrderActionField req;
    memset(&req, 0, sizeof(CThostFtdcInputOrderActionField));
    ///经纪公司代码
    strcpy(req.BrokerID, orderInfo->brokerID.c_str());
    ///投资者代码
    strcpy(req.InvestorID, orderInfo->investorID.c_str());
    ///报单操作引用
    //	TThostFtdcOrderActionRefType	OrderActionRef;
    ///报单引用
    strcpy(req.OrderRef, orderInfo->orderRef.c_str());
    ///请求编号
    req.RequestID = iRequestID++;
    ///前置编号
    req.FrontID = orderInfo->frontID;
    ///会话编号
    req.SessionID = orderInfo->sessionID;
    ///交易所代码
    //strcpy(req.ExchangeID, pOrder->ExchangeID);
    //	TThostFtdcExchangeIDType	ExchangeID;
    ///报单编号
    strcpy(req.OrderSysID, orderInfo->orderSysID.c_str());
    //	TThostFtdcOrderSysIDType	OrderSysID;
    ///操作标志
    req.ActionFlag = THOST_FTDC_AF_Delete;
    ///合约代码
    strcpy(req.InstrumentID, orderInfo->instrumentID.c_str());
    CTPInterface* interface=getTradeApi(orderInfo->investorID);
    if(interface){
        CTraderSpi* userSpi=(CTraderSpi*)interface->pUserSpi;
        userSpi->ReqOrderActionTwo(&req,interface->pUserApi);
        string msg="addOrderAction:--->>> OK,investorID="+orderInfo->investorID+";interface="+interface->investorID;
        LOG(INFO) <<msg;
    }else{
        string msg="addOrderAction:investorID="+orderInfo->investorID+",can't find tradeApi in tradeApiMap.";
        cerr<<msg<<endl;
        LOG(ERROR) <<msg;
    }
}

void DataInitInstance::addNewOrderInsert(UserOrderField* userOrderField){

    int frontID=userOrderField->frontID;
    int sessionID=userOrderField->sessionID;
    string orderRef=boost::lexical_cast<string>(userOrderField->orderRef);
    string direction=userOrderField->direction;
    string instrumentID=userOrderField->instrumentID;
    string offsetFlag=userOrderField->offsetFlag;
    string hedgeFlag=userOrderField->hedgeFlag;
    double orderInsertPrice=userOrderField->orderInsertPrice;
    int volume=userOrderField->volume;
    int followCout=userOrderField->followCount;
    string mkType=userOrderField->mkType;
    //string orderType=userOrderField->orderType;//addition info
    string function;//addition info
    string timeFlag="0";
    string investorID=userOrderField->investorID;
    string brokerID=userOrderField->brokerID;
    //unsigned int newOrderToken = orderRef;
    unsigned int requestID = userOrderField->requestID;
    //double price = orderInsertPrice;
    char orderPriceType[2]="";
    strcpy(orderPriceType,userOrderField->orderPriceType);
    string orderType = "-1";
    if(direction == "0"){
        //wait for trade,used for orderAction
        OrderInfo* orderInfo = new OrderInfo();
        orderInfo->investorID = investorID;
        orderInfo->orderRef = orderRef;
        orderInfo->frontID=frontID;
        orderInfo->sessionID=sessionID;
        orderInfo->brokerID=brokerID;
        orderInfo->investorID=investorID;
        orderInfo->orderInsertTime=getCurrentTimeByBoost();
        orderInfo->hedgeFlag=hedgeFlag;
        orderInfo->followCount=followCout;
        orderInfo->orderPriceType=userOrderField->orderPriceType;
        orderType = "0";
        if(offsetFlag == "1"){//choose close today or close yestoday
            orderType = "0" + offsetFlag;
            offsetFlag = getCloseMethod(investorID,instrumentID,"sell");
            DLOG(INFO) << "buy close,orderType=" + orderType;
        }else{
            DLOG(INFO) << "buy open,orderType=" + orderType;
        }
        orderInfo->offsetFlag = offsetFlag;
        orderInfo->direction = direction;
        orderInfo->price = orderInsertPrice;
        orderInfo->orderType = orderType;
        orderInfo->volume = volume;
        orderInfo->instrumentID = instrumentID;
        //orderInfo->begin_down_cul = down_culculate;
        //orderInfo->mkType = mkType;
        //orderInfo->function = addinfo->function;
        bidList.emplace_back(orderInfo);
        //DLOG(INFO) << "down_culculate=" + boost::lexical_cast<string>(down_culculate) + ",up_culculate=" + boost::lexical_cast<string>(up_culculate) + ",buy open.";
        DLOG(INFO) << "new add bid list.bidlist size=" + boost::lexical_cast<string>(bidList.size()) +","+ getOrderInfo(orderInfo);
    }else{
        //wait for trade,used for orderAction
        OrderInfo* orderInfo = new OrderInfo();
        orderInfo->investorID = investorID;
        orderInfo->orderRef = orderRef;
        orderInfo->frontID=frontID;
        orderInfo->sessionID=sessionID;
        orderInfo->direction = direction;
        orderInfo->brokerID=brokerID;
        orderInfo->investorID=investorID;
        orderInfo->orderInsertTime=getCurrentTimeByBoost();
        orderInfo->hedgeFlag=hedgeFlag;
        orderInfo->followCount=followCout;
        orderInfo->orderPriceType=userOrderField->orderPriceType;
        orderType = "1";
        if(offsetFlag == "1"){//choose close today or close yestoday
            orderType = "1" + offsetFlag;
            offsetFlag = getCloseMethod(investorID,instrumentID,"buy");
            DLOG(INFO) << "sell close,orderType=" + orderType;
        }else{
            DLOG(INFO) << "sell open,orderType=" + orderType;
        }
        orderInfo->offsetFlag = offsetFlag;
        orderInfo->price = orderInsertPrice;
        orderInfo->orderType = orderType;
        orderInfo->volume = volume;
        orderInfo->instrumentID = instrumentID;
        //orderInfo->begin_up_cul = up_culculate;
        //orderInfo->mkType = mkType;
        //orderInfo->function = addinfo->function;
        askList.emplace_back(orderInfo);
        //DLOG(INFO) << "down_culculate=" + boost::lexical_cast<string>(down_culculate) + ",up_culculate=" + boost::lexical_cast<string>(up_culculate) + ",sell open.";
        DLOG(INFO) << "new add ask list.asklist size=" + boost::lexical_cast<string>(askList.size())  +","+  getOrderInfo(orderInfo);
    }
    //CTP
    //报单结构体
    CThostFtdcInputOrderField req;
    memset(&req, 0, sizeof(req));
    //EES_EnterOrderField orderField;
    //memset(&orderField, 0, sizeof(EES_EnterOrderField));
    if(mkType=="pas" || mkType=="0"){
        ///有效期类型: 当日有效
        req.TimeCondition = THOST_FTDC_TC_GFD;
        //orderField.m_Tif = EES_OrderTif_Day;//立即单 EES_OrderTif_Day  EES_OrderTif_IOC
    }else if(mkType == "agg"){
        ///有效期类型: 当日有效
        req.TimeCondition = THOST_FTDC_TC_IOC;
        //orderField.m_Tif = EES_OrderTif_IOC;//立即单 EES_OrderTif_Day  EES_OrderTif_IOC
    }/*
    orderField.m_HedgeFlag = EES_HedgeFlag_Speculation;////组合投机套保标 投机 '1'套保 '3'
    strcpy(orderField.m_Account, INVESTOR_ID.c_str());
    strcpy(orderField.m_Symbol, instrumentID.c_str());//modify 2.
    orderField.m_Side = m_side;//modify 1.buy open
    orderField.m_Exchange = EES_ExchangeID_shfe;
    orderField.m_SecType = EES_SecType_fut;
    orderField.m_Price = price;//modify 3.
    orderField.m_Qty = volume;
    orderField.m_ClientOrderToken = newOrderToken;*/
    ///经纪公司代码
    strcpy(req.BrokerID, brokerID.c_str());
    ///投资者代码
    strcpy(req.InvestorID, investorID.c_str());
    ///合约代码
    strcpy(req.InstrumentID, instrumentID.c_str());
    ///报单引用
    strcpy(req.OrderRef, orderRef.c_str());
    req.RequestID = requestID;
    ///用户代码
//	TThostFtdcUserIDType	UserID;
    ///报单价格条件: 限价
    //req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    req.OrderPriceType  = orderPriceType[0];
    ///买卖方向:
    //char tmpdir[2]="0";
    //strcpy(tmpdir,direction.c_str());
    req.Direction=direction.c_str()[0];
    ///组合开平标志: 开仓
    req.CombOffsetFlag[0]=offsetFlag.c_str()[0];
    //strcpy(req.CombOffsetFlag[0],offsetFlag.c_str());
    ///组合投机套保标志
    req.CombHedgeFlag[0]=hedgeFlag.c_str()[0];
    //strcpy(req.CombHedgeFlag[0],hedgeFlag.c_str());
    ///价格
    req.LimitPrice = orderInsertPrice;
    ///数量: 1
    req.VolumeTotalOriginal = volume;
    ///有效期类型: 当日有效
    req.TimeCondition = THOST_FTDC_TC_GFD;
    ///GTD日期
//	TThostFtdcDateType	GTDDate;
    ///成交量类型: 任何数量
    req.VolumeCondition = THOST_FTDC_VC_AV;
    ///最小成交量: 1
    req.MinVolume = 1;
    ///触发条件: 立即
    req.ContingentCondition = THOST_FTDC_CC_Immediately;
    ///止损价
//	TThostFtdcPriceType	StopPrice;
    ///强平原因: 非强平
    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    ///自动挂起标志: 否
    req.IsAutoSuspend = 0;
    ///业务单元
//	TThostFtdcBusinessUnitType	BusinessUnit;
    ///请求编号
//	TThostFtdcRequestIDType	RequestID;
    ///用户强评标志: 否
    req.UserForceClose = 0;
    CTPInterface* interface=getTradeApi(investorID);
    if(interface){
        CTraderSpi* userSpi=(CTraderSpi*)interface->pUserSpi;
        userSpi->ReqOrderInsertTwo(&req,interface->pUserApi);
        string msg="addNewOrder:--->>> OK,investorID="+investorID+";interface="+interface->investorID;
        LOG(INFO) <<msg;
    }else{
        string msg="addNewOrder:investorID="+investorID+",can't find tradeApi in tradeApiMap.";
        cerr<<msg<<endl;
        LOG(ERROR) <<msg;
    }
}
void DataInitInstance::processOrder(CThostFtdcOrderField *pOrder,string type){
    string dir=boost::lexical_cast<string>(pOrder->Direction);
    if(dir=="0"){//buy
        if(type=="delete"){
            for(list<OrderInfo*>::iterator untrade_it = bidList.begin();untrade_it!=bidList.end();untrade_it++){
                OrderInfo* order=*untrade_it;
                if(order->frontID==pOrder->FrontID && order->sessionID==pOrder->SessionID && order->orderRef==pOrder->OrderRef){
                    DLOG(INFO) << ("processOrder,delete:frontid=" + boost::lexical_cast<string>(pOrder->FrontID) + ",sessionid="+boost::lexical_cast<string>(pOrder->SessionID)+",orderRef="+boost::lexical_cast<string>(string(pOrder->OrderRef))+","+
                                   "volume delete from "+boost::lexical_cast<string>(order->volume)+" to "+boost::lexical_cast<string>(order->volume-pOrder->VolumeTotalOriginal));
                    order->volume-=pOrder->VolumeTotalOriginal;
                    if(order->volume==0){
                        untrade_it = bidList.erase(untrade_it);
                        DLOG(INFO) << "processOrder,delete:volume=0,delete this info";
                    }
                    break;
                }
            }

        }else if(type=="upEx"){//update exchange info

        }

    }

}

/*执行平仓操作，需要查询持仓情况。做出平今平昨决定。只针对上期所品种.
参数：instrumentID 要查询对手的合约
type 持仓类型.执行卖平仓操作时,输入buy,表示要查询多头的持仓情况.执行买平仓操作时,输入sell，表示要查询空头的持仓情况.
返回值:开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'*/
string DataInitInstance::getCloseMethod(string investorID,string instrumentID, string type) {
    //boost::recursive_mutex::scoped_lock SLock(pst_mtx);
    UserAccountInfo* uai;
    unordered_map<string, BaseAccount*>::iterator fnamIT = followNBAccountMap.find(investorID);
    if(fnamIT==followNBAccountMap.end()){
        string msg="getCloseMethod:investorID="+investorID+",can't find UserAccountInfo in followNBAccountMap.";
        LOG(ERROR)<<msg;
        return "-1";
    }else{
        uai=(UserAccountInfo*)fnamIT->second;
    }
    unordered_map<string, HoldPositionInfo*> positionmap=uai->positionmap;
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
            DLOG(INFO) << instrumentID+" untradeVolume in askList is " + boost::lexical_cast<string>(untradeVolume) + ",untradeYdVolume in askList is " + boost::lexical_cast<string>(untradeYdVolume) + ",longYdPst=" + boost::lexical_cast<string>(longYdPst) +
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
            DLOG(INFO) << instrumentID+" untradeVolume in bidList is " + boost::lexical_cast<string>(untradeVolume) + ",untradeYdVolume in bidList is " + boost::lexical_cast<string>(untradeYdVolume) + ",shortYdPst=" + boost::lexical_cast<string>(shortYdPst) +
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
void DataInitInstance::processHowManyHoldsCanBeClose(CThostFtdcOrderField *pOrder,string type) {
    /*********find holdPstIsLocked and positionmap*********/
    UserAccountInfo* uai;
    unordered_map<string, BaseAccount*>::iterator fnamIT = followNBAccountMap.find(boost::lexical_cast<string>(pOrder->InvestorID));
    if(fnamIT==followNBAccountMap.end()){
        string msg="getCloseMethod:investorID="+boost::lexical_cast<string>(pOrder->InvestorID)+",can't find UserAccountInfo in followNBAccountMap.";
        LOG(ERROR)<<msg;
        return;
    }else{
        uai=(UserAccountInfo*)fnamIT->second;
    }
    //unordered_map<string, HoldPositionInfo*> positionmap=uai->positionmap;
    unordered_map<string, bool> holdPstIsLocked=uai->holdPstIsLocked;
    string instrumentID = pOrder->InstrumentID;
    string offsetFlag = string(pOrder->CombOffsetFlag);
    string lockID = boost::lexical_cast<string>(pOrder->OrderRef) + boost::lexical_cast<string>(pOrder->SessionID) + boost::lexical_cast<string>(pOrder->FrontID);
    if ("lock" == type) {//锁仓
        unordered_map<string, bool>::iterator lockIT = holdPstIsLocked.find(lockID);
        if (lockIT == holdPstIsLocked.end()) {//未锁定
            holdPstIsLocked[lockID] = true;
        } else {//已经锁定，不再锁定
            return;
        }
        if (pOrder->Direction == '0' && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//买平仓，锁定空头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = uai->positionmap.find(instrumentID);
            if (it == uai->positionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，买平仓锁仓操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                DLOG(INFO) << "lock," + instrumentID + " can be closed volume from " + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition) + " to " + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition - pOrder->VolumeTotalOriginal);
            }

            holdInfo->shortAvaClosePosition = holdInfo->shortAvaClosePosition - pOrder->VolumeTotalOriginal;

        } else if (pOrder->Direction == '1' && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//卖平仓，锁定多头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = uai->positionmap.find(instrumentID);
            if (it == uai->positionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，卖平仓锁仓操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                DLOG(INFO) << "lock," + instrumentID + " can be closed volume from " + boost::lexical_cast<string>(holdInfo->longAvaClosePosition) + " to " + boost::lexical_cast<string>(holdInfo->longAvaClosePosition - pOrder->VolumeTotalOriginal);
            }
            holdInfo->longAvaClosePosition = holdInfo->longAvaClosePosition - pOrder->VolumeTotalOriginal;
        }else{
            DLOG(INFO) << "open position,do not need lock.";
        }
    } else if ("release" == type) {//释放持仓
        if (pOrder->Direction == '0' && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//买平仓，释放空头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = uai->positionmap.find(instrumentID);
            if (it == uai->positionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，买平仓释放操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                DLOG(INFO) << "release," + instrumentID + " can be closed volume from " + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition) + " to " + boost::lexical_cast<string>(holdInfo->shortAvaClosePosition + pOrder->VolumeTotalOriginal);
            }
            holdInfo->shortAvaClosePosition = holdInfo->shortAvaClosePosition + pOrder->VolumeTotalOriginal;
        } else if (pOrder->Direction == '1' && (offsetFlag == "1" || offsetFlag == "2" || offsetFlag == "3" || offsetFlag == "4")) {//卖平仓，释放多头可平量
            unordered_map<string, HoldPositionInfo*>::iterator it = uai->positionmap.find(instrumentID);
            if (it == uai->positionmap.end()) {
                LOG(ERROR) << "合约" + instrumentID + " 无持仓信息，卖平仓释放操作错误!!";
                return;
            }
            HoldPositionInfo* holdInfo = it->second;
            if(isLogout){
                DLOG(INFO) << "release," + instrumentID + " can be closed volume from " + boost::lexical_cast<string>(holdInfo->longAvaClosePosition) + " to " + boost::lexical_cast<string>(holdInfo->longAvaClosePosition + pOrder->VolumeTotalOriginal);
            }
            holdInfo->longAvaClosePosition = holdInfo->longAvaClosePosition + pOrder->VolumeTotalOriginal;
        }
    }
}
unordered_map<string, HoldPositionInfo*> DataInitInstance::getPositionMap(string investorID){
    /*********find positionmap*********/
    UserAccountInfo* uai;
    unordered_map<string, BaseAccount*>::iterator fnamIT = followNBAccountMap.find(investorID);
    if(fnamIT==followNBAccountMap.end()){
        string msg="getCloseMethod:investorID="+investorID+",can't find UserAccountInfo in followNBAccountMap.";
        LOG(ERROR)<<msg;
        unordered_map<string, HoldPositionInfo*> xx;
        return xx;
    }else{
        uai=(UserAccountInfo*)fnamIT->second;
    }
    return uai->positionmap;
}

int DataInitInstance::processtrade(TradeInfo *pTrade){
//int DataInitInstance::processtrade(CThostFtdcTradeField *pTrade){
    /*********find positionmap*********/
    string investorID=pTrade->investorID;
    LOG(INFO)<<"processtrade:"<<pTrade->investorID;
    UserAccountInfo* uai;
    unordered_map<string, BaseAccount*>::iterator fnamIT = followNBAccountMap.find(pTrade->investorID);
    if(fnamIT==followNBAccountMap.end()){
        string msg="processtrade:investorID="+pTrade->investorID+",can't find UserAccountInfo in followNBAccountMap.";
        LOG(ERROR)<<msg;
        return -1;
    }else{
        uai=(UserAccountInfo*)fnamIT->second;
        LOG(INFO)<<"processtrade:"<<uai->investorID<<";"<<uai->tradingAccount.closeProfit;
    }
    uai->positionmap;
    ///买卖方向
    //TThostFtdcDirectionType	direction = pTrade->Direction;
    //char Direction[] = { direction,'\0' };
    //sprintf(Direction,"%s",direction);
    ///开平标志
    //TThostFtdcOffsetFlagType	offsetFlag = pTrade->OffsetFlag;
    //char OffsetFlag[] = { offsetFlag,'\0' };
    ///合约代码
    //char	*InstrumentID = pTrade->InstrumentID;
    string str_inst = pTrade->instrumentID;
    int volume = pTrade->volume;
    //买卖方向
    string str_dir = pTrade->direction;
    //开平方向
    string str_offset = pTrade->offsetFlag;
    //成交价格
    double tradePrice = pTrade->tradePrice;
    //合约乘数
    unordered_map<string, InstrumentInfo*>::iterator ins_it = instruments.find(str_inst);
    if (ins_it == instruments.end()) {
        LOG(ERROR) << "处理成交信息时,无法查找到合约信息.instrumentID=" + str_inst;
        return 0;
    }
    InstrumentInfo* insinfo = ins_it->second;
    double multiplyFactor = insinfo->VolumeMultiple;

    unordered_map<string, HoldPositionInfo*>::iterator map_iterator = uai->positionmap.find(str_inst);
    //新开仓
    if (map_iterator == uai->positionmap.end()) {
        //unordered_map<string, int> tmpmap;
        HoldPositionInfo* tmpmap = new HoldPositionInfo();
        if (str_dir == "0") {//买
                             //多头
            tmpmap->longTdPosition = volume;
            tmpmap->longYdPosition = 0;
            tmpmap->longTotalPosition = volume;
            tmpmap->longAvaClosePosition = volume;
            tmpmap->longHoldAvgPrice = tradePrice;
            tmpmap->longAmount = tradePrice*volume*multiplyFactor;
            //空头
            tmpmap->shortTdPosition = 0;
            tmpmap->shortYdPosition = 0;
            tmpmap->shortTotalPosition = 0;
            tmpmap->shortHoldAvgPrice = 0;
            tmpmap->shortAmount = 0;
        } else if (str_dir == "1") {//卖
                                    //空头
            tmpmap->shortTdPosition = volume;
            tmpmap->shortYdPosition = 0;
            tmpmap->shortTotalPosition = volume;
            tmpmap->shortAvaClosePosition = volume;
            tmpmap->shortHoldAvgPrice = tradePrice;
            tmpmap->shortAmount = tradePrice*volume*multiplyFactor;
            //多头
            tmpmap->longTdPosition = 0;
            tmpmap->longYdPosition = 0;
            tmpmap->longTotalPosition = 0;
            tmpmap->longHoldAvgPrice = 0;
            tmpmap->longAmount = 0;
        }
        uai->positionmap[str_inst] = tmpmap;
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
        if (str_dir == "0") {//买
            double tmp_shortHoldAvgPrice = tmpinfo->shortHoldAvgPrice;//空头持仓均价
            int tmp_totalPst = tmpinfo->shortTotalPosition;//原空头持仓量
            double tmp_totalAmount = tmpinfo->shortAmount;//原空头交易金额
            if (str_offset == "0") {//买开仓,多头增加
                tmpinfo->longTdPosition = tmpinfo->longTdPosition + volume;
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                double tmp_longHoldAvgPrice = tmpinfo->longHoldAvgPrice;//多头持仓均价
                int tmp_totalPst = tmpinfo->longTotalPosition;//原多头持仓量
                double tmp_totalAmount = tmpinfo->longAmount;//原多头交易金额
                realLongPstLimit = tmp_tdpst + tmp_ydpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
                tmpinfo->longAvaClosePosition = tmpinfo->longAvaClosePosition + volume;
                tmpinfo->longHoldAvgPrice = (tmp_longHoldAvgPrice*tmp_totalPst + tradePrice*volume) / (realLongPstLimit);//当前多头持仓均价
                tmpinfo->longAmount = tmp_totalAmount + tradePrice*volume*multiplyFactor;//当前多头交易金额
            } else if (str_offset == "1") {//买平仓,空头减少
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                //int tmp_num = map_iterator->second["shortTotalPosition"];
                if (tmp_tdpst > 0) {
                    if (tmp_tdpst <= volume) {
                        tmp_ydpst = tmp_ydpst - (volume - tmp_tdpst);
                        tmp_tdpst = 0;
                    } else {
                        tmp_tdpst = tmp_tdpst - volume;
                    }
                } else if (tmp_tdpst == 0) {
                    tmp_ydpst = tmp_ydpst - volume;
                } else {
                    cout << "tdposition is error!!!" << endl;
                    LOG(ERROR) << "tdposition is error!!!";
                }
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->shortTdPosition = tmp_tdpst;
                tmpinfo->shortYdPosition = tmp_ydpst;
                tmpinfo->shortTotalPosition = realShortPstLimit;
            } else if (str_offset == "3") {//平今
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                tmp_tdpst = tmp_tdpst - volume;
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->shortTdPosition = tmp_tdpst;
                tmpinfo->shortTotalPosition = realShortPstLimit;
            } else if (str_offset == "4") {//平昨
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                if (tmp_ydpst == 0) {
                    char c_err[100];
                    sprintf(c_err, "shortYdPosition is zero!!!,please check this rtn trade.");
                    cout << c_err << endl;
                    LOG(ERROR) << c_err;
                }
                tmp_ydpst = tmp_ydpst - volume;
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->shortYdPosition = tmp_ydpst;
                tmpinfo->shortTotalPosition = realShortPstLimit;
            }
            //买平仓处理空头平均价格;买开仓不需要单独处理
            if (str_offset != "0") {
                tmpinfo->shortHoldAvgPrice = (tmp_shortHoldAvgPrice*tmp_totalPst - tradePrice*volume) / (realShortPstLimit);//当前空头持仓均价
                tmpinfo->shortAmount = tmp_totalAmount - tradePrice*volume*multiplyFactor;//当前空头交易金额
                tmpinfo->shortAvaClosePosition = tmpinfo->shortAvaClosePosition - volume;//当前空头可平量
            }
        } else if (str_dir == "1") {//卖
            double tmp_longHoldAvgPrice = tmpinfo->longHoldAvgPrice;//原多头持仓均价
            int tmp_totalPst = tmpinfo->longTotalPosition;//原多头持仓量
            double tmp_totalAmount = tmpinfo->longAmount;//原多头交易金额
            if (str_offset == "0") {//卖开仓,空头增加
                tmpinfo->shortTdPosition = tmpinfo->shortTdPosition + volume;
                double tmp_shortHoldAvgPrice = tmpinfo->shortHoldAvgPrice;//原空头持仓均价
                int tmp_totalPst = tmpinfo->shortTotalPosition;//原空头持仓量
                double tmp_totalAmount = tmpinfo->shortAmount;//原空头交易金额
                int tmp_tdpst = tmpinfo->shortTdPosition;
                int tmp_ydpst = tmpinfo->shortYdPosition;
                realShortPstLimit = tmp_tdpst + tmp_ydpst;
                tmpinfo->shortTotalPosition = realShortPstLimit;
                tmpinfo->shortAvaClosePosition = tmpinfo->shortAvaClosePosition + volume;
                tmpinfo->shortHoldAvgPrice = (tmp_shortHoldAvgPrice*tmp_totalPst + tradePrice*volume) / (realShortPstLimit);//当前空头持仓均价
                tmpinfo->shortAmount = tmp_totalAmount + tradePrice*volume*multiplyFactor;//当前空头交易金额
            } else if (str_offset == "1") {//卖平仓,多头减少
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                if (tmp_tdpst > 0) {
                    if (tmp_tdpst <= volume) {
                        tmp_ydpst = tmp_ydpst - (volume - tmp_tdpst);
                        tmp_tdpst = 0;
                    } else {
                        tmp_tdpst = tmp_tdpst - volume;
                    }
                } else if (tmp_tdpst == 0) {
                    tmp_ydpst = tmp_ydpst - volume;
                } else {
                    cout << "tdposition is error!!!" << endl;
                    LOG(ERROR) << "tdposition is error!!!";
                }
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->longTdPosition = tmp_tdpst;
                tmpinfo->longYdPosition = tmp_ydpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
            } else if (str_offset == "3") {//平今
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                tmp_tdpst = tmp_tdpst - volume;
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->longTdPosition = tmp_tdpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
            } else if (str_offset == "4") {//平昨
                int tmp_tdpst = tmpinfo->longTdPosition;
                int tmp_ydpst = tmpinfo->longYdPosition;
                if (tmp_ydpst == 0) {
                    char c_err[100];
                    sprintf(c_err, "longYdPosition is zero!!!,please check this rtn trade.");
                    cout << c_err << endl;
                    LOG(ERROR) << c_err;
                }
                tmp_ydpst = tmp_ydpst - volume;
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                tmpinfo->longYdPosition = tmp_ydpst;
                tmpinfo->longTotalPosition = realLongPstLimit;
            }
            //卖平仓处理多头平均价格;卖开仓不需要单独处理
            if (str_offset != "0") {
                tmpinfo->longHoldAvgPrice = (tmp_longHoldAvgPrice*tmp_totalPst - tradePrice*volume) / (realLongPstLimit);//当前多头持仓均价
                tmpinfo->longAmount = tmp_totalAmount - tradePrice*volume*multiplyFactor;//当前多头交易金额
                tmpinfo->longAvaClosePosition = tmpinfo->longAvaClosePosition - volume;
            }
        }
    }
    //processAverageGapGprice(pTrade);
    //tradeParaProcessTwo();
    string tmpmsg=investorID+";";
    for (unordered_map<string, HoldPositionInfo*>::iterator it = uai->positionmap.begin(); it != uai->positionmap.end(); it++) {
        HoldPositionInfo* tmpinfo = it->second;
        //tmpmsg.append(it->first).append("持仓情况:");
        tmpmsg.append(it->first).append("hold position:");
        char char_tmp_pst[10] = { '\0' };
        char char_longyd_pst[10] = { '\0' };
        char char_longtd_pst[10] = { '\0' };
        sprintf(char_tmp_pst, "%d", tmpinfo->longTotalPosition);
        sprintf(char_longyd_pst, "%d", tmpinfo->longYdPosition);
        sprintf(char_longtd_pst, "%d", tmpinfo->longTdPosition);
        char str_avgBuy[25], str_avgSell[25];
        int sig = 2;
        gcvt(tmpinfo->longHoldAvgPrice, sig, str_avgBuy);
        tmpmsg.append("longposition=");
        tmpmsg.append(char_tmp_pst);
        tmpmsg.append(";todayposition=");
        tmpmsg.append(char_longtd_pst);
        tmpmsg.append(";yesposition=");
        tmpmsg.append(char_longyd_pst);
        tmpmsg.append(";longAvaClosePosition=" + boost::lexical_cast<string>(tmpinfo->longAvaClosePosition));
        tmpmsg.append(";longHoldAvgPrice=");
        tmpmsg.append(boost::lexical_cast<string>(tmpinfo->longHoldAvgPrice));
        tmpmsg.append(";longAmount=" + boost::lexical_cast<string>(tmpinfo->longAmount));
        char char_tmp_pst2[10] = { '\0' };
        char char_shortyd_pst[10] = { '\0' };
        char char_shorttd_pst[10] = { '\0' };
        gcvt(tmpinfo->shortHoldAvgPrice, sig, str_avgSell);
        sprintf(char_tmp_pst2, "%d", tmpinfo->shortTotalPosition);
        sprintf(char_shortyd_pst, "%d", tmpinfo->shortYdPosition);
        sprintf(char_shorttd_pst, "%d", tmpinfo->shortTdPosition);
        tmpmsg.append("shortposition=");
        tmpmsg.append(char_tmp_pst2);
        tmpmsg.append(";yesposition=");
        tmpmsg.append(char_shorttd_pst);
        tmpmsg.append(";yesposition=");
        tmpmsg.append(char_shortyd_pst);
        tmpmsg.append(";shortAvaClosePosition=" + boost::lexical_cast<string>(tmpinfo->shortAvaClosePosition));
        tmpmsg.append(";shortHoldAvgPrice=");
        tmpmsg.append(boost::lexical_cast<string>(tmpinfo->shortHoldAvgPrice));
        tmpmsg.append(";shortAmount=" + boost::lexical_cast<string>(tmpinfo->shortAmount) + ";#");
    }
    cout << tmpmsg << endl;
    char errMsg[1024];
    char initMsg[1024];
    strcpy(initMsg,tmpmsg.c_str());
    codeCC->convert(initMsg,strlen(initMsg),errMsg,1024);
    DLOG(INFO) << errMsg;
    return 0;
}
bool DataInitInstance::isAllLoginOK(){
    for(unordered_map<string, bool>::iterator it = loginOK.begin();it!=loginOK.end();it++){
        cout<<it->first<<";ok="<<it->second<<endl;
        if(!it->second){//if any user is not ok,return false
            return false;
        }
    }
    return true;
}
//update order flags
//type:"ctp","exg"
void DataInitInstance::updateOriOrder(CThostFtdcOrderField *pOrder,string type){
    int sessionID=pOrder->SessionID;
    int frontID=pOrder->FrontID;
    string direction=boost::lexical_cast<string>(pOrder->Direction);
    string orderRef=boost::lexical_cast<string>(pOrder->OrderRef);
    if("ctp"==type){
        if(direction=="0"){//buy
            for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();it++){
                OrderInfo* orderInfo = *it;
                if(orderInfo->sessionID==sessionID &&orderInfo->frontID==frontID &&  orderInfo->orderRef == orderRef){
                    orderInfo->brokerOrderSeq=pOrder->BrokerOrderSeq;
                    orderInfo->orderStatus=boost::lexical_cast<string>(pOrder->OrderStatus);
                    orderInfo->orderLocalID=boost::lexical_cast<string>(pOrder->OrderLocalID);
                    LOG(INFO) << "updateOriOrder ctp,size="+boost::lexical_cast<string>(bidList.size())+";find buy order insert,and update:" + getOrderInfo(orderInfo);
                    break;
                }
            }
        }else if(direction=="1"){//sell
            for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();it++){
                OrderInfo* orderInfo = *it;
                if(orderInfo->sessionID==sessionID &&orderInfo->frontID==frontID &&  orderInfo->orderRef == orderRef){
                    orderInfo->brokerOrderSeq=pOrder->BrokerOrderSeq;
                    orderInfo->orderStatus=boost::lexical_cast<string>(pOrder->OrderStatus);
                    orderInfo->orderLocalID=boost::lexical_cast<string>(pOrder->OrderLocalID);
                    LOG(INFO) << "updateOriOrder ctp,size="+boost::lexical_cast<string>(askList.size())+";find sell order insert,and update:" + getOrderInfo(orderInfo);
                    break;
                }
            }
        }else{
            LOG(ERROR) << "ERROR:can not find order insert.";
        }
    }else if("exg"==type){
        if(direction=="0"){//buy
            for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();it++){
                OrderInfo* orderInfo = *it;
                if(orderInfo->sessionID==sessionID &&orderInfo->frontID==frontID &&  orderInfo->orderRef == orderRef){
                    orderInfo->orderSysID=pOrder->OrderSysID;
                    orderInfo->orderStatus=boost::lexical_cast<string>(pOrder->OrderStatus);
                    //orderInfo->orderLocalID=boost::lexical_cast<string>(pOrder->OrderLocalID);
                    LOG(INFO) << "updateOriOrder exg,size="+boost::lexical_cast<string>(bidList.size())+";find buy order insert,and update:" + getOrderInfo(orderInfo);
                    break;
                }
            }
        }else if(direction=="1"){//sell
            for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();it++){
                OrderInfo* orderInfo = *it;
                if(orderInfo->sessionID==sessionID &&orderInfo->frontID==frontID &&  orderInfo->orderRef == orderRef){
                    orderInfo->orderSysID=pOrder->OrderSysID;
                    orderInfo->orderStatus=boost::lexical_cast<string>(pOrder->OrderStatus);
                    LOG(INFO) << "updateOriOrder exg,size="+boost::lexical_cast<string>(askList.size())+";find sell order insert,and update:" + getOrderInfo(orderInfo);
                    break;
                }
            }
        }else{
            LOG(ERROR) << "ERROR:can not find order insert.";
        }
    }

}
bool DataInitInstance::orderIsResbonsed(string investorID){
    for(list<OrderInfo*>::iterator lit= bidList.begin();lit!=bidList.end();lit++){//order list
        OrderInfo* oinfo = *lit;
        if(oinfo->investorID==investorID &&oinfo->orderStatus=="-1"){//not receive order response
            LOG(INFO)<<"orderIsResbonsed:in bidList,there is order not received response.";
            return false;
        }
    }
    for(list<OrderInfo*>::iterator lit= askList.begin();lit!=askList.end();lit++){//order list
        OrderInfo* oinfo = *lit;
        if(oinfo->investorID==investorID &&oinfo->orderStatus=="-1"){//not receive order response
            LOG(INFO)<<"orderIsResbonsed:in askList,there is order not received response.";
            return false;
        }
    }
    if(bidList.size()==0&&askList.size()==0){
        LOG(INFO)<<"orderIsResbonsed:in bidList and askList,all is 0.";

    }
    return true;
}
int DataInitInstance::getFollowTick(string investorID){
    /*********find *********/
    UserAccountInfo* uai;
    unordered_map<string, BaseAccount*>::iterator fnamIT = followNBAccountMap.find(investorID);
    if(fnamIT==followNBAccountMap.end()){
        string msg="getFollowTick:investorID="+investorID+",can't find UserAccountInfo in followNBAccountMap.";
        LOG(ERROR)<<msg;
        return 0;
    }else{
        uai=(UserAccountInfo*)fnamIT->second;
    }
    return uai->followTick;
}

void DataInitInstance::processAction(CThostFtdcOrderField *pOrder){
    string investorID=boost::lexical_cast<string>(pOrder->InvestorID);
    for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();){
        OrderInfo* orderInfo = *it;
        if(orderInfo->investorID==investorID&&orderInfo->orderSysID==boost::lexical_cast<string>(pOrder->OrderSysID)){
            if(orderInfo->status==FOLLOWFLAG){//is follow order action
                //2.insert new order
                double tickMetric=getTickMetric(orderInfo->instrumentID);
                int followTick=getFollowTick(orderInfo->investorID);
                UserOrderField* userOrderField = new UserOrderField();
                userOrderField->direction=orderInfo->direction;
                userOrderField->offsetFlag=orderInfo->offsetFlag;
                userOrderField->orderInsertPrice=orderInfo->price+tickMetric*followTick;
                string msg="processAction:investorID="+orderInfo->investorID+"'s followTick="+boost::lexical_cast<string>(followTick)+
                        ",before price="+boost::lexical_cast<string>(orderInfo->price)+",after price="+boost::lexical_cast<string>(userOrderField->orderInsertPrice)+
                        ",current follow count="+boost::lexical_cast<string>(orderInfo->followCount)+",multifactor="+boost::lexical_cast<string>(tickMetric)+
                        ",instrumentID="+orderInfo->instrumentID;
                LOG(INFO)<<msg;
                userOrderField->brokerID=orderInfo->brokerID;
                userOrderField->frontID=orderInfo->frontID;
                userOrderField->sessionID=orderInfo->sessionID;
                userOrderField->hedgeFlag=orderInfo->hedgeFlag;
                userOrderField->instrumentID=orderInfo->instrumentID;
                userOrderField->investorID=orderInfo->investorID;
                strcpy(userOrderField->orderPriceType,orderInfo->orderPriceType.c_str());
                userOrderField->orderRef=iRequestID++;
                userOrderField->volume=orderInfo->volume;
                userOrderField->followCount=orderInfo->followCount;
                addNewOrderInsert(userOrderField);
            }
            //1.delete original order
            it=bidList.erase(it);
            return;
        }else{
            it++;
        }
    }
    for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();){
        OrderInfo* orderInfo = *it;
        if(orderInfo->investorID==investorID&&orderInfo->orderSysID==boost::lexical_cast<string>(pOrder->OrderSysID)){
            if(orderInfo->status==FOLLOWFLAG){//is follow order action
                //2.insert new order
                double tickMetric=getTickMetric(orderInfo->instrumentID);
                int followTick=getFollowTick(orderInfo->investorID);
                UserOrderField* userOrderField = new UserOrderField();
                userOrderField->direction=orderInfo->direction;
                userOrderField->offsetFlag=orderInfo->offsetFlag;
                userOrderField->orderInsertPrice=orderInfo->price-tickMetric*followTick;
                string msg="processAction:investorID="+orderInfo->investorID+"'s followTick="+boost::lexical_cast<string>(followTick)+
                        ",before price="+boost::lexical_cast<string>(orderInfo->price)+",after price="+boost::lexical_cast<string>(userOrderField->orderInsertPrice)+
                        ",current follow count="+boost::lexical_cast<string>(orderInfo->followCount)+",multifactor="+boost::lexical_cast<string>(tickMetric)+
                        ",instrumentID="+orderInfo->instrumentID;
                LOG(INFO)<<msg;
                userOrderField->brokerID=orderInfo->brokerID;
                userOrderField->frontID=orderInfo->frontID;
                userOrderField->sessionID=orderInfo->sessionID;
                userOrderField->hedgeFlag=orderInfo->hedgeFlag;
                userOrderField->instrumentID=orderInfo->instrumentID;
                userOrderField->investorID=orderInfo->investorID;
                strcpy(userOrderField->orderPriceType,orderInfo->orderPriceType.c_str());
                userOrderField->orderRef=iRequestID++;
                userOrderField->volume=orderInfo->volume;
                userOrderField->followCount=orderInfo->followCount;
                addNewOrderInsert(userOrderField);
            }
            //1.delete original order
            it=askList.erase(it);
            return;
        }else{
            it++;
        }

    }
}

void DataInitInstance::deleteOriOrder(string orderSysID){
    //string direction = cancledOrderInfo->Direction;
    DLOG(INFO) << "delete ori orderinfo:before cancle order,bidlist size=" + boost::lexical_cast<string>(bidList.size());
    for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();){
        OrderInfo* orderInfo = *it;
        if(orderInfo->orderSysID==orderSysID){
            LOG(INFO) << "deleteOriOrder;find order insert ,and delete:" + getOrderInfo(orderInfo);
            it = bidList.erase(it);
            break;
        }else{
            it++;
        }
    }
    DLOG(INFO) << "after cancle order,bidlist size=" + boost::lexical_cast<string>(bidList.size());

    DLOG(INFO) << "delete ori orderinfo:before cancle order,asklist size=" + boost::lexical_cast<string>(askList.size());
    for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();){
        OrderInfo* orderInfo = *it;
        if(orderInfo->orderSysID==orderSysID){
            LOG(INFO) << "deleteOriOrder;find order insert ,and delete:" + getOrderInfo(orderInfo);
            it = askList.erase(it);
            break;
        }else{
            it++;
        }
    }
    DLOG(INFO) << "after cancle order,askList size=" + boost::lexical_cast<string>(askList.size());
}

//only stop profit order action will be process,other order action will be deleted directly
void DataInitInstance::deleteOriOrder(int frontID,int sessionID,string orderRef){
    //string direction = cancledOrderInfo->Direction;
    DLOG(INFO) << "delete ori orderinfo:before cancle order,bidlist size=" + boost::lexical_cast<string>(bidList.size());
    for(list<OrderInfo*>::iterator it = bidList.begin();it != bidList.end();){
        OrderInfo* orderInfo = *it;
        if(orderInfo->sessionID==sessionID &&orderInfo->frontID==frontID &&  orderInfo->orderRef == orderRef){
            LOG(INFO) << "deleteOriOrder;find order insert ,and delete:" + getOrderInfo(orderInfo);
            it = bidList.erase(it);
            break;
        }else{
            it++;
        }
    }
    DLOG(INFO) << "after cancle order,bidlist size=" + boost::lexical_cast<string>(bidList.size());

    DLOG(INFO) << "delete ori orderinfo:before cancle order,asklist size=" + boost::lexical_cast<string>(askList.size());
    for(list<OrderInfo*>::iterator it = askList.begin();it != askList.end();){
        OrderInfo* orderInfo = *it;
        if(orderInfo->sessionID==sessionID &&orderInfo->frontID==frontID &&  orderInfo->orderRef == orderRef){
            LOG(INFO) << "deleteOriOrder;find order insert ,and delete:" + getOrderInfo(orderInfo);
            it = askList.erase(it);
            break;
        }else{
            it++;
        }
    }
    DLOG(INFO) << "after cancle order,askList size=" + boost::lexical_cast<string>(askList.size());
}
void DataInitInstance::setLoginOk(string investorID){
    unordered_map<string, bool>::iterator it = loginOK.find(investorID);
    if(it != loginOK.end()){
        string msg="setLoginOk:set login flag from "+boost::lexical_cast<string>(it->second)+" to true.";
        LOG(INFO)<<msg;
        cout<<msg;
        it->second=true;
    }else{
        string msg="setLoginOk:to true.";
        LOG(INFO)<<msg;
        cout<<msg;
        loginOK[investorID]=true;
    }
}
string DataInitInstance::getTime(){
    time_t timep;
    time (&timep);
    char tmp[64];
    //strftime(tmp, sizeof(tmp), "%Y-%m-%d~%H:%M:%S",localtime(&timep) );
    strftime(tmp, sizeof(tmp), "%Y-%m-%d",localtime(&timep) );
    return tmp;
}
string DataInitInstance::processRspReqInstrument(CThostFtdcInstrumentField *pInstrument) {
    InstrumentInfo* info = new InstrumentInfo();
    strcpy(info->ExchangeID, pInstrument->ExchangeID);
    info->PriceTick = pInstrument->PriceTick;
    info->instrumentID = string(pInstrument->InstrumentID);
    info->VolumeMultiple = pInstrument->VolumeMultiple;
    instruments[string(pInstrument->InstrumentID)] = info;
    LOG(INFO) << "查询到合约信息:" + info->instrumentID + ";priceTick=" + boost::lexical_cast<string>(info->PriceTick) + ";volumeMultiple=" + boost::lexical_cast<string>(info->VolumeMultiple);
    return "";
}
string DataInitInstance::getOrderInfo(OrderInfo* info){
    string msg = "";
    msg += "brokerID=" + info->brokerID + ";";
    msg += "investorID=" + info->investorID + ";";
    msg += "frontID=" + boost::lexical_cast<string>(info->frontID) + ";";
    msg += "sessionID=" + boost::lexical_cast<string>(info->sessionID) + ";";
    msg += "orderRef=" + boost::lexical_cast<string>(info->orderRef) + ";";
    msg += "direction=" + info->direction + ";";
    msg += "offsetFlag=" + info->offsetFlag + ";";
    msg += "orderStatus=" + info->orderStatus + ";";
    msg += "status=" + info->status + ";";
    msg += "orderPriceType=" + info->orderPriceType + ";";
    msg += "followCout=" + boost::lexical_cast<string>(info->followCount) + ";";
    //msg += "clientOrderToken=" + boost::lexical_cast<string>(info->clientOrderToken) + ";";
    msg += "instrumentID=" + info->instrumentID + ";";
    msg += "function=" + info->function + ";";
    msg += "orderLocalID=" + info->orderLocalID + ";";
    msg += "orderSysID=" + info->orderSysID + ";";
    msg += "mkType=" + info->mkType + ";";
    msg += "orderType=" + info->orderType + ";";
    msg += "price=" + boost::lexical_cast<string>(info->price) + ";";
    msg += "volume=" + boost::lexical_cast<string>(info->volume) + ";";
    return msg;
}
void DataInitInstance::startStrategy(){
    int isbegin = 0;
    cout << "是否启动策略程序?0 否，1是" << endl;
    cin >> isbegin;
    if (isbegin == 1) {
        start_process = 1;
    }
}
#endif
