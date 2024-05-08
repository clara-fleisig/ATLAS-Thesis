#include <vector>
#include <cstdint>
#include <TFile.h>
#include <TTree.h>
#include <iostream> 
#include <vector>
#include "SysRTVector.h"
#include <TTreeReader.h>
#include <TTreeReaderValue.h>

//includes exclusively for generating targets
#include <random> //for random number generation
#include <cmath> //for trig functions for generating root file

//define number of target hits to be generated
#define NUMB_TARGS 1000;

//define range in which 
#define RHO_MAX 1014;
#define RHO_MIN 33;
#define PHI_MAX 2*3.141593;
#define PHI_MIN 0;
#define Z_MAX 2860;
#define Z_MIN -2860;

bool CheckValue(ROOT::Internal::TTreeReaderValueBase& value) {
   if (value.GetSetupStatus() < 0) {
      std::cerr << "Error " << value.GetSetupStatus()
                << "setting up reader for " << value.GetBranchName() << '\n';
      return false;
   }
   return true;
}

std::vector<Hit_org::Hit> create_hit_vec(char const *filename){
    //open file
    TFile *File = TFile::Open(filename, "UPDATE");

    //read branches of interest into vecotrs of type std::vector<float>
    TTreeReader reader("HitInfo", File);
    TTreeReaderValue<std::vector<float>> x(reader, "hit_x");
    TTreeReaderValue<std::vector<float>> y(reader, "hit_y");
    TTreeReaderValue<std::vector<float>> z(reader, "hit_z");
    reader.Next();

    //check if branches are there
    if (!CheckValue(x) && !CheckValue(y) && !CheckValue(z)){
        std::cerr << "Missing Expected Branch" << std::endl;
        exit(-1);
    }

    //setup for itterating through vectors
    std::vector<Hit_org::Hit> hit_vec;
    auto x_it = x -> begin();
    auto y_it = y -> begin();
    auto z_it = z -> begin();
    auto x_end = x -> end();

    //itterate over vectors to create objects and push them
    while(x_it != x_end){
        //create object and add to hit_vec
        Hit_org::Hit cur_hit(*x_it, *y_it, *z_it);
        hit_vec.push_back(cur_hit);

        //iterate through vectors
        x_it++; y_it++; z_it++;
    }

    //error catching
    if ((x_it != x->end()) || (y_it != y -> end()) || (z_it != z -> end())){
        std::cerr << "Itteration through branches was unsuccessful" << std::endl;
        exit(-1);
    }

    //close file
    File -> Close();

    return hit_vec;
}

void create_targ_root(char const *filename){
    //type set macros so they can be used in functions
    constexpr float n = NUMB_TARGS;
    constexpr float rho_max = RHO_MAX;
    constexpr float rho_min = RHO_MIN;
    constexpr float phi_max = PHI_MAX;
    constexpr float phi_min = PHI_MIN;
    constexpr float z_max = Z_MAX;
    constexpr float z_min = Z_MIN;

    //setup random number generators with unifrom distributions
    std::random_device rd_device; // Create an instance of an engine
    std::mt19937 engine {rd_device()};  // Engine generates random integers
    std::uniform_real_distribution<float> z_dist {z_min, z_max}; //specify distribution
    std::uniform_real_distribution<float> rho_dist {rho_min, rho_max}; //specify distribution
    std::uniform_real_distribution<float> phi_dist {phi_min, phi_max}; //specify distribution

    //initialise varibles to be used
    float targ_x; float targ_y; float targ_z;
    float targ_rho; float targ_phi;

    //open root file
    TFile ResultsFile(filename,"RECREATE");

    //create tree
    TTree ResultsTree("TargetHits", "Search for target hits with runtime efficiencies");
    ResultsTree.Branch("targ_x",&targ_x,"targ_x/F");
    ResultsTree.Branch("targ_y", &targ_y, "targ_y/F");
    ResultsTree.Branch("targ_z", &targ_z, "targ_z/F");
    for(int i = 0; i < n; i++){
        targ_rho = rho_dist(engine);
        targ_phi = phi_dist(engine);
        targ_x = targ_rho * std::cos(targ_phi);
        targ_y = targ_rho * std::sin(targ_phi);
        targ_z = z_dist(engine);
        ResultsTree.Fill();
    }

    //write to tree to root file and close file
    ResultsTree.Write();
    ResultsFile.Close();
}

int main()
{
    //open file with info
    //TFile *File = TFile::Open("files/ClusterHitSeedRoot.root", "UPDATE");

    //Create root file filled with random target hits (only needs to be called once)
    //create_targ_root("files/VectorRuntime.root");
    
    //vector objects are allocated on heap
    std::vector<Hit_org::Hit> hit_vec = create_hit_vec("files/ClusterHitSeedRoot.root");

    //close file
    //File -> Close();
    
    return 0;
}