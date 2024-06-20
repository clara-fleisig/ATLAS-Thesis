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
#include <TGraphErrors.h>
#include <TAxis.h>

#define NUMB_HIT_IT 15; //number of hit iterations (must be >0) 
#define NUMB_TARGS 100; //number of randomly generated targets to search for (must be >0)
//example: NUMB_HIT_IT=4 & NUMB_TARGS=100 & total hits in root file=50000
//                   -> assess runtimes for 50000 hits, 40000 hits, 30000 hits, 20000 hits, 10000 hits
//                   -> each time you assessT runtimes you test with 100 randomly generated hits

//define range in which target hits should be generated
#define RHO_MAX 1014;
#define RHO_MIN 33;
#define PHI_MAX 2*3.141593;
#define PHI_MIN 0;
#define Z_MAX 2860;
#define Z_MIN -2860;

#pragma optimize( "", off )
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

Hit_org::Hit find_closest_hit(Hit_org::Hit targ_hit, std::vector<Hit_org::Hit> hit_vec, const int n_hits){
    //initialize closest_hit object and min_dis variabale, where min_dis is currently the largest possible distance
    Hit_org::Hit closest_hit(0, 0, 0);
    float min_dis = std::numeric_limits<float>::max();

    //itterate through each ATHENA hit (stored in hit_vec)
    for(int i = n_hits; i>n_hits; i++){
        //calculate distance between target hit and current ATHENA hit of interest
        auto cur_dis = calc_dis(hit_vec.at(i), targ_hit);

        //replace min_dis and closest_hit with values from current ATHENA hit if ATHENA hit is closer than stored values
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
    constexpr int n_targs = NUMB_TARGS;
    constexpr int n_hit_it =  NUMB_HIT_IT;
    const int total_hits = hit_vec.size();
    int n_hits;
    std::vector<float> rt_vec, sel_x_vec, sel_y_vec, sel_z_vec;

    //write output to root file
    TFile ResultsFile(filename, "UPDATE");
    auto ReadTree = ResultsFile.Get<TTree>("TargetHits");
    ReadTree -> SetBranchStatus("targ_x",0); ReadTree -> SetBranchStatus("targ_y",0); ReadTree -> SetBranchStatus("targ_z",0);

    TTree *ResultsTree = new TTree("MyResults", "Search for target hits with runtime efficiencies");
    
    auto n_hits_branch = ResultsTree->Branch("n_hits", &n_hits);
    auto rt_branch = ResultsTree->Branch("sel_rt", &rt_vec);
    auto sel_x_branch = ResultsTree->Branch("sel_x", &sel_x_vec); 
    auto sel_y_branch = ResultsTree->Branch("sel_y", &sel_y_vec); 
    auto sel_z_branch = ResultsTree->Branch("sel_z", &sel_z_vec);

    // loop over every target hit
    for(float j = 0; j<=n_hit_it; j++){

        //determine number of hits to search through
        n_hits =  total_hits * (j / n_hit_it);
        //sel_x_vec.clear(); sel_y_vec.clear(); sel_z_vec.clear();
        
        //search through hits for all targets
        for(int i = 0; i<n_targs; i++){

            //find closest hit and time function
            auto t1 = std::chrono::high_resolution_clock::now();
            auto sel_hit = find_closest_hit(targ_vec.at(i), hit_vec, n_hits);
            auto t2 = std::chrono::high_resolution_clock::now();
            auto dur = std::chrono::duration<double, std::milli>(t2-t1);

            //save data in vector
            sel_x_vec.push_back(sel_hit.x);
            sel_y_vec.push_back(sel_hit.y);
            sel_z_vec.push_back(sel_hit.z);
            rt_vec.push_back(dur.count());
        }

        //ResultsTree -> Fill(); technically right, but annoying because gives a warning
        ResultsTree -> Fill();
        //n_hits_branch -> Fill(); rt_branch -> Fill(); sel_x_branch -> Fill(); sel_y_branch -> Fill(); sel_z_branch -> Fill();
        std::cout << n_hits << " hits searched for " << n_targs << " targets" << std::endl;
    }

    ResultsTree->Write("", TObject::kOverwrite);
    //ResultsFile.Close();

}

void gen_rt_graph(){
    std::cout << "Generating Runtime Graph..." << std::endl;
    TFile file("files/VectorRuntime.root", "UPDATE"); auto myTree = file.Get<TTree>("MyResults");
    std::vector<float> *rt_vec = nullptr; int n_hits;
    myTree -> SetBranchAddress("sel_rt", &rt_vec); myTree -> SetBranchAddress("n_hits", &n_hits);
    
    const int n = myTree -> GetEntries();
    Double_t n_hits_arr[n]; Double_t avg_arr[n]; Double_t stdev_arr[n];

    for(int i = 0; i<n; i++){
        myTree -> GetEntry(i);
        n_hits_arr[i] = n_hits / 1000;
        double sum = std::accumulate(rt_vec->begin(), rt_vec->end(), 0.0);
        avg_arr[i] = sum / rt_vec->size();
        double sq_sum = std::inner_product(rt_vec->begin(), rt_vec->end(), rt_vec->begin(), 0.0);
        stdev_arr[i] = std::sqrt(sq_sum / rt_vec->size() - avg_arr[i] * avg_arr[i]);
    }
    
    TGraphErrors gr(n, n_hits_arr, avg_arr, nullptr, stdev_arr);
    gr.SetTitle("Runtime Scaling");
    gr.GetXaxis()->SetTitle("Number of kHits");
    gr.GetYaxis()->SetTitle("Average Runtime (ms)");
    gr.Write("Runtime Graph");
    file.Close();
}

void gen_rt_graph(){
    std::cout << "Generating Runtime Graph..." << std::endl;
    TFile file("files/VectorRuntime.root", "UPDATE"); auto myTree = file.Get<TTree>("MyResults");
    std::vector<float> *rt_vec = nullptr; int n_hits;
    myTree -> SetBranchAddress("sel_rt", &rt_vec); myTree -> SetBranchAddress("n_hits", &n_hits);
    
    const int n = myTree -> GetEntries();
    Double_t n_hits_arr[n]; Double_t avg_arr[n]; Double_t stdev_arr[n];

    for(int i = 0; i<n; i++){
        myTree -> GetEntry(i);
        n_hits_arr[i] = n_hits / 1000;
        double sum = std::accumulate(rt_vec->begin(), rt_vec->end(), 0.0);
        avg_arr[i] = sum / rt_vec->size();
        double sq_sum = std::inner_product(rt_vec->begin(), rt_vec->end(), rt_vec->begin(), 0.0);
        stdev_arr[i] = std::sqrt(sq_sum / rt_vec->size() - avg_arr[i] * avg_arr[i]);
    }
    
    TGraphErrors gr(n, n_hits_arr, avg_arr, nullptr, stdev_arr);
    gr.SetTitle("Runtime Scaling");
    gr.GetXaxis()->SetTitle("Number of kHits");
    gr.GetYaxis()->SetTitle("Average Runtime (ms)");
    gr.Write("Runtime Graph");
    file.Close();
}

int main(){
    //start timer
    auto tot_t1 = std::chrono::high_resolution_clock::now();
    
    //Create root file filled with random target hits (only needs to be called once to create file)
    Hit_org::create_targ_root("files/VectorRuntime.root");
    
    //Read hits and target_hits into vectors
    std::vector<Hit_org::Hit> hit_vec = Hit_org::create_hit_vec("files/ClusterHitSeedRoot.root", "HitInfo", "hit_x", "hit_y", "hit_z");
    std::vector<Hit_org::Hit> targ_vec = Hit_org::create_hit_vec("files/VectorRuntime.root", "TargetHits", "targ_x", "targ_y", "targ_z");
    
    //run search with varying number of hits and store in VectorRuntime.root
    Hit_org::full_search_mc(targ_vec, hit_vec, "files/VectorRuntime.root");

    //generate graph from data stored in MyResults Tree
    gen_rt_graph();

    //report runtime of program
    auto tot_t2 = std::chrono::high_resolution_clock::now();
    auto tot_dur = std::chrono::duration<double, std::milli>(tot_t2-tot_t1);
    std::cout << "Total runtime duration: \t" << tot_dur.count() << "ms" << std::endl;

    return 0;
}
#pragma optimize( "", on )