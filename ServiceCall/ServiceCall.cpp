#include "ServiceCall.h"
#include "NtPort.h"
#include "str.h"
#include "ntstatus.h"
#include <vector>
#pragma warning(disable:4996)

typedef struct _SERVICE_ROUTINE {
	HANDLE hPort;
	HANDLE hServerPort;

	HANDLE hClientProcess;
	PLPC_MESSAGE lm;
	
	LPVOID lpRoutine;
	PSERVICE_ROUTINE_PARAMETER lpParameters;

	BOOL CloseEvent;

	_SERVICE_ROUTINE() {
		RtlZeroMemory(this, sizeof(*this));
	}
	_SERVICE_ROUTINE(HANDLE hPort, LPVOID lpRoutine, PSERVICE_ROUTINE_PARAMETER lpParameters) {
		RtlZeroMemory(this, sizeof(*this));
		this->hPort = hPort;
		this->lpParameters = lpParameters;
		this->lpRoutine = lpRoutine;
	}
	_SERVICE_ROUTINE(_SERVICE_ROUTINE*src) {
		RtlCopyMemory(this, src, sizeof(*this));
	}

}SERVICE_ROUTINE, *PSERVICE_ROUTINE;
typedef std::vector<std::pair<HANDLE, DWORD>> CLIENT_ID_LIST, *PCLIENT_ID_LIST;

static PCRITICAL_SECTION lpCriticalSection = nullptr;
static PCLIENT_ID_LIST client_id_list = nullptr;
static DWORD system_lpc_message_length = 0;

#ifndef _WIN64
NTSTATUS __declspec(naked) __cdecl __x86ServiceCall(LPVOID lpServiceRoutine, PSERVICE_ROUTINE_PARAMETER lpParameters) {
	__asm {
		push ebx;								//save ebx
		mov edx, dword ptr ss : [esp + 0x8];	//edx = lpServiceRoutine
		mov ebx, dword ptr ss : [esp + 0xC];	//ebx = lpParameters
		mov ecx, dword ptr ds : [ebx];			//ecx = index = lpParameters->dwParameters
		lea ebx, dword ptr ds : [ebx + 0x8];	//ebx = &lpParameters->Parameters[0]
		lea eax, dword ptr ds : [ecx - 0x1];
		shl eax, 4;
		add eax, ebx;							//eax = &lpParameters->Parameters[index]

		/*
		while(index) {
			push lpParameters->Parameters[index].dwParamLow;
			index--;
		}
		*/
		_loop:
			cmp ecx, 0;
			je _break;
			push dword ptr ds : [eax + 0x8];
			sub eax, 0x10;
			dec ecx;
			jmp _loop;
		_break:
		call edx;
		pop ebx;
		ret;
	}
}
#define __ServiceCall __x86ServiceCall
#else
extern "C" NTSTATUS __x64ServiceCall(LPVOID lpServiceRoutine, PSERVICE_ROUTINE_PARAMETER lpParameters);
#define __ServiceCall __x64ServiceCall
#endif

_SERVICE_PARAMETER_BLOB * _SERVICE_PARAMETER_BLOB::operator=(_SERVICE_PARAMETER_BLOB __right) {
	RtlCopyMemory(this, &__right, sizeof(*this));
	return this;
}
_SERVICE_PARAMETER_BLOB * _SERVICE_PARAMETER_BLOB::operator=(_SERVICE_PARAMETER_BLOB * __right) {
	RtlCopyMemory(this, __right, sizeof(*this));
	return this;
}

