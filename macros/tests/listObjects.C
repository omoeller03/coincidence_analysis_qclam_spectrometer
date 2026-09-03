void listObjects()
{
	TGo4AbstractInterface* go4 = TGo4AbstractInterface::Instance();
	TObject* obj = go4->GetObject("selected");

	std::cout << "Selected: " << obj->ClassName() << " name=" << obj->GetName() << std::endl;
	if(!obj)
	{
		std::cout << "No selected object" << std::endl;
		return;
	}
	
	std::cout << "Selected: " << obj->ClassName() << " name=" << obj->GetName() << std::endl;

	TTree* t = dynamic_cast<TTree*>(obj);
	if(!t)
	{
		std::cout << "Selected object is not a TTree" << std::endl;
		return;
	}

	t->Scan();
}
