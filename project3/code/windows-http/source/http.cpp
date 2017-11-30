#include "http_handler.h"
//***********************************************
using namespace std;
//***********************************************
#define WM_SOCKET_NOTIFY (WM_USER + 1)
//***********************************************
static BOOL CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);
//***********************************************
static void startHTTP(HWND hwnd, const char* listen_ip, unsigned short listen_port, LPCTSTR working_dir);
static void main_socket_accept(HWND hwnd);
static DWORD WINAPI handle_client(LPVOID param);
static void close_main_socket();
//***********************************************
static HWND hwnd_working_dir = NULL, hwnd_listen_port = NULL, hwnd_listen_btn = NULL, hwnd_edit_console = NULL;
//***********************************************
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	initial_http_handler();

	return DialogBox(hInstance, MAKEINTRESOURCE(ID_MAIN), NULL, MainDlgProc);
}
//***********************************************
static BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	static SOCKET msock = SOCKET_ERROR;

	switch(Message)
	{
	case WM_INITDIALOG:
		windows_env::hwnd_edit_console = GetDlgItem(hwnd, IDC_RESULT);
		hwnd_working_dir = GetDlgItem(hwnd, IDC_EDIT_WORKING_DIR);
		hwnd_listen_port = GetDlgItem(hwnd, IDC_EDIT_LISTEN_PORT);
		hwnd_listen_btn = GetDlgItem(hwnd, ID_LISTEN);
		hwnd_edit_console = GetDlgItem(hwnd, IDC_RESULT);
		
		WSADATA wsaData;
		
		if(WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
			error_handle("Fail to init WSA", true);
		
		close_main_socket();
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_LISTEN:
			if(windows_env::main_socket != INVALID_SOCKET)
				close_main_socket();
			else
			{
				CHAR working_dir[BUFFER + 1];
				CHAR port_string[BUFFER + 1];
				UINT working_dir_len = GetDlgItemText(hwnd, IDC_EDIT_WORKING_DIR, working_dir, sizeof(working_dir)/sizeof(working_dir[0]));  
				UINT port_len = GetDlgItemText(hwnd, IDC_EDIT_LISTEN_PORT, port_string, sizeof(port_string)/sizeof(port_string[0]));  
				unsigned short port = port_len != 0 ? atoi(port_string) : HTTP_PORT;
				startHTTP(hwnd, NULL, port, working_dir_len > 0 ? working_dir : NULL);
			}
			break;
		case ID_EXIT:
			EndDialog(hwnd, 0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		WSACleanup();
		break;
	case WM_SOCKET_NOTIFY:
		switch( WSAGETSELECTEVENT(lParam) )
		{
			case FD_ACCEPT:
				main_socket_accept(hwnd);
				break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
//***********************************************
static void startHTTP(HWND hwnd, const char* listen_ip, unsigned short listen_port, LPCTSTR working_dir)
{
	debug_handle("=== Server START ===\n");
	close_main_socket();

	if(working_dir != NULL && !SetCurrentDirectory(working_dir))
		error_handle("Fail to set the working directory\n", true);

	CHAR current_dir[BUFFER + 1];

	GetCurrentDirectory(sizeof(current_dir)/sizeof(current_dir[0]), current_dir);

	debug_handle("Working directory: %s\n", current_dir);
	debug_handle("Listen port: %hu\n", listen_port);

	windows_env::main_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if( windows_env::main_socket == INVALID_SOCKET )
		error_handle("Fail to create socket\n", true);

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port	 = htons(listen_port);
	sa.sin_addr.s_addr	= INADDR_ANY;

	if(bind(windows_env::main_socket, (const struct sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
		error_handle("Fail to bind the socket", true);

	if(listen(windows_env::main_socket, 50) == SOCKET_ERROR)
		error_handle("Fail to change the socket to listen socket", true);

	if(WSAAsyncSelect(windows_env::main_socket, hwnd, WM_SOCKET_NOTIFY, FD_ACCEPT) != 0)
		error_handle("Fail to select on socket", true);

	Button_SetText(hwnd_listen_btn, "Close");
}
//***********************************************
static void main_socket_accept(HWND hwnd)
{
	auto_ptr<SocketInfo> sockInfo(new SocketInfo());
	sockInfo->saLen = sizeof(sockInfo->sa);
	sockInfo->socket = AutoSocket(accept(windows_env::main_socket, (struct sockaddr*)&sockInfo->sa, &sockInfo->saLen));

	if(sockInfo->socket == INVALID_SOCKET)
		error_handle("Fail to accept socket");

	// Set to blocking mode
	if(WSAAsyncSelect(sockInfo->socket, hwnd, WM_SOCKET_NOTIFY, 0) != 0)
	{
		int error = WSAGetLastError();
		
		error_handle("Fail to disable async select on socket");
	}
	
	unsigned long nonblocking = 0;
	
	if(ioctlsocket(sockInfo->socket, FIONBIO, &nonblocking) != 0)
	{
		int error = WSAGetLastError();
		
		error_handle("Fail to set the socket to blocking mode");
	}
	
	char addrBuf[50];
	unsigned short port;
	
	if(sockInfo->sa.ss_family == AF_INET)
	{
		struct sockaddr_in *saInet = (struct sockaddr_in *)&sockInfo->sa;
		
		if(InetNtop(saInet->sin_family, &saInet->sin_addr, addrBuf, sizeof(addrBuf)) == NULL)
			error_handle("Fail to convert the client address into representation");
		
		port = ntohs(saInet->sin_port);
	}
	else
		error_handle("Receive socket with unsupported address family");
	
	debug_handle("=== Receive new connection from %s:%hu\n", addrBuf, port);

	DWORD threadId;
	HANDLE thread = CreateThread(NULL, 0, handle_client, sockInfo.get(), 0, &threadId);
	
	if(thread == NULL)
		error_handle("Fail to create thread", false);
	
	sockInfo.release();

	CloseHandle(thread);
}
//***********************************************
static DWORD WINAPI handle_client(LPVOID param)
{
	auto_ptr<SocketInfo> sockInfo((SocketInfo*)param);
	
	HTTP_HANDLER http_handler(sockInfo->socket);

	if(!http_handler.request())
		return 0;

	http_handler.analysis();
	http_handler.response();

	return 0;
}
//***********************************************
static void close_main_socket()
{
	Button_SetText(hwnd_listen_btn, "Listen");
	
	if(windows_env::main_socket == INVALID_SOCKET)
		return;
	
	closesocket(windows_env::main_socket);
	
	windows_env::main_socket = INVALID_SOCKET;
	
	debug_handle("=== Server STOP ===\n");
}
//***********************************************
