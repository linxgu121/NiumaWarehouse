#include "NiumaNetwork/Session/NiumaAccountRuntimeSession.h"

namespace
{
	bool Fail(
		FString* OutError,
		const FString& Error)
	{
		if (OutError != nullptr)
		{
			*OutError = Error;
		}

		return false;
	}

	bool IsValidPlayerUid(
		const FString& PlayerUid)
	{
		if (PlayerUid.Len() != 9 ||
			PlayerUid[0] == TEXT('0'))
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

	bool IsValidTime(double TimeSeconds)
	{
		return TimeSeconds >= 0.0 &&
			FMath::IsFinite(TimeSeconds);
	}
}

bool FNiumaAccountRuntimeSession::TryEstablish(
	const FNiumaAccountLoginDataDto& LoginData,
	double CurrentTimeSeconds,
	FString* OutError)
{
	if (!IsValidTime(CurrentTimeSeconds))
	{
		return Fail(OutError,TEXT("当前运行时间无效"));
	}

	if (LoginData.AccessToken.IsEmpty() ||
		LoginData.AccessToken !=
		LoginData.AccessToken.TrimStartAndEnd())
	{
		return Fail(OutError,TEXT("Access Token 无效"));
	}

	if (LoginData.TokenType != TEXT("Bearer"))
	{
		return Fail(OutError,TEXT("只支持 Bearer Token"));
	}

	if (LoginData.ExpiresInSeconds <= 0)
	{
		return Fail(OutError,TEXT("Token 有效期必须大于 0"));
	}

	if (!IsValidPlayerUid(LoginData.PlayerUid))
	{
		return Fail(OutError,TEXT("PlayerUid 必须是有效的九位数字"));
	}

	const double CandidateExpiresAt =
		CurrentTimeSeconds +
		static_cast<double>(
			LoginData.ExpiresInSeconds);

	if (!FMath::IsFinite(CandidateExpiresAt) ||
		CandidateExpiresAt <= CurrentTimeSeconds)
	{
		return Fail(OutError,TEXT("Token 到期时间无效"));
	}

	// 所有数据验证完成后再统一修改当前会话。
	AccessToken = LoginData.AccessToken;
	PlayerUid = LoginData.PlayerUid;
	ExpiresAtSeconds = CandidateExpiresAt;

	if (OutError != nullptr)
	{
		OutError->Reset();
	}

	return true;
}

void FNiumaAccountRuntimeSession::Clear()
{
	AccessToken.Reset();
	PlayerUid.Reset();
	ExpiresAtSeconds = 0.0;
}

bool FNiumaAccountRuntimeSession::IsAuthenticated(
	double CurrentTimeSeconds) const
{
	return IsValidTime(CurrentTimeSeconds) &&
		!AccessToken.IsEmpty() &&
		!PlayerUid.IsEmpty() &&
		CurrentTimeSeconds < ExpiresAtSeconds;
}

bool
FNiumaAccountRuntimeSession::TryBuildAuthorizationHeader(
	double CurrentTimeSeconds,
	FString& OutAuthorizationHeader) const
{
	if (!IsAuthenticated(CurrentTimeSeconds))
	{
		return false;
	}

	const FString CandidateHeader =
		FString::Printf(
			TEXT("Bearer %s"),
			*AccessToken);

	OutAuthorizationHeader = CandidateHeader;
	return true;
}

bool FNiumaAccountRuntimeSession::TryGetPlayerUid(
	double CurrentTimeSeconds,
	FString& OutPlayerUid) const
{
	if (!IsAuthenticated(CurrentTimeSeconds))
	{
		return false;
	}

	OutPlayerUid = PlayerUid;
	return true;
}



