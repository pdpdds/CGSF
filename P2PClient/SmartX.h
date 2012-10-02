////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2005.11.25
// 설  명 :
//

#ifndef __SMARTX_H__
#define __SMARTX_H__

//
template <class I>
class CSmartX
{
public:
	CSmartX(I* pObject);
	virtual ~CSmartX();

private:
	I* m_pObject;
};
//

template <class I>
CSmartX<I>::CSmartX(I* pObject):
	m_pObject(pObject)
{
	if (m_pObject)
	{
		m_pObject->AddRef();
	}
}

template <class I>
CSmartX<I>::~CSmartX()
{
	if (m_pObject)
	{
		m_pObject->Release();
	}
}

#endif
