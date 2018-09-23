/*!****************************************************************************
 @note   Copyright (coffee), 2005-2014, Shengli Tech. Co., Ltd.
 @file   EESQuoteDemo.h
 @date   2014/4/27   12:31
 @author zhou.hu
 
 \brief  本类演示是EES行情API的使用示例。

 @note 
******************************************************************************/

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
#include "EESQuoteApi.h"

using std::string;

class QuoteDemo : public EESQuoteEvent
{
public:
	QuoteDemo(void);
	virtual ~QuoteDemo(void);
	
	/// \brief 测试入口函数
	void Run();

private:
    /// \brief 初始化
    bool Init();
	/// \brief 关闭
	void Close();

	/// \brief 暂停
	void Pause();

private:

	/// \brief 释放EES行情API的动态库
	void UnloadEESQuote();

	/// \brief Windows版加载行情API的动态库
	bool Windows_LoadEESQuote();
	/// \brief Windows版释放行情API的动态库
	void Windows_UnloadEESQuote();

	/// \brief Linux版本加载行情API的动态库
	bool Linux_LoadEESQuote();
	/// \brief Linux版本释放行情API的动态库
	void Linux_UnloadEESQuote();

	/// \brief 初始化参数调整方法
	void InputParam();

	/// \brief 登录
	void Logon();
	/// \brief 查询所有合约
	void QueryAllSymbol();
	/// \brief 注册指定合约的行情
	void RegisterSymbol(EesEqsIntrumentType chInstrumentType, const char* pSymbol);
	/// \brief 显示行情信息
	void ShowQuote(EESMarketDepthQuoteData* pQuote);
	///
	
private:
    /// \brief 加载EES行情API的动态库
    bool LoadEESQuote();
	/// \brief 服务器连接响应
	virtual void OnEqsConnected();
	/// \brief 服务器连接断开响应
	virtual void OnEqsDisconnected();
	/// \brief 登录服务器响应
	virtual void OnLoginResponse(bool bSuccess, const char* pReason);
	/// \brief 接收到行情信息的响应
	virtual void OnQuoteUpdated(EesEqsIntrumentType chInstrumentType, EESMarketDepthQuoteData* pDepthQuoteData);
	/// \brief 注册合约的响应
	virtual void OnSymbolRegisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess);
	/// \brief 注销合约的响应
	virtual void OnSymbolUnregisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess);
	/// \brief 查询合约列表的响应
	virtual void OnSymbolListResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bLast);

private:
	EESQuoteApi*				m_eesApi;				///< EES行情API接口
	T_DLL_HANDLE				m_handle;				///< EES行情API句柄
	funcDestroyEESQuoteApi		m_distoryFun;			///< EES行情API对象销毁函数

	string						m_quoteServerIp;		///< 行情服务器IP
	int							m_quotePort;			///< 行情服务器端口
	string						m_logonId;				///< 登录编号
	string						m_logonPwd;				///< 登录密码
};

