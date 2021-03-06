#include "stdafx.h"

#include <ntsecapi.h> 
#include <stdlib.h>
#include <codecvt>
#include <windows.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>

#ifdef _DEBUG
#import "..\BuildOutput\x64\Debug\MpfProxy.tlb" raw_interfaces_only
#else
#import "..\BuildOutput\x64\Release\MpfProxy.tlb" raw_interfaces_only
#endif

using namespace MpfProxy;
using namespace std;

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif

extern "C" __declspec(dllexport) BOOLEAN __stdcall InitializeChangeNotify(void) {
#ifdef DEBUG
	OutputDebugString(TEXT("Initialize Change Notify called!\n"));
#endif
	return TRUE;
}

//NTSTATUS NTAPI PasswordChangeNotify(
extern "C" __declspec(dllexport) int __stdcall
PasswordChangeNotify(
	PUNICODE_STRING UserName,
	ULONG RelativeId,
	PUNICODE_STRING Password
)
{
	return STATUS_SUCCESS;
}

extern "C" __declspec(dllexport) BOOLEAN __stdcall PasswordFilter(
	PUNICODE_STRING AccountName,
	PUNICODE_STRING FullName,
	PUNICODE_STRING Password,
	BOOLEAN SetOperation)
{
	VARIANT_BOOL isComplex = FALSE;

	FILE *filepoint;
	errno_t err;

	if ((err = fopen_s(&filepoint, "C:\\Windows\\System32\\MpfConfig.xml", "r")) != 0) {
		wprintf(L"File 'C:\\Windows\\System32\\MpfConfig.xml' could not be opened. Password filter is disabled.\n");
		return true;
	}
	else
	{
		fclose(filepoint);
	}

	using convert_type = codecvt_utf8<wchar_t>;
	wstring_convert<convert_type, wchar_t> converter;

	wstring wAccountName(AccountName->Buffer, AccountName->Length / sizeof(WCHAR));
	string sAccountName = converter.to_bytes(wAccountName);
	BSTR bsAccountName = SysAllocStringLen(wAccountName.data(), wAccountName.size());

	wstring wFullName(FullName->Buffer, FullName->Length / sizeof(WCHAR));
	string sFullName = converter.to_bytes(wFullName);
	BSTR bsFullName = SysAllocStringLen(wFullName.data(), wFullName.size());

	wstring wPassword(Password->Buffer, Password->Length / sizeof(WCHAR));
	string sPassword = converter.to_bytes(wPassword);
	BSTR bsPassword = SysAllocStringLen(wPassword.data(), wPassword.size());

	HRESULT hr = CoInitialize(NULL);

	IProxyPtr pIProxy(__uuidof(MpfProxy::Proxy));

	wprintf(L"Input data in unmanaged code: AccountName:'%ls' FullName:'%ls' Password:'%ls'\n\n", bsAccountName, bsFullName, bsPassword);
	pIProxy->TestPassword(bsAccountName, bsFullName, bsPassword, &isComplex);
	wprintf(L"Result in unmanaged code: %d\n\n", isComplex);

	CoUninitialize();
	
	return isComplex;
}