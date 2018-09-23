
// S36Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "S36.h"
#include "S36Dlg.h"
#include "calculate.h"
#include "read_data.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "Strategy.h"
#include "api_config_extern.h"
#include "taskEditorDialog.h"
using namespace std;

extern vector<Strategy* > allRunningTasks;
//extern IShZdTradeInLib* InHandler;
CS36Dlg* pDlg;
read_data read_vector;							// 读取本地数据




CS36Dlg::CS36Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CS36Dlg::IDD, pParent)
	, dlg_add_openvolume(1)
	, dlg_add_instrumentB(_T("jm1809"))   //TA809
	, dlg_add_setlong(-1)
	, dlg_add_hang_frequency(3)
	, dlg_add_hang_depth(7)
	, dlg_add_dual_trust(0.7)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CS36Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_tasklist, dlg_list_tasklist);
	//  DDX_Text(pDX, IDC_EDIT_code, dlg_add_code);
	//  DDX_Text(pDX, IDC_EDIT_exchangecode, dlg_add_exchangecode);
	//  DDX_Text(pDX, IDC_EDIT_instrument, dlg_add_instrument);
	DDX_Text(pDX, IDC_EDIT_openvolume, dlg_add_openvolume);
	//  DDX_Text(pDX, IDC_EDIT_threshold, dlg_add_threshold);
	//  DDX_Text(pDX, IDC_EDIT_threshold2, dlg_add_threshold2);
	DDX_Text(pDX, IDC_EDIT_instrumentB, dlg_add_instrumentB);
	//  DDX_Text(pDX, IDC_EDIT_grid_reversal, dlg_add_grid_reversal);
	DDX_Radio(pDX, IDC_set_long, dlg_add_setlong);
	//  DDX_Text(pDX, IDC_EDIT_grid_breakwaitingtime, dlg_add_breaktime);
	DDX_Text(pDX, IDC_EDIT_hang_frequency, dlg_add_hang_frequency);
	DDX_Text(pDX, IDC_EDIT_hang_depth, dlg_add_hang_depth);
	DDX_Text(pDX, IDC_EDIT_dual_trust, dlg_add_dual_trust);
}


BEGIN_MESSAGE_MAP(CS36Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_addtask, &CS36Dlg::OnBnClickedButtonaddtask)
	ON_BN_CLICKED(IDC_BUTTON_start, &CS36Dlg::OnBnClickedButtonstart)
	ON_BN_CLICKED(IDC_BUTTON_stop, &CS36Dlg::OnBnClickedButtonstop)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_tasklist, &CS36Dlg::OnNMRClickListtasklist)
	ON_COMMAND(ID_ROOT_edit, &CS36Dlg::editTask)
	ON_COMMAND(ID_ROOT_delete, &CS36Dlg::deleteTask)
	ON_BN_CLICKED(IDC_BUTTON_forceclose, &CS36Dlg::OnBnClickedButtonforceclose)
	ON_BN_CLICKED(IDC_set_long, &CS36Dlg::OnBnClickedsetlong)
	ON_BN_CLICKED(IDC_set_short, &CS36Dlg::OnBnClickedsetshort)
	
	ON_BN_CLICKED(IDC_BUTTON_test_func, &CS36Dlg::OnBnClickedButtontestfunc)
	ON_BN_CLICKED(IDC_BUTTON_unlock_order, &CS36Dlg::OnBnClickedButtonunlockorder)
	ON_BN_CLICKED(IDC_BUTTON_test, &CS36Dlg::OnBnClickedButtontest)
END_MESSAGE_MAP()


// CS36Dlg message handlers

