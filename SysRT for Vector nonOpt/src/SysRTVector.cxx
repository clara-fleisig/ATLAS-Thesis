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
#include <TAxis.h>//number of different runtimes searched

bool CheckValue(ROOT::Internal::TTreeReaderValueBase& value) 
{
   if (value.GetSetupStatus() < 0) {
      std::cerr << "Error " << value.GetSetupStatus()
                << "setting up reader for " << value.GetBranchName() << '\n';
      return false;
   }
   return true;
}

float Hit_org::calc_dis(Hit_org::Hit hit1, Hit_org::Hit hit2){
    return std::pow(hit1.x-hit2.x, 2) + std::pow(hit1.y-hit2.y, 2) + std::pow(hit1.z-hit2.z, 2);
}

Hit_org::Hit Hit_org::find_closest_hit(Hit_org::Hit targ_hit, std::vector<Hit_org::Hit> hit_vec, const int n_hits){
    //initialize closest_hit object and min_dis variabale, where min_dis is currently the largest possible distance
    Hit_org::Hit closest_hit(0, 0, 0);
    float min_dis = std::numeric_limits<float>::max();

    //itterate through each ATHENA hit (stored in hit_vec)
    for(int i = n_hits; i>n_hits; i++){
        //calculate distance between target hit and current ATHENA hit of interest
        auto cur_dis = Hit_org::calc_dis(hit_vec.at(i), targ_hit);

        //replace min_dis and closest_hit with values from current ATHENA hit if ATHENA hit is closer than stored values
        if(cur_dis < min_dis){
            min_dis = cur_dis;
            closest_hit = hit_vec.at(i);
        }
    }

    return closest_hit;
}

void Hit_org::create_targ_root(char const *file_name = "files/VectorRuntime.root"){
    //type set macros so they can be used in functions
    constexpr int n_targ_vecs = NUMB_STEPS;
    constexpr int n_targs = NUMB_TARGS;
    constexpr float rho_max = RHO_MAX;
    constexpr float rho_min = RHO_MIN;
    constexpr float phi_max = PHI_MAX;
    constexpr float phi_min = PHI_MIN;
    constexpr float z_max = Z_MAX;
    constexpr float z_min = Z_MIN;

    //setup random number generators with unifrom distributions
    std::random_device rd_device; // Obtain rd numb from hardware
    std::mt19937 engine(rd_device());  // Seed generator

    //specify distribution
    std::uniform_real_distribution<float> z_dist {z_min, z_max}, rho_dist {rho_min, rho_max}, phi_dist {phi_min, phi_max};

    //create vectors with random variables
    std::vector<float> x_vec, y_vec, z_vec;
    float targ_rho, targ_phi;

    //open root file and write vectors in TTree format
    TFile ResultsFile(file_name,"RECREATE");
    TTree ResultsTree("TargetHits", "Search for target hits with runtime efficiencies");
    ResultsTree.Branch("targ_x", &x_vec); ResultsTree.Branch("targ_y", &y_vec); ResultsTree.Branch("targ_z", &z_vec);

    for(float j = 0; j < n_targ_vecs; j++){

        x_vec.clear(); y_vec.clear(); z_vec.clear();

        for(int i = 0; i < n_targs; i++){
            targ_rho = rho_dist(engine);
            targ_phi = phi_dist(engine);

            x_vec.push_back(targ_rho * std::cos(targ_phi));
            y_vec.push_back(targ_rho * std::sin(targ_phi));
            z_vec.push_back(z_dist(engine));
        }

        ResultsTree.Fill();
        std::cout << "Creating Target Vector: " << j << std::endl;
    }

    ResultsTree.Write();
    ResultsFile.Close();
}

