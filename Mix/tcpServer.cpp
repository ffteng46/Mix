
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "property.h"
#include "tcpServer.h"
using namespace std;
extern int start_process;
//extern CTraderSpi* pTradeUserSpi;
extern TraderDemo* ptradeApi;
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
extern int remoteTradeServerPort;//交易端口
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
        cout << "start network service....";
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
            std::cout << "client: " << socket->remote_endpoint().address() << std::endl;
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

int tradeEngineWriter(boost::shared_ptr<boost::asio::ip::tcp::socket> _socket) {
    cout << "hello world";
    int initial = 0;
    boost::system::error_code error;
    int loglength = 0;
    LogMsg *pData;
    try {
        while (true) {
            if (initial == 0) {
                if (isTwoStartStrategy > 0) {
                    initial = 1;
                    continue;
                }
                string tt;
                char frontid[20] = "\0";
                char sessionid[30] = "\0";
                sprintf(frontid, "%d", 10000);
                sprintf(sessionid, "%d", 10000);
                tt.append("businessType=2000;orderRef=");
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
                size_t sendSize = boost::asio::write(*_socket, boost::asio::buffer(strinfo,datalen + 4));
                //boost::asio::async_write(*_socket, boost::asio::buffer(strinfo, strlen(info) + 1),
                //boost::bind(&Server::write_handler, this));
                //_socket->send(boost::asio::buffer(strinfo, strlen(info) + 1), error);
                //printf("Bytes Sent: %d\n", iResult);
                //cout <<"发送数据量:"<<returnAmount<< " 发送数据==》"<< info <<";共"<<iResult<<"字节"<<endl;
                //cout << " data will be sended:" << info << ";发送字节数 " << boost::lexical_cast<string>(sendSize) << " bytes,实际字节数 " << strlen(info) << endl;
                //loglist.push_back("发送数据==》" + strinfo);
                LOG(INFO) << "tradeEngineWriter服务器发送数据字节数=" + boost::lexical_cast<string>(sendSize) + " bytes：" + boost::lexical_cast<string>(strlen(info)) + ",content=" + strinfo;
                strinfo.clear();
                if (error == boost::asio::error::eof)
                    break; // Connection closed cleanly by peer.
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

            size_t readsize = boost::asio::read(*_socket, boost::asio::buffer(pkg_head, 4));
            //size_t readsize = _socket->read(boost::asio::buffer(pkg_head, 4), error);
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else {
                cout << "本次读取Head字节数=" + boost::lexical_cast<string>(readsize) + "," + boost::lexical_cast<string>(pkg_head)<<endl;
                LOG(INFO) << "本次读取head字节数=" + boost::lexical_cast<string>(readsize) + "," + boost::lexical_cast<string>(pkg_head);
            }
            pkg_databodylen = atoi(pkg_head);
            if (pkg_databodylen == 0) {
                this_thread::yield();
            } else {
                cout << "length of databody：" << pkg_databodylen << endl;
                size_t readsize2 = boost::asio::read(*_socket, boost::asio::buffer(recvbuf, pkg_databodylen + 1));
                //size_t readsize2 = _socket->read_some(boost::asio::buffer(recvbuf, pkg_databodylen), error);
                //boost::asio::async_read(_socket, boost::asio::buffer(recvbuf, pkg_databodylen), error);
                if (error == boost::asio::error::eof)
                    break; // Connection closed cleanly by peer.
                else {
                    cout << "本次读取body字节数=" + boost::lexical_cast<string>(readsize2) + "," + boost::lexical_cast<string>(recvbuf) << endl;;
                    LOG(INFO) << "本次读取body字节数=" + boost::lexical_cast<string>(readsize2) + "," + boost::lexical_cast<string>(recvbuf);
                }
                //wprintf(L"Bytes received: %d\n", strlen(recvbuf));
                orderInsertAmount = orderInsertAmount + 1;
                cout << "orderinsert amount：" << orderInsertAmount << ";received data：" << recvbuf << " length：" << strlen(recvbuf) << endl;
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
        cout << p << endl;
        strlist.push_back(p);
        p = strtok(NULL, split); //指向下一个指针
    }
    if (strlist.size() > 1) {
        ot = strlist.front();
        optype = atoi(ot.c_str());
        cout << atoi(ot.c_str()) << endl;
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
