#pragma once
#pragma warning(disable:4005)	//宏重定义
#pragma warning(disable:4200)	//零大小成员
#pragma warning(disable:4244)	//大到小丢失数据
#pragma warning(disable:4302)	//截断
#pragma warning(disable:4311)	//指针截断
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
 * VL: 参数传值, 与PT不兼容, 置位后将按照值尺寸复制参数到服务程序.
 * PT: 参数传指针, 与VL不兼容, 置位后将会映射指针到服务程序.
 * NL: 指针可以为空指针(nullptr), PT置位后有效, 指针为nullptr时不映射指针指向的内存到服务进程.
 *     如果PT置位而NL未置位, 且参数为nullptr时, 服务调用将失败, STATUS = STATUS_INVALID_PARAMETERS
 *
 * U:     未使用, 保留, 必须为0, 否则服务调用将失败, STATUS = STATUS_INVALID_PARAMETERS
 *
 * SIZE:  PT置位后忽略, 如有多个置位, 将按顺序从QW向后取第一个.
 * QW: QWORD 64位无符号整数
 * DW: DWORD 32位无符号整数
 * WO: WORD  16位无符号整数
 * BY: BYTE  8位无符号整数
 * ***浮点数只能传指针***
 *
*/

//Value
#define _QWORD			0b10001000	//服务进程为32位时等于DWORD
#define _DWORD			0b10000100
#define _WORD			0b10000010
#define _BYTE			0b10000001

//Ptr
#define _PTR			0b01000000
#define _PTR_NO_MAP		0b01100000

//初始化结构
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

//调用参数结构
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

//通信数据包结构
//typedef struct _SERVICE_PACKAGE {
//	BYTE cbMajor;		//主要版本号, 1
//	BYTE cbMinor;		//次要版本号, 1
//	BYTE cbClientArch;	//客户端架构, x64=1 x86=0
//	BYTE cbPkgType;		//数据包类型, Request=0 Reply=1
//	DWORD dwSize;		//数据包总长度
//
//	union {
//		struct {
//			NTSTATUS status; //服务例程返回的状态
//			DWORD unused;
//		};
//		PSERVICE_ROUTINE_PARAMETER ServiceParameters; //传递给服务例程的参数
//		BYTE data[0];
//	};
//
//}SERVICE_PACKAGE, *PSERVICE_PACKAGE;

//内部函数
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
		成功
		Function call succeeded
 *	STATUS_NOT_FOUND
		当前线程不属于服务例程
		The current thread does not belong to the service routine
 *	STATUS_NOT_SUPPORTED
		没有调用初始化函数
		No initialization function called
*/
NTSTATUS RtlSafeThreadServiceClientId(PHANDLE hProcess, PDWORD lpdwProcessId);


//API 函数

//注册服务例程前必须调用, 客户端不需要.
//Must be called before registering the service program, the client does not need.
/*
 * return:
 *  STATUS_SUCCESS
		成功
		Function call succeeded
*/
NTSTATUS BSAPI ScInitializeService();

//注册服务例程
//Registration service routine
/*
 * return:
 *	STATUS_SUCCESS
		成功
		Function call succeeded
 *	STATUS_ACCESS_VIOLATION
		参数中包含不可访问的内存地址
		The parameter contains an inaccessible memory address
 *	STATUS_INVALID_PARAMETER_3
		参数3错误
		Parameter 3 error
 *	STATUS_OBJECT_NAME_EXISTS
		该服务例程名称已存在
		The service routine name already exists
 *	STATUS_NO_MEMORY
		无法创建新的线程
		Unable to create new thread
 *	STATUS_NOT_SUPPORTED
		没有调用初始化函数
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


//取消注册服务例程
//
/*
 * return:
 *	STATUS_SUCCESS
		成功
		Function call succeeded
 *	STATUS_ACCESS_VIOLATION
		参数中包含不可访问的内存地址
		The parameter contains an inaccessible memory address
 *	STATUS_INVALID_PARAMETER
		参数错误
		Parameter error
 *
*/
NTSTATUS BSAPI ScUnregisterServiceRoutine(HANDLE hServiceRoutine);



//客户端调用服务例程
//
/*
 * return:
 *	STATUS_SUCCESS
		成功
		Function call succeeded
 *	STATUS_ACCESS_VIOLATION
		参数中包含不可访问的内存地址
		The parameter contains an inaccessible memory address
 *	STATUS_INVALID_PARAMETER_1
		参数1错误
		Parameter 1 error
 *	STATUS_INVALID_PARAMETER_2
		参数2错误
		Parameter 2 error
 *	STATUS_UNSUCCESSFUL
		与服务例程建立连接不成功
		Connection to service routine was unsuccessful

 * lpServiceStatus: 
	服务例程返回的状态码(用户定义).
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

//查询调用客户端id, 只能由服务例程调用, 进程句柄将在服务例程返回后失效.
//Get the id of the calling client, which can only be called by the service routine,
//	and the process handle will be invalidated after the service routine returns.
//进程句柄以查询进程信息,复制句柄,创建线程,虚拟内存操作的权限打开.
//The process handle is opened with the permission of
//	query process information, dup handle, create thread, virtual memory operation.
bool BSAPI RtlClientId(PHANDLE hClientProcess, PDWORD lpdwClientProcessId);
