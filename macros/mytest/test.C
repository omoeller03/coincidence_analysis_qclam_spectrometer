Bool_t test()
{
   auto go4 = TGo4AbstractInterface::Instance();

   TObject* obj = go4->GetObject("Objects/MyTest",5000);

   std::cout << "obj=" << obj << std::endl;

   return kTRUE;
}