BOOL CS36Dlg::OnInitDialog()
{
	pDlg = this;
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	//LVS_REPORT:报表形式，LVS_EX_GRIDLINES:呈现网格条纹
	dlg_list_tasklist.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	CRect dlg_list_tasklist_size;
	dlg_list_tasklist.GetClientRect(dlg_list_tasklist_size);
	dlg_list_tasklist.InsertColumn(0, _T("序号"), LVCFMT_CENTER, 50, 0);
	dlg_list_tasklist.InsertColumn(1, _T("合约名称"), LVCFMT_CENTER, 80, 0);
	dlg_list_tasklist.InsertColumn(2, _T("波动率"), LVCFMT_CENTER, 80, 0);
	dlg_list_tasklist.InsertColumn(3, _T("基准线"), LVCFMT_CENTER, 80, 0);
	dlg_list_tasklist.InsertColumn(4, _T("间隙间隙"), LVCFMT_CENTER, 90, 0);
	dlg_list_tasklist.InsertColumn(5, _T("阈值"), LVCFMT_CENTER, 60, 0);
	//dlg_list_tasklist.InsertColumn(6, _T("阈值B"), LVCFMT_CENTER, 60, 0);
	dlg_list_tasklist.InsertColumn(6, _T("状态"), LVCFMT_CENTER, dlg_list_tasklist_size.Width() - 440, 0);

	taskCounter = 0;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CS36Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1)/2 ;
		int y = (rect.Height() - cyIcon + 1) /2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CS36Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CS36Dlg::OnBnClickedButtonaddtask()
{
	// TODO:  在此添加控件通知处理程序代码
	SYSTEMTIME lt;
	GetLocalTime(&lt);


	UpdateData(true);
	Strategy* temp = new Strategy;
	temp->nTask = taskCounter;


	char p_instrument[20];
	WideCharToMultiByte(CP_ACP, 0, LPCTSTR(dlg_add_instrumentB), -1, p_instrument, 20, NULL, NULL);
	strcpy(temp->InstrumentID, p_instrument);



	char filename1[30];
	sprintf(filename1, "%d_%s_data_%d%d%d.txt", taskCounter, temp->InstrumentID, lt.wYear, lt.wMonth, lt.wDay);
	/*temp->datalog.open(filename1, ios::app);*/
	bool isthere = true;
	fstream _file;
	_file.open(filename1, ios::in);
	if (!_file) isthere = false;
	_file.close();


	temp->datalog.open(filename1, ios::out | ios::app);
	//if (!isthere)
	//{
		//temp->datalog << "交易日," << "时间," << "合约," << "买一价," << "买一量," << "卖一价," << "卖一量," << "现价," <<"基准价,"<< "tick成交量," << "K线," <<"网格线"<< endl;
		//<< "回溯高点," << "回溯低点,"
		//<< "突破," << "主动买," << "主动卖," << "持仓量变化," << "开平方向," << "开仓次数," << "平仓次数," << "毫秒," << "账户多仓," << "账户空仓," << "回执时间,"		
	//}


	char filename5[30];
	sprintf(filename5, "%d_%s_Kdata_15sec_%d%d%d.txt", taskCounter, temp->InstrumentID, lt.wYear, lt.wMonth, lt.wDay);
	temp->kdata15sec.open(filename5, ios::app);

	char filename6[30];
	sprintf(filename6, "%d_%s_Kdata_15min_%d%d%d.txt", taskCounter, temp->InstrumentID, lt.wYear, lt.wMonth, lt.wDay);
	temp->kdata15min.open(filename6, ios::app);

	char filename7[30];
	sprintf(filename7, "%d_%s_Kdata_1hour_%d%d%d.txt", taskCounter, temp->InstrumentID, lt.wYear, lt.wMonth, lt.wDay);
	temp->kdata1hour.open(filename7, ios::app);


	char filename3[30];
	sprintf(filename3, "%d_%s_orderlog_%d%d%d.txt", taskCounter, temp->InstrumentID, lt.wYear, lt.wMonth, lt.wDay);
	temp->orderlog.open(filename3, ios::app);

	/*char filename2[30];
	sprintf(filename2, "%d_%s_tradeRecordlog_%d%d%d.txt", taskCounter, temp->InstrumentID, lt.wYear, lt.wMonth, lt.wDay);
	temp->tradeRecordlog.open(filename2, ios::app);*/

	char filename4[30];
	sprintf(filename4, "%d_%s_tradeRecord_%d%d%d.txt", taskCounter, temp->InstrumentID, lt.wYear, lt.wMonth, lt.wDay);
	temp->tradeRecord.open(filename4, ios::app);

	//数据传递给策略
	temp->openVolume = dlg_add_openvolume;
	temp->set_deepth = dlg_add_hang_depth;
	temp->set_tick_frequenc = dlg_add_hang_frequency;
	temp->ks = dlg_add_dual_trust;


	//报单预置
	memset(&temp->insertOpenOrder, 0, sizeof(temp->insertOpenOrder));
	strcpy(temp->insertOpenOrder.BrokerID, BROKER_ID);
	strcpy(temp->insertOpenOrder.InvestorID, INVESTOR_ID);
	strcpy(temp->insertOpenOrder.InstrumentID, temp->InstrumentID);
	temp->insertOpenOrder.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	temp->insertOpenOrder.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	temp->insertOpenOrder.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	temp->insertOpenOrder.TimeCondition = THOST_FTDC_TC_GTC;									//不成交立即撤销   以前策略到第二天是不是就失效了
	temp->insertOpenOrder.VolumeCondition = THOST_FTDC_VC_AV;
	temp->insertOpenOrder.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	temp->insertOpenOrder.ContingentCondition = THOST_FTDC_CC_Immediately;
	temp->insertOpenOrder.MinVolume = 1;
	temp->insertOpenOrder.IsAutoSuspend = 0;
	temp->insertOpenOrder.UserForceClose = 0;

	//报单预置
	memset(&temp->insertCloseOrder, 0, sizeof(temp->insertCloseOrder));
	strcpy(temp->insertCloseOrder.BrokerID, BROKER_ID);
	strcpy(temp->insertCloseOrder.InvestorID, INVESTOR_ID);
	strcpy(temp->insertCloseOrder.InstrumentID, temp->InstrumentID);
	temp->insertCloseOrder.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
	temp->insertCloseOrder.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	temp->insertCloseOrder.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	temp->insertCloseOrder.TimeCondition = THOST_FTDC_TC_GTC;
	temp->insertCloseOrder.VolumeCondition = THOST_FTDC_VC_AV;
	temp->insertCloseOrder.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	temp->insertCloseOrder.ContingentCondition = THOST_FTDC_CC_Immediately;
	temp->insertCloseOrder.MinVolume = 1;
	temp->insertCloseOrder.IsAutoSuspend = 0;
	temp->insertCloseOrder.UserForceClose = 0;

	
	//撤单预置
	memset(&temp->cancelOrder, 0, sizeof(temp->cancelOrder));
	strcpy(temp->cancelOrder.InstrumentID, temp->InstrumentID);
	strcpy(temp->cancelOrder.BrokerID, BROKER_ID);
	strcpy(temp->cancelOrder.InvestorID, INVESTOR_ID);
	temp->cancelOrder.FrontID = FRONT_ID;
	temp->cancelOrder.SessionID = SESSION_ID;
	temp->cancelOrder.ActionFlag = THOST_FTDC_AF_Delete;

	allRunningTasks.push_back(temp);

	//添加网格
	CString str;
	int nRow = dlg_list_tasklist.GetItemCount();
	str.Format(_T("%d"), taskCounter);
	dlg_list_tasklist.InsertItem(nRow, str);

	//
	int   nLen = strlen(temp->InstrumentID) + 1;
	int   nwLen = MultiByteToWideChar(CP_ACP, 0, temp->InstrumentID, nLen, NULL, 0);
	TCHAR   lpszStr[256];
	MultiByteToWideChar(CP_ACP, 0, temp->InstrumentID, nLen, lpszStr, nwLen);
	dlg_list_tasklist.SetItemText(nRow, 1, lpszStr);

	

	/*int   nLen3 = strlen(temp->InstrumentID) + 1;
	int   nwLen3 = MultiByteToWideChar(CP_ACP, 0, temp->InstrumentID, nLen3, NULL, 0);
	TCHAR   lpszStr3[256];
	MultiByteToWideChar(CP_ACP, 0, temp->InstrumentID, nLen3, lpszStr3, nwLen3);
	dlg_list_tasklist.SetItemText(nRow, 3, lpszStr3);*/

	str.Format(_T("%.2f"), temp->tickData.BidPrice1);
	dlg_list_tasklist.SetItemText(nRow, 2, str);


	str.Format(_T("%.2f"), temp->tickData.AskPrice1);
	dlg_list_tasklist.SetItemText(nRow, 3, str);

	str.Format(_T("%.2f"), temp->tickData.AskPrice1);
	dlg_list_tasklist.SetItemText(nRow, 4, str);



	//str.Format(_T("%.2f"), temp->threshold2);
	//dlg_list_tasklist.SetItemText(nRow, 6, str);

	if (temp->isOn == true)
	{
		dlg_list_tasklist.SetItemText(nRow, 6, _T("On"));
	}
	else
	{
		dlg_list_tasklist.SetItemText(nRow, 6, _T("Off"));
	}


	taskCounter++;

	//订阅市场行情
	char *ppInstrumentID2[1];
	ppInstrumentID2[0] = temp->InstrumentID;
	pMdUserApi->SubscribeMarketData(ppInstrumentID2, 1);

	//查询最小价格
	CThostFtdcQryInstrumentField pQryInstrument;
	memset(&pQryInstrument, 0, sizeof(pQryInstrument));
	pTraderApi->ReqQryInstrument(&pQryInstrument, 0);


	/////////////////////////////////  读取本地K线数据
	vector<string> row;
	string line;
	ifstream in("test.txt");
	vector<double> df;

	df.clear();
	if (in.fail())  
	{ cout << "File not found" << endl; }

	while (getline(in, line) && in.good())
	{
		read_vector.csvline_populate(row, line, ',');

		double open = atof(row[13].c_str());										//第13列
		double high = atof(row[14].c_str());
		double low = atof(row[15].c_str());
		double close = atof(row[15].c_str());
		int n = atof(row[16].c_str());

		Strategy::Kdata newKata;
		newKata.closePrice = close;
		newKata.openPrice = open;
		newKata.highPrice = high;
		newKata.lowPrice = low;
		newKata.nIndex = n;
		newKata.TR = high - low;


		//temp->KData_3600.push_back(newKata);


		//cout <<n<< " 的高开低收: "<< n<<""<<endl;

		//for (int i = 0, leng = row.size(); i < leng; i++)
		//{


		//	cout <<i<<" :"<< row[i] << "--";
		//	cout << endl;
		//}
		//cout <<"++++++++++"<< endl;
		//df.push_back();

	}
	in.close();

	cout << "本地数据读取： " << temp->KData_1m.size() << endl;
	
}


