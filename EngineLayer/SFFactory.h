#pragma once
#include <map>
#include <string>

namespace CGSF
{
	typedef std::string ProductName;

	template <typename T>
	class SFCreatorBase
	{
	public:
		virtual ~SFCreatorBase(void){}
		virtual T* Create() const = 0;
	};

	template <typename T, typename Product>
	class SFCreator : public SFCreatorBase<T>
	{
	public:

		virtual T* Create() const {return new Product;}
	};

	template <typename T>
	class SFFactory
	{
		typedef std::map<ProductName, SFCreatorBase<T>*> CreatorMap;

	public:
		T* Create(ProductName Name);
		BOOL Register(ProductName Name, SFCreatorBase<T>* pCreator);

	protected:

	private:
		CreatorMap m_CreatorMap;
	};

	template <typename T>
	BOOL SFFactory<T>::Register(ProductName Name, SFCreatorBase<T>* pCreator)
	{
		CreatorMap::iterator iter = m_CreatorMap.find(Name);

		if(iter != m_CreatorMap.end())
		{
			delete pCreator;
			return FALSE;
		}

		m_CreatorMap[Name] = pCreator;

		return TRUE;
	}

	template <typename T>
	T* SFFactory<T>::Create(ProductName Name)
	{
		CreatorMap::iterator iter = m_CreatorMap.find(Name);

		if(iter == m_CreatorMap.end())
		{
			return NULL;
		}

		SFCreatorBase<T>* pCreator = (*iter).second;

		return pCreator->Create();
	}
};