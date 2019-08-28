//--------------------------------------------------------------------------------------------------
// 版权声明：本程序模块属于金证微内核架构平台(KMAP)的一部分
//           国信证券 & 金证科技  版权所有
//
// 文件名称：SvcFuncCosAdpSW.cpp
// 模块名称：申万宏源风控适配器
// 模块描述：
// 开发作者：李东
// 创建日期：2019-07-25
// 模块版本：1.0.000.000
//--------------------------------------------------------------------------------------------------
// 修改日期      版本          作者            备注
//--------------------------------------------------------------------------------------------------
// 2019-07-25   1.0.000.000   李东          初创
//--------------------------------------------------------------------------------------------------
#include "SvcFuncCosAdpSW.h"
#include "maGlobal.h"
#include "xsdk_string.h"
#include "xsdk_datetime.h"
#include "xsdk_numeric.h"
#if defined(OS_IS_LINUX)
#include "iconv.h"
#include "json/json.h"
#else
#include "json\json.h"
#endif
#include <string>

#if defined(OS_IS_WINDOWS)
#pragma comment(lib,"libjson.lib")
#endif
#define CP_GB2312 20936

USE_NAMESPACE_MA

  IMPLEMENT_DYNCREATE(CSvcFuncCosAdpSWReq, ISvcFunc, _V("001.000.001"))
  IMPLEMENT_DYNCREATE(CSvcFuncCosAdpSWAns, ISvcFunc, _V("001.000.000"))

xsdk_atomic_t CSvcFuncCosAdpSWReq::m_iInstanceCnt = 0;
xsdk::CRWLock CSvcFuncCosAdpSWReq::m_clRWLock;

 std::map<std::string, ST_ORDER_PARAM>  CSvcFuncCosAdpSWReq::m_mapCosOrderParam;

int UTF8ToGB2312(char *p_pszGBText, int p_iGBSize, const char *p_pUtf8Text, int p_iUtf8Len)
{
  int iRetCode = MA_OK;

  try{
#if defined(OS_IS_WINDOWS)
    int len = MultiByteToWideChar(CP_UTF8, 0, p_pUtf8Text, -1, NULL, 0);
    if (0 == len)
    {
      return -1;
    }
    wchar_t* wstr = new wchar_t[len + 1];
    if (NULL == wstr)
    {
      return -1;
    }
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_UTF8, 0, p_pUtf8Text, -1, wstr, len);

    WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

    memset(p_pszGBText, 0, p_iGBSize);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, p_pszGBText, p_iUtf8Len, NULL, NULL);
    if(wstr)
    {
      delete[] wstr;
    }
    return 0;
#else//OS_IS_LINUX
     iconv_t cd;
     int rc;
     char **szIn = &p_pUtf8Text;
     char **szOut = &p_pszGBText;
     int iOutlen = p_iGBSize; 
     cd == iconv_open("GB2312", "utf-8");
     if (cd == 0 )
     {
       return -1;
     }
     memset(p_pszGBText, 0, strlen(p_pszGBText));
     if (iconv(cd, szIn, (size_t *)&p_iUtf8Len, szOut, (size_t *)&iOutlen) == -1)
     {
       iconv_close(cd);
       return -1;
     }
     iconv_close(cd);
     return 0;

#endif//OS_IS_WINDOWS
  }
  catch(...)
  {
  }
  return iRetCode;
}

CSvcFuncCosAdpSWReq::CSvcFuncCosAdpSWReq(void)
{
}

CSvcFuncCosAdpSWReq::~CSvcFuncCosAdpSWReq(void)
{
}

