//--------------------------------------------------------------------------------------------------
// ��Ȩ������������ģ�����ڽ�֤΢�ں˼ܹ�ƽ̨(KMAP)��һ����
//           ����֤ȯ & ��֤�Ƽ�  ��Ȩ����
//
// �ļ����ƣ�SvcFuncCosAdpSW.cpp
// ģ�����ƣ������Դ���������
// ģ��������
// �������ߣ��
// �������ڣ�2019-07-25
// ģ��汾��1.0.000.000
//--------------------------------------------------------------------------------------------------
// �޸�����      �汾          ����            ��ע
//--------------------------------------------------------------------------------------------------
// 2019-07-25   1.0.000.000   �          ����
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
std::map<std::string, ST_CANCEL_PARAM> CSvcFuncCosAdpSWReq::m_mapCosCancelParam;

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
      m_mapCosCancelParam.clear();
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
      m_mapCosCancelParam.clear();
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

int  CSvcFuncCosAdpSWReq::MakeCancelParam(ST_CANCEL_PARAM &p_stCancelParam)
{
  int iRetCode = MA_OK;
  int iValueLen = 0;
  long long llCurrentTime = 0LL;
  SYSTEMTIME stCurrentTime = {0}; 
  xsdk::GetCurrentTimestamp(stCurrentTime);
  _ma_try
  {
    xsdk::DatetimeToInt64(llCurrentTime, stCurrentTime);
    p_stCancelParam.llOrderTime = llCurrentTime;    //��ʱ�����
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szStationAddr, sizeof(p_stCancelParam.szStationAddr), "8812");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szChannel, sizeof(p_stCancelParam.szChannel), "8813");
    m_ptrPacketMapIn->GetHdrColValue(p_stCancelParam.szUserSession, sizeof(p_stCancelParam.szUserSession), iValueLen, MAP_FID_USER_SESSION);
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szSession, sizeof(p_stCancelParam.szSession), "8814");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szCuacctCode,sizeof(p_stCancelParam.szCuacctCode), "8920");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.siIntOrg, "8911");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szStkbd, sizeof(p_stCancelParam.szStkbd),"625");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.iOrderDate, "8834");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.iOrderNo, "9106");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.iOrderBsn, "66");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szOrderId, sizeof(p_stCancelParam.szOrderId), "11");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.siAttrCode,  "9101");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szCuacctType, sizeof(p_stCancelParam.szCuacctType), "8826");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szCliRemark, sizeof(p_stCancelParam.szCliRemark), "8914");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.iPassNum,  "8828");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szCliOrderNo, sizeof(p_stCancelParam.szCliOrderNo), "9102");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szCustCode, sizeof(p_stCancelParam.szCustCode), "8810");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.iCancleOrderNo, "8992");
    m_ptrPacketMapIn->GetValue(p_stCancelParam.szChannelId, sizeof(p_stCancelParam.szChannelId), "9082");
    p_stCancelParam.szExSystem[0] = '@';
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}