void CS36Dlg::OnBnClickedButtonstart()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(true);

	for (int i = 0; i < dlg_list_tasklist.GetItemCount(); i++)
	{

		CString str = dlg_list_tasklist.GetItemText(i, 0);
		int taskid = _ttoi(str);
		for (vector<Strategy*>::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
		{
			(*it)->isOn = true;

			if ((*it)->isOn)
			{
				dlg_list_tasklist.SetItemText(taskid, 6, _T("On"));
			}
			else
			{
				dlg_list_tasklist.SetItemText(taskid, 6, _T("Off"));
			}
		}
	}

	cout << "start total tasks: " << allRunningTasks.size() << endl;
}


void CS36Dlg::OnBnClickedButtonstop()
{
	// TODO:  在此添加控件通知处理程序代码

	for (int i = 0; i < dlg_list_tasklist.GetItemCount(); i++)
	{
		CString str = dlg_list_tasklist.GetItemText(i, 0);
		int taskid = _ttoi(str);
		for (vector<Strategy*>::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
		{
			(*it)->isOn = false;

			if ((*it)->isOn)
			{
				dlg_list_tasklist.SetItemText(taskid, 6, _T("On"));
			}
			else
			{
				dlg_list_tasklist.SetItemText(taskid, 6, _T("Off"));
			}
		}
	}

	cout << "stop total tasks: " << allRunningTasks.size() << endl;
}


void CS36Dlg::OnNMRClickListtasklist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	*pResult = 0;

	int itemIndex = pNMItemActivate->iItem;
	activeListID = pNMItemActivate->iItem;
	if (itemIndex != -1)
	{
		CMenu rightPopMenu;
		rightPopMenu.LoadMenu(IDR_MENU1);
		ASSERT(rightPopMenu.GetSafeHmenu());
		CMenu* pSubMenu = rightPopMenu.GetSubMenu(0);
		ASSERT(pSubMenu);
		CPoint curPoint;
		GetCursorPos(&curPoint);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, curPoint.x, curPoint.y, this);
	}

}

