#pragma once
#include <cstdint>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>

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
    
    //std::vector<Hit_org::Hit> create_hit_vec(TFile *File);
}