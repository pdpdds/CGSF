#include "stdafx.h"
#include "SerializationTest.h"
/*#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_iarchive.hpp> 
#include <boost/archive/binary_oarchive.hpp>*/

/*class SampleBoostSerialization
{
	friend class boost::serialization::access;

public:
	SampleBoostSerialization(){}
	virtual ~SampleBoostSerialization(){}

	void SetHP(int HP){m_HP = HP;}
	void SetMP(int MP){m_MP = MP;}

protected:

private: 
	template<class Archive> void serialize( Archive& ar, unsigned int ver )
	{
		ar & m_HP;
		ar & m_MP;
	}

private:
	int m_HP;
	int m_MP;
};*/

SerializationTest::SerializationTest(void)
{
}


SerializationTest::~SerializationTest(void)
{
}

//////////////////////////////////////////////////////////////////////////
//부스트 직렬화 테스트
//////////////////////////////////////////////////////////////////////////
bool SerializationTest::Run()
{
/*
	//std::ostringstream archive_stream;
	std::stringstream ss;
    {
        SampleBoostSerialization SamplePlayer;
		SamplePlayer.SetHP(100);
		SamplePlayer.SetMP(90);
       
        ofstream file("PlayerInfo.dat");
		
        boost::archive::text_oarchive oa(ss);

        oa << SamplePlayer;
    }

    {
        SampleBoostSerialization SamplePlayer;

		ifstream file("PlayerInfo.dat");
        boost::archive::text_iarchive ia(ss);
        ia >> SamplePlayer;

		int j = 6;
    }
	*/
	return true;
}
