#include "parameter/TQClamTcpParameter.h"

#include "TGo4Log.h"


/**
 * Diese Klasse habe ich geschrieben, um via Tcp an EPICS Daten zu kommen, die lediglich von einem anderen PC erreicht werden.
 * Gedacht ist, die Enton-Rate von 'experiment@spectrometercontrol' via tcp-Verbindung einzulesen,
 * um mit den LaBr-Raten relative Daten zu bestimmen.
 * Die Tcp-Verbindung ist dann in TDagobertHitProc implementiert.
 */

ClassImp(TQClamTcpParameter)

TQClamTcpParameter::TQClamTcpParameter()
	: TGo4Parameter("TcpParam")
{}


TQClamTcpParameter::TQClamTcpParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamTcpParameter] Create %s.",name);
}



Bool_t TQClamTcpParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamTcpParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamTcpParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamTcpParameter] UpdateFrom nicht erfolgreich!");
		return kFALSE;
	}

	fbOpenTcpConnection = other->fbOpenTcpConnection;
	fbPauseTcpConnection = other->fbPauseTcpConnection;
	fxTcpHost = other->fxTcpHost;
	fiTcpPort = other->fiTcpPort;


	return kTRUE;
}
