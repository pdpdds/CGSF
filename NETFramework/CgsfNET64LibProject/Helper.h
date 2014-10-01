#pragma once 

#include "stdafx.h"
#include "SFNETPacket.h"

#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Concurrent;

namespace CgsfNET64Lib {

	/// <summary>
	/// 헬퍼 기능을 모아 놓은 클래스
	/// </summary>
	ref class Helper
	{
	public:
		/// <summary>
		/// 네트워크 라이브러리용 로그를 사용하여 메시지를 파일에 즉시 남긴다.
		/// </summary>
		/// <param name="logLevel">로그레벨</param>
		/// <param name="logMessage">로그 메시지</param>
		static void WriteLog(int logLevel, String^ logMessage)
		{
			auto log = msclr::interop::marshal_as<std::string>(logMessage);

			switch (logLevel)
			{
			case google::GLOG_INFO:
				LOG(INFO) << log.data();
				google::FlushLogFiles(google::GLOG_INFO);
				break;
			case google::GLOG_ERROR:
				LOG(ERROR) << log.data();
				google::FlushLogFiles(google::GLOG_ERROR);
				break;
			}
			
		}
	};
}