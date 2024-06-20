//hitInfoTree->Draw("hit_x:hit_moduleID", c1&&c2)
//c2 = "hit_volumeID==8"

void colormap_2d(const char *x_str, const char *y_str, const char *col_str){
    //load in filename and tree
    std::unique_ptr<TFile> myFile(TFile::Open("ClusterHitSeedRoot.root"));
    auto hitInfoTree = myFile->Get<TTree>("HitInfo");

    vector<float> *x = nullptr; 
    vector<float> *y = nullptr; 
    vector<float> *col = nullptr;

    hitInfoTree -> SetBranchAddress(x_str, &x); 
    hitInfoTree -> SetBranchAddress(y_str, &y); 
    hitInfoTree -> SetBranchAddress(col_str, &col);
    hitInfoTree -> GetEntry(0);

    //determine length of branches
    int const n = x -> size();

    //setup graph on canvas
    auto c = new TCanvas("c","Graph2D example",0,0,700,600);
    auto gr = new TGraph2D();

    //add data points
    for (int i=0; i<n; i++){gr->SetPoint(i, x->at(i), y->at(i), col->at(i));}

    gr->Draw("pcolz");
}

void graph_3d(const char *x_str, const char *y_str, const char *z_str){
    //load in filename and tree
    std::unique_ptr<TFile> myFile(TFile::Open("ClusterHitSeedRoot.root"));
    auto hitInfoTree = myFile->Get<TTree>("HitInfo");

    vector<float> *x = nullptr;
    vector<float> *y = nullptr;
    vector<float> *z = nullptr;

    //point pointer variables to start of branches
    hitInfoTree -> SetBranchAddress(x_str, &x);
    hitInfoTree -> SetBranchAddress(y_str, &y);
    hitInfoTree -> SetBranchAddress(z_str, &z);
    hitInfoTree -> GetEntry(0);

    //determine length of branches
    int const n = x -> size();

    //setup graph on canvas
    auto c = new TCanvas("c","Graph2D example",0,0,700,600);
    auto gr = new TGraph2D();

    //add title
    char title[55];
    strcpy(title, x_str);
    strcat(title, " vs. ");
    strcat(title, y_str);
    strcat(title, " vs. ");
    strcat(title, z_str);
    gr->SetTitle(title);

    //add data points
    for (int i=0; i<n; i++){gr->SetPoint(i, x->at(i), y->at(i), z->at(i));}

    gr->SetMarkerStyle(7);
    gr->GetXaxis()->SetTitle(x_str);
    gr->GetYaxis()->SetTitle(y_str);
    gr->GetZaxis()->SetTitle(z_str);
    gr->Draw("AP");
}

void graph_2d(const char *x_str, const char *y_str)
{
    //load in filename and tree
    std::unique_ptr<TFile> myFile(TFile::Open("ClusterHitSeedRoot.root"));
    auto hitInfoTree = myFile->Get<TTree>("HitInfo");

    //set up pointer variables for branches
    vector<float> *x = nullptr;
    vector<float> *y = nullptr;

    //point pointer variables to start of branches
    hitInfoTree -> SetBranchAddress(x_str, &x);
    hitInfoTree -> SetBranchAddress(y_str, &y);
    hitInfoTree -> GetEntry(0);
    
    //determine length of branches
    const uint n = x -> size();

    //setup graph on canvas
    auto c = new TCanvas();
    c->SetCanvasSize(500, 500);
    auto gr = new TGraph();

    //add title
    char title[35];
    strcpy(title, x_str);
    strcat(title, " vs. ");
    strcat(title, y_str);
    gr->SetTitle(title);
    
    //add data from branches
    for (uint i=0; i<n; i++){gr->AddPoint(x->at(i), y->at(i));}
    
    //set subtitiles, marker style and plot
    gr->SetMarkerStyle(7);
    gr->GetXaxis()->SetTitle(x_str);
    gr->GetYaxis()->SetTitle(y_str);
    gr->Draw("AP");

    //save canvas as jpg
    strcat(title, ".png");
    c->SaveAs(title);
}

