#include "theMVAtool.h"
#include "Func_other.h"

#include <cassert> 	//Can be used to terminate program if argument is not true. Ex : assert(test > 0 && "Error message");
#include <sys/stat.h> // to be able to use mkdir

#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"
#include "TString.h"
#include "TLegend.h"
#include "TRandom.h"
#include "TLatex.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TObject.h"
#include "TRandom3.h"
#include "TObjArray.h"

//-----------------------------------------------------------------------------------------
//    _____   __  __  __     __     _         ____    ___    ____    _____
//   |_   _| |  \/  | \ \   / /    / \       / ___|  / _ \  |  _ \  | ____|
//     | |   | |\/| |  \ \ / /    / _ \     | |     | | | | | | | | |  _|
//     | |   | |  | |   \ V /    / ___ \    | |___  | |_| | | |_| | | |___
//     |_|   |_|  |_|    \_/    /_/   \_\    \____|  \___/  |____/  |_____|
//
//-----------------------------------------------------------------------------------------

using namespace std;

//-----------------------------------------------------------------------------------------
//    _           _   _     _           _   _                 _     _
//   (_)  _ __   (_) | |_  (_)   __ _  | | (_)  ____   __ _  | |_  (_)   ___    _ __
//   | | | '_ \  | | | __| | |  / _` | | | | | |_  /  / _` | | __| | |  / _ \  | '_ \
//   | | | | | | | | | |_  | | | (_| | | | | |  / /  | (_| | | |_  | | | (_) | | | | |
//   |_| |_| |_| |_|  \__| |_|  \__,_| |_| |_| /___|  \__,_|  \__| |_|  \___/  |_| |_|
//
//-----------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Default Constructor
theMVAtool::theMVAtool()
{
  cout<<BOLD(FRED("### ERROR : USE THE OVERLOADED CONSTRUCTOR ! ###"))<<endl;
  stop_program = true;
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Overloaded Constructor
theMVAtool::theMVAtool(std::vector<TString > thevarlist, std::vector<TString > thesamplelist, std::vector<TString > thesamplelist_forreading, std::vector<TString > thechanlist, std::vector<TString > set_v_cut_name, std::vector<TString > set_v_cut_def, std::vector<bool > set_v_cut_IsUsedForBDT, int nofbin_templates = 5 , string PlaceOfTuples_ = "", TString region_name_ = "")

{
  PlaceOfTuples = PlaceOfTuples_;
  region_name = region_name_;
  
  for(int i=0; i<thechanlist.size(); i++)
  {
    channel_list.push_back(thechanlist[i]);
  }
  for(int i=0; i<thevarlist.size(); i++)
  {
    
    var_list.push_back(thevarlist[i]);
    vec_variables.push_back(0);
    
  }
  
  for(int i=0; i<set_v_cut_name.size(); i++)
  {
    v_cut_name.push_back(set_v_cut_name[i]);
    v_cut_def.push_back(set_v_cut_def[i]);
    v_cut_IsUsedForBDT.push_back(set_v_cut_IsUsedForBDT[i]);
    v_cut_float.push_back(-999);
  }
  for(int i=0; i<thesamplelist.size(); i++)
  {
    sample_list.push_back(thesamplelist[i]);
    sample_listread.push_back(thesamplelist[i]);
  }
  for(int i=0; i<thesamplelist_forreading.size(); i++)
  {
    //sample_listread.push_back(thesamplelist[i]);
    sample_listread.push_back(thesamplelist_forreading[i]);
  }
  for(int isample=0; isample<sample_listread.size(); isample++)
  {
    
    //TString inputfile = PlaceOfTuples+"MVA_tree_" + sample_listread[isample] + "_80X.root";
    cout << PlaceOfTuples+"MVA_tree_" + sample_listread[isample] + "_80X.root" << endl;
  }
  reader = new TMVA::Reader( "!Color:!Silent" );
  
  nbin = nofbin_templates;
  
  stop_program = false;
  
  //Make sure that the "equal to" sign is written properly
  for(int ivar=0; ivar<v_cut_name.size(); ivar++)
  {
    if( v_cut_def[ivar].Contains("=") && !v_cut_def[ivar].Contains("==") && !v_cut_def[ivar].Contains("<") && !v_cut_def[ivar].Contains(">") )
    {
      v_cut_def[ivar] = "==" + Convert_Number_To_TString(Find_Number_In_TString(v_cut_def[ivar]));
      cout<<endl<<BOLD(FBLU("##################################"))<<endl;
      cout<<"--- Changed cut on "<<v_cut_name[ivar]<<" to: "<<v_cut_def[ivar]<<" ---"<<endl;
      cout<<BOLD(FBLU("##################################"))<<endl<<endl;
    }
  }
  
  //Store the "cut name" that will be written as a suffix in the name of each output file
  filename_suffix = "";
  TString tmp = "";
  for(int ivar=0; ivar<v_cut_name.size(); ivar++)
  {
    if(v_cut_def[ivar] != "")
    {
      if(!v_cut_def[ivar].Contains("&&")) //Single condition
      {
        tmp = "_" + v_cut_name[ivar] + Convert_Sign_To_Word(v_cut_def[ivar]) + Convert_Number_To_TString(Find_Number_In_TString(v_cut_def[ivar]));
        filename_suffix+= tmp;
      }
      else //Double condition
      {
        TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first, cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
        tmp+= "_" + v_cut_name[ivar] + Convert_Sign_To_Word(cut1) + Convert_Number_To_TString(Find_Number_In_TString(cut1));
        tmp+= "_" + v_cut_name[ivar] + Convert_Sign_To_Word(cut2) + Convert_Number_To_TString(Find_Number_In_TString(cut2));
        filename_suffix+= tmp;
      }
    }
  }
}



//-----------------------------------------------------------------------------------------
//    _                    _           _
//   | |_   _ __    __ _  (_)  _ __   (_)  _ __     __ _
//   | __| | '__|  / _` | | | | '_ \  | | | '_ \   / _` |
//   | |_  | |    | (_| | | | | | | | | | | | | | | (_| |
//    \__| |_|     \__,_| |_| |_| |_| |_| |_| |_|  \__, |
//                                                 |___/
//-----------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Train, test and evaluate the BDT with signal and bkg MC
void theMVAtool::Train_Test_Evaluate(TString channel, TString bdt_type = "BDT")
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("---TRAINING ---")<<endl;
  cout<<FYEL("---TEMPLATE ---")<< bdt_type << endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  
  //---------------------------------------------------------------
  // This loads the TMVA librar
  
  TMVA::Tools::Instance();
  placeOutputTraining = "output";
  mkdir(placeOutputTraining,0777);
  placeOutputTraining += "/training";
  mkdir(placeOutputTraining,0777);
  placeOutputTraining += "/" + bdt_type;
  mkdir(placeOutputTraining,0777);
  //placeOfWeights = placeOutputTraining + "/weights/";"
  //mkdir(placeOfWeights,0777);
  TString output_file_name = placeOutputTraining+"/" + bdt_type;
  if(channel != "") {output_file_name+= "_" + channel;}
  output_file_name+= filename_suffix;
  output_file_name+= ".root";
  TFile* output_file = TFile::Open( output_file_name, "RECREATE" );
  
  // Create the factory object
  //TMVA::Factory* factory = new TMVA::Factory(bdt_type.Data(), output_file, "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" ); //some Transformations can trigger warnings
  TMVA::Factory* factory = new TMVA::Factory(bdt_type.Data(), output_file, "!V:!Silent:Color:!DrawProgressBar:Transformations=I:AnalysisType=Classification" );
  
  // Define the input variables that shall be used for the MVA training
  for(int i=0; i<var_list.size(); i++)
  {
    //if(!var_list[i].Contains("nJet") && !var_list[i].Contains("NJet") && !var_list[i].Contains("Charge") ) factory->AddVariable(var_list_all[i].Data(), 'F');
    //else
    factory->AddVariable(var_list[i].Data(), 'F');
  }
  //Choose if the cut variables are used in BDT or not
  for(int i=0; i<v_cut_name.size(); i++)
  {
    cout<<"Is "<<v_cut_name[i]<<" used ? "<<(v_cut_IsUsedForBDT[i] );
    
    if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("==")) {
      cout << " -- variable " << endl;
      factory->AddVariable(v_cut_name[i].Data(), 'F');
    }
    else {
      factory->AddSpectator(v_cut_name[i].Data(), v_cut_name[i].Data(), 'F');
      cout << " -- spectator " << endl;
    }
  }
  
  TFile *f(0);
  
  for(int isample=0; isample<sample_list.size(); isample++)
  {
    // Read training and test data
    // --- Register the training and test trees
    TString inputfile;
    
    inputfile = PlaceOfTuples + "MVA_tree_" + sample_list[isample] + "_80X.root";
    // cout << "input file: " << inputfile << endl;
    
    TFile* file_input = TFile::Open( inputfile.Data() );
    
    TTree* tree = (TTree*)file_input->Get("mvatree");
    
    // global event weights per tree (see below for setting event-wise weights)
    Double_t signalWeight     = 1.0;
    Double_t backgroundWeight = 1.0;
    cout << " sample " << sample_list[isample] << endl;
    
    // You can add an arbitrary number of signal or background trees
    //NB : can't account for luminosity rescaling here, but it is not necessary for the training (only relative weights matter ?)
    if(!bdt_type.Contains("fake")){
      if(sample_list[isample].Contains("FCNC") ){factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "MVA_weight" );}
      else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "MVA_weight" );}
    }
    else{
      if(sample_list[isample].Contains("FCNC") ){factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "MVA_weight" );}
      else {factory->AddBackgroundTree( tree, backgroundWeight );}
      
    }
  }
  
  
  // Apply additional cuts on the signal and background samples (can be different)
  TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
  TCut mycutb = ""; // for example: TCut mycutb = "abs(var1)<0.5";
  
  if(channel != "all")
  {
    if(channel == "uuu")         		{mycuts = "MVA_channel==0"; mycutb = "MVA_channel==0";}
    else if(channel == "uue" )     		{mycuts = "MVA_channel==1"; mycutb = "MVA_channel==1";}
    else if(channel == "eeu"  )     	{mycuts = "MVA_channel==2"; mycutb = "MVA_channel==2";}
    else if(channel == "eee"   )     	{mycuts = "MVA_channel==3"; mycutb = "MVA_channel==3";}
    else 								{cout << "WARNING : wrong channel name while training " << endl;}
  }
  
  //--------------------------------
  //--- Apply cuts during training
  TString tmp = "";
  
  for(int ivar=0; ivar<v_cut_name.size(); ivar++)
  {
    if(v_cut_def[ivar] != "")
    {
      if(!v_cut_def[ivar].Contains("&&")) {tmp+= v_cut_name[ivar] + v_cut_def[ivar];} //If cut contains only 1 condition
      else //If 2 conditions in the cut, break it in 2
      {
        tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).first;
        tmp+= " && ";
        tmp+= v_cut_name[ivar] + Break_Cuts_In_Two(v_cut_def[ivar]).second;
      }
    }
    
    //Complicated way of concatenating the TStrings
    if((ivar+1) < v_cut_name.size() && v_cut_def[ivar+1] != "")
    {
      for(int i=0; i<ivar+1; i++)
      {
        if(v_cut_def[i] != "") {tmp += " && "; break;}
      }
    }
    
    //cout<<"tmp = "<<tmp<<endl;
  }
  
  cout<<"Total cut chain : "<<tmp<<endl;
  
  if(tmp != "") {mycuts+= tmp; mycutb+= tmp;}
  
  
  //--------------------------------
  
  // Tell the factory how to use the training and testing events    //
  // If no numbers of events are given, half of the events in the tree are used for training, and the other half for testing:
  factory->PrepareTrainingAndTestTree( mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );
  cout << "prepare trees" << endl;
  
  TString method_title = channel + filename_suffix; //So that the output weights are labelled differently for each channel
  
  // Boosted Decision Trees // Adaptive Boost
  //factory->BookMethod( TMVA::Types::kBDT, method_title.Data(),    "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );
  //	factory->BookMethod( TMVA::Types::kBDT, method_title.Data(), "!H:!V:NTrees=100:MinNodeSize=15:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:IgnoreNegWeightsInTraining=True" );
  // Isis
  /*if(region_name.Contains("toppair") factory->BookMethod( TMVA::Types::kBDT,method_title.Data(),"!H:!V:Ntrees=25:MinNodeSize=5%:MaxDepth=3:BoostType=Grad:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:NegWeightTreatment=Pray:Shrinkage=0.5");
   else if(region_name.Contains("toppair")factory->BookMethod( TMVA::Types::kBDT,method_title.Data(),"!H:!V:Ntrees=25:MinNodeSize=2.5%:MaxDepth=3:BoostType=Adaboost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:NegWeightTreatment=Pray:"); // ST method*/
  if(region_name.Contains("toppair") ) factory->BookMethod( TMVA::Types::kBDT,method_title.Data(),"!H:!V:NTrees=250:MinNodeSize=5%:BoostType=Grad:Shrinkage=0.2:UseBaggedBoost:BaggedSampleFraction=0.8:SeparationType=GiniIndex:nCuts=15:MaxDepth=3:NegWeightTreatment=Pray" );
  else if(region_name.Contains("singletop") ) factory->BookMethod( TMVA::Types::kBDT,method_title.Data(),"!H:!V:NTrees=25:MinNodeSize=5%:BoostType=Grad:Shrinkage=0.3::UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=15:MaxDepth=3:NegWeightTreatment=Pray:PruneMethod=NoPruning" );
  // cout << "set weights directory " << placeOfWeights << endl;
  output_file->cd();
  cout << "in outputfile " << output_file_name << endl;
  // Train MVAs using the set of training events
  factory->TrainAllMethods();
  cout << "training " << endl;
  // ---- Evaluate all MVAs using the set of test events
  factory->TestAllMethods();
  // ----- Evaluate and compare performance of all configured MVAs
  factory->EvaluateAllMethods();
  //NB : Test & Evaluation recap in the output files
  
  // --------------------------------------------------------------
  
  // Save the output
  output_file->Close();
  
  std::cout << "==> Wrote root file: " << output_file->GetName() << std::endl;
  std::cout << "==> TMVA is done!" << std::endl;
  
  delete factory;
  
  // Launch the GUI for the root macros    //NB : write interactively in the ROOT environment --> TMVA::TMVAGui("output.root")
  //TMVA::TMVAGui(output_file_name);
}