PSERVICE_ROUTINE_PARAMETER RtlCaptureParameters(PSERVICE_ROUTINE_PARAMETER lpParameters) {
	PSERVICE_ROUTINE_PARAMETER lpCapturedParameters =
		reinterpret_cast<PSERVICE_ROUTINE_PARAMETER>(new char[sizeof(DWORD64) + lpParameters->dwParameters * sizeof(SERVICE_PARAMETER_BLOB)]);
	for (lpCapturedParameters->dwParameters = 0; lpCapturedParameters->dwParameters < lpParameters->dwParameters; lpCapturedParameters->dwParameters++) {
		BYTE cbMask = lpParameters->Parameters[lpCapturedParameters->dwParameters].ParameterMask;
		if ((cbMask & 0b00010000) ||
			((cbMask & 0b10000000) ? ((cbMask & 0b01000000) || (cbMask & 0b00100000) || !(cbMask & 0b00001111)) : !(cbMask & 0b01000000))) {
			delete[]reinterpret_cast<LPVOID>(lpCapturedParameters);
			return nullptr;
		}
		lpCapturedParameters->Parameters[lpCapturedParameters->dwParameters] = lpParameters->Parameters + lpCapturedParameters->dwParameters;
	}
	return lpCapturedParameters;
}

PSERVICE_ROUTINE_PARAMETER RtlMapPtrParameters(HANDLE hProcess, PSERVICE_ROUTINE_PARAMETER lpParametersToMap) {
	PSERVICE_ROUTINE_PARAMETER tmp =
		reinterpret_cast<PSERVICE_ROUTINE_PARAMETER>(new char[sizeof(DWORD64) + sizeof(SERVICE_PARAMETER_BLOB)*lpParametersToMap->dwParameters]);
	tmp->dwParameters = lpParametersToMap->dwParameters;
	for (DWORD i = 0; i < tmp->dwParameters; i++) {
		RtlCopyMemory(&tmp->Parameters[i], &lpParametersToMap->Parameters[i], sizeof(SERVICE_PARAMETER_BLOB));
		if (lpParametersToMap->Parameters[i].ParameterMask & 0b01000000) {
			if (lpParametersToMap->Parameters[i].ParameterMask & 0b00100000) {
				if (lpParametersToMap->Parameters[i].ulParameter &&
					(!(tmp->Parameters[i].ulParameter = reinterpret_cast<ULONG64>(new(std::nothrow)char[tmp->Parameters[i].dwMaxBufferSize])) ||
						!NT_SUCCESS(NtReadVirtualMemory(hProcess, reinterpret_cast<PVOID>(lpParametersToMap->Parameters[i].ulParameter),
							reinterpret_cast<PVOID>(tmp->Parameters[i].ulParameter), lpParametersToMap->Parameters[i].dwMaxBufferSize, nullptr)))) {
					RtlUnmapPtrParameters(tmp);
					return nullptr;
				}
			}
			else {
				if (!(tmp->Parameters[i].ulParameter = reinterpret_cast<ULONG64>(new(std::nothrow)char[tmp->Parameters[i].dwMaxBufferSize])) ||
					!NT_SUCCESS(NtReadVirtualMemory(hProcess, reinterpret_cast<PVOID>(lpParametersToMap->Parameters[i].ulParameter),
						reinterpret_cast<PVOID>(tmp->Parameters[i].ulParameter), lpParametersToMap->Parameters[i].dwMaxBufferSize, nullptr))) {
					RtlUnmapPtrParameters(tmp);
					return nullptr;
				}
			}
		}
	}
	return tmp;
}
void RtlUnmapPtrParameters(PSERVICE_ROUTINE_PARAMETER lpParametersToUnmap) {
	for (DWORD i = 0; i < lpParametersToUnmap->dwParameters; i++) {
		if ((lpParametersToUnmap->Parameters[i].ParameterMask & 0b01000000) &&
			lpParametersToUnmap->Parameters[i].dwMaxBufferSize && lpParametersToUnmap->Parameters[i].ulParameter)
			delete[]reinterpret_cast<LPVOID>(lpParametersToUnmap->Parameters[i].ulParameter);
	}
	delete[]reinterpret_cast<LPVOID>(lpParametersToUnmap);
}
bool RtlSyncMappedPtrParameters(HANDLE hClientProcess, PSERVICE_ROUTINE_PARAMETER lpClientPara, PSERVICE_ROUTINE_PARAMETER lpMappedPara) {
	for (DWORD i = 0; i < lpClientPara->dwParameters; i++) {
		BYTE cbMask = lpClientPara->Parameters[i].ParameterMask;
		if ((cbMask & 0b01000000) && lpClientPara->Parameters[i].ulParameter) {
			if (!NT_SUCCESS(NtWriteVirtualMemory(hClientProcess,
				reinterpret_cast<PVOID>(lpClientPara->Parameters[i].ulParameter),
				reinterpret_cast<PVOID>(lpMappedPara->Parameters[i].ulParameter),
				lpClientPara->Parameters[i].dwMaxBufferSize, nullptr)))return false;
		}
	}
	return true;
}

