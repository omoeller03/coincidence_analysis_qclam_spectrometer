/*
#include "source/TQClamOfflineLmdSource.h"

#include "readoutmarkers.h"

#include "TGo4Log.h"
#include "TGo4RuntimeException.h"
#include "TGo4EventErrorException.h"

#include "TClass.h"

#include "evt/TQClamRawEvent.h"

#include "parameter/TQClamSourceParameter.h"
*/

#include "source/TQClamOfflineLmdSource.h"

#include "TGo4Log.h"
#include "TGo4RuntimeException.h"
#include "TGo4EventErrorException.h"
#include "TClass.h"

#include "evt/TQClamRawEvent.h"
#include "parameter/TQClamSourceParameter.h"

#include "readoutmarkers.h"

#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>

extern "C" {
  #include "s_filhe.h"
  #include "s_filhe_swap.h"
  #include "s_bufhe_swap.h"
  #include "s_ves10_1.h"
  #include "s_ve10_1_swap.h"
  #include "s_evhe_swap.h"
//  #include "fLmd.h"
  #include "f_evt.h"
}




ClassImp(TQClamOfflineLmdSource)

//----------------------------------------------------------
// ctor / dtor
//----------------------------------------------------------

TQClamOfflineLmdSource::TQClamOfflineLmdSource()
  : TGo4EventSource("TQClamOfflineLmdSource")
{
  TGo4Log::Info("[TQClamOfflineLmdSource] default ctor");
}

TQClamOfflineLmdSource::TQClamOfflineLmdSource(TGo4UserSourceParameter* sourcepar)
  : TGo4EventSource("TQClamOfflineLmdSource")
{
  TGo4Log::Info("[TQClamOfflineLmdSource] ctor, par=%p", sourcepar);
  auto* par = dynamic_cast<TQClamSourceParameter*>(sourcepar);

  if(!par) {
    TGo4Log::Error("[TQClamOfflineLmdSource] Falscher TQClamSourceParameter Typ!");
    throw TGo4RuntimeException();
  }

  fFilename = "";//par->fLmdFilename;
  TGo4Log::Info("[TQClamOfflineLmdSource] ctor: fFilename='%s'", fFilename.c_str());

  Open();
}

TQClamOfflineLmdSource::~TQClamOfflineLmdSource()
{
  TGo4Log::Info("[TQClamOfflineLmdSource] dtor");
  Close();
}

//----------------------------------------------------------
// Open / Close
//----------------------------------------------------------



Int_t TQClamOfflineLmdSource::Open()
{
    if(fChannel) return 0;

    TGo4Log::Info("[TQClamOfflineLmdSource::Open] open '%s'", fFilename.c_str());

    auto* chan = f_evt_control();
    if(!chan) {
        TGo4Log::Error("[TQClamOfflineLmdSource::Open] f_evt_control() failed");
        return 1;
    }

    s_filhe* fileHeader = nullptr;   // <-- so nennen
    INTS4 res = f_evt_get_open(GETEVT__FILE,
                               (char*)fFilename.c_str(),
                               chan,
                               (CHARS**)&fileHeader,
                               1, 0);
    if(res != GETEVT__SUCCESS) {
        TGo4Log::Error("[TQClamOfflineLmdSource::Open] f_evt_get_open failed, res=%d", res);
        return 1;
    }

    fChannel    = chan;
    fFileHeader = fileHeader;
    fBufHeader  = nullptr;
    fEventNumber = 0;

    if(fileHeader) {
        TGo4Log::Info("[TQClamOfflineLmdSource::Open] filhe_dlen=%d file='%s' user='%s'",
                      fileHeader->filhe_dlen,
                      fileHeader->filhe_file,
                      fileHeader->filhe_user);
    }

    return 0;
}






Int_t TQClamOfflineLmdSource::Close()
{
    auto* chan = static_cast<s_evt_channel*>(fChannel);
    if(chan) {
        TGo4Log::Info("[TQClamOfflineLmdSource::Close] closing");
        f_evt_get_close(chan);
    }

    fChannel    = nullptr;
    fFileHeader = nullptr;
    fBufHeader  = nullptr;
    return 0;
}





//----------------------------------------------------------
// BuildEvent
//----------------------------------------------------------




