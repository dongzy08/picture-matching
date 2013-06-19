// SockThd.cpp : implementation file
//

#include "stdafx.h"
#include "SockDemo.h"
#include "SockThd.h"
#include "JAdo.h"
#include <strstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSockThd
int	userList[15][6];


IMPLEMENT_DYNCREATE(CSockThd, CWinThread)

CSockThd::CSockThd()
{
	for(int i=0;i<15;i++){
		tableState.tableOn[i]=-1;
	}
}

CSockThd::~CSockThd()
{
}

BOOL CSockThd::InitInstance(){

	m_sock.Create(5600,SOCK_DGRAM);
	return TRUE;
}

int CSockThd::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CSockThd, CWinThread)
	//{{AFX_MSG_MAP(CSockThd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSockThd message handlers



int CSockThd::Run() {
	sockaddr_in remoteAddr; //�Է��ĵ�ַ
	int len = sizeof(sockaddr_in);
	int recvLen = 0;

	while(1){
		char data[1024] = {0};
		recvLen = m_sock.ReceiveFrom(data,sizeof(data),(SOCKADDR*)&remoteAddr,&len);
		TRACE("data=%s\n",data);
		if(recvLen > 0){
			int type;
			sscanf(data,"%d",&type);//�����Ϣ����

			if(type==1){//��½��Ϣ
				CString str;
				int uid;
				char password[6]="";
				sscanf(data,"%d %d %s",&type,&uid,password);
				UserInfo ui = GetUserById(uid);
				ui.address=remoteAddr;
				ui.isReady=0;
				if(uid==ui.id && strcmp(password,ui.password)==0){
					
					map<int,UserInfo>::iterator u_it;
					u_it=clients.find(uid);
					if(u_it==clients.end()){
						clients.insert(pair<int,UserInfo>(ui.id,ui));
						str.Format("1 success %d %s %d %d",ui.id,ui.nick,ui.sex,ui.type);
					}
					else{
						str.Format("1 hasLogin");
					}
					strcpy(data,str);
				}
				else{
					strcpy(data,"1 fail");
				}
				m_sock.SendTo(data,strlen(data)+1,(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
			}
			else if(type==2){//ע����Ϣ
				UserInfo ui;
				char nick[6]="",pwd[6]="";
				sscanf(data,"%d %s %s %d",&type,nick,pwd,&ui.sex);
				ui.nick.Format("%s",nick);
				ui.password.Format("%s",pwd);
				int res = AddUser(ui);
				if(res<=0){
					strcpy(data,"2 fail");
				}
				else {
					CString str;
					str.Format("2 success %d",res);
					strcpy(data,str);
				}
				m_sock.SendTo(data,strlen(data)+1,(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
			}
			else if(type==3){//�޸�������Ϣ
				int uid;
				char password[6]="",repassword[6]="";
				sscanf(data,"%d %d %s %s",&type,&uid,password,repassword);
				CString strPwd,strRePwd;
				strPwd.Format("%s",password);
				strRePwd.Format("%s",repassword);
				int res = ChangePassword(uid,strPwd,strRePwd);
				if(res==-1){
					strcpy(data,"3 fail");
				}
				else if(res==0){
					strcpy(data,"3 wrong");
				}
				else{
					strcpy(data,"3 success");
				}
				m_sock.SendTo(data,strlen(data)+1,(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
			}
			else if(type==4){
				int tabNum,chaNum,uid;
				char action[10];
				CString msg("");
				sscanf(data,"%d %s %d %d %d",&type,action,&chaNum,&tabNum,&uid);
				map<int,UserInfo>::iterator it;
				it=clients.find(uid);	
				UserInfo ui = it->second;
				if(strcmp(action,"in")==0){//����������Ϣ
					if(userList[tabNum-1][chaNum-1]==0){//�ɹ�
						userList[tabNum-1][chaNum-1]=uid;
						it->second.tabNum=tabNum;
						it->second.chaNum=chaNum;
						strcpy(data,"4 success");
						m_sock.SendTo(data,strlen(data)+1,(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
						
						//�㲥������Ϣ
						msg.Format("4 userin %d %d %d %s",chaNum,tabNum,uid,it->second.nick);
						char tmpMsg[1024];
						strcpy(tmpMsg,msg);
						SendMsgToClients(tmpMsg);
						//�㲥������Ϣ
						msg.Format("5 userin %d %s %d %d %d",ui.id,ui.nick,ui.sex,chaNum,ui.isReady);
						strcpy(tmpMsg,msg);
						SendMsgToClients(tabNum,tmpMsg);
					}
					else{//ʧ��
						strcpy(data,"4 fail");
						m_sock.SendTo(data,strlen(data)+1,(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
					}
				}
				else if(strcmp(action,"out")==0){//�뿪������Ϣ
					CString tmp;
					tmp.Format("5 userout %d %d",tabNum,chaNum);
					strcpy(data,tmp);
					SendMsgToClients(tabNum,data);

					userList[tabNum-1][chaNum-1]=0;
					it->second.tabNum=0;
					it->second.chaNum=0;
					it->second.isReady=0;
					msg.Format("4 userout %d %d %d %s",chaNum,tabNum,uid,it->second.nick);
					//�㲥
					char tmpMsg[1024];
					strcpy(tmpMsg,msg);
					SendMsgToClients(tmpMsg);
				}
			}
			else if (type==6){//��Ϸ׼����Ϣ
				int chaNum,tabNum,isMan;
				char isReady[10];
				sscanf(data,"%d %s %d %d %d",&type,isReady,&tabNum,&chaNum,&isMan);
				int uid=userList[tabNum-1][chaNum-1];
				CString msg;
				map<int,UserInfo>::iterator it;
				it= clients.find(uid);
				UserInfo ui = it->second;
				if(strcmp(isReady,"cancel")==0){
					it->second.isReady=0;
					msg.Format("6 notReady %d %d %d %d",tabNum,chaNum,isMan,uid);
				}
				else if(strcmp(isReady,"ready")==0){
					it->second.isReady=1;
					msg.Format("6 ready %d %d %d %d",tabNum,chaNum,isMan,uid);
				}
				//��׼����Ϣ��ͬ��
				char tmpMsg[1024]={0};
				strcpy(tmpMsg,msg);
				SendMsgToClients(tabNum,tmpMsg);
			}
			else if(type==7){//����ͬ���û���Ϣ
				int uid;
				sscanf(data,"%d %d",&type,&uid);
				map<int,UserInfo>::iterator it;
				it= clients.find(uid);
				UserInfo ui = it->second;
				SendSameTableUserInfo(ui);
			}
			else if(type==8){//���������û���Ϣ
				int uid;
				sscanf(data,"%d %d",&type,&uid);
				map<int,UserInfo>::iterator it;
				it= clients.find(uid);
				UserInfo ui = it->second;
				SendAllUserInfo(ui);
			}
			else if(type==9){//��Ϸ��ʼ����
				int tabNum,uid;
				sscanf(data,"%d %d %d",&type,&tabNum,&uid);
				SendGameStartMsg(tabNum,uid);
			}
			else if(type==10){//�뿪��Ϸ����
				int uid;
				sscanf(data,"%d %d",&type,&uid);
				clients.erase(uid);//ɾ���û���Ϣ
				strcpy(data,"10 exit");
				m_sock.SendTo(data,strlen(data)+1,(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
			}
			else if(type==11){//�����������
				int uid;
				sscanf(data,"%d %d",&type,&uid);
				clients[uid].mapInfo.type=11;
				m_sock.SendTo((char*)&clients[uid].mapInfo,sizeof(MapInfo),(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
			}
			else if(type==12){//��Ϸ������Ϣ
				int uid,isFail;
				char action[10];
				sscanf(data,"%d %s %d",&type,action,&uid);
				if(strcmp(action,"fail")==0){
					isFail=1;
				}
				else if(strcmp(action,"win")==0){
					isFail=0;
				}
				else if(strcmp(action,"exit")==0){
					isFail=-1;
				}
				SendGameOverMsg(uid,isFail);
			}
			else if(type==13){//����������Ϣ
				int uid,x1,y1,x2,y2;
				sscanf(data,"%d %d %d %d %d %d",&type,&uid,&x1,&y1,&x2,&y2);
				DealBoxClearMsg(uid,x1,y1,x2,y2);
			}
			else if(type==14){//��������
				int uid;
				sscanf(data,"%d %d",&type,&uid);
				DealRedoMapRequest(uid);
			}
			else if (type==16){//���濪ʼ�������
				int uid;
				sscanf(data,"%d %d",&type,&uid);
				tableState.type=16;
				m_sock.SendTo((char*)&tableState,sizeof(TableState),(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
			}
			else if(type==18){//�Թ�������Ϣ
				int uid,tabNum,chaNum;
				char action[10];
				sscanf(data,"%d %s %d %d %d",&type,action,&chaNum,&tabNum,&uid);
				int lookedUid = userList[tabNum-1][chaNum-1];
				clients[lookedUid].onlookers.push_back(uid);
				CString msg;
				msg.Format("18 success %d %d",lookedUid,tabNum);
				char buf[1024];
				strcpy(buf,msg);
				m_sock.SendTo(buf,sizeof(buf),(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
			}
			else if(type==19){//����ǰ�������
				int type,lookerUid,tabNum;
				sscanf(data,"%d %d %d",&type,&lookerUid,&tabNum);
				for(int i=0;i<6;i++){
					if(userList[tabNum-1][i]!=0){
						int tmpUid=userList[tabNum-1][i];
						MapInfo mapInfo = clients[tmpUid].mapInfo;
						mapInfo.type=19;
						mapInfo.val=tmpUid;
						m_sock.SendTo((char*)&mapInfo,sizeof(MapInfo),(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
					}
				}
			}
			else if(type==20){//�Թ����˳�
				int uid,lookedId;
				char action[10];
				sscanf(data,"%d %s %d %d",&type,action,&lookedId,&uid);
				clients[uid].onlookers.remove(lookedId);
				CString msg;
				msg.Format("20 success");
				char buf[1024];
				strcpy(buf,msg);
				m_sock.SendTo(buf,sizeof(buf),(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
			}
			else if (type==21){//����
				int tabNum;CString talkMsg;
				sscanf(data,"%d %d %s:%s",&type,&tabNum,talkMsg);
				CString msg;
				msg.Format("21 success %s",talkMsg);
				//char buf[1024];
				//strcpy(buf,msg);
				//m_sock.SendTo(buf,sizeof(buf),(sockaddr*)&remoteAddr,sizeof(SOCKADDR));
				//int m=msg.GetLength();
				//char * tmpMsg=new char[m];
				
				char tmpMsg[1024]={0};
				strcpy(tmpMsg,msg);
				SendMsgToClients(tabNum,tmpMsg);
			}
		}
		else{//������
			int errCode = GetLastError();
			if(errCode == 10054){
				continue;
			}
			break;
		}
	}
	return 0;
}
/************************************************************************/
/* ��������������Ϣ                                                     */
/************************************************************************/
void CSockThd::DealRedoMapRequest(int uid){
	RedoMap(clients[uid].mapInfo.recMap);
	clients[uid].mapInfo.type=14;
	clients[uid].mapInfo.val=clients[uid].chaNum;
	int tabNum=clients[uid].tabNum;
	//�㲥
	for(int i=0;i<6;i++){
		if(userList[tabNum-1][i]!=0){
			int tmpUid=userList[tabNum-1][i];
			m_sock.SendTo((char*)&clients[uid].mapInfo,sizeof(MapInfo),(sockaddr*)&clients[tmpUid].address,sizeof(SOCKADDR));
			list<int>::iterator it;
			for ( it=clients[tmpUid].onlookers.begin() ; it != clients[tmpUid].onlookers.end(); it++ ){
				int lookerId=*it;
				m_sock.SendTo((char*)&clients[uid].mapInfo,sizeof(MapInfo),(sockaddr*)&clients[lookerId].address,sizeof(SOCKADDR));
			}
		}
	}
}
/************************************************************************/
/* ������������Ϣ                                                     */
/************************************************************************/
void CSockThd::DealBoxClearMsg(int uid,int x1,int y1,int x2,int y2){
	clients[uid].mapInfo.recMap[x1][y1]=0;
	clients[uid].mapInfo.recMap[x2][y2]=0;
	int chaNum=clients[uid].chaNum;
	int tabNum=clients[uid].tabNum;
	CString buf;
	buf.Format("13 %d %d %d %d %d",chaNum,x1,y1,x2,y2);
	//�㲥
	for(int i=0;i<6;i++){
		if(userList[tabNum-1][i]!=0){
			int tmpUid=userList[tabNum-1][i];
			CString str;
			char msg[1024];
			strcpy(msg,buf);
			m_sock.SendTo(msg,sizeof(msg),(sockaddr*)&clients[tmpUid].address,sizeof(SOCKADDR));
			//������Ϣ���Թ���
			list<int>::iterator it;
			for ( it=clients[tmpUid].onlookers.begin() ; it != clients[tmpUid].onlookers.end(); it++ ){
				int lookerId=*it;
				m_sock.SendTo(msg,sizeof(msg),(sockaddr*)&clients[lookerId].address,sizeof(SOCKADDR));
			}
		}
	}
}
/************************************************************************/
/* ������Ϸ������Ϣ                                                     */
/************************************************************************/
void CSockThd::SendGameOverMsg(int uid,int isFail){
	CString buf;
	CString str;
	RankListMsg rankListMsg;
	int tabNum = clients[uid].tabNum;
	if (isFail==0){//���û��ɹ���ֱ�ӷ�����Ϸ������Ϣ
		buf.Format("12 win %d",uid);
		str.Format("15 finish %d",tabNum);
		rankListMsg=GetRankList(tabNum);
		tableState.tableOn[tabNum-1]=-1;
	}
	else if(isFail==1){//���û�ʧ�ܣ��ж���Ϸ���
		int activeUser;
		clients[uid].isFail=1;
		
		int activeNum=0;//δʧ�ܸ���
		for(int i=0;i<6;i++){
			if(userList[tabNum-1][i]!=0){
				int tmpUid=userList[tabNum-1][i];
				if(clients[tmpUid].isFail==0){
					activeNum++;
					activeUser=tmpUid;
				}
			}
		}
		if (activeNum>1){//����ĳ�û���Ϸʧ����Ϣ
			buf.Format("12 fail %d",uid);
		}
		else if(activeNum==1){//������Ϸ������Ϣ
			rankListMsg= GetRankList(tabNum);
			buf.Format("12 win %d",activeUser);
			str.Format("15 finish %d",tabNum);
			tableState.tableOn[tabNum-1]=-1;
		}
	}
	else if(isFail==-1){//���û�ǿ���˳����ж���Ϸ���
// 		int activeUser,activeNum=0;
// 		clients[uid].isFail=1;
// 
// 		for(int i=0;i<6;i++){
// 			if(userList[tabNum-1][i]!=0){
// 				int tmpUid=userList[tabNum-1][i];
// 				if(clients[tmpUid].isFail==0){
// 					activeNum++;
// 					activeUser=tmpUid;
// 				}
// 			}
// 		}
// 		if (activeNum>1){//����ĳ�û���Ϸʧ����Ϣ
// 			buf.Format("12 exit %d",uid);
// 		}
// 		else if(activeNum==1){//������Ϸ������Ϣ
// 			buf.Format("12 win %d",activeUser);
// 		}
	}

	//����str to ����
	char tmp[1024];
	strcpy(tmp,str);
	if(sizeof(str)!=0){
		map<int,UserInfo>::iterator it=clients.begin();       
		for(;it!=clients.end();it++){
			m_sock.SendTo(tmp,sizeof(tmp),(sockaddr*)&it->second.address,sizeof(SOCKADDR));
       }     
	}

	//�㲥tmp to ͬ��
	strcpy(tmp,buf);
	for(int i=0;i<6;i++){
		if(userList[tabNum-1][i]!=0){
			int tmpUid=userList[tabNum-1][i];
			m_sock.SendTo(tmp,sizeof(tmp),(sockaddr*)&clients[tmpUid].address,sizeof(SOCKADDR));
			if(rankListMsg.type==12){
				m_sock.SendTo((char*)&rankListMsg,sizeof(RankListMsg),(sockaddr*)&clients[tmpUid].address,sizeof(SOCKADDR));
			}
			//������Ϣ���Թ���
			list<int>::iterator it;
			for ( it=clients[tmpUid].onlookers.begin() ; it != clients[tmpUid].onlookers.end(); it++ ){
				int lookerId=*it;
				m_sock.SendTo(tmp,sizeof(tmp),(sockaddr*)&clients[lookerId].address,sizeof(SOCKADDR));
				//����������Ϣ
				if(rankListMsg.type==12){
					m_sock.SendTo((char*)&rankListMsg,sizeof(RankListMsg),(sockaddr*)&clients[lookerId].address,sizeof(SOCKADDR));
				}
			}
			clients[tmpUid].onlookers.clear();//����Թ����б�
		}
	}
}
/************************************************************************/
/* ������Ϸ��ʼ��Ϣ                                                     */
/************************************************************************/
void CSockThd::SendGameStartMsg(int tabNum,int uid){
	bool isReady=true;
	int userNum=0;
	//�ж��û��Ƿ�׼��
	for(int i=0;i<6;i++){
		if(userList[tabNum-1][i]!=0){
			userNum++;
			int uid=userList[tabNum-1][i];
			UserInfo userInfo = clients[uid];
			if (!userInfo.isReady){
				isReady=false;
				break;
			}
		}
	}

	if (isReady && userNum>1){//�û���׼������׼���û�������1
		MapInfo mapInfo;
		GeneralRec(mapInfo.recMap);//�����������
		tableState.tableOn[tabNum-1]=1;
		//����ͬ��
		for(int i=0;i<6;i++){
			if(userList[tabNum-1][i]!=0){
				int uid=userList[tabNum-1][i];
				UserInfo userInfo = clients[uid];
				clients[uid].isFail=false;
				CopyMap(mapInfo.recMap,clients[uid].mapInfo.recMap);
				CString str;
				char buf[1024];
				str.Format("9 start");
				strcpy(buf,str);
				m_sock.SendTo(buf,sizeof(buf),(sockaddr*)&userInfo.address,sizeof(SOCKADDR));
			}
		}
		//��������
		map<int,UserInfo>::iterator it=clients.begin();       
		for(;it!=clients.end();it++){
			CString str;
			char buf[1024];
			str.Format("15 start %d",tabNum);
			strcpy(buf,str);
			m_sock.SendTo(buf,sizeof(buf),(sockaddr*)&it->second.address,sizeof(SOCKADDR));
		}     
	}
	else{//���û�δ׼��
		UserInfo userInfo = clients[uid];
		CString str;
		char buf[1024];
		
		if(userNum==1){
			str.Format("9 oneuser");
		}
		if(!isReady){
			str.Format("9 fail");
		}
		strcpy(buf,str);
		m_sock.SendTo(buf,sizeof(buf),(sockaddr*)&userInfo.address,sizeof(SOCKADDR));
	}

}
/************************************************************************/
/* �������û���Ϣ���ݸ�ĳ�û�                                           */
/************************************************************************/
void CSockThd::SendSameTableUserInfo(UserInfo ui){
	for(int i=0;i<6;i++){
		if(userList[ui.tabNum-1][i]!=0){
			int uid=userList[ui.tabNum-1][i];
			UserInfo userInfo = clients[uid];
			CString str;
			char buf[1024];
			str.Format("7 %d %s %d %d %d",userInfo.id,userInfo.nick,userInfo.sex,userInfo.chaNum,userInfo.isReady);
			strcpy(buf,str);
			m_sock.SendTo(buf,sizeof(buf),(sockaddr*)&ui.address,sizeof(SOCKADDR));
		}
	}
}
/************************************************************************/
/* ���������û���Ϣ��ĳ�û�                                             */
/************************************************************************/
void CSockThd::SendAllUserInfo(UserInfo ui){
	for(int i=0;i<15;i++){
		for(int j=0;j<6;j++){
			if(userList[i][j]!=0){
				int uid=userList[i][j];
				UserInfo userInfo = clients[uid];
				CString str;
				char buf[1024];
				str.Format("8 %d %s %d %d %d %d",userInfo.id,userInfo.nick,userInfo.sex,userInfo.tabNum,userInfo.chaNum,userInfo.isReady);
				strcpy(buf,str);
				m_sock.SendTo(buf,sizeof(buf),(sockaddr*)&ui.address,sizeof(SOCKADDR));
			}
		}
	}
}
/************************************************************************/
/* ������Ϣ�����пͻ���                                                 */
/************************************************************************/
void CSockThd::SendMsgToClients(char *data){
// 	int len = sizeof(data);
// 	char *buf = new char[len];
// 	memcpy(buf,data,len);

	map<int,UserInfo>::iterator clients_it;
	for(clients_it=clients.begin();clients_it!=clients.end();clients_it++){
		UserInfo ui=clients_it->second;
		m_sock.SendTo(data,strlen(data)+1,(sockaddr*)&(ui.address),sizeof(SOCKADDR));
	} 
}

/************************************************************************/
/* ����Ϣ��ͬ��                                                         */
/************************************************************************/
void CSockThd::SendMsgToClients(int tabNum,char *data){
	map<int,UserInfo>::iterator clients_it;
	for(int i=0;i<6;i++){
		int uid=userList[tabNum-1][i];
		if(uid!=0){
			clients_it=clients.find(uid);
			UserInfo ui = clients_it->second;
			m_sock.SendTo(data,strlen(data)+1,(sockaddr*)&(ui.address),sizeof(SOCKADDR));
		}
	}
}
/************************************************************************/
/* ����ID�õ��û���Ϣ                                                   */
/************************************************************************/
UserInfo CSockThd::GetUserById(int uid){
	UserInfo ui;
	JAdo  db;
	HRESULT ret = db.Connect("Provider=Microsoft.Jet.OLEDB.4.0;Password=;User ID=;Data Source=llk.mdb;Persist Security Info=True");
	if(SUCCEEDED(ret)){
		CString sql;
		sql.Format("select * from tbUser where id=%d",uid);

		RECORDSET rs = db.JExecuteWithRecordset(sql);
		if(rs != NULL){
			if(rs->rsEOF == FALSE){
				ui.id = GIF(rs,"id");
				ui.nick = GTF(rs,"nick");
				ui.sex = GIF(rs,"sex");
				ui.password = GTF(rs,"pwd");
				ui.type=GIF(rs,"score");
			}
		}
	}
	return ui;
}
int CSockThd::ChangeScore(int uid,int score){
	JAdo  db;
	HRESULT ret = db.Connect("Provider=Microsoft.Jet.OLEDB.4.0;Password=;User ID=;Data Source=llk.mdb;Persist Security Info=True");
	if(SUCCEEDED(ret)){
		CString sql;
		sql.Format("update tbUser set score=%d where id=%d",score,uid);
		return db.JExecuteWithoutRecordset(sql);
	}
	return -1;
}
/************************************************************************/
/* �����û�                                                             */
/************************************************************************/
int CSockThd::AddUser(UserInfo ui){
	JAdo  db;
	HRESULT ret = db.Connect("Provider=Microsoft.Jet.OLEDB.4.0;Password=;User ID=;Data Source=llk.mdb;Persist Security Info=True");
	if(SUCCEEDED(ret)){
		CString sql;
		sql.Format("INSERT INTO tbUser(nick,pwd,sex) VALUES('%s','%s',%d)",ui.nick,ui.password,ui.sex);
		LONG res = db.JExecuteWithoutRecordset(sql);
		if(res<=0){
			return res;
		}
		else{
			sql.Format("select id from tbUser order by id desc");
			RECORDSET rs = db.JExecuteWithRecordset(sql);
			if(rs != NULL){
				if(rs->rsEOF == FALSE){
					 int id= GIF(rs,"id");
					 return id;
				}
			}
		}
	}
	return -1;
}
/************************************************************************/
/* �޸�����                                                             */
/************************************************************************/
int CSockThd::ChangePassword(int uid,CString password,CString repassword){
	JAdo  db;
	HRESULT ret = db.Connect("Provider=Microsoft.Jet.OLEDB.4.0;Password=;User ID=;Data Source=llk.mdb;Persist Security Info=True");
	if(SUCCEEDED(ret)){
		CString sql;
		sql.Format("update tbUser set pwd='%s' where id=%d and pwd='%s'",repassword,uid,password);
		LONG res = db.JExecuteWithoutRecordset(sql);
		return res;
	}
	return -1;
}
/************************************************************************/
/* �����������                                                         */
/************************************************************************/
void CSockThd::GeneralRec(int (&randMap)[14][10]){
	//��ʼ��
	for(int i=0;i<14;i++){
		for(int j=0;j<10;j++){
			randMap[i][j]=0;
		}
	}
	srand((unsigned)GetCurrentTime());
	int m=12,n=8;
	int tmpMap[12][4];

	for (i=0;i<m;i++){
		for(int j=0;j<n/2;j++){
			tmpMap[i][j]=(i+j*12)/3+1;
		}
	}
	
	for(int k=0;k<2;k++){
		for (i=1;i<=m;i++){
			for(int j=1;j<=n/2;j++){
				bool flag=true;
				while(flag){
					int tmp1=rand()%m+1;
					int tmp2=rand()%n+1;
					if(randMap[tmp1][tmp2]==0){
						randMap[tmp1][tmp2]=tmpMap[i-1][j-1];
						flag=false;
					}
				}
			}
		}
	}	
	
	return;
}

void CSockThd::CopyMap(int (&src)[14][10],int (&trg)[14][10]){
	for (int i=0;i<14;i++){
		for(int j=0;j<10;j++){
			trg[i][j]=src[i][j];
		}
	}
}
/************************************************************************/
/* �õ���������                                                         */
/************************************************************************/
RankListMsg CSockThd::GetRankList(int tabNum){

	int **recNum =(int **)malloc(sizeof(int)*2);
	for(int i=0;i<2;i++){
		recNum[i]=(int *)malloc(sizeof(int)*6); 
	}
		

	for(i=0;i<6;i++){
		if(userList[tabNum-1][i]!=0){
			recNum[1][i]=userList[tabNum-1][i];
			recNum[0][i]=96-GetRecNum(clients[recNum[1][i]].mapInfo.recMap);
		}
		else{
			recNum[1][i]=0;//UID
			recNum[0][i]=-1;
		}
	}

	for (int j=1; j<6;j++){
		for (int i=0; i <5; i++){
			if (recNum[0][i]<recNum[0][i+1]){
				int tmp=recNum[0][i];
				recNum[0][i]=recNum[0][i+1];
				recNum[0][i+1]=tmp;
				tmp=recNum[1][i];
				recNum[1][i]=recNum[1][i+1];
				recNum[1][i+1]=tmp;
			}
		}
    }
	RankListMsg msg;
	for(i=0;i<6;i++){
		msg.uid[i]=recNum[1][i];
		msg.rankNum[i]=recNum[0][i];
		UserInfo ui = GetUserById(msg.uid[i]);
		ui.type+=recNum[0][i];
		ChangeScore(msg.uid[i],ui.type);
		msg.score[i]=ui.type;
	}
	msg.type=12;
	return msg;
}
/************************************************************************/
/* �õ�ʣ�෽��ĸ���                                                   */
/************************************************************************/
int CSockThd::GetRecNum(int (&recMap)[14][10]){
	int num=0;
	for (int i=1;i<=12;i++){
		for (int j=1;j<=8;j++){
			if (recMap[i][j]!=0){
				num++;
			}
		}
	}
	return num;
}
/************************************************************************/
/* ����                                                                 */
/************************************************************************/
void CSockThd::RedoMap(int (&recMap)[14][10]){
	int sum=0;
	int tmp=0;
	int tmp1;
	int i,j;
	for (i=1;i<=12;i++)
	{
		for (j=1;j<=8;j++)
		{
			if (recMap[i][j]!=0)
			{
				sum++;
			}
		}
	}
	int *b=new int[sum];
	for (i=1;i<=12;i++)
	{
		for (j=1;j<=8;j++)
		{
			if (recMap[i][j]!=0)
			{
				b[tmp]=recMap[i][j];
				tmp++;
			}
		}
	}
	for (i=1;i<=12;i++)
	{
		for (j=1;j<=8;j++)
		{
			if (recMap[i][j]!=0)
			{
				do 
				{
					tmp1=rand()%sum;
				} while(b[tmp1]==0);
				recMap[i][j]=b[tmp1];
				b[tmp1]=0;
			}
		}
	}

 }