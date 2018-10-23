
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "property.h"
#include "tcpServer.h"
#include <pthread.h>
#include "TimeProcesser.h"
using namespace std;
extern int start_process;
//extern CTraderSpi* pTradeUserSpi;
extern TraderDemo* ptradeApi;
extern boost::thread_group thread_log_group;
extern int isTwoStartStrategy;//等于2的时候，表示明细和汇总持仓查询完毕，启动系统
//测试队列
extern list<LogMsg*> msgList;
// 会话参数
//extern TThostFtdcFrontIDType 	FRONT_ID;	//前置编号
//extern TThostFtdcSessionIDType	SESSION_ID;	//会话编号
extern string	ORDER_REF;	//报单引用
extern char tradingDay[];
//接受客户端报单数量
int orderInsertAmount;

extern boost::lockfree::queue<LogMsg*> networkTradeQueue;////报单、成交消息队列,网络通讯使用
extern int remoteTradeServerPort;//交易端口e
extern int mkAmount;
extern bool recvOK;
///for test
extern list<MarketData*> allMk;
                                 // socket智能指针
typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
// 异步服务器类
class Server {

private:

    // 服务实例
    boost::asio::io_service& ios_;
    // 接收器实例
    boost::asio::ip::tcp::acceptor acceptor_;

    socket_ptr socket_;
public:
    void stop() {
        this->socket_->close();
    }
    Server(boost::asio::io_service& _ios) : ios_(_ios),
        acceptor_(_ios, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), remoteTradeServerPort)) {
        // 默认执行
        start();
    }
    // 启动网络侦听的操作入口
    void start(void) {
        cout << "start network service...."<<endl;
        while (true) {
            // 自定义的智能指针
            socket_ptr socket(new boost::asio::ip::tcp::socket(ios_));
            //this->socket_ = socket;
            boost::system::error_code ec;
            acceptor_.accept(*socket, ec);
            if (ec) {
                LOG(INFO) << "客户端连接异常";
                continue;
            }
            // 打印当前连接进来的客户端
            std::cout << "client from: " << socket->remote_endpoint().address() << std::endl;
            boost::thread t1(&tradeEngineWriter, socket);
            boost::thread t2(&tradeEngineReader, socket);
        }
        /*
        // 异步侦听，若有服务连接，则自动调用Server::handler_accept函数，并将error, socket传入作为参数
        acceptor_.async_accept(*socket,
            boost::bind(&Server::accept_handler, this,
                boost::asio::placeholders::error, socket));*/
    }

    // 请求者响应后触发的处理器
    void accept_handler(const boost::system::error_code& _ec, socket_ptr _socket) {
        // 错误码检测
        if (_ec) {
            return;
        }
        // 启动新的异步监听
        start();
        // 打印当前连接进来的客户端
        std::cout << "client: " << _socket->remote_endpoint().address() << std::endl;
        //boost::thread t1(&tradeEngineWriter, _socket);
        //boost::thread t2(&tradeEngineReader, _socket);
    }

    // 完成异步写操作后的处理器
    void write_handler(const boost::system::error_code& _ec) {
        if (!_ec){
            std::cout << "server: send message complete." << std::endl;
        } else {
            stop();
        }

    }

};
static int api_get_thread_policy (pthread_attr_t *attr)
{
    int policy;
    int rs = pthread_attr_getschedpolicy (attr, &policy);
    assert (rs == 0);

    switch (policy)
    {
        case SCHED_FIFO:
            printf ("policy = SCHED_FIFO\n");
            break;
        case SCHED_RR:
            printf ("policy = SCHED_RR");
            break;
        case SCHED_OTHER:
            printf ("policy = SCHED_OTHER\n");
            break;
        default:
            printf ("policy = UNKNOWN\n");
            break;
    }
    return policy;
}

static void api_show_thread_priority (pthread_attr_t *attr,int policy)
{
    int priority = sched_get_priority_max (policy);
    assert (priority != -1);
    printf ("max_priority = %d\n", priority);
    priority = sched_get_priority_min (policy);
    assert (priority != -1);
    printf ("min_priority = %d\n", priority);
}

static int api_get_thread_priority (pthread_attr_t *attr)
{
    struct sched_param param;
    int rs = pthread_attr_getschedparam (attr, &param);
    assert (rs == 0);
    printf ("priority = %d\n", param.__sched_priority);
    return param.__sched_priority;
}

