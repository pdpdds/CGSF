#pragma once
#include <boost/serialization/singleton.hpp>

class SFIOController : public boost::serialization::singleton<SFIOController>
{
	friend class boost::serialization::singleton<SFIOController>;

public:
	SFIOController();

	virtual ~SFIOController();

	static SFIOController& Instance() {

		return boost::serialization::singleton<SFIOController>::get_mutable_instance();
	}

	void IncreaseIO()
	{
		InterlockedIncrement((LONG*)&m_curIO);
	}

	void DecreaseIO()
	{
		InterlockedDecrement((LONG*)&m_curIO);
	}

private:
	int m_maxIO;
	int m_curIO;
};
