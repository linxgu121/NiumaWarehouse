#include "NiumaNetwork/Serialization/NiumaAccountJsonConverter.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace 
{
	bool Fail(FString* OutError,const FString& Error)
	{
		if (OutError != nullptr)
		{
			*OutError = Error;
		}
		return false;
	}

	bool IsValidPlayerUid(const FString& PlayerUid)
	{
		if (PlayerUid.Len() != 9 || PlayerUid[0] == TEXT('0'))
		{
			return false;
		}

		for (const TCHAR Character : PlayerUid)
		{
			if (!FChar::IsDigit(Character))
			{
				return false;
			}
		}

		return true;
	}
}

bool FNiumaAccountJsonConverter::TryParseLoginResponse(
	const FString& ResponseJson,
	FNiumaAccountLoginResponseDto& OutResponse,
	FString* OutError)
{
	TSharedPtr<FJsonObject> RootObject;

	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseJson);

	if (!FJsonSerializer::Deserialize(Reader,RootObject) || !RootObject.IsValid())
	{
		return Fail(OutError,TEXT("登录响应不是合法 JSON 对象"));
	}

	// 使用候选对象保证失败时不污染调用者原有数据。
	FNiumaAccountLoginResponseDto Candidate;

	if (!RootObject->TryGetBoolField(
		TEXT("success"),Candidate.bSuccess))
	{
		return Fail(OutError,TEXT("登录响应缺少合法的 success"));
	}

	if (!RootObject->TryGetStringField(
		TEXT("code"),Candidate.Code) || Candidate.Code.TrimStartAndEnd().IsEmpty())
	{
		return Fail(OutError,TEXT("登录响应缺少合法的 code"));
	}

	if (!RootObject->TryGetStringField(
		TEXT("message"),Candidate.Message))
	{
		return Fail(OutError,TEXT("登录响应缺少合法的 message"));
	}

	if (!RootObject->TryGetNumberField(
		TEXT("timestamp"),Candidate.Timestamp) || Candidate.Timestamp <= 0)
	{
		return Fail(OutError,TEXT("登录响应缺少合法的 timestamp"));
	}

	//当前 JSON 对象中查找键名为 `data` 的字段
	const TSharedPtr<FJsonValue> DataValue = RootObject->TryGetField(TEXT("data"));

	if (!DataValue.IsValid())
	{
		return Fail(OutError,TEXT("登录响应缺少 data 字段"));
	}

	if (!Candidate.bSuccess)
	{
		if (Candidate.Code == TEXT("OK"))
		{
			return Fail(OutError,TEXT("失败响应不能使用 OK 错误码"));
		}

		if (DataValue->Type != EJson::Null)
		{
			return Fail(OutError,TEXT("登录失败响应的 data 必须为 null"));
		}

		OutResponse = MoveTemp(Candidate);

		if (OutError != nullptr)
		{
			OutError->Reset();
		}

		return true;
	}

	if (Candidate.Code != TEXT("OK"))
	{
		return Fail(
			OutError,
			TEXT("登录成功响应必须使用 OK 代码"));
	}

	const TSharedPtr<FJsonObject>* DataObject = nullptr;

	if (!RootObject->TryGetObjectField(
		TEXT("data"),
		DataObject) ||
		DataObject == nullptr ||
		!DataObject->IsValid())
	{
		return Fail(
			OutError,
			TEXT("登录成功响应缺少合法的 data 对象"));
	}

	FText ConversionError;

	if (!FJsonObjectConverter::JsonObjectToUStruct(
		DataObject->ToSharedRef(),
		&Candidate.Data,
		0,
		0,
		true,
		&ConversionError))
	{
		return Fail(OutError,TEXT("登录响应 data 结构不符合协议"));
	}

	if (Candidate.Data.AccessToken.TrimStartAndEnd().IsEmpty())
	{
		return Fail(OutError,TEXT("登录响应 AccessToken 不能为空"));
	}

	if (Candidate.Data.TokenType != TEXT("Bearer"))
	{
		return Fail(OutError,TEXT("登录响应 TokenType 必须为 Bearer"));
	}

	if (Candidate.Data.ExpiresInSeconds <= 0)
	{
		return Fail(OutError,TEXT("登录响应有效期必须大于 0"));
	}

	if (!IsValidPlayerUid(Candidate.Data.PlayerUid))
	{
		return Fail(
			OutError,
			TEXT("登录响应 PlayerUid 必须是九位数字"));
	}

	OutResponse = MoveTemp(Candidate);

	if (OutError != nullptr)
	{
		OutError->Reset();
	}

	return true;
}