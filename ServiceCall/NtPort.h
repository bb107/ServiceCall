#pragma once
#include <Windows.h>
#include "Native.h"

#ifdef _WIN64
#define PORT_MAXIMUM_MESSAGE_LENGTH 512
#else
#define PORT_MAXIMUM_MESSAGE_LENGTH 256
#endif
#ifdef USE_LPC6432
#define LPC_CLIENT_ID CLIENT_ID64
#define LPC_SIZE_T ULONGLONG
#define LPC_PVOID ULONGLONG
#define LPC_HANDLE ULONGLONG
#else
#define LPC_CLIENT_ID CLIENT_ID
#define LPC_SIZE_T SIZE_T
#define LPC_PVOID PVOID
#define LPC_HANDLE HANDLE
#endif
typedef short CSHORT;
typedef struct _PORT_MESSAGE {
	union {
		struct {
			CSHORT DataLength;
			CSHORT TotalLength;
		} s1;
		ULONG Length;
	} u1;
	union {
		struct {
			CSHORT Type;
			CSHORT DataInfoOffset;
		} s2;
		ULONG ZeroInit;
	} u2;
	union {
		LPC_CLIENT_ID ClientId;
		double DoNotUseThisField;
	};
	ULONG MessageId;
	union {
		LPC_SIZE_T ClientViewSize;
		ULONG CallbackId;
	};
} PORT_MESSAGE, *PPORT_MESSAGE, LPC_MESSAGE, *PLPC_MESSAGE, LPC_MESSAGE_HEADER, *PLPC_MESSAGE_HEADER;
#define LPC_MESSAGE_LENGTH_32 24U
#define LPC_MESSAGE_LENGTH_64 40U
typedef struct _LPC_SECTION_MEMORY {
	ULONG                   Length;
	ULONG                   ViewSize;
	PVOID                   ViewBase;
} LPC_SECTION_MEMORY, *PLPC_SECTION_MEMORY;
typedef struct _LPC_SECTION_OWNER_MEMORY {
	ULONG                   Length;
	HANDLE                  SectionHandle;
	ULONG                   OffsetInSection;
	ULONG                   ViewSize;
	PVOID                   ViewBase;
	PVOID                   OtherSideViewBase;
} LPC_SECTION_OWNER_MEMORY, *PLPC_SECTION_OWNER_MEMORY;
typedef struct _LPC_TERMINATION_MESSAGE {
	LPC_MESSAGE_HEADER      Header;
	LARGE_INTEGER           CreationTime;
} LPC_TERMINATION_MESSAGE, *PLPC_TERMINATION_MESSAGE;
typedef enum _PORT_INFORMATION_CLASS {
	PortNoInformation
} PORT_INFORMATION_CLASS, *PPORT_INFORMATION_CLASS;
typedef enum _LPC_TYPE {
	LPC_NEW_MESSAGE,
	LPC_REQUEST,
	LPC_REPLY,
	LPC_DATAGRAM,
	LPC_LOST_REPLY,
	LPC_PORT_CLOSED,
	LPC_CLIENT_DIED,
	LPC_EXCEPTION,
	LPC_DEBUG_EVENT,
	LPC_ERROR_EVENT,
	LPC_CONNECTION_REQUEST,
	LPC_CONNECTION_REFUSED,
	LPC_MAXIMUM
} LPC_TYPE;
typedef struct _LPCP_MESSAGE {
	UCHAR Data[0x14];
	PORT_MESSAGE Request;
} LPCP_MESSAGE;
typedef struct _LPCP_CONNECTION_MESSAGE {
	UCHAR Data[0x2C];
} LPCP_CONNECTION_MESSAGE;

#define N_ROUND_UP(x,s) \
     (((ULONG)(x)+(s)-1) & ~((ULONG)(s)-1))
#define LPCP_MAX_MESSAGE_SIZE \
     N_ROUND_UP(PORT_MAXIMUM_MESSAGE_LENGTH + \
     sizeof(LPCP_MESSAGE) + \
     sizeof(LPCP_CONNECTION_MESSAGE), 16)

NTSTATUS NTAPI NtAcceptConnectPort(
	OUT PHANDLE             ServerPortHandle,
	IN HANDLE               AlternativeReceivePortHandle OPTIONAL,
	IN PLPC_MESSAGE         ConnectionReply,
	IN BOOLEAN              AcceptConnection,
	IN OUT PLPC_SECTION_OWNER_MEMORY ServerSharedMemory OPTIONAL,
	OUT PLPC_SECTION_MEMORY ClientSharedMemory OPTIONAL);

NTSTATUS NTAPI NtCompleteConnectPort(IN HANDLE PortHandle);

NTSTATUS NTAPI NtConnectPort(
	OUT PHANDLE             ClientPortHandle,
	IN PUNICODE_STRING      ServerPortName,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
	IN OUT PLPC_SECTION_OWNER_MEMORY ClientSharedMemory OPTIONAL,
	OUT PLPC_SECTION_MEMORY ServerSharedMemory OPTIONAL,
	OUT PULONG              MaximumMessageLength OPTIONAL,
	IN PVOID                ConnectionInfo OPTIONAL,
	IN PULONG               ConnectionInfoLength OPTIONAL);

NTSTATUS NTAPI NtCreatePort(
	OUT PHANDLE             PortHandle,
	IN POBJECT_ATTRIBUTES   ObjectAttributes,
	IN ULONG                MaxConnectInfoLength,
	IN ULONG                MaxDataLength,
	IN OUT PULONG           Reserved OPTIONAL);

NTSTATUS NTAPI NtImpersonateClientOfPort(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Request);

NTSTATUS NTAPI NtListenPort(
	IN HANDLE               PortHandle,
	OUT PLPC_MESSAGE        ConnectionRequest);

NTSTATUS NTAPI NtQueryInformationPort(
	IN HANDLE               PortHandle,
	IN PORT_INFORMATION_CLASS PortInformationClass,
	OUT PVOID               PortInformation,
	IN ULONG                Length,
	OUT PULONG              ResultLength OPTIONAL);

NTSTATUS NTAPI NtReadRequestData(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Request,
	IN ULONG                DataIndex,
	OUT PVOID               Buffer,
	IN ULONG                Length,
	OUT PULONG              ResultLength OPTIONAL);

NTSTATUS NTAPI NtReplyPort(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Reply);

NTSTATUS NTAPI NtReplyWaitReceivePort(
	IN HANDLE               PortHandle,
	OUT PHANDLE             ReceivePortHandle OPTIONAL,
	IN PLPC_MESSAGE         Reply OPTIONAL,
	OUT PLPC_MESSAGE        IncomingRequest);

NTSTATUS NTAPI NtReplyWaitReplyPort(
	IN HANDLE               PortHandle,
	IN OUT PLPC_MESSAGE     Reply);

NTSTATUS NTAPI NtRequestPort(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Request);

NTSTATUS NTAPI NtRequestWaitReplyPort(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Request,
	OUT PLPC_MESSAGE        IncomingReply);

NTSTATUS NTAPI NtWriteRequestData(
	IN HANDLE               PortHandle,
	IN PLPC_MESSAGE         Request,
	IN ULONG                DataIndex,
	IN PVOID                Buffer,
	IN ULONG                Length,
	OUT PULONG              ResultLength OPTIONAL);
