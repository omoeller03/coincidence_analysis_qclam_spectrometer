#ifndef TDAGOBERTUNPACKPROC_H
#define TDAGOBERTUNPACKPROC_H

#include "TGo4EventProcessor.h"
#include <vector>
#include <memory>
#include <fstream>

class TDagobertRawEvent;

class TDagobertUnpackProc : public TGo4EventProcessor
{
        public:
                TDagobertUnpackProc();
                TDagobertUnpackProc(const char* name);
                virtual ~TDagobertUnpackProc();

                Bool_t BuildEvent(TGo4EventElement* dest) override;
                Bool_t BuildEventNew(TGo4EventElement* dest);
                Bool_t BuildEvent2(TGo4EventElement* dest);


        private:

                bool firstZeroSeen[19]{false};

		std::ofstream fDebugFile;






        //ClassDefOverride(TDagobertUnpackProc,1)
};

#endif