static void api_set_thread_policy (pthread_attr_t *attr,int policy)
{
    int rs = pthread_attr_setschedpolicy (attr, policy);
    assert (rs == 0);
    api_get_thread_policy (attr);
}
int tradeEngineWriter(boost::shared_ptr<boost::asio::ip::tcp::socket> _socket) {
    cout << "hello world";
    int initial = 0;
    boost::system::error_code error;
    int loglength = 0;
    LogMsg *pData;
    try {
        while (true) {
            if (initial == 0) {
                string tt;
                char frontid[20] = "\0";
                char sessionid[30] = "\0";
                sprintf(frontid, "%d", 10000);
                sprintf(sessionid, "%d", 10000);
                tt.append("businessType=wtm_9999;orderRef=");
                tt.append(ORDER_REF).append(";frontID=").append(frontid);
                tt.append(";sessionID=").append(sessionid);
                tt.append(";tradingDay=").append(tradingDay).append(";");
                LogMsg *logmsg = new LogMsg();
                logmsg->setMsg(tt);
                networkTradeQueue.push(logmsg);
                initial = 1;/*
                            if (msgList.size() > 0) {
                            for (list<LogMsg*>::iterator it = msgList.begin(); it != msgList.end(); it++) {
                            LogMsg* tmp2 = *it;
                            networkTradeQueue.push(tmp2);
                            }
                            }*/
            }
            if (networkTradeQueue.empty()) {
                this_thread::yield();
            } else if (networkTradeQueue.pop(pData)) {
                /*
                int id = pData->GetID();
                if (id == 0) {
                pData->setID(10);
                msgList.push_back(pData);
                }*/
                string strinfo = pData->getMsg();;
                int datalen = strinfo.length();
                char dheader[10] = "\0";
                sprintf(dheader, "%4d", datalen);
                string strheader = dheader;
                strinfo = strheader.append(strinfo);

                const char *info = strinfo.c_str();
                if(!_socket->is_open()){
                    boost::system::error_code errorcode;
                    cerr << "socket  error: " << errorcode.message() << endl;
                    break;
                }
                // 异步发送 "hello CSND_Ayo" 消息到客户端，发送成功后，自动调用Server::write_handler函数
                size_t sendSize = boost::asio::write(*_socket, boost::asio::buffer(strinfo,datalen + 4),error);
                //boost::asio::async_write(*_socket, boost::asio::buffer(strinfo, strlen(info) + 1),
                //boost::bind(&Server::write_handler, this));
                //_socket->send(boost::asio::buffer(strinfo, strlen(info) + 1), error);
                //printf("Bytes Sent: %d\n", iResult);
                //cout <<"发送数据量:"<<returnAmount<< " 发送数据==》"<< info <<";共"<<iResult<<"字节"<<endl;
                //cout << " data will be sended:" << info << ";发送字节数 " << boost::lexical_cast<string>(sendSize) << " bytes,实际字节数 " << strlen(info) << endl;
                //loglist.push_back("发送数据==》" + strinfo);
                LOG(INFO) << "tradeEngineWriter服务器发送数据字节数=" + boost::lexical_cast<string>(sendSize) + " bytes：" + boost::lexical_cast<string>(strlen(info)) + ",content=" + strinfo;
                strinfo.clear();
                if (error == boost::asio::error::eof){
                    LOG(ERROR)<<"Writer:Connection closed cleanly by peer.";
                    break; // Connection closed cleanly by peer.
                }
            }
        }
        return 0;
    }catch (const runtime_error &re) {
        cerr <<"error"<< re.what() << endl;
        LOG(ERROR) << re.what();
    }catch (exception* e) {
        cerr <<"error"<< e->what() << endl;
        LOG(ERROR) << e->what();
    }
    return 0;
}

