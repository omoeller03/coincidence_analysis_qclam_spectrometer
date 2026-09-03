#include "TGo4Analysis.h"
#include "TTree.h"
#include <iostream>

void test_analysis_tree()
{
   auto ana = TGo4Analysis::Instance();

   std::cout << "ana = " << ana << std::endl;

   if(!ana) return;

   TTree* tr = ana->GetTree("Track");

   std::cout << "tree = " << tr << std::endl;

   if(tr)
      tr->Print();
}
