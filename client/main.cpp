#include "../ServiceCall/ServiceCall.h"
#include "../ServiceCall/ntstatus.h"
#include <cstdio>

NTSTATUS __cdecl plus_dec_routine(bool plus, int a, int b, int*c) {
	NTSTATUS status, status2;
	SIZE_T size = sizeof(DWORD64) + sizeof(SERVICE_PARAMETER_BLOB) * 3;
	PSERVICE_ROUTINE_PARAMETER param = reinterpret_cast<PSERVICE_ROUTINE_PARAMETER>(RtlZeroMemory(new char[size], size));
	RtlInitializeServiceParameters(param, 3);
	RtlServiceParametersValue(param, 0, _DWORD, a);
	RtlServiceParametersValue(param, 1, _DWORD, b);
	RtlServiceParametersPtr(param, 2, _PTR, c, sizeof(int));
	if (!NT_SUCCESS(status2 = ScServiceCallW(plus ? L"ServerPlusInt" : L"ServerDecInt", param, &status))) {
		status = status2;
	}
	delete[]reinterpret_cast<LPVOID>(param);
	return status;
}
#define plus(a, b, c) plus_dec_routine(true, a, b, c)
#define dec(a, b, c) plus_dec_routine(false, a, b, c)

int main() {
	int a = 0, b = 0, c = 0, d = 0, e = 0;
	plus(1, 1, &a);
	plus(111, 222, &b);
	plus(a, b, &c);
	dec(2333, 666, &d);
	dec(d, c, &e);
	printf("1 + 1 = %d\n", a);			//1 + 1 = 2
	printf("111 + 222 = %d\n", b);		//111 + 222 = 333
	printf("a + b = %d\n", c);			//a + b = 335
	printf("2333 - 666 = %d\n", d);		//2333 - 666 = 1667
	printf("d - c = %d\n", e);			//d - c = 1332
	return 0;
}
