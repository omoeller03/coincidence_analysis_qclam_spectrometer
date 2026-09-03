#include "source/TQClamTcpSource.h"

#include "TGo4Log.h"
#include "TGo4EventErrorException.h"
#include "TGo4EventEndException.h"
#include "TGo4UserSourceParameter.h"

#include "evt/TQClamTcpEvent.h"

#include "TClass.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>



ClassImp(TQClamTcpSource)

TQClamTcpSource::TQClamTcpSource(const char* name, const char* host, Int_t port)
	: TGo4EventSource(name),
	fxHost(host),
	fiPort(port),
	fSocket(-1),
	fbIsOpen(kFALSE)
{
	Open();
}



TQClamTcpSource::TQClamTcpSource(TGo4UserSourceParameter* par)
	: TGo4EventSource("TcpSource"),
	fSocket(-1),
	fbIsOpen(kFALSE)
{
	if(par)
	{
		fxHost = par->GetExpression();
		fiPort = par->GetPort();
		Open();
	}
}


Int_t TQClamTcpSource::Open()
{
	if(fbIsOpen) { return 0; }

	fSocket = socket(AF_INET,SOCK_STREAM,0);
	if(fSocket < 0)
	{
		TGo4Log::Error("[TQClamTcpSource] Socket wurde nicht erstellt!");
		throw TGo4EventErrorException(this);
	}


	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(fiPort);
	inet_pton(AF_INET,fxHost.c_str(),&addr.sin_addr);

	if(connect(fSocket,(sockaddr*)&addr,sizeof(addr))<0)
	{
		TGo4Log::Error("[TQClamTcpSource] Verbindung zu %s:%d fehlgeschlagen!",fxHost.c_str(),fiPort);
		throw TGo4EventErrorException(this);
	}


	int flags = fcntl(fSocket, F_GETFL, 0);
	fcntl(fSocket, F_SETFL, flags | O_NONBLOCK);

	fbIsOpen = kTRUE;
	TGo4Log::Info("[TQClamTcpSource] Verbindung hergestelltzu %s:%d.", fxHost.c_str(),fiPort);
	return 0;
}


Int_t TQClamTcpSource::Close()
{
	if(fSocket >= 0) { close(fSocket); }

	fSocket = -1;
	fbIsOpen = kFALSE;
	return 0;
}


Bool_t TQClamTcpSource::CheckEventClass(TClass* cl)
{
	return cl->InheritsFrom(TQClamTcpEvent::Class());
}


Bool_t TQClamTcpSource::ReadLine(std::string& out)
{
	out.clear();
	char c;
	while(true)//read(fSocket, &c, 1) == 1)
	{
		printf("readline: %d",c);
        	int n = read(fSocket, &c, 1);
		if(n==1)
		{
			// byte empfangen
			TGo4Log::Info("[TcpSource] Byte empfangen: %d '%c'", (int)c, c);

			if(c == '\n')
			{
				// komplette zeile fertig
				out = fBuffer;
				fBuffer.clear();
				return kTRUE;
			}

			fBuffer.push_back(c);
		}

		else if(n == -1 && errno == EWOULDBLOCK)
		{
			// keine neuen Daten
			return kFALSE;
		}

		else
		{
			// Verbindung beendet
			throw TGo4EventEndException(this);
		}
	}

	return kFALSE;
}

Bool_t TQClamTcpSource::BuildEvent(TGo4EventElement* dest)
{
	auto* evt = dynamic_cast<TQClamTcpEvent*>(dest);


	if(!evt)
	{
		printf("TcpSource: no evt");
		return kFALSE;
	}

	std::string line;
	if(!ReadLine(line)) { return kFALSE;}//throw TGo4EventEndException(this); }

	sscanf(line.c_str(), "%lf,%lf", &evt->fEntonRate, &evt->fSzintScattChamberRate);
	printf("line: %s",line.c_str());
	return kTRUE;
}