int  CSvcFuncCosAdpSWReq::MakeOrderParam(ST_ORDER_PARAM &p_stOrderParam)
{
  int iRetCode = MA_OK;
  int iValueLen = 0;
  long long llCurrentTime = 0LL;
  SYSTEMTIME stCurrentTime = {0}; 
  xsdk::GetCurrentTimestamp(stCurrentTime);
  _ma_try
  {
    xsdk::DatetimeToInt64(llCurrentTime, stCurrentTime);
    p_stOrderParam.llOrderTime = llCurrentTime;    //��ʱ�����
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szCuacctCode, sizeof(p_stOrderParam.szCuacctCode), "8810");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szStationAddr, sizeof(p_stOrderParam.szStationAddr), "8812");
    m_ptrPacketMapIn->GetHdrColValue(p_stOrderParam.szUserSession, sizeof(p_stOrderParam.szUserSession), iValueLen, MAP_FID_USER_SESSION);
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szChannel, sizeof(p_stOrderParam.szChannel), "8813");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.iFuncId,"8815");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szSession, sizeof(p_stOrderParam.szSession), "8814");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szCuacctCode, sizeof(p_stOrderParam.szCuacctCode),"8920");//
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szCuacctType, sizeof(p_stOrderParam.szCuacctType), "8826");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szCustCode, sizeof(p_stOrderParam.szCustCode),"8902");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szTrdacct, sizeof(p_stOrderParam.szTrdacct), "448");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szExchange, sizeof(p_stOrderParam.szExchange), "207");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szStkbd, sizeof(p_stOrderParam.szStkbd), "625");//
    m_ptrPacketMapIn->GetValue(p_stOrderParam.siStkBiz, "8842");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.siStkBizAction, "40");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szTrdCode, sizeof(p_stOrderParam.szTrdCode), "48");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szOptNum, sizeof(p_stOrderParam.szOptNum), "9082");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.siIntOrg, "8911");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.iOrderBsn, "66");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.llOrderQty, "38");

    char szTempBuff[64 + 1] = {0};
    m_ptrPacketMapIn->GetValue(szTempBuff,sizeof(szTempBuff), "44");
    p_stOrderParam.llOrderPrice = xsdk::CPrice4(szTempBuff).CvtToLonglong();
    m_ptrPacketMapIn->GetValue(szTempBuff,sizeof(szTempBuff), "8975"); 
    p_stOrderParam.llStopPrice = xsdk::CPrice4(szTempBuff).CvtToLonglong();

    m_ptrPacketMapIn->GetValue(p_stOrderParam.iValidDate, "8859");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szTrdCodeCls, sizeof(p_stOrderParam.szTrdCodeCls), "8970");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szOrderAttr, sizeof(p_stOrderParam.szOrderAttr), "9100");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.siAttrCode, "9101");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.iBgnExeTime, "916");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.iEndExeTime, "917");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szTimeUnit, sizeof(p_stOrderParam.szTimeUnit), "918");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szSpreadName, sizeof(p_stOrderParam.szSpreadName), "8971");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szUndlCode, sizeof(p_stOrderParam.szUndlCode), "8972");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.iConExpDate, "8976");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szExercisePrice, sizeof(p_stOrderParam.szExercisePrice), "8973"); 
    m_ptrPacketMapIn->GetValue(p_stOrderParam.llConUnit, "8974");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szCliOrderNo, sizeof(p_stOrderParam.szCliOrderNo), "9102");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szCliRemark, sizeof(p_stOrderParam.szCliRemark), "8914");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szBusinessUnit, sizeof(p_stOrderParam.szBusinessUnit), "8717");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szGtdData, sizeof(p_stOrderParam.szGtdData), "8723");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szContingentCondition, sizeof(p_stOrderParam.szContingentCondition), "8713");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szForceCloseReason, sizeof(p_stOrderParam.szForceCloseReason), "8715");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.siIsSwapOrder, "8720");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szCombOffsetFlag, sizeof(p_stOrderParam.szCombOffsetFlag), "8741");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szOrderIdEx, sizeof(p_stOrderParam.szOrderIdEx), "9093");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szComponetStkCode, sizeof(p_stOrderParam.szComponetStkCode), "8963");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szComponetStkbd, sizeof(p_stOrderParam.szComponetStkbd),"8962");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szStkbdLink, sizeof(p_stOrderParam.szStkbdLink), "17");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szTrdacctLink, sizeof(p_stOrderParam.szTrdacctLink), "8964");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szRtgsFlag, sizeof(p_stOrderParam.szRtgsFlag), "9090");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szStkBizCtvFlag, sizeof(p_stOrderParam.szStkBizCtvFlag), "9094");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.iPassNum, "8828");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.chOrderFuncType, "9140");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.iConferNum, "70");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szTargetTrader, sizeof(p_stOrderParam.szTargetTrader), "71");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szTargetCompany, sizeof(p_stOrderParam.szTargetCompany), "72");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szTraderId, sizeof(p_stOrderParam.szTraderId), "73");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szSupplemental, sizeof(p_stOrderParam.szSupplemental), "75");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.iOrderNo, "9106");
    m_ptrPacketMapIn->GetValue(p_stOrderParam.szChannelId, sizeof(p_stOrderParam.szChannelId), "9082");
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
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
    if (m_ptrBizOrderDataInit->GetSysTrdDate(p_refTrdDate) != MA_OK)  //��ȡ��������
    {
      _ma_throw ma::CFuncException(m_ptrBizOrderDataInit->GetLastErrorCode(), m_ptrBizOrderDataInit->GetLastErrorText()); 
    }
  }
  _ma_catch_finally
  {
  }
  return MA_OK;
}

std::string CSvcFuncCosAdpSWReq::GetTrdOrderNo(int iOrderNo, int iTrdDate)
{
  char szTrdCuacctCode[32] = {0};
  snprintf(szTrdCuacctCode,sizeof(szTrdCuacctCode), "%d%d", iOrderNo, iTrdDate);
  return szTrdCuacctCode;
}