int tradeEngineReader(boost::shared_ptr<boost::asio::ip::tcp::socket> _socket) {
    printf("开始接受数据\n");
    LogMsg *logmsg = new LogMsg();
    logmsg->setMsg("交易服务接收进程启动");
    networkTradeQueue.push(logmsg);
    string ss;
    const char * split = ","; //分割符号
    boost::system::error_code error;
    try {
        while (1) {
            char recvbuf[4096] = "\0";
            char pkg_head[5] = "\0";
            ss.clear();
            char a[100] = "\0";
            int pkg_databodylen = 0;

            size_t readsize = boost::asio::read(*_socket, boost::asio::buffer(pkg_head, 4),error);
            //size_t readsize = _socket->read(boost::asio::buffer(pkg_head, 4), error);
            if (error == boost::asio::error::eof){
                LOG(ERROR)<<"Reader:Connection closed cleanly by peer.";
                break; // Connection closed cleanly by peer.
            }else {
                //cout << "本次读取Head字节数=" + boost::lexical_cast<string>(readsize) + "," + boost::lexical_cast<string>(pkg_head)<<endl;
                //LOG(INFO) << "本次读取head字节数=" + boost::lexical_cast<string>(readsize) + "," + boost::lexical_cast<string>(pkg_head);
            }
            pkg_databodylen = atoi(pkg_head);
            if (pkg_databodylen == 0) {
                this_thread::yield();
            } else {
                //cout << "length of databody：" << pkg_databodylen << endl;
                size_t readsize2 = boost::asio::read(*_socket, boost::asio::buffer(recvbuf, pkg_databodylen),error);
                //size_t readsize2 = _socket->read_some(boost::asio::buffer(recvbuf, pkg_databodylen), error);
                //boost::asio::async_read(_socket, boost::asio::buffer(recvbuf, pkg_databodylen), error);
                if (error == boost::asio::error::eof){
                    LOG(ERROR)<<"Reader:Connection closed cleanly by peer.";
                    break; // Connection closed cleanly by peer.
                }else {
                    //cout << "本次读取body字节数=" + boost::lexical_cast<string>(readsize2) + "," + boost::lexical_cast<string>(recvbuf) << endl;;
                    //LOG(INFO) << "本次读取body字节数=" + boost::lexical_cast<string>(readsize2) + "," + boost::lexical_cast<string>(recvbuf);
                }
                //wprintf(L"Bytes received: %d\n", strlen(recvbuf));
                orderInsertAmount = orderInsertAmount + 1;
                //cout << "orderinsert amount：" << orderInsertAmount << ";received data：" << recvbuf << " length：" << strlen(recvbuf) << endl;
                ss.append("received data from client:" + string(recvbuf));
                //简单解析之后调用相关组件
                simpleAsamble(recvbuf);
            }

        }
        return 0;
    }catch (const runtime_error &re) {
        cerr << re.what() << endl;
        LOG(ERROR) << re.what();
    }catch (exception* e) {
        cerr << e->what() << endl;
        LOG(ERROR) << e->what();
    }
    return 0;
}
/*
111:初始化套利组合持仓数据
*/
void simpleAsamble(char *ch) {
    string ot;
    int optype = 0;
    //charpoint=recvBuffer.GetBuffer(recvBuffer.GetLength());
    const char * split = ";"; //分割符号
    char * p = 0;
    p = strtok(ch, split); //分割字符串
    list<string> strlist;
    while (p != NULL) {
        //cout << p << endl;
        strlist.push_back(p);
        p = strtok(NULL, split); //指向下一个指针
    }
    if (strlist.size() > 0) {
        ot = strlist.front();
        optype = atoi(ot.c_str());
        //cout << atoi(ot.c_str())<<",after="<<boost::lexical_cast<string>(optype) << endl;
        strlist.pop_front();
    }
    if (optype == 100) {
        cout << "ReqOrderInsert" << endl;
        //pTradeUserSpi->ReqOrderInsert(strlist);
    } else if (optype == 200) {
        cout << "ReqOrderAction" << endl;
        //pTradeUserSpi->ReqOrderActionTmp(strlist);
    } else if (optype == 1000) {//查询结算价格、收盘价格
        cout << "ReqOrderAction" << endl;
    } else if (optype == 300) {//查询持仓
                               //pUserSpi->tmpInvestorPosition();
        cout << "ReqQryInvestorPosition" << endl;
    } else if (optype == 111) {//初始化数据
        //初始化数据，暂停行情接受
        start_process = 0;
        initArbgComOrders(strlist);
    } else if (optype == 112) {//初始化数据完成
        //初始化数据，暂停行情接受.修改可平组合数量。
        start_process = 0;
        LOG(INFO) << getAllAlreadyTradedInfo();
        completeInitArbgComOrders();
    } else if (optype == 113) {//初始化均值数据
        initGapPriceData(strlist);
    }else if (optype == 5001) {//初始化techMetric
        if(isTwoStartStrategy>0){
            LOG(INFO)<<"already init.";
            return;
        }else
            initGapPriceData(strlist);
    }else if (optype == 1) {//初始化infrastructure
        initInfrastructure(strlist);
    }else if (optype == 9999) {//初始化marketdata

        if(strlist.size()<=2){
            cout<<"is over;receive mkdata "<<mkAmount<<endl;
            //thread_log_group.create_thread(&lookbacktest);
            pthread_attr_t attr;       // 线程属性
            struct sched_param sched;  // 调度策略
            int rs;

            /*
             * 对线程属性初始化
             * 初始化完成以后，pthread_attr_t 结构所包含的结构体
             * 就是操作系统实现支持的所有线程属性的默认值
             */
            rs = pthread_attr_init (&attr);
            assert (rs == 0);     // 如果 rs 不等于 0，程序 abort() 退出

            /* 获得当前调度策略 */
            int policy = api_get_thread_policy (&attr);

            /* 显示当前调度策略的线程优先级范围 */
            printf ("Show current configuration of priority\n");
            api_show_thread_priority(&attr, policy);

            /* 获取 SCHED_FIFO 策略下的线程优先级范围 */
            printf ("show SCHED_FIFO of priority\n");
            api_show_thread_priority(&attr, SCHED_FIFO);

            /* 获取 SCHED_RR 策略下的线程优先级范围 */
            printf ("show SCHED_RR of priority\n");
            api_show_thread_priority(&attr, SCHED_RR);

            /* 显示当前线程的优先级 */
            printf ("show priority of current thread\n");
            int priority = api_get_thread_priority (&attr);

            /* 手动设置调度策略 */
            //printf ("Set thread policy\n");

            //printf ("set SCHED_FIFO policy\n");
            api_set_thread_policy(&attr, SCHED_FIFO);

            //printf ("set SCHED_RR policy\n");
            //api_set_thread_policy(&attr, SCHED_RR);

            /* 还原之前的策略 */
            //printf ("Restore current policy\n");
            //api_set_thread_policy (&attr, policy);

            /*
             * 反初始化 pthread_attr_t 结构
             * 如果 pthread_attr_init 的实现对属性对象的内存空间是动态分配的，
             * phread_attr_destory 就会释放该内存空间
             */
            //rs = pthread_attr_destroy (&attr);
            pthread_t thread_id;
            //pthread_attr_t attr;
            //pthread_attr_init(&attr);
            //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);		///<设置线程可分离
            //pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);		///<设置线程的继承策略和参数来自于schedpolicy 与 schedparam中属性中显示设置
            //pthread_attr_setscope(&thread_attr, PTHREAD_SCOPE_SYSTEM);				///<设置线程的与系统中所有线程进行竞争

            //pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);					///<设置线程的调试策略
            //int max_priority = sched_get_priority_max(SCHED_FIFO);					///<取得最大的优先级
            ////int min_priority = sched_get_priority_min(SCHED_FIFO);				///<取得最小的优先级

            //struct sched_param sched_value;
            //sched_value.sched_priority = max_priority;
            //pthread_attr_setschedparam(&thread_attr, &sched_value);					///<设置优先级

            int ret = pthread_create(&thread_id, &attr, lookbacktest,NULL);
            //pthread_attr_destroy(&attr);
            /*
            int k=0;
            //开始时间
            boost::posix_time::ptime startTime = getCurrentTimeByBoost();
            for(list<MarketData*>::iterator it = allMk.begin();it!=allMk.end();it++){
                k++;
                cout<<k<<endl;
                MarketData* marketdatainfo=*it;
                //开始时间
                boost::posix_time::ptime startTime = getCurrentTimeByBoost();
                metricProcesserForSingleThread(marketdatainfo);
                boost::posix_time::ptime endTime = getCurrentTimeByBoost();
                int heatBeatSecond = getTimeInterval(startTime,endTime,  "t");
                cout<<"chuli="<<heatBeatSecond<<endl;
            }*/
        }
        start_process=1;
        initMarketData(strlist);
        mkAmount++;

        if(mkAmount%1000==0){
            cout<<"receive mkdata "<<mkAmount<<endl;
            if(mkAmount==151000){
                int i = 0;
                int j=i;
            }
        }

    }
}

int startTCPServer(void) {
    try {
        std::cout << "server start." << std::endl;
        // 建造服务对象
        boost::asio::io_service ios;
        //ttest();
        // 构建Server实例
        Server server(ios);
        // 启动异步调用事件处理循环
        ios.run();
    }
    catch (std::exception& _e) {
        std::cout << _e.what() << std::endl;
    }
    std::cout << "server end." << std::endl;
    return 0;
}