void CS36Dlg::editTask()
{
	taskEditorDialog dlgEditTask;
	dlgEditTask.DoModal();

	CString str2 = dlg_list_tasklist.GetItemText(activeListID, 0);
	int taskid = _ttoi(str2);
	for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
	{
		if ((*it)->nTask == taskid)
		{
			CString str;
			int nRow = activeListID;

			str.Format(_T("%d"), (*it)->openVolume);
			dlg_list_tasklist.SetItemText(nRow, 4, str);
			str.Format(_T("%.2f"), (*it)->threshold);
			dlg_list_tasklist.SetItemText(nRow, 5, str);

			//str.Format(_T("%.2f"), (*it)->threshold2);
			//dlg_list_tasklist.SetItemText(nRow, 6, str);

			if ((*it)->isOn == true)
			{
				dlg_list_tasklist.SetItemText(nRow, 6, _T("On"));
			}
			else
			{
				dlg_list_tasklist.SetItemText(nRow, 6, _T("Off"));
			}
		}
	}
}


void CS36Dlg::deleteTask()
{
	CString str2 = dlg_list_tasklist.GetItemText(activeListID, 0);
	int taskid = _ttoi(str2);

	for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
	{
		if ((*it)->nTask == taskid)
		{
			allRunningTasks.erase(it);
			break;
		}
	}

	dlg_list_tasklist.DeleteItem(activeListID);
}



