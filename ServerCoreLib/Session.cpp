#include "ServerCorePch.h"
#include "Session.h"
#include "OverlappedEx.h"
#include "SocketUtils.h"
#include "Service.h"
#include "SendBuffer.h"

const int32 BUFFERSIZE = 0x10000;

Session::Session()
	: m_recvBuffer(BUFFERSIZE)
{
	m_socket = SocketUtils::CreateDefaultSocket();
}

Session::~Session()
{
	SocketUtils::CloseSocket(m_socket);
}

void Session::Send(RefSendBuffer refSendbuffer)
{
	// 이곳의 개선점?
	// 1. 무조건 LOCK을 잡아야해
	// 2. LOCK 잡는 시간이 길어져.

	if (false == IsConnected())
		return;

	bool registerSend = false;

	{
		WRITE_LOCK;

		m_sendBufferQueue.push(refSendbuffer);

		if (false == m_sendRegistered.exchange(true))
			registerSend = true;
	}

	if (registerSend)
		RegisterSend();
}

bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	// 이미 연결해제요청된 상황
	if (false == m_Connected.exchange(false))
		return;

	std::wcout << "Disconnect : " << cause << std::endl;

	// SocketUtils::CloseSocket(m_socket); 소켓의 재사용을 할 수가 없다.
	RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(m_socket);
}

void Session::Dispatch(OverlappedEx* pOverlappedEx, int32 numofBytes)
{
	switch (pOverlappedEx->m_EventType)
	{
	case NetworkEventType::Send:
		ProcessSend(numofBytes);
		break;
	case NetworkEventType::Connect:
		ProcessConnect();
		break;
	case NetworkEventType::Accept:
		break;
	case NetworkEventType::Recv:
		ProcessRecv(numofBytes);
		break;
	case NetworkEventType::Disconnect:
		ProcessDisconnect();
		break;
	}
}

bool Session::RegisterConnect()
{
	// 이미 연결된 상태
	if (IsConnected())
		return false;

	if (ServiceType::Client != GetService()->GetServiceType())
		return false;

	if (false == SocketUtils::SetReuseAddress(m_socket, true))
		return false;

	if (false == SocketUtils::BindAnyAddress(m_socket, 0))
		return false;

	m_overlappedConnect.Init();
	m_overlappedConnect.m_owner = shared_from_this(); // refcount 증가

	DWORD numofBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();

	if (false == SocketUtils::ConnectEx(m_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr),
		nullptr, 0, &numofBytes, &m_overlappedConnect))
	{
		int32 error = ::WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			m_overlappedConnect.m_owner = nullptr;
			return false;
		}
	}

	return true;
}

bool Session::RegisterDisconnect()
{
	m_overlappedDisconnect.Init();
	m_overlappedDisconnect.m_owner = shared_from_this();

	DWORD flags = TF_REUSE_SOCKET;

	if (false == SocketUtils::DisconnectEx(m_socket, &m_overlappedDisconnect, flags, 0))
	{
		int32 error = ::WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			m_overlappedDisconnect.m_owner = nullptr;
			return false;
		}
	}
	return true;
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	m_overlappedRecv.Init();
	m_overlappedRecv.m_owner = shared_from_this(); // Session의 refCount를 1늘린다.

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(m_recvBuffer.WritePos());
	wsaBuf.len = m_recvBuffer.FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	// recvBuffer와 recv를 하는 주체는 하나만 있어도 된다.
	if (SOCKET_ERROR == ::WSARecv(m_socket, &wsaBuf, 1, OUT & numOfBytes, OUT & flags, &m_overlappedRecv, nullptr))
	{
		int32 error = ::WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			HandleError(error);
			m_overlappedRecv.m_owner = nullptr; // release
		}
	}

}