bool RtlEqualParameterType(PSERVICE_ROUTINE_PARAMETER p1, PSERVICE_ROUTINE_PARAMETER p2) {
	if (p1->dwParameters != p2->dwParameters)return false;
	for (DWORD i = 0; i < p1->dwParameters; i++) {
		if (p1->Parameters[i].ParameterMask != p2->Parameters[i].ParameterMask)return false;
	}
	return true;
}

DWORD RtlSystemLpcMessageLength() {
	SYSTEM_INFO info;
	GetNativeSystemInfo(&info);
	switch (info.wProcessorArchitecture) {
	case PROCESSOR_ARCHITECTURE_INTEL:return LPC_MESSAGE_LENGTH_32;
	case PROCESSOR_ARCHITECTURE_AMD64:return LPC_MESSAGE_LENGTH_64;
	default:return 0;
	}
}

//查询调用客户端id, 只能由服务例程调用, 进程句柄将在服务例程返回后失效.
NTSTATUS RtlSafeThreadServiceClientId(PHANDLE hProcess, PDWORD lpdwProcessId) {
	NTSTATUS result = STATUS_NOT_FOUND;
	DWORD dwThreadId = reinterpret_cast<DWORD>(NtCurrentTeb()->ClientId.UniqueThread);
	if (!lpCriticalSection || !client_id_list)return STATUS_NOT_SUPPORTED;
	EnterCriticalSection(lpCriticalSection);
	for (CLIENT_ID_LIST::const_iterator i = client_id_list->cbegin(); i != client_id_list->cend(); i++) {
		if (i->second == dwThreadId) {
			*hProcess = i->first;
			*lpdwProcessId = GetProcessId(*hProcess);
			result = STATUS_SUCCESS;
			break;
		}
	}
	LeaveCriticalSection(lpCriticalSection);
	return result;
}
bool BSAPI RtlClientId(PHANDLE hClientProcess, PDWORD lpdwClientProcessId) {
	return NT_SUCCESS(RtlSafeThreadServiceClientId(hClientProcess, lpdwClientProcessId));
}

DWORD WINAPI WorkerThread(LPVOID lpParameters) {
	PSERVICE_ROUTINE lpService = reinterpret_cast<PSERVICE_ROUTINE>(lpParameters);
	PSERVICE_ROUTINE_PARAMETER lpMappedParam = nullptr;
	DWORD dwThread;

	EnterCriticalSection(lpCriticalSection);
	client_id_list->push_back(std::make_pair(lpService->hClientProcess, dwThread = reinterpret_cast<DWORD>(NtCurrentTeb()->ClientId.UniqueThread)));
	LeaveCriticalSection(lpCriticalSection);

#ifndef _WIN64
	for (DWORD i = 0; i < lpService->lpParameters->dwParameters; i++) {
		BYTE cbMask = lpService->lpParameters->Parameters[i].ParameterMask;
		if (cbMask & 0b10000000) {
			if (cbMask & 0b00000100) {
				lpService->lpParameters->Parameters[i].ulParameter &= 0xffffffffull;
			}
			else if (cbMask & 0b00000010) {
				lpService->lpParameters->Parameters[i].ulParameter &= 0xffffull;
			}
			else if (cbMask & 0b00000001) {
				lpService->lpParameters->Parameters[i].ulParameter &= 0xffull;
			}
		}
		else {
			lpService->lpParameters->Parameters[i].ulParameter &= 0xffffffffull;
		}
	}
#endif

	if (!(lpMappedParam = RtlMapPtrParameters(lpService->hClientProcess, lpService->lpParameters))) {
		*LPC_MESSAGE_DATA_BUFFER(lpService->lm, PNTSTATUS) = STATUS_INVALID_PARAMETER;
	}
	else {
		*LPC_MESSAGE_DATA_BUFFER(lpService->lm, PNTSTATUS) = __ServiceCall(lpService->lpRoutine, lpMappedParam);
		if (!RtlSyncMappedPtrParameters(lpService->hClientProcess, lpService->lpParameters, lpMappedParam)) {
			*LPC_MESSAGE_DATA_BUFFER(lpService->lm, PNTSTATUS) = STATUS_ACCESS_DENIED;
		}
		RtlUnmapPtrParameters(lpMappedParam);
	}

	lpService->lm->u1.s1.TotalLength = system_lpc_message_length + sizeof(NTSTATUS);
	lpService->lm->u1.s1.DataLength = sizeof(NTSTATUS);
	NtReplyWaitReplyPort(lpService->hServerPort, lpService->lm);

	EnterCriticalSection(lpCriticalSection);
	for (CLIENT_ID_LIST::iterator i = client_id_list->begin(); i != client_id_list->end(); i++) {
		if (i->second == dwThread) {
			client_id_list->erase(i);
			break;
		}
	}
	LeaveCriticalSection(lpCriticalSection);

	NtClose(lpService->hClientProcess);
	NtClose(lpService->hServerPort);
	delete[]reinterpret_cast<LPVOID>(lpService->lm);
	delete lpService;
	return 0;
}