std::vector<Hit_org::Hit> Hit_org::get_ATHENA_hits(int n, 
    char const *file_name="files/ClusterHitSeedRoot.root", char const *tree_name="HitInfo", 
    char const *x_name="hit_x", char const *y_name="hit_y", char const *z_name="hit_z"){
    // Open the ROOT file
    TFile *file = TFile::Open(file_name, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file" << std::endl;
        exit(-1);
    }

    // Retreive vectors
    TTreeReader reader(tree_name, file);
    TTreeReaderValue<std::vector<float>> x(reader, x_name);
    TTreeReaderValue<std::vector<float>> y(reader, y_name);
    TTreeReaderValue<std::vector<float>> z(reader, z_name);
    reader.Next();

    // Get the number of entries in the tree
    Long64_t nATHENA_hits = x -> size();

    // Generate a vector of random indices
    std::vector<int> indices;
    for (int i = 0; i < nATHENA_hits; ++i) { indices.push_back(i); } //create a vector with all the indices in athena hits
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g); //shuffle indices

    // Use random indices to create hit vector in random order
    std::vector<Hit_org::Hit> rd_hits_vec;
    for(int i = 0; i < n; i++){
        rd_hits_vec.push_back(Hit_org::Hit(x->at(i), y->at(i), z->at(i)));
    }

    // Close the file
    file->Close();

    return rd_hits_vec;
}

std::vector<Hit_org::Hit> Hit_org::create_hit_vec(std::vector<float> x, std::vector<float> y, std::vector<float> z){
    //setup for iterating through vectors
    std::vector<Hit_org::Hit> hit_vec;
    auto x_it = x.begin();
    auto y_it = y.begin();
    auto z_it = z.begin();
    auto x_end = x.end();

    //iterate over vectors to create objects and push them
    while(x_it != x_end){
        //create object and add to hit_vec
        Hit_org::Hit cur_hit(*x_it, *y_it, *z_it);
        hit_vec.push_back(cur_hit);

        //iterate through vectors
        x_it++; y_it++; z_it++;
    }

    //error catching
    if ((x_it != x.end()) || (y_it != y.end()) || (z_it != z.end())){
        std::cerr << "Iteration through branches was unsuccessful" << std::endl;
        exit(-1);
    }

    return hit_vec;
}

std::vector<Hit_org::Hit> Hit_org::get_target_hits(int i, char const *file_name="files/VectorRuntime.root", char const *tree_name="TargetHits", char const *x_name="targ_x", char const *y_name="targ_y", char const *z_name="targ_z"){

    //open file and tree
    std::vector<float> *x_vec = nullptr;
    std::vector<float> *y_vec = nullptr;
    std::vector<float> *z_vec = nullptr;
    TFile ResultsFile(file_name, "READ");
    auto tree = ResultsFile.Get<TTree>(tree_name);
    tree -> SetBranchAddress(x_name,&x_vec); tree -> SetBranchAddress(y_name,&y_vec); tree -> SetBranchAddress(z_name,&z_vec);

    // Ensure the entry index is within bounds
    const int n = tree->GetEntries();
    if (i < 0 || i >= n) {
        std::cerr << "Index out of bounds: " << i << std::endl;
        return {};
    }

    // Get branch of interest
    tree->GetEntry(i);

    // Check if vectors are valid
    if (!x_vec || !y_vec || !z_vec) {
        std::cerr << "Error: Vectors not properly set" << std::endl;
        return {};
    }

    return create_hit_vec(*x_vec, *y_vec, *z_vec);
}

int Hit_org::total_ATHENA_hits(char const *file_name = "files/ClusterHitSeedRoot.root", char const *tree_name="HitInfo", 
    char const *x_name="hit_x"){
        TFile *file = TFile::Open(file_name, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file" << std::endl;
        exit(-1);
    }

    // Retreive vectors
    TTreeReader reader(tree_name, file);
    TTreeReaderValue<std::vector<float>> x(reader, x_name);
    reader.Next();
    // Get the number of entries in the tree
    int nATHENA_hits = x -> size();
    file -> Close();
    return nATHENA_hits;
}