int CSvcFuncCosAdpSWReq::DoWork(void *p_pvdParam)
{
  int	iRetCode = MA_OK;
  int iValueLen = 0;
  char szFuncId[16 + 1] = {0};
  char szCuacctCode[16 + 1] = {0};
  char szUserSession[32 + 1] ={0};
  int  iOrderNo = 0;
  int  iTrdDate = 0;
  char szMsgText[256 + 1] = {0};
  char szMsgId[ 32 + 1] = {0};
  int iFuncId = 0;
  char  szStationAddr[256+1] = {0};
  short siStkBiz  = 0;
  short siStkBizAction = 0;
  std::string ssTrdOrderNo = "";
  char szAttrCode[1 + 1] = {0};
  unsigned int uiEventId = 0;
  char chCuacctType = 0X00;
  long long llCurrentTime = 0LL;
  char szChannel[1 + 1] = {0};
  char szSession[256 + 1] = {0};
  SYSTEMTIME stCurrentTime = {0};
  m_clMsgDataIn.Close();
  m_clMsgDataOut.Close();
  _ma_try
  {
    if ((iRetCode = m_ptrServiceEnv->GetQueueData(m_clMsgDataIn, m_uiInQueId, m_uiSrcFuncId)) != MA_OK)
    {
      return	MA_NO_DATA;
    } 
    if ((iRetCode = m_ptrPacketMapIn->Parse(m_clMsgDataIn)) != MA_OK)
    {
      ThrowError(NULL, "CSvcFuncCosAdpSWReq::{@1} SW Data Parse failed, return {@2}, PacketData:[{@3}]",
        &(_P(__FUNCTION__) + _P(iRetCode) + _P((char*)m_clMsgDataIn.Data())));
      _ma_leave;
    }

    m_ptrPacketMapIn->GetValue(szCuacctCode, sizeof(szCuacctCode), "8920");
    m_ptrPacketMapIn->GetValue(chCuacctType, "8826");
    m_ptrPacketMapIn->GetValue(szChannel,  sizeof(szChannel), "8813");
    m_ptrPacketMapIn->GetValue(szStationAddr, sizeof(szStationAddr), "8812");
    m_ptrPacketMapIn->GetValue(szSession, sizeof(szSession), "8814");
    m_ptrPacketMapIn->GetHdrColValue(szUserSession, sizeof(szUserSession), iValueLen, MAP_FID_USER_SESSION);
    m_ptrPacketMapIn->GetHdrColValue(szFuncId, sizeof(szFuncId), iValueLen, MAP_FID_FUNC_ID);
    m_ptrPacketMapIn->GetValue(iOrderNo, "9106");
    if (iOrderNo == 0)
    {
      ThrowError(NULL, "CSvcFuncCosAdpSWReq::{@1} get OrderNO:{@2} failed",
        &(_P(__FUNCTION__) + _P(iOrderNo) ));
      _ma_leave;
    }
    iRetCode = GetTrdDate(iTrdDate);
    if (0 == iTrdDate)
    {
      ThrowError(NULL, "CSvcFuncCosAdpSWReq::{@1} get TrdDate failed",
        &(_P(__FUNCTION__) + _P(iRetCode) ));
      _ma_leave;
    }
    ssTrdOrderNo = CSvcFuncCosAdpSWReq::GetTrdOrderNo(iOrderNo, iTrdDate);
    // ί��
    if (0 == strcmp(szFuncId, COS_FUN_ID_10302001) || 0 == strcmp(szFuncId, COS_FUN_ID_10312008)
      || 0 == strcmp(szFuncId, COS_FUN_ID_10312001) || 0 == strcmp(szFuncId, COS_FUN_ID_10330003) )
    {
      //ThrowInfo(NULL, "ί������:{@1}", &(_P((char *)m_clMsgDataOut.Data())));
      iRetCode = m_ptrServiceEnv->PutQueueData(m_clMsgDataIn, m_uiOutQueId, m_uiSrcFuncId, "", 0);
      if (iRetCode != MA_OK)
      {
        ma::ThrowWarn(NULL, "CSvcFuncCosAdpSWReq::{@1} Put SWHY Data into OutQueue:{@2} Failed, iRetCode = {@3}", 
          &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode))); 

        xsdk::GetCurrentTimestamp(stCurrentTime);
        strcpy(szMsgText, "����������ϵͳ����ʧ�ܣ���������������");
        m_ptrPacketMapIn->GetValue(szCuacctCode, sizeof(szCuacctCode), "8810");
        sprintf(szMsgId, "%lld|%s|%d|", 0, szCuacctCode, iOrderNo);
        m_ptrPacketMapOut->BeginWrite();
        m_ptrPacketMapOut->SetHdrColValue("01", sizeof("01"), MAP_FID_PKT_VER);							
        m_ptrPacketMapOut->SetHdrColValue(MAP_MSG_TYPE_REQ, MAP_FID_MSG_TYPE);									
        m_ptrPacketMapOut->SetHdrColValue('1', MAP_FID_TOKEN_FLAG);						
        m_ptrPacketMapOut->SetHdrColValue('T', MAP_FID_FUNC_TYPE);			
        m_ptrPacketMapOut->SetHdrColValue(szUserSession, strlen(szUserSession), MAP_FID_USER_SESSION);
        m_ptrPacketMapOut->SetHdrColValue("10388902", strlen("10388902"), MAP_FID_FUNC_ID);
        m_ptrPacketMapOut->SetHdrColValue(MAP_PKT_TYPE_BIZ, MAP_FID_PKT_TYPE);   
        xsdk::DatetimeToInt64(llCurrentTime, stCurrentTime);
        m_ptrPacketMapOut->SetHdrColValue(llCurrentTime, MAP_FID_TIMESTAMP);
        m_ptrPacketMapOut->SetHdrColValue(ORDER_PUSH_TOPIC, strlen(ORDER_PUSH_TOPIC), ma::MAP_FID_PUB_TOPIC); 
        m_ptrPacketMapOut->SetHdrColValue(szMsgId, strlen(szMsgId), ma::MAP_FID_MSG_ID);
        m_ptrPacketMapOut->SetHdrColValue('0', MAP_FID_RESEND_FLAG);
        m_ptrPacketMapOut->SetValue(szCuacctCode, strlen(szCuacctCode), "8810");
        m_ptrPacketMapOut->SetValue("1", strlen("1"), "8811");
        m_ptrPacketMapOut->SetValue(szStationAddr, strlen(szStationAddr), "8812");
        m_ptrPacketMapOut->SetValue(szChannel, strlen(szChannel), "8813");
        m_ptrPacketMapOut->SetValue(szSession, strlen(szSession), "8814");
        m_ptrPacketMapOut->SetValue(COS_FUN_ID_10388902_INT, "8815");
        m_ptrPacketMapOut->SetValue(llCurrentTime, "8816");
        m_ptrPacketMapOut->SetValue(1, "9301");
        m_ptrPacketMapOut->SetValue('100', "8900");
        m_ptrPacketMapOut->SetValue(szMsgText, strlen(szMsgText), "8901");
        m_ptrPacketMapOut->EndWrite();  
        iRetCode = m_ptrPacketMapOut->Make(m_clMsgDataOut);
        if ((iRetCode = m_ptrServiceEnv->PutQueueData(m_clMsgDataOut, 1001, m_uiSrcFuncId)) != MA_OK)
        {
          ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:1001 Failed return[{@3}]", 
            &(_P(__FUNCTION__)  + _P(iRetCode)));
        }
        _ma_leave;
      }
      else
      {
        ST_ORDER_PARAM stOrderParam;
        MakeOrderParam(stOrderParam);
        m_clRWLock.WriteLock();
        m_mapCosOrderParam[ssTrdOrderNo] = stOrderParam;
        m_clRWLock.WriteUnlock();
       
      }
    }
    // ����
    else if( 0 == strcmp(szFuncId, COS_FUN_ID_10330004) || 0 == strcmp(szFuncId, COS_FUN_ID_10312002) 
      || 0 == strcmp(szFuncId, COS_FUN_ID_10302004))
    {
      std::map<std::string, ST_CANCEL_PARAM>::iterator itrCancel;     
      CSvcFuncCosAdpSWReq::m_clRWLock.WriteLock();
      itrCancel = CSvcFuncCosAdpSWReq::m_mapCosCancelParam.find(ssTrdOrderNo);
      if (itrCancel != CSvcFuncCosAdpSWReq::m_mapCosCancelParam.end())
      {
        CSvcFuncCosAdpSWReq::m_mapCosCancelParam.erase(itrCancel);
      }
      ST_CANCEL_PARAM stCancelParam;
      MakeCancelParam(stCancelParam);
      m_mapCosCancelParam[ssTrdOrderNo] = stCancelParam;
      CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
      iRetCode = m_ptrServiceEnv->PutQueueData(m_clMsgDataIn, m_uiOutQueId, m_uiSrcFuncId, "", 0);
      if (iRetCode != MA_OK)
      {
        ma::ThrowWarn(NULL, "CSvcFuncCosAdpSWReq::{@1} Put SWHY Data into OutQueue:{@2} Failed, iRetCode = {@3}", 
          &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode))); 
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
    if (m_ptrBizOrderDataInit->GetSysTrdDate(p_refTrdDate) != MA_OK)  //��ȡ��������
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
  std::map<std::string, ST_CANCEL_PARAM>::iterator itrCancel;

  _ma_try
  {   
    CSvcFuncCosAdpSWReq::m_clRWLock.WriteLock();
    for (itrOrder = CSvcFuncCosAdpSWReq::m_mapCosOrderParam.begin(); itrOrder != CSvcFuncCosAdpSWReq::m_mapCosOrderParam.end();)
    {  
      if ((p_llCurrentTime - itrOrder->second.llOrderTime) >= m_llOutTime)
      {
        if (m_bIsRefuse)
        {
          Make10388902(itrOrder->second, false, "���Ӧ�𳬳�Ԥ��,�ܵ�", clMsgDataOut);
          if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
          {
            ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
              &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
          }
          if (m_bWriteLogFlag)
          {
            snprintf(szLogBuf, sizeof(szLogBuf), "ORDER_NO:[%d][%04d%02d%02d-%02d%02d%02d] %s", itrOrder->second.iOrderNo,  stCurrentTime.wYear, stCurrentTime.wMonth, stCurrentTime.wDay, stCurrentTime.wHour, stCurrentTime.wMinute, stCurrentTime.wSecond,
              "���Ӧ�𳬳�Ԥ��,�ܵ�"); 
            m_clLogFile.Write(szLogBuf, strlen(szLogBuf));
            m_clLogFile.Write("\n", 1);
            m_clLogFile.Flush();
          }
        }  
        else
        {
          Make10388105(itrOrder->second, clMsgDataOut);
          if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
          {
            ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
              &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
          }
          if (m_bWriteLogFlag)
          {
            snprintf(szLogBuf, sizeof(szLogBuf), "ORDER_NO:[%d][%04d%02d%02d-%02d%02d%02d] %s", itrOrder->second.iOrderNo,  stCurrentTime.wYear, stCurrentTime.wMonth, stCurrentTime.wDay, stCurrentTime.wHour, stCurrentTime.wMinute, stCurrentTime.wSecond,
              "���Ӧ�𳬳�Ԥ��,�µ�");  
            m_clLogFile.Write(szLogBuf, strlen(szLogBuf));
            m_clLogFile.Write("\n", 1);
            m_clLogFile.Flush();
          }
        }
        CSvcFuncCosAdpSWReq::m_mapCosOrderParam.erase(itrOrder++);
      }
      else
      {
        itrOrder++;
      }

    }
    for (itrCancel = CSvcFuncCosAdpSWReq::m_mapCosCancelParam.begin(); itrCancel != CSvcFuncCosAdpSWReq::m_mapCosCancelParam.end();)
    {  
      if ((p_llCurrentTime - itrCancel->second.llOrderTime) >= m_llCancelOutTime)
      {
        if (m_bIsRefuseCancel)
        {
          MakeCancel10388902(itrCancel->second, false, "��س���Ӧ�𳬳�Ԥ��,�ܵ�", clMsgDataOut);
          if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
          {
            ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
              &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
          }
          if (m_bWriteLogFlag)
          {
            snprintf(szLogBuf, sizeof(szLogBuf), "ORDER_NO:[%d][%04d%02d%02d-%02d%02d%02d] %s", itrOrder->second.iOrderNo,  stCurrentTime.wYear, stCurrentTime.wMonth, stCurrentTime.wDay, stCurrentTime.wHour, stCurrentTime.wMinute, stCurrentTime.wSecond,
              "��س���Ӧ�𳬳�Ԥ��,�ܵ�"); 
            m_clLogFile.Write(szLogBuf, strlen(szLogBuf));
            m_clLogFile.Write("\n", 1);
            m_clLogFile.Flush();
          }
        }  
        else
        {
          Make10388102(itrCancel->second, clMsgDataOut);
          m_ptrPacketMapOut->Make(clMsgDataOut);
          if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
          {
            ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}],text:{@4}", 
              &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode) + _P(m_ptrServiceEnv->GetLastErrorText())));
          }
          if (m_bWriteLogFlag)
          {
            snprintf(szLogBuf, sizeof(szLogBuf), "ORDER_NO:[%d][%04d%02d%02d-%02d%02d%02d] %s", itrCancel->second.iOrderNo,  stCurrentTime.wYear, stCurrentTime.wMonth, stCurrentTime.wDay, stCurrentTime.wHour, stCurrentTime.wMinute, stCurrentTime.wSecond,
              "��س���Ӧ�𳬳�Ԥ��,����");  
            m_clLogFile.Write(szLogBuf, strlen(szLogBuf));
            m_clLogFile.Write("\n", 1);
            m_clLogFile.Flush();
          }
        }
        CSvcFuncCosAdpSWReq::m_mapCosCancelParam.erase(itrCancel++);
      }
      else
      {
        itrCancel++;
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
  ptrPacketMap->SetHdrColValue('2', MAP_FID_RESEND_FLAG); // �ط���־��0������1�ط�, 2����Ӧ��
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

int CSvcFuncCosAdpSWAns::Make10388902(const ST_ORDER_PARAM &stOrderParam,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData)
{
  int iRetCode = MA_OK;
  char szFuncId[8+1] = {"10388902"};
  char szMsgId[32 + 1] = {0};
  _ma_try
  {
    m_ptrPacketMapOut->BeginWrite();
    SetPkgHead(m_ptrPacketMapOut, MAP_PKT_TYPE_BIZ, MAP_MSG_TYPE_REQ, 'T', szFuncId);
    SetRegular(m_ptrPacketMapOut, stOrderParam.szCuacctCode, "", szFuncId, stOrderParam.szStationAddr, stOrderParam.siIntOrg, stOrderParam.szChannel[0]);
    m_ptrPacketMapOut->SetHdrColValue(stOrderParam.szUserSession, strlen(stOrderParam.szUserSession), MAP_FID_USER_SESSION); 
    m_ptrPacketMapOut->SetHdrColValue(stOrderParam.szChannelId, strlen(stOrderParam.szChannelId), MAP_FID_BIZ_CHANNEL); 
    snprintf(szMsgId, sizeof(szMsgId), "0|%s|%d|00000000", stOrderParam.szCuacctCode, stOrderParam.iOrderNo);						
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
    DumpMsgData(clMakeMsgData);
  }
  return iRetCode;
}

int CSvcFuncCosAdpSWAns::MakeCancel10388902(const ST_CANCEL_PARAM &stCancelParam,  bool bIsOk, const char * szErrInfo, CMsgData &clMakeMsgData)
{
  int iRetCode = MA_OK;
  char szFuncId[8+1] = {"10388902"};
  char szMsgId[32 + 1] = {0};
  _ma_try
  {
    m_ptrPacketMapOut->BeginWrite();
    SetPkgHead(m_ptrPacketMapOut, MAP_PKT_TYPE_BIZ, MAP_MSG_TYPE_REQ, 'T', szFuncId);
    SetRegular(m_ptrPacketMapOut, stCancelParam.szCuacctCode, "", szFuncId, stCancelParam.szStationAddr, stCancelParam.siIntOrg, stCancelParam.szChannel[0]);
				
    snprintf(szMsgId, sizeof(szMsgId), "0|%s|%d|C|", stCancelParam.szCuacctCode, stCancelParam.iCancleOrderNo);
    m_ptrPacketMapOut->SetHdrColValue(szMsgId, strlen(szMsgId), ma::MAP_FID_MSG_ID);
    m_ptrPacketMapOut->SetHdrColValue(ORDER_PUSH_TOPIC, strlen(ORDER_PUSH_TOPIC), ma::MAP_FID_PUB_TOPIC); 
    m_ptrPacketMapOut->SetHdrColValue(m_uiCurrNodeId, MAP_FID_SRC_NODE);
    ////////////
    m_ptrPacketMapOut->SetValue(stCancelParam.szCuacctCode,strlen(stCancelParam.szCuacctCode), "8920");
    m_ptrPacketMapOut->SetValue(stCancelParam.szStkbd, strlen(stCancelParam.szStkbd), "625");
    m_ptrPacketMapOut->SetValue(stCancelParam.iOrderDate, "8834");
    m_ptrPacketMapOut->SetValue(stCancelParam.iOrderNo, "9106");
    m_ptrPacketMapOut->SetValue(stCancelParam.iOrderBsn, "66");
    m_ptrPacketMapOut->SetValue(stCancelParam.szOrderId, strlen(stCancelParam.szOrderId), "11");
    m_ptrPacketMapOut->SetValue(stCancelParam.siAttrCode, "9101");
    m_ptrPacketMapOut->SetValue(stCancelParam.szCuacctType, strlen(stCancelParam.szCuacctType), "8826");
    m_ptrPacketMapOut->SetValue(stCancelParam.szCliRemark, strlen(stCancelParam.szCliRemark), "8914");
    m_ptrPacketMapOut->SetValue(stCancelParam.iPassNum, "8828");
    m_ptrPacketMapOut->SetValue(stCancelParam.szCliOrderNo, strlen(stCancelParam.szCliOrderNo), "9102");
    //��������״̬���ᵼ�¿ͻ����յ���Ϣ����ʾ����״̬�� iMsgCode����
    //m_ptrPacketMapOut->SetValue(ORDER_EXE_STATUS_INVALIDE, "9103");

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
    DumpMsgData(clMakeMsgData);
  }
  return iRetCode;
}

int CSvcFuncCosAdpSWAns::Make10388102(const ST_CANCEL_PARAM &stCancelParam, CMsgData &clMakeMsgData)
{
  int iRetCode = MA_OK;
  char szFuncId[8+1] = {"10388102"};
  char szPktVer[2+1] = {"01"};
  char szMsgId[32 + 1] = {0};
  _ma_try
  {
    m_ptrPacketMapOut->BeginWrite();
    SetPkgHead(m_ptrPacketMapOut, MAP_PKT_TYPE_BIZ, MAP_MSG_TYPE_REQ, 'T', szFuncId);
    SetRegular(m_ptrPacketMapOut, stCancelParam.szCuacctCode, "", szFuncId, stCancelParam.szStationAddr, stCancelParam.siIntOrg, stCancelParam.szChannel[0]);
    m_ptrPacketMapOut->SetHdrColValue(stCancelParam.szUserSession, strlen(stCancelParam.szUserSession), MAP_FID_USER_SESSION);
    m_ptrPacketMapOut->SetHdrColValue(stCancelParam.szChannel, strlen(stCancelParam.szChannel), MAP_FID_BIZ_CHANNEL);
    snprintf(szMsgId, sizeof(szMsgId), "0|%s|%d|00000000", stCancelParam.szCuacctCode, stCancelParam.iOrderNo);						
    m_ptrPacketMapOut->SetHdrColValue(szMsgId, strlen(szMsgId), ma::MAP_FID_MSG_ID);

    m_ptrPacketMapOut->SetValue(stCancelParam.szCuacctCode, strlen(stCancelParam.szCuacctCode),"8920");
    m_ptrPacketMapOut->SetValue(stCancelParam.szCuacctType,strlen(stCancelParam.szCuacctType),"8826");
    m_ptrPacketMapOut->SetValue(stCancelParam.szStkbd, strlen(stCancelParam.szStkbd), "625");
    m_ptrPacketMapOut->SetValue(stCancelParam.siIntOrg, "8911");  
    m_ptrPacketMapOut->SetValue(stCancelParam.iOrderBsn, "66");
    m_ptrPacketMapOut->SetValue(stCancelParam.szCliOrderNo, strlen(stCancelParam.szCliOrderNo), "9102");
    m_ptrPacketMapOut->SetValue(stCancelParam.iOrderDate, "8834"); 
    m_ptrPacketMapOut->SetValue(stCancelParam.iOrderNo, "9106");
    m_ptrPacketMapOut->SetValue(stCancelParam.siAttrCode, "9101");
    m_ptrPacketMapOut->SetValue(stCancelParam.szCliRemark, "8914");
    m_ptrPacketMapOut->SetValue('@', "76");
    m_ptrPacketMapOut->SetValue(stCancelParam.iCancleOrderNo, "77");
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
    DumpMsgData(clMakeMsgData);
  }

  return iRetCode;
}

int CSvcFuncCosAdpSWAns::Make10388105(const ST_ORDER_PARAM &stOrderParam, CMsgData &clMakeMsgData)
{
  int iRetCode = MA_OK;
  char szFuncId[8+1] = {"10388105"};
  char szPktVer[2+1] = {"01"};
  char szMsgId[32 + 1] = {0};
  char szPubKey2[32 + 1] = {0};
  _ma_try
  {
    m_ptrPacketMapOut->BeginWrite();
    SetPkgHead(m_ptrPacketMapOut, MAP_PKT_TYPE_BIZ, MAP_MSG_TYPE_REQ, 'T', "10388105");
    SetRegular(m_ptrPacketMapOut, stOrderParam.szCustCode, "", "10388105", stOrderParam.szStationAddr, stOrderParam.siIntOrg, stOrderParam.szChannel[0]);
    sprintf(szPubKey2, "0|%s|%c", stOrderParam.szCuacctCode, stOrderParam.szCuacctType[0]);

    m_ptrPacketMapOut->SetHdrColValue(stOrderParam.szUserSession, strlen(stOrderParam.szUserSession), MAP_FID_USER_SESSION);
    m_ptrPacketMapOut->SetHdrColValue(stOrderParam.szChannel, strlen(stOrderParam.szChannel), MAP_FID_BIZ_CHANNEL); 
    m_ptrPacketMapOut->SetHdrColValue(m_uiCurrNodeId, MAP_FID_SRC_NODE);
    m_ptrPacketMapOut->SetHdrColValue(szPubKey2, sizeof(szPubKey2), ma::MAP_FID_MSG_ID);
    m_ptrPacketMapOut->SetHdrColValue(szPubKey2, strlen(szPubKey2), ma::MAP_FID_PUB_KEY2);

    m_ptrPacketMapOut->SetValue(1, "9301");
    m_ptrPacketMapOut->SetValue(stOrderParam.szTrdCodeCls, strlen(stOrderParam.szTrdCodeCls),"8970");
    m_ptrPacketMapOut->SetValue(stOrderParam.szCustCode, strlen(stOrderParam.szCustCode),"8902");
    m_ptrPacketMapOut->SetValue(stOrderParam.szCuacctCode, strlen(stOrderParam.szCuacctCode),"8920");
    m_ptrPacketMapOut->SetValue(stOrderParam.szCuacctType,strlen(stOrderParam.szCuacctType),"8826");
    m_ptrPacketMapOut->SetValue(stOrderParam.szTrdacct, strlen(stOrderParam.szTrdacct),"448");
    m_ptrPacketMapOut->SetValue(stOrderParam.szExchange, strlen(stOrderParam.szExchange), "207");
    m_ptrPacketMapOut->SetValue(stOrderParam.szStkbd, strlen(stOrderParam.szStkbd), "625");
    m_ptrPacketMapOut->SetValue(stOrderParam.szTrdCode, strlen(stOrderParam.szTrdCode), "48");
    m_ptrPacketMapOut->SetValue(stOrderParam.siStkBiz, "8842");
    m_ptrPacketMapOut->SetValue(stOrderParam.siStkBizAction, "40");
    m_ptrPacketMapOut->SetValue(stOrderParam.llOrderQty, "38");
    xsdk::CPrice4 clOrderPrice;clOrderPrice.InitFromLonglong(stOrderParam.llOrderPrice);
    m_ptrPacketMapOut->SetValue(clOrderPrice.GetNumeric(), "44");

    m_ptrPacketMapOut->SetValue(1, "9080");
    m_ptrPacketMapOut->SetValue(stOrderParam.siIntOrg, "8911");
    m_ptrPacketMapOut->SetValue(stOrderParam.szOptNum, strlen(stOrderParam.szOptNum), "9082");
    m_ptrPacketMapOut->SetValue(stOrderParam.iOrderBsn, "66");
    m_ptrPacketMapOut->SetValue(stOrderParam.szOrderIdEx, strlen(stOrderParam.szOrderIdEx), "9093");
    m_ptrPacketMapOut->SetValue(stOrderParam.szUndlCode, strlen(stOrderParam.szUndlCode), "8972");
    m_ptrPacketMapOut->SetValue(stOrderParam.iConExpDate, "8976");
    m_ptrPacketMapOut->SetValue(stOrderParam.szExercisePrice, strlen(stOrderParam.szExercisePrice), "8973");
    m_ptrPacketMapOut->SetValue(stOrderParam.llConUnit, "8974");
    m_ptrPacketMapOut->SetValue(stOrderParam.szCliOrderNo, strlen(stOrderParam.szCliOrderNo), "9102");
    m_ptrPacketMapOut->SetValue(stOrderParam.szCliRemark, strlen(stOrderParam.szCliRemark), "8914");
    m_ptrPacketMapOut->SetValue(stOrderParam.szOrderDate, strlen(stOrderParam.szOrderDate), "8834");
    m_ptrPacketMapOut->SetValue(0, "8717");
    m_ptrPacketMapOut->SetValue(0, "8723");
    m_ptrPacketMapOut->SetValue(stOrderParam.szContingentCondition, strlen(stOrderParam.szContingentCondition), "8713");
    m_ptrPacketMapOut->SetValue(stOrderParam.szForceCloseReason, strlen(stOrderParam.szForceCloseReason), "8715");
    m_ptrPacketMapOut->SetValue(stOrderParam.siIsSwapOrder, "8720");
    m_ptrPacketMapOut->SetValue(stOrderParam.szCombOffsetFlag, strlen(stOrderParam.szCombOffsetFlag), "8741");
    m_ptrPacketMapOut->SetValue(stOrderParam.llStopPrice, "8975");
    m_ptrPacketMapOut->SetValue(stOrderParam.iOrderNo, "9106");
    m_ptrPacketMapOut->SetValue(0, "9101");

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
    DumpMsgData(clMakeMsgData);
  }

  return iRetCode;
}

void CSvcFuncCosAdpSWAns::DumpMsgData(CMsgData &oMsgData)
{
  if(oMsgData.Size())
  {
    CMsgData tmpMsgData;
    tmpMsgData.InitSize(oMsgData.Size()+1);
    memcpy(tmpMsgData.Data(),oMsgData.Data(),oMsgData.Size());
    ((char*)tmpMsgData.Data())[oMsgData.Size()] = 0x00;
    ma::ThrowDebug(NULL, "DumpMsgData msg:{@1},size:{@2}", &(_P((char *)tmpMsgData.Data()) + _P(oMsgData.Size())));  
  }  
}

int CSvcFuncCosAdpSWAns::DoWork(void *p_pvdParam)
{
  int	iRetCode = MA_OK;
  int iValueLen = 0;
  char szCuacctCode[16 + 1] = {0};
  char szMsgId[32 + 1] = {0};
  char szPktVer[2 + 1] = "01";
  char szPubKey2[32 + 1] = {0};
  int  iTrdDate = 0;
  char szOrderDate[32] = {0};
  int  iOrderNo = 0;
  char szSubsysConnstr[128 + 1]={0}; 
  char szCurrTimeStamp[24] = {0};
  long long llCurrentTime = 0LL;
  SYSTEMTIME stCurrentTime = {0};
  SYSTEMTIME stTimestamp = {0};
  int iTableNum = 0;
  int iMsgCode = 0;
  char szMsgDataIn[256 + 1] =  {0};
  char szMsgText[256 + 1] = {0};
  CMsgData clMsgDataIn;
  CMsgData clMsgDataOut;
  char szFuncId[16 + 1] = {0};
  Json::Reader Reader;
  Json::Value  JsonValue;

  _ma_try
  {
    xsdk::GetCurrentTimestamp(stCurrentTime);
    xsdk::DatetimeToInt64(llCurrentTime, stCurrentTime);

    if (0 != CSvcFuncCosAdpSWReq::m_mapCosOrderParam.size() || 0 != CSvcFuncCosAdpSWReq::m_mapCosCancelParam.size())
    {  
      CheckTimeOut(llCurrentTime);
    }
    if ((iRetCode = m_ptrServiceEnv->GetQueueData(clMsgDataIn, m_uiInQueId, m_uiSrcFuncId)) != MA_OK)
    {
      return	MA_NO_DATA;
    }
    if (0 == CSvcFuncCosAdpSWReq::m_mapCosOrderParam.size() && 0 == CSvcFuncCosAdpSWReq::m_mapCosCancelParam.size())
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
      iOrderNo = atoi(strOrderNo.c_str());
    }
    if(JsonValue["8819"].isString())
    {
      strUMsgText = JsonValue["8819"].asString();
    }

    // ��ر���Ϊutf-8
    UTF8ToGB2312(szMsgText, sizeof(szMsgText), strUMsgText.c_str(), strUMsgText.length());

    iRetCode = GetTrdDate(iTrdDate);
    if (0 == iTrdDate)
    {
      ThrowError(NULL, "CSvcFuncCosAdpSWReq::{@1} get TrdDate failed",
        &(_P(__FUNCTION__) + _P(iRetCode) ));
      _ma_leave;
    }

    std::string ssTrdOrderNo = CSvcFuncCosAdpSWReq::GetTrdOrderNo(iOrderNo, iTrdDate);
    std::map<std::string, ST_ORDER_PARAM>::iterator itrOrder;

    CSvcFuncCosAdpSWReq::m_clRWLock.WriteLock();
    itrOrder = CSvcFuncCosAdpSWReq::m_mapCosOrderParam.find(ssTrdOrderNo);

    // �µ�ί��
    if (itrOrder != CSvcFuncCosAdpSWReq::m_mapCosOrderParam.end())
    {
      if (iMsgCode  == MA_OK)
      {
        Make10388105(itrOrder->second, clMsgDataOut);
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
        Make10388902(itrOrder->second, false, szMsgText, clMsgDataOut);
        if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
        {
          ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
            &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
          CSvcFuncCosAdpSWReq::m_mapCosOrderParam.erase(itrOrder);
          CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
          _ma_leave;
        }
      }

      CSvcFuncCosAdpSWReq::m_mapCosOrderParam.erase(itrOrder);
      CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
    }
    // ����ί��
    else 
    {
      std::map<std::string, ST_CANCEL_PARAM>::iterator itrCancel;
      itrCancel = CSvcFuncCosAdpSWReq::m_mapCosCancelParam.find(ssTrdOrderNo);
      if (itrCancel != CSvcFuncCosAdpSWReq::m_mapCosCancelParam.end())
      {
        // ��10388102 ����
        if (iMsgCode  == MA_OK)
        {
          Make10388102(itrCancel->second, clMsgDataOut);
          m_ptrPacketMapOut->Make(clMsgDataOut);
          if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
          {
            ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}],text:{@4}", 
              &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode) + _P(m_ptrServiceEnv->GetLastErrorText())));
            CSvcFuncCosAdpSWReq::m_mapCosCancelParam.erase(itrCancel);
            CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
            _ma_leave;
          }
        }
        else
        {
          MakeCancel10388902(itrCancel->second, false, szMsgText, clMsgDataOut);
          if ((iRetCode = m_ptrServiceEnv->PutQueueData(clMsgDataOut, m_uiOutQueId, m_uiSrcFuncId)) != MA_OK)
          {
            ma::ThrowError(NULL, "CSvcFuncCosAdpSWAns::{@1} Put Data into m_uiOutQueId:{@2} Failed return[{@3}]", 
              &(_P(__FUNCTION__) + _P(m_uiOutQueId) + _P(iRetCode)));
            CSvcFuncCosAdpSWReq::m_mapCosCancelParam.erase(itrCancel);
            CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
            _ma_leave;
          }
        }
        CSvcFuncCosAdpSWReq::m_mapCosCancelParam.erase(itrCancel);
        CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
      }
      else
      {
        CSvcFuncCosAdpSWReq::m_clRWLock.WriteUnlock();
        // ��ڵ������,���ܻ��յ������ڱ��ڵ�İ�, ֱ�Ӷ���
        iRetCode = MA_OK;
        _ma_leave;
      }     
    }
  }
  _ma_catch_finally
  {
  }

  return iRetCode;
}