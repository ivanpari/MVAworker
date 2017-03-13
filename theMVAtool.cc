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
theMVAtool::theMVAtool(std::vector<TString > thevarlist, std::vector<TString > thesamplelist, std::vector<TString > thesamplelist_forreading, std::vector<TString > thechanlist,
                       std::vector<TString > set_v_cut_name, std::vector<TString > set_v_cut_def, std::vector<bool > set_v_cut_IsUsedForBDT, int nofbin_templates = 5 , string PlaceOfTuples_ = "", TString region_name_ = "")

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
  TMVA::Factory* factory = new TMVA::Factory(bdt_type.Data(), output_file, "!V:!Silent:Color:!DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );
  
  // Define the input variables that shall be used for the MVA training
  for(int i=0; i<var_list.size(); i++)
  {
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
    cout << "input file: " << inputfile << endl;
    
    TFile* file_input = TFile::Open( inputfile.Data() );
    
    TTree* tree = (TTree*)file_input->Get("mvatree");
    
    // global event weights per tree (see below for setting event-wise weights)
    Double_t signalWeight     = 1.0;
    Double_t backgroundWeight = 1.0;
    cout << " sample " << sample_list[isample] << endl;
    
    // You can add an arbitrary number of signal or background trees
    //NB : can't account for luminosity rescaling here, but it is not necessary for the training (only relative weights matter ?)
    if(sample_list[isample].Contains("FCNC") ){factory->AddSignalTree ( tree, signalWeight ); factory->SetSignalWeightExpression( "MVA_weight" );}
    else {factory->AddBackgroundTree( tree, backgroundWeight ); factory->SetBackgroundWeightExpression( "MVA_weight" );}
    
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
  factory->BookMethod( TMVA::Types::kBDT,method_title.Data(),"!H:!V:Ntrees=25:MinNodeSize=5%:MaxDepth=3:BoostType=Grad:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:NegWeightTreatment=Pray");
  //factory->BookMethod( TMVA::Types::kBDT,method_title.Data(),"!H:!V:Ntrees=25:MinNodeSize=2.5%:MaxDepth=3:BoostType=Adaboost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:NegWeightTreatment=Pray"); // ST method
  // factory->BookMethod( TMVA::Types::kBDT,method_title.Data(),"!H:!V:NTrees=25:BoostType=Grad:Shrinkage=0.20:UseBaggedBoost:BaggedSampleFraction=0.6:SeparationType=GiniIndex:nCuts=20:MaxDepth=3:NegWeightTreatment=Pray" );
  
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
  
  TString output_file_name = placeOutputReading+"/Reader_" + template_name + filename_suffix + ".root";
  TFile* file_output = TFile::Open( output_file_name, "RECREATE" );
  
  
  
  TString output_file_name_eee = placeOutputReading+"/Reader_eee_" + template_name + filename_suffix + ".root";
  TFile* file_output_eee = TFile::Open( output_file_name_eee, "RECREATE" );
  TString output_file_name_eeu = placeOutputReading+"/Reader_eeu_" + template_name + filename_suffix + ".root";
  TFile* file_output_eeu = TFile::Open( output_file_name_eeu, "RECREATE" );
  TString output_file_name_uue = placeOutputReading+"/Reader_uue_" + template_name + filename_suffix + ".root";
  TFile* file_output_uue = TFile::Open( output_file_name_uue, "RECREATE" );
  TString output_file_name_uuu = placeOutputReading+"/Reader_uuu_" + template_name + filename_suffix + ".root";
  TFile* file_output_uuu = TFile::Open( output_file_name_uuu, "RECREATE" );
  
  
  
  
  
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
  
  
  TH1::SetDefaultSumw2();
  TH1F *hist_BDT(0), *hist_BDTG(0);
  TH1F *hist_uuu = 0, *hist_uue = 0, *hist_eeu = 0, *hist_eee = 0;
  
  //Loop on samples, syst., events ---> Fill histogram/channel --> Write()
  for(int isample=0; isample<sample_listread.size(); isample++)
  {
    
    TString inputfile = PlaceOfTuples+"MVA_tree_" + sample_listread[isample] + "_80X.root";
    TFile* file_input = TFile::Open( inputfile.Data() );
    
    std::cout << "--- Select "<<sample_listread[isample]<<" sample" << std::endl;
    // prepare outpout histograms
    hist_uuu     = new TH1F( (template_name+"_uuu").Data(),           (template_name+"_uuu").Data(),           nbin, -1, 1 );
    hist_uue     = new TH1F( (template_name+"_uue").Data(),           (template_name+"_uue").Data(),           nbin, -1, 1 );
    hist_eeu     = new TH1F( (template_name+"_eeu").Data(),           (template_name+"_eeu").Data(),           nbin, -1, 1 );
    hist_eee     = new TH1F( (template_name+"_eee").Data(),           (template_name+"_eee").Data(),           nbin, -1, 1 );
    
    
    // prepare output controltree
    TString output_file_name_tree = placeOutputReading+"/TreeOfReader_" + template_name + filename_suffix + ".root";
    TFile* file_output_tree = TFile::Open( output_file_name_tree, "UPDATE" );
    
    TTree* tree_control(0);
    tree_control = new TTree("tree_control", "Control Tree");
    for(int ivar=0; ivar<var_list.size(); ivar++)
    {
      TString var_type = var_list[ivar] + "/F";
      tree_control->Branch(var_list[ivar].Data(), &(vec_variables[ivar]), var_type.Data());
    }
    for(int ivar=0; ivar<v_cut_name.size(); ivar++)
    {
      TString var_type = v_cut_name[ivar] + "/F";
      tree_control->Branch(v_cut_name[ivar].Data(), &v_cut_float[ivar], var_type.Data());
    }

    TTree* tree(0);
    TString tree_name = "";
    tree = (TTree*) file_input->Get("mvatree");
    
    // Prepare the event tree
    for(int i=0; i<var_list.size(); i++)
    {
      tree->SetBranchAddress(var_list[i].Data(), &vec_variables[i]);
    }
    for(int i=0; i<v_cut_name.size(); i++)
    {
      tree->SetBranchAddress(v_cut_name[i].Data(), &v_cut_float[i]);
    }
    
    float i_channel;
    float weight;
    
    float BDT; Double_t MVA_EqLumi;
    tree_control->Branch("MVA_weight", &weight, "weight/F"); //Give it the same name regardless of the systematic, since we create a separate tree for each syst anyway
    tree_control->Branch("MVA_channel", &i_channel, "i_channel/F");
    tree_control->Branch("BDT",&BDT,"BDT/F");
    tree_control->Branch("MVA_EqLumi",&MVA_EqLumi, "MVA_EqLumi/D");
    
    tree->SetBranchAddress("MVA_channel", &i_channel);
    tree->SetBranchAddress("MVA_weight", &weight);
    // FIX ME FOR SYST else {tree->SetBranchAddress(syst_list[isyst].Data(), &weight);}
    
    
    
    
    std::cout << "--- Processing: " << tree->GetEntries() << " events" << std::endl;
    
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
      weight = 1; i_channel = 9;
      
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
              wcutArray[ivar] = wcutArray[ivar] + weight;
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
              wcutArray[ivar] = wcutArray[ivar]+ weight;
            }
          }
        }
        else {
          cutArray[ivar]++;
        }
      }
      
      totalEntries++;
      wtotalEntries = wtotalEntries + weight;
      if(!pass_all_cuts) {continue;}
      endEntries++;
      wendEntries = wendEntries + weight;
      //------------------------------------------------------------
      //------------------------------------------------------------
      
      // fill the histograms
      //  cout << "i_channel " << i_channel << endl;
      if(i_channel == 0) 		{hist_uuu->Fill( reader->EvaluateMVA( template_name+"_uuu"+filename_suffix+ " method"), weight);}
      else if(i_channel == 1) {hist_uue->Fill( reader->EvaluateMVA( template_name+"_uue"+filename_suffix+" method"), weight);}
      else if(i_channel == 2) {hist_eeu->Fill( reader->EvaluateMVA( template_name+"_eeu"+filename_suffix+" method"), weight);}
      else if(i_channel == 3) {hist_eee->Fill( reader->EvaluateMVA( template_name+"_eee"+filename_suffix+" method"), weight);}
      else if(i_channel == 9 || weight == 0) {cout<<__LINE__<<BOLD(FRED(" : problem  weight")) << weight <<endl;}
      
      // fill trees
      if(i_channel == 0 ) {BDT =  reader->EvaluateMVA( template_name+"_uuu"+filename_suffix+ " method");}
      else if(i_channel == 1 ){BDT = reader->EvaluateMVA( template_name+"_uuu"+filename_suffix+ " method");}
      else if(i_channel == 2 ){BDT= reader->EvaluateMVA( template_name+"_uuu"+filename_suffix+ " method");}
      else if(i_channel == 3 ){BDT= reader->EvaluateMVA( template_name+"_uuu"+filename_suffix+ " method") ;}
      
      tree_control->Fill();
      
    } //end entries loop
    cout << "RAW events" << endl;
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
    
    // Write tree
    file_output_tree->cd();
    TString output_tree_name = "Control_" + sample_listread[isample]+"_80X";
    tree_control->Write(output_tree_name.Data(), TObject::kOverwrite);
    delete tree_control;
    file_output_tree->Close();
    
    // --- Write histograms
    file_output->cd();
    
    //NB : theta name convention = <observable>__<process>[__<uncertainty>__(plus,minus)] FIX ME
    TString output_histo_name = "";
    TString sample_name = sample_listread[isample];
    if (sample_listread[isample].Contains("Fakes") ) //Last fake MC sample or data-driven fakes -> write fake histo w/ special name (for THETA)
    {
      output_histo_name = template_name+"_uuu_FakeMu" ;
      hist_uuu->SetTitle(output_histo_name.Data());
      hist_uuu->Write(output_histo_name.Data());
      output_histo_name = template_name+"_uue_FakeEl" ;
      hist_uue->SetTitle(output_histo_name.Data());
      hist_uue->Write(output_histo_name.Data());
      output_histo_name = template_name+"_eeu_FakeMu" ;
      hist_eeu->SetTitle(output_histo_name.Data());
      hist_eeu->Write(output_histo_name.Data());
      output_histo_name = template_name+"_eee_FakeEl";
      hist_eee->SetTitle(output_histo_name.Data());
      hist_eee->Write(output_histo_name.Data());
    }
    else //If fakes are not considered, or if sample is not fake --> write directly !
    {
      output_histo_name = template_name+"_uuu_" + sample_name ;
      hist_uuu->SetTitle(output_histo_name.Data());
      hist_uuu->Write(output_histo_name.Data());
      output_histo_name = template_name+"_uue_" + sample_name ;
      hist_uue->SetTitle(output_histo_name.Data());
      hist_uue->Write(output_histo_name.Data());
      output_histo_name = template_name+"_eeu_" + sample_name ;
      hist_eeu->SetTitle(output_histo_name.Data());
      hist_eeu->Write(output_histo_name.Data());
      output_histo_name = template_name+"_eee_" + sample_name ;
      hist_eee->SetTitle(output_histo_name.Data());
      hist_eee->Write(output_histo_name.Data());
    }
    
    
    file_output_eee->cd();
    if( sample_listread[isample].Contains("Fakes")) //Last fake MC sample or data-driven fakes -> write fake histo w/ special name (for THETA)
    {
      output_histo_name = template_name+"_eee_FakeEl" ;
      hist_eee->Write(output_histo_name.Data());
    }
    else //If fakes are not considered, or if sample is not fake --> write directly !
    {
      output_histo_name = template_name+"_eee_" + sample_name ;
      hist_eee->Write(output_histo_name.Data());
    }
    file_output_eeu->cd();
    if( sample_listread[isample].Contains("Fakes") ) //Last fake MC sample or data-driven fakes -> write fake histo w/ special name (for THETA)
    {
      output_histo_name = template_name+"_eeu_FakeMu" ;
      hist_eeu->Write(output_histo_name.Data());
      
    }
    else //If fakes are not considered, or if sample is not fake --> write directly !
    {
      output_histo_name = template_name+"_eeu_" + sample_name ;
      hist_eeu->Write(output_histo_name.Data());
      
    }
    file_output_uue->cd();
    if(sample_listread[isample].Contains("Fakes")) //If sample is MC fake, don't reinitialize histos -> sum 3 MC fake samples
    {
      output_histo_name = template_name+"_uue_FakeEl" ;
      hist_uue->Write(output_histo_name.Data());
      
    }
    else //If fakes are not considered, or if sample is not fake --> write directly !
    {
      
      output_histo_name = template_name+"_uue_" + sample_name ;
      hist_uue->Write(output_histo_name.Data());
      
    }
    file_output_uuu->cd();
    if( sample_listread[isample].Contains("Fakes")) //Last fake MC sample or data-driven fakes -> write fake histo w/ special name (for THETA)
    {
      output_histo_name = template_name+"_uuu_FakeMu";
      hist_uuu->Write(output_histo_name.Data());
      
    }
    else //If fakes are not considered, or if sample is not fake --> write directly !
    {
      output_histo_name = template_name+"_uuu_" + sample_name ;
      hist_uuu->Write(output_histo_name.Data());
      
    }
    
    
    
    
    
    
    //don't delete if processing MC fake templates (unless all the loops have reached their ends)
    if(isample == (sample_listread.size() - 1))
    {
      //cout<<"deleting dynamic histograms"<<endl;
      delete hist_uuu; delete hist_uue; delete hist_eeu; delete hist_eee;
    }
    
    cout<<"Done with "<<sample_listread[isample]<<" sample"<<endl;
  } //end sample loop
  
  
  file_output->Close();
  file_output_eee->Close();
  file_output_eeu->Close();
  file_output_uue->Close();
  file_output_uuu->Close();
  std::cout << "--- Created root file: \""<<file_output->GetName()<<"\" containing the output histograms" << std::endl;
  std::cout << "==> Reader() is done!" << std::endl << std::endl;
  
  
}



