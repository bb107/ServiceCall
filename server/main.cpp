#include "../ServiceCall/ServiceCall.h"
#include "../ServiceCall/ntstatus.h"
#include <cstdio>

//NTSTATUS BSAPI plus(int a, int b, int* c) {
//	HANDLE hProcess;
//	DWORD dwPid;
//	if (RtlClientId(&hProcess, &dwPid)) {
//		printf("[*] [plus] hClientProcess = %p, dwClientProcessId = %d\n", hProcess, dwPid);
//	}
//	else {
//		printf("[-] [plus] RtlClientId failed.\n");
//	}
//	*c = a + b;
//	return STATUS_SUCCESS;
//}
//NTSTATUS BSAPI dec(int a, int b, int* c) {
//	HANDLE hProcess;
//	DWORD dwPid;
//	if (RtlClientId(&hProcess, &dwPid)) {
//		printf("[*] [dec] hClientProcess = %p, dwClientProcessId = %d\n", hProcess, dwPid);
//	}
//	else {
//		printf("[-] [dec] RtlClientId failed.\n");
//	}
//	*c = a - b;
//	return STATUS_SUCCESS;
//}

NTSTATUS BSAPI BsQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, LPVOID lpBuffer, DWORD dwBufferSize, LPDWORD lpdwReturnLength) {
	HANDLE hProcess;
	DWORD dwProcessId;
	if (RtlClientId(&hProcess, &dwProcessId)) {
		printf("[*] hProcess = 0x%08X, ProcessId = %d\n\tInformationClass = %d, lpBuffer = 0x%p, dwBufferSize = %d\n",
			hProcess, dwProcessId, SystemInformationClass, lpBuffer, dwBufferSize);
	}
	return NtQuerySystemInformation(SystemInformationClass, lpBuffer, dwBufferSize, lpdwReturnLength);
}
HANDLE BsRegisterService() {
	HANDLE hServiceRoutine = nullptr;
	PSERVICE_ROUTINE_PARAMETER param = reinterpret_cast<PSERVICE_ROUTINE_PARAMETER>(new char[PARAMETER_BUFFER_SIZE(4)]);
	RtlInitializeServiceParameters(param, 4);
	RtlServiceParametersValue(param, 0, _DWORD, 0);
	RtlServiceParametersPtr(param, 1, _PTR_NO_MAP, nullptr, 0);
	RtlServiceParametersValue(param, 2, _DWORD, 0);
	RtlServiceParametersPtr(param, 3, _PTR_NO_MAP, nullptr, 0);
	ScRegisterServiceRoutine(&hServiceRoutine, BsQuerySystemInformation, param, L"BsQuerySystemInformation");
	delete[]reinterpret_cast<LPVOID>(param);
	return hServiceRoutine;
}

int main() {
	//SIZE_T size = sizeof(DWORD64) + sizeof(SERVICE_PARAMETER_BLOB) * 3;
	//PSERVICE_ROUTINE_PARAMETER param = reinterpret_cast<PSERVICE_ROUTINE_PARAMETER>(RtlZeroMemory(new char[size], size));
	//HANDLE hServicePlus, hServiceDec, hServiceDec2;
	//RtlInitializeServiceParameters(param, 3);
	//RtlServiceParametersValue(param, 0, _DWORD, 0);
	//RtlServiceParametersValue(param, 1, _DWORD, 0);
	//RtlServiceParametersPtr(param, 2, _PTR, nullptr, 0);
	ScInitializeService();
	HANDLE hServiceRoutine = BsRegisterService();
	if (hServiceRoutine) {
		while (true)Sleep(0xff);
		ScUnregisterServiceRoutine(hServiceRoutine);
	}
	//if (ScRegisterServiceRoutineW(&hServicePlus, plus, param, L"ServerPlusInt") != STATUS_SUCCESS) return 0;
	//if (ScRegisterServiceRoutineW(&hServiceDec, dec, param, L"ServerDecInt") != STATUS_SUCCESS) return 0;
	//if (ScRegisterServiceRoutineW(&hServiceDec2, dec, param, L"ServerDecInt") == STATUS_SUCCESS) return 0;
	//delete[]reinterpret_cast<LPVOID>(param);
	//while (true)Sleep(0xff);
	//ScUnregisterServiceRoutine(hServicePlus);
	//ScUnregisterServiceRoutine(hServiceDec);
	return 0;
}
