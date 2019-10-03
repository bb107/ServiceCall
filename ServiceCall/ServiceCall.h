#pragma once
#pragma warning(disable:4005)	//���ض���
#pragma warning(disable:4200)	//���С��Ա
#pragma warning(disable:4244)	//��С��ʧ����
#pragma warning(disable:4302)	//�ض�
#pragma warning(disable:4311)	//ָ��ض�
#include <Windows.h>
#include "Native.h"

#ifndef BSAPI
#ifndef _WIN64
#define BSAPI __stdcall
#else
#define BSAPI __fastcall
#endif
#endif

/*
 * Parameter Mask Bits:
 *
 *        7   6   5   4   3   2   1   0 
 *		+-----------+---+---------------+
 *		|   TYPE    | U |     SIZE      |
 *		|-------------------------------|
 *		| V | P | N | - | Q | D | W | B |
 *		| L | T | L | - | W | W | O | Y |
 *		+-----------+---+---------------+
 *
 * TYPE:
 * VL: ������ֵ, ��PT������, ��λ�󽫰���ֵ�ߴ縴�Ʋ������������.
 * PT: ������ָ��, ��VL������, ��λ�󽫻�ӳ��ָ�뵽�������.
 * NL: ָ�����Ϊ��ָ��(nullptr), PT��λ����Ч, ָ��Ϊnullptrʱ��ӳ��ָ��ָ����ڴ浽�������.
 *     ���PT��λ��NLδ��λ, �Ҳ���Ϊnullptrʱ, ������ý�ʧ��, STATUS = STATUS_INVALID_PARAMETERS
 *
 * U:     δʹ��, ����, ����Ϊ0, ���������ý�ʧ��, STATUS = STATUS_INVALID_PARAMETERS
 *
 * SIZE:  PT��λ�����, ���ж����λ, ����˳���QW���ȡ��һ��.
 * QW: QWORD 64λ�޷�������
 * DW: DWORD 32λ�޷�������
 * WO: WORD  16λ�޷�������
 * BY: BYTE  8λ�޷�������
 * ***������ֻ�ܴ�ָ��***
 *
*/

//Value
#define _QWORD			0b10001000	//�������Ϊ32λʱ����DWORD
#define _DWORD			0b10000100
#define _WORD			0b10000010
#define _BYTE			0b10000001

//Ptr
#define _PTR			0b01000000
#define _PTR_NO_MAP		0b01100000

//��ʼ���ṹ
//initialize parameter structure
#define RtlInitializeParameterBlob(_ptr_blob_, _type_, _parameter_) {\
	RtlZeroMemory(_ptr_blob_, sizeof(SERVICE_PARAMETER_BLOB));\
	_ptr_blob_->ParameterMask = _type_;\
	_ptr_blob_->ulParameter = static_cast<ULONG64>(_parameter_);\
}
#define RtlInitializeParameterBlobPtr(_ptr_blob_, _type_, _parameter_, _buffer_size_) {\
	RtlZeroMemory(_ptr_blob_, sizeof(SERVICE_PARAMETER_BLOB));		\
	_ptr_blob_->ParameterMask = _type_;								\
	_ptr_blob_->ulParameter = reinterpret_cast<ULONG64>(_parameter_);\
	_ptr_blob_->dwMaxBufferSize = _buffer_size_;\
}
#define RtlInitializeServiceParameters(_service_parameter_, _number_of_parameters_) { \
	RtlZeroMemory(_service_parameter_, sizeof(DWORD) + sizeof(SERVICE_PARAMETER_BLOB) * _number_of_parameters_);\
	(_service_parameter_->dwParameters) = _number_of_parameters_;\
}

#define RtlServiceParametersValue(_service_parameter_, _index_, _type_, _value_) \
	RtlInitializeParameterBlob((_service_parameter_->Parameters + _index_), (_type_), (_value_))
