void drawClusterGo4(int ev = 0)
{
	// Zugriff auf die Analysis-Schnittstelle
	TGo4AbstractInterface* go4 = TGo4AbstractInterface::Instance();
	if(!go4)
	{
		std::cout << "Go4 interface not available" << std::endl;
		return;
	}

	// Hole das aktuell selektierte Objekt aus der Analysis-Object-List
//	TObject* obj = go4->GetObject("selected");
	TObject* obj = go4->GetObject("/Analysis/Trees/clusterHits");
    	if(!obj)
	{
		std::cout << "No selected object in Go4" << std::endl;
		return;
	}

	TTree* t = dynamic_cast<TTree*>(obj);
	if(!t)
	{
		std::cout << "Selected object is not a TTree" << std::endl;
		return;
	}

	// Jetzt zeichnen wir wie in deinem ROOT-Makro
	int event, wire, layer, cluster;
	double drift;

	t->SetBranchAddress("eventNumber",&event);
	t->SetBranchAddress("wireNumber",&wire);
	t->SetBranchAddress("drifttime",&drift);
	t->SetBranchAddress("layer",&layer);
	t->SetBranchAddress("clusterNumber",&cluster);

	TCanvas* c = new TCanvas("c","Cluster Display",900,700);
	c->Divide(1,3);

	for(int lay=0; lay<3; lay++)
	{
		c->cd(lay+1);
		gPad->DrawFrame(0,0,200,400,Form("Layer %d;wire;drift", lay));
	}

	Long64_t N = t->GetEntries();

	for(Long64_t i=0;i<N;i++)
	{
		t->GetEntry(i);

		if(event == ev && cluster >= 0)
		{
			c->cd(layer+1);
			TLine* L = new TLine(wire, 0, wire, drift);
			L->SetLineWidth(3);
			L->Draw();
		}
	}

	c->Update();
}

