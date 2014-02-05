//-< TESTDB.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     10-Dec-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 12-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Sample of database application: supplier/contract/details database
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

USE_FASTDB_NAMESPACE

struct STK_ORDER
{
  int4  TRD_DATE;
  int4  ORDER_DATE;
  char const *  ORDER_TIME;
  int4  ORDER_SN;
  int4  ORDER_BSN;
  char const *  ORDER_ID;
  char const *  ORDER_TYPE;
  char const *  ORDER_STATUS;
  char const *  ORDER_VALID_FLAG;
  int4  ORDER_VALID_DATE;
  int2  SUBSYS_SN;
  int2  INT_ORG;
  int8  CUST_CODE;
  char const *  CUST_NAME;
  char const *  CUST_TYPE;
  char const *  CUST_CLS;
  int8  CUACCT_CODE;
  char const *  CUACCT_ATTR;
  char const *  CUACCT_CLS;
  char const *  CUACCT_LVL;
  char const *  CUACCT_GRP;
  char const *  CUACCT_DMF;
  char const *  STKEX;
  char const *  STKBD;
  char const *  STKPBU;
  char const *  FIRMID;
  char const *  TRDACCT;
  char const *  TRDACCT_EXID;
  char const *  TRDACCT_TYPE;
  char const *  TRDACCT_EXCLS;
  int2  STK_BIZ;
  int2  STK_BIZ_ACTION;
  char const *  STK_BIZ_EX;
  char const *  STK_TRD_EXTCLS;
  char const *  STK_CODE;
  char const *  STK_NAME;
  char const *  STK_CLS;
  char const *  STK_FLAG;
  char const *  CURRENCY;
  int8  BOND_INT;
  int8  ORDER_PRICE;
  int8  ORDER_QTY;
  int8  ORDER_AMT;
  int8  ORDER_FRZ_AMT;
  int8  ORDER_UFZ_AMT;
  int8  OFFER_QTY;
  int4  OFFER_STIME;
  int8  WITHDRAWN_QTY;
  int8  MATCHED_QTY;
  int8  MATCHED_AMT;
  int8  RLT_SETT_AMT;
  char const *  IS_WITHDRAW;
  char const *  IS_WITHDRAWN;
  int8  OP_USER;
  char const *  OP_ROLE;
  char const *  OP_NAME;
  int2  OP_ORG;
  char const *  OP_SITE;
  char const *  CHANNEL;
  char const *  RAW_ORDER_ID;
  int4  OFFER_REC_SN;
  char const *  OFFER_RET_MSG;
  char const *  ORDER_REMARK;
  char const *  OPPT_STKPBU;
  char const *  OPPT_TRDACCT;
  char const *  STKEX_ORG_ID;
  char const *  POST_STR;

  TYPE_DESCRIPTOR((
    KEY(TRD_DATE, HASHED),
    FIELD(ORDER_DATE),
    FIELD(ORDER_TIME),
    FIELD(ORDER_SN),
    FIELD(ORDER_BSN),
    KEY(ORDER_ID, INDEXED),
    FIELD(ORDER_TYPE),
    FIELD(ORDER_STATUS),
    FIELD(ORDER_VALID_FLAG),
    FIELD(ORDER_VALID_DATE),
    FIELD(SUBSYS_SN),
    KEY(INT_ORG, INDEXED),
    KEY(CUST_CODE, INDEXED),
    FIELD(CUST_NAME),
    FIELD(CUST_TYPE),
    FIELD(CUST_CLS),
    KEY(CUACCT_CODE, INDEXED),
    FIELD(CUACCT_ATTR),
    FIELD(CUACCT_CLS),
    FIELD(CUACCT_LVL),
    FIELD(CUACCT_GRP),
    FIELD(CUACCT_DMF),
    FIELD(STKEX),
    KEY(STKBD, INDEXED),
    KEY(STKPBU, INDEXED),
    FIELD(FIRMID),
    FIELD(TRDACCT),
    FIELD(TRDACCT_EXID),
    FIELD(TRDACCT_TYPE),
    FIELD(TRDACCT_EXCLS),
    FIELD(STK_BIZ),
    FIELD(STK_BIZ_ACTION),
    FIELD(STK_BIZ_EX),
    FIELD(STK_TRD_EXTCLS),
    FIELD(STK_CODE),
    FIELD(STK_NAME),
    FIELD(STK_CLS),
    FIELD(STK_FLAG),
    FIELD(CURRENCY),
    FIELD(BOND_INT),
    FIELD(ORDER_PRICE),
    FIELD(ORDER_QTY),
    FIELD(ORDER_AMT),
    FIELD(ORDER_FRZ_AMT),
    FIELD(ORDER_UFZ_AMT),
    FIELD(OFFER_QTY),
    FIELD(OFFER_STIME),
    FIELD(WITHDRAWN_QTY),
    FIELD(MATCHED_QTY),
    FIELD(MATCHED_AMT),
    FIELD(RLT_SETT_AMT),
    KEY(IS_WITHDRAW, INDEXED),
    FIELD(IS_WITHDRAWN),
    FIELD(OP_USER),
    FIELD(OP_ROLE),
    FIELD(OP_NAME),
    FIELD(OP_ORG),
    FIELD(OP_SITE),
    FIELD(CHANNEL),
    FIELD(RAW_ORDER_ID),
    FIELD(OFFER_REC_SN),
    FIELD(OFFER_RET_MSG),
    FIELD(ORDER_REMARK),
    FIELD(OPPT_STKPBU),
    FIELD(OPPT_TRDACCT),
    FIELD(STKEX_ORG_ID),
    KEY(POST_STR, INDEXED)
    ));
};

REGISTER_UNASSIGNED(STK_ORDER);

struct ST_STK_ORDER
    {
      int iTrdDate; short siInd_iTrdDate;                                           // 交易日期
      int iOrderDate; short siInd_iOrderDate;                                       // 委托日期
      char szOrderTime[32 + 1]; short siInd_szOrderTime;                            // 委托时间
      int iOrderSn; short siInd_iOrderSn;                                           // 委托序号
      int iOrderBsn; short siInd_iOrderBsn;                                         // 委托批号
      char szOrderId[10 + 1]; short siInd_szOrderId;                                // 合同序号
      char chOrderType; short siInd_chOrderType;                                    // 委托类型
      char chOrderStatus; short siInd_chOrderStatus;                                // 委托状态
      char chOrderValidFlag; short siInd_chOrderValidFlag;                          // 委托有效标志
      int iOrderValidDate; short siInd_iOrderValidDate;                             // 委托有效日期
      short siSubsysSn; short siInd_siSubsysSn;                                     // 子系统编码
      short siIntOrg; short siInd_siIntOrg;                                         // 内部机构
      long long llCustCode; short siInd_llCustCode;                                 // 客户代码
      char szCustName[32 + 1]; short siInd_szCustName;                              // 客户姓名
      char chCustType; short siInd_chCustType;                                      // 客户类型
      char chCustCls; short siInd_chCustCls;                                        // 客户类别
      long long llCuacctCode; short siInd_llCuacctCode;                             // 资产账户
      char chCuacctAttr; short siInd_chCuacctAttr;                                  // 资产账户属性
      char chCuacctCls; short siInd_chCuacctCls;                                    // 资产账户类别
      char chCuacctLvl; short siInd_chCuacctLvl;                                    // 资产账户级别
      char chCuacctGrp; short siInd_chCuacctGrp;                                    // 资产账户组别
      char chCuacctDmf; short siInd_chCuacctDmf;                                    // 资产账户存管
      char chStkex; short siInd_chStkex;                                            // 证券交易所
      char szStkbd[2 + 1]; short siInd_szStkbd;                                     // 交易板块
      char szStkpbu[8 + 1]; short siInd_szStkpbu;                                   // 交易单元
      char szFirmid[8 + 1]; short siInd_szFirmid;                                   // 代理商号
      char szTrdacct[10 + 1]; short siInd_szTrdacct;                                // 交易账户
      char szTrdacctExid[10 + 1]; short siInd_szTrdacctExid;                        // 报盘交易账户
      char chTrdacctType; short siInd_chTrdacctType;                                // 交易账户类型
      char chTrdacctExcls; short siInd_chTrdacctExcls;                              // 交易账户类别
      short siStkBiz; short siInd_siStkBiz;                                         // 证券业务
      short siStkBizAction; short siInd_siStkBizAction;                             // 证券业务行为
      char szStkBizEx[8 + 1]; short siInd_szStkBizEx;                               // 证券业务（外部）
      char chStkTrdExtcls; short siInd_chStkTrdExtcls;                              // 交易方式分类
      char szStkCode[8 + 1]; short siInd_szStkCode;                                 // 证券代码
      char szStkName[16 + 1]; short siInd_szStkName;                                // 证券名称
      char chStkCls; short siInd_chStkCls;                                          // 证券类别
      char chStkFlag; short siInd_chStkFlag;                                        // 证券标志
      char chCurrency; short siInd_chCurrency;                                      // 货币代码
      long long llBondInt; short siInd_llBondInt;                                   // 债券利息
      long long llOrderPrice; short siInd_llOrderPrice;                             // 委托价格
      long long llOrderQty; short siInd_llOrderQty;                                 // 委托数量
      long long llOrderAmt; short siInd_llOrderAmt;                                 // 委托金额
      long long llOrderFrzAmt; short siInd_llOrderFrzAmt;                           // 委托冻结金额
      long long llOrderUfzAmt; short siInd_llOrderUfzAmt;                           // 委托解冻金额
      long long llOfferQty; short siInd_llOfferQty;                                 // 申报数量
      int iOfferStime; short siInd_iOfferStime;                                     // 申报时间
      long long llWithdrawnQty; short siInd_llWithdrawnQty;                         // 已撤单数量
      long long llMatchedQty; short siInd_llMatchedQty;                             // 已成交数量
      long long llMatchedAmt; short siInd_llMatchedAmt;                             // 已成交金额
      long long llRltSettAmt; short siInd_llRltSettAmt;                             // 实时清算金额
      char chIsWithdraw; short siInd_chIsWithdraw;                                  // 撤单标志
      char chIsWithdrawn; short siInd_chIsWithdrawn;                                // 已撤单标志
      long long llOpUser; short siInd_llOpUser;                                     // 操作用户
      char chOpRole; short siInd_chOpRole;                                          // 操作用户角色
      char szOpName[32 + 1]; short siInd_szOpName;                                  // 操作用户姓名
      short siOpOrg; short siInd_siOpOrg;                                           // 操作机构
      char szOpSite[32 + 1]; short siInd_szOpSite;                                  // 操作站点
      char chChannel; short siInd_chChannel;                                        // 操作渠道
      char szRawOrderId[10 + 1]; short siInd_szRawOrderId;                          // 原合同序号
      int iOfferRecSn; short siInd_iOfferRecSn;                                     // 申报序号
      char szOfferRetMsg[64 + 1]; short siInd_szOfferRetMsg;                        // 申报返回信息
      char szOrderRemark[256 + 1]; short siInd_szOrderRemark;                       // 委托备注
      char szOpptStkpbu[8 + 1]; short siInd_szOpptStkpbu;                           // 交易单元(对手)
      char szOpptTrdacct[10 + 1]; short siInd_szOpptTrdacct;                        // 交易账户(对手)
      char szStkexOrgId[5 + 1]; short siInd_szStkexOrgId;                           // 营业部识别码
      char szPostStr[32 + 1];   short siInd_szPostStr;                              // 定位串(8位[交易日期]+2位[板块]+10位[合同序号]+6位[交易单元]+1位[撤单标志])
    };

using namespace std;

int main() 
{
    ST_STK_ORDER stStkOrder = {0};
    STK_ORDER    stStkOrderFastdb = {0};

    {
      stStkOrder.iTrdDate = 20130410;
      stStkOrder.iOrderDate = 20130410;
      sprintf(stStkOrder.szOrderTime, "2013-04-10 10:10:10.999");
      stStkOrder.iOrderSn = 1;
      stStkOrder.iOrderBsn = 1;
      sprintf(stStkOrder.szOrderId, "%010d", 1);
      stStkOrder.chOrderType = '0';
      stStkOrder.chOrderStatus = '0';
      stStkOrder.chOrderValidFlag = '0';
      stStkOrder.iOrderValidDate = 20130410;
      stStkOrder.siSubsysSn = 0;
      stStkOrder.siIntOrg = 1001;
      stStkOrder.llCustCode = 1001000001LL;
      sprintf(stStkOrder.szCustName, "%lld", 1001000001LL);
      stStkOrder.llCustCode = 1001000001LL;
      stStkOrder.chCustType = '0';
      stStkOrder.chCustCls = '0';
      stStkOrder.llCuacctCode = 100100000001LL;
      stStkOrder.chCuacctAttr = '0';
      stStkOrder.chCuacctCls = '0';
      stStkOrder.chCuacctLvl = '0';
      stStkOrder.chCuacctGrp = '0';
      stStkOrder.chCuacctDmf = '0';
      stStkOrder.chStkex = '0';
      sprintf(stStkOrder.szStkbd, "%s", "00");
      sprintf(stStkOrder.szStkpbu, "%s", "239000");
      sprintf(stStkOrder.szTrdacct, "%s", "0010010001");
      sprintf(stStkOrder.szTrdacctExid, "%s", "0010010001");
      stStkOrder.chTrdacctType = '0';
      stStkOrder.chTrdacctExcls = '0';
      stStkOrder.siStkBiz = 100;
      stStkOrder.siStkBizAction = 100;
      sprintf(stStkOrder.szStkBizEx, "%s", "0010010001");
      stStkOrder.chStkTrdExtcls = '0';
      sprintf(stStkOrder.szStkCode, "%s", "000001");
      sprintf(stStkOrder.szStkName, "%s", "平安银行");
      stStkOrder.chStkCls = 'A';
      stStkOrder.chStkFlag = '0';
      stStkOrder.chCurrency = '0';
      stStkOrder.llBondInt = 0LL;
      stStkOrder.llOrderPrice = 1000LL;
      stStkOrder.llOrderQty = 100LL;
      stStkOrder.llOrderAmt = 100000LL;
      stStkOrder.llOrderFrzAmt = 105000LL;
      stStkOrder.llOrderUfzAmt = 0LL;
      stStkOrder.llOfferQty = 100LL;
      stStkOrder.iOfferStime = 101010LL;
      stStkOrder.llWithdrawnQty = 0LL;
      stStkOrder.llMatchedQty = 0LL;
      stStkOrder.llMatchedAmt = 0LL;
      stStkOrder.llRltSettAmt = 0LL;
      stStkOrder.chIsWithdraw = 'F';
      stStkOrder.chIsWithdrawn = '0';
      stStkOrder.llOpUser = 1001000001LL;
      stStkOrder.chOpRole = '0';
      sprintf(stStkOrder.szOpName, "%lld", 1001000001LL);
      stStkOrder.siOpOrg = 1001;
      sprintf(stStkOrder.szOpSite, "%s", "10.10.10.1");
      stStkOrder.chChannel = '0';
      sprintf(stStkOrder.szRawOrderId, "%s", " ");
      stStkOrder.iOfferRecSn = 1;
      sprintf(stStkOrder.szOfferRetMsg, "%s", " ");
      sprintf(stStkOrder.szOrderRemark,  "%s", " ");
      sprintf(stStkOrder.szOpptStkpbu,  "%s", " ");
      sprintf(stStkOrder.szOpptTrdacct,  "%s", " ");
      sprintf(stStkOrder.szStkexOrgId, "%s", " ");

      sprintf(stStkOrder.szPostStr,  "%lld.%010d", 1234567890, stStkOrder.iOrderSn);

      stStkOrderFastdb.TRD_DATE  =           stStkOrder.iTrdDate;
      stStkOrderFastdb.ORDER_DATE  =         stStkOrder.iOrderDate;
      stStkOrderFastdb.ORDER_TIME  =         stStkOrder.szOrderTime;
      stStkOrderFastdb.ORDER_SN  =           stStkOrder.iOrderSn;
      stStkOrderFastdb.ORDER_BSN  =          stStkOrder.iOrderBsn;
      stStkOrderFastdb.ORDER_ID  =           stStkOrder.szOrderId;
      stStkOrderFastdb.ORDER_TYPE  =         "0";
      stStkOrderFastdb.ORDER_STATUS  =       "0";
      stStkOrderFastdb.ORDER_VALID_FLAG  =   "0";
      stStkOrderFastdb.ORDER_VALID_DATE  =   stStkOrder.iOrderValidDate;
      stStkOrderFastdb.SUBSYS_SN  =          stStkOrder.siSubsysSn;
      stStkOrderFastdb.INT_ORG  =            stStkOrder.siIntOrg;
      stStkOrderFastdb.CUST_CODE  =          stStkOrder.llCustCode;
      stStkOrderFastdb.CUST_NAME  =          stStkOrder.szCustName;
      stStkOrderFastdb.CUST_TYPE  =          "0";
      stStkOrderFastdb.CUST_CLS  =           "0";
      stStkOrderFastdb.CUACCT_CODE  =        stStkOrder.llCuacctCode;
      stStkOrderFastdb.CUACCT_ATTR  =        "0";
      stStkOrderFastdb.CUACCT_CLS  =         "0";
      stStkOrderFastdb.CUACCT_LVL  =         "0";
      stStkOrderFastdb.CUACCT_GRP  =         "0";
      stStkOrderFastdb.CUACCT_DMF  =         "0";
      stStkOrderFastdb.STKEX  =              "0";
      stStkOrderFastdb.STKBD  =              stStkOrder.szStkbd;
      stStkOrderFastdb.STKPBU  =             stStkOrder.szStkpbu;
      stStkOrderFastdb.FIRMID  =             stStkOrder.szFirmid;
      stStkOrderFastdb.TRDACCT  =            stStkOrder.szTrdacct;
      stStkOrderFastdb.TRDACCT_EXID  =       stStkOrder.szTrdacctExid;
      stStkOrderFastdb.TRDACCT_TYPE  =       "0";
      stStkOrderFastdb.TRDACCT_EXCLS  =      "0";
      stStkOrderFastdb.STK_BIZ  =            stStkOrder.siStkBiz;
      stStkOrderFastdb.STK_BIZ_ACTION  =     stStkOrder.siStkBizAction;
      stStkOrderFastdb.STK_BIZ_EX  =         stStkOrder.szStkBizEx;
      stStkOrderFastdb.STK_TRD_EXTCLS  =     "0";
      stStkOrderFastdb.STK_CODE  =           stStkOrder.szStkCode;
      stStkOrderFastdb.STK_NAME  =           stStkOrder.szStkName;
      stStkOrderFastdb.STK_CLS  =            "A";
      stStkOrderFastdb.STK_FLAG  =           "0";
      stStkOrderFastdb.CURRENCY  =           "0";
      stStkOrderFastdb.BOND_INT  =           stStkOrder.llBondInt;
      stStkOrderFastdb.ORDER_PRICE  =        stStkOrder.llOrderPrice;
      stStkOrderFastdb.ORDER_QTY  =          stStkOrder.llOrderQty;
      stStkOrderFastdb.ORDER_AMT  =          stStkOrder.llOrderAmt;
      stStkOrderFastdb.ORDER_FRZ_AMT  =      stStkOrder.llOrderFrzAmt;
      stStkOrderFastdb.ORDER_UFZ_AMT  =      stStkOrder.llOrderUfzAmt;
      stStkOrderFastdb.OFFER_QTY  =          stStkOrder.llOfferQty;
      stStkOrderFastdb.OFFER_STIME  =        stStkOrder.iOfferStime;
      stStkOrderFastdb.WITHDRAWN_QTY  =      stStkOrder.llWithdrawnQty;
      stStkOrderFastdb.MATCHED_QTY  =        stStkOrder.llMatchedQty;
      stStkOrderFastdb.MATCHED_AMT  =        stStkOrder.llMatchedAmt;
      stStkOrderFastdb.RLT_SETT_AMT  =       stStkOrder.llRltSettAmt;
      stStkOrderFastdb.IS_WITHDRAW  =        "F";
      stStkOrderFastdb.IS_WITHDRAWN  =       "0";
      stStkOrderFastdb.OP_USER  =            stStkOrder.llOpUser;
      stStkOrderFastdb.OP_ROLE  =            "0";
      stStkOrderFastdb.OP_NAME  =            stStkOrder.szOpName;
      stStkOrderFastdb.OP_ORG  =             stStkOrder.siOpOrg;
      stStkOrderFastdb.OP_SITE  =            stStkOrder.szOpSite;
      stStkOrderFastdb.CHANNEL  =            "0";
      stStkOrderFastdb.RAW_ORDER_ID  =       stStkOrder.szRawOrderId;
      stStkOrderFastdb.OFFER_REC_SN  =       stStkOrder.iOfferRecSn;
      stStkOrderFastdb.OFFER_RET_MSG  =      stStkOrder.szOfferRetMsg;
      stStkOrderFastdb.ORDER_REMARK  =       stStkOrder.szOrderRemark;
      stStkOrderFastdb.OPPT_STKPBU  =        stStkOrder.szOpptStkpbu;
      stStkOrderFastdb.OPPT_TRDACCT  =       stStkOrder.szOpptTrdacct;
      stStkOrderFastdb.STKEX_ORG_ID  =       stStkOrder.szStkexOrgId;
      stStkOrderFastdb.POST_STR      =       stStkOrder.szPostStr;
    }


    dbDatabase db;
    int max_num = 1000*500;

    struct dbDatabase::OpenParameters params;
    params.databaseName = _T("test001");
    params.initSize = 1024 * 1024 * 512;

    //If transactionCommitDelay == 1, the program will not run properly
    //when transactionCommitDelay == 0, everything is OK!
    params.transactionCommitDelay = 1;

    db.open(params);

    long long start, end;
    int i;

    cout<<"record size: "<<sizeof(STK_ORDER)<<endl;
    cout<<"record number: "<<max_num<<endl;

    start = GetTickCount();
    for(i = 0; i < max_num; i++)
    {
      db.insert(stStkOrderFastdb);
      db.commit();
    }
    end = GetTickCount();
    cout<<"total time:"<<end-start<<endl;
    cout<<"time used by one insert: "<< (end-start) * 1000.0 / max_num<<"us"<<endl;

    db.close();

    getchar();

    return 0;        
}














