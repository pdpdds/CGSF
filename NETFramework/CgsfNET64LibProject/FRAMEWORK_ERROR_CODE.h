#pragma once

using namespace System;

namespace CgsfNET64Lib {

	public enum class ERROR_CODE_N : short
	{
		// 중요!!!
		// 정의된 값은 CommonLayer의 ErrorCode.h에 정의된 것과 같아야 한다.
		SUCCESS = 0,

		ENGINE_INIT_LOGIC_ENTRY_FAIL = 101,
		ENGINE_INIT_NULL_CONFIG_INFO = 102,

		ENGINE_INIT_CREAT_ENGINE_LOAD_DLL_FAIL = 103,
		ENGINE_INIT_CREAT_ENGINE_FUNC_NULL = 104,
		ENGINE_INIT_CREAT_ENGINE_INIT_FAIL = 105,

		ENGINE_INIT_CREAT_LOGIC_SYSTEM_FAIL = 106,
	};
}