# ServiceCall
Interprocess communication library, providing the ability to call functions from each other.</br>
进程间通信库,提供相互调用函数的能力.

[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![HitCount](http://hits.dwyl.io/bb107/ServiceCall.svg)](http://hits.dwyl.io/bb107/ServiceCall)

## Usage (用法)
**提示:详细定义请参阅ServiceCall.h**</br>
**Tip: For detailed definitions, please refer to ServiceCall.h**
### 1.Initialize the service process  (初始化服务进程):
```
//注册服务例程前必须调用, 客户端不需要.
//Must be called before registering the service program, the client does not need.
/*
 * return:
 *  STATUS_SUCCESS
		成功
		Function call succeeded
*/
NTSTATUS BSAPI ScInitializeService();
```
### 2.Register a service routine that allows external calls (注册一个允许外部调用的服务例程):
**32位版本服务例程必须声明为__stdcall**</br>
**32-bit version service routines must be declared as __stdcall**
```
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
```
### 3.Unregister the service routine  (取消注册服务例程)
**注意:成功调用后客户端的调用请求将立即失败(STATUS_UNSUCCESSFUL),服务例程立即停止,不会等待相关调用结束.**</br>
**Note: The client's call request will fail(STATUS_UNSUCCESSFUL) immediately after a successful call, the service routine will stop immediately, and will not wait for the relevant call to end.**
```
//取消注册服务例程
//Unregister the service routine
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
```
### 4.Client call service routine (客户端调用服务例程):
指针参数将会根据自动映射到服务例程的进程空间中, 服务例程返回后会同步到客户端.</br>
The pointer parameters will be automatically mapped to the process space of the service routine, and will be synchronized to the client after the service routine returns.
* 指针中如果包含多级指针,需要使用WINDOWS提供的虚拟内存操作API读取和写入客户端进程,RtlClientId()函数可以获得客户端的进程句柄和进程id.
* If the pointer contains multiple levels of pointers, you need to use the virtual memory operation API provided by WINDOWS to read and write to the client process. The RtlClientId() function can get the client's process handle and process id.
```
//客户端调用服务例程
//Client call service routine
/*
 * return:
 *	STATUS_SUCCESS
		成功
		Function call succeeded
 *	STATUS_ACCESS_VIOLATION
		参数中包含不可访问的内存地址
		The parameter contains an inaccessible memory address
 *	STATUS_INVALID_PARAMETER_1
		参数1错误(服务例程不存在)
		Parameter 1 error(Service routine does not exist)
 *	STATUS_INVALID_PARAMETER_2
		参数2错误
		Parameter 2 error
 *	STATUS_UNSUCCESSFUL
		与服务例程建立连接不成功,或者服务例程主动断开了连接
		Connection to service routine was unsuccessful, or the service routine actively disconnected

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
```
### 5.Query the ID of the calling client  (查询调用客户端id)
```
//查询调用客户端id, 只能由服务例程调用, 进程句柄将在服务例程返回后失效.
//Get the id of the calling client, which can only be called by the service routine,
//	and the process handle will be invalidated after the service routine returns.
//进程句柄以查询进程信息,复制句柄,创建线程,虚拟内存操作的权限打开.
//The process handle is opened with the permission of
//	query process information, dup handle, create thread, virtual memory operation.
bool BSAPI RtlClientId(PHANDLE hClientProcess, PDWORD lpdwClientProcessId);
```
## Test screenshots	(测试截图)
![alt text](screenshots/test.png?raw=true "test1")