DWORD WINAPI WaitThread(LPVOID lpParameters) {
	PSERVICE_ROUTINE lpService = reinterpret_cast<PSERVICE_ROUTINE>(lpParameters), param = nullptr;
	PLPC_MESSAGE lm = reinterpret_cast<PLPC_MESSAGE>(RtlZeroMemory(new char[LPCP_MAX_MESSAGE_SIZE], LPCP_MAX_MESSAGE_SIZE));
	PSERVICE_ROUTINE_PARAMETER callparam = nullptr;
	while (true) {
		Sleep(10);
		if (!NT_SUCCESS(NtListenPort(lpService->hPort, lm))) {
			if(lpService->CloseEvent) break;
			continue;
		}
		param = new SERVICE_ROUTINE(lpService);
		if (!(param->hClientProcess =
			OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_DUP_HANDLE | PROCESS_CREATE_THREAD,
				FALSE, reinterpret_cast<DWORD>(lm->ClientId.UniqueProcess))) ||
			!lm->u1.s1.DataLength || !RtlEqualParameterType(lpService->lpParameters,
				callparam = reinterpret_cast<PSERVICE_ROUTINE_PARAMETER>(reinterpret_cast<size_t>(lm) + lm->u1.s1.TotalLength - lm->u1.s1.DataLength))) {
			NtAcceptConnectPort(&param->hServerPort, 0, lm, FALSE, nullptr, nullptr);
			if (param->hClientProcess)NtClose(param->hClientProcess);
			delete param;
			continue;
		}
		if (!NT_SUCCESS(NtAcceptConnectPort(&param->hServerPort, 0, lm, TRUE, nullptr, nullptr)) ||
			!NT_SUCCESS(NtCompleteConnectPort(param->hServerPort)) ||
			!NT_SUCCESS(NtReplyWaitReceivePort(param->hServerPort, nullptr, nullptr, lm))) {
			NtClose(param->hClientProcess);
			delete param;
			continue;
		}
		param->lm = reinterpret_cast<PLPC_MESSAGE>(RtlCopyMemory(new char[LPCP_MAX_MESSAGE_SIZE], lm, LPCP_MAX_MESSAGE_SIZE));
		param->lpParameters = RtlCaptureParameters(callparam);
		CreateThread(nullptr, 0, WorkerThread, param, 0, nullptr);
	}
	delete[]reinterpret_cast<LPVOID>(lm);
	delete[]reinterpret_cast<LPVOID>(lpService->lpParameters);
	delete lpService;
	return 0;
}


//注册服务例程前必须调用
NTSTATUS BSAPI ScInitializeService() {
	if (lpCriticalSection)return STATUS_SUCCESS;
	InitializeCriticalSection(lpCriticalSection = new CRITICAL_SECTION);
	client_id_list = new CLIENT_ID_LIST(0);
	system_lpc_message_length = RtlSystemLpcMessageLength();
	return STATUS_SUCCESS;
}

