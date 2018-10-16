#pragma once
#ifdef WIN32
/// add by zhou.hu review 2014/4/22 Windwos平台引用的头文件
#include <Windows.h>

typedef HMODULE		T_DLL_HANDLE;

#else
/// add by zhou.hu review 2014/4/22 linux平台引用的头文件
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>

typedef void*		T_DLL_HANDLE;

#endif

#include <string>
#include "EesTraderApi.h"
using std::string;

class TraderDemo : public EESTraderEvent
{
public:
	TraderDemo(void);
	virtual ~TraderDemo(void);

	/// \brief 测试入口函数
	void Run();
private:
	/// \brief 初始化
	bool Init();
	/// \brief 关闭
	void Close();

	/// \brief 延时函数
	void DemoSleep(int nMilliSeconds);
	/// \brief 暂停
	void Pause();

private:
	/// \brief 加载EES交易API的动态库
	bool LoadEESTrader();
	/// \brief 释放EES交易API的动态库
	void UnloadEESTrader();

	/// \brief Windows版加载交易API的动态库
	bool Windows_LoadEESTrader();
	/// \brief Windows版释放交易API的动态库
	void Windows_UnloadEESTrader();

	/// \brief Linux版本加载交易API的动态库
	bool Linux_LoadEESTrader();
	/// \brief Linux版本释放交易API的动态库
	void Linux_UnloadEESTrader();

	/// \brief 初始化参数调整方法
	void InputParam();

	/// \brief 登录
	void Logon();
	/// \brief 下单
	void NewOrder();
	/// \brief 撤单
	void CxlOrder();

public:
    void reqOrderAction(EES_CancelOrder* order);
    void reqOrderInsert(EES_EnterOrderField* orderField);
    void reqInstruments();
    void reqQryInvestorPosition();
    void reqQryInvestorAccount();
    void OnOrderAcceptSim(EES_OrderAcceptField* pAccept);
    void OnOrderMarketAcceptSim(EES_OrderMarketAcceptField* pAccept);
    void OnOrderExecutionSim(EES_OrderExecutionField* pExec);
    void OnOrderCxledSim(EES_OrderCxled* pOrder);
private:
	/// \brief 服务器连接事件
	virtual void OnConnection(ERR_NO errNo, const char* pErrStr);
	/// \brief	服务器主动断开，会收到这个消息
	virtual void OnDisConnection(ERR_NO errNo, const char* pErrStr);
	/// \brief	登录消息的回调
	virtual void OnUserLogon(EES_LogonResponse* pLogon);
	/// \brief	下单被REM接受的事件
	virtual void OnOrderAccept(EES_OrderAcceptField* pAccept);
	/// \brief	下单被市场接受的事件
	virtual void OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept);
	///	\brief	下单被REM拒绝的事件
	virtual void OnOrderReject(EES_OrderRejectField* pReject);
	///	\brief	下单被市场拒绝的事件
	virtual void OnOrderMarketReject(EES_OrderMarketRejectField* pReject);
	///	\brief	订单成交的消息事件
	virtual void OnOrderExecution(EES_OrderExecutionField* pExec);
	///	\brief	订单成功撤销事件
	virtual void OnOrderCxled(EES_OrderCxled* pCxled);
	///	\brief	撤单被拒绝的消息事件
	virtual void OnCxlOrderReject(EES_CxlOrderRej* pReject);
    virtual void OnQuerySymbol(EES_SymbolField* pSymbol, bool bFinish);
    virtual void OnQueryAccountPosition(const char* pAccount, EES_AccountPosition* pAccoutnPosition, int nReqId, bool bFinish);
    virtual void OnQueryAccountBP(const char* pAccount, EES_AccountBP* pAccout, int nReqId );
public:
    void reqOrderInsert(EES_EnterOrderField* orderField,AdditionOrderInfo* aoi);

private:
	EESTraderApi*				m_tradeApi;				///< EES交易API接口
	T_DLL_HANDLE				m_handle;				///< EES交易API句柄
	funcDestroyEESTraderApi		m_distoryFun;			///< EES交易API动态库销毁函数

	string						m_tradeServerIp;		///< 交易通道IP地址
	int							m_tradeServerPort;		///< 交易通道端口
	string						m_queryServerIp;		///< 查询通道IP地址
	int							m_queryServerPort;		///< 查询通道端口
	string						m_logonId;				///< 登录交易服器编号
	string						m_logonPwd;				///< 登录交易服器密码
	string						m_account;				///< 交易帐号
	string						m_symbol;				///< 交易合约
	string						m_local_mac;			///< 本机MAC地址
	int							m_quantity;				///< 发单数量
	EES_MarketToken				m_marketOrderID;		///< 市场订单编号
	int							m_logonStatus;			///< 0-尚未登录; 1-已连接正在登录; 2-登录成功; 3-登录失败，或者已经断开连接
};

