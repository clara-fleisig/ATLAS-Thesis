#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <iostream> 

namespace hit_info {
    struct Hit
    {
        float x;
        float y;
        float z;
    };
}

hit_info::Hit create_hit_obj(std::int64_t i){
    //get point to tree
    std::unique_ptr<TFile> myFile(TFile::Open("ClusterHitSeedRoot.root"));
    auto hitInfoTree = myFile->Get<TTree>("HitInfo");
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
    hit_info::Hit my_hit;
    my_hit.x = x->at(i);
    my_hit.y = y->at(i);
    my_hit.z = z->at(i);

    return my_hit;
}