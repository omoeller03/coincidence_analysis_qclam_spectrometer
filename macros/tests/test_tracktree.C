#include "Riostream.h"
#include "TTree.h"

/*
Bool_t test_tracktree()
{
   auto go4 = TGo4AbstractInterface::Instance();

   TString path = go4->FindItem("Track");

   std::cout << "Path = " << path << std::endl;

   TObject* obj = go4->GetObject(path,5000);

   std::cout << "obj = " << obj << std::endl;

   if(!obj)
   {
      std::cout << "NULL OBJECT" << std::endl;
      return kFALSE;
   }

   std::cout << "Class = " << obj->ClassName() << std::endl;

   return kTRUE;
}
*/

Bool_t test_tracktree()
{
   auto go4 = TGo4AbstractInterface::Instance();

   TString path = go4->FindItem("Track");

   std::cout << "Path = " << path << std::endl;

   TObject* obj = go4->GetObject(path,5000);

   TTree* t = dynamic_cast<TTree*>(obj); //GetTree(path); 
   if(!obj)
   {
	   std::cout << "ERROR" << std::endl;
	   return kFALSE;
   }

   t->GetEntry(10);
   double phi;
   t->SetBranchAddress("phi",&phi);
   std::cout << phi << std::endl;

   std::cout << "Object ptr = " << obj << std::endl;

   if(obj)
      std::cout << obj->ClassName() << std::endl;

   return kTRUE;
}