int CSvcFuncCosAdpSWReq::Initialize(void)
{
  int iRetCode = MA_OK;

  _ma_try
  {
    if (xsdk_AtomicAdd(&CSvcFuncCosAdpSWReq::m_iInstanceCnt, 1) <= 0)
    {    
      m_mapCosOrderParam.clear();
      m_clRWLock.Create();
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

int CSvcFuncCosAdpSWReq::Uninitialize(void)
{
  int iRetCode = MA_OK;

  _ma_try
  {
    if (xsdk_AtomicDec(&CSvcFuncCosAdpSWReq::m_iInstanceCnt) <= 0)
    {  
      m_mapCosOrderParam.clear();
      m_clRWLock.Close();
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

int CSvcFuncCosAdpSWReq::SetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize)
{
  int iRetCode = MA_OK;

  _ma_try
  {
    if (p_pvdValuePtr == NULL)
    {
      iRetCode = MA_ERROR_INVALID_PARAM;
      _ma_throw ma::CFuncException(iRetCode,
        "parameter '{@1}' of function '{@2}' invalid paramter ",
        &(CRtmParam("p_pvdValuePtr") + CRtmParam("CSvcFuncCosAptSfitCtp::SetOptions")));
    }

    switch (p_iOptionInd)
    {
    case OPT_SVCFUNC_ID:
      m_uiSrcFuncId = *((unsigned int *)p_pvdValuePtr);
      break;
    case OPT_SVCFUNC_NAME:
      m_strSrcFuncName.assign((char *)p_pvdValuePtr);
      break;
    case OPT_SVCFUNC_INQUE:
      m_strInQueueId.assign((char *)p_pvdValuePtr);
      m_uiInQueId = atoi(m_strInQueueId.c_str());
      break;
    case OPT_SVCFUNC_OUTQUE:
      m_strOutQueueId.assign((char *)p_pvdValuePtr);
      m_uiOutQueId = atoi(m_strOutQueueId.c_str());
      break;
    default:
      iRetCode = MA_ERROR_OPTION_UNSUPPORTED;
      _ma_throw ma::CFuncException(iRetCode,
        "{@1} option {@2} unsupported",
        &(CRtmParam("CSvcFuncCosAptSfitCtp") + CRtmParam(p_iOptionInd)));
      break;
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

int CSvcFuncCosAdpSWReq::GetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize)
{
  int iRetCode = MA_OK;

  _ma_try
  {
    if (p_pvdValuePtr == NULL)
    {
      iRetCode = MA_ERROR_INVALID_PARAM;
      _ma_throw ma::CFuncException(iRetCode,
        "invalid parameter '{@1}' of function '{@2}' ",
        &(CRtmParam("p_pvdValuePtr") + CRtmParam("CSvcFuncCosAptSfitCtp::GetOptions")));
    }

    if (p_iValueSize <= 0)
    {
      iRetCode = MA_ERROR_INVALID_PARAM;
      _ma_throw ma::CFuncException(iRetCode,
        "paramter {@1} of function {@2} invalid paramter",
        &(CRtmParam("p_iValueSize")+ CRtmParam("CSvcFuncCosAptSfitCtp::GetOptions")));
    }

    switch(p_iOptionInd)
    {
    case OPT_SVCFUNC_ID:
      *((unsigned int *)p_pvdValuePtr) = m_uiSrcFuncId;
      break;
    case OPT_SVCFUNC_NAME:
      memset(p_pvdValuePtr, 0x00, p_iValueSize);
      strncpy((char *)p_pvdValuePtr, m_strSrcFuncName.c_str(), p_iValueSize - 1);
      break;
    case OPT_SVCFUNC_INQUE:
      memset(p_pvdValuePtr, 0x00, p_iValueSize);
      strncpy((char *)p_pvdValuePtr, m_strInQueueId.c_str(), p_iValueSize - 1);
      break;
    case OPT_SVCFUNC_OUTQUE:
      memset(p_pvdValuePtr, 0x00, p_iValueSize);
      strncpy((char *)p_pvdValuePtr, m_strOutQueueId.c_str(), p_iValueSize - 1);
      break;
    default:
      iRetCode = MA_ERROR_OPTION_UNSUPPORTED;
      _ma_throw ma::CFuncException(iRetCode,
        "{@1} option {@2} unsupported",
        &(CRtmParam("CSvcFuncCosAdpSW") + CRtmParam(p_iOptionInd)));
      break;
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

int CSvcFuncCosAdpSWReq::SetSvcEnv(ma::CObjectPtr<ma::IServiceEnv> &p_refptrSvcEnv)
{
  int iRetCode = MA_OK;
  char szValue[8] = {0};

  _ma_try
  {
    if (p_refptrSvcEnv.IsNull())
    {
      _ma_throw CException(MA_ERROR_INVALID_PARAM,
        "paramter {@1} of function {@2} invalid paramter",
        &(CRtmParam("p_pvdValuePtr")
        + CRtmParam(__FUNCTION__)));
    }
    m_ptrServiceEnv = p_refptrSvcEnv;

    if (m_ptrPacketMapIn.Create("CPacketMap").IsNull())
    {
      _ma_throw ma::CFuncException(MA_ERROR_UNDEFINED_OBJECT,
        "create object {@1} failed",
        &(_P("CPacketMap")));
    }
    if (m_ptrPacketMapOut.Create("CPacketMap").IsNull())
    {
      _ma_throw ma::CFuncException(MA_ERROR_UNDEFINED_OBJECT,
        "create object {@1} failed",
        &(_P("CPacketMap")));
    }
    if(m_ptrBizOrderDataInit.Create("CBizOrderDataInit").IsNull()
      || m_ptrBizOrderDataInit->Initialize() != MA_OK)
    {
      _ma_throw ma::CBizException(MA_ERROR_INIT_OBJECT, "Initialize Object [{@1}] fail!",
        &(CRtmParam("CBizOrderDataInit")));
    }
    ma::g_hKernelEnv->UN_HANDLE.pclKernelEnv->GetOptions(ma::OPT_KERNEL_NODE_ID, &m_uiCurrNodeId, sizeof(m_uiCurrNodeId));
    memset(&m_stSysNode, 0x00, sizeof(ST_SYSMA_NODE));
    iRetCode = m_ptrServiceEnv->GetSysNode(m_stSysNode, m_uiCurrNodeId);

    iRetCode = GetXaInfo(m_stDefaultXaInfo, m_stSysNode.siDftXaId);
    if (iRetCode != MA_OK)
    {
      ma::ThrowError(NULL, "CSvcFuncCosAdpSfitCtpReq[{@1}] call object GetXaInfo() failed, return {@2}", 
        &(_P(__LINE__) + _P(iRetCode)));
      _ma_leave;
    }
    if (strlen(m_stDefaultXaInfo.szDaoPath) > 0 && ::g_hKernelEnv->UN_HANDLE.pclKernelEnv->GetObjectFactory() != NULL)
    {
      iRetCode = ma::g_hKernelEnv->UN_HANDLE.pclKernelEnv->GetObjectFactory()->SetObjectPaths(m_stDefaultXaInfo.szDaoPath);
    }

    if ((iRetCode = m_stDefaultXaInfo.ptrXa->Open(m_stDefaultXaInfo.szOpen, 0, 0)) != MA_OK)
    {
      ma::ThrowError(NULL, "CSvcFuncCosAdpSfitCtpReq open:[{@1}] fail, return {@2} - {@3}",
        &(_P(m_stDefaultXaInfo.szOpen) + _P(iRetCode) + _P(m_stDefaultXaInfo.ptrXa->GetLastErrorText())));
      _ma_leave;
    }
    m_stDefaultXaInfo.ptrXa->GetDBEngine(m_ptrDBEngine);
  }
  _ma_catch_finally
  {
  }
  return iRetCode;
}

int CSvcFuncCosAdpSWReq::OnEvent(unsigned int p_uiEventId, ma::CObjectPtr<ma::IData> &p_refptrEventData)
{
  int iRetCode = MA_OK;

  return iRetCode;
}

std::string CSvcFuncCosAdpSWReq::GetTrdCuacctCode(const char *p_pszCuacctCode, const char chCuacctType)
{
  char szTrdCuacctCode[32] = {0};
  snprintf(szTrdCuacctCode,sizeof(szTrdCuacctCode), "%s%c", p_pszCuacctCode, chCuacctType);
  return szTrdCuacctCode;
}

int ma::CSvcFuncCosAdpSWReq::GetXaInfo(ST_XA_INFO &p_refstXaInfo, short siXaId)
{
  int iRetCode = MA_OK;

  CObjectPtr<IDataSysbpuXa> ptrDataXa;
  CObjectPtr<IDaoSysbpuXa> ptrDaoXa;
  CObjectPtr<IDataSysbpuXaUidx1> ptrDataSysbpuXaUidx1;
  CObjectPtr<IRuntimeDb> ptrRuntimeDb;
  CObjectPtr<IDBEngine> ptrRtdbDBEngine;

  _ma_try
  {
    m_ptrServiceEnv->GetRuntimeDb(ptrRuntimeDb);

    if (ptrRuntimeDb.IsNull()
      || ptrRuntimeDb->GetDBEngine(ptrRtdbDBEngine) != MA_OK
      || ptrRtdbDBEngine.IsNull())
    {
      _ma_throw ma::CFuncException(MA_ERROR_OBJECT_UNINITIATED,
        "object {@1} uninitiated",
        &(_P("ptrRuntimeDb::GetDBEngine(ptrRtdbDBEngine)")));
    }

    if (ptrDataXa.Create("CDataSysbpuXa").IsNull()
      || ptrDaoXa.Create("CDaoSysbpuXa").IsNull()
      || ptrDataSysbpuXaUidx1.Create("CDataSysbpuXaUidx1").IsNull())
    {
      _ma_throw ma::CFuncException(MA_ERROR_UNDEFINED_OBJECT,
        "create object {@1} failed",
        &(_P("CDataSysbpuXa/CDaoSysbpuXa/CDataSysbpuXaEx1")));
    }

    ptrDaoXa->SetDBEngine(ptrRtdbDBEngine);
    ptrDataSysbpuXaUidx1->SetXaId(siXaId);

    iRetCode = ptrDaoXa->Select(ptrDataXa.Ptr(), ptrDataSysbpuXaUidx1.Ptr());

    if (iRetCode != MA_OK && iRetCode != MA_NO_DATA)
    {
      _ma_throw ma::CDaoException(iRetCode, ptrDaoXa->GetLastErrorText());
    }
    else if (iRetCode == MA_NO_DATA)
    {
      _ma_leave;
    }
    else
    {
      p_refstXaInfo.uiId = ptrDataXa->GetXaId();
      snprintf(p_refstXaInfo.szName, sizeof(p_refstXaInfo.szName), "%s", ptrDataXa->GetXaName());
      snprintf(p_refstXaInfo.szClsid, sizeof(p_refstXaInfo.szClsid), "%s", ptrDataXa->GetXaClsid());
      snprintf(p_refstXaInfo.szOpen, sizeof(p_refstXaInfo.szOpen), "%s", ptrDataXa->GetXaOpen());
      snprintf(p_refstXaInfo.szClose, sizeof(p_refstXaInfo.szClose), "%s", ptrDataXa->GetXaClose());
      snprintf(p_refstXaInfo.szOption, sizeof(p_refstXaInfo.szOption), "%s", ptrDataXa->GetXaOption());
      snprintf(p_refstXaInfo.szDaoPath, sizeof(p_refstXaInfo.szDaoPath), "%s", ptrDataXa->GetXaDaoPath());

      if (p_refstXaInfo.ptrXa.Create(p_refstXaInfo.szClsid).IsNull()
        || (iRetCode = p_refstXaInfo.ptrXa->SetOptions(OPT_XA_ID, &p_refstXaInfo.uiId, sizeof(p_refstXaInfo.uiId))) != MA_OK
        || (iRetCode = p_refstXaInfo.ptrXa->SetOptions(OPT_XA_NAME, p_refstXaInfo.szName, strlen(p_refstXaInfo.szName))) != MA_OK
        /*|| (iRetCode = p_refstXaInfo.ptrXa->Open(p_refstXaInfo.szOpen, 0, 0)) != MA_OK*/)
      {
        ma::ThrowError(NULL,
          "create xa object [id:{@1}, name:{@2}, clsid:{@3}] fail, return {@4} - {@5}",
          &(_P(p_refstXaInfo.uiId)
          + _P(p_refstXaInfo.szName)
          + _P(p_refstXaInfo.szClsid)
          + _P(iRetCode)
          + _P(p_refstXaInfo.ptrXa->GetLastErrorText())));
        iRetCode = MA_OK;
      }
      else
      {
        ma::ThrowInfo(NULL,
          "create xa object [id:{@1}, name:{@2}, clsid:{@3}] success",
          &(_P(p_refstXaInfo.uiId)
          + _P(p_refstXaInfo.szName)
          + _P(p_refstXaInfo.szClsid)));
      }
    }
  }
  _ma_catch_finally
  {
    ptrRtdbDBEngine->Commit();
  }
  return iRetCode;
}

void CSvcFuncCosAdpSWReq::SetPkgHead(CObjectPtr<IPacketMap> &ptrPacketMap, char chPkgType, char chMsgType, char chFunType, const char *szFunID)
{
  ptrPacketMap->SetHdrColValue(chPkgType, MAP_FID_PKT_TYPE);
  ptrPacketMap->SetHdrColValue(chMsgType, MAP_FID_MSG_TYPE); 
  ptrPacketMap->SetHdrColValue("01", strlen("01"), MAP_FID_PKT_VER);
  ptrPacketMap->SetHdrColValue(chFunType, MAP_FID_FUNC_TYPE); 
  ptrPacketMap->SetHdrColValue(szFunID, strlen(szFunID), MAP_FID_FUNC_ID); 
  ptrPacketMap->SetHdrColValue('2', MAP_FID_RESEND_FLAG); // 重发标志：0正常，1重发, 2不需应答
  ptrPacketMap->SetHdrColValue(CToolKit::GetLongLongCurTime(), MAP_FID_TIMESTAMP);
  ptrPacketMap->SetHdrColValue('1', MAP_FID_TOKEN_FLAG);
}

void CSvcFuncCosAdpSWReq::SetRegular(CObjectPtr<IPacketMap> &ptrPacketMap, const char *p_pszCust, const char * szSession, const char *szFunID, const char * szOptSite, short siOpOrg, char chChannel)
{
  ptrPacketMap->SetValue(p_pszCust, strlen(p_pszCust), "8810");
  ptrPacketMap->SetValue('1', "8811");
  ptrPacketMap->SetValue(szOptSite, strlen(szOptSite), "8812");
  ptrPacketMap->SetValue(chChannel, "8813");
  strlen(szSession) < 1 ? ptrPacketMap->SetValue("123456", strlen("123456"), "8814") : 
    ptrPacketMap->SetValue(szSession, strlen(szSession), "8814");  
  ptrPacketMap->SetValue(szFunID, strlen(szFunID), "8815");

  char szCurrTimeStamp[24] = {0};
  SYSTEMTIME stTimestamp;
  xsdk::GetCurrentTimestamp(stTimestamp);
  xsdk::DatetimeToString(szCurrTimeStamp, sizeof(szCurrTimeStamp), "YYYY-MM-DD HH24:MI:SS.nnn", stTimestamp);
  ptrPacketMap->SetValue(szCurrTimeStamp, strlen(szCurrTimeStamp), "8816");

  ptrPacketMap->SetValue(siOpOrg, "8821");
}

int CSvcFuncCosAdpSWReq::GetTrdDate(int &p_refTrdDate)
{
  int iRetCode = MA_OK;
  _ma_try
  {
    if(m_ptrBizOrderDataInit.Create("CBizOrderDataInit").IsNull()
      || m_ptrBizOrderDataInit->Initialize() != MA_OK
      || (iRetCode = m_ptrBizOrderDataInit->SetServiceDBEngine(m_ptrDBEngine)) != MA_OK)
    {
      iRetCode = MA_ERROR_OBJECT_UNINITIATED;
      ma::ThrowError(NULL, "CSFuncTsuGetReq[{@1}] CBizOrderDataInit Create Object fail!", &_P(__LINE__));
      _ma_leave;
    }
    if (m_ptrBizOrderDataInit->GetSysTrdDate(p_refTrdDate) != MA_OK)  //获取交易日期
    {
      _ma_throw ma::CFuncException(m_ptrBizOrderDataInit->GetLastErrorCode(), m_ptrBizOrderDataInit->GetLastErrorText()); 
    }
  }
  _ma_catch_finally
  {
  }
  return MA_OK;
}

std::string CSvcFuncCosAdpSWReq::GetTrdOrderNo(long long  llOrderNo, int iTrdDate)
{
  char szTrdCuacctCode[32] = {0};
  snprintf(szTrdCuacctCode,sizeof(szTrdCuacctCode), "%d%d", llOrderNo, iTrdDate);
  return szTrdCuacctCode;
}

int CSvcFuncCosAdpSWReq::Make10388902(ma::CMsgData clMakeMsgDataIn ,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData)
{
  int iRetCode = MA_OK;
  char szFuncId[8+1] = {"10388902"};
  char szMsgId[32 + 1] = {0};
  char szStationAddr[64 + 1] = {0};
  char szChannel[1 + 1] = {0};
  short siIntOrg = 0;
  char szCuacctCode[16 + 1] = {0};
  char szUserSession[126 + 1] = {0};
  char szChannelId[16 + 1] = {0};
  long long llOrderNo = 0;
  int  iValueLen = 0;
  _ma_try
  {
    if ((iRetCode = m_ptrPacketMapIn->Parse(clMakeMsgDataIn)) != MA_OK)
    {
      ThrowError(NULL, "CSvcFuncCosAdpSfitCtpReq::{@1} CTP Data Parse failed, return {@2}, PacketData:[{@3}]",
        &(_P(__FUNCTION__) + _P(iRetCode) + _P((char*)m_clMsgDataIn.Data())));
      _ma_leave;
    }
    m_ptrPacketMapIn->GetValue(szStationAddr, sizeof(szStationAddr), "8810");
    m_ptrPacketMapIn->GetValue(siIntOrg, "8821");
    m_ptrPacketMapIn->GetValue(szChannel, sizeof(szChannel), "8815");
    m_ptrPacketMapIn->GetValue(szCuacctCode, sizeof(szCuacctCode), "8920");
    m_ptrPacketMapIn->GetValue(llOrderNo, "9106");
    m_ptrPacketMapIn->GetHdrColValue(szUserSession, sizeof(szUserSession), iValueLen, MAP_FID_USER_SESSION);
    m_ptrPacketMapIn->GetHdrColValue(szChannelId, sizeof(szChannelId), iValueLen, MAP_FID_BIZ_CHANNEL);
    m_ptrPacketMapOut->BeginWrite();
    SetPkgHead(m_ptrPacketMapOut, MAP_PKT_TYPE_BIZ, MAP_MSG_TYPE_REQ, 'T', szFuncId);
    SetRegular(m_ptrPacketMapOut, szCuacctCode, "", szFuncId, szStationAddr, siIntOrg, szChannel[0]);
    m_ptrPacketMapOut->SetHdrColValue(szUserSession, strlen(szUserSession), MAP_FID_USER_SESSION); 
    m_ptrPacketMapOut->SetHdrColValue(szChannelId, strlen(szChannelId), MAP_FID_BIZ_CHANNEL); 
    snprintf(szMsgId, sizeof(szMsgId), "0|%s|%d|00000000", szCuacctCode, llOrderNo);						
    m_ptrPacketMapOut->SetHdrColValue(ORDER_PUSH_TOPIC, strlen(ORDER_PUSH_TOPIC), ma::MAP_FID_PUB_TOPIC); 
    m_ptrPacketMapOut->SetHdrColValue(szMsgId, strlen(szMsgId), ma::MAP_FID_MSG_ID);

    m_ptrPacketMapOut->SetValue(1, "9301");
    m_ptrPacketMapOut->SetValue(bIsOk ? 0 : -1, "8900");
    m_ptrPacketMapOut->SetValue(szErrInfo, strlen(szErrInfo), "8901");
    m_ptrPacketMapOut->EndWrite();
    if((iRetCode = m_ptrPacketMapOut->Make(clMakeMsgData)) != MA_OK)
    {
      ma::ThrowError(NULL, "Failed to make packet map{@1}, {@2}", &(_P(__FUNCTION__) + _P(m_strLastErrorText.c_str())));
    }
  }
  _ma_catch_finally
  {
    if(iRetCode != MA_OK)
    {
      ma::ThrowError(NULL, "{@1},{@2}", &(_P(__FUNCTION__) + _P(m_strLastErrorText.c_str())));
    }
  }
  return iRetCode;
}

int CSvcFuncCosAdpSWReq::MakeCancel10388902(ma::CMsgData clMakeMsgDataIn,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData)
{
  int iRetCode = MA_OK;
  char szFuncId[8+1] = {"10388902"};
  char szMsgId[32 + 1] = {0};
  char szStationAddr[64 + 1] = {0};
  char szChannel[1 + 1] = {0};
  short siIntOrg = 0;
  char szCuacctCode[16 + 1] = {0};
  int  iCancleOrderNo = 0;
  long long llOrderNo = 0;
  _ma_try
  {
    if ((iRetCode = m_ptrPacketMapIn->Parse(clMakeMsgDataIn)) != MA_OK)
    {
      ThrowError(NULL, "CSvcFuncCosAdpSfitCtpReq::{@1} CTP Data Parse failed, return {@2}, PacketData:[{@3}]",
        &(_P(__FUNCTION__) + _P(iRetCode) + _P((char*)clMakeMsgDataIn.Data())));
      _ma_leave;
    }
    m_ptrPacketMapIn->GetValue(szStationAddr, sizeof(szStationAddr), "8810");
    m_ptrPacketMapIn->GetValue(siIntOrg, "8821");
    m_ptrPacketMapIn->GetValue(szChannel, sizeof(szChannel), "8815");
    m_ptrPacketMapIn->GetValue(szCuacctCode, sizeof(szCuacctCode), "8920");
    m_ptrPacketMapIn->GetValue(iCancleOrderNo, "8992");
    m_ptrPacketMapIn->GetValue(llOrderNo, "9106");

    m_ptrPacketMapOut->BeginWrite();
    SetPkgHead(m_ptrPacketMapOut, MAP_PKT_TYPE_BIZ, MAP_MSG_TYPE_REQ, 'T', szFuncId);
    SetRegular(m_ptrPacketMapOut, szCuacctCode, "", szFuncId, szStationAddr, siIntOrg, szChannel[0]);

    snprintf(szMsgId, sizeof(szMsgId), "0|%s|%d|C|", szCuacctCode, iCancleOrderNo);
    m_ptrPacketMapOut->SetHdrColValue(szMsgId, strlen(szMsgId), ma::MAP_FID_MSG_ID);
    m_ptrPacketMapOut->SetHdrColValue(ORDER_PUSH_TOPIC, strlen(ORDER_PUSH_TOPIC), ma::MAP_FID_PUB_TOPIC); 
    m_ptrPacketMapOut->SetHdrColValue(m_uiCurrNodeId, MAP_FID_SRC_NODE);
    m_ptrPacketMapOut->SetValue(szCuacctCode,strlen(szCuacctCode), "8920");
    m_ptrPacketMapOut->SetValue(llOrderNo, "9106");
    m_ptrPacketMapOut->SetValue(llOrderNo, "66");
    m_ptrPacketMapOut->SetValue(COS_ORDER_CANCEL, "9086");
    m_ptrPacketMapOut->SetValue(bIsOk ? 0 : -1, "8900");
    m_ptrPacketMapOut->SetValue(szErrInfo, strlen(szErrInfo), "8901");
    m_ptrPacketMapOut->EndWrite();

    if((iRetCode = m_ptrPacketMapOut->Make(clMakeMsgData)) != MA_OK)
    {
      ma::ThrowError(NULL, "Failed to make packet map{@1}, {@2}", &(_P(__FUNCTION__) + _P(m_strLastErrorText.c_str())));
    }
  }
  _ma_catch_finally
  {
    if(iRetCode != MA_OK)
    {
      ma::ThrowError(NULL, "{@1},{@2}", &(_P(__FUNCTION__) + _P(m_strLastErrorText.c_str())));
    }
  }
  return iRetCode;
}

int CSvcFuncCosAdpSWReq::DoWork(void *p_pvdParam)
{
  int	iRetCode = MA_OK;

  char szFuncId[16 +1] = {0};
  char szHead[64 + 1] = {0};
  short  siSubsysSn = 0;                   // 子系统编码
  long long   llRetOrderNo = 0LL;          //委托号
  char szSubsysSn[10] = {0};
  char szRetOrderNo[16 + 1] = {0};
  int iTrdDate = 0;
  std::string ssTrdOrderNo = "";
  ma::CMsgData  clMsgDataOut;
  _ma_try
  {
    if ((iRetCode = m_ptrServiceEnv->GetQueueData(m_clMsgDataIn, m_uiInQueId, m_uiSrcFuncId)) != MA_OK)
    {
      return	MA_NO_DATA;
    } 
    memcpy(szHead, (char*)m_clMsgDataIn.Data(), 64);
    xsdk::SubDelString(szFuncId, sizeof(szFuncId), szHead, 0, '|');
    xsdk::SubDelString(szSubsysSn, sizeof(szSubsysSn), szHead, 1, '|');
    xsdk::SubDelString(szRetOrderNo, sizeof(szRetOrderNo), szHead, 2, '|');

    siSubsysSn = atoi(szSubsysSn);
    llRetOrderNo = atol(szRetOrderNo);

    iRetCode = GetTrdDate(iTrdDate);
    if (0 == iTrdDate)
    {
      ThrowError(NULL, "CSvcFuncCosAdpSWReq::{@1} get TrdDate failed",
        &(_P(__FUNCTION__) + _P(iRetCode) ));
      _ma_leave;
    }

    // 获取当前时间，做时间控制
    long long llCurrentTime = 0LL;
    SYSTEMTIME stCurrentTime = {0}; 
    xsdk::GetCurrentTimestamp(stCurrentTime);
    xsdk::DatetimeToInt64(llCurrentTime, stCurrentTime);

    ssTrdOrderNo = CSvcFuncCosAdpSWReq::GetTrdOrderNo(llRetOrderNo, iTrdDate);

    // 委托
    if (0 == strcmp(szFuncId, COS_FUN_ID_10302001) || 0 == strcmp(szFuncId, COS_FUN_ID_10312008)
      || 0 == strcmp(szFuncId, COS_FUN_ID_10312001) || 0 == strcmp(szFuncId, COS_FUN_ID_10330003) )
    {
      iRetCode = m_ptrServiceEnv->PutQueueData(m_clMsgDataIn, m_uiOutQueId, m_uiSrcFuncId, "", 0);
      if (iRetCode != MA_OK)
      {
        ma::ThrowError(NULL, "CSvcFuncCosAdpSWReq::{@1} Put SWHY Data into OutQueue:{@2} Failed, iRetCode = {@3}", 
          &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode))); 

        Make10388902(m_clMsgDataIn, false, "风控连接失败,请检查配置与网络", clMsgDataOut);
        if ((iRetCode = m_ptrServiceEnv->PutQueueData(m_clMsgDataOut, 1001, m_uiSrcFuncId)) != MA_OK)
        {
          ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:1001 Failed return[{@3}]", 
            &(_P(__FUNCTION__)  + _P(iRetCode)));
        }
      }
      else
      {
        m_clRWLock.WriteLock();
        ST_ORDER_PARAM stOrderParam;

        stOrderParam.bIsCancel = FALSE;
        stOrderParam.clMsgData.Copy(m_clMsgDataIn);
        stOrderParam.llOrderTime = llCurrentTime;
        stOrderParam.siSubsysSn = siSubsysSn;

        m_mapCosOrderParam[ssTrdOrderNo] = stOrderParam;
        m_clRWLock.WriteUnlock();
       
      }
    }
    // 撤单
    else if( 0 == strcmp(szFuncId, COS_FUN_ID_10330004) || 0 == strcmp(szFuncId, COS_FUN_ID_10312002) 
      || 0 == strcmp(szFuncId, COS_FUN_ID_10302004))
    {
      iRetCode = m_ptrServiceEnv->PutQueueData(m_clMsgDataIn, m_uiOutQueId, m_uiSrcFuncId, "", 0);
      if (iRetCode != MA_OK)
      {
        ma::ThrowWarn(NULL, "CSvcFuncCosAdpSWReq::{@1} Put SWHY Data into OutQueue:{@2} Failed, iRetCode = {@3}", 
          &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode))); 
        CMsgData clMsgDataOut;

        MakeCancel10388902(m_clMsgDataIn, false, "撤单发送失败", clMsgDataOut);
        if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, 1001, m_uiSrcFuncId)) != MA_OK)
        {
          ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
            &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
          _ma_leave;
        }
      }
      else
      {
        CSvcFuncCosAdpSWReq::m_clRWLock.WriteLock();

        std::map<std::string, ST_ORDER_PARAM>::iterator itrOrder;  
        itrOrder = CSvcFuncCosAdpSWReq::m_mapCosOrderParam.find(ssTrdOrderNo);
        if (itrOrder == CSvcFuncCosAdpSWReq::m_mapCosOrderParam.end())
        {
          ST_ORDER_PARAM stOrderParam;

          stOrderParam.bIsCancel = TRUE;
          stOrderParam.clMsgData.Copy(m_clMsgDataIn);
          stOrderParam.llOrderTime = llCurrentTime;
          stOrderParam.siSubsysSn = siSubsysSn;
          m_mapCosOrderParam[ssTrdOrderNo] = stOrderParam;
        }
        else
        {
          itrOrder->second.bIsCancel = TRUE;
          itrOrder->second.llOrderTime = llCurrentTime;
        }
        CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
      }
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
CSvcFuncCosAdpSWAns::CSvcFuncCosAdpSWAns(void)
{
  m_uiCurrNodeId = 0;
  m_bIsRefuse = FALSE;
  m_bWriteLogFlag = FALSE;
  m_llOutTime = 500;
  m_bIsRefuseCancel = FALSE;
  m_llCancelOutTime = 500;
}

CSvcFuncCosAdpSWAns::~CSvcFuncCosAdpSWAns(void)
{
  m_uiCurrNodeId = 0;
}

int CSvcFuncCosAdpSWAns::Initialize(void)
{
  int iRetCode = MA_OK;

  _ma_try
  {
    if (m_ptrPacketMapOut.Create("CPacketMap").IsNull())
    {
      _ma_throw ma::CFuncException(MA_ERROR_UNDEFINED_OBJECT,
        "create object {@1} failed",
        &(_P("CPacketMap")));
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

int CSvcFuncCosAdpSWAns::Uninitialize(void)
{
  int iRetCode = MA_OK;

  _ma_try
  {
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

int CSvcFuncCosAdpSWAns::SetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize)
{
  int iRetCode = MA_OK;

  _ma_try
  {
    if (p_pvdValuePtr == NULL)
    {
      iRetCode = MA_ERROR_INVALID_PARAM;
      _ma_throw ma::CFuncException(iRetCode,
        "parameter '{@1}' of function '{@2}' invalid paramter ",
        &(CRtmParam("p_pvdValuePtr") + CRtmParam("CSvcFuncCosAptSfitCtp::SetOptions")));
    }
    char szTmp[10]={0};

    switch (p_iOptionInd)
    {
    case OPT_SVCFUNC_ID:
      m_uiSrcFuncId = *((unsigned int *)p_pvdValuePtr);
      break;
    case OPT_SVCFUNC_NAME:
      m_strSrcFuncName.assign((char *)p_pvdValuePtr);
      break;
    case OPT_SVCFUNC_INQUE:
      m_strInQueueId.assign((char *)p_pvdValuePtr);
      m_uiInQueId = atoi(m_strInQueueId.c_str());
      break;
    case OPT_SVCFUNC_OUTQUE:
      m_strOutQueueId.assign((char *)p_pvdValuePtr);
      memset(szTmp , 0 , sizeof(szTmp));
      xsdk::SubDelString(szTmp, sizeof(szTmp), m_strOutQueueId.c_str(), 0, ',');
      m_uiOutQueId = atoi(szTmp);

      memset(szTmp , 0 , sizeof(szTmp));
      xsdk::SubDelString(szTmp, sizeof(szTmp), m_strOutQueueId.c_str(), 1, ',');
      m_uiPutQueId = atoi(szTmp);
      break;
    default:
      iRetCode = MA_ERROR_OPTION_UNSUPPORTED;
      _ma_throw ma::CFuncException(iRetCode,
        "{@1} option {@2} unsupported",
        &(CRtmParam("CSvcFuncCosAptSfitCtp") + CRtmParam(p_iOptionInd)));
      break;
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

int CSvcFuncCosAdpSWAns::GetOptions(int p_iOptionInd, void *p_pvdValuePtr, int p_iValueSize)
{
  int iRetCode = MA_OK;

  _ma_try
  {
    if (p_pvdValuePtr == NULL)
    {
      iRetCode = MA_ERROR_INVALID_PARAM;
      _ma_throw ma::CFuncException(iRetCode,
        "invalid parameter '{@1}' of function '{@2}' ",
        &(CRtmParam("p_pvdValuePtr") + CRtmParam("CSvcFuncCosAptSfitCtp::GetOptions")));
    }

    if (p_iValueSize <= 0)
    {
      iRetCode = MA_ERROR_INVALID_PARAM;
      _ma_throw ma::CFuncException(iRetCode,
        "paramter {@1} of function {@2} invalid paramter",
        &(CRtmParam("p_iValueSize")+ CRtmParam("CSvcFuncCosAptSfitCtp::GetOptions")));
    }

    switch(p_iOptionInd)
    {
    case OPT_SVCFUNC_ID:
      *((unsigned int *)p_pvdValuePtr) = m_uiSrcFuncId;
      break;
    case OPT_SVCFUNC_NAME:
      memset(p_pvdValuePtr, 0x00, p_iValueSize);
      strncpy((char *)p_pvdValuePtr, m_strSrcFuncName.c_str(), p_iValueSize - 1);
      break;
    case OPT_SVCFUNC_INQUE:
      memset(p_pvdValuePtr, 0x00, p_iValueSize);
      strncpy((char *)p_pvdValuePtr, m_strInQueueId.c_str(), p_iValueSize - 1);
      break;
    case OPT_SVCFUNC_OUTQUE:
      memset(p_pvdValuePtr, 0x00, p_iValueSize);
      strncpy((char *)p_pvdValuePtr, m_strOutQueueId.c_str(), p_iValueSize - 1);
      break;
    default:
      iRetCode = MA_ERROR_OPTION_UNSUPPORTED;
      _ma_throw ma::CFuncException(iRetCode,
        "{@1} option {@2} unsupported",
        &(CRtmParam("CSvcFuncCosAptSfitCtp") + CRtmParam(p_iOptionInd)));
      break;
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

int CSvcFuncCosAdpSWAns::SetSvcEnv(ma::CObjectPtr<ma::IServiceEnv> &p_refptrSvcEnv)
{
  int iRetCode = MA_OK;
  char szValue[8] = {0};

  _ma_try
  {
    if (p_refptrSvcEnv.IsNull())
    {
      _ma_throw CException(MA_ERROR_INVALID_PARAM,
        "paramter {@1} of function {@2} invalid paramter",
        &(CRtmParam("p_pvdValuePtr")
        + CRtmParam(__FUNCTION__)));
    }
    if (m_ptrPacketMapOut.Create("CPacketMap").IsNull() || m_ptrDataEventPackage.Create("CDataEventPackage").IsNull())
    {
      _ma_throw ma::CFuncException(MA_ERROR_UNDEFINED_OBJECT,
        "create object {@1} failed",
        &(_P("CPacketMap")));
    }
    if (m_ptrPacketMapIn.Create("CPacketMap").IsNull())
    {
      _ma_throw ma::CFuncException(MA_ERROR_UNDEFINED_OBJECT,
        "create object {@1} failed",
        &(_P("CPacketMap")));
    }
    if (m_ptrPacketMapOut.Create("CPacketMap").IsNull())
    {
      _ma_throw ma::CFuncException(MA_ERROR_UNDEFINED_OBJECT,
        "create object {@1} failed",
        &(_P("CPacketMap")));
    }
    if(m_ptrBizOrderDataInit.Create("CBizOrderDataInit").IsNull()
      || m_ptrBizOrderDataInit->Initialize() != MA_OK)
    {
      _ma_throw ma::CBizException(MA_ERROR_INIT_OBJECT, "Initialize Object [{@1}] fail!",
        &(CRtmParam("CBizOrderDataInit")));
    }
    m_ptrServiceEnv = p_refptrSvcEnv;

    ma::g_hKernelEnv->UN_HANDLE.pclKernelEnv->GetOptions(ma::OPT_KERNEL_NODE_ID, &m_uiCurrNodeId, sizeof(m_uiCurrNodeId));
    memset(&m_stSysNode, 0x00, sizeof(ST_SYSMA_NODE));
    iRetCode = m_ptrServiceEnv->GetSysNode(m_stSysNode, m_uiCurrNodeId);
    // ma::g_hKernelEnv->UN_HANDLE.pclKernelEnv->GetOptions(ma::OPT_KERNEL_NODE_ID, &m_uiCurrNodeId, sizeof(m_uiCurrNodeId));
  
    char szOutTime[128] = {0},  szWirteLogFlag[32] = {0}, szMatchType[1 + 1] = {0}; 
     char szOutTime2[128] = {0},  szMatchType2[1 + 1] = {0}; 

    if (m_ptrServiceEnv->GetParamater(szOutTime, sizeof(szOutTime), "out_time", m_uiSrcFuncId) == MA_OK)
    {
      m_llOutTime  = atol(szOutTime);
      if (0 == m_llOutTime)
      {
        m_llOutTime = 500;
      }
    }
    if (m_ptrServiceEnv->GetParamater(szOutTime2, sizeof(szOutTime2), "out_time2", m_uiSrcFuncId) == MA_OK)
    {
      m_llCancelOutTime  = atol(szOutTime2);
      if (0 == m_llCancelOutTime)
      {
        m_llCancelOutTime = 500;
      }
    }
    if (m_ptrServiceEnv->GetParamater(szMatchType, sizeof(szMatchType), "match_type", m_uiSrcFuncId) == MA_OK)
    {
      if (atoi(szMatchType) == 0 )
      {
        m_bIsRefuse  = TRUE;
      }
      else
      {
        m_bIsRefuse = FALSE;
      }
    }
    if (m_ptrServiceEnv->GetParamater(szMatchType2, sizeof(szMatchType2), "match_type2", m_uiSrcFuncId) == MA_OK)
    {
      if (atoi(szMatchType2) == 0 )
      {
        m_bIsRefuseCancel  = TRUE;
      }
      else
      {
        m_bIsRefuseCancel = FALSE;
      }
    }
    if (m_ptrServiceEnv->GetParamater(szWirteLogFlag, sizeof(szWirteLogFlag), "write_log", m_uiSrcFuncId) == MA_OK)
    {
      if (stricmp("true", szWirteLogFlag) == 0)
      {
        m_bWriteLogFlag = TRUE;
      }
      else
      {
        m_bWriteLogFlag = FALSE;
      }     
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

int CSvcFuncCosAdpSWAns::OnEvent(unsigned int p_uiEventId, ma::CObjectPtr<ma::IData> &p_refptrEventData)
{
  int iRetCode = MA_OK;

  return iRetCode;
}

int CSvcFuncCosAdpSWAns::GetTrdDate(int &p_refTrdDate)
{
  int iRetCode = MA_OK;

  _ma_try
  {
    if(m_ptrBizOrderDataInit.Create("CBizOrderDataInit").IsNull()
      || m_ptrBizOrderDataInit->Initialize() != MA_OK
      || (iRetCode = m_ptrBizOrderDataInit->SetServiceDBEngine(m_ptrDBEngine)) != MA_OK)
    {
      iRetCode = MA_ERROR_OBJECT_UNINITIATED;
      ma::ThrowError(NULL, "CSFuncTsuGetReq[{@1}] CBizOrderDataInit Create Object fail!", &_P(__LINE__));
      _ma_leave;
    }
    if (m_ptrBizOrderDataInit->GetSysTrdDate(p_refTrdDate) != MA_OK)  //获取交易日期
    {
      _ma_throw ma::CFuncException(m_ptrBizOrderDataInit->GetLastErrorCode(), m_ptrBizOrderDataInit->GetLastErrorText()); 
    }
  }
  _ma_catch_finally
  {

  }
  return iRetCode;
}

int CSvcFuncCosAdpSWAns::CheckTimeOut(long long p_llCurrentTime)
{
  int iRetCode = MA_OK;  
  int iHandle = -1;
  char szMsgId[32 + 1] = {0};
  CMsgData clMsgDataOut;
  char szPubKey2[32 + 1] = {0};
  SYSTEMTIME stCurrentTime = {0};  
  char szLogBuf[1024] = {0};
  long long llCurrentTime = 0LL;
  std::map<std::string, ST_ORDER_PARAM>::iterator itrOrder;

  _ma_try
  {   
    CSvcFuncCosAdpSWReq::m_clRWLock.WriteLock();

    for (itrOrder = CSvcFuncCosAdpSWReq::m_mapCosOrderParam.begin(); itrOrder != CSvcFuncCosAdpSWReq::m_mapCosOrderParam.end();)
    {  
      if ((p_llCurrentTime - itrOrder->second.llOrderTime) >= m_llOutTime)
      {
        if (!itrOrder->second.bIsCancel)
        {
          // 委托
          if (m_bIsRefuse)
          {
            Make10388902(itrOrder->second.clMsgData, false, "风控应答超出预期,拒单", clMsgDataOut);
            if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
            {
              ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
                &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
            }
            if (m_bWriteLogFlag)
            {
              snprintf(szLogBuf, sizeof(szLogBuf), "ORDER_NO:[%d][%04d%02d%02d-%02d%02d%02d] %s", itrOrder->second.llOrderNo,  stCurrentTime.wYear, stCurrentTime.wMonth, stCurrentTime.wDay, stCurrentTime.wHour, stCurrentTime.wMinute, stCurrentTime.wSecond,
                "风控应答超出预期,拒单"); 
              m_clLogFile.Write(szLogBuf, strlen(szLogBuf));
              m_clLogFile.Write("\n", 1);
              m_clLogFile.Flush();
            }
          }  
          else
          {
            //  找到原委托应该出的队列
            if ((iRetCode = m_ptrServiceEnv->PutQueueData(itrOrder->second.clMsgData, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
            {
              ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
                &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
            }
            if (m_bWriteLogFlag)
            {
              snprintf(szLogBuf, sizeof(szLogBuf), "ORDER_NO:[%d][%04d%02d%02d-%02d%02d%02d] %s", itrOrder->second.llOrderNo,  stCurrentTime.wYear, stCurrentTime.wMonth, stCurrentTime.wDay, stCurrentTime.wHour, stCurrentTime.wMinute, stCurrentTime.wSecond,
                "风控应答超出预期,下单");  
              m_clLogFile.Write(szLogBuf, strlen(szLogBuf));
              m_clLogFile.Write("\n", 1);
              m_clLogFile.Flush();
            }
          }
        }
        else
        {
          // 撤单
           if (m_bIsRefuse)
           {
             MakeCancel10388902(itrOrder->second.clMsgData, false, "风控应答超时,拒单", clMsgDataOut);
             if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
             {
               ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
                 &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
             }
             if (m_bWriteLogFlag)
             {
               snprintf(szLogBuf, sizeof(szLogBuf), "ORDER_NO:[%d][%04d%02d%02d-%02d%02d%02d] %s", itrOrder->second.llOrderNo,  stCurrentTime.wYear, stCurrentTime.wMonth, stCurrentTime.wDay, stCurrentTime.wHour, stCurrentTime.wMinute, stCurrentTime.wSecond,
                 "风控应答超时,拒单"); 
               m_clLogFile.Write(szLogBuf, strlen(szLogBuf));
               m_clLogFile.Write("\n", 1);
               m_clLogFile.Flush();
             }
           }
           else
           {
             if ((iRetCode = m_ptrServiceEnv->PutQueueData(itrOrder->second.clMsgData, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
             {
               ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
                 &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
             }
             if (m_bWriteLogFlag)
             {
               snprintf(szLogBuf, sizeof(szLogBuf), "ORDER_NO:[%d][%04d%02d%02d-%02d%02d%02d] %s", itrOrder->second.llOrderNo,  stCurrentTime.wYear, stCurrentTime.wMonth, stCurrentTime.wDay, stCurrentTime.wHour, stCurrentTime.wMinute, stCurrentTime.wSecond,
                 "风控应答超时,继续撤单"); 
               m_clLogFile.Write(szLogBuf, strlen(szLogBuf));
               m_clLogFile.Write("\n", 1);
               m_clLogFile.Flush();
             }
           }
        }

      }
      else
      {
        itrOrder++;
      }

    }
    CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
  }
  _ma_catch_finally
  {
  }
  return iRetCode;
}

void CSvcFuncCosAdpSWAns::SetPkgHead(CObjectPtr<IPacketMap> &ptrPacketMap, char chPkgType, char chMsgType, char chFunType, const char *szFunID)
{
  ptrPacketMap->SetHdrColValue(chPkgType, MAP_FID_PKT_TYPE);
  ptrPacketMap->SetHdrColValue(chMsgType, MAP_FID_MSG_TYPE); 
  ptrPacketMap->SetHdrColValue("01", strlen("01"), MAP_FID_PKT_VER);
  ptrPacketMap->SetHdrColValue(chFunType, MAP_FID_FUNC_TYPE); 
  ptrPacketMap->SetHdrColValue(szFunID, strlen(szFunID), MAP_FID_FUNC_ID); 
  ptrPacketMap->SetHdrColValue('2', MAP_FID_RESEND_FLAG); // 重发标志：0正常，1重发, 2不需应答
  ptrPacketMap->SetHdrColValue(CToolKit::GetLongLongCurTime(), MAP_FID_TIMESTAMP);
  ptrPacketMap->SetHdrColValue('1', MAP_FID_TOKEN_FLAG);
}

void CSvcFuncCosAdpSWAns::SetRegular(CObjectPtr<IPacketMap> &ptrPacketMap, const char *p_pszCust, const char * szSession, const char *szFunID, const char * szOptSite, short siOpOrg, char chChannel)
{
  ptrPacketMap->SetValue(p_pszCust, strlen(p_pszCust), "8810");
  ptrPacketMap->SetValue('1', "8811");
  ptrPacketMap->SetValue(szOptSite, strlen(szOptSite), "8812");
  ptrPacketMap->SetValue(chChannel, "8813");
  strlen(szSession) < 1 ? ptrPacketMap->SetValue("123456", strlen("123456"), "8814") : 
    ptrPacketMap->SetValue(szSession, strlen(szSession), "8814");  
  ptrPacketMap->SetValue(szFunID, strlen(szFunID), "8815");

  char szCurrTimeStamp[24] = {0};
  SYSTEMTIME stTimestamp;
  xsdk::GetCurrentTimestamp(stTimestamp);
  xsdk::DatetimeToString(szCurrTimeStamp, sizeof(szCurrTimeStamp), "YYYY-MM-DD HH24:MI:SS.nnn", stTimestamp);
  ptrPacketMap->SetValue(szCurrTimeStamp, strlen(szCurrTimeStamp), "8816");

  ptrPacketMap->SetValue(siOpOrg, "8821");
}

int CSvcFuncCosAdpSWAns::Make10388902(ma::CMsgData clMakeMsgDataIn,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData)
{
  int iRetCode = MA_OK;
  char szFuncId[8+1] = {"10388902"};
  char szMsgId[32 + 1] = {0};
  char szStationAddr[64 + 1] = {0};
  char szChannel[1 + 1] = {0};
  short siIntOrg = 0;
  char szCuacctCode[16 + 1] = {0};
  char szUserSession[126 + 1] = {0};
  char szChannelId[16 + 1] = {0};
  long long llOrderNo = 0;
  int  iValueLen = 0;
  _ma_try
  {
    if ((iRetCode = m_ptrPacketMapIn->Parse(clMakeMsgDataIn)) != MA_OK)
    {
      ThrowError(NULL, "CSvcFuncCosAdpSfitCtpReq::{@1} CTP Data Parse failed, return {@2}, PacketData:[{@3}]",
        &(_P(__FUNCTION__) + _P(iRetCode) + _P((char*)m_clMsgDataIn.Data())));
      _ma_leave;
    }
    m_ptrPacketMapIn->GetValue(szStationAddr, sizeof(szStationAddr), "8810");
    m_ptrPacketMapIn->GetValue(siIntOrg, "8821");
    m_ptrPacketMapIn->GetValue(szChannel, sizeof(szChannel), "8815");
    m_ptrPacketMapIn->GetValue(szCuacctCode, sizeof(szCuacctCode), "8920");
    m_ptrPacketMapIn->GetValue(llOrderNo, "9106");
    
    m_ptrPacketMapIn->GetHdrColValue(szUserSession, sizeof(szUserSession), iValueLen, MAP_FID_USER_SESSION); 

    m_ptrPacketMapOut->BeginWrite();
    SetPkgHead(m_ptrPacketMapOut, MAP_PKT_TYPE_BIZ, MAP_MSG_TYPE_REQ, 'T', szFuncId);
    SetRegular(m_ptrPacketMapOut, szCuacctCode, "", szFuncId, szStationAddr, siIntOrg, szChannel[0]);
    m_ptrPacketMapOut->SetHdrColValue(szUserSession, strlen(szUserSession), MAP_FID_USER_SESSION); 
    m_ptrPacketMapOut->SetHdrColValue(szChannelId, strlen(szChannelId), MAP_FID_BIZ_CHANNEL); 
    snprintf(szMsgId, sizeof(szMsgId), "0|%s|%d|00000000", szCuacctCode, llOrderNo);						
    m_ptrPacketMapOut->SetHdrColValue(ORDER_PUSH_TOPIC, strlen(ORDER_PUSH_TOPIC), ma::MAP_FID_PUB_TOPIC); 
    m_ptrPacketMapOut->SetHdrColValue(szMsgId, strlen(szMsgId), ma::MAP_FID_MSG_ID);

    m_ptrPacketMapOut->SetValue(1, "9301");
    m_ptrPacketMapOut->SetValue(bIsOk ? 0 : -1, "8900");
    m_ptrPacketMapOut->SetValue(szErrInfo, strlen(szErrInfo), "8901");
    m_ptrPacketMapOut->EndWrite();
    if((iRetCode = m_ptrPacketMapOut->Make(clMakeMsgData)) != MA_OK)
    {
      ma::ThrowError(NULL, "Failed to make packet map{@1}, {@2}", &(_P(__FUNCTION__) + _P(m_strLastErrorText.c_str())));
    }
  }
  _ma_catch_finally
  {
    if(iRetCode != MA_OK)
    {
      ma::ThrowError(NULL, "{@1},{@2}", &(_P(__FUNCTION__) + _P(m_strLastErrorText.c_str())));
    }
  }
  return iRetCode;
}

int CSvcFuncCosAdpSWAns::MakeCancel10388902(ma::CMsgData clMakeMsgDataIn,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData)
{
  int iRetCode = MA_OK;
  char szFuncId[8+1] = {"10388902"};
  char szMsgId[32 + 1] = {0};
  char szStationAddr[64 + 1] = {0};
  char szChannel[1 + 1] = {0};
  short siIntOrg = 0;
  char szCuacctCode[16 + 1] = {0};
  int  iCancleOrderNo = 0;
  long long llOrderNo = 0;
  _ma_try
  {
    if ((iRetCode = m_ptrPacketMapIn->Parse(clMakeMsgDataIn)) != MA_OK)
    {
      ThrowError(NULL, "CSvcFuncCosAdpSfitCtpReq::{@1} CTP Data Parse failed, return {@2}, PacketData:[{@3}]",
        &(_P(__FUNCTION__) + _P(iRetCode) + _P((char*)clMakeMsgDataIn.Data())));
      _ma_leave;
    }
    m_ptrPacketMapIn->GetValue(szStationAddr, sizeof(szStationAddr), "8810");
    m_ptrPacketMapIn->GetValue(siIntOrg, "8821");
    m_ptrPacketMapIn->GetValue(szChannel, sizeof(szChannel), "8815");
    m_ptrPacketMapIn->GetValue(szCuacctCode, sizeof(szCuacctCode), "8920");
    m_ptrPacketMapIn->GetValue(iCancleOrderNo, "8992");
    m_ptrPacketMapIn->GetValue(llOrderNo, "9106");

    m_ptrPacketMapOut->BeginWrite();
    SetPkgHead(m_ptrPacketMapOut, MAP_PKT_TYPE_BIZ, MAP_MSG_TYPE_REQ, 'T', szFuncId);
    SetRegular(m_ptrPacketMapOut, szCuacctCode, "", szFuncId, szStationAddr, siIntOrg, szChannel[0]);
				
    snprintf(szMsgId, sizeof(szMsgId), "0|%s|%d|C|", szCuacctCode, iCancleOrderNo);
    m_ptrPacketMapOut->SetHdrColValue(szMsgId, strlen(szMsgId), ma::MAP_FID_MSG_ID);
    m_ptrPacketMapOut->SetHdrColValue(ORDER_PUSH_TOPIC, strlen(ORDER_PUSH_TOPIC), ma::MAP_FID_PUB_TOPIC); 
    m_ptrPacketMapOut->SetHdrColValue(m_uiCurrNodeId, MAP_FID_SRC_NODE);
    m_ptrPacketMapOut->SetValue(szCuacctCode,strlen(szCuacctCode), "8920");
    m_ptrPacketMapOut->SetValue(llOrderNo, "9106");
    m_ptrPacketMapOut->SetValue(llOrderNo, "66");
    m_ptrPacketMapOut->SetValue(COS_ORDER_CANCEL, "9086");
    m_ptrPacketMapOut->SetValue(bIsOk ? 0 : -1, "8900");
    m_ptrPacketMapOut->SetValue(szErrInfo, strlen(szErrInfo), "8901");
    m_ptrPacketMapOut->EndWrite();

    if((iRetCode = m_ptrPacketMapOut->Make(clMakeMsgData)) != MA_OK)
    {
      ma::ThrowError(NULL, "Failed to make packet map{@1}, {@2}", &(_P(__FUNCTION__) + _P(m_strLastErrorText.c_str())));
    }
  }
  _ma_catch_finally
  {
    if(iRetCode != MA_OK)
    {
      ma::ThrowError(NULL, "{@1},{@2}", &(_P(__FUNCTION__) + _P(m_strLastErrorText.c_str())));
    }
  }
  return iRetCode;
}

int CSvcFuncCosAdpSWAns::GetXaInFoFromName()
{
  int iRetCode = MA_OK;
  CObjectPtr<IDataSysbpuXa>            ptrDataXa;
  CObjectPtr<IDaoSysbpuXa>             ptrDaoXa;
  CObjectPtr<IDataSysbpuXaUidx1>       ptrDataSysbpuXaUidx1;
  CObjectPtr<IRuntimeDb>               ptrRuntimeDb;
  CObjectPtr<IDBEngine>                ptrRtdbDBEngine;
  int iHandle = -1;
  char* pszTemp = NULL;
  char szQueueId[8] = {0};
  char szOpen[128+1] = {0};

  _ma_try
  {
    m_ptrServiceEnv->GetRuntimeDb(ptrRuntimeDb);

    if (ptrRuntimeDb.IsNull()
      || ptrRuntimeDb->GetDBEngine(ptrRtdbDBEngine) != MA_OK
      || ptrRtdbDBEngine.IsNull())
    {
      iRetCode = MA_ERROR_OBJECT_UNINITIATED;
      ma::ThrowError(NULL, "CSFuncTsuGetReq[{@1}] ptrRuntimeDb::GetDBEngine(ptrRtdbDBEngine) fail!", &_P(__LINE__));
      _ma_leave;
    }

    if (ptrDataXa.Create("CDataSysbpuXa").IsNull()
      || ptrDaoXa.Create("CDaoSysbpuXa").IsNull()
      || ptrDataSysbpuXaUidx1.Create("CDataSysbpuXaUidx1").IsNull()
      || ptrDaoXa->SetDBEngine(ptrRtdbDBEngine) != MA_OK
      )
    {
      iRetCode = MA_ERROR_OBJECT_UNINITIATED;
      ma::ThrowError(NULL, "CSFuncTsuGetReq[{@1}] CDataSysbpuXa/CDaoSysbpuXa/CDataSysbpuXaUidx2 Create fail!", &_P(__LINE__));
      _ma_leave;
    }

    if (m_ptrDaoSubsysCfg.Create("CDaoSubsysCfg").IsNull()
      || m_ptrDataSubsysCfg.Create("CDataSubsysCfg").IsNull()
      || m_ptrDataSubsysCfgUidx1.Create("CDataSubsysCfgUidx1").IsNull()
      || m_ptrDataSubsysCfgEx1.Create("CDataSubsysCfgEx1").IsNull()
      || m_ptrDaoSubsysCfg->SetDBEngine(m_ptrDBEngine) != MA_OK)           
    {
      iRetCode = MA_ERROR_OBJECT_UNINITIATED;
      ma::ThrowError(NULL, "CSFuncTsuGetReq[{@1}] CDaoSubsysCfg/CDataSubsysCfg/CDataSubsysCfgUidx1/CDataSubsysCfgEx1 Create fail!", &_P(__LINE__));
      _ma_leave;
    }  

    m_ptrDataSubsysCfgEx1->SetInput(1);
    iRetCode = m_ptrDaoSubsysCfg->OpenCursor(iHandle, m_ptrDataSubsysCfg, m_ptrDataSubsysCfgEx1);
    if (iRetCode != MA_OK)
    {
      m_strLastErrorText = m_ptrDaoSubsysCfg->GetLastErrorText();
      m_iLastErrorCode = iRetCode;
      _ma_leave;
    }

    ST_XA stXa = {0};
    while(m_ptrDaoSubsysCfg->Fetch(iHandle) == MA_OK)
    {
      ptrDataSysbpuXaUidx1->Initialize();
      ptrDataSysbpuXaUidx1->SetXaId(m_ptrDataSubsysCfg->GetSubsysSn());
      iRetCode = ptrDaoXa->Select(ptrDataXa, ptrDataSysbpuXaUidx1);
      if (iRetCode != MA_OK && iRetCode != MA_NO_DATA)
      {
        m_strLastErrorText = ptrDaoXa->GetLastErrorText();
        m_ptrDaoSubsysCfg->CloseCursor(iHandle);
        _ma_leave;
      }
      else if (iRetCode == MA_NO_DATA)
      {
        continue;
      }
      else
      {
        snprintf(szOpen,sizeof(szOpen), "%s", ptrDataXa->GetXaOpen());
        pszTemp = strstr(szOpen, "trans_que=");
        if (pszTemp != NULL)
        {
          xsdk::SubTagString(szQueueId, sizeof(szQueueId), pszTemp, "trans_que", '=', ';');

          //将交易系统编码与查找到的队列加入map
          memset(&stXa, 0, sizeof(ST_XA));
          stXa.iQueueId = atoi(szQueueId);
          stXa.siSubSys = m_ptrDataSubsysCfg->GetSubsys();
          stXa.siSubSysSn = m_ptrDataSubsysCfg->GetSubsysSn();
          stXa.chSubSysStatus = m_ptrDataSubsysCfg->GetSubsysStatus();
          strcpy(stXa.szSubsysSnType, m_ptrDataSubsysCfg->GetSubsysType());
          strcpy(stXa.szSubsysConnstr, m_ptrDataSubsysCfg->GetSubsysConnstr());
          strcpy(stXa.szSubSysDbConnstr, m_ptrDataSubsysCfg->GetSubsysDbConnstr());
          m_mapSysSnQueue[stXa.siSubSysSn] = stXa;
        }
        else
        {
          ma::ThrowError(NULL,  "CSFuncTsuGetReq[{@1}] GetXaOpen[{@2}] have not trans_que=",
            &( _P(__LINE__) + _P(szOpen)));
        }
      }
    }
    m_ptrDaoSubsysCfg->CloseCursor(iHandle);
  }

  _ma_catch_finally
  {
    ptrRtdbDBEngine->Commit();
    COS_ERROR_PRINT
  }
  return iRetCode;
}

int CSvcFuncCosAdpSWAns::DoWork(void *p_pvdParam)
{
  int	iRetCode = MA_OK;

  long long   llOrderNo = 0LL;
  int iMsgCode = 0;
  int iTrdDate = 0;
  long long llCurrentTime = 0LL;

  char szMsgText[256 + 1] = {0};
  CMsgData clMsgDataIn;
  CMsgData clMsgDataOut;
  Json::Reader Reader;
  Json::Value  JsonValue;
  SYSTEMTIME stCurrentTime = {0};
  char szMsgDataIn[128] = {0};
  unsigned int uiOutQueId = 0;

  _ma_try
  {
    xsdk::GetCurrentTimestamp(stCurrentTime);
    xsdk::DatetimeToInt64(llCurrentTime, stCurrentTime);

    if (0 != CSvcFuncCosAdpSWReq::m_mapCosOrderParam.size())
    {  
      CheckTimeOut(llCurrentTime);
    }
    if ((iRetCode = m_ptrServiceEnv->GetQueueData(clMsgDataIn, m_uiInQueId, m_uiSrcFuncId)) != MA_OK)
    {
      return	MA_NO_DATA;
    }
    if (0 == CSvcFuncCosAdpSWReq::m_mapCosOrderParam.size())
    {
      return	MA_NO_DATA;
    }
    memcpy(szMsgDataIn, (char *)clMsgDataIn.Data(), sizeof(szMsgDataIn) - 1);

    if ('{' != szMsgDataIn[0])
    {
      return	MA_NO_DATA;
    }
    if (!(Reader.parse(szMsgDataIn, JsonValue)))
    {
      return	MA_NO_DATA;
    }
    if (!JsonValue.isMember("8817") && !JsonValue.isMember("9106") && !JsonValue.isMember("8819"))
    {
      return	MA_NO_DATA;
    }
    std::string  strMsgCode;
    std::string  strOrderNo;
    std::string  strUMsgText;

    if (JsonValue["8817"].isString())
    {
      strMsgCode = JsonValue["8817"].asString();
      iMsgCode = atoi(strMsgCode.c_str());
    }
    if (JsonValue["9106"].isString())
    {
      strOrderNo = JsonValue["9106"].asString();
      llOrderNo = atol(strOrderNo.c_str());
    }
    if(JsonValue["8819"].isString())
    {
      strUMsgText = JsonValue["8819"].asString();
    }

    // 风控编码为utf-8
    UTF8ToGB2312(szMsgText, sizeof(szMsgText), strUMsgText.c_str(), strUMsgText.length());

    iRetCode = GetTrdDate(iTrdDate);
    if (0 == iTrdDate)
    {
      ThrowError(NULL, "CSvcFuncCosAdpSWReq::{@1} get TrdDate failed",
        &(_P(__FUNCTION__) + _P(iRetCode) ));
      _ma_leave;
    }

    std::string ssTrdOrderNo = CSvcFuncCosAdpSWReq::GetTrdOrderNo(llOrderNo, iTrdDate);
    std::map<std::string, ST_ORDER_PARAM>::iterator itrOrder;

    CSvcFuncCosAdpSWReq::m_clRWLock.WriteLock();
    itrOrder = CSvcFuncCosAdpSWReq::m_mapCosOrderParam.find(ssTrdOrderNo);

    // 找到委托
    if (itrOrder != CSvcFuncCosAdpSWReq::m_mapCosOrderParam.end())
    {

      // 应答成功
      if (iMsgCode  == MA_OK)
      {
        // todo找到原委托需要发送队列
        // 成功直接把包发出去
        std::map<short, ST_XA>::iterator itrXa;
        itrXa = m_mapSysSnQueue.find(itrOrder->second.siSubsysSn);
        if(itrXa == m_mapSysSnQueue.end())
        {
          ma::ThrowError(NULL,"CSvcFuncCosAdpSWAns  not find SubsysSn[{@1}]",&(_P(itrOrder->second.siSubsysSn)));
          _ma_leave; 
        }
        if ((iRetCode = m_ptrServiceEnv->PutQueueData(itrOrder->second.clMsgData, itrXa->second.iQueueId, m_uiSrcFuncId)) != MA_OK)
        {
          ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
            &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
          CSvcFuncCosAdpSWReq::m_mapCosOrderParam.erase(itrOrder);
          CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
          _ma_leave;
        }
        CSvcFuncCosAdpSWReq::m_mapCosOrderParam.erase(itrOrder);
        CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
      }
      // 应答失败
      else  
      {
        // 下单委托
        if (!itrOrder->second.bIsCancel)
        {
          Make10388902(itrOrder->second.clMsgData, false, szMsgText, clMsgDataOut);
          if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
          {
            ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
              &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
            CSvcFuncCosAdpSWReq::m_mapCosOrderParam.erase(itrOrder);
            CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
            _ma_leave;
          }
        }
        else
        {
          // 撤单委托
          MakeCancel10388902(itrOrder->second.clMsgData, false, szMsgText, clMsgDataOut);
          if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
          {
            ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
              &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
            CSvcFuncCosAdpSWReq::m_mapCosOrderParam.erase(itrOrder);
            CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
            _ma_leave;
          }
        }
      }

      CSvcFuncCosAdpSWReq::m_mapCosOrderParam.erase(itrOrder);
      CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
    }
    // 不是本节点
    else 
    {
     
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}