void mg_graph_2d(TMultiGraph *mg, const char *x_str, const char *y_str)
{
    //TMultiGraph *mg = new TMultiGraph();
    //load in filename and tree
    std::unique_ptr<TFile> myFile(TFile::Open("ClusterHitSeedRoot.root"));
    auto hitInfoTree = myFile->Get<TTree>("HitInfo");
    
    //create main graph
    auto gr = new TGraph();
    vector<float> *x = nullptr;
    vector<float> *y = nullptr;
    hitInfoTree -> SetBranchAddress(x_str, &x);
    hitInfoTree -> SetBranchAddress(y_str, &y);
    hitInfoTree -> GetEntry(0);
    uint const n = x -> size();
    for (uint i=0; i<n; i++){gr->AddPoint(x->at(i), y->at(i));}
    
    //add settings and add to mg
    gr->SetMarkerStyle(7);
    mg -> Add(gr, "AP");
}

void mg_cut_graph_2d(const int lb, const int ub, TMultiGraph *mg, const char *x_str, const char *y_str, const char *crit_str, const int colour_id)
{
    //TMultiGraph *mg = new TMultiGraph();
    //load in filename and tree
    std::unique_ptr<TFile> myFile(TFile::Open("ClusterHitSeedRoot.root"));
    auto hitInfoTree = myFile->Get<TTree>("HitInfo");
    
    //create main graph
    auto gr = new TGraph();
    vector<float> *x = nullptr;
    vector<float> *y = nullptr;
    vector<float> *crit = nullptr;
    hitInfoTree -> SetBranchAddress(x_str, &x);
    hitInfoTree -> SetBranchAddress(y_str, &y);
    hitInfoTree -> SetBranchAddress(crit_str, &crit);
    hitInfoTree -> GetEntry(0);
    uint const n = x -> size();
    
    for (uint i=0; i<n; i++){
        if(((crit->at(i))>=lb) && ((crit->at(i))<ub)){
            gr->AddPoint(x->at(i), y->at(i));
            }
    }
    
    //add settings and add to mg
    gr->SetMarkerStyle(7);
    gr->SetMarkerColor(colour_id);
    mg -> Add(gr, "AP");
}

void my_func(){
   // setup
    vector<float> *x = nullptr;
    vector<float> *y = nullptr;
    vector<float> *z = nullptr;
    vector<float> *rho = nullptr;
    vector<float> *moduleID = nullptr;
    vector<float> *volumeID = nullptr;
    vector<float> *layerID = nullptr;
    vector<float> *etaModule = nullptr;
    vector<float> *phiModule = nullptr;
    hitInfoTree -> SetBranchAddress("hit_x", &x);
    hitInfoTree -> SetBranchAddress("hit_y", &y);
    hitInfoTree -> SetBranchAddress("hit_z", &z);
    hitInfoTree -> SetBranchAddress("hit_rho", &rho);
    hitInfoTree -> SetBranchAddress("hit_moduleID", &moduleID);
    hitInfoTree -> SetBranchAddress("hit_volumeID", &volumeID);
    hitInfoTree -> SetBranchAddress("hit_layerID", &layerID);
    hitInfoTree -> SetBranchAddress("hit_etaModule", &etaModule);
    hitInfoTree -> SetBranchAddress("hit_phiModule", &phiModule);
    hitInfoTree -> GetEntry(0);
    uint const n = x -> size();

    std::unique_ptr<TFile> myFile(TFile::Open("ClusterHitSeedRoot.root"));
    auto hitInfoTree = myFile->Get<TTree>("HitInfo");
    auto c = new TCanvas("c","Graph2D example",0,0,700,600);
    
    //gr->SetTitle(title);
    //hitInfoTree->Draw("hit_x:hit_y:hit_z:hit_moduleID","","COLZ");
}