void Session::RegisterSend()
{
	if (false == IsConnected())
		return;

	m_overlappedSend.Init();
	m_overlappedSend.m_owner = shared_from_this(); // Session의 refCount를 1늘린다.

	{
		WRITE_LOCK; // sendBufferQueue에 대한 동기화

		int32 writeSize = 0;
		while (false == m_sendBufferQueue.empty())
		{
			RefSendBuffer sendBuffer = m_sendBufferQueue.front();
			int32 bufferSize = sendBuffer->WriteSize();

			if (writeSize + bufferSize > MaxSendSize)
				break;

			writeSize += bufferSize;

			m_sendBufferQueue.pop();
			m_overlappedSend.m_sendBuffers.push_back(sendBuffer);
		}
	}

	exvector<WSABUF> wsaBufs;
	wsaBufs.reserve(m_overlappedSend.m_sendBuffers.size());
	for (RefSendBuffer refSendBuffer : m_overlappedSend.m_sendBuffers)
	{
		WSABUF wsabuf; // Queue의 데이터를 한번에
		wsabuf.len = static_cast<ULONG>(refSendBuffer->WriteSize());
		wsabuf.buf = reinterpret_cast<CHAR*>(refSendBuffer->Buffer());
		wsaBufs.push_back(wsabuf);
	}

	DWORD numOfBytes = 0;
	// NO THREAD SAFE
	if (SOCKET_ERROR == ::WSASend(m_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, 0, &m_overlappedSend, nullptr))
	{
		int32 error = ::WSAGetLastError();
		if (WSA_IO_PENDING != error)
		{
			HandleError(error);
			m_overlappedSend.m_owner = nullptr;
			m_overlappedSend.m_sendBuffers.clear();

			m_sendRegistered.store(false);
		}
	}
}

void Session::ProcessConnect()
{
	m_overlappedConnect.m_owner = nullptr;

	m_Connected.store(true);

	GetService()->AddSession(GetSession());

	OnConnected(); // 각 Session마다 다른 연결처리

	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	m_overlappedDisconnect.m_owner = nullptr;

	// Session마다 재정의된 코드
	OnDisconncted();
	//Service에 등록되어 있는 나를 삭제, ref count 감소.
	GetService()->ReleaseSession(GetSession());

	m_Connected.store(false);
}

void Session::ProcessRecv(int32 numofBytes)
{
	m_overlappedRecv.m_owner = nullptr; // release

	if (0 == numofBytes)
	{
		Disconnect(L"Recv 0 Bytes");
		return;
	}

	if (false == m_recvBuffer.OnWrite(numofBytes))
	{
		Disconnect(L"RecvBuffer Write OverFlow");
		return;
	}

	int32 dataSize = m_recvBuffer.DataSize();
	// 각 Session에서 정의
	int32 processLen = OnRecv(m_recvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || false == m_recvBuffer.OnRead(processLen))
	{
		Disconnect(L"RecvBuffer Read OverFlow");
		return;
	}

	m_recvBuffer.Clear();

	RegisterRecv();
}

void Session::ProcessSend(int32 numofBytes)
{
	m_overlappedSend.m_owner = nullptr; // release
	m_overlappedSend.m_sendBuffers.clear();

	if (0 == numofBytes)
	{
		Disconnect(L"Send 0 Bytes");
		return;
	}

	// 각 Session에서 정의
	OnSend(numofBytes);

	WRITE_LOCK;

	if (m_sendBufferQueue.empty())
		m_sendRegistered.store(false);
	else // 누가 또 등록을 했다.
		RegisterSend();
}

void Session::HandleError(int32 errorCode)
{
	// TODO : log를 찍는 전문 쓰레드.
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"error");
		break;
	default:
		std::cout << "Handle error : " << errorCode << std::endl;
	}
}

PacketSession::PacketSession()
{
}

PacketSession::~PacketSession()
{
}

int32 PacketSession::OnRecv(BYTE* pBuffer, int32 len)
{
	int32 processLen = 0;
	while (true)
	{
		int32 dataSize = len - processLen;

		if (dataSize < sizeof(PacketHeader))
			break;

		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&pBuffer[processLen]));

		if (dataSize < header.size)
			break;

		// 패킷 조립
		OnRecvPacket(&pBuffer[processLen], header.size);

		processLen += header.size;
	}

	return processLen;
}
