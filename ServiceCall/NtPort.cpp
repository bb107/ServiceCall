#include "NtPort.h"

NTSTATUS NTAPI NtAcceptConnectPort(
	OUT PHANDLE							ServerPortHandle,
	IN HANDLE							AlternativeReceivePortHandle	OPTIONAL,
	IN PLPC_MESSAGE						ConnectionReply,
	IN BOOLEAN							AcceptConnection,
	IN OUT PLPC_SECTION_OWNER_MEMORY	ServerSharedMemory				OPTIONAL,
	OUT PLPC_SECTION_MEMORY				ClientSharedMemory				OPTIONAL) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(PHANDLE, HANDLE, PLPC_MESSAGE, BOOLEAN, PLPC_SECTION_OWNER_MEMORY, PLPC_SECTION_MEMORY)>
		(RtlGetNtProcAddress("NtAcceptConnectPort"))(
			ServerPortHandle,
			AlternativeReceivePortHandle,
			ConnectionReply,
			AcceptConnection,
			ServerSharedMemory,
			ClientSharedMemory);
}

NTSTATUS NTAPI NtCompleteConnectPort(IN HANDLE PortHandle) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE)>(RtlGetNtProcAddress("NtCompleteConnectPort"))(PortHandle);
}

NTSTATUS NTAPI NtConnectPort(
	OUT PHANDLE							ClientPortHandle,
	IN PUNICODE_STRING					ServerPortName,
	IN PSECURITY_QUALITY_OF_SERVICE		SecurityQos,
	IN OUT PLPC_SECTION_OWNER_MEMORY	ClientSharedMemory		OPTIONAL,
	OUT PLPC_SECTION_MEMORY				ServerSharedMemory		OPTIONAL,
	OUT PULONG							MaximumMessageLength	OPTIONAL,
	IN PVOID							ConnectionInfo			OPTIONAL,
	IN PULONG							ConnectionInfoLength	OPTIONAL) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(PHANDLE, PUNICODE_STRING, PSECURITY_QUALITY_OF_SERVICE,
		PLPC_SECTION_OWNER_MEMORY, PLPC_SECTION_MEMORY, PULONG, PVOID, PULONG)>
		(RtlGetNtProcAddress("NtConnectPort"))(
			ClientPortHandle,
			ServerPortName,
			SecurityQos,
			ClientSharedMemory,
			ServerSharedMemory,
			MaximumMessageLength,
			ConnectionInfo,
			ConnectionInfoLength);
}

NTSTATUS NTAPI NtCreatePort(
	OUT PHANDLE             PortHandle,
	IN POBJECT_ATTRIBUTES   ObjectAttributes,
	IN ULONG                MaxConnectInfoLength,
	IN ULONG                MaxDataLength,
	IN OUT PULONG           Reserved OPTIONAL) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(PHANDLE, POBJECT_ATTRIBUTES, ULONG, ULONG, PULONG)>
		(RtlGetNtProcAddress("NtCreatePort"))(PortHandle, ObjectAttributes, MaxConnectInfoLength, MaxDataLength, Reserved);
}

NTSTATUS NTAPI NtImpersonateClientOfPort(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Request) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PLPC_MESSAGE)>(RtlGetNtProcAddress("NtImpersonateClientOfPort"))(PortHandle, Request);
}

NTSTATUS NTAPI NtListenPort(
	IN HANDLE               PortHandle,
	OUT PLPC_MESSAGE        ConnectionRequest) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PLPC_MESSAGE)>(RtlGetNtProcAddress("NtListenPort"))(PortHandle, ConnectionRequest);
}

NTSTATUS NTAPI NtQueryInformationPort(
	IN HANDLE					PortHandle,
	IN PORT_INFORMATION_CLASS	PortInformationClass,
	OUT PVOID					PortInformation,
	IN ULONG					Length,
	OUT PULONG					ResultLength OPTIONAL) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PORT_INFORMATION_CLASS, PVOID, ULONG, PULONG)>
		(RtlGetNtProcAddress("NtQueryInformationPort"))(PortHandle, PortInformationClass, PortInformation, Length, ResultLength);
}

NTSTATUS NTAPI NtReadRequestData(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Request,
	IN ULONG                DataIndex,
	OUT PVOID               Buffer,
	IN ULONG                Length,
	OUT PULONG              ResultLength OPTIONAL) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PLPC_MESSAGE, ULONG, PVOID, ULONG, PULONG)>
		(RtlGetNtProcAddress("NtReadRequestData"))(PortHandle, Request, DataIndex, Buffer, Length, ResultLength);
}

NTSTATUS NTAPI NtReplyPort(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Reply) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PLPC_MESSAGE)>(RtlGetNtProcAddress("NtReplyPort"))(PortHandle, Reply);
}

NTSTATUS NTAPI NtReplyWaitReceivePort(
	IN HANDLE               PortHandle,
	OUT PHANDLE             ReceivePortHandle OPTIONAL,
	IN PLPC_MESSAGE         Reply OPTIONAL,
	OUT PLPC_MESSAGE        IncomingRequest) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE,PHANDLE, PLPC_MESSAGE, PLPC_MESSAGE)>
		(RtlGetNtProcAddress("NtReplyWaitReceivePort"))(PortHandle, ReceivePortHandle, Reply, IncomingRequest);
}

NTSTATUS NTAPI NtReplyWaitReplyPort(
	IN HANDLE               PortHandle,
	IN OUT PLPC_MESSAGE     Reply) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PLPC_MESSAGE)>(RtlGetNtProcAddress("NtReplyWaitReplyPort"))(PortHandle, Reply);
}

NTSTATUS NTAPI NtRequestPort(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Request) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PLPC_MESSAGE)>(RtlGetNtProcAddress("NtRequestPort"))(PortHandle, Request);
}

NTSTATUS NTAPI NtRequestWaitReplyPort(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Request,
	OUT PLPC_MESSAGE        IncomingReply) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PLPC_MESSAGE, PLPC_MESSAGE)>(RtlGetNtProcAddress("NtRequestWaitReplyPort"))(PortHandle, Request, IncomingReply);
}

NTSTATUS NTAPI NtWriteRequestData(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Request,
	IN ULONG                DataIndex,
	IN PVOID                Buffer,
	IN ULONG                Length,
	OUT PULONG              ResultLength OPTIONAL) {
	return reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PLPC_MESSAGE, ULONG, PVOID, ULONG, PULONG)>
		(RtlGetNtProcAddress("NtWriteRequestData"))(PortHandle, Request, DataIndex, Buffer, Length, ResultLength);
}