//注册服务例程
NTSTATUS BSAPI ScRegisterServiceRoutineA(
	OUT PHANDLE hServiceRoutine,
	IN LPVOID lpServiceRoutine,
	IN PSERVICE_ROUTINE_PARAMETER lpParameters,
	IN LPCSTR lpRoutineName) {
	std::wstring name(CharToWchar(lpRoutineName));
	LPWSTR lpName = new wchar_t[name.length() + 1];
	NTSTATUS status = ScRegisterServiceRoutineW(hServiceRoutine, lpServiceRoutine, lpParameters, wcscpy(lpName, name.c_str()));
	delete[]lpName;
	return status;
}

NTSTATUS BSAPI ScRegisterServiceRoutineW(
	OUT PHANDLE hServiceRoutine,
	IN LPVOID lpServiceRoutine,
	IN PSERVICE_ROUTINE_PARAMETER lpParameters,
	IN LPCWSTR lpRoutineName) {
	std::wstring NtRoutineName = L"\\RPC Control\\";
	UNICODE_STRING us;
	OBJECT_ATTRIBUTES oa;
	HANDLE hPort;
	PSERVICE_ROUTINE hResult = nullptr;
	PSERVICE_ROUTINE_PARAMETER lpCapturedParameters = nullptr;
	const ULONG ulMaxMessageSize = LPCP_MAX_MESSAGE_SIZE - FIELD_OFFSET(LPCP_MESSAGE, Request),
		ulMaxConnectionSize = ulMaxMessageSize - sizeof(PORT_MESSAGE) - sizeof(LPCP_CONNECTION_MESSAGE);
	if (!lpCriticalSection || !client_id_list) {
		return STATUS_NOT_SUPPORTED;
	}
	if (IsBadWritePtr(hServiceRoutine, sizeof(HANDLE)) || IsBadReadPtr(lpServiceRoutine, sizeof(LPVOID)) ||
		IsBadReadPtr(lpParameters, sizeof(SERVICE_ROUTINE_PARAMETER)) ||
		IsBadReadPtr(lpParameters, lpParameters->dwParameters * sizeof(SERVICE_PARAMETER_BLOB) + sizeof(DWORD)) ||
		IsBadStringPtrW(lpRoutineName, 0xffffffff)) return STATUS_ACCESS_VIOLATION;
	NtRoutineName += lpRoutineName;
	RtlCreateUnicodeString(&us, NtRoutineName.c_str());
	InitializeObjectAttributes(&oa, &us, 0, nullptr, nullptr);
	*hServiceRoutine = nullptr;
	if (!(lpCapturedParameters = RtlCaptureParameters(lpParameters))) {
		RtlFreeUnicodeString(&us);
		return STATUS_INVALID_PARAMETER_3;
	}
	if (!NT_SUCCESS(NtCreatePort(&hPort, &oa, ulMaxConnectionSize, ulMaxMessageSize, nullptr))) {
		delete[]reinterpret_cast<LPVOID>(lpCapturedParameters);
		RtlFreeUnicodeString(&us);
		return STATUS_OBJECT_NAME_EXISTS;
	}
	RtlFreeUnicodeString(&us);
	if (!CreateThread(nullptr, 0, WaitThread, hResult = new SERVICE_ROUTINE(hPort, lpServiceRoutine, lpCapturedParameters), 0, nullptr)) {
		delete[]reinterpret_cast<LPVOID>(lpCapturedParameters);
		delete hResult;
		NtClose(hPort);
		return STATUS_NO_MEMORY;
	}
	*hServiceRoutine = hResult;
	return STATUS_SUCCESS;
}

