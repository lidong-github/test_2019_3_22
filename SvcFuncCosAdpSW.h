#if !defined(__MA_SVCFUNC_APD_CTP_H__)
#define __MA_SVCFUNC_APD_CTP_H__

#include "iFunc.h"
#include "iPacketMap.h"
#include "iDataEventPackage.h"
#include "maCosOrderComm.h"
#include "iBiz.h"
#include "maCosComm.h"
#include "iBizOrderDataInit.h"
#include "iDataEventPackage.h"
#include "xsdk_atomic.h"
#include "xsdk_rwlock.h"
#include <map>

BGN_NAMESPACE_MA

#define  HEAD_LENTHS 256

struct ST_ORDER_PARAM
{
  short siSubsysSn;
  long long llOrderTime;
  BOOL bIsCancel;
  ma::CMsgData clMsgData;
  long long llOrderNo;
};

/*struct ST_ORDER_PARAM 
{
  char          szOrderDate[32 + 1];               // ί������
  char          szSession[256 + 1];
  char          szUserSession[32 + 1];
  char          szChannel[2 + 1];             // ��������
  int           iFuncId;
  char          szStationAddr[256+1];
  char          szCuacctCode[16 + 1];         // �ʲ��˻�
  char          szCuacctType[1 + 1];          // �˻�����
  char          szSubsysConnstr [128 + 1];    // ��ϵͳ���Ӵ�
  char          szCustCode[16 + 1];           // �ͻ�����
  char          szTrdacct[32 + 1];            // �����˻�
  char          szExchange[1 + 1];            // ������
  char          szStkbd[2 + 1];               // ���װ��
  short         siStkBiz;                     // ����ҵ��
  short         siStkBizAction;               // ҵ��
  char          szTrdCode[30 + 1];             // Ʒ�ִ���
  char          szOptNum[16 + 1];             // ��Լ����
  short         siIntOrg;                     // �ڲ�����
  int           iOrderBsn;                    // ί������
  long long     llOrderQty;                   // ί������
  long long     llOrderPrice;                 // ί�м۸�
  long long     llStopPrice;                  // �����۸�
  int           iValidDate;                   // ��ֹ����
  char          szTrdCodeCls[1 + 1];          // Ʒ������
  char          szOrderAttr[256 + 1];         // �߼�����
  short         siAttrCode;                   // ���Դ���
  int           iBgnExeTime;                  // ִ�п�ʼʱ��
  int           iEndExeTime;                  // ִ�н���ʱ��
  char          szTimeUnit[1 + 1];            // ʱ�䵥λ��������
  char          szSpreadName[64 + 1];         // �������
  char          szUndlCode[16 + 1];           // ��Ĵ���
  int           iConExpDate;                  // ��Լ������
  char          szExercisePrice[64];          // ��Ȩ��
  long long     llConUnit;                    // ��Լ��λ
  char          szCliOrderNo[32 + 1];         // �ͻ���ί�б��
  char          szCliRemark[256 + 1];         // ������Ϣ
  char          szBusinessUnit[21 + 1];       // ҵ��Ԫ
  char          szGtdData[9 + 1];             // GTD����
  char          szContingentCondition[1 + 1]; // ��������
  char          szForceCloseReason[1 + 1];    // ǿƽԭ��
  short         siIsSwapOrder;                // ��������־
  char          szCombOffsetFlag[5 + 1];      // ��Ͽ�ƽ��־
  char          szOrderIdEx[64 + 1];          // �ⲿ��ͬ���
  char          szComponetStkCode[8 + 1];     // �ɷݹɴ���
  char          szComponetStkbd[2 + 1];       // �ɷݹɰ��
  char          szStkbdLink[2 + 1];           // �������
  char          szTrdacctLink[10 + 1];        // �����ɶ�
  char          szRtgsFlag[1 + 1];            // �Ƿ�����RTGS	
  char          szStkBizCtvFlag[1 + 1];       // �ֻ�-ҵ��ת����־; ��Ȩ-�ɷݺ�Լ����4
  int           iPassNum;                     // ͨ����
  char          szOpSite[32 + 1];             // TCP/IP��ַ
  char          chOrderFuncType;              // ί�й�������
  int           iConferNum;                   // Լ����
  char          szTargetTrader[6 + 1];        // ���ַ�����Ա
  char          szTargetCompany[3 + 1];       // ���ַ�������
  char          szTraderId[6 + 1];            // ���𷽽���Ա
  char          szCompanyId[3 + 1];           // ���𷽽�����
  char          szSupplemental[255 + 1];      // ����Э��
  int           iOrderNo;                     // ί�����
  int           iOrderdate;                   // ί������
  char          szChannelId[2 + 1];           // ͨ����
  long long     llOrderTime;                  // �µ�ʱ��
};

struct ST_CANCEL_PARAM
{
  long long     llOrderTime;
  char          szChannelId[2 + 1];           // ͨ����
  char          szSession[256 + 1];
  char          szStationAddr[256+1];
  char          szUserSession[32 + 1];
  char          szChannel[1 + 1];             // ��������
  char          szCustCode[16 + 1];           // �ͻ�����
  char          szCuacctCode[16 + 1];         // �ʲ��˻�
  short         siIntOrg;                     // �ڲ�����
  char          szStkbd[2 + 1];               // ���װ��
  int           iOrderDate;                   // ί������
  int           iOrderNo;                     // ί�б��
  int           iOrderBsn;                    // ί������
  char          szOrderId[21 + 1];            // ��ͬ���
  short         siAttrCode;                   // ���Դ���
  char          szCuacctType[1 + 1];          // �˻�����
  char          szCliRemark[256 + 1];         // ������Ϣ
  int           iPassNum;                     // ͨ����    
  char          szCliOrderNo[32 + 1];         // �ͻ���ί�б��
  char          szExSystem[1 + 1];            // �ⲿϵͳ
  int           iCancleOrderNo;               // ����ί�б��    
};
*/
class CSvcFuncCosAdpSWReq : public ISvcFunc
{
  DECLARE_DYNCREATE(CSvcFuncCosAdpSWReq)

public:
  CSvcFuncCosAdpSWReq(void);
  virtual ~CSvcFuncCosAdpSWReq(void);

