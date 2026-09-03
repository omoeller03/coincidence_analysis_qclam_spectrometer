void showTree()
{
	TGo4AbstractInterface* go4 = TGo4AbstractInterface::Instance();
	TTree* t = (TTree*)go4->GetObject("/Analysis/Trees/clusterHits");
	if(!t)
	{
		std::cout << "Tree not found" << std::endl;
		return;
	}
	t->Show(0);
	t->Show(100);
}
