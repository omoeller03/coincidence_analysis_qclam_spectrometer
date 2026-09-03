#include "TGo4Analysis.h"
#include "TGo4AnalysisObjectManager.h"
#include <iostream>

void list_trees()
{
   auto ana = TGo4Analysis::Instance();

   auto om = ana->GetObjectManager();

   std::cout << "ana = " << ana << std::endl;
   std::cout << "om  = " << om << std::endl;

   if(!om) return;

   om->Print();
}
