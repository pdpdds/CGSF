#pragma once

using namespace System;

namespace CgsfNET64Lib {
	class _3DESEncrypt
	{
	public:
		static array<Byte>^ Encrypt(array<Byte>^ sourceData, array<Byte>^ key)
		{
			array<Byte>^ encrypted;

			// Step 1. MD5 해쉬를 사용해서 암호화하고,	   
			// MD5 해쉬 생성기를 사용해서 결과는 128 비트 바이트 배열인데,          
			// 3DES 인코딩을 위한 올바른 길이가 됨.

			auto HashProvider = gcnew System::Security::Cryptography::MD5CryptoServiceProvider();
			auto TDESKey = HashProvider->ComputeHash(key);

			// Step 2. TripleDESCryptoServiceProvider object 생성            
			auto TDESAlgorithm = gcnew System::Security::Cryptography::TripleDESCryptoServiceProvider();

			// Step 3. Encoder 설정
			TDESAlgorithm->Key = TDESKey;
			TDESAlgorithm->Mode = System::Security::Cryptography::CipherMode::ECB;
			TDESAlgorithm->Padding = System::Security::Cryptography::PaddingMode::PKCS7;

			
			// Step 4. 실제로 문자열을 암호화      
			try
			{
				auto Encryptor = TDESAlgorithm->CreateEncryptor();
				encrypted = Encryptor->TransformFinalBlock(sourceData, 0, sourceData->Length);
			}
			finally
			{
				// 중요한 3DES, Hashprovider의 속성을 해제       
				TDESAlgorithm->Clear();
				HashProvider->Clear();
			}

			return encrypted;
		}


		static array<Byte>^ Decrypt(array<Byte>^ sourceData, array<Byte>^ key)
		{
			array<Byte>^ decrypted;

			auto HashProvider = gcnew System::Security::Cryptography::MD5CryptoServiceProvider();
			auto TDESKey = HashProvider->ComputeHash(key);

			auto TDESAlgorithm = gcnew System::Security::Cryptography::TripleDESCryptoServiceProvider();
			TDESAlgorithm->Key = TDESKey;
			TDESAlgorithm->Mode = System::Security::Cryptography::CipherMode::ECB;
			TDESAlgorithm->Padding = System::Security::Cryptography::PaddingMode::PKCS7;

			
			try
			{
				auto Decryptor = TDESAlgorithm->CreateDecryptor();
				decrypted = Decryptor->TransformFinalBlock(sourceData, 0, sourceData->Length);
			}
			finally
			{
				// 중요한 3DES, Hashprovider의 속성을 해제
				TDESAlgorithm->Clear();
				HashProvider->Clear();
			}

			return decrypted;
		}

	};
}