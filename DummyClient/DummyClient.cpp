﻿#include "pch.h"
#include <iostream>
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"
#include "ServerPacketHandler.h"

char sendData[] = "Hello World";

class ServerSession : public PacketSession
{
public:
    ~ServerSession()
    {
        cout << "~ServerSession" << endl;
    }
    virtual void OnConnected() override
    {
        Protocol::C_LOGIN pkt;
        auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
        Send(sendBuffer);
    }
    virtual void OnDisconnected() override
    {
        // << "DisConneted" << endl;
    }
    virtual void OnRecvPacket(BYTE* buffer, int32 len) override
    {
        PacketSessionRef session = GetPacketSessionRef();
        PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

        ServerPacketHandler::HandlePacket(session, buffer, len);
    }

    virtual void OnSend(int32 len) override
    {
        //cout << "OnSend Len = " << len << endl;
    }
};

int main()
{
    ServerPacketHandler::Init();

    ClientServiceRef service = MakeShared<ClientService>(
        NetAddress(L"127.0.0.1", 7777),
        MakeShared<IocpCore>(),
        MakeShared<ServerSession>,
        1);

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 2; i++)
    {
        GThreadManager->Launch([=]()
            {
                while (true)
                {
                    service->GetIocpCore()->Dispatch();
                }
            });
    }

    Protocol::C_CHAT chatPkt;
    
    

    while (true)
    {
        string text;
        getline(cin, text);
        chatPkt.set_msg(text);

        auto sendBuffer = ServerPacketHandler::MakeSendBuffer(chatPkt);
        service->Broadcast(sendBuffer);
        this_thread::sleep_for(1s);
    }

    GThreadManager->Join();
}