  //�̺�IObject�ӿ�
  virtual int Initialize(void);
  virtual int Uninitialize(void);

  //�̳�ISrvFun�ӿ�
  virtual int SetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize);
  virtual int GetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize);

  virtual int SetSvcEnv(ma::CObjectPtr<ma::IServiceEnv> &p_refptrSvcEnv);
  virtual int DoWork(void *p_pvdParam);
  virtual int OnEvent(unsigned int p_uiEventId, ma::CObjectPtr<ma::IData> &p_refptrEventData);

  static xsdk::CRWLock   m_clRWLock;
  static xsdk_atomic_t   m_iInstanceCnt;
  static std::map<std::string, ST_ORDER_PARAM>  m_mapCosOrderParam;
public:
  int GetTrdDate(int &p_refTrdDate);
  int  GetXaInfo(ST_XA_INFO &p_refstXaInfo, short siXaId);
  static std::string GetTrdOrderNo(long long llOrderNo, int iTrdDate);
  std::string GetTrdCuacctCode(const char *p_pszCuacctCode, const char chCuacctType);
  int MakeCancel10388902(ma::CMsgData clMakeMsgDataIn,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData);
  int Make10388902(ma::CMsgData clMakeMsgDataIn,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData);
  void SetPkgHead(CObjectPtr<IPacketMap> &ptrPacketMap, char chPkgType, char chMsgType, char chFunType, const char *szFunID);
  void SetRegular(CObjectPtr<IPacketMap> &ptrPacketMap, const char *p_pszCust, const char * szSession, const char *szFunID, const char * szOptSite, short siOpOrg, char chChannel);