//取消注册服务例程
NTSTATUS BSAPI ScUnregisterServiceRoutine(HANDLE hServiceRoutine) {
	PSERVICE_ROUTINE lpService = reinterpret_cast<PSERVICE_ROUTINE>(hServiceRoutine);
	if (IsBadReadPtr(lpService, sizeof(SERVICE_ROUTINE) || IsBadReadPtr(lpService->lpParameters, sizeof(LPVOID)) ||
		IsBadReadPtr(lpService->lpParameters, lpService->lpParameters->dwParameters * sizeof(SERVICE_PARAMETER_BLOB) + sizeof(DWORD)))) {
		return STATUS_ACCESS_VIOLATION;
	}
	lpService->CloseEvent = TRUE;
	return NT_SUCCESS(NtClose(lpService->hPort)) ? STATUS_SUCCESS : STATUS_INVALID_PARAMETER;
}

//客户端调用服务例程
NTSTATUS BSAPI ScServiceCallA(
	IN LPCSTR lpServiceName,
	IN PSERVICE_ROUTINE_PARAMETER lpParameters,
	OUT PNTSTATUS lpServiceStatus OPTIONAL) {
	std::wstring name(CharToWchar(lpServiceName));
	LPWSTR lpName = new wchar_t[name.length() + 1];
	NTSTATUS status = ScServiceCallW(name.c_str(), lpParameters, lpServiceStatus);
	delete[]lpName;
	return status;
}

NTSTATUS BSAPI ScServiceCallW(
	IN LPCWSTR lpServiceName,
	IN PSERVICE_ROUTINE_PARAMETER lpParameters,
	OUT PNTSTATUS lpServiceStatus OPTIONAL) {
	std::wstring NtRoutineName = L"\\RPC Control\\";
	UNICODE_STRING us;
	HANDLE hPort;
	PLPC_MESSAGE lm = nullptr;
	ULONG length, len;
	LPVOID lpCapture = nullptr;
	NTSTATUS status;

	if (IsBadReadPtr(lpParameters, sizeof(DWORD)) ||
		IsBadReadPtr(lpParameters, length = lpParameters->dwParameters * sizeof(SERVICE_PARAMETER_BLOB) + sizeof(DWORD64)) ||
		IsBadStringPtrW(lpServiceName, 0xffffffff)) {
		return STATUS_ACCESS_VIOLATION;
	}
	NtRoutineName += lpServiceName;
	RtlCreateUnicodeString(&us, NtRoutineName.c_str());

	for (DWORD i = 0; i < lpParameters->dwParameters; i++) {
		BYTE cbMask = lpParameters->Parameters[i].ParameterMask;
		if (cbMask & 0b01000000) {
			if (!(cbMask & 0b00100000) && !lpParameters->Parameters[i].ulParameter) {
				RtlFreeUnicodeString(&us);
				return STATUS_INVALID_PARAMETER_2;
			}
		}
	}
	if (!(lpCapture = RtlCaptureParameters(lpParameters))) {
		RtlFreeUnicodeString(&us);
		return STATUS_INVALID_PARAMETER_2;
	}
	delete[]lpCapture;

	status = NtConnectPort(&hPort, &us, nullptr, nullptr, nullptr, &len, lpParameters, &length);
	RtlFreeUnicodeString(&us);
	switch (status) {
	case STATUS_SUCCESS:break;
	case STATUS_PORT_CONNECTION_REFUSED:return STATUS_INVALID_PARAMETER_2;
	case STATUS_OBJECT_NAME_NOT_FOUND:return STATUS_INVALID_PARAMETER_1;
	default:return STATUS_UNSUCCESSFUL;
	}

	lm = reinterpret_cast<PLPC_MESSAGE>(RtlZeroMemory(new char[len], len));
	lm->u1.s1.TotalLength = len;
	status = STATUS_UNSUCCESSFUL;
	if (NT_SUCCESS(NtRequestWaitReplyPort(hPort, lm, lm))) {
		status = STATUS_SUCCESS;
		if (!system_lpc_message_length)system_lpc_message_length = RtlSystemLpcMessageLength();
		if (lpServiceStatus && !IsBadWritePtr(lpServiceStatus, sizeof(NTSTATUS)))*lpServiceStatus = *LPC_MESSAGE_DATA_BUFFER(lm, PNTSTATUS);
	}
	lm->u1.s1.DataLength = 0;
	NtReplyPort(hPort, lm);
	delete[]reinterpret_cast<LPVOID>(lm);
	NtClose(hPort);
	return status;
}
