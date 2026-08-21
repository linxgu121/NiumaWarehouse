#pragma once

#include "CoreMinimal.h"


/*
* 远程请求最终结果分类。
* 这是网络层内部 C++ 类型，不直接暴露给蓝图。
*/
enum class ENiumaRemoteOutcome : uint8
{
	/** 后端确认业务成功 */
	Success,

	/** 后端返回结构合法的业务失败响应 */
	BusinessFailure,

	/** 未收到有效 HTTP 响应，例如断网、超时 */
	TransportFailure,

	/** 收到响应，但 JSON 不符合约定 */
	ProtocolFailure
};
