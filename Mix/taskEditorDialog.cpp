// taskEditorDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "S36.h"
#include "taskEditorDialog.h"
#include "afxdialogex.h"
#include "S36Dlg.h"
#include "Strategy.h"
#include <vector>
extern vector<Strategy* > allRunningTasks;
using namespace std;


// taskEditorDialog 对话框
IMPLEMENT_DYNAMIC(taskEditorDialog, CDialogEx)

taskEditorDialog::taskEditorDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(taskEditorDialog::IDD, pParent)
	, dlg_edit_openvolume(0)
	, dlg_edit_threshold(0)
	, dlg_edit_threshold2(0)
{

}

taskEditorDialog::~taskEditorDialog()
{
}

void taskEditorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_openvolume, dlg_edit_openvolume);
	DDX_Text(pDX, IDC_EDIT_threshold, dlg_edit_threshold);
	DDX_Text(pDX, IDC_EDIT_threshold2, dlg_edit_threshold2);
}



BEGIN_MESSAGE_MAP(taskEditorDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_modify, &taskEditorDialog::OnBnClickedButtonmodify)
END_MESSAGE_MAP()


// taskEditorDialog 消息处理程序


void taskEditorDialog::OnBnClickedButtonmodify()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(true);

	CS36Dlg* pParentclass = (CS36Dlg*)GetParent();
	CString str2 = pParentclass->dlg_list_tasklist.GetItemText(pParentclass->activeListID, 0);
	int taskid = _ttoi(str2);
	for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
	{
		if ((*it)->nTask == taskid)
		{
			(*it)->openVolume = dlg_edit_openvolume;
			(*it)->threshold= dlg_edit_threshold;
			(*it)->threshold2 = dlg_edit_threshold2;
		}
	}

	taskEditorDialog::OnOK();

}


BOOL taskEditorDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CS36Dlg* pParentclass = (CS36Dlg*)GetParent();
	CString str2 = pParentclass->dlg_list_tasklist.GetItemText(pParentclass->activeListID, 0);
	int taskid = _ttoi(str2);

	for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
	{
		if ((*it)->nTask == taskid)
		{
			dlg_edit_openvolume = (*it)->openVolume;
			dlg_edit_threshold = (*it)->threshold;
			dlg_edit_threshold2 = (*it)->threshold2;

			UpdateData(false);
		}
	}



	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}