private:
  unsigned int m_uiSrcFuncId;
  std::string m_strSrcFuncName;

  unsigned int m_uiInQueId;
  unsigned int m_uiOutQueId;

  std::string m_strInQueueId;
  std::string m_strOutQueueId;

  CObjectPtr<IRuntimeDb>    m_ptrRuntimeDb;
  CObjectPtr<IDBEngine>     m_ptrDBEngine;

  unsigned int                      m_uiCurrNodeId;
  ST_SYSMA_NODE                     m_stSysNode;

  ST_XA_INFO                        m_stDefaultXaInfo;

  CObjectPtr<IBizOrderDataInit>  m_ptrBizOrderDataInit;
  CObjectPtr<IServiceEnv>   m_ptrServiceEnv;
  CObjectPtr<IPacketMap>    m_ptrPacketMapIn;
  ma::CMsgData              m_clMsgDataOut;
  CObjectPtr<IPacketMap>    m_ptrPacketMapOut;
  ma::CMsgData              m_clMsgDataIn; 
};

class CSvcFuncCosAdpSWAns : public ISvcFunc
{
  DECLARE_DYNCREATE(CSvcFuncCosAdpSWAns)

public:
  CSvcFuncCosAdpSWAns(void);
  virtual ~CSvcFuncCosAdpSWAns(void);

  virtual int Initialize(void);
  virtual int Uninitialize(void);

  virtual int SetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize);
  virtual int GetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize);

  virtual int SetSvcEnv(ma::CObjectPtr<ma::IServiceEnv> &p_refptrSvcEnv);
  virtual int DoWork(void *p_pvdParam);
  virtual int OnEvent(unsigned int p_uiEventId, ma::CObjectPtr<ma::IData> &p_refptrEventData);

private:
  int  Make10388902(ma::CMsgData clMakeMsgDataIn,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData);
  int  MakeCancel10388902(ma::CMsgData clMsgDataIn,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData);
  void SetPkgHead(CObjectPtr<IPacketMap> &ptrPacketMap, char chPkgType, char chMsgType, char chFunType, const char *szFunID);
  void SetRegular(CObjectPtr<IPacketMap> &ptrPacketMap, const char *p_pszCust, const char * szSession, const char *szFunID, const char * szOptSite, short siOpOrg, char chChannel);
  int  CheckTimeOut(long long p_llCurrentTime);
  int  GetTrdDate(int &p_refTrdDate);
private:
  unsigned int m_uiSrcFuncId;
  std::string m_strSrcFuncName;

  BOOL m_bIsRefuse;
  BOOL m_bWriteLogFlag;
  BOOL m_bIsRefuseCancel;
  BOOL m_llCancelOutTime;
  char m_szFuncId[16 + 1];
  unsigned int m_uiInQueId;
  unsigned int m_uiOutQueId;
  unsigned int m_uiPutQueId;
  long long m_llOutTime;
  xsdk::CFile m_clLogFile;

  std::string m_strInQueueId;
  std::string m_strOutQueueId; 

  unsigned int m_uiCurrNodeId;

  CObjectPtr<IDBEngine>						m_ptrDBEngine;
  CObjectPtr<IDataEventPackage>   m_ptrDataEventPackage;
  CObjectPtr<IBizOrderDataInit>  m_ptrBizOrderDataInit;
  CObjectPtr<IServiceEnv>   m_ptrServiceEnv;
  CObjectPtr<IPacketMap>    m_ptrPacketMapOut;
  CObjectPtr<IPacketMap>    m_ptrPacketMapIn;
  ST_SYSMA_NODE             m_stSysNode;

  ma::CMsgData m_clMsgDataIn;
  ma::CMsgData m_clMsgDataOut;
};

END_NAMESPACE_MA

#endif  // __MA_SVCFUNC_APD_CTP_H__
