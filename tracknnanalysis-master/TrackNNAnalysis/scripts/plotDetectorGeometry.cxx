
#include "../Root/atlasstyle/AtlasStyle.C"
#include "../Root/atlasstyle/AtlasLabels.C"
#include "TFile.h"
#include "TH2.h"
#include "TTree.h"
#include "TLegend.h"
#include "TCanvas.h"
//#include "atlasstyle/AtlasStyle.C"
//#include "atlasstyle/AtlasLabels.C"

using namespace std;
void setPrettyStuff();
TString getFancyName(int id, TString colorVar);


void plotDetectorGeometry(TString xName = "x", TString yName = "y", TString colorVar = "module_volumeId")

{
	setPrettyStuff();
	

	TFile* file = TFile::Open("../../../run/ClusterHitSeedRoot.root", "read");

	TTree* tree = (TTree*)file->Get("DetectorGeo");

	TString fullXName = "module_center_" + xName;
	TString fullYName = "module_center_" + yName;

	std::map<TString, TH2F*> histMap;

	// you can do x,y,z,r,rho for x and y
    float xVar;
    float yVar; 
    //tree->SetBranchAddress(fullXName, &xVar);
    //tree->SetBranchAddress(fullYName, &yVar);

    int selectionVar;
    tree->SetBranchAddress(colorVar, &selectionVar);

    float maxX = tree->GetMaximum(fullXName);
    float minX = tree->GetMinimum(fullXName);

    float maxY = tree->GetMaximum(fullYName);
    float minY = tree->GetMinimum(fullYName);

    int binsX = 1000;
    int binsY = 1000;

    // Set each id to a unique color for col draw option
    Int_t colors[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14}; // #colors >= #levels - 1
    Double_t colorLevels[16];
    gStyle->SetPalette((sizeof(colors)/sizeof(Int_t)), colors);


    // get unique Idenifiers of the cut Variable
    std::vector<int> uniqueIDs;
    cout << "Tree has " << tree->GetEntries() << endl;
    for(int i = 0; i < tree->GetEntries(); i++)
    {
    	tree->GetEntry(i);
    	uniqueIDs.push_back(selectionVar);
    }
	sort( uniqueIDs.begin(), uniqueIDs.end() );
	uniqueIDs.erase( unique( uniqueIDs.begin(), uniqueIDs.end() ), uniqueIDs.end() );
	//sort( uniqueIDs.begin(), uniqueIDs.end(), std::greater<>() );
	int ci = 1756;

	float minXPad = minX;
	float maxXPad = maxX;
	float diffX = maxXPad - minXPad;

	minXPad -= diffX*.1;
	maxXPad += diffX*.1;

	float minYPad = minY;
	float maxYPad = maxY;
	float diffY = maxYPad - minYPad;

	minYPad -= diffY*.1;
	maxYPad += diffY*.5;


	TH2F* frame = new TH2F("frame", "frame", binsX, minXPad, maxXPad, binsY, minYPad, maxYPad );

   	TLegend *elLeg =  new TLegend (0.35, 0.77, 0.85, 0.9);
    elLeg->SetFillColor(0);
    elLeg->SetBorderSize(0);
    elLeg->SetTextFont(42);
    elLeg->SetTextSize(0.015);
    elLeg->SetNColumns(2);

	// draw the histograms
    int colorArrayIdx = 1;
    colors[0] = 0;
	for(auto id: uniqueIDs)
	{
        histMap[id] = new TH2F(Form("Hist_%d", id),Form("Hist_%d", id), binsX, minX, maxX, binsY, minY, maxY );

        tree->Draw(fullYName+":"+fullXName+">>" + Form("Hist_%d", id), "(" + colorVar + " == " + Form("%d", id) + ")");
        for(int i=1;i<=1000;i++)
        {
            for(int j=1;j<=1000; j++)
            {
                if(histMap[id]->GetBinContent(i,j)>0) histMap[id]->SetBinContent(i,j,id);
            }
        }
        histMap[id]->SetMarkerColor(ci);
        histMap[id]->SetMarkerSize(0.5);
        elLeg->AddEntry(histMap[id], getFancyName(id, colorVar), "p" );

        // ci++;
        colorLevels[ci] = id;
		ci++;
        if(ci > 1770) ci = 0;
	}

	TCanvas* c = new TCanvas("", "", 0, 0, 600, 600);
	frame->GetXaxis()->SetTitle(fullXName);
	frame->GetYaxis()->SetTitle(fullYName);
	frame->Draw();

	for(auto hist:histMap) 
    {
        
        hist.second->SetContour((sizeof(colorLevels)/sizeof(Double_t)), colorLevels);
        hist.second->Draw("col same");
    }
	elLeg->Draw();


    float leftDist = 0.19;
    // ATLASLabel(leftDist, 0.875, "Internal", 1);

	c->SaveAs("ITKnew_" + fullXName + "_" + fullYName + "_inSliced_" + colorVar +".pdf");


    

}

TString getFancyName(int id, TString colorVar)
{
	if(colorVar.EqualTo("module_volumeId"))
	{
		return Form("VolumeID: %d",id);	
	}
	else return Form("%d", id);
}

void setPrettyStuff()
{

    SetAtlasStyle();

    Int_t ci = 1756; // color index
    vector<TColor*> TColors;

    new TColor(ci, 0.8500,    0.3250,    0.0980); //57
    ci++;          
    new TColor(ci, 0.4940,    0.1840,    0.5560); //58
    ci++;
    new TColor(ci, 0.9290,    0.6940,    0.1250); //59
    ci++; 
    new TColor(ci, 0.4660,    0.6740,    0.1880); //60
    ci++;
    new TColor(ci, 0.3010,    0.7450,    0.9330); //61
    ci++;
    new TColor(ci, 0.6350,    0.0780,    0.1840); //62
    ci++;
    new TColor(ci, 142.0/255 , 0.0/255 , 62.0/255);
    ci++;
    new TColor(ci, 96.0/255 , 78.0/255 , 0.0/255);
    ci++;
    new TColor(ci, 92.0/255 , 174.0/255 , 0.0/255);
    ci++;
    new TColor(ci, 1.0/255 , 237.0/255 , 171.0/255);
    ci++;
    new TColor(ci, 50.0/255 , 134.0/255 , 255.0/255);
    ci++;
    new TColor(ci, 112.0/255 , 0.0/255 , 88.0/255);
    ci++;
    new TColor(ci, 28.0/255 , 0.0/255 , 19.0/255);
    ci++;
    new TColor(ci, 255.0/255 , 102.0/255 , 165.0/255);
    ci++;
    new TColor(ci, 25.0/255 , 102.0/255 , 165.0/255);
    ci++;
}