void Hit_org::full_search_mc(const char *file_name) {
    std::vector<Hit_org::Hit> targ_vec, hit_vec;

    constexpr int n_targs = NUMB_TARGS;
    constexpr int n_hit_it = NUMB_STEPS;
    int n_hits;
    int total_hits = Hit_org::total_ATHENA_hits();
    std::cout << "Total Athena Hits: " << total_hits << std::endl;
    std::vector<float> rt_vec, sel_x_vec, sel_y_vec, sel_z_vec;

    // Write output to root file
    TFile ResultsFile(file_name, "UPDATE");
    auto ReadTree = ResultsFile.Get<TTree>("TargetHits");

    if (!ReadTree) {
        std::cerr << "Error: TargetHits tree not found." << std::endl;
        return;
    }

    ReadTree->SetBranchStatus("targ_x", 0);
    ReadTree->SetBranchStatus("targ_y", 0);
    ReadTree->SetBranchStatus("targ_z", 0);
    TTree *ResultsTree = new TTree("MyResults", "Search for target hits with runtime efficiencies");

    auto n_hits_branch = ResultsTree->Branch("n_hits", &n_hits);
    auto rt_branch = ResultsTree->Branch("sel_rt", &rt_vec);
    auto sel_x_branch = ResultsTree->Branch("sel_x", &sel_x_vec);
    auto sel_y_branch = ResultsTree->Branch("sel_y", &sel_y_vec);
    auto sel_z_branch = ResultsTree->Branch("sel_z", &sel_z_vec);

    // Loop over every target hit
    for (float j = 0; j < n_hit_it; j++) {

        // Determine number of hits to search through
        n_hits = (total_hits / n_hit_it) * (j+1);
        sel_x_vec.clear(); sel_y_vec.clear(); sel_z_vec.clear(); rt_vec.clear();
        targ_vec = Hit_org::get_target_hits(j);

        // Search through hits for all targets
        for (int i = 0; i < targ_vec.size(); i++) {

            // Randomly generate hit_vec
            hit_vec = Hit_org::get_ATHENA_hits(n_hits);

            // Find closest hit and time function
            auto t1 = std::chrono::high_resolution_clock::now();
            auto sel_hit = find_closest_hit(targ_vec.at(i), hit_vec, n_hits);
            auto t2 = std::chrono::high_resolution_clock::now();
            auto dur = std::chrono::duration<double, std::milli>(t2 - t1);

            // Save data in vector
            sel_x_vec.push_back(sel_hit.x);
            sel_y_vec.push_back(sel_hit.y);
            sel_z_vec.push_back(sel_hit.z);
            rt_vec.push_back(dur.count());
        }

        // Fill the results tree
        ResultsTree->Fill();
        n_hits_branch->Fill(); rt_branch->Fill(); sel_x_branch->Fill(); sel_y_branch->Fill(); sel_z_branch->Fill();
        std::cout << n_hits << " hits searched for " << n_targs << " targets" << std::endl;
    }

    ResultsFile.cd();
    ResultsTree->Write("", TObject::kOverwrite);
    ResultsFile.Close();
}

void Hit_org::gen_rt_graph(const char* file_name) {
    std::cout << "Generating Runtime Graph..." << std::endl;
    TFile file(file_name, "UPDATE");
    
    auto myTree = file.Get<TTree>("MyResults");
    if (!myTree) {
        std::cerr << "Error: MyResults tree not found in file " << file_name << std::endl;
        return;
    }

    std::vector<float> *rt_vec = nullptr;
    int n_hits;
    myTree->SetBranchAddress("sel_rt", &rt_vec);
    myTree->SetBranchAddress("n_hits", &n_hits);

    const int n = myTree->GetEntries();
    std::vector<Double_t> n_hits_arr(n), avg_arr(n), stdev_arr(n);

    for (int i = 0; i < n; ++i) {
        myTree->GetEntry(i);

        if (!rt_vec) {
            std::cerr << "Error: rt_vec is nullptr after GetEntry" << std::endl;
            return;
        }

        n_hits_arr[i] = static_cast<Double_t>(n_hits) / 1000.0;

        double sum = std::accumulate(rt_vec->begin(), rt_vec->end(), 0.0);
        avg_arr[i] = sum / rt_vec->size();

        double sq_sum = std::inner_product(rt_vec->begin(), rt_vec->end(), rt_vec->begin(), 0.0);
        stdev_arr[i] = std::sqrt(sq_sum / rt_vec->size() - avg_arr[i] * avg_arr[i]);
    }

    TGraphErrors gr(n, n_hits_arr.data(), avg_arr.data(), nullptr, stdev_arr.data());
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
    Hit_org::create_targ_root();

    
    //run search with varying number of hits and store in VectorRuntime.root
    Hit_org::full_search_mc("files/VectorRuntime.root");
    Hit_org::gen_rt_graph("files/VectorRuntime.root");

    //report runtime of program
    auto tot_t2 = std::chrono::high_resolution_clock::now();
    auto tot_dur = std::chrono::duration<double, std::milli>(tot_t2-tot_t1);
    std::cout << "Total runtime duration: \t" << tot_dur.count() << "ms" << std::endl;

    return 0;
}