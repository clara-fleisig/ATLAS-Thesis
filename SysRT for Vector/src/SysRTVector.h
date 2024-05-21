#pragma once
#include <cstdint>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>

//define number of target hits to be generated
#define NUMB_TARGS 100;

//define range in which 
#define RHO_MAX 1014;
#define RHO_MIN 33;
#define PHI_MAX 2*3.141593;
#define PHI_MIN 0;
#define Z_MAX 2860;
#define Z_MIN -2860;

namespace Hit_org{
    class Hit{
        public:
            //attributes
            float x;
            float y;
            float z;

            // Constructor with parameters
            Hit(float my_x, float my_y, float my_z) {
                x = my_x;
                y = my_y;
                z = my_z;
            }

            //show_Hit method
            void show_Hit(){
                std::cout << "x: " << x << "\t" << "y: " << y << "\t" << "z: " << z << "\t" << std::endl;
            }

    };
    
    //create root file with randomly generated target hits
    void create_targ_root(char const *filename);

    //load root file info into a vector of hits
    std::vector<Hit_org::Hit> create_hit_vec(char const *filename, char const *treename, char const *xname, char const *yname, char const *zname);
    
    //do mc simulation of brute force method and save runtime results in root file
    void full_search_mc(std::vector<Hit_org::Hit> targ_vec, std::vector<Hit_org::Hit> hit_vec, const char *filename);
}