void CS36Dlg::OnBnClickedButtonforceclose()
{
	// TODO:  在此添加控件通知处理程序代码


	for (vector<Strategy* >::iterator itx = allRunningTasks.begin(); itx != allRunningTasks.end(); itx++)
	{
		for (vector<Strategy::ORDERINFO>::iterator it = (*itx)->closeLongOrder.begin(); it != (*itx)->closeLongOrder.end(); it++)
		{
			if (it->orderReturn.OrderStatus != THOST_FTDC_OST_AllTraded && it->orderReturn.OrderStatus != THOST_FTDC_OST_Canceled && !it->isCancelSent )
			{
				strcpy((*itx)->cancelOrder.OrderRef, it->OrderRef);
				(*itx)->cancelOrder.OrderActionRef = nOrderRef++;
				pTraderApi->ReqOrderAction(&(*itx)->cancelOrder, nRequestID++);
				it->isCancelSent = true;

				cout << "force close " << it->orderInput.LimitPrice << " " << (*itx)->tickData.AskPrice1 << endl;
			}
		}

		for (vector<Strategy::ORDERINFO>::iterator it = (*itx)->closeShortOrder.begin(); it != (*itx)->closeShortOrder.end(); it++)
		{
			if (it->orderReturn.OrderStatus != THOST_FTDC_OST_AllTraded && it->orderReturn.OrderStatus != THOST_FTDC_OST_Canceled && !it->isCancelSent )
			{
				strcpy((*itx)->cancelOrder.OrderRef, it->OrderRef);
				(*itx)->cancelOrder.OrderActionRef = nOrderRef++;
				pTraderApi->ReqOrderAction(&(*itx)->cancelOrder, nRequestID++);
				it->isCancelSent = true;

				cout << "force close " << it->orderInput.LimitPrice << " " << (*itx)->tickData.BidPrice1 << endl;
			}
		}

	}

}


