#pragma once

using namespace System;
using System::StringComparer;

namespace CgsfNET64Lib {
	public class AESEncrypt
	{
	public:
		// 128bit(16byte)의 IV（초기화 벡터）와 Key(암호키）
		//static private string AesIV = @"!QAZ2WSX#EDC4RFV";
		//static private string AesKey = @"5TGB&YHN7UJM(IK<";

		/// <summary>
		/// 문자열을 AES로 암호화 한다
		/// </summary>
		static array<Byte>^ Encrypt(array<Byte>^ sourceData, array<Byte>^ aesKey, array<Byte>^ aesIV)
		{
			// AES 암호화 서비스 프로바이더. System.Core.Dll을 추가해야 한다.
			auto aes = gcnew System::Security::Cryptography::AesCryptoServiceProvider();
			aes->BlockSize = 128;
			aes->KeySize = 128;
			aes->IV = aesIV;
			aes->Key = aesKey;
			aes->Mode = System::Security::Cryptography::CipherMode::CBC;
			aes->Padding = System::Security::Cryptography::PaddingMode::PKCS7;
						
			auto encrypt = aes->CreateEncryptor();
			auto encrypted = encrypt->TransformFinalBlock(sourceData, 0, sourceData->Length);

			encrypt->Dispose();

			return encrypted;
		}

		/// <summary>
		/// 문자열을 AES로 복호화 한다
		/// </summary>
		static array<Byte>^ Decrypt(array<Byte>^ sourceData, array<Byte>^ aesKey, array<Byte>^ aesIV)
		{
			// AES 암호화 서비스 프로바이더
			auto aes = gcnew System::Security::Cryptography::AesCryptoServiceProvider();
			aes->BlockSize = 128;
			aes->KeySize = 128;
			aes->IV = aesIV;
			aes->Key = aesKey;
			aes->Mode = System::Security::Cryptography::CipherMode::CBC;
			aes->Padding = System::Security::Cryptography::PaddingMode::PKCS7;

			auto decrypt = aes->CreateDecryptor();
			auto decrypted = decrypt->TransformFinalBlock(sourceData, 0, sourceData->Length);
			return decrypted;
		}
	};
}