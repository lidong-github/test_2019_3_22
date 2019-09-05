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
  long long llCancleOrderSn;
};

struct ST_XA
{
  char  szSubsysConnstr[128 + 1];
  char  szSubsysSnType[2 + 1];
  char  chSubSysStatus;
  int   iQueueId;
  short siSubSys;
  short siSubSysSn;
  char  szSubSysDbConnstr[32];
};
class CSvcFuncCosAdpSwReq : public ISvcFunc
{
  DECLARE_DYNCREATE(CSvcFuncCosAdpSwReq)

public:
  CSvcFuncCosAdpSwReq(void);
  virtual ~CSvcFuncCosAdpSwReq(void);

  //继函IObject接口
  virtual int Initialize(void);
  virtual int Uninitialize(void);

  //继承ISrvFun接口
  virtual int SetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize);
  virtual int GetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize);

  virtual int SetSvcEnv(ma::CObjectPtr<ma::IServiceEnv> &p_refptrSvcEnv);
  virtual int DoWork(void *p_pvdParam);
  virtual int OnEvent(unsigned int p_uiEventId, ma::CObjectPtr<ma::IData> &p_refptrEventData);

  static xsdk::CRWLock   m_clRWLock;
  static xsdk_atomic_t   m_iInstanceCnt;
  static std::map<std::string, ST_ORDER_PARAM>  m_mapCosOrderParam;
  static std::map<short, ST_XA>         m_mapSysSnQueue;   //系统编码-对应xa配置的队列
public:
  int GetTrdDate(int &p_refTrdDate);
  int  GetXaInfo(ST_XA_INFO &p_refstXaInfo, short siXaId);
  static std::string GetTrdOrderNo(long long llOrderNo, int iTrdDate);
  std::string GetTrdCuacctCode(const char *p_pszCuacctCode, const char chCuacctType);
  int MakeCancel10388904(ma::CMsgData clMakeMsgDataIn,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData, long long p_llCancleOrderSn);
  int Make10388904(ma::CMsgData clMakeMsgDataIn,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData);
  void SetPkgHead(CObjectPtr<IPacketMap> &ptrPacketMap, char chPkgType, char chMsgType, char chFunType, const char *szFunID);
  void SetRegular(CObjectPtr<IPacketMap> &ptrPacketMap, const char *p_pszCust, const char * szSession, const char *szFunID, const char * szOptSite, short siOpOrg, char chChannel);
  int  GetXaInFoFromName();
private:
  unsigned int m_uiSrcFuncId;
  std::string m_strSrcFuncName;

  unsigned int m_uiInQueId;
  unsigned int m_uiOutQueId;

  std::string m_strInQueueId;
  std::string m_strOutQueueId;

  CObjectPtr<IDataSubsysCfg>               m_ptrDataSubsysCfg;
  CObjectPtr<IDaoSubsysCfg>                m_ptrDaoSubsysCfg;
  CObjectPtr<IDataSubsysCfgEx1>            m_ptrDataSubsysCfgEx1;
  CObjectPtr<IDataSubsysCfgUidx1>          m_ptrDataSubsysCfgUidx1;

  CObjectPtr<IRuntimeDb>    m_ptrRuntimeDb;
  CObjectPtr<IDBEngine>     m_ptrDBEngine;

  unsigned int                      m_uiCurrNodeId;
  ST_SYSMA_NODE                     m_stSysNode;

  ST_XA_INFO                        m_stDefaultXaInfo;

  CObjectPtr<IBizOrderDataInit>  m_ptrBizOrderDataInit;
  CObjectPtr<IServiceEnv>   m_ptrServiceEnv;
  CObjectPtr<IPacketMap>    m_ptrPacketMapIn;
  CObjectPtr<IMsgQueue>     m_ptrMsgQueuePut;
  ma::CMsgData              m_clMsgDataOut;
  CObjectPtr<IPacketMap>    m_ptrPacketMapOut;
  ma::CMsgData              m_clMsgDataIn; 
};

class CSvcFuncCosAdpSwAns : public ISvcFunc
{
  DECLARE_DYNCREATE(CSvcFuncCosAdpSwAns)

public:
  CSvcFuncCosAdpSwAns(void);
  virtual ~CSvcFuncCosAdpSwAns(void);

  virtual int Initialize(void);
  virtual int Uninitialize(void);

  virtual int SetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize);
  virtual int GetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize);

  virtual int SetSvcEnv(ma::CObjectPtr<ma::IServiceEnv> &p_refptrSvcEnv);
  virtual int DoWork(void *p_pvdParam);
  virtual int OnEvent(unsigned int p_uiEventId, ma::CObjectPtr<ma::IData> &p_refptrEventData);

private:
  int  Make10388904(ma::CMsgData clMakeMsgDataIn,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData);
  int  MakeCancel10388904(ST_ORDER_PARAM stOrderParam,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData);
  void SetPkgHead(CObjectPtr<IPacketMap> &ptrPacketMap, char chPkgType, char chMsgType, char chFunType, const char *szFunID);
  void SetRegular(CObjectPtr<IPacketMap> &ptrPacketMap, const char *p_pszCust, const char * szSession, const char *szFunID, const char * szOptSite, short siOpOrg, char chChannel);
  int  CheckTimeOut(SYSTEMTIME p_stCurrentTime);
  int  GetTrdDate(int &p_refTrdDate);
  int  OpenLogFile();
  int  GetXaInfo(ST_XA_INFO &p_refstXaInfo, short siXaId);
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
  ST_XA_INFO                m_stDefaultXaInfo;

  ma::CMsgData m_clMsgDataIn;
  ma::CMsgData m_clMsgDataOut;
};

END_NAMESPACE_MA

#endif  // __MA_SVCFUNC_APD_CTP_H__
