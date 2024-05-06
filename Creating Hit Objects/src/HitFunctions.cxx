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
    while(x_it != x_end){
        //create object and add to hit_vec
        Hit_org::Hit cur_hit(*x_it, *y_it, *z_it);
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

void hit_vec_runtime(std::vector<Hit_org::Hit> hit_vec){
    auto hit_vec_size = hit_vec.size();

    auto t1_beg = std::chrono::high_resolution_clock::now();
        hit_vec.begin() -> show_Hit();
    auto t2_beg = std::chrono::high_resolution_clock::now();
    auto beg_dur = std::chrono::duration<double, std::micro>(t2_beg-t1_beg);

    auto t1_beg_un_wc = std::chrono::high_resolution_clock::now();
        auto beg_un_wc_it = hit_vec.end();
        for(int i = 0; i < hit_vec_size; i++){
            beg_un_wc_it--;
        }
        beg_un_wc_it -> show_Hit();
    auto t2_beg_un_wc = std::chrono::high_resolution_clock::now();
    auto beg_un_wc_dur = std::chrono::duration<double, std::micro>(t2_beg_un_wc-t1_beg_un_wc);

    auto t1_beg_un = std::chrono::high_resolution_clock::now();
        auto beg_un_it = hit_vec.end();
        for(int i = 0; i < hit_vec_size; i++){
            beg_un_it -> x;
            beg_un_it -> y;
            beg_un_it -> z;
            beg_un_it--;
        }
        beg_un_it -> show_Hit();
    auto t2_beg_un = std::chrono::high_resolution_clock::now();
    auto beg_un_dur = std::chrono::duration<double, std::micro>(t2_beg_un-t1_beg_un);
    
    auto t1_end = std::chrono::high_resolution_clock::now();
        auto end_it = hit_vec.end();
        end_it--;
        end_it -> show_Hit();
    auto t2_end = std::chrono::high_resolution_clock::now();
    auto end_dur = std::chrono::duration<double, std::micro>(t2_end-t1_end);

    auto t1_end_ua = std::chrono::high_resolution_clock::now();
        auto end_ua_it = hit_vec.begin();
        std::advance(end_ua_it, hit_vec_size-1);
        end_ua_it -> show_Hit();
    auto t2_end_ua = std::chrono::high_resolution_clock::now();
    auto end_ua_dur = std::chrono::duration<double, std::micro>(t2_end_ua-t1_end_ua);

    auto t1_end_un_wc = std::chrono::high_resolution_clock::now();
        auto end_un_wc_it = hit_vec.begin();
        for(int i = 0; i < (hit_vec_size-1); i++){
            end_un_wc_it++;
        }
        end_un_wc_it -> show_Hit();
    auto t2_end_un_wc = std::chrono::high_resolution_clock::now();
    auto end_un_wc_dur = std::chrono::duration<double, std::micro>(t2_end_un_wc-t1_end_un_wc);

    auto t1_end_un = std::chrono::high_resolution_clock::now();
        auto end_un_it = hit_vec.begin();
        for(int i = 0; i < (hit_vec_size-1); i++){
            end_un_it -> x;
            end_un_it -> y;
            end_un_it -> z;
            end_un_it++;
        }
        end_un_it -> show_Hit();
    auto t2_end_un = std::chrono::high_resolution_clock::now();
    auto end_un_dur = std::chrono::duration<double, std::micro>(t2_end_un-t1_end_un);

    std::cout << std::endl;
    std::cout << "hit_vec.begin():" << "\t\t\t\t" << beg_dur.count() << "us" << std::endl;
    std::cout << "hit_vec.beg() using next:" << "\t\t\t" << beg_un_dur.count() << "us" << std::endl;
    std::cout << "hit_vec.beg() using next with mem access:" << "\t" << beg_un_wc_dur.count() << "us" << std::endl;
    
    std::cout << std::endl;
    std::cout << "hit_vec.end():" << "\t\t\t\t\t" << end_dur.count() << "us" << std::endl;
    std::cout << "hit_vec.end() using advance:" << "\t\t\t" << end_ua_dur.count() << "us" << std::endl;
    std::cout << "hit_vec.end() using next:" << "\t\t\t" << end_un_dur.count() << "us" << std::endl;
    std::cout << "hit_vec.end() using next with mem access:" << "\t" << end_un_wc_dur.count() << "us" << std::endl;
}

int main()
{
    TFile *File = TFile::Open("src/ClusterHitSeedRoot.root");

    //vector objects are allocated on heap
    std::vector<Hit_org::Hit> hit_vec = create_hit_vec(File);
    hit_vec_runtime(hit_vec);

    //pull a specific hit only from root file
    Hit_org::Hit myhit = create_hit_obj(0);
    myhit.show_Hit();

    return 0;
}