#include "rpc.h"
#include "tray_h.h"

static HANDLE g_hStopEvent = NULL;

extern "C" boolean StopService(handle_t hBinding) {
    (void)hBinding;
    if (g_hStopEvent) {
        SetEvent(g_hStopEvent);
        return TRUE;
    }
    return FALSE;
}

void StartRpcServer(HANDLE hStopEvent) {
    g_hStopEvent = hStopEvent;

    RPC_STATUS status = RpcServerUseProtseqEpW(
        (RPC_WSTR)L"ncalrpc",
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
        (RPC_WSTR)L"TrayRpcEndpoint",
        NULL
    );
    if (status != RPC_S_OK) return;

    status = RpcServerRegisterIf2(
        TrayRpc_v1_0_s_ifspec,
        NULL, NULL,
        RPC_IF_ALLOW_LOCAL_ONLY,
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
        0, NULL
    );
    if (status != RPC_S_OK) return;

    RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, TRUE);
}

void StopRpcServer() {
    RpcMgmtStopServerListening(NULL);
}

// Включаем MIDL-серверный стаб напрямую (обёртка extern "C" сохраняет C-искажение имён)
extern "C" {
#include "C:/Users/Student/Desktop/TrayApp/build/tray_s.c"
}

// Обязательные функции RPC runtime для маршалинга памяти
extern "C" void* __RPC_USER MIDL_user_allocate(size_t size) {
    return malloc(size);
}

extern "C" void __RPC_USER MIDL_user_free(void* ptr) {
    free(ptr);
}