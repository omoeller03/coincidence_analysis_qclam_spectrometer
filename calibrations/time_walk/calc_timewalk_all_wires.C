void calc_timewalk(const char* filename="DrifttimeCalib.root")
{
    TFile* f = TFile::Open(filename);
    TTree* t = (TTree*)f->Get("DrifttimeCalib");

    std::vector<TQClamHit>* hits = nullptr;
    t->SetBranchAddress("hits", &hits);

    std::map<std::pair<int,int>, TH2D*> h2;

    for(Long64_t i=0;i<t->GetEntries();i++){
        t->GetEntry(i);
        for(auto& h : *hits){
            double rawT = h.leading - h.tdcTrigTime;
            double tot  = h.trailing - h.leading;

            auto key = std::make_pair(h.layerID, h.wireNum);

            if(!h2[key]){
                h2[key] = new TH2D(Form("tw_%d_%d",h.layerID,h.wireNum),
                                   "ToT vs drift time",
                                   200, 0, 100, 200, -100, 100);
            }
            h2[key]->Fill(tot, rawT);
        }
    }

    std::ofstream fout("timewalk_params.txt");

    for(auto& kv : h2){
        auto key = kv.first;
        TH2D* hist = kv.second;

        TF1* f = fittimewalk(hist); // dein Makro
        fout << key.first << " " << key.second << " "
             << f->GetParameter(0) << " "
             << f->GetParameter(1) << " "
             << f->GetParameter(2) << " "
             << f->GetParameter(3) << "\n";
    }
}
