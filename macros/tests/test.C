#include <vector>
#include <algorithm>

TTree* t = nullptr;
std::vector<int> clusterEvents;
int clusterIndex = -1;

void initClusterNavigator(TTree* tree)
{
t = tree;
clusterEvents.clear();
currentIndex = -1;

Long64_t n = t->Draw("eventNumber", "clusterNumber>=0", "goff");
Int_t* ev = t->GetV1();

for(Long64_t i=0; i<n; i++)
{
clusterEvents.push_back(ev[i]);
}

// duplikate entfernen
std::sort(clusterEvents.begin(), clusterEvents.end());
clusterEvents.erase(std::unique(clusterEvents.begin(), clusterEvents.end()), clusterEvents.end());

std::cout << "Gefundene Events mit Clustern: " << std::to_string(clusterEvents.size()) << std::endl;
}



void applyEventCut(int event)
{
TString cut = Form("eventNumber == %d", event);
std::cout << "Zeige Event " << std::to_string(event) << std::endl;

t->SetAlias("eventCut", cut);

t->Draw("wireNumber:drifttime", "eventCut");
}


void nexClusterEvent()
{
if(clusterEvents.empty())
{
std::cout << "Keine Cluster-Events gefunden." << std::endl;
return;
}

if(currentIndx < (int)clusterEvents.size()-1)
{
currentIndex++;
}

else
{
std::cout << "Letztes Cluster Event erreicht." << std::endl;
return;
}

applyEventCut(clusterEvents[currentIndex]);
}



void prevClusterEvent()
[
if(clusterEvents.empty())
{
std::cout << "Keine Cluster-Events gefunden" << std::endl;
return;
}

if(clusterIndex > 0)
{
clusterIndex--;
}

else
{
std::cout << "Erstes Cluster-Event erreicht" << sdt::endl;
return;
}

applyEventCut(clusterEvents[currentIndex]);
}