void CS36Dlg::OnBnClickedsetlong()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(true);
	
	CString str2 = dlg_list_tasklist.GetItemText(activeListID, 0);
	int taskid = _ttoi(str2);

	//cout <<"taskid"<< taskid << endl;

	for (vector<Strategy* >::iterator itx = allRunningTasks.begin(); itx != allRunningTasks.end(); itx++)
	{
		if ((*itx)->nTask == taskid)
		{
			//cout << " frist:" << (*itx)->tupoBuy << endl;;
			
			cout << "全天只做多:" << (*itx)->do_trend << endl;;

		}


	}

}


void CS36Dlg::OnBnClickedsetshort()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(true);
	CString str2 = dlg_list_tasklist.GetItemText(activeListID, 0);
	int taskid = _ttoi(str2);

	//cout << "taskid" << taskid << endl;

	for (vector<Strategy* >::iterator itx = allRunningTasks.begin(); itx != allRunningTasks.end(); itx++)
	{
		if ((*itx)->nTask == taskid)
		{
			//cout << " frist:" << (*itx)->tupoBuy << endl;;
			(*itx)->do_trend = -1;
			cout << "全天只做空:" << (*itx)->do_trend << endl;;

		}


	}
}





void CS36Dlg::OnBnClickedButtontestfunc()
{
	// TODO:  测试按钮
	for (vector<Strategy* >::iterator itx = allRunningTasks.begin(); itx != allRunningTasks.end(); itx++)
	{
		//(*itx)->send_hang(2,30,  1,  11);
		//cout << "  历史价" << (*itx)->KData_360[(*itx)->KData_360.size() - 1].lowPrice << " da   " << (*itx)->KData_360.size() << endl;

		//(*itx)->update_kline(888, (*itx)->KData_360);
		//(*itx)->creat_Kline(666, 360,(*itx)->KData_360);

		//cout << "atr:" << (*itx)->result_15.ATR << " kdj: " << (*itx)->result_15.KDJ << " macd " << (*itx)->result_15.MACD << endl;

		//(*itx)->run_tech_lib((*itx)->KData_15, (*itx)->result_15);

		//(*itx)->HFT_set_order(1,  30,  true);
		(*itx)->run_tech_lib((*itx)->KData_1m);
		//(*itx)->send_hang(1, (*itx)->tickData.BidPrice1, 1, 5);
		//Sleep(20);
		for (vector<Strategy::Kdata>::iterator it = (*itx)->KData_1m.begin(); it< (*itx)->KData_1m.end(); it++)
		{
			cout<< it->openPrice << "," << it->highPrice << "," << it->lowPrice << "," << it->closePrice << "," << it->macd_diff << "," << it->macd_dea << "," << it->macd_sign << endl;
		}
		
	}

}


void CS36Dlg::OnBnClickedButtonunlockorder()
{
	// TODO:  手动解锁，默认10手
	
	for (vector<Strategy* >::iterator itx = allRunningTasks.begin(); itx != allRunningTasks.end(); itx++)
	{
		if ((*itx)->holdVolumeLong >= 10 && (*itx)->holdVolumeShort >=10)
		{
			(*itx)->set_close_long((*itx)->tickData.AskPrice1, 10, FALSE, 66, 0);//价格，数量，默认FAK，订单类型（决定是否继续开仓等），反向单编号
			(*itx)->set_close_short((*itx)->tickData.BidPrice1, 10, FALSE, 66, 0);

		}
		else
		{
			cout << "可平仓位不足" << endl;
		}
		

	}
	

}


void CS36Dlg::OnBnClickedButtontest()
{
	// TODO: 测试按钮
	for (vector<Strategy* >::iterator itx = allRunningTasks.begin(); itx != allRunningTasks.end(); itx++)
	{
		//(*itx)->hft_test_order() ;
		//(*itx)->set_close_long((*itx)->tickData.BidPrice1, 1, false, 10, 0);
		//cout << "多： " << (*itx)->hft_order_long.size() << "," << "空： " << (*itx)->hft_order_short.size() << endl;
		cout << "手动开多首开： " << endl;


		//
		(*itx)->set_open_long((*itx)->tickData.AskPrice1, (*itx)->frist_volume, true, 1, 0);

	}



}