void theMVAtool::PSDataCreator(TString histoTempl, TString channel_, TString template_name ){
  cout<<FYEL("--- Producing "<<template_name<<" PseudoData Templates ---")<<endl;
  
  TRandom3 therand(0); //Randomization
  
  TString pseudodata_input_name = "";
  if(channel_.Contains("All")) pseudodata_input_name += placeOutputReading+"/Reader_" + template_name + filename_suffix + ".root";
  if(channel_.Contains("eee")) pseudodata_input_name += placeOutputReading+"/Reader_eee_" + template_name + filename_suffix + ".root";
  if(channel_.Contains("eeu")) pseudodata_input_name += placeOutputReading+"/Reader_eeu_" + template_name + filename_suffix + ".root";
  if(channel_.Contains("uue")) pseudodata_input_name += placeOutputReading+"/Reader_uue_" + template_name + filename_suffix + ".root";
  if(channel_.Contains("uuu")) pseudodata_input_name += placeOutputReading+"/Reader_uuu_" + template_name + filename_suffix + ".root";
  cout << pseudodata_input_name << endl;
  
  
  TFile* file = 0;
  file = TFile::Open( pseudodata_input_name.Data(), "UPDATE");
  if(file == 0) {cout<<BOLD(FRED("--- ERROR : Reader file not found ! Exit !"))<<endl; }
  
  
  
  TH1F *h_sum = 0, *h_tmp = 0;
  TString histo_name = "";
  TString template_fake_name = "";
  
  
  
  for(int ichan=0; ichan<channel_list.size(); ichan++)
  {
    h_sum = 0;
    histo_name = "";
    
    if(channel_.Contains("eee") && !channel_list[ichan].Contains("eee")) continue;
    if(channel_.Contains("eeu") && !channel_list[ichan].Contains("eeu")) continue;
    if(channel_.Contains("uue") && !channel_list[ichan].Contains("uue")) continue;
    if(channel_.Contains("uuu") && !channel_list[ichan].Contains("uuu")) continue;
    
    if(channel_list[ichan] == "uuu" || channel_list[ichan] == "eeu") {template_fake_name = "FakeMu";}
    else {template_fake_name = "FakeEl";}
    
    
    for(int isample = 0; isample < sample_listread.size(); isample++)
    {
      if(sample_listread[isample].Contains("FCNC")) {continue; } // no signal in data
      if(!sample_listread[isample].Contains("Fake")) {
        cout << "  -- sample " << sample_listread[isample] << endl;
        h_tmp = 0;
        histo_name = template_name + "_" + channel_list[ichan] + "_" + sample_list[isample];
        cout << "  --- histo " << histo_name << endl;
        if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<endl<<BOLD(FRED("--- Empty histogram (Reader empty ?) ! Exit !"))<<endl<<endl; break;}
        h_tmp = (TH1F*) file->Get(histo_name.Data());
        if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
        else {h_sum->Add(h_tmp);}
      }
      else{
        histo_name = template_name + "_" + channel_list[ichan] + "_" + template_fake_name;
        cout << "  --- histo " << histo_name << endl;
        if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
        else
        {
          h_tmp = (TH1F*) file->Get(histo_name.Data());
          if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
          else {h_sum->Add(h_tmp);}
        }
        
      }
    }
    
    if(h_sum == 0) {cout<<endl<<BOLD(FRED("--- Empty histogram (Reader empty ?) ! Exit !"))<<endl<<endl; }
    int nofbins = h_sum->GetNbinsX();
    
    for(int i=0; i<nofbins; i++)
    {
      double bin_content = h_sum->GetBinContent(i+1); //cout<<"bin "<<i+1<<endl; cout<<"initial content = "<<bin_content<<endl;
      double new_bin_content = therand.Poisson(bin_content);// cout<<"new content = "<<new_bin_content<<endl;
      h_sum->SetBinContent(i+1, new_bin_content);
      h_sum->SetBinError(i+1, sqrt(new_bin_content)); //Poissonian error
    }
    
    file->cd();
    TString output_histo_name = template_name + "_" + channel_list[ichan] + "_data_obs"; // TO FIX
    h_sum->Write(output_histo_name, TObject::kOverwrite);
    
  }
  
  file->Close();
  
  cout<<"--- Done with generation of pseudo-data"<<endl<<endl;
}




