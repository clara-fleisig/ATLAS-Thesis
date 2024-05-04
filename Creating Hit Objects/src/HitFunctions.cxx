#include <vector>
#include <cstdint>
#include <TFile.h>
#include <TTree.h>
#include <iostream> 
#include <vector>
#include "HitFunctions.h"
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <chrono>

bool CheckValue(ROOT::Internal::TTreeReaderValueBase& value) {
   if (value.GetSetupStatus() < 0) {
      std::cerr << "Error " << value.GetSetupStatus()
                << "setting up reader for " << value.GetBranchName() << '\n';
      return false;
   }
   return true;
}

Hit_org::Hit create_hit_obj(std::int64_t i){
    //get pointer to tree
    std::unique_ptr<TFile> myFile(TFile::Open("src/ClusterHitSeedRoot.root"));
    TTree *hitInfoTree = myFile->Get<TTree>("HitInfo");
    if(hitInfoTree == nullptr){
        std::cerr << "TTree::HitInfo does not exist in ClusterHitSeedRoot.root" << std::endl;
        exit(-1);
    }

    //intialise branches
    std::vector<float> *x = nullptr;
    std::vector<float> *y = nullptr;
    std::vector<float> *z = nullptr;
    hitInfoTree -> SetBranchAddress("hit_x", &x);
    hitInfoTree -> SetBranchAddress("hit_y", &y);
    hitInfoTree -> SetBranchAddress("hit_z", &z);
    hitInfoTree -> GetEntry(0);

    //create hit_info object
    Hit_org::Hit my_hit(x->at(i), y->at(i), z->at(i));
    //my_hit.x = x->at(i);
    //my_hit.y = y->at(i);
    //my_hit.z = z->at(i);

    return my_hit;
}

std::vector<Hit_org::Hit> create_hit_vec(TFile *File){
    //read branches of interest into vecotrs of type std::vector<float>
    TTreeReader reader("HitInfo", File);
    TTreeReaderValue<std::vector<float>> x(reader, "hit_x");
    TTreeReaderValue<std::vector<float>> y(reader, "hit_y");
    TTreeReaderValue<std::vector<float>> z(reader, "hit_z");
    reader.Next();

    //check if branches are there
    if (!CheckValue(x) && !CheckValue(y) && !CheckValue(z)) exit(-1);
    
    //setup for itterating through vectors
    std::vector<Hit_org::Hit> hit_vec;
    auto x_it = x -> begin();
    auto y_it = y -> begin();
    auto z_it = z -> begin();
    auto x_end = x -> end();

    //itterate over vectors to create objects and push them
    while(x_it < x_end){
        //create object and add to hit_vec
        Hit_org::Hit cur_hit(*x_it, *y_it, *z_it);
        /*cur_hit.x = *x_it;
        cur_hit.y = *y_it;
        cur_hit.z = *z_it;*/
        hit_vec.push_back (cur_hit);

        //iterate through vectors
        x_it++; y_it++; z_it++;
    }

    //error catching
    if ((x_it != x->end()) || (y_it != y -> end()) || (z_it != z -> end())){
        std::cerr << "Itteration through branches was unsuccessful" << std::endl;
        exit(-1);
    }

    return hit_vec;
}

int main()
{
    TFile *File = TFile::Open("src/ClusterHitSeedRoot.root");

    std::vector<Hit_org::Hit> hit_vec = create_hit_vec(File);
    std::cout << "Hit Vector Size: " << hit_vec.size() << '\n';

    //auto t1 = high_resolution_clock::now();
    //auto t2 = high_resolution_clock::now();

    Hit_org::Hit myhit = create_hit_obj(0);
    myhit.show_Hit();

    return 0;
}