#include "TROOT.h"
#include "TTree.h"
#include <iostream>

void findtrack()
{
   gROOT->GetListOfBrowsables()->Print();

   TObject* obj = gROOT->FindObject("Track");

   std::cout << "obj = " << obj << std::endl;

   if(obj)
      std::cout << obj->ClassName() << std::endl;
}
