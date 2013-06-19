// LLKChangePwd.cpp : implementation file
//

#include "stdafx.h"
#include "LLKCLI.h"
#include "LLKChangePwd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLLKChangePwd dialog


CLLKChangePwd::CLLKChangePwd(CWnd* pParent /*=NULL*/)
	: CDialog(CLLKChangePwd::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLLKChangePwd)
	m_pwd = _T("");
	m_repwd = _T("");
	m_repwd2 = _T("");
	m_user = _T("");
	//}}AFX_DATA_INIT
}


void CLLKChangePwd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLLKChangePwd)
	DDX_Text(pDX, IDC_EDIT_PWD, m_pwd);
	DDV_MaxChars(pDX, m_pwd, 6);
	DDX_Text(pDX, IDC_EDIT_REPWD, m_repwd);
	DDV_MaxChars(pDX, m_repwd, 6);
	DDX_Text(pDX, IDC_EDIT_REPWD2, m_repwd2);
	DDV_MaxChars(pDX, m_repwd2, 6);
	DDX_Text(pDX, IDC_EDIT_USER, m_user);
	DDV_MaxChars(pDX, m_user, 6);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLLKChangePwd, CDialog)
	//{{AFX_MSG_MAP(CLLKChangePwd)
	ON_MESSAGE(NET_CHANGEPWD,OnChangePwd)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLLKChangePwd message handlers

void CLLKChangePwd::OnOK() 
{
	UpdateData();
	if(m_user==""){
		MessageBox("�û�������Ϊ�գ�");
		return;
	}
	else if(m_pwd==""){
		MessageBox("ԭ���벻��Ϊ�գ�");
		return;
	}
	else if (m_repwd=="")
	{
		MessageBox("�����벻��Ϊ�գ�");
		return;
	}
	else if (m_repwd!=m_repwd2)
	{
		MessageBox("����ȷ�ϲ�һ�£�");
		return;
	}
	
	//�����û�������
	CLLKCLIApp* pApp = (CLLKCLIApp*)AfxGetApp();
	sockaddr_in remotAddr;
	remotAddr.sin_family = AF_INET;
	remotAddr.sin_addr.S_un.S_addr = inet_addr(pApp->ip);
	remotAddr.sin_port = htons(pApp->port);
	CString buf;
	buf.Format("3 %s %s %s",m_user,m_pwd,m_repwd);
	
	CSocket *pSock = pApp->m_pSockThd->GetSock();
	int res;
	res = sendto(pSock->m_hSocket,buf.GetBuffer(buf.GetLength()),buf.GetLength(),0,(SOCKADDR*)&remotAddr,sizeof(SOCKADDR));
}

BOOL CLLKChangePwd::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CLLKCLIApp* pApp = (CLLKCLIApp*)AfxGetApp();
	pApp->m_pSockThd->SetHwnd(m_hWnd);	
	// TODO: Add extra initialization here

	m_bmpChangePwdBg.LoadBitmap(IDB_ADDUSER_BG);
	m_bmpChangePwdBg.GetBitmap(&m_infoBmpChangePwdBg);
	CDC *pDC = GetDC();
	
	m_dcMem.CreateCompatibleDC(pDC);
	m_dcBuf.CreateCompatibleDC(pDC);
	
	m_dcMem.SelectObject(&m_bmpChangePwdBg);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CLLKChangePwd::OnChangePwd(WPARAM w, LPARAM l){
	char *data = (char*)w;
	if(strcmp(data,"success")==0){
		MessageBox("�޸ĳɹ�");
	}
	else if(strcmp(data,"wrong")==0){
		MessageBox("�û������������");
	}
	else if(strcmp(data,"fail")==0){
		MessageBox("�޸�ʧ��");
	}
	delete[] data;
	CDialog::OnOK();
	return 0;
}

HBRUSH CLLKChangePwd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if(pWnd->GetDlgCtrlID()==IDC_NAME)
	{
		pDC->SetBkMode(TRANSPARENT);//���ñ���͸��
		hbr=(HBRUSH)GetStockObject(NULL_BRUSH);  
	} 
	if(pWnd->GetDlgCtrlID()==IDC_PWDOLD)
	{
		pDC->SetBkMode(TRANSPARENT);//���ñ���͸��
		hbr=(HBRUSH)GetStockObject(NULL_BRUSH);  
	} 
	if(pWnd->GetDlgCtrlID()==IDC_PWDNEW)
	{
		pDC->SetBkMode(TRANSPARENT);//���ñ���͸��
		hbr=(HBRUSH)GetStockObject(NULL_BRUSH);  
	} 
	if(pWnd->GetDlgCtrlID()==IDC_PWDTWICE)
	{
		pDC->SetBkMode(TRANSPARENT);//���ñ���͸��
		hbr=(HBRUSH)GetStockObject(NULL_BRUSH);  
	} 
	return hbr;
}

void CLLKChangePwd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	dc.BitBlt(0,0,m_infoBmpChangePwdBg.bmWidth,m_infoBmpChangePwdBg.bmHeight,&m_dcMem,0,0,SRCCOPY);
	
	// Do not call CDialog::OnPaint() for painting messages
}
