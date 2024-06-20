#include <string>     // std::string, std::stoull
#include <cmath>

std::vector<TString> tokenizeStr(TString str, TString key);


void convertGeoCSVRoot()
{

    ifstream myfile ("DetectorGeo_ACTS_ITK_new.csv");
    TString outFileName ("DetectorGeo_ACTS_ITK_new.root");
    TFile* file = new TFile(outFileName, "recreate");
    TTree* tree = new TTree("DetectorGeo", "DetectorGeo");

    unsigned long long geometry_id;
    int volume_id    ;
    int boundary_id  ;
    int layer_id     ;
    int module_id    ;
    int pixOrSCT     ;
    float cx           ;
    float cy           ;
    float cz           ;
    float cr           ;
    float crho         ;
    float rot_xu       ;
    float rot_xv       ;
    float rot_xw       ;
    float rot_yu       ;
    float rot_yv       ;
    float rot_yw       ;
    float rot_zu       ;
    float rot_zv       ;
    float rot_zw       ;
    float bounds_type  ;
    float bound_param0 ;
    float bound_param1 ;
    float bound_param2 ;
    float bound_param3 ;
    float bound_param4 ;
    float bound_param5 ;
    float bound_param6 ;

    float module_thickness = 0.03 ;
    float module_minhu ;
    float module_maxhu ;
    float module_hv ;
    float module_pitch_u = 0.0001;
    float module_pitch_v = 0.0001;

    tree->Branch("module_index",    &geometry_id);
    tree->Branch("module_volumeId", &volume_id);
    tree->Branch("module_layerId",  &layer_id);
    tree->Branch("module_moduleId", &module_id);
    tree->Branch("module_PixorSCT", &pixOrSCT);
    tree->Branch("module_center_x", &cx);
    tree->Branch("module_center_y", &cy);
    tree->Branch("module_center_z", &cz);
    tree->Branch("module_center_r", &cr);
    tree->Branch("module_center_rho", &crho);

    tree->Branch("module_rot_xu", &rot_xu);
    tree->Branch("module_rot_xv", &rot_xv);
    tree->Branch("module_rot_xw", &rot_xw);
    tree->Branch("module_rot_yu", &rot_yu);
    tree->Branch("module_rot_yv", &rot_yv);
    tree->Branch("module_rot_yw", &rot_yw);
    tree->Branch("module_rot_zu", &rot_zu);
    tree->Branch("module_rot_zv", &rot_zv);
    tree->Branch("module_rot_zw", &rot_zw);


    tree->Branch("module_thickness",    &module_thickness);
    tree->Branch("module_minhu",        &module_minhu);
    tree->Branch("module_maxhu",        &module_maxhu);
    tree->Branch("module_hv",           &module_hv);
    tree->Branch("module_pitch_u",      &module_pitch_u);
    tree->Branch("module_pitch_v",      &module_pitch_v);


    string line;
    int index = 0;
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {

            TString sLine(line);
            if(sLine.Contains("geometry")) continue;

            auto varParts = tokenizeStr(sLine, ",");

            geometry_id  = std::stoull(varParts.at(0).Data());
            volume_id    = atoi(varParts.at(1));
            boundary_id  = atoi(varParts.at(2));
            layer_id     = atoi(varParts.at(3));
            module_id    = atoi(varParts.at(4));
            cx           = atof(varParts.at(5));
            cy           = atof(varParts.at(6));
            cz           = atof(varParts.at(7));
            rot_xu       = atof(varParts.at(8));
            rot_xv       = atof(varParts.at(9));
            rot_xw       = atof(varParts.at(10));
            rot_yu       = atof(varParts.at(11));
            rot_yv       = atof(varParts.at(12));
            rot_yw       = atof(varParts.at(13));
            rot_zu       = atof(varParts.at(14));
            rot_zv       = atof(varParts.at(15));
            rot_zw       = atof(varParts.at(16));
            bounds_type  = atof(varParts.at(17));
            bound_param0 = atof(varParts.at(18));
            bound_param1 = atof(varParts.at(19));
            bound_param2 = atof(varParts.at(20));
            bound_param3 = atof(varParts.at(21));
            bound_param4 = atof(varParts.at(22));
            bound_param5 = atof(varParts.at(23));
            bound_param6 = atof(varParts.at(24));   

            // HGTD
            // if(volume_id == 2 || volume_id == 25) continue;
            // if(volume_id <= 20) continue;
            // // if(volume_id == 23) continue;

            // // if(volume_id == 24 || volume_id == 22)
            // // {
            // //     continue;
            // // }

            
            if(bounds_type == 3 || bounds_type == 1) // Cylinder
            {
                continue;
            } 
            if(bounds_type == 6) // rectangle
            {
                module_minhu = std::abs(bound_param0);
                module_maxhu = module_minhu;
                module_hv    = std::abs(bound_param1);
            }  
            else if(bounds_type == 7) // trapazoid
            {
                module_minhu = bound_param0;
                module_maxhu = bound_param1;
                module_hv    = bound_param2;
            }      
            else if(bounds_type == 11) // annulus
            {
                module_minhu = bound_param0;
                module_maxhu = bound_param1;
                module_hv    = bound_param2;
                //cx *= 100;
                //cy *= 100;
                double nX = cx * rot_xu + cy * rot_xv + cz * rot_xw;
                double nY = cx * rot_yu + cy * rot_yv + cz * rot_yw;
                double nZ = cx * rot_zu + cy * rot_zv + cz * rot_zw;
                cx = nX;
                cy = nY;
                cz = nZ;
            }    
            else
            {
                module_minhu = bound_param0;
                module_maxhu = bound_param1;
                module_hv    = bound_param2;
                cout<<sLine<<endl;
                std::cout<<"Do not recongize bound type "<<bounds_type<<" volume: "<<volume_id<<endl;
                // continue;
            } 


            cr = std::sqrt(cx*cx + cy*cy + cz*cz);         
            crho = std::sqrt(cx*cx + cy*cy);  

            tree->Fill();
        }
        myfile.close();
    }

    // tree->Draw("module_center_x:module_center_y:module_center_z");

    tree->Write();
    file->Close();

}

std::vector<TString> tokenizeStr(TString str, TString key)
{
    TObjArray *Varparts = str.Tokenize(key);
    vector<TString> varNameVec;
    if(Varparts->GetEntriesFast()) {
        TIter iString(Varparts);
        TObjString* os=0;
        while ((os=(TObjString*)iString())) {
            varNameVec.push_back(os->GetString());
        }
    }
    return varNameVec;
}
