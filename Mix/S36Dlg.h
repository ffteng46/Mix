
// S36Dlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "resource.h"
#include "afxcmn.h"


// CS36Dlg dialog
class CS36Dlg : public CDialogEx
{
// Construction
public:
	CS36Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_S36_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl dlg_list_tasklist;
//	CString dlg_add_code;
//	CString dlg_add_exchangecode;
//	CString dlg_add_instrument;
	int dlg_add_openvolume;
//	double dlg_add_threshold;
//	double dlg_add_threshold2;
	afx_msg void OnBnClickedButtonaddtask();
	afx_msg void OnBnClickedButtonstart();
	afx_msg void OnBnClickedButtonstop();
	afx_msg void OnNMRClickListtasklist(NMHDR *pNMHDR, LRESULT *pResult);

public:
	int taskCounter;
	int activeListID;

	afx_msg void editTask();
	afx_msg void deleteTask();

	
	afx_msg void OnBnClickedButtonforceclose();
	CString dlg_add_instrumentB;
//	int dlg_add_grid_reversal;
	int dlg_add_setlong;
	afx_msg void OnBnClickedsetlong();
	afx_msg void OnBnClickedsetshort();
//	int dlg_add_breaktime;
	
	afx_msg void OnBnClickedButtontestfunc();
	afx_msg void OnBnClickedButtonunlockorder();
	afx_msg void OnBnClickedButtontest();
	// 反向单的间距，国债是6跳
	int dlg_add_hang_frequency;
	// 反向单深度
	int dlg_add_hang_depth;
	double dlg_add_dual_trust;
};
