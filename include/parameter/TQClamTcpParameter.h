#ifndef TQCLAMTCPPARAMETER_H
#define TQCLAMTCPPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"


class TQClamTcpParameter : public TGo4Parameter
{
	public:
		TQClamTcpParameter();
		TQClamTcpParameter(const char* name);

		Bool_t fbOpenTcpConnection; // soll Tcp-Verbindung geoeffnet werden?
		Bool_t fbPauseTcpConnection; // pausiere das Einlesen der Werte aus der Tcp-Verbindung

		TString fxTcpHost; // HOST PC --> '192.168.2.8' (experiment@spectrometercontrol)
		Int_t fiTcpPort; // PORT --> i.d.R. 5050, 5000 wird meist von Go4 selbst verwendet

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;


	ClassDefOverride(TQClamTcpParameter,1);
};


#endif
