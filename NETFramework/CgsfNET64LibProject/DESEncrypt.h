#pragma once

using namespace System;

namespace CgsfNET64Lib { 
	
	class DESEncrypt
	{
	public:
		// 암호화
		static array<Byte>^ Encrypt(array<Byte>^ sourceData, array<Byte>^ key)
		{
			auto des = gcnew System::Security::Cryptography::DESCryptoServiceProvider();
			des->Key = key;
			des->IV = key;

			auto transform = des->CreateEncryptor();
			auto encrypted = transform->TransformFinalBlock(sourceData, 0, sourceData->Length);

			transform->Dispose();

			return encrypted;
		}

		// 복호화
		static array<Byte>^ Decrypt(array<Byte>^ sourceData, array<Byte>^ key)
		{
			auto des = gcnew System::Security::Cryptography::DESCryptoServiceProvider();
			des->Key = key;
			des->IV = key;

			auto destransform = des->CreateDecryptor();
			auto decrypted = destransform->TransformFinalBlock(sourceData, 0, sourceData->Length);

			destransform->Dispose();
			return decrypted;
		}
	};
}