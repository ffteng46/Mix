
// S36.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "S36.h"
#include "S36Dlg.h"
#include "calculate.h"
#include "Strategy.h"
#include "sendOpenOrder.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "api_config.h"
#include "api_trader.h"
#include "api_market.h"
//#include "product.h"
#include <vector>
#include <thread>
//#include "calculateCCI.h"
//void compute();


using namespace std;
calculate cal;				//实例化计算函数类
sendOpenOrder send_hft_order;
void init_hft_order();				//启动高频发单线程


vector<Strategy* > allRunningTasks;
// CS36App

/*
BEGIN_MESSAGE_MAP(CS36App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()
*/

// CS36App construction

CS36App::CS36App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CS36App object

CS36App theApp;


// CS36App initialization

BOOL CS36App::InitInstance()
{

	AllocConsole();
	CWinApp::InitInstance();


	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	thread extraThreadOne(init_hft_order);

	initctp();


	CS36Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CS36App::initctp()
{

	//strcpy(MD_FRONT_ADDR, "tcp://180.168.146.187:10010");
	strcpy(MD_FRONT_ADDR, "tcp://112.65.105.118:19000");
	strcpy(TRADER_FRONT_ADDR, "tcp://180.168.146.187:10000");
	strcpy(BROKER_ID, "9999");
	strcpy(INVESTOR_ID, "054282");
	strcpy(PASSWORD, "WEIxiang19900906");
	//strcpy(INVESTOR_ID, "113593");
	//strcpy(PASSWORD, "cxl690830");					//cdj690113


	nRequestID = 0;

	pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();			// create trader operation api
	CTraderSpi* pTraderSpi = new CTraderSpi();							// spi
	pTraderApi->RegisterSpi(pTraderSpi);								// register api
	pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);					// public stream
	pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);				// private stream
	pTraderApi->RegisterFront(TRADER_FRONT_ADDR);						// connect
	pTraderApi->Init();
	Sleep(500);
	pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();					// create market data api
	CThostFtdcMdSpi* pMdUserSpi = new CMdSpi();							// spi
	pMdUserApi->RegisterSpi(pMdUserSpi);								// register api
	pMdUserApi->RegisterFront(MD_FRONT_ADDR);							// connect
	pMdUserApi->Init();
}

void init_hft_order()
{
	//此线程一直存在
	send_hft_order.sendOrder();



}
