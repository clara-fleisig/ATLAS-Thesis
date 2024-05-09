#include "SysRTVector.h"
#include <vector>
#include <cstdint>
#include <iostream>
#include <TFile.h> //for reading and writing files
#include <TTree.h> //for reading and writing files
#include <TTreeReader.h> //for reading and writing files
#include <TTreeReaderValue.h> //for reading and writing files
#include <limits> //to set to largest possible float, for find_closest_hit()
#include <random> //for random number generation, includes exclusively for generating targets
#include <cmath> //for trig functions for generating root file,includes exclusively for generating targets
#include <chrono> //for runtime evaluations

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

float calc_dis(Hit_org::Hit hit1, Hit_org::Hit hit2){
    return std::pow(hit1.x-hit2.x, 2) + std::pow(hit1.y-hit2.y, 2) + std::pow(hit1.z-hit2.z, 2);
}

Hit_org::Hit find_closest_hit(Hit_org::Hit targ_hit, std::vector<Hit_org::Hit> hit_vec, const int numb_hits){
    //setup 
    Hit_org::Hit closest_hit(0, 0, 0);
    float min_dis = std::numeric_limits<float>::max();

    for(int i = 0; i<numb_hits; i++){
        auto cur_dis = calc_dis(hit_vec.at(i), targ_hit);
        if(cur_dis < min_dis){
            min_dis = cur_dis;
            closest_hit = hit_vec.at(i);
        }
    }

    return closest_hit;
}

std::vector<Hit_org::Hit> Hit_org::create_hit_vec(char const *filename, char const *treename, char const *xname, char const *yname, char const *zname){
    //open file
    TFile *File = TFile::Open(filename, "READ");

    //read branches of interest into vecotrs of type std::vector<float>
    TTreeReader reader(treename, File);
    TTreeReaderValue<std::vector<float>> x(reader, xname);
    TTreeReaderValue<std::vector<float>> y(reader, yname);
    TTreeReaderValue<std::vector<float>> z(reader, zname);
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

void Hit_org::create_targ_root(char const *filename){
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

    //create vectors with random variables
    std::vector<float> x_vec; std::vector<float> y_vec; std::vector<float> z_vec;
    
    float targ_rho; float targ_phi;
    for(int i = 0; i < n; i++){
        targ_rho = rho_dist(engine);
        targ_phi = phi_dist(engine);

        x_vec.push_back(targ_rho * std::cos(targ_phi));
        y_vec.push_back(targ_rho * std::sin(targ_phi));
        z_vec.push_back(z_dist(engine));
    }

    //open root file and write vectors in TTree format
    TFile ResultsFile(filename,"RECREATE");
    TTree ResultsTree("TargetHits", "Search for target hits with runtime efficiencies");
    ResultsTree.Branch("targ_x", &x_vec); 
    ResultsTree.Branch("targ_y", &y_vec); 
    ResultsTree.Branch("targ_z", &z_vec);
    ResultsTree.Fill();
    ResultsTree.Write();
    ResultsFile.Close();
}

void Hit_org::full_search_mc(std::vector<Hit_org::Hit> targ_vec, std::vector<Hit_org::Hit> hit_vec, const char *filename){
    constexpr float n_targs = NUMB_TARGS;
    std::vector<float> rt_vec;
    std::vector<float> sel_x_vec; 
    std::vector<float> sel_y_vec; 
    std::vector<float> sel_z_vec;

    // loop over every target hit
    for(int i = 0; i<n_targs; i++){
        //find closest hit and time function
        auto t1 = std::chrono::high_resolution_clock::now();
        auto sel_hit = find_closest_hit(targ_vec.at(i), hit_vec, hit_vec.size());
        auto t2 = std::chrono::high_resolution_clock::now();
        auto dur = std::chrono::duration<double, std::micro>(t2-t1);

        //save data
        sel_x_vec.push_back(sel_hit.x);
        sel_y_vec.push_back(sel_hit.y);
        sel_z_vec.push_back(sel_hit.z);
        rt_vec.push_back(dur.count());
    }

    //write output to root file
    TFile ResultsFile(filename, "UPDATE");
    auto ResultsTree = ResultsFile.Get<TTree>("TargetHits");
    auto rt_branch = ResultsTree->Branch("sel_rt", &rt_vec);
    auto sel_x_branch = ResultsTree->Branch("sel_x", &sel_x_vec);
    auto sel_y_branch = ResultsTree->Branch("sel_y", &sel_y_vec);
    auto sel_z_branch = ResultsTree->Branch("sel_z", &sel_z_vec);
    rt_branch -> Fill(); sel_x_branch -> Fill(); sel_y_branch -> Fill(); sel_z_branch -> Fill();
    ResultsTree->Write("", TObject::kOverwrite);

}

int main(){
    auto tot_t1 = std::chrono::high_resolution_clock::now();
    
    //Create root file filled with random target hits (only needs to be called once to create file)
    //Hit_org::create_targ_root("files/VectorRuntime.root");
    
    //Read hits and target_hits into vectors
    std::vector<Hit_org::Hit> hit_vec = Hit_org::create_hit_vec("files/ClusterHitSeedRoot.root", "HitInfo", "hit_x", "hit_y", "hit_z");
    std::vector<Hit_org::Hit> targ_vec = Hit_org::create_hit_vec("files/VectorRuntime.root", "TargetHits", "targ_x", "targ_y", "targ_z");
    Hit_org::full_search_mc(targ_vec, hit_vec, "files/VectorRuntime.root");

    auto tot_t2 = std::chrono::high_resolution_clock::now();
    auto tot_dur = std::chrono::duration<double, std::milli>(tot_t2-tot_t1);
    std::cout << "Total runtime duration: \t" << tot_dur.count() << "ms" << std::endl;

    return 0;
}