#define RtlServiceParametersPtr(_service_parameter_, _index_, _type_, _value_, _max_buffer_size_) \
	RtlInitializeParameterBlobPtr((_service_parameter_->Parameters + _index_), (_type_), (_value_), (_max_buffer_size_))

#define LPC_MESSAGE_DATA_BUFFER(_lm_, type)\
	reinterpret_cast<type>(reinterpret_cast<size_t>(_lm_) + system_lpc_message_length)


#define PARAMETER_BUFFER_SIZE(_number_of_parameters_) (sizeof(DWORD64) + sizeof(SERVICE_PARAMETER_BLOB) * _number_of_parameters_)

//���ò����ṹ
//Service call parameter
typedef struct _SERVICE_PARAMETER_BLOB {
	BYTE ParameterMask;
	BYTE Reserved1 : 3;
	DWORD dwMaxBufferSize;		//Buffer length
	union {
		struct {
			DWORD dwParamHigh;
			DWORD dwParamLow;
		};
		ULONG64 ulParameter;
	};
	_SERVICE_PARAMETER_BLOB *operator=(_SERVICE_PARAMETER_BLOB __right);
	_SERVICE_PARAMETER_BLOB *operator=(_SERVICE_PARAMETER_BLOB *__right);
}SERVICE_PARAMETER_BLOB, *PSERVICE_PARAMETER_BLOB;
typedef struct _SERVICE_ROUTINE_PARAMETER {
	DWORD dwParameters;
	DWORD unused;
	SERVICE_PARAMETER_BLOB Parameters[0];
}SERVICE_ROUTINE_PARAMETER, *PSERVICE_ROUTINE_PARAMETER;

//ͨ�����ݰ��ṹ
//typedef struct _SERVICE_PACKAGE {
//	BYTE cbMajor;		//��Ҫ�汾��, 1
//	BYTE cbMinor;		//��Ҫ�汾��, 1
//	BYTE cbClientArch;	//�ͻ��˼ܹ�, x64=1 x86=0
//	BYTE cbPkgType;		//���ݰ�����, Request=0 Reply=1
//	DWORD dwSize;		//���ݰ��ܳ���
//
//	union {
//		struct {
//			NTSTATUS status; //�������̷��ص�״̬
//			DWORD unused;
//		};
//		PSERVICE_ROUTINE_PARAMETER ServiceParameters; //���ݸ��������̵Ĳ���
//		BYTE data[0];
//	};
//
//}SERVICE_PACKAGE, *PSERVICE_PACKAGE;

//�ڲ�����
//internal functions
PSERVICE_ROUTINE_PARAMETER RtlCaptureParameters(PSERVICE_ROUTINE_PARAMETER lpParameters);
PSERVICE_ROUTINE_PARAMETER RtlMapPtrParameters(HANDLE hProcess, PSERVICE_ROUTINE_PARAMETER lpParametersToMap);
void RtlUnmapPtrParameters(PSERVICE_ROUTINE_PARAMETER lpParametersToUnmap);
bool RtlSyncMappedPtrParameters(HANDLE hClientProcess, PSERVICE_ROUTINE_PARAMETER lpClientPara, PSERVICE_ROUTINE_PARAMETER lpMappedPara);
bool RtlEqualParameterType(PSERVICE_ROUTINE_PARAMETER p1, PSERVICE_ROUTINE_PARAMETER p2);
DWORD RtlSystemLpcMessageLength();
/*
 * return:
 *	STATUS_SUCCESS
		�ɹ�
		Function call succeeded
 *	STATUS_NOT_FOUND
		��ǰ�̲߳����ڷ�������
		The current thread does not belong to the service routine
 *	STATUS_NOT_SUPPORTED
		û�е��ó�ʼ������
		No initialization function called
*/
NTSTATUS RtlSafeThreadServiceClientId(PHANDLE hProcess, PDWORD lpdwProcessId);


//API ����

