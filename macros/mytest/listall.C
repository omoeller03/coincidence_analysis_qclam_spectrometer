Bool_t listall()
{
   auto go4 = TGo4AbstractInterface::Instance();

   go4->PrintAnalysisObjects();

   return kTRUE;
}
