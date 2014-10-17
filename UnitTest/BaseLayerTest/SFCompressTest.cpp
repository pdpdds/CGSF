#include "stdafx.h"
#include "CppUnitTest.h"

#include "SFCompressLzf.h"
#include "SFCompressZLib.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BaseLayerTest
{
	TEST_CLASS(SFCompressTest)
	{
	public:
		TEST_METHOD(TestLzfCompressUnCompress)
		{
			// [준비]
			char szSource[] = "dsdsd1223098978나나나";
			int sourceLen = (int)(strlen(szSource));

			char szDest1[256] = { 0, };
			int destLen1 = 256;

			SFCompressLzf Lzf;

			// [실행 - 압축]
			auto compressRelust = Lzf.Compress((BYTE*)&szDest1, destLen1, (BYTE*)&szSource, sourceLen);
			// [검증]
			Assert::AreEqual(true, compressRelust);

			
			char szDest2[256] = { 0, };
			int destLen2 = 256;

			// [실행 - 압축 해제]
			auto unCompressRelust = Lzf.Uncompress((BYTE*)&szDest2, destLen2, (BYTE*)&szDest1, destLen1);
			// [검증]
			Assert::AreEqual(true, unCompressRelust);
			
			// [검증 - 압축 해제한 데이터를 원 데이터와 비교]
			Assert::AreEqual(szSource, szDest2);
		}


		TEST_METHOD(TestZipCompressUnCompress)
		{
			// [준비]
			char szSource[] = "dsdsd1223098978나나나";
			int sourceLen = (int)(strlen(szSource));

			char szDest1[256] = { 0, };
			int destLen1 = 256;

			SFCompressZLib zlib;

			// [실행 - 압축]
			auto compressRelust = zlib.Compress((BYTE*)&szDest1, destLen1, (BYTE*)&szSource, sourceLen);
			// [검증]
			Assert::AreEqual(true, compressRelust);


			char szDest2[256] = { 0, };
			int destLen2 = 256;

			// [실행 - 압축 해제]
			auto unCompressRelust = zlib.Uncompress((BYTE*)&szDest2, destLen2, (BYTE*)&szDest1, destLen1);
			// [검증]
			Assert::AreEqual(true, unCompressRelust);

			// [검증 - 압축 해제한 데이터를 원 데이터와 비교]
			Assert::AreEqual(szSource, szDest2);
		}
	};
}