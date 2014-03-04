#pragma once


#include "DXUTgui.h"


class CScrollBarCtrl : public CDXUTScrollBar
{

public:
	CScrollBarCtrl( CDXUTDialog *pDialog = NULL) : CDXUTScrollBar(pDialog){}
		
	int GetEndSiz(){return m_nEnd-GetPageSize();}
};