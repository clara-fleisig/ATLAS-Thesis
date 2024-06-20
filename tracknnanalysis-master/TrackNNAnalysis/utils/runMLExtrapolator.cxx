// C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <typeinfo>
#include "TObjString.h"
#include "TObjArray.h"
#include <Eigen/Core>
#include <nlohmann/json.hpp>

// Local includes
#include "TrackNNAnalysis/MLTrackExtrapolatorReco.h"
#include "TrackNNEDM/TrackTree.h"

using namespace std;

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts);


int main(int argc, char *argv[])
{
    // Process command line arguments
    map<std::string, std::string> opts;
    if(!cmdline(argc,argv, opts)) return 0;
    
    auto trackCreator = std::make_shared<MLTrackExtrapolatorReco>();
    trackCreator->setConfigFile(opts.at("configFile"));
    trackCreator->setInputFile(opts.at("inputFile"));
    trackCreator->setNEvents(atoi(opts["nEvents"].c_str()));

    trackCreator->initialize();

    auto nEvents = trackCreator->getNEvents();

    for(int i = 0; i < nEvents; i++)
    {
        if(i%10 == 0) cout<<"Processing event: "<<i<<" percent done: "<<float(i)/nEvents * 100<<endl;
        trackCreator->run(i);

        auto trackTree = trackCreator->getTrackTree();

        for (auto& tree:trackTree)
        {
            std::cout<<"final hits size "<<tree->getFinalHits().size()<<std::endl;
            for(auto h:tree->getFinalHits())
            {
                std::cout<<"printing"<<std::endl;
                tree->printDebug(h);
            }
        }
    }

} // main

bool cmdline(int argc, char** argv, map<std::string, std::string>& opts)
{
    opts["inputFile"]       = "HitInformation.root";
    opts["nEvents"]         = "-1";
    opts["configFile"]      = "HitInformation.root";



    for(int i = 1; i < argc; ++i)
    {
        string opt=argv[i];
        if(opt=="--help" || opt == "--options" || opt =="--h")
        {
            cout<< "Options menu \n ============================================== \n" << endl;
            cout<<"--inputFile          : Path to the input file to process"<< endl;
            cout<<"--nEvents            : Number of events to run over (-1 is all)"<<endl;
            cout<<"--configFile         : Path to config file"<<endl;
            return false; 
        }

        if(0!=opt.find("--")) {
            cout<<"ERROR: options start with '--'!"<<endl;
            cout<<"ERROR: options is: "<<opt<<endl;
            return false;
        }
        opt.erase(0,2);


        string nxtopt=argv[i+1];
        if(0==nxtopt.find("--")||i+1>=argc) {
            cout<<"ERROR: option '"<<opt<<"' requires value!"<<endl;
            return false;
        }
        if(opts.find(opt)!=opts.end())  opts[opt]=nxtopt;
        else
        {
            cout<<"ERROR: invalid option '"<<opt<<"'!"<<endl;
            return false;
        }

        i++;
    }

    return true;
}

std::vector<TString> tokenizeStr(TString str, TString key)
{
    TObjArray *Varparts = str.Tokenize(key);
    vector<TString> varNameVec;
    if(Varparts->GetEntriesFast()) {
        TIter iString(Varparts);
        TObjString* os=0;
        while ((os=(TObjString*)iString())) {
            varNameVec.push_back(os->GetString());
        }
    }
    return varNameVec;
}


