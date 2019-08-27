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
  char          szOrderDate[32 + 1];               // 委托日期
  char          szSession[256 + 1];
  char          szUserSession[32 + 1];
  char          szChannel[2 + 1];             // 操作渠道
  int           iFuncId;
  char          szStationAddr[256+1];
  char          szCuacctCode[16 + 1];         // 资产账户
  char          szCuacctType[1 + 1];          // 账户类型
  char          szSubsysConnstr [128 + 1];    // 子系统连接串
  char          szCustCode[16 + 1];           // 客户代码
  char          szTrdacct[32 + 1];            // 交易账户
  char          szExchange[1 + 1];            // 交易所
  char          szStkbd[2 + 1];               // 交易板块
  short         siStkBiz;                     // 交易业务
  short         siStkBizAction;               // 业务活动
  char          szTrdCode[30 + 1];             // 品种代码
  char          szOptNum[16 + 1];             // 合约编码
  short         siIntOrg;                     // 内部机构
  int           iOrderBsn;                    // 委托批号
  long long     llOrderQty;                   // 委托数量
  long long     llOrderPrice;                 // 委托价格
  long long     llStopPrice;                  // 触发价格
  int           iValidDate;                   // 截止日期
  char          szTrdCodeCls[1 + 1];          // 品种类型
  char          szOrderAttr[256 + 1];         // 高级属性
  short         siAttrCode;                   // 属性代码
  int           iBgnExeTime;                  // 执行开始时间
  int           iEndExeTime;                  // 执行结束时间
  char          szTimeUnit[1 + 1];            // 时间单位，秒或毫秒
  char          szSpreadName[64 + 1];         // 组合名称
  char          szUndlCode[16 + 1];           // 标的代码
  int           iConExpDate;                  // 合约到期日
  char          szExercisePrice[64];          // 行权价
  long long     llConUnit;                    // 合约单位
  char          szCliOrderNo[32 + 1];         // 客户端委托编号
  char          szCliRemark[256 + 1];         // 留痕信息
  char          szBusinessUnit[21 + 1];       // 业务单元
  char          szGtdData[9 + 1];             // GTD日期
  char          szContingentCondition[1 + 1]; // 触发条件
  char          szForceCloseReason[1 + 1];    // 强平原因
  short         siIsSwapOrder;                // 互换单标志
  char          szCombOffsetFlag[5 + 1];      // 组合开平标志
  char          szOrderIdEx[64 + 1];          // 外部合同序号
  char          szComponetStkCode[8 + 1];     // 成份股代码
  char          szComponetStkbd[2 + 1];       // 成份股板块
  char          szStkbdLink[2 + 1];           // 关联板块
  char          szTrdacctLink[10 + 1];        // 关联股东
  char          szRtgsFlag[1 + 1];            // 是否启用RTGS	
  char          szStkBizCtvFlag[1 + 1];       // 现货-业务转换标志; 期权-成份合约编码4
  int           iPassNum;                     // 通道号
  char          szOpSite[32 + 1];             // TCP/IP地址
  char          chOrderFuncType;              // 委托功能类型
  int           iConferNum;                   // 约定号
  char          szTargetTrader[6 + 1];        // 对手方交易员
  char          szTargetCompany[3 + 1];       // 对手方交易商
  char          szTraderId[6 + 1];            // 发起方交易员
  char          szCompanyId[3 + 1];           // 发起方交易商
  char          szSupplemental[255 + 1];      // 补充协议
  int           iOrderNo;                     // 委托序号
  int           iOrderdate;                   // 委托日期
  char          szChannelId[2 + 1];           // 通道号
  long long     llOrderTime;                  // 下单时间
};

struct ST_CANCEL_PARAM
{
  long long     llOrderTime;
  char          szChannelId[2 + 1];           // 通道号
  char          szSession[256 + 1];
  char          szStationAddr[256+1];
  char          szUserSession[32 + 1];
  char          szChannel[1 + 1];             // 操作渠道
  char          szCustCode[16 + 1];           // 客户代码
  char          szCuacctCode[16 + 1];         // 资产账户
  short         siIntOrg;                     // 内部机构
  char          szStkbd[2 + 1];               // 交易板块
  int           iOrderDate;                   // 委托日期
  int           iOrderNo;                     // 委托编号
  int           iOrderBsn;                    // 委托批号
  char          szOrderId[21 + 1];            // 合同序号
  short         siAttrCode;                   // 属性代码
  char          szCuacctType[1 + 1];          // 账户类型
  char          szCliRemark[256 + 1];         // 留痕信息
  int           iPassNum;                     // 通道号    
  char          szCliOrderNo[32 + 1];         // 客户端委托编号
  char          szExSystem[1 + 1];            // 外部系统
  int           iCancleOrderNo;               // 撤单委托编号    
};
*/
class CSvcFuncCosAdpSWReq : public ISvcFunc
{
  DECLARE_DYNCREATE(CSvcFuncCosAdpSWReq)

public:
  CSvcFuncCosAdpSWReq(void);
  virtual ~CSvcFuncCosAdpSWReq(void);

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
