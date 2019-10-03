#include "../ServiceCall/ServiceCall.h"
#include "../ServiceCall/ntstatus.h"
#include <cstdio>

//NTSTATUS __cdecl plus_dec_routine(bool plus, int a, int b, int*c) {
//	NTSTATUS status, status2;
//	SIZE_T size = sizeof(DWORD64) + sizeof(SERVICE_PARAMETER_BLOB) * 3;
//	PSERVICE_ROUTINE_PARAMETER param = reinterpret_cast<PSERVICE_ROUTINE_PARAMETER>(RtlZeroMemory(new char[size], size));
//	RtlInitializeServiceParameters(param, 3);
//	RtlServiceParametersValue(param, 0, _DWORD, a);
//	RtlServiceParametersValue(param, 1, _DWORD, b);
//	RtlServiceParametersPtr(param, 2, _PTR, c, sizeof(int));
//	if (!NT_SUCCESS(status2 = ScServiceCallW(plus ? L"ServerPlusInt" : L"ServerDecInt", param, &status))) {
//		status = status2;
//	}
//	delete[]reinterpret_cast<LPVOID>(param);
//	return status;
//}
//#define plus(a, b, c) plus_dec_routine(true, a, b, c)
//#define dec(a, b, c) plus_dec_routine(false, a, b, c)

NTSTATUS BSAPI BsQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, LPVOID lpBuffer, DWORD dwBufferSize, LPDWORD lpdwReturnLength) {
	static PSERVICE_ROUTINE_PARAMETER param = reinterpret_cast<PSERVICE_ROUTINE_PARAMETER>(new char[PARAMETER_BUFFER_SIZE(4)]);
	NTSTATUS status, status2;
	RtlInitializeServiceParameters(param, 4);
	RtlServiceParametersValue(param, 0, _DWORD, SystemInformationClass);
	RtlServiceParametersPtr(param, 1, _PTR_NO_MAP, lpBuffer, dwBufferSize);
	RtlServiceParametersValue(param, 2, _DWORD, dwBufferSize);
	RtlServiceParametersPtr(param, 3, _PTR_NO_MAP, lpdwReturnLength, lpdwReturnLength ? sizeof(DWORD) : 0);
	if (!NT_SUCCESS(status2 = ScServiceCallW(L"BsQuerySystemInformation", param, &status))) {
		status = status2;
	}
	return status;
}

int main() {
	//int a = 0, b = 0, c = 0, d = 0, e = 0;
	//plus(1, 1, &a);
	//plus(111, 222, &b);
	//plus(a, b, &c);
	//dec(2333, 666, &d);
	//dec(d, c, &e);
	//printf("1 + 1 = %d\n", a);			//1 + 1 = 2
	//printf("111 + 222 = %d\n", b);		//111 + 222 = 333
	//printf("a + b = %d\n", c);			//a + b = 335
	//printf("2333 - 666 = %d\n", d);		//2333 - 666 = 1667
	//printf("d - c = %d\n", e);			//d - c = 1332
	DWORD len = 0;
	LPVOID lpBuffer = nullptr;
	BsQuerySystemInformation(SystemProcessInformation, lpBuffer, len, &len);
	if (!len) {
		//error
		abort();
	}
	lpBuffer = new char[len];
	if (!NT_SUCCESS(BsQuerySystemInformation(SystemProcessInformation, lpBuffer, len, &len))) {
		//error
		abort();
	}
	delete[]lpBuffer;
	return 0;
}
