#pragma once


// taskEditorDialog �Ի���

class taskEditorDialog : public CDialogEx
{
	DECLARE_DYNAMIC(taskEditorDialog)

public:
	taskEditorDialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~taskEditorDialog();

// �Ի�������
	enum { IDD = IDD_DIALOG_Edit };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	int dlg_edit_openvolume;
	double dlg_edit_threshold;
	double dlg_edit_threshold2;
	afx_msg void OnBnClickedButtonmodify();
	virtual BOOL OnInitDialog();
};
