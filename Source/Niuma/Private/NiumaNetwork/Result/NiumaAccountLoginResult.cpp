#include "NiumaNetwork/Result/NiumaAccountLoginResult.h"

FNiumaAccountLoginResult FNiumaAccountLoginResult::MakeSuccess(
	int32 InHttpStatusCode,
	FNiumaAccountLoginDataDto InLoginData)
{
	FNiumaAccountLoginResult Result;

	Result.Outcome = ENiumaRemoteOutcome::Success;

	Result.HttpStatusCode = InHttpStatusCode;

	Result.ServerCode = TEXT("OK");

	Result.LoginData = MoveTemp(InLoginData);

	return Result;
}

FNiumaAccountLoginResult FNiumaAccountLoginResult::MakeBusinessFailure(
	int32 InHttpStatusCode,
	FString InServerCode,
	FString InMessage)
{
	FNiumaAccountLoginResult Result;

	Result.Outcome = ENiumaRemoteOutcome::BusinessFailure;

	Result.HttpStatusCode = InHttpStatusCode;

	Result.ServerCode = MoveTemp(InServerCode);

	Result.Message = MoveTemp(InMessage);

	return Result;
}

FNiumaAccountLoginResult FNiumaAccountLoginResult::MakeTransportFailure(
	FString InDiagnosticMessage)
{
	FNiumaAccountLoginResult Result;

	Result.Outcome = ENiumaRemoteOutcome::TransportFailure;

	Result.Message = MoveTemp(InDiagnosticMessage);

	return Result;
}

FNiumaAccountLoginResult FNiumaAccountLoginResult::MakeProtocolFailure(
	int32 InHttpStatusCode,
	FString InDiagnosticMessage)
{
	FNiumaAccountLoginResult Result;

	Result.Outcome = ENiumaRemoteOutcome::ProtocolFailure;

	Result.HttpStatusCode = InHttpStatusCode;

	Result.Message = MoveTemp(InDiagnosticMessage);

	return Result;
}

bool FNiumaAccountLoginResult::IsSuccess() const
{
	return Outcome == ENiumaRemoteOutcome::Success;
}

bool FNiumaAccountLoginResult::HasHttpResponse() const
{
	return HttpStatusCode > 0;
}

ENiumaRemoteOutcome FNiumaAccountLoginResult::GetOutcome() const
{
	return Outcome;
}

int32 FNiumaAccountLoginResult::GetHttpStatusCode() const
{
	return HttpStatusCode;
}

const FString& FNiumaAccountLoginResult::GetServerCode() const
{
	return ServerCode;
}

const FString& FNiumaAccountLoginResult::GetMessage() const
{
	return Message;
}

const FNiumaAccountLoginDataDto* FNiumaAccountLoginResult::GetLoginData() const
{
	return IsSuccess() ? &LoginData : nullptr;
}