Bool_t TQClamOfflineLmdSource::BuildEvent(TGo4EventElement* dest)
{
    auto* evt = dynamic_cast<TQClamRawEvent*>(dest);
    if(!evt) return kFALSE;

    auto* chan  = static_cast<s_evt_channel*>(fChannel);
    auto*& bufh = *reinterpret_cast<s_bufhe**>(&fBufHeader);
    if(!chan) return kFALSE;

    evt->Clear();

    INTS4* evData = nullptr;
    INTS4  res    = f_evt_get_event(chan, &evData, (INTS4**)&bufh);

    if(res == GETEVT__NOMORE)
        return kFALSE;
    if(res != GETEVT__SUCCESS)
        throw TGo4EventErrorException(this);

    if(!evData || !bufh)
        throw TGo4EventErrorException(this);

    // kleine lokale Swap-Helfer
    auto swap16 = [](uint16_t x) {
        return (uint16_t)((x >> 8) | (x << 8));
    };
    auto swap32 = [](uint32_t x) {
        return ((x >> 24) & 0x000000FF) |
               ((x >>  8) & 0x0000FF00) |
               ((x <<  8) & 0x00FF0000) |
               ((x << 24) & 0xFF000000);
    };

    // --- Event-Header ---
    auto* evh = reinterpret_cast<s_ve10_1*>(evData);

    evh->l_dlen    = swap32(evh->l_dlen);
    evh->i_type    = swap16(evh->i_type);
    evh->i_subtype = swap16(evh->i_subtype);
    evh->i_trigger = swap32(evh->i_trigger);

    INTS4 ev_words = evh->l_dlen;
    INTS4 ev_bytes = ev_words * 2;

    evt->eventNumber = fEventNumber++;
    evt->triggerID   = evh->i_trigger;

    // --- Subevents ---
    for(INTS4 sub = 1; ; ++sub)
    {
        s_ves10_1* subh      = nullptr;
        INTS4*     subWords  = nullptr;
        INTS4      dlenWords = 0;

        res = f_evt_get_subevent(evh, sub, (INTS4**)&subh, &subWords, &dlenWords);
        if(res == GETEVT__NOMORE)
            break;
        if(res != GETEVT__SUCCESS)
            break;
        if(!subh || !subWords || dlenWords <= 0)
            continue;

        // Header swappen
        subh->l_dlen     = swap32(subh->l_dlen);
        subh->i_type     = swap16(subh->i_type);
        subh->i_subtype  = swap16(subh->i_subtype);
        subh->i_procid   = swap16(subh->i_procid);
        subh->h_subcrate = swap16(subh->h_subcrate);
        subh->h_control  = swap16(subh->h_control);

        INTS4 sub_words   = subh->l_dlen;
        INTS4 sub_bytes   = sub_words * 2;
        INTS4 payload_len = sub_bytes - (INTS4)sizeof(s_ves10_1);
        if(payload_len <= 0)
            continue;

        INTS4 nwords = payload_len / 4;
        std::vector<uint32_t> payload(nwords);

        for(INTS4 i = 0; i < nwords; ++i)
            payload[i] = swap32((uint32_t)subWords[i]);

        // -------- ElbVfb6Unpacker anwenden --------
        for(size_t i = 0; i < payload.size(); ++i)
        {
            uint32_t word = payload[i];

            if(word == ELB_VFB6TDC_ID) {
                if(i + 1 >= payload.size())
                    break;

                uint32_t tdc_id = payload[i + 1];

                size_t i_start = i + 2;
                size_t i_end   = i_start;
                while(i_end < payload.size() &&
                      payload[i_end] != END_ID &&
                      payload[i_end] != END_ID_OLD)
                {
                    ++i_end;
                }
                if(i_end <= i_start || i_end > payload.size())
                    continue;

                ElbVfb6Unpacker::VFB6_EVENT_LIST list;
                list.reserve(256);

                fVfb6.analyseFileFormat(payload, i_start, i_end, list, 250, false);

                for(const auto& tev : list)
                {
                    for(size_t h = 0; h < tev.nHits; ++h)
                    {
                        uint32_t hit = tev.hits[h];

                        RawHit rh;
                        rh.tdcID   = (int)tdc_id;
                        rh.channel = fVfb6.getChannel(hit);
                        rh.rawTimeInNanoSeconds =
                            fVfb6.getTime(hit, ElbVfb6Unpacker::Random);
                        rh.accepted = true;

                        evt->hits.push_back(rh);
                    }
                }

                i = i_end;
            }
        }
    }

    return kTRUE;
}






//----------------------------------------------------------

Bool_t TQClamOfflineLmdSource::CheckEventClass(TClass* cl)
{
  return cl->InheritsFrom(TQClamRawEvent::Class());
}

