// LLKBgReady.cpp : implementation file
//

#include "stdafx.h"
#include "LLKCLI.h"
#include "LLKBgReady.h"
#include "LLKGame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int count;
char userNick[6][100];
/////////////////////////////////////////////////////////////////////////////
// CLLKBgReady dialog

CLLKBgReady::CLLKBgReady(CWnd* pParent /*=NULL*/)
	: CDialog(CLLKBgReady::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLLKBgReady)
	m_Record = _T("");
	m_MyEnter = _T("");
	//}}AFX_DATA_INIT
	for(int i=0;i<6;i++){
		userIds[i]=0;
	}
	count=0;
}


void CLLKBgReady::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLLKBgReady)
	DDX_Control(pDX, IDC_EDIT_OUT, m_RCDCon);
	DDX_Text(pDX, IDC_EDIT_OUT, m_Record);
	DDX_Text(pDX, IDC_EDIT_ENTER, m_MyEnter);
	DDV_MaxChars(pDX, m_MyEnter, 200);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLLKBgReady, CDialog)
	//{{AFX_MSG_MAP(CLLKBgReady)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_TEST, OnTest)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_MESSAGE(NET_OUTTABLE,OnChangeTable)
	ON_MESSAGE(NET_READY,OnChangeReady)
	ON_MESSAGE(NET_USERINFO_READY,OnUserInfoReady)
	ON_MESSAGE(NET_GAME_START,OnGameStart)
	ON_MESSAGE(NET_TALKMSG,RecvMsg)
	ON_BN_CLICKED(IDC_READY, OnReady)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLLKBgReady message handlers

void CLLKBgReady::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	dc.BitBlt(0,0,m_infoBmpBgReady.bmWidth,m_infoBmpBgReady.bmHeight,&m_dcMem,0,0,SRCCOPY);

 	CString info;
 	info.Format("%s",c_nick);

	CFont font;
	font.CreatePointFont(90,"����",&dc);
	dc.SelectObject(&font);
	dc.SetTextColor(RGB(255,0,0));

	dc.SetBkMode(TRANSPORT_TYPE_CN);
 	dc.TextOut(700,93,info);
	char strScore[100];
	itoa(c_score,strScore,10);
	info.Format("%s",strScore);
	dc.TextOut(700,109,info);
	CString level;
	if(c_score<100){
		level.Format("������");	
	}
	else if(c_score<500){
		level.Format("������");
	}
	else if(c_score<1000){
		level.Format("��ֻ��");
	}
	else{
		level.Format("���۽�");
	}
	dc.TextOut(700,124,level);
}

