#ifndef TQCLAMTCPSOURCE_H
#define TQCLAMTCPSOURCE_H

#include "TGo4EventSource.h"
#include "TGo4UserSourceParameter.h"
#include <string>

class TQClamTcpSource : public TGo4EventSource
{
	public:
		TQClamTcpSource(const char* name, const char* host, Int_t port);
		TQClamTcpSource(TGo4UserSourceParameter* par);
		
		virtual ~TQClamTcpSource() {}

		Bool_t CheckEventClass(TClass* cl) override;
		Bool_t BuildEvent(TGo4EventElement* dest) override;

		virtual Int_t Open();
		virtual Int_t Close();


	private:
		std::string fxHost;
		Int_t fiPort;
		Int_t fSocket;
		std::string fBuffer;

		Bool_t fbIsOpen;

		Bool_t ReadLine(std::string& out);

	ClassDefOverride(TQClamTcpSource,1);
};


#endif
