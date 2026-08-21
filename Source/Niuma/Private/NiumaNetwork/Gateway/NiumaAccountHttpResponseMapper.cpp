#include "NiumaNetwork/Gateway/NiumaAccountHttpResponseMapper.h"

#include "NiumaNetwork/Dto/NiumaAccountRemoteDtos.h"
#include "NiumaNetwork/Serialization/NiumaAccountJsonConverter.h"

namespace
{
	constexpr int32 LoginSuccessStatusCode = 200;
	constexpr int32 MinimumErrorStatusCode = 400;
	constexpr int32 MaximumErrorStatusCode = 599;
}

FNiumaAccountLoginResult FNiumaAccountHttpResponseMapper::MapResponse(
	int32 HttpStatusCode,
	const FString& ResponseBody)
{
	if (HttpStatusCode <= 0)
	{
		return FNiumaAccountLoginResult::MakeTransportFailure(
			TEXT("登录响应缺少有效 HTTP 状态码"));
	}

	FNiumaAccountLoginResponseDto ParsedResponse;
	FString ParseError;

	if (!FNiumaAccountJsonConverter::TryParseLoginResponse(
		ResponseBody,
		ParsedResponse,
		&ParseError))
	{
		return FNiumaAccountLoginResult::MakeProtocolFailure(
			HttpStatusCode,
			MoveTemp(ParseError));
	}

	if (ParsedResponse.bSuccess)
	{
		if (HttpStatusCode != LoginSuccessStatusCode)
		{
			return FNiumaAccountLoginResult::MakeProtocolFailure(
				HttpStatusCode,TEXT("登录业务成功但 HTTP 状态码不是 200"));
		}

		return FNiumaAccountLoginResult::MakeSuccess(
			HttpStatusCode,
			MoveTemp(ParsedResponse.Data));
	}

	if (HttpStatusCode < MinimumErrorStatusCode ||
		HttpStatusCode > MaximumErrorStatusCode)
	{
		return FNiumaAccountLoginResult::MakeProtocolFailure(
			HttpStatusCode,TEXT("登录业务失败但 HTTP 状态码不是错误状态"));
	}

	return FNiumaAccountLoginResult::MakeBusinessFailure(
		HttpStatusCode,
		MoveTemp(ParsedResponse.Code),
		MoveTemp(ParsedResponse.Message));
}


