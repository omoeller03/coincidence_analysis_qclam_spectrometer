#include <iostream>

Bool_t find_mytest()
{
   auto go4 = TGo4AbstractInterface::Instance();

   TString path = go4->FindItem("MyTest");

   std::cout << "Path = " << path << std::endl;

   if(path.IsNull()) {
      std::cout << "MyTest nicht gefunden" << std::endl;
      return kFALSE;
   }

   TObject* obj = go4->GetObject(path, 2000);

   std::cout << "obj = " << obj << std::endl;

   if(!obj) {
      std::cout << "NULL OBJECT" << std::endl;
      return kFALSE;
   }

   std::cout << "Class = " << obj->ClassName() << std::endl;
   obj->Print();

   return kTRUE;
}