BOOL CLLKBgReady::OnInitDialog() 
{
	CDialog::OnInitDialog();

 	CLLKCLIApp* pApp = (CLLKCLIApp*)AfxGetApp();
  	pApp->m_pSockThd->SetHwnd(m_hWnd);	
	pApp->m_pSockThd->readyMsg=m_hWnd;

	m_bmpBgReady.LoadBitmap(IDB_READY_BG);
	m_bmpBgTmp.LoadBitmap(IDB_READY_BG);
	m_bmpBgReady3.LoadBitmap(IDB_READY_BG);
	m_bmpReadyPlayer.LoadBitmap(IDB_READY_PLAYER);
	m_bmpTool.LoadBitmap(IDB_GAME_TOOL);
	m_bmpBgReady2.LoadBitmap(IDB_READY_BG2);

	m_bmpBgReady.GetBitmap(&m_infoBmpBgReady);
	m_bmpReadyPlayer.GetBitmap(&m_infoBmpReadyPlayer);
	m_bmpTool.GetBitmap(&m_infoBmpTool);
	m_bmpBgReady2.GetBitmap(&m_infoBmpBgReady2);

	CDC *pDC = GetDC();
	
	m_dcMem.CreateCompatibleDC(pDC);
	m_dcBuf.CreateCompatibleDC(pDC);
	
	m_dcMem.SelectObject(&m_bmpBgReady);
	
	RECT rc;
	GetWindowRect(&rc);//�õ�������Ϣ
	
	int cyCaption = GetSystemMetrics(SM_CYCAPTION);//�˵��߶�
	
	MoveWindow(rc.left,rc.top,m_infoBmpBgReady.bmWidth,m_infoBmpBgReady.bmHeight+cyCaption);//�������ڴ�С

	isReady=false;

	int	n=5;
	Tool(n);//��ʾ����
	
	//ShowName(chaNum,true);//��ʾ�ǳ�
	
 	SetTimer(1,500,NULL);
 	//SetTimer(2,500,NULL);
	
	StartButton.LoadBitmaps(IDB_GAME_START,IDB_GAME_START_DOWN,0,0);
	StartButton.SubclassDlgItem(IDC_START,this);
	StartButton.SizeToContent();

	TestButton.LoadBitmaps(IDB_GAME_TEST,IDB_GAME_TEST_DOWN,0,0);
	TestButton.SubclassDlgItem(IDC_TEST,this);
	TestButton.SizeToContent();

	CloseButton.LoadBitmaps(IDB_GAME_CLOSE,IDB_GAME_CLOSE_DOWN,0,0);
	CloseButton.SubclassDlgItem(IDC_CANCEL,this);
	CloseButton.SizeToContent();

	//�����û���Ϣ
	sockaddr_in remotAddr;
	remotAddr.sin_family = AF_INET;
	remotAddr.sin_addr.S_un.S_addr = inet_addr(pApp->ip);
	remotAddr.sin_port = htons(pApp->port);
	CString buf;
	buf.Format("7 %d",c_uid);
	
	CSocket *pSock = pApp->m_pSockThd->GetSock();
	int res;
	res = sendto(pSock->m_hSocket,buf.GetBuffer(buf.GetLength()),buf.GetLength(),0,(SOCKADDR*)&remotAddr,sizeof(SOCKADDR));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLLKBgReady::OnLButtonDown(UINT nFlags, CPoint point) {
	CDialog::OnLButtonDown(nFlags, point);
}
/************************************************************************/
/* �޸��û�׼��ͷ��                                                     */
/************************************************************************/
void CLLKBgReady::ChangeReadyHead(int cn,boolean isMan,boolean userReady,boolean isIn){
	int x,y;
	int left=186;
	int top=197;
	if (cn==1)
	{
		x=left;
		y=top;
	}
	else if (cn==2)
	{
		x=left+m_infoBmpReadyPlayer.bmWidth/4+20;
		y=top;
	}
	else if (cn==3)
	{
		x=left+2*(m_infoBmpReadyPlayer.bmWidth/4+20);
		y=top;
		
	}
	else if (cn==4)
	{
		x=left;
		y=top+m_infoBmpReadyPlayer.bmHeight+45;
	}
	else if (cn==5)
	{
		x=left+m_infoBmpReadyPlayer.bmWidth/4+20;
		y=top+m_infoBmpReadyPlayer.bmHeight+45;
	}
	else if (cn==6)
	{
		x=x=left+2*(m_infoBmpReadyPlayer.bmWidth/4+20);
		y=top+m_infoBmpReadyPlayer.bmHeight+45;
	}
	m_dcBuf.SelectObject(&m_bmpReadyPlayer);
	if(isIn){
		if(userReady){
			if(isMan){
				m_dcMem.BitBlt(x,y,m_infoBmpReadyPlayer.bmWidth/4,m_infoBmpReadyPlayer.bmHeight,&m_dcBuf,m_infoBmpReadyPlayer.bmWidth/2,0,SRCCOPY);
			}
			else{
				m_dcMem.BitBlt(x,y,m_infoBmpReadyPlayer.bmWidth/4,m_infoBmpReadyPlayer.bmHeight,&m_dcBuf,m_infoBmpReadyPlayer.bmWidth/4*3,0,SRCCOPY);
			}
		}
		else{
			if(isMan){
				m_dcMem.BitBlt(x,y,m_infoBmpReadyPlayer.bmWidth/4,m_infoBmpReadyPlayer.bmHeight,&m_dcBuf,0,0,SRCCOPY);
			}
			else{
				m_dcMem.BitBlt(x,y,m_infoBmpReadyPlayer.bmWidth/4,m_infoBmpReadyPlayer.bmHeight,&m_dcBuf,m_infoBmpReadyPlayer.bmWidth/4,0,SRCCOPY);
			}
		}
	}
	else{
		m_dcBuf.SelectObject(&m_bmpBgReady3);
		m_dcMem.BitBlt(x,y-20,m_infoBmpReadyPlayer.bmWidth/4,m_infoBmpReadyPlayer.bmHeight+20,&m_dcBuf,x,y-20,SRCCOPY);
	}
	Invalidate(false);
	UpdateWindow();
}
/************************************************************************/
/* ����                                                                  */
/************************************************************************/
void CLLKBgReady::Tool(int cn)
{
	int x=619,y=190;
	m_dcBuf.SelectObject(&m_bmpTool);
	if (cn==1)
	{
		m_dcMem.BitBlt(x,y,m_infoBmpTool.bmWidth/6,m_infoBmpTool.bmHeight,&m_dcBuf,m_infoBmpTool.bmWidth/6,0,SRCCOPY);
	}
	else if (cn==2)
	{
		m_dcMem.BitBlt(x,y,m_infoBmpTool.bmWidth/6,m_infoBmpTool.bmHeight,&m_dcBuf,m_infoBmpTool.bmWidth/3,0,SRCCOPY);
	}
	else if (cn==3)
	{
		m_dcMem.BitBlt(x,y,m_infoBmpTool.bmWidth/6,m_infoBmpTool.bmHeight,&m_dcBuf,m_infoBmpTool.bmWidth/2,0,SRCCOPY);
	}
	else if (cn==4)
	{
		m_dcMem.BitBlt(x,y,m_infoBmpTool.bmWidth/6,m_infoBmpTool.bmHeight,&m_dcBuf,m_infoBmpTool.bmWidth/6*4,0,SRCCOPY);
	}
	else if (cn==5)
	{
		m_dcMem.BitBlt(x,y,m_infoBmpTool.bmWidth/6,m_infoBmpTool.bmHeight,&m_dcBuf,m_infoBmpTool.bmWidth/6*5,0,SRCCOPY);
	}	
	else if (cn==0)
	{
		m_dcMem.BitBlt(x,y,m_infoBmpTool.bmWidth/6,m_infoBmpTool.bmHeight,&m_dcBuf,0,0,SRCCOPY);
	}
	Invalidate(false);
	UpdateWindow();
}

void CLLKBgReady::OnTimer(UINT nIDEvent) {
	if(nIDEvent==1){
		if(count%2==0){
			m_dcBuf.SelectObject(&m_bmpBgReady2);
		}
		else{
			m_dcBuf.SelectObject(&m_bmpBgTmp);
		}
		m_dcMem.BitBlt(0,20,m_infoBmpBgReady2.bmWidth/5,m_infoBmpBgReady2.bmHeight,&m_dcBuf,0,20,SRCCOPY);
		RECT rc;
		rc.left=0;
		rc.top=0;
		rc.right=m_infoBmpBgReady2.bmWidth/5;
		rc.bottom=m_infoBmpBgReady2.bmHeight;
		InvalidateRect(&rc,FALSE);
		count++;
	}
	CDialog::OnTimer(nIDEvent);
}
/************************************************************************/
/* ��ϰ����                                                             */
/************************************************************************/
void CLLKBgReady::OnTest() 
{
// 	CLLKGame game;
// 	game.DoModal();
	
}
/************************************************************************/
/* �˳�����                                                             */
/************************************************************************/
void CLLKBgReady::OnCancel() 
{
	// TODO: Add your control notification handler code here
 	CLLKCLIApp* pApp = (CLLKCLIApp*)AfxGetApp();
 	sockaddr_in remotAddr;
	remotAddr.sin_family = AF_INET;
	remotAddr.sin_addr.S_un.S_addr = inet_addr(pApp->ip);
	remotAddr.sin_port = htons(pApp->port);
	CString buf;
	buf.Format("4 out %d %d %d",chaNum,tabNum,c_uid);
	
	CSocket *pSock = pApp->m_pSockThd->GetSock();
	int res;
	res = sendto(pSock->m_hSocket,buf.GetBuffer(buf.GetLength()),buf.GetLength(),0,(SOCKADDR*)&remotAddr,sizeof(SOCKADDR));
}
/************************************************************************/
/* �뿪��λ                                                             */
/************************************************************************/
LRESULT CLLKBgReady::OnChangeTable(WPARAM w, LPARAM l){
	char *data = (char*)w;
	int type;
	char action[10],nick[100];
	int tmpTab,tmpCha,uid,sex,userReady;
	sscanf(data,"%d %s",&type,action);
 	if(strcmp(action,"userout")==0){
		sscanf(data,"%d %s %d %d",&type,action,&tmpTab,&tmpCha);
		if(tmpTab==tabNum && tmpCha==chaNum){
			EndDialog(IDCANCEL);
		}
		else{//�������˳���λ
			ChangeReadyHead(tmpCha,true,0,false);
			userIds[tmpCha-1]=0;
			strcpy(userNick[tmpCha-1],"");
		}
 	}
	else if(strcmp(action,"userin")==0){//�����û�����
		sscanf(data,"%d %s %d %s %d %d %d",&type,action,&uid,nick,&sex,&tmpCha,&userReady);
		userIds[tmpCha-1]=uid;
		strcpy(userNick[tmpCha-1],nick);
		ChangeReadyHead(tmpCha,sex,userReady,true);
		ShowName(tmpCha,nick);
	}
	delete[] data;
	return 0;
}
/************************************************************************/
/*��Ӧ�û��ı�׼��״̬��Ϣ                                              */
/************************************************************************/
LRESULT CLLKBgReady::OnChangeReady(WPARAM w, LPARAM l){
	char *data = (char*)w;
	int type,tabNum,chaNum,isMan,uid;
	char action[10];
	sscanf(data,"%d %s %d %d %d %d",&type,action,&tabNum,&chaNum,&isMan,&uid);
	if(strcmp(action,"ready")==0){
		if(uid==c_uid)
			isReady=true;
		ChangeReadyHead(chaNum,isMan,true,true);
	}
	else if(strcmp(action,"notReady")==0){
		if(uid==c_uid)
			isReady=false;
 		ChangeReadyHead(chaNum,isMan,false,true);
 	}
	delete[] data;
	return 0;
}
/************************************************************************/
/* ��Ӧ�û�׼����Ϣ���ս��뷿��ʱ��                                     */
/************************************************************************/
LRESULT CLLKBgReady::OnUserInfoReady(WPARAM w, LPARAM l){
	char *data = (char*)w;
	UserInfo tmpUser;
	char usernick[100];
	sscanf(data,"%d %d %s %d %d %d",&tmpUser.type,&tmpUser.id,usernick,&tmpUser.sex,&tmpUser.chaNum,&tmpUser.isReady);
	ChangeReadyHead(tmpUser.chaNum,tmpUser.sex,tmpUser.isReady,true);
	userIds[tmpUser.chaNum-1]=tmpUser.id;
	strcpy(userNick[tmpUser.chaNum-1],usernick);
	ShowName(tmpUser.chaNum,usernick);
	delete[] data;
	return 0;
}
/************************************************************************/
/* ��Ӧ��Ϸ��ʼ��Ϣ                                                     */
/************************************************************************/
LRESULT CLLKBgReady::OnGameStart(WPARAM w, LPARAM l){
	char *data = (char*)w;
	UserInfo tmpUser;
	int type;
	char action[100];
	sscanf(data,"%d %s",&type,action);
	if(strcmp(action,"start")==0){
		//��Ϸ��ʼ
		CLLKGame game;
		for(int i=0;i<6;i++){
			game.userMap[i].type=userIds[i];
			strcpy(game.userNick[i],userNick[i]);
		}
		game.tabNum=tabNum;
		game.chaNum=chaNum;
		game.m_uid=c_uid;
		game.isLooked=false;
		game.DoModal();
		CLLKCLIApp* pApp = (CLLKCLIApp*)AfxGetApp();
		pApp->m_pSockThd->SetHwnd(m_hWnd);

		sockaddr_in remotAddr;
		remotAddr.sin_family = AF_INET;
		remotAddr.sin_addr.S_un.S_addr = inet_addr(pApp->ip);
		remotAddr.sin_port = htons(pApp->port);
		CString buf;
		buf.Format("6 cancel %d %d %d",tabNum,chaNum,c_sex=="��"?1:0);
		CSocket *pSock = pApp->m_pSockThd->GetSock();
		int res;
		res = sendto(pSock->m_hSocket,buf.GetBuffer(buf.GetLength()),buf.GetLength(),0,(SOCKADDR*)&remotAddr,sizeof(SOCKADDR));


	}
	else if(strcmp(action,"fail")==0){
		//��Ϸ����ʧ��
	//	MessageBox("�����û�δ׼��");
	}
	else if(strcmp(action,"oneuser")==0){
	//	MessageBox("��ǰֻ��һ���û����޷���ʼ");
	}
	ChangeReadyHead(tmpUser.chaNum,tmpUser.sex,tmpUser.isReady,true);
	delete[] data;
	return 0;
}
/************************************************************************/
/* ׼��                                                                 */
/************************************************************************/
void CLLKBgReady::OnReady(){
/*	CLLKCLIApp* pApp = (CLLKCLIApp*)AfxGetApp();
	sockaddr_in remotAddr;
	remotAddr.sin_family = AF_INET;
	remotAddr.sin_addr.S_un.S_addr = inet_addr(pApp->ip);
	remotAddr.sin_port = htons(pApp->port);
	CString buf;
	if(isReady){
		buf.Format("6 cancel %d %d %d",tabNum,chaNum,c_sex=="��"?1:0);
	}
	else{
		buf.Format("6 ready %d %d %d",tabNum,chaNum,c_sex=="��"?1:0);
	}
	CSocket *pSock = pApp->m_pSockThd->GetSock();
	int res;
	res = sendto(pSock->m_hSocket,buf.GetBuffer(buf.GetLength()),buf.GetLength(),0,(SOCKADDR*)&remotAddr,sizeof(SOCKADDR));
	*/
}
/************************************************************************/
/* ��ʼ                                                                 */
/************************************************************************/
void CLLKBgReady::OnStart() {
	CLLKCLIApp* pApp = (CLLKCLIApp*)AfxGetApp();
	sockaddr_in remotAddr;
	remotAddr.sin_family = AF_INET;
	remotAddr.sin_addr.S_un.S_addr = inet_addr(pApp->ip);
	remotAddr.sin_port = htons(pApp->port);
	CString buf;
	if(isReady){
		buf.Format("6 cancel %d %d %d",tabNum,chaNum,c_sex=="��"?1:0);
		CSocket *pSock = pApp->m_pSockThd->GetSock();
		int res;
		res = sendto(pSock->m_hSocket,buf.GetBuffer(buf.GetLength()),buf.GetLength(),0,(SOCKADDR*)&remotAddr,sizeof(SOCKADDR));
	}
	else{
		buf.Format("6 ready %d %d %d",tabNum,chaNum,c_sex=="��"?1:0);
		CSocket *pSock = pApp->m_pSockThd->GetSock();
		int res;
		res = sendto(pSock->m_hSocket,buf.GetBuffer(buf.GetLength()),buf.GetLength(),0,(SOCKADDR*)&remotAddr,sizeof(SOCKADDR));	
		buf.Format("9 %d %d",tabNum,c_uid);
		//	CSocket *pSock = pApp->m_pSockThd->GetSock();
		//	int res;
		res = sendto(pSock->m_hSocket,buf.GetBuffer(buf.GetLength()),buf.GetLength(),0,(SOCKADDR*)&remotAddr,sizeof(SOCKADDR));
	}

}
/************************************************************************/
/* ͷ�����ǳ���ʾ                                                       */
/************************************************************************/
void CLLKBgReady::ShowName(int chaNum,CString nick){
	CString name;
	name.Format("%s",nick);
	int left=216;
	int top=177;
	int x,y;
	if (chaNum==1)
	{
		x=left;
		y=top;
	}
	else if (chaNum==2)
	{
		x=left+m_infoBmpReadyPlayer.bmWidth/4+20;
		y=top;
	}
	else if (chaNum==3)
	{
		x=left+2*(m_infoBmpReadyPlayer.bmWidth/4+20);
		y=top;
		
	}
	else if (chaNum==4)
	{
		x=left;
		y=top+m_infoBmpReadyPlayer.bmHeight+45;
	}
	else if (chaNum==5)
	{
		x=left+m_infoBmpReadyPlayer.bmWidth/4+20;
		y=top+m_infoBmpReadyPlayer.bmHeight+45;
	}
	else if (chaNum==6)
	{
		x=left+2*(m_infoBmpReadyPlayer.bmWidth/4+20);
		y=top+m_infoBmpReadyPlayer.bmHeight+45;
	}
	CFont font;
	font.CreatePointFont(120,"����",&m_dcMem);
	m_dcMem.SelectObject(&font);
	m_dcMem.SetTextColor(RGB(255,0,255));
	m_dcMem.SetBkColor(RGB(0,104,152));
	m_dcMem.TextOut(x,y,name);

	Invalidate(false);
	UpdateWindow();

}
/************************************************************************/
/* ����������Ϣ                                                         */
/************************************************************************/
void CLLKBgReady::SendMsg()
{
	CLLKCLIApp* pApp = (CLLKCLIApp*)AfxGetApp();
	sockaddr_in remotAddr;
	remotAddr.sin_family = AF_INET;
	remotAddr.sin_addr.S_un.S_addr = inet_addr(pApp->ip);
	remotAddr.sin_port = htons(pApp->port);
	UpdateData(true);
	if (m_MyEnter.GetLength()==0)
		return;
	CString buf;
 	buf.Format("21 %d %s:%s",tabNum,c_nick,m_MyEnter);
	if (buf.GetLength()%2!=0)buf+=" ";

 	CSocket *pSock = pApp->m_pSockThd->GetSock();
 	
 	int res = sendto(pSock->m_hSocket,buf.GetBuffer(buf.GetLength()),buf.GetLength(),0,(SOCKADDR*)&remotAddr,sizeof(SOCKADDR));
	
 	m_MyEnter="";
	UpdateData(false);
}
/************************************************************************/
/* ��Ӧ��ʾ������Ϣ                                                     */
/************************************************************************/
LRESULT CLLKBgReady::RecvMsg(WPARAM w, LPARAM l)
{
	char *data = (char*)w;
	int type,len;
	CString talkMsg;
	char result[7]="";
//	sscanf(data,"%s",talkMsg);
	sscanf(data,"%d %s %s",&type,result,talkMsg);
	
	if(strcmp(result,"success")==0){

		char *p=(char *)(LPCTSTR)talkMsg;
		len=strlen(p);
	//	len=talkMsg.GetLength();
		CString temp;
		char *buf=new char[len+1];
		memcpy(buf,talkMsg,len);
		buf[len]='\0';
		temp=buf;
		m_Record+=temp;
		m_Record+="\r\n";
		m_MyEnter="";
		UpdateData(false);
		m_RCDCon.SetSel(0,-1);

	
		//MessageBox(talkMsg);
	}
	delete []data;
	return 0;
}

/************************************************************************/
/* ��ӦENTER,ESC                                                        */
/************************************************************************/
BOOL   CLLKBgReady::PreTranslateMessage(MSG*   pMsg)   
{ 
	if   (pMsg-> message   ==   WM_KEYDOWN   &&   pMsg-> wParam   ==   VK_ESCAPE) 

		OnCancel();

	else if (pMsg-> message   ==   WM_KEYDOWN   &&   pMsg-> wParam   ==   VK_RETURN)
		{
			SendMsg();
			return   TRUE; 
		}
	return   CDialog::PreTranslateMessage(pMsg); 
}
	