//ע���������ǰ�������, �ͻ��˲���Ҫ.
//Must be called before registering the service program, the client does not need.
/*
 * return:
 *  STATUS_SUCCESS
		�ɹ�
		Function call succeeded
*/
NTSTATUS BSAPI ScInitializeService();

//ע���������
//Registration service routine
/*
 * return:
 *	STATUS_SUCCESS
		�ɹ�
		Function call succeeded
 *	STATUS_ACCESS_VIOLATION
		�����а������ɷ��ʵ��ڴ��ַ
		The parameter contains an inaccessible memory address
 *	STATUS_INVALID_PARAMETER_3
		����3����
		Parameter 3 error
 *	STATUS_OBJECT_NAME_EXISTS
		�÷������������Ѵ���
		The service routine name already exists
 *	STATUS_NO_MEMORY
		�޷������µ��߳�
		Unable to create new thread
 *	STATUS_NOT_SUPPORTED
		û�е��ó�ʼ������
		No initialization function called
 * 
*/
NTSTATUS BSAPI ScRegisterServiceRoutineA(
	OUT PHANDLE hServiceRoutine,
	IN LPVOID lpServiceRoutine,
	IN PSERVICE_ROUTINE_PARAMETER lpParameters,
	IN LPCSTR lpRoutineName);

NTSTATUS BSAPI ScRegisterServiceRoutineW(
	OUT PHANDLE hServiceRoutine,
	IN LPVOID lpServiceRoutine,
	IN PSERVICE_ROUTINE_PARAMETER lpParameters,
	IN LPCWSTR lpRoutineName);
#define ScRegisterServiceRoutine ScRegisterServiceRoutineW


//ȡ��ע���������
//
/*
 * return:
 *	STATUS_SUCCESS
		�ɹ�
		Function call succeeded
 *	STATUS_ACCESS_VIOLATION
		�����а������ɷ��ʵ��ڴ��ַ
		The parameter contains an inaccessible memory address
 *	STATUS_INVALID_PARAMETER
		��������
		Parameter error
 *
*/
NTSTATUS BSAPI ScUnregisterServiceRoutine(HANDLE hServiceRoutine);



//�ͻ��˵��÷�������
//
/*
 * return:
 *	STATUS_SUCCESS
		�ɹ�
		Function call succeeded
 *	STATUS_ACCESS_VIOLATION
		�����а������ɷ��ʵ��ڴ��ַ
		The parameter contains an inaccessible memory address
 *	STATUS_INVALID_PARAMETER_1
		����1����
		Parameter 1 error
 *	STATUS_INVALID_PARAMETER_2
		����2����
		Parameter 2 error
 *	STATUS_UNSUCCESSFUL
		��������̽������Ӳ��ɹ�
		Connection to service routine was unsuccessful

 * lpServiceStatus: 
	�������̷��ص�״̬��(�û�����).
	Service returned status code(user define).
 *
*/
NTSTATUS BSAPI ScServiceCallA(
	IN LPCSTR lpServiceName,
	IN PSERVICE_ROUTINE_PARAMETER lpParameters,
	OUT PNTSTATUS lpServiceStatus OPTIONAL);

NTSTATUS BSAPI ScServiceCallW(
	IN LPCWSTR lpServiceName,
	IN PSERVICE_ROUTINE_PARAMETER lpParameters,
	OUT PNTSTATUS lpServiceStatus OPTIONAL);
#define ScServiceCall ScServiceCallW

//��ѯ���ÿͻ���id, ֻ���ɷ������̵���, ���̾�����ڷ������̷��غ�ʧЧ.
//Get the id of the calling client, which can only be called by the service routine,
//	and the process handle will be invalidated after the service routine returns.
//���̾���Բ�ѯ������Ϣ,���ƾ��,�����߳�,�����ڴ������Ȩ�޴�.
//The process handle is opened with the permission of
//	query process information, dup handle, create thread, virtual memory operation.
bool BSAPI RtlClientId(PHANDLE hClientProcess, PDWORD lpdwClientProcessId);
