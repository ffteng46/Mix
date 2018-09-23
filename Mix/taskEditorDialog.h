#pragma once


// taskEditorDialog 对话框

class taskEditorDialog : public CDialogEx
{
	DECLARE_DYNAMIC(taskEditorDialog)

public:
	taskEditorDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~taskEditorDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_Edit };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int dlg_edit_openvolume;
	double dlg_edit_threshold;
	double dlg_edit_threshold2;
	afx_msg void OnBnClickedButtonmodify();
	virtual BOOL OnInitDialog();
};
