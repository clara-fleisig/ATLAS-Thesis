#pragma once
#include <cstdint>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>

//define number of target hits to be generated
#define NUMB_TARGS 100;
#define NUMB_STEPS 15;

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
    void create_targ_root(char const *file_name);
    std::vector<Hit_org::Hit> get_ATHENA_hits(int n, char const *file_name, char const *tree_name, char const *x_name, char const *y_name, char const *z_name);
    std::vector<Hit_org::Hit> create_hit_vec(std::vector<float> x, std::vector<float> y, std::vector<float> z);
    float calc_dis(Hit_org::Hit hit1, Hit_org::Hit hit2);
    Hit_org::Hit find_closest_hit(Hit_org::Hit targ_hit, std::vector<Hit_org::Hit> hit_vec, const int n_hits);
    void full_search_mc(const char *filename);
    std::vector<Hit_org::Hit> get_target_hits(int i, char const *file_name, char const *tree_name, char const *x_name, char const *y_name, char const *z_name);
    int total_ATHENA_hits(char const *file_name, char const *tree_name, char const *x_name);
    void gen_rt_graph(const char* file_name);
}