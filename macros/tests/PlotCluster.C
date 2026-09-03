#include "Riostream.h"

Bool_t PlotCluster(Int_t tracknumber, Int_t layerID)
{
   TTree* tree = ...; // später einsetzen

   tree->GetEntry(tracknumber);

   std::vector<int>* layer = nullptr;
   std::vector<int>* wire = nullptr;
   std::vector<double>* z = nullptr;

   tree->SetBranchAddress("res_layerID",&layer);
   tree->SetBranchAddress("res_wireNumber",&wire);
   tree->SetBranchAddress("z_measured",&z);

   tree->GetEntry(tracknumber);

   int minWire = 999999;
   int maxWire = -999999;

   for(size_t i=0;i<layer->size();i++)
   {
      if(layer->at(i)!=layerID)
         continue;

      minWire = std::min(minWire, wire->at(i));
      maxWire = std::max(maxWire, wire->at(i));
   }

   TH1D* h = new TH1D(
      Form("cluster_t%d_l%d",tracknumber,layerID),
      Form("Track %d Layer %d",tracknumber,layerID),
      maxWire-minWire+1,
      minWire-0.5,
      maxWire+0.5);

   for(size_t i=0;i<layer->size();i++)
   {
      if(layer->at(i)!=layerID)
         continue;

      h->SetBinContent(
         h->FindBin(wire->at(i)),
         z->at(i));
   }

   h->GetXaxis()->SetTitle("Wire Number");
   h->GetYaxis()->SetTitle("z_{measured}");

   h->Draw("hist");

   return kTRUE;
}