//-----------------------------------------------------------------------------------------
//                              _   _
//    _ __    ___    __ _    __| | (_)  _ __     __ _
//   | '__|  / _ \  / _` |  / _` | | | | '_ \   / _` |
//   | |    |  __/ | (_| | | (_| | | | | | | | | (_| |
//   |_|     \___|  \__,_|  \__,_| |_| |_| |_|  \__, |
//                                              |___/
//-----------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

//Reader function. Uses output from training (weights, ...) and read samples to create distributions of the BDT discriminant *OR* mWt
void theMVAtool::Read(TString template_name)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("--- Producing "<<template_name<<" Templates ---")<<endl;
  
  placeOutputReading = "output";
  mkdir(placeOutputReading, 0777);
  placeOutputReading += "/read";
  mkdir(placeOutputReading, 0777);
  placeOutputReading += "/" + template_name;
  mkdir(placeOutputReading, 0777);
  
  
  
  
  reader = new TMVA::Reader( "!Color:!Silent" );
  
  // Name & adress of local variables which carry the updated input values during the event loop
  // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
  for(int i=0; i<var_list.size(); i++)
  {
    reader->AddVariable(var_list[i].Data(), &vec_variables[i]); //cout<<"Added variable "<<var_list[i]<<endl;
  }
  for(int i=0; i<v_cut_name.size(); i++)
  {
    if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("==")) {
      reader->AddVariable(v_cut_name[i].Data(), &v_cut_float[i]);
    }
    else {reader->AddSpectator(v_cut_name[i].Data(), &v_cut_float[i]);}
  }
  
  
  // --- Book the MVA methods
  TString dir    = "weights/";
  for(int ichan=0; ichan<channel_list.size(); ichan++) //Book the method for each channel (separate BDTs)
  {
    cout << " CHANNEL " << channel_list[ichan] << endl;
    TString MVA_method_name = template_name + "_" + channel_list[ichan] + filename_suffix + TString(" method");
    TString weightfile = dir + template_name + "_" + channel_list[ichan] + filename_suffix + TString(".weights.xml"); //Contains weights related to BDT branches
    reader->BookMVA( MVA_method_name, weightfile );
  }
  
  
  Double_t         MVA_BDT;
  Double_t         MVA_EqLumi;
  Double_t         MVA_Luminosity;
  Int_t            MVA_channel;
  Float_t          MVA_weight;
  Double_t         MVA_weight_nom;
  Double_t         MVA_weight_puSF_up;
  Double_t         MVA_weight_puSF_down;
  Double_t         MVA_weight_electronSF_up;
  Double_t         MVA_weight_electronSF_down;
  Double_t         MVA_weight_muonSF_up;
  Double_t         MVA_weight_muonSF_down;
  Double_t         MVA_weight_btagSF_cferr1_up;
  Double_t         MVA_weight_btagSF_cferr1_down;
  Double_t         MVA_weight_btagSF_cferr2_up;
  Double_t         MVA_weight_btagSF_cferr2_down;
  Double_t         MVA_weight_btagSF_hf_up;
  Double_t         MVA_weight_btagSF_hf_down;
  Double_t         MVA_weight_btagSF_hfstats1_up;
  Double_t         MVA_weight_btagSF_hfstats1_down;
  Double_t         MVA_weight_btagSF_hfstats2_up;
  Double_t         MVA_weight_btagSF_hfstats2_down;
  Double_t         MVA_weight_btagSF_lf_up;
  Double_t         MVA_weight_btagSF_lf_down;
  Double_t         MVA_weight_btagSF_lfstats1_up;
  Double_t         MVA_weight_btagSF_lfstats1_down;
  Double_t         MVA_weight_btagSF_lfstats2_up;
  Double_t         MVA_weight_btagSF_lfstats2_down;
  Double_t MVA_weight_puSF;
  Double_t MVA_weight_btagSF ;
  Double_t MVA_weight_muonSF ;
  Double_t MVA_weight_electronSF ;
  
  
  TBranch        *b_MVA_weight_puSF;
  TBranch        *b_MVA_weight_btagSF ;
  TBranch        *b_MVA_weight_muonSF ;
  TBranch        *b_MVA_weight_electronSF ;
  TBranch        *b_MVA_channel;   //!
  TBranch        *b_MVA_weight;   //!
  TBranch        *b_MVA_weight_nom;
  TBranch        *b_MVA_weight_puSF_up;   //!
  TBranch        *b_MVA_weight_puSF_down;   //!
  TBranch        *b_MVA_weight_electronSF_up;   //!
  TBranch        *b_MVA_weight_electronSF_down;   //!
  TBranch        *b_MVA_weight_muonSF_up;   //!
  TBranch        *b_MVA_weight_muonSF_down;   //!
  TBranch        *b_MVA_weight_btagSF_cferr1_up;   //!
  TBranch        *b_MVA_weight_btagSF_cferr1_down;   //!
  TBranch        *b_MVA_weight_btagSF_cferr2_up;   //!
  TBranch        *b_MVA_weight_btagSF_cferr2_down;   //!
  TBranch        *b_MVA_weight_btagSF_hf_up;   //!
  TBranch        *b_MVA_weight_btagSF_hf_down;   //!
  TBranch        *b_MVA_weight_btagSF_hfstats1_up;   //!
  TBranch        *b_MVA_weight_btagSF_hfstats1_down;   //!
  TBranch        *b_MVA_weight_btagSF_hfstats2_up;   //!
  TBranch        *b_MVA_weight_btagSF_hfstats2_down;   //!
  TBranch        *b_MVA_weight_btagSF_lf_up;   //!
  TBranch        *b_MVA_weight_btagSF_lf_down;   //!
  TBranch        *b_MVA_weight_btagSF_lfstats1_up;   //!
  TBranch        *b_MVA_weight_btagSF_lfstats1_down;   //!
  TBranch        *b_MVA_weight_btagSF_lfstats2_up;   //!
  TBranch        *b_MVA_weight_btagSF_lfstats2_down;   //!
  
  Int_t         MVA_id1;
  Int_t         MVA_id2;
  Double_t         MVA_x1;
  Double_t         MVA_x2;
  Double_t         MVA_q;
  
  TBranch         *b_MVA_id1;
  TBranch         *b_MVA_id2;
  TBranch         *b_MVA_x1;
  TBranch         *b_MVA_x2;
  TBranch         *b_MVA_q;
  TBranch         *b_MVA_Luminosity;
  TBranch         *b_MVA_EqLumi;
  
  
  
  
  
  
  
  
  bool doJECup_, doJECdown_, doJERup_, doJERdown_;
  //doJECdown_ = doJECup_ = doJERdown_ = doJERup_ = false;
  //Loop on samples, syst., events ---> Fill histogram/channel --> Write()
  TString postfix = "";
  
  int systematicsForNewTree = 1; //5
  
  
  
  
  for(int isample=0; isample<sample_listread.size(); isample++)
  {
    
    
    
    std::cout << "--- Select "<<sample_listread[isample]<<" sample" << std::endl;
    
    
    // prepare output controltree
    TString output_file_name_tree = placeOutputReading+"/TreeOfReader_" + template_name + filename_suffix + ".root";
    TFile* file_output_tree(0);
    
    for(int isystree = 0; isystree < systematicsForNewTree; isystree++)
    {
      // cout << "isystree " << isystree << endl;
      if( (sample_listread[isample].Contains("fake") || sample_listread[isample].Contains("data")) && isystree != 0 ){ // no jec / jer for data/fakes
        // cout << "found data/fakes, continue" << endl;
        continue;
      }
      
      if(isystree == 0) {doJECdown_ = doJECup_ = doJERdown_ = doJERup_ = false;}
      else if(isystree == 4) {doJECdown_ = true; doJECup_ = doJERdown_ = doJERup_ = false;}
      else if(isystree == 3) {doJECdown_ = doJECup_ = doJERdown_ = false; doJERup_ = true;}
      else if(isystree == 2) {doJECdown_ = doJECup_ = doJERup_ = false; doJERdown_ = true;}
      else if(isystree == 1) {doJECdown_ = doJERup_ = doJERdown_ = false; doJECup_ = true;}
      if(doJECdown_) postfix = "_JESdown";
      else if(doJECup_) postfix = "_JESup";
      else if(doJERdown_) postfix = "_JERdown";
      else if(doJERup_) postfix = "_JERup";
      else postfix = "";
      
      TString inputfile = PlaceOfTuples+"MVA_tree_" + sample_listread[isample] + "_80X" + postfix + ".root";
      if( sample_listread[isample].Contains("data")) inputfile = PlaceOfTuples+"MVA_tree_" + sample_listread[isample] + postfix + ".root";
      TFile* file_input = TFile::Open(inputfile.Data());
      cout << "input file " << inputfile.Data() << endl;
      TString tree_name = "mvatree"+postfix;
      TTree* tree = (TTree*) file_input->Get(tree_name.Data());
      cout << "mva tree coming from " << tree_name.Data() << " " << tree <<  endl;
      
      
      //TFile* file_output_tree(0);
      if(isystree == 0 && isample == 0) { // recreate at first instance
        file_output_tree = new TFile( output_file_name_tree, "RECREATE" );
      }
      else file_output_tree = new TFile( output_file_name_tree, "UPDATE" );
      
      TTree* tree_control = new TTree("tree_control"+postfix, "Control Tree " + postfix);
      cout << "control tree " << "tree_control"+postfix << endl;
      for(int ivar=0; ivar<var_list.size(); ivar++)
      {
        
        TString var_type ="";
        var_type= var_list[ivar] + "/F";
        
        // cout <<  var_type << endl;
        tree_control->Branch(var_list[ivar].Data(), &(vec_variables[ivar]), var_type.Data());
      }
      
      for(int ivar=0; ivar<v_cut_name.size(); ivar++)
      {
        TString var_type = "";
        //if(!v_cut_name[ivar].Contains("nJet") && !v_cut_name[ivar].Contains("NJet") && !v_cut_name[ivar].Contains("Charge") ) var_type = v_cut_name[ivar] + "/F";
        //else var_type =  v_cut_name[ivar] + "/I";
        var_type = v_cut_name[ivar] + "/F";
        tree_control->Branch(v_cut_name[ivar].Data(), &v_cut_float[ivar], var_type.Data());
      }
      
      
      
      // cout << "start branches set " << endl;
      // Prepare the event tree
      for(int i=0; i<var_list.size(); i++)
      {
        tree->SetBranchAddress(var_list[i].Data(), &vec_variables[i]);
      }
      for(int i=0; i<v_cut_name.size(); i++)
      {
        
        tree->SetBranchAddress(v_cut_name[i].Data(), &v_cut_float[i]);
        
      }
      
      
      // cout << "all branches mva vars set " << endl;
      
      
      tree_control->Branch("MVA_weight_nom", &MVA_weight_nom, "MVA_weight_nom/D");
      tree_control->Branch("MVA_channel", &MVA_channel, "MVA_channel/I");
      tree_control->Branch("MVA_BDT",&MVA_BDT,"MVA_BDT/D");
      tree_control->Branch("MVA_EqLumi",&MVA_EqLumi, "MVA_EqLumi/D");
      tree_control->Branch("MVA_Luminosity",&MVA_Luminosity, "MVA_Luminosity/D");
      tree_control->Branch("MVA_weight_puSF_up", &MVA_weight_puSF_up, "MVA_weight_puSF_up/D");
      tree_control->Branch("MVA_weight_puSF_down", &MVA_weight_puSF_down, "MVA_weight_puSF_down/D");
      tree_control->Branch("MVA_weight_electronSF_up", &MVA_weight_electronSF_up, "MVA_weight_electronSF_up/D");
      tree_control->Branch("MVA_weight_electronSF_down", &MVA_weight_electronSF_down, "MVA_weight_electronSF_down/D");
      tree_control->Branch("MVA_weight_puSF",&MVA_weight_puSF, "MVA_weight_puSF/D");
      tree_control->Branch("MVA_weight_muonSF",&MVA_weight_muonSF, "MVA_weight_muonSF/D");
      tree_control->Branch("MVA_weight_eletcronSF",&MVA_weight_electronSF, "MVA_weight_electronSF/D");
      tree_control->Branch("MVA_weight_btagSF",&MVA_weight_btagSF, "MVA_weight_btagSF/D");
      tree_control->Branch("MVA_weight_muonSF_up", &MVA_weight_muonSF_up, "MVA_weight_muonSF_up/D");
      tree_control->Branch("MVA_weight_muonSF_down", &MVA_weight_muonSF_down, "MVA_weight_muonSF_down/D");
      tree_control->Branch("MVA_weight_btagSF_cferr1_up", &MVA_weight_btagSF_cferr1_up, "MVA_weight_btagSF_cferr1_up/D");
      tree_control->Branch("MVA_weight_btagSF_cferr1_down", &MVA_weight_btagSF_cferr1_down, "MVA_weight_btagSF_cferr1_down/D");
      tree_control->Branch("MVA_weight_btagSF_cferr2_up", &MVA_weight_btagSF_cferr2_up, "MVA_weight_btagSF_cferr2_up/D");
      tree_control->Branch("MVA_weight_btagSF_cferr2_down", &MVA_weight_btagSF_cferr2_down, "MVA_weight_btagSF_cferr2_down/D");
      tree_control->Branch("MVA_weight_btagSF_hf_up", &MVA_weight_btagSF_hf_up, "MVA_weight_btagSF_hf_up/D");
      tree_control->Branch("MVA_weight_btagSF_hf_down", &MVA_weight_btagSF_hf_down, "MVA_weight_btagSF_hf_down/D");
      tree_control->Branch("MVA_weight_btagSF_hfstats1_up", &MVA_weight_btagSF_hfstats1_up, "MVA_weight_btagSF_hfstats1_up/D");
      tree_control->Branch("MVA_weight_btagSF_hfstats1_down", &MVA_weight_btagSF_hfstats1_down, "MVA_weight_btagSF_hfstats1_down/D");
      tree_control->Branch("MVA_weight_btagSF_hfstats2_up", &MVA_weight_btagSF_hfstats2_up, "MVA_weight_btagSF_hfstats2_up/D");
      tree_control->Branch("MVA_weight_btagSF_hfstats2_down", &MVA_weight_btagSF_hfstats2_down, "MVA_weight_btagSF_hfstats2_down/D");
      tree_control->Branch("MVA_weight_btagSF_lf_up", &MVA_weight_btagSF_lf_up, "MVA_weight_btagSF_lf_up/D");
      tree_control->Branch("MVA_weight_btagSF_lf_down", &MVA_weight_btagSF_lf_down, "MVA_weight_btagSF_lf_down/D");
      tree_control->Branch("MVA_weight_btagSF_lfstats1_up", &MVA_weight_btagSF_lfstats1_up, "MVA_weight_btagSF_lfstats1_up/D");
      tree_control->Branch("MVA_weight_btagSF_lfstats1_down", &MVA_weight_btagSF_lfstats1_down, "MVA_weight_btagSF_lfstats1_down/D");
      tree_control->Branch("MVA_weight_btagSF_lfstats2_up", &MVA_weight_btagSF_lfstats2_up, "MVA_weight_btagSF_lfstats2_up/D");
      tree_control->Branch("MVA_weight_btagSF_lfstats2_down", &MVA_weight_btagSF_lfstats2_down, "MVA_weight_btagSF_lfstats2_down/D");
      tree_control->Branch("MVA_x1", &MVA_x1, "MVA_b_x1/D");
      tree_control->Branch("MVA_x2", &MVA_x2, "MVA_b_x2/D");
      tree_control->Branch("MVA_id1", &MVA_id1, "MVA_b_id1/I");
      tree_control->Branch("MVA_id2", &MVA_id2, "MVA_b_id2/I");
      tree_control->Branch("MVA_q", &MVA_q, "MVA_q/D");
      cout << "all control branches set " << endl;
      tree->SetBranchAddress("MVA_EqLumi",&MVA_EqLumi, &b_MVA_EqLumi);
      tree->SetBranchAddress("MVA_Luminosity", &MVA_Luminosity, &b_MVA_Luminosity);
     
      tree->SetBranchAddress("MVA_weight", &MVA_weight,&b_MVA_weight);
      tree->SetBranchAddress("MVA_weight_nom", &MVA_weight_nom, &b_MVA_weight_nom);
      tree->SetBranchAddress("MVA_weight_puSF_up", &MVA_weight_puSF_up, &b_MVA_weight_puSF_up);
      tree->SetBranchAddress("MVA_weight_puSF_down", &MVA_weight_puSF_down, &b_MVA_weight_puSF_down);
      tree->SetBranchAddress("MVA_weight_electronSF_up", &MVA_weight_electronSF_up, &b_MVA_weight_electronSF_up);
      tree->SetBranchAddress("MVA_weight_electronSF_down", &MVA_weight_electronSF_down, &b_MVA_weight_electronSF_down);
      tree->SetBranchAddress("MVA_weight_muonSF_up", &MVA_weight_muonSF_up, &b_MVA_weight_muonSF_up);
      tree->SetBranchAddress("MVA_weight_muonSF_down", &MVA_weight_muonSF_down, &b_MVA_weight_muonSF_down);
      tree->SetBranchAddress("MVA_weight_btagSF_cferr1_up", &MVA_weight_btagSF_cferr1_up, &b_MVA_weight_btagSF_cferr1_up);
      tree->SetBranchAddress("MVA_weight_btagSF_cferr1_down", &MVA_weight_btagSF_cferr1_down, &b_MVA_weight_btagSF_cferr1_down);
      tree->SetBranchAddress("MVA_weight_btagSF_cferr2_up", &MVA_weight_btagSF_cferr2_up, &b_MVA_weight_btagSF_cferr2_up);
      tree->SetBranchAddress("MVA_weight_btagSF_cferr2_down", &MVA_weight_btagSF_cferr2_down, &b_MVA_weight_btagSF_cferr2_down);
      tree->SetBranchAddress("MVA_weight_btagSF_hf_up", &MVA_weight_btagSF_hf_up, &b_MVA_weight_btagSF_hf_up);
      tree->SetBranchAddress("MVA_weight_btagSF_hf_down", &MVA_weight_btagSF_hf_down, &b_MVA_weight_btagSF_hf_down);
      tree->SetBranchAddress("MVA_weight_btagSF_hfstats1_up", &MVA_weight_btagSF_hfstats1_up, &b_MVA_weight_btagSF_hfstats1_up);
      tree->SetBranchAddress("MVA_weight_btagSF_hfstats1_down", &MVA_weight_btagSF_hfstats1_down, &b_MVA_weight_btagSF_hfstats1_down);
      tree->SetBranchAddress("MVA_weight_btagSF_hfstats2_up", &MVA_weight_btagSF_hfstats2_up, &b_MVA_weight_btagSF_hfstats2_up);
      tree->SetBranchAddress("MVA_weight_btagSF_hfstats2_down", &MVA_weight_btagSF_hfstats2_down, &b_MVA_weight_btagSF_hfstats2_down);
      tree->SetBranchAddress("MVA_weight_btagSF_lf_up", &MVA_weight_btagSF_lf_up, &b_MVA_weight_btagSF_lf_up);
      tree->SetBranchAddress("MVA_weight_btagSF_lf_down", &MVA_weight_btagSF_lf_down, &b_MVA_weight_btagSF_lf_down);
      tree->SetBranchAddress("MVA_weight_btagSF_lfstats1_up", &MVA_weight_btagSF_lfstats1_up, &b_MVA_weight_btagSF_lfstats1_up);
      tree->SetBranchAddress("MVA_weight_btagSF_lfstats1_down", &MVA_weight_btagSF_lfstats1_down, &b_MVA_weight_btagSF_lfstats1_down);
      tree->SetBranchAddress("MVA_weight_btagSF_lfstats2_up", &MVA_weight_btagSF_lfstats2_up, &b_MVA_weight_btagSF_lfstats2_up);
      tree->SetBranchAddress("MVA_weight_btagSF_lfstats2_down", &MVA_weight_btagSF_lfstats2_down, &b_MVA_weight_btagSF_lfstats2_down);
      tree->SetBranchAddress("MVA_x1", &MVA_x1, &b_MVA_x1);
      tree->SetBranchAddress("MVA_x2", &MVA_x2, &b_MVA_x2);
      tree->SetBranchAddress("MVA_id1", &MVA_id1, &b_MVA_id1);
      tree->SetBranchAddress("MVA_id2", &MVA_id2, &b_MVA_id2);
      tree->SetBranchAddress("MVA_q", &MVA_q, &b_MVA_q);
      
      tree->SetBranchAddress("MVA_weight_puSF",&MVA_weight_puSF, &b_MVA_weight_puSF);
      tree->SetBranchAddress("MVA_weight_muonSF",&MVA_weight_muonSF, &b_MVA_weight_muonSF);
      tree->SetBranchAddress("MVA_weight_eletcronSF",&MVA_weight_electronSF, &b_MVA_weight_electronSF);
      tree->SetBranchAddress("MVA_weight_btagSF",&MVA_weight_btagSF, &b_MVA_weight_btagSF);
      
      
      
      // cout << "all branches set " << endl;
      //std::cout << "--- Processing: " << tree->GetEntries() << " events" << std::endl;
      tree->SetBranchAddress("MVA_channel",&MVA_channel,&b_MVA_channel);
     // tree->SetBranchStatus("*",1);
     // tree_control = tree->CloneTree();
     // TBranch *bdtbr = tree_control->Branch("MVA_BDT",&MVA_BDT,"MVA_BDT/D");
      
      //------------------------------------------------------------
      // --- Event loop
      int cutArray[v_cut_name.size()];
      double wcutArray[v_cut_name.size()];
      int endEntries = 0;
      for(int ivar=0; ivar<v_cut_name.size(); ivar++)
      {
        cutArray[ivar] = 0;
        wcutArray[ivar] = 0.;
      }
      int totalEntries = 0;
      double wendEntries = 0.;
      double wtotalEntries = 0.;
      
      for(int ievt=0; ievt<tree->GetEntries(); ievt++)
      {
        MVA_weight = 1.; MVA_channel = 9.; MVA_BDT = 1.;
        
        tree->GetEntry(ievt);
        
        //------------------------------------------------------------
        //------------------------------------------------------------
        //---- Apply cuts on Reader here -----------------------------
        float cut_tmp = 0; bool pass_all_cuts = true;
        
        
        for(int ivar=0; ivar<v_cut_name.size(); ivar++)
        {
          
          if(v_cut_def[ivar] != "")
          {
            if(!v_cut_def[ivar].Contains("&&")) //If cut contains only 1 condition
            {
              cut_tmp = Find_Number_In_TString(v_cut_def[ivar]);
              
              if(v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] < cut_tmp)		 {pass_all_cuts = false; break; }
              else if(v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] > cut_tmp)	 {pass_all_cuts = false; break; }
              else if(v_cut_def[ivar].Contains(">") && v_cut_float[ivar] <= cut_tmp)	 {pass_all_cuts = false; break; }
              else if(v_cut_def[ivar].Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break; }
              else if(v_cut_def[ivar].Contains("==") && v_cut_float[ivar] != cut_tmp)  {pass_all_cuts = false; break; }
              else {
                cutArray[ivar]++;
                wcutArray[ivar] = wcutArray[ivar] + MVA_weight;
              }
              
            }
            else //If 2 conditions in the cut, break it in 2
            {
              TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first; TString cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
              //CUT 1
              bool passed = false;
              cut_tmp = Find_Number_In_TString(cut1);
              if(cut1.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; break; }
              else if(cut1.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; break; }
              else if(cut1.Contains(">") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; break; }
              else if(cut1.Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break; }
              else if(cut1.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; break; }
              else(passed = true);
              //CUT 2
              cut_tmp = Find_Number_In_TString(cut2);
              if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; break; }
              else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; break; }
              else if(cut2.Contains(">") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; break; }
              else if(cut2.Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break; }
              else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; break; }
              else if(passed) {
                cutArray[ivar]++;
                wcutArray[ivar] = wcutArray[ivar]+ MVA_weight;
              }
            }
          }
          else {
            cutArray[ivar]++;
          }
        }
        
        totalEntries++;
        wtotalEntries = wtotalEntries + MVA_weight;
        if(!pass_all_cuts) {continue;}
        endEntries++;
        wendEntries = wendEntries + MVA_weight;
        //------------------------------------------------------------
        //------------------------------------------------------------
        
        
        // fill trees
        //cout << template_name+"_uuu"+filename_suffix+ " method"<< endl;
        if(MVA_channel == 0 ) {MVA_BDT =  reader->EvaluateMVA( template_name+"_uuu"+filename_suffix+ " method");}
        else if(MVA_channel == 1 ){MVA_BDT = reader->EvaluateMVA( template_name+"_uue"+filename_suffix+ " method");}
        else if(MVA_channel == 2 ){MVA_BDT= reader->EvaluateMVA( template_name+"_eeu"+filename_suffix+ " method");}
        else if(MVA_channel == 3 ){MVA_BDT= reader->EvaluateMVA( template_name+"_eee"+filename_suffix+ " method") ;}
        
        cout << "BDT: " <<  MVA_BDT << endl;
        
        tree_control->Fill();
        
      } //end entries loop
      /*cout << "RAW events" << endl;
       for(int iC=0; iC<v_cut_name.size(); iC++)
       {
       cout << "         -- iC=" << iC << " cutname: " << v_cut_name[iC] << " events passed: " << cutArray[iC]<< " = " << ((double) cutArray[iC]/(double)totalEntries) *100<< " % of total " << totalEntries << endl;
       }
       cout << "         -- leaving " << endEntries << " = " << ((double) endEntries/ (double) totalEntries)*100 << " % of total " << totalEntries << endl;
       cout << "WEIGHTED events" << endl;
       for(int iC=0; iC<v_cut_name.size(); iC++)
       {
       cout << "         -- iC=" << iC << " cutname: " << v_cut_name[iC] << " events passed: " << wcutArray[iC]* luminosity_rescale<< " = " << ((double) wcutArray[iC]/(double)wtotalEntries) *100<< " % of total " << wtotalEntries * luminosity_rescale<< endl;
       }
       cout << "         -- leaving " << wendEntries* luminosity_rescale << " = " << ((double) wendEntries/ (double) wtotalEntries)*100 << " % of total " << wtotalEntries* luminosity_rescale << endl;
       */
      // Write tree
      file_output_tree->cd();
      TString output_tree_name = "Control_" + sample_listread[isample]+"_80X"+postfix;
      if( sample_listread[isample].Contains("data")) output_tree_name = "Control_" + sample_listread[isample]+postfix;
      tree_control->Write(output_tree_name.Data(), TObject::kOverwrite);
      file_output_tree->Close();
      delete tree;
    } // sys loop
    // --- Write histograms
    
    
    cout<<"Done with "<<sample_listread[isample]<<" sample"<<endl;
  } //end sample loop
  
  
  std::cout << "==> Reader() is done!" << std::endl << std::endl;
  
  
}




