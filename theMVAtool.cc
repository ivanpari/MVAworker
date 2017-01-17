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
theMVAtool::theMVAtool(std::vector<TString > thevarlist, std::vector<TString > thesamplelist, std::vector<TString > thesystlist, std::vector<TString > thechanlist, vector<int> v_color, std::vector<TString > set_v_cut_name, std::vector<TString > set_v_cut_def, std::vector<bool > set_v_cut_IsUsedForBDT, int nofbin_templates = 5)
{
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
  }
  for(int i=0; i<thesystlist.size(); i++)
  {
    syst_list.push_back(thesystlist[i]);
  }
  for(int i=0; i<v_color.size(); i++)
  {
    colorVector.push_back(v_color[i]);
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



/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Set the luminosity re-scaling factor to be used thoughout the code
void theMVAtool::Set_Luminosity(double desired_luminosity = 20)
{
  double current_luminosity = 2.26; //2015 data / 7.6.x Ntuples --- TO BE CHANGED IN 8.0 / 2016 data!
  this->luminosity_rescale = desired_luminosity / current_luminosity;
  
  
  
  cout<<endl<<BOLD(FBLU("##################################"))<<endl;
  cout<<"--- Using luminosity scale factor : "<<desired_luminosity<<" / "<<current_luminosity<<" = "<<luminosity_rescale<<" ! ---"<<endl;
  cout<<BOLD(FBLU("##################################"))<<endl<<endl;
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
  // This loads the TMVA libraries
  TMVA::Tools::Instance();
  
  mkdir("outputs",0777);
  
  TString output_file_name = "outputs/" + bdt_type;
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
    if(sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("WW") || sample_list[isample].Contains("Data") || sample_list[isample].Contains("Fakes")) {cout<<"Train only on MC without fakes -- ignored sample " << sample_list[isample] <<endl; continue;} //Train only on MC
    
    // Read training and test data
    // --- Register the training and test trees
    TString inputfile;
    //inputfile = "Ntuples/FCNCNTuple_" + sample_list[isample] + ".root";
    
    //This sample has more stat. but doesn't contain the Q2 systematic -> TRAIN on this sample, but all the other functions will use the smaller "ttZ" (amcatnlo) sample !
   /* if(sample_list[isample] == "ttZ") {
      inputfile = "Ntuples/FCNCNTuple_ttZMad.root"; cout<<endl<<BOLD(FBLU("[NB : use ttZ Madgraph sample for training only !]"))<<endl<<endl;
    }
    else {
      inputfile = "Ntuples/FCNCNTuple_" + sample_list[isample] + ".root";
    }*/
    inputfile = "Ntuples/" + sample_list[isample] + "_80X.root";
    
    TFile* file_input = TFile::Open( inputfile.Data() );
    
    TTree* tree = (TTree*)file_input->Get("tree");
    
    // global event weights per tree (see below for setting event-wise weights)
    Double_t signalWeight     = 1.0;
    Double_t backgroundWeight = 1.0;
    
    // You can add an arbitrary number of signal or background trees
    //NB : can't account for luminosity rescaling here, but it is not necessary for the training (only relative weights matter ?)
    if(sample_list[isample] == "NP_overlay_ST_FCNC_zut") {factory->AddSignalTree ( tree, signalWeight ); }//factory->SetSignalWeightExpression( "fabs(Weight)" );}
    else {factory->AddBackgroundTree( tree, backgroundWeight );} // factory->SetBackgroundWeightExpression( "fabs(Weight)" );}

  }
  
  cout<<"////////WARNING : TAKE *ABSOLUTE* VALUES OF WEIGHTS////////"<<endl;
  
  // Apply additional cuts on the signal and background samples (can be different)
  TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
  TCut mycutb = ""; // for example: TCut mycutb = "abs(var1)<0.5";
  
  if(channel != "all")
  {
    if(channel == "uuu")         		{mycuts = "channelInt==0"; mycutb = "channelInt==0";}
    else if(channel == "uue" )     		{mycuts = "channelInt==1"; mycutb = "channelInt==1";}
    else if(channel == "eeu"  )     	{mycuts = "channelInt==2"; mycutb = "channelInt==2";}
    else if(channel == "eee"   )     	{mycuts = "channelInt==3"; mycutb = "channelInt==3";}
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
  factory->BookMethod( TMVA::Types::kBDT,method_title.Data(),"!H:!V:Ntrees=250:MinNodeSize=5%:MaxDepth=3:BoostType=Grad:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:NegWeightTreatment=Pray");

  cout << "book method " << method_title << endl;
  
  
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
//                                               _   _
//    _ __    ___           ___    ___    __ _  | | (_)  _ __     __ _
//   | '__|  / _ \  _____  / __|  / __|  / _` | | | | | | '_ \   / _` |
//   | |    |  __/ |_____| \__ \ | (__  | (_| | | | | | | | | | | (_| |
//   |_|     \___|         |___/  \___|  \__,_| |_| |_| |_| |_|  \__, |
//                                                               |___/
//-----------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Computes ratio of fakes in MC compared to data, to re-scale mWt template of fakes from data in Read()
float theMVAtool::Compute_Fake_Ratio(TString channel, bool fakes_summed_channels)
{
  vector<TString> MC_fake_samples_list;
  int sample_list_size = sample_list.size();
  //The MC fake samples are supposed to be the last 3 of the sample_list !
  for(int isample = sample_list.size() - 3 ; isample < sample_list.size(); isample++)
  {
    if(!sample_list[isample].Contains("Zjets") && !sample_list[isample].Contains("TTJets") && !sample_list[isample].Contains("WW")) {cout<<__LINE__<<BOLD(FRED(" Warning : wrong MC fake sample !"))<<endl;}
    else {MC_fake_samples_list.push_back(sample_list[isample].Data());}
  }
  if(MC_fake_samples_list.size() != 3) {cout<<__LINE__<<BOLD(FRED(" Warning : There are "<<MC_fake_samples_list.size()<<" MC fake samples ! Normal ?"))<<endl;}
  
  //FAKES FROM MC
  double integral_MC_fake = 0;
  float weight = 1; float i_channel = 9;
  
  for(int i=0; i<MC_fake_samples_list.size(); i++)
  {
    TString inputfile = "Ntuples/" + MC_fake_samples_list[i] + "_80X.root";
    TFile* file_fake = TFile::Open( inputfile.Data() );
    TTree* tree = (TTree*) file_fake->Get("Default");
    //tree->SetBranchAddress("Weight", &weight);
    tree->SetBranchAddress("channelInt", &i_channel);
    for(int ivar=0; ivar<v_cut_name.size(); ivar++)
    {
      //Use only these variables for cuts
      if(v_cut_name[ivar] == "nJets" || v_cut_name[ivar] == "nJets_CSVL" || v_cut_name[ivar] == "AdditionalMuonIso" || v_cut_name[ivar] == "AdditionalEleIso" || v_cut_name[ivar] == "AddLepPT")
      {
        tree->SetBranchAddress(v_cut_name[ivar].Data(), &v_cut_float[ivar]);
      }
    }
    
    for(int ientry=0; ientry<tree->GetEntries(); ientry++)
    {
      weight = 0; i_channel = 9;
      tree->GetEntry(ientry);
      
      //Apply cuts (only those on jets, bjets, leptons)
      //------------------------------------------------------------
      if(channel == "uuu" && i_channel != 0) 		{continue;}
      else if(channel == "uue" && i_channel != 1) {continue;}
      else if(channel == "eeu" && i_channel != 2) {continue;}
      else if(channel == "eee" && i_channel != 3) {continue;}
      
      
      float cut_tmp = 0; bool pass_all_cuts = true;
      for(int ivar=0; ivar<v_cut_name.size(); ivar++)
      {
        //Always compute the ratio in the WZ CR !
        //NB : Make the hypothesis that the ratio [MC fakes]/[Data fakes enriched region] is the same in WZ CR and SR
        if( (v_cut_name[i] == "nJets" && v_cut_float[ivar] == 0) || (v_cut_name[i] == "nJets_CSVL" && v_cut_float[ivar] != 0) ) {pass_all_cuts = false; break;}
        //Also apply cuts on leptons
        else if(v_cut_def[ivar] != "" && (v_cut_name[ivar] == "AdditionalMuonIso" || v_cut_name[ivar] == "AdditionalEleIso" || v_cut_name[ivar] == "AddLepPT") )
        {
          cut_tmp = Find_Number_In_TString(v_cut_def[ivar]);
          if(v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] < cut_tmp)		 {pass_all_cuts = false; break;}
          else if(v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] > cut_tmp)	 {pass_all_cuts = false; break;}
          else if(v_cut_def[ivar].Contains(">") && v_cut_float[ivar] <= cut_tmp)	 {pass_all_cuts = false; break;}
          else if(v_cut_def[ivar].Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
          else if(v_cut_def[ivar].Contains("==") && v_cut_float[ivar] != cut_tmp)  {pass_all_cuts = false; break;}
        }
      }
      
      if(!pass_all_cuts) {continue;}
      //------------------------------------------------------------
      
      integral_MC_fake+= weight;
    }
  }
  
  //FAKES FROM DATA
  double integral_data_fake = 0;
  TString inputfile = "Ntuples/Fakes.root";
  TFile* file_fake = TFile::Open( inputfile.Data() );
  TTree* tree = (TTree*) file_fake->Get("tree");
  tree->SetBranchAddress("Weight", &weight);
  tree->SetBranchAddress("channelInt", &i_channel);
  for(int ivar=0; ivar<v_cut_name.size(); ivar++)
  {
    //Use only these variables for cuts
    if(v_cut_name[ivar] == "nJets" || v_cut_name[ivar] == "nJets_CSVL" || v_cut_name[ivar] == "AdditionalMuonIso" || v_cut_name[ivar] == "AdditionalEleIso" || v_cut_name[ivar] == "AddLepPT")
    {
      tree->SetBranchAddress(v_cut_name[ivar].Data(), &v_cut_float[ivar]);
    }
  }
  
  
  
  
  for(int ientry=0; ientry<tree->GetEntries(); ientry++)
  {
    weight = 0; i_channel = 9;
    tree->GetEntry(ientry);
  
    
    //Apply cuts (only those on jets, bjets, leptons)
    //------------------------------------------------------------
    if(fakes_summed_channels)
    {
      //Fake muon
      if( (channel=="uuu" || channel=="eeu") && i_channel != 0 && i_channel != 2) {continue;}
      //Fake electron
      else if( (channel=="eee" || channel=="uue") && i_channel != 1 && i_channel != 3 ) {continue;}
    }
    else
    {
      if(channel == "uuu" && i_channel != 0) {continue;}
      else if(channel == "uue" && i_channel != 1) {continue;}
      else if(channel == "eeu" && i_channel != 2) {continue;}
      else if(channel == "eee" && i_channel != 3) {continue;}
    }
    
    float cut_tmp = 0; bool pass_all_cuts = true;
    for(int ivar=0; ivar<v_cut_name.size(); ivar++)
    {
      //Always compute the ratio in the WZ CR !
      //NB : Make the hypothesis that the ratio [MC fakes]/[Data fakes enriched region] is the same in WZ CR and SR
      if( (v_cut_name[ivar] == "nJets" && v_cut_float[ivar] == 0) || (v_cut_name[ivar] == "nJets_CSVL" && v_cut_float[ivar] != 0) ) {pass_all_cuts = false; break;}
      //Also apply cuts on leptons
      else if(v_cut_def[ivar] != "" && (v_cut_name[ivar] == "AdditionalMuonIso" || v_cut_name[ivar] == "AdditionalEleIso" || v_cut_name[ivar] == "AddLepPT") )
      {
        cut_tmp = Find_Number_In_TString(v_cut_def[ivar]);
        if(v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] < cut_tmp)		 {pass_all_cuts = false; break;}
        else if(v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] > cut_tmp)	 {pass_all_cuts = false; break;}
        else if(v_cut_def[ivar].Contains(">") && v_cut_float[ivar] <= cut_tmp)	 {pass_all_cuts = false; break;}
        else if(v_cut_def[ivar].Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
        else if(v_cut_def[ivar].Contains("==") && v_cut_float[ivar] != cut_tmp)  {pass_all_cuts = false; break;}
      }
    }
    
    if(!pass_all_cuts) {continue;}
    //------------------------------------------------------------
    
    integral_data_fake+= weight;
  }
  
  //cout<<"integral_MC_fake = "<<integral_MC_fake<<" || integral_data_fake = "<<integral_data_fake<<endl;
  
  float ratio = integral_MC_fake / integral_data_fake; //cout<<"ratio = "<<ratio<<endl;
  
  return ratio;
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

//FIXME : need to replace "Fakes" with name of Ntuple containing fakes from data !

//Reader function. Uses output from training (weights, ...) and read samples to create distributions of the BDT discriminant *OR* mWt
int theMVAtool::Read(TString template_name, bool fakes_from_data, bool real_data, bool fakes_summed_channels)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mWt" || template_name == "m3l") {cout<<FYEL("--- Producing "<<template_name<<" Templates ---")<<endl;}
  else {cout<<BOLD(FRED("--- ERROR : invalid template_name value ! Exit !"))<<endl; cout<<"Valid names are : BDT/BDTttZ/mWt/m3l !"<<endl; return 0;}
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  if(!fakes_from_data) {cout<<FYEL("--- Using fakes from MC ---")<<endl;}
  else {cout<<FYEL("--- Using fakes from data ---")<<endl;}
  if(!real_data) {cout<<FYEL("--- Not using real data ---")<<endl<<endl;}
  else {cout<<FYEL("--- Using REAL data ---")<<endl<<endl;}
  if(fakes_summed_channels) {cout<<FYEL("--- Fakes : summing channels 2 by 2 (artificial stat. increase) ---")<<endl<<endl;}
  
  TString output_file_name = "outputs/Reader_" + template_name + filename_suffix + ".root";
  TFile* file_output = TFile::Open( output_file_name, "RECREATE" );
  
  TString output_file_name_eee = "outputs/Reader_eee_" + template_name + filename_suffix + ".root";
  TFile* file_output_eee = TFile::Open( output_file_name_eee, "RECREATE" );
  TString output_file_name_eeu = "outputs/Reader_eeu_" + template_name + filename_suffix + ".root";
  TFile* file_output_eeu = TFile::Open( output_file_name_eeu, "RECREATE" );
  TString output_file_name_uue = "outputs/Reader_uue_" + template_name + filename_suffix + ".root";
  TFile* file_output_uue = TFile::Open( output_file_name_uue, "RECREATE" );
  TString output_file_name_uuu = "outputs/Reader_uuu_" + template_name + filename_suffix + ".root";
  TFile* file_output_uuu = TFile::Open( output_file_name_uuu, "RECREATE" );
  
  TString outputfile = "outputs/Reader_ControlPlots_" + template_name + filename_suffix + ".root";
  TFile* fileoutput = TFile::Open( outputfile, "RECREATE" );
  
  /*TString outputfile_eee = "outputs/Reader_ControlPlots_eee_" + template_name + filename_suffix + ".root";
  TFile* fileoutput_eee = TFile::Open( outputfile_eee, "RECREATE" );
  TString outputfile_eeu = "outputs/Reader_ControlPlots_eeu_" + template_name + filename_suffix + ".root";
  TFile* fileoutput_eeu = TFile::Open( outputfile_eeu, "RECREATE" );
  TString outputfile_uue = "outputs/Reader_ControlPlots_uue_" + template_name + filename_suffix + ".root";
  TFile* fileoutput_uue = TFile::Open( outputfile_uue, "RECREATE" );
  TString outputfile_uuu = "outputs/Reader_ControlPlots_uuu_" + template_name + filename_suffix + ".root";
  TFile* fileoutput_uuu = TFile::Open( outputfile_uuu, "RECREATE" );
  */
  
  
  mkdir("outputs",0777);
  
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
  
  //Look in the variable vectors, and find the positions of the 2 floats associated with these variables
  //Needed in case we want to create templates for 1 of these variables !
  int i_mWt = -1, i_m3l = -1;
  for(int i=0; i<var_list.size(); i++)
  {
    if(var_list[i] == "m3l") {i_m3l = i; break;}
  }
  for(int i=0; i<v_cut_name.size(); i++)
  {
    if(v_cut_name[i] == "mWt") {i_mWt = i; break;}
  }
  
  // --- Book the MVA methods
  TString dir    = "weights/";
  if(template_name == "BDT" || template_name == "BDTttZ")
  {
    for(int ichan=0; ichan<channel_list.size(); ichan++) //Book the method for each channel (separate BDTs)
    {
      cout << " CHANNEL " << channel_list[ichan] << endl;
      TString MVA_method_name = template_name + "_" + channel_list[ichan] + filename_suffix + TString(" method");
      TString weightfile = dir + template_name + "_" + channel_list[ichan] + filename_suffix + TString(".weights.xml"); //Contains weights related to BDT branches
      reader->BookMVA( MVA_method_name, weightfile );
    }
  }
  
  TH1::SetDefaultSumw2();
  TH1F *hist_BDT(0), *hist_BDTG(0);//, *hist_cutflow(0), *hist_cutflowweighted(0);
  TH1F *hist_uuu = 0, *hist_uue = 0, *hist_eeu = 0, *hist_eee = 0;
  TH1F *h_sum_fake = 0;
  
  // --- Systematics loop
  for(int isyst=0; isyst<syst_list.size(); isyst++)
  {
    //Loop on samples, syst., events ---> Fill histogram/channel --> Write()
    for(int isample=0; isample<sample_list.size(); isample++)
    {
      if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
      else if(fakes_from_data && (sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only
      
      if(!real_data && sample_list[isample].Contains("Data")) {continue;} //Don't use the real data
      
      if((sample_list[isample].Contains("Data") || sample_list[isample].Contains("Fakes")) && syst_list[isyst]!="") {continue;}
      //else if(sample_list[isample].Contains("ttZ") && syst_list[isyst].Contains("Q2")) {continue;} //incompatible w/ ttZMad
      
      TString inputfile = "Ntuples/" + sample_list[isample] + "_80X.root";
      TFile* file_input = TFile::Open( inputfile.Data() );
      
      std::cout << "--- Select "<<sample_list[isample]<<" sample" << std::endl;
      
      if(fakes_from_data || (!fakes_from_data && isample <= (sample_list.size() - 3)) ) //Don't reinitialize histos -> sum MC fakes !
      {
        // Book output histograms
        if (template_name == "BDT" || template_name == "BDTttZ") //create histogram for each channel (-1 = bkg, +1 = signal)
        {
          hist_uuu     = new TH1F( (template_name+"_uuu").Data(),           (template_name+"_uuu").Data(),           nbin, -1, 1 );
          hist_uue     = new TH1F( (template_name+"_uue").Data(),           (template_name+"_uue").Data(),           nbin, -1, 1 );
          hist_eeu     = new TH1F( (template_name+"_eeu").Data(),           (template_name+"_eeu").Data(),           nbin, -1, 1 );
          hist_eee     = new TH1F( (template_name+"_eee").Data(),           (template_name+"_eee").Data(),           nbin, -1, 1 );
        }
        else if (template_name == "mWt")
        {
          hist_uuu     = new TH1F( "mWt_uuu",           "mWt_uuu",           nbin, 0, 100 );
          hist_uue     = new TH1F( "mWt_uue",           "mWt_uue",           nbin, 0, 100 );
          hist_eeu     = new TH1F( "mWt_eeu",           "mWt_eeu",           nbin, 0, 100 );
          hist_eee     = new TH1F( "mWt_eee",           "mWt_eee",           nbin, 0, 100 );
        }
        else if (template_name == "m3l")
        {
          hist_uuu     = new TH1F( "m3l_uuu",           "m3l_uuu",           nbin, 0, 150 );
          hist_uue     = new TH1F( "m3l_uue",           "m3l_uue",           nbin, 0, 150 );
          hist_eeu     = new TH1F( "m3l_eeu",           "m3l_eeu",           nbin, 0, 150 );
          hist_eee     = new TH1F( "m3l_eee",           "m3l_eee",           nbin, 0, 150 );
        }
      }
      
    //  hist_cutflow = new TH1F( "cutflow",          "cutflow",           v_cut_name.size(), -0.5, v_cut_name.size()-0.5 );
    //  hist_cutflowweighted = new TH1F( "cutflow",          "cutflow",           v_cut_name.size(), -0.5, v_cut_name.size()-0.5 );
      
      TTree* tree(0);
      TString tree_name = "";
      
      //For JES & JER systematics, need a different tree (modify the variables distributions' shapes)
      if(syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") ) {tree = (TTree*) file_input->Get(syst_list[isyst].Data());}
      else {tree = (TTree*) file_input->Get("tree");}
      
      // Prepare the event tree
      for(int i=0; i<var_list.size(); i++)
      {
        tree->SetBranchAddress(var_list[i].Data(), &vec_variables[i]);
      }
      for(int i=0; i<v_cut_name.size(); i++)
      {
        tree->SetBranchAddress(v_cut_name[i].Data(), &v_cut_float[i]);
      }
      
      float i_channel; tree->SetBranchAddress("channelInt", &i_channel);
      float weight;
      //For all other systematics, only the events weights change
      if(syst_list[isyst] == "" || syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES"))	{tree->SetBranchAddress("Weight", &weight);}
      else {tree->SetBranchAddress(syst_list[isyst].Data(), &weight);}
      
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
              
              if(v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] < cut_tmp)		 {pass_all_cuts = false; }
              else if(v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] > cut_tmp)	 {pass_all_cuts = false; }
              else if(v_cut_def[ivar].Contains(">") && v_cut_float[ivar] <= cut_tmp)	 {pass_all_cuts = false; }
              else if(v_cut_def[ivar].Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; }
              else if(v_cut_def[ivar].Contains("==") && v_cut_float[ivar] != cut_tmp)  {pass_all_cuts = false; }
              else {
                cutArray[ivar]++;
               // cout << wcutArray[ivar] << endl;
                wcutArray[ivar] = wcutArray[ivar] + weight;
               // cout << " wcutArray[ivar] " << wcutArray[ivar] << " weight " << weight << endl;
      //          hist_cutflowweighted->Fill(ivar,weight);
      //          hist_cutflow->Fill(ivar);
              }
                
            }
            else //If 2 conditions in the cut, break it in 2
            {
              TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first; TString cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
              //CUT 1
              bool passed = false;
              cut_tmp = Find_Number_In_TString(cut1);
              if(cut1.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; }
              else if(cut1.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; }
              else if(cut1.Contains(">") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; }
              else if(cut1.Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; }
              else if(cut1.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; }
              else(passed = true);
              //CUT 2
              cut_tmp = Find_Number_In_TString(cut2);
              if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; }
              else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; }
              else if(cut2.Contains(">") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; }
              else if(cut2.Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; }
              else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; }
              else if(passed) {
                cutArray[ivar]++;
                wcutArray[ivar] = wcutArray[ivar]+ weight;
      //          hist_cutflowweighted->Fill(ivar,weight);
      //          hist_cutflow->Fill(ivar);
              }
            }
          }
          else {
            cutArray[ivar]++;
       //     hist_cutflowweighted->Fill(ivar,weight);
       //     hist_cutflow->Fill(ivar);
          }
        }
        
        totalEntries++;
        wtotalEntries = wtotalEntries + weight;
        if(!pass_all_cuts) {continue;}
        endEntries++;
        wendEntries = wendEntries + weight;
        //------------------------------------------------------------
        //------------------------------------------------------------
        
        // --- Return the MVA outputs and fill into histograms
        if (template_name == "BDT" || template_name == "BDTttZ")
        {
          //For fakes, can choose to sum channels 2 by 2 to increase statistics
          if(fakes_summed_channels && sample_list[isample].Contains("Fakes"))
          {
            if(i_channel == 0 || i_channel == 2)
            {
              hist_uuu->Fill( reader->EvaluateMVA( template_name+"_uuu"+filename_suffix+ " method"), weight);
              hist_eeu->Fill( reader->EvaluateMVA( template_name+"_eeu"+filename_suffix+ " method"), weight);
            }
            else if(i_channel == 1 || i_channel == 3)
            {
             hist_uue->Fill( reader->EvaluateMVA( template_name+"_uue"+filename_suffix+" method"), weight);
              hist_eee->Fill( reader->EvaluateMVA( template_name+"_eee"+filename_suffix+" method"), weight);
            }
            else {cout<<__LINE__<<BOLD(FRED(" : problem"))<<endl;}
          }
          //If sample is not fake or fakes_summed_channels = false
          else
          {
           // cout << "i_channel " << i_channel << endl;
            if(i_channel == 0) 		{hist_uuu->Fill( reader->EvaluateMVA( template_name+"_uuu"+filename_suffix+ " method"), weight);}
            else if(i_channel == 1) {hist_uue->Fill( reader->EvaluateMVA( template_name+"_uue"+filename_suffix+" method"), weight);}
            else if(i_channel == 2) {hist_eeu->Fill( reader->EvaluateMVA( template_name+"_eeu"+filename_suffix+" method"), weight);}
            else if(i_channel == 3) {hist_eee->Fill( reader->EvaluateMVA( template_name+"_eee"+filename_suffix+" method"), weight);}
            else if(i_channel == 9 || weight == 0) {cout<<__LINE__<<BOLD(FRED(" : problem  weight")) << weight <<endl;}
           
           
          }
        }
        // --- Return the MVA outputs and fill into histograms
        else if (template_name == "mWt")
        {
          //For fakes, can choose to sum channels 2 by 2 to increase statistics
          if(fakes_summed_channels && sample_list[isample].Contains("Fakes"))
          {
            if(i_channel == 0 || i_channel == 2)
            {
              hist_uuu->Fill( v_cut_float[i_mWt], weight); hist_eeu->Fill( v_cut_float[i_mWt], weight);
            }
            else if(i_channel == 1 || i_channel == 3)
            {
              hist_uue->Fill( v_cut_float[i_mWt], weight); hist_eee->Fill( v_cut_float[i_mWt], weight);
            }
            else {cout<<__LINE__<<BOLD(FRED(" : problem"))<<endl;}
          }
          //If sample is not fake or fakes_summed_channels = false
          else
          {
            if(i_channel == 0) 		{hist_uuu->Fill( v_cut_float[i_mWt], weight);}
            else if(i_channel == 1) {hist_uue->Fill( v_cut_float[i_mWt], weight);}
            else if(i_channel == 2) {hist_eeu->Fill( v_cut_float[i_mWt], weight);}
           else if(i_channel == 3) {hist_eee->Fill( v_cut_float[i_mWt], weight);}
            else {cout<<__LINE__<<BOLD(FRED(" : problem  weight ")) << weight<<endl;}
          }
        }
        else if (template_name == "m3l") //NB : order is important ! m3l must be 1rst in vector
        {
          //For fakes, can choose to sum channels 2 by 2 to increase statistics
          if(fakes_summed_channels && sample_list[isample].Contains("Fakes"))
          {
            if(i_channel == 0 || i_channel == 2)
            {
              hist_uuu->Fill( v_cut_float[i_m3l], weight); hist_eeu->Fill( v_cut_float[i_m3l], weight);
            }
            else if(i_channel == 1 || i_channel == 3)
            {
              hist_uue->Fill( v_cut_float[i_m3l], weight); hist_eee->Fill( v_cut_float[i_m3l], weight);
            }
            else {cout<<__LINE__<<BOLD(FRED(" : problem"))<<endl;}
          }
          //If sample is not fake or fakes_summed_channels = false
          else
          {
            if(i_channel == 0) 		{hist_uuu->Fill( v_cut_float[i_m3l], weight);}
            else if(i_channel == 1) {hist_uue->Fill( v_cut_float[i_m3l], weight);}
            else if(i_channel == 2) {hist_eeu->Fill( v_cut_float[i_m3l], weight);}
           else if(i_channel == 3) {hist_eee->Fill( v_cut_float[i_m3l], weight);}
           else {cout<<__LINE__<<BOLD(FRED(" : problem"))<<endl;}
          }
        }
        
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
      //--- RE-SCALING (NB : bin content & error !)
      //If fakes from data, re-scale histos to expected MC fake yields via function Compute_Fake_Ratio
      float fake_ratio = 0;
      if(fakes_from_data && sample_list[isample] == "Fakes" )
      {
        //cout<<"hist_uuu = "<<hist_uuu->Integral()<<endl;
        fake_ratio = Compute_Fake_Ratio("uuu", fakes_summed_channels); hist_uuu->Scale(fake_ratio);
        //cout<<"hist_uuu = "<<hist_uuu->Integral()<<endl;
        fake_ratio = Compute_Fake_Ratio("uue", fakes_summed_channels); hist_uue->Scale(fake_ratio);
        fake_ratio = Compute_Fake_Ratio("eeu", fakes_summed_channels); hist_eeu->Scale(fake_ratio);
        //cout<<"hist_eee = "<<hist_eee->Integral()<<endl;
        fake_ratio = Compute_Fake_Ratio("eee", fakes_summed_channels); hist_eee->Scale(fake_ratio);
        //cout<<"hist_eee = "<<hist_eee->Integral()<<endl;
      }
      //Re-scale to desired luminosity, unless it's data
      if(sample_list[isample] != "Data")
      {
        //cout <<" luminosity_rescale " << luminosity_rescale << endl;
        hist_uuu->Scale(luminosity_rescale); hist_uue->Scale(luminosity_rescale); hist_eeu->Scale(luminosity_rescale); hist_eee->Scale(luminosity_rescale);
       // hist_cutflowweighted->Scale(luminosity_rescale);
      }
      /* if(sample_list[isample].Contains("FCNCttbarZct"))
       {
       cout << " scaling " <<  hist_uuu->GetBinContent(1) << " becomes " ;
       hist_uuu->Scale(3); hist_uue->Scale(3); hist_eeu->Scale(3); hist_eee->Scale(3);
       cout << hist_uuu->GetBinContent(1) << endl;
       }*/
      
      // --- Write histograms
      
      
      
      file_output->cd();
      
      
      //NB : theta name convention = <observable>__<process>[__<uncertainty>__(plus,minus)]
      TString output_histo_name = "";
      TString syst_name = "";
      if(syst_list[isyst] != "") {syst_name = "__" + syst_list[isyst];}
      
      TString sample_name = sample_list[isample];
      if(real_data && sample_list[isample] == "Data") {sample_name = "DATA";} //THETA CONVENTION
      
      if(!fakes_from_data && (isample == (sample_list.size() - 3) || isample == (sample_list.size() - 2)) ) //If sample is MC fake, don't reinitialize histos -> sum 3 MC fake samples
      {
        continue;
      }
      else if( (!fakes_from_data && isample == (sample_list.size() - 1)) || (fakes_from_data && sample_list[isample] == "Fakes") ) //Last fake MC sample or data-driven fakes -> write fake histo w/ special name (for THETA)
      {
        output_histo_name = template_name+"_uuu__FakeMu" + syst_name;
        hist_uuu->Write(output_histo_name.Data());
        output_histo_name = template_name+"_uue__FakeEl" + syst_name;
        hist_uue->Write(output_histo_name.Data());
        output_histo_name = template_name+"_eeu__FakeMu" + syst_name;
        hist_eeu->Write(output_histo_name.Data());
        output_histo_name = template_name+"_eee__FakeEl" + syst_name;
        hist_eee->Write(output_histo_name.Data());
      }
      else //If fakes are not considered, or if sample is not fake --> write directly !
      {
        output_histo_name = template_name+"_uuu__" + sample_name + syst_name;
        hist_uuu->Write(output_histo_name.Data());
        output_histo_name = template_name+"_uue__" + sample_name + syst_name;
        hist_uue->Write(output_histo_name.Data());
        output_histo_name = template_name+"_eeu__" + sample_name + syst_name;
        hist_eeu->Write(output_histo_name.Data());
        output_histo_name = template_name+"_eee__" + sample_name + syst_name;
        hist_eee->Write(output_histo_name.Data());
      }
      
      
      file_output_eee->cd();
      if(!fakes_from_data && (isample == (sample_list.size() - 3) || isample == (sample_list.size() - 2)) ) //If sample is MC fake, don't reinitialize histos -> sum 3 MC fake samples
      {
        continue;
      }
      else if( (!fakes_from_data && isample == (sample_list.size() - 1)) || (fakes_from_data && sample_list[isample] == "Fakes") ) //Last fake MC sample or data-driven fakes -> write fake histo w/ special name (for THETA)
      {
        output_histo_name = template_name+"_eee__FakeEl" + syst_name;
        hist_eee->Write(output_histo_name.Data());
      }
      else //If fakes are not considered, or if sample is not fake --> write directly !
      {
        output_histo_name = template_name+"_eee__" + sample_name + syst_name;
        hist_eee->Write(output_histo_name.Data());
      }
      file_output_eeu->cd();
      if(!fakes_from_data && (isample == (sample_list.size() - 3) || isample == (sample_list.size() - 2)) ) //If sample is MC fake, don't reinitialize histos -> sum 3 MC fake samples
      {
        continue;
      }
      else if( (!fakes_from_data && isample == (sample_list.size() - 1)) || (fakes_from_data && sample_list[isample] == "Fakes") ) //Last fake MC sample or data-driven fakes -> write fake histo w/ special name (for THETA)
      {
        output_histo_name = template_name+"_eeu__FakeMu" + syst_name;
        hist_eeu->Write(output_histo_name.Data());
        
      }
      else //If fakes are not considered, or if sample is not fake --> write directly !
      {
        output_histo_name = template_name+"_eeu__" + sample_name + syst_name;
        hist_eeu->Write(output_histo_name.Data());
        
      }
      file_output_uue->cd();
      if(!fakes_from_data && (isample == (sample_list.size() - 3) || isample == (sample_list.size() - 2)) ) //If sample is MC fake, don't reinitialize histos -> sum 3 MC fake samples
      {
        continue;
      }
      else if( (!fakes_from_data && isample == (sample_list.size() - 1)) || (fakes_from_data && sample_list[isample] == "Fakes") ) //Last fake MC sample or data-driven fakes -> write fake histo w/ special name (for THETA)
      {
        
        output_histo_name = template_name+"_uue__FakeEl" + syst_name;
        hist_uue->Write(output_histo_name.Data());
        
      }
      else //If fakes are not considered, or if sample is not fake --> write directly !
      {
        
        output_histo_name = template_name+"_uue__" + sample_name + syst_name;
        hist_uue->Write(output_histo_name.Data());
        
      }
      file_output_uuu->cd();
      if(!fakes_from_data && (isample == (sample_list.size() - 3) || isample == (sample_list.size() - 2)) ) //If sample is MC fake, don't reinitialize histos -> sum 3 MC fake samples
      {
        continue;
      }
      else if( (!fakes_from_data && isample == (sample_list.size() - 1)) || (fakes_from_data && sample_list[isample] == "Fakes") ) //Last fake MC sample or data-driven fakes -> write fake histo w/ special name (for THETA)
      {
        output_histo_name = template_name+"_uuu__FakeMu" + syst_name;
        hist_uuu->Write(output_histo_name.Data());
        
      }
      else //If fakes are not considered, or if sample is not fake --> write directly !
      {
        output_histo_name = template_name+"_uuu__" + sample_name + syst_name;
        hist_uuu->Write(output_histo_name.Data());
        
      }
      
      
      
      
      
      
      //don't delete if processing MC fake templates (unless all the loops have reached their ends)
      if 	(
           fakes_from_data ||
           (!fakes_from_data &&
            (isample < (sample_list.size() - 3) ||    (isample == (sample_list.size() - 1) && isyst == (syst_list.size() - 1)) )
            ) )
      {
        //cout<<"deleting dynamic histograms"<<endl;
        delete hist_uuu; delete hist_uue; delete hist_eeu; delete hist_eee;
      }
      
      //cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;
    } //end sample loop
    cout<<"Done with "<<syst_list[isyst]<<" syst"<<endl;
  } //end syst loop
  
  file_output->Close();
  file_output_eee->Close();
  file_output_eeu->Close();
  file_output_uue->Close();
  file_output_uuu->Close();
  std::cout << "--- Created root file: \""<<file_output->GetName()<<"\" containing the output histograms" << std::endl;
  std::cout << "==> Reader() is done!" << std::endl << std::endl;
  return 0;
}



//-----------------------------------------------------------------------------------------
//     ____   ____      _                             ___       _       _         _
//    / ___| |  _ \    | |_   _ __    ___    ___     ( _ )     | |__   (_)  ___  | |_
//   | |     | |_) |   | __| | '__|  / _ \  / _ \    / _ \/\   | '_ \  | | / __| | __|
//   | |___  |  _ <    | |_  | |    |  __/ |  __/   | (_>  <   | | | | | | \__ \ | |_
//    \____| |_| \_\    \__| |_|     \___|  \___|    \___/\/   |_| |_| |_| |___/  \__|
//
//-----------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Reads the output from Read() and determines a cut value on BDT to create a BDT CR
// --> Can use this cut value as input parameter in Create_Control_Trees()
float theMVAtool::Determine_Control_Cut()
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("--- Determine Control Cut ---")<<endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  
  TString input_file_name = "outputs/Reader_BDT" + filename_suffix + ".root";
  TFile* f = 0;
  f = TFile::Open(input_file_name.Data());
  if(f == 0) {cout<<endl<<"--- No templates found for BDT -- Can't determine BDT CR cut value !"<<endl<<endl; return 0;}
  
  TH1F *h_sum_bkg(0), *h_sig(0), *h_tmp(0);
  
  TString input_histo_name = "";
  
  for(int isample=0; isample<sample_list.size(); isample++)
  {
    if(sample_list[isample].Contains("Data")) {continue;}
    //cout<<"--- Use sample "<<sample_list[isample]<<endl;
    
    for(int ichan=0; ichan<channel_list.size(); ichan++)
    {
      h_tmp = 0;
      
      input_histo_name = "BDT_" + channel_list[ichan] + "__" + sample_list[isample];
      
      if(!f->GetListOfKeys()->Contains(input_histo_name.Data())) {continue;}
      
      h_tmp = (TH1F*) f->Get(input_histo_name.Data())->Clone();
      
      if(sample_list[isample] == "NP_overlay_ST_FCNC_zut")
      {
        if(h_sig == 0) {h_sig = (TH1F*) h_tmp->Clone();}
        else {h_sig->Add(h_tmp);}
      }
      else
      {
        if(h_sum_bkg == 0) {h_sum_bkg = (TH1F*) h_tmp->Clone();}
        else {h_sum_bkg->Add(h_tmp);}
      }
    } //end channel loop
  } //end sample loop
  
  if(h_sum_bkg == 0 || h_sig == 0) {cout<<endl<<BOLD(FRED("--- Empty histogram ! Exit !"))<<endl<<endl; return 0;}
  
  //S+B histogram
  TH1F* h_total = (TH1F*) h_sum_bkg->Clone();
  h_total->Add(h_sig);
  
  //Normalization
  //h_total->Scale(1/h_total->Integral()); //Integral() : Return integral of bin contents in range [binx1,binx2] (inclusive !)
  //h_sum_bkg->Scale(1/h_sum_bkg->Integral());
  //h_sig->Scale(1/h_sig->Integral());
  
  double sig_over_total = 100; //initialize to unreasonable value
  int bin_cut = -1; //initialize to false value
  int nofbins = h_total->GetNbinsX();
  
  for(int ibin=nofbins; ibin>0; ibin--)
  {
    //Search the bin w/ lowest sig/total, while keeping enough bkg events (criterion needs to be optimized/tuned)
    if( (h_sig->Integral(1, ibin) / h_total->Integral(1, ibin)) < sig_over_total && (h_sum_bkg->Integral(1, ibin) / h_sum_bkg->Integral()) >= 0.6 )
    {
      bin_cut = ibin;
      sig_over_total = h_sig->Integral(1, bin_cut) / h_total->Integral(1,bin_cut);
    }
  }
  
  double cut = h_total->GetBinLowEdge(bin_cut+1); //Get the BDT cut value to apply to create a BDT CR control tree
  
  //Create plot to represent the cut on BDT
  TCanvas* c = new TCanvas("c", "Signal VS Background");
  gStyle->SetOptStat(0);
  h_sum_bkg->GetXaxis()->SetTitle("Discriminant");
  h_sum_bkg->SetTitle("Signal VS Background");
  h_sum_bkg->SetLineColor(kBlue);
  h_sig->SetLineColor(kGreen);
  h_sum_bkg->Draw("HIST");
  h_sig->Draw("HIST SAME");
  TLegend* leg = new TLegend(0.7,0.75,0.88,0.85);
  leg->SetHeader("");
  leg->AddEntry(h_sig,"Signal","L");
  leg->AddEntry("h_sum_bkg","Background","L");
  leg->Draw();
  //Draw vertical line at cut value
  TLine* l = new TLine(cut, 0, cut, h_sum_bkg->GetMaximum());
  l->SetLineWidth(3);
  l->Draw("");
  c->SaveAs("outputs/Signal_Background_BDT"+filename_suffix+".png");
  
  //Cout some results
  cout<<"---------------------------------------"<<endl;
  cout<<"* Cut Value = "<<cut<<endl;
  cout<<"-> BDT_CR defined w/ all events inside bins [1 ; "<<bin_cut<<"] of the BDT distribution!"<<endl<<endl;
  cout<<"* Signal integral = "<<h_sig->Integral(1, bin_cut)<<" / Total integral "<<h_total->Integral(1, bin_cut)<<endl;
  cout<<"Signal contamination in CR --> Sig/Total = "<<sig_over_total<<endl;
  cout<<"Bkg(CR) / Bkg(Total) = "<<h_sum_bkg->Integral(1,bin_cut) / h_sum_bkg->Integral()<<endl;
  cout<<"---------------------------------------"<<endl<<endl;
  
  //for(int i=0; i<h_sig->GetNbinsX(); i++) {cout<<"bin content "<<i+1<<" = "<<h_sig->GetBinContent(i+1)<<endl;} //If want to verify that the signal is computed correctly
  f->Close(); delete c; delete leg; delete l;
  return cut;
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Similar to Read(). Takes cut value as input parameter, and outputs histograms containing only events verifying BDT<cut (mainly bkg events) --> Create histogram with these events for further control studies
void theMVAtool::Create_Control_Trees(bool fakes_from_data, bool cut_on_BDT, double cut = 0, bool use_pseudodata = false)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("--- Create Control Trees ---")<<endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  if(!fakes_from_data) {cout<<FYEL("--- Using fakes from MC ---")<<endl;}
  else {cout<<FYEL("--- Using fakes from data ---")<<endl;}
  if(cut_on_BDT) {cout<<FYEL("--- Creating control tree WITH cut on BDT value // Cut value = "<<cut<<" ----")<<endl;}
  else if(!cut_on_BDT) {cout<<FYEL("--- Creating control tree WITHOUT cut on BDT value ---")<<endl;}
  
  reader = new TMVA::Reader( "!Color:!Silent" );
  
  mkdir("outputs",0777);
  
  // Name & adress of local variables which carry the updated input values during the event loop
  // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
  for(int i=0; i<var_list.size(); i++)
  {
    reader->AddVariable(var_list[i].Data(), &vec_variables[i]);
  }
  for(int i=0; i<v_cut_name.size(); i++)
  {
    if(v_cut_IsUsedForBDT[i] && !v_cut_def[i].Contains("==")) {reader->AddVariable(v_cut_name[i].Data(), &v_cut_float[i]);}
    else {reader->AddSpectator(v_cut_name[i].Data(), &v_cut_float[i]);}
  }
  
  // --- Book the MVA methods
  TString dir    = "weights/";
  
  // Book method
  for(int ichan=0; ichan<channel_list.size(); ichan++) //Book the method for each channel (separate BDTs)
  {
    TString MVA_method_name = "BDT_" + channel_list[ichan] + filename_suffix + TString(" method");
    TString weightfile = dir + "BDT_" + channel_list[ichan] + filename_suffix + TString(".weights.xml");
    if(cut_on_BDT) {reader->BookMVA( MVA_method_name, weightfile );}
  }
  
  //---Loop on histograms
  for(int isyst = 0; isyst < syst_list.size(); isyst++)
  {
    for(int isample=0; isample<sample_list.size(); isample++)
    {
      if(use_pseudodata && sample_list[isample] == "Data") {continue;} //Will generate pseudodata instead
      
      if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
      else if(fakes_from_data && (sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only
      
      TString inputfile = "Ntuples/" + sample_list[isample] + "_80X.root";
      TFile* file_input = TFile::Open( inputfile.Data() );
      
      if((sample_list[isample].Contains("Data") || sample_list[isample].Contains("Fakes")) && syst_list[isyst]!="") {continue;}
      //else if(sample_list[isample].Contains("ttZ") && syst_list[isyst].Contains("Q2")) {continue;} //incompatible w/ ttZMad
      
      //NB : call the output file here in UPDATE mode because otherwise I got memory errors
      TString output_file_name = "outputs/Control_Trees" + filename_suffix + ".root";
      TFile* output_file = TFile::Open( output_file_name, "UPDATE" );
      
      //Create new tree, that will be filled only with events verifying MVA<cut
      TTree *tree(0), *tree_control(0);
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
      
      float weight; float i_channel;
      tree_control->Branch("Weight", &weight, "weight/F"); //Give it the same name regardless of the systematic, since we create a separate tree for each syst anyway
      tree_control->Branch("channelInt", &i_channel, "i_channel/F");
      
      
      
      //For JES & JER systematics, need a different tree (modify the variables distributions' shapes)
      if(syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES") ) {tree = (TTree*) file_input->Get(syst_list[isyst].Data());}
      else {tree = (TTree*) file_input->Get("tree");}
      
      // SetBranchAddress
      for(int i=0; i<var_list.size(); i++)
      {
        tree->SetBranchAddress(var_list[i].Data(), &vec_variables[i]);
      }
      for(int i=0; i<v_cut_name.size(); i++)
      {
        tree->SetBranchAddress(v_cut_name[i].Data(), &v_cut_float[i]);
      }
      tree->SetBranchAddress("channelInt", &i_channel);
      //For all other systematics, only the events weights change
      if(syst_list[isyst] == "" || syst_list[isyst].Contains("JER") || syst_list[isyst].Contains("JES"))	{tree->SetBranchAddress("Weight", &weight);}
      else {tree->SetBranchAddress(syst_list[isyst].Data(), &weight);}
      
      
      // --- Event loop
      for(int ievt=0; ievt<tree->GetEntries(); ievt++)
      {
        weight = 0; i_channel = 9;

        tree->GetEntry(ievt);
        
        //------------------------------------------------------------
        
        //------------------------------------------------------------
        //------------------- Apply cuts -----------------------------
        float cut_tmp = 0; bool pass_all_cuts = true;
        
        for(int ivar=0; ivar<v_cut_name.size(); ivar++)
        {
          if(v_cut_def[ivar] != "")
          {
            if(!v_cut_def[ivar].Contains("&&")) //If cut contains only 1 condition
            {
              cut_tmp = Find_Number_In_TString(v_cut_def[ivar]);
              if(v_cut_def[ivar].Contains(">=") && v_cut_float[ivar] < cut_tmp)		 {pass_all_cuts = false; break;}
              else if(v_cut_def[ivar].Contains("<=") && v_cut_float[ivar] > cut_tmp)	 {pass_all_cuts = false; break;}
              else if(v_cut_def[ivar].Contains(">") && v_cut_float[ivar] <= cut_tmp)	 {pass_all_cuts = false; break;}
              else if(v_cut_def[ivar].Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
              else if(v_cut_def[ivar].Contains("==") && v_cut_float[ivar] != cut_tmp)  {pass_all_cuts = false; break;}
            }
            else //If 2 conditions in the cut, break it in 2
            {
              TString cut1 = Break_Cuts_In_Two(v_cut_def[ivar]).first; TString cut2 = Break_Cuts_In_Two(v_cut_def[ivar]).second;
              //CUT 1
              cut_tmp = Find_Number_In_TString(cut1);
              if(cut1.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; break;}
              else if(cut1.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; break;}
              else if(cut1.Contains(">") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; break;}
              else if(cut1.Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
              else if(cut1.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; break;}
              //CUT 2
              cut_tmp = Find_Number_In_TString(cut2);
              if(cut2.Contains(">=") && v_cut_float[ivar] < cut_tmp)			 {pass_all_cuts = false; break;}
              else if(cut2.Contains("<=") && v_cut_float[ivar] > cut_tmp)		 {pass_all_cuts = false; break;}
              else if(cut2.Contains(">") && v_cut_float[ivar] <= cut_tmp)		 {pass_all_cuts = false; break;}
              else if(cut2.Contains("<") && v_cut_float[ivar] >= cut_tmp) 	 {pass_all_cuts = false; break;}
              else if(cut2.Contains("==") && v_cut_float[ivar] != cut_tmp) 	 {pass_all_cuts = false; break;}
            }
          }
        }
        
        if(!pass_all_cuts) {continue;}
        //------------------------------------------------------------
        
        //------------------------------------------------------------
        if(cut_on_BDT)
        {
          if(i_channel == 0 && reader->EvaluateMVA( "BDT_uuu"+filename_suffix+" method") > cut) 		{continue;}
          else if(i_channel == 1 && reader->EvaluateMVA( "BDT_uue"+filename_suffix+" method") > cut) 	{continue;}
          else if(i_channel == 2 && reader->EvaluateMVA( "BDT_eeu"+filename_suffix+" method") > cut) 	{continue;}
          else if(i_channel == 3 && reader->EvaluateMVA( "BDT_eee"+filename_suffix+" method") > cut) 	{continue;}
        }
        
        tree_control->Fill();
      } //end event loop
      
      // --- Write histograms
      
      output_file->cd();
      
      //NB : theta name convention = <observable>__<process>__<uncertainty>[__(plus,minus)]
      TString output_tree_name = "Control_" + sample_list[isample];
      if (syst_list[isyst] != "") {output_tree_name+= "_" + syst_list[isyst];}
      
      tree_control->Write(output_tree_name.Data(), TObject::kOverwrite);
      
      delete tree_control;
      output_file->Close();
      //cout<<"Done with "<<sample_list[isample]<<" sample"<<endl;
    } //end sample loop
    cout<<"Done with "<<syst_list[isyst]<<" syst"<<endl;
  } //end syst loop
  
  std::cout << "--- Created root file containing the output trees" << std::endl;
  std::cout << "==> Create_Control_Trees() is done!" << std::endl << std::endl;
}


//Create histograms from control trees (in same file)
//NB : no separation by channel is made ! It is possible but it would require to create 4 times histograms at beginning...
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void theMVAtool::Create_Control_Histograms(bool fakes_from_data, bool use_pseudodata_CR_plots)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("--- Create Control Histograms ---")<<endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  if(!fakes_from_data) {cout<<FYEL("--- Using fakes from MC ---")<<endl;}
  else {cout<<"--- Using fakes from data ---"<<endl<<endl;}
  
  TString input_file_name = "outputs/Control_Trees" + filename_suffix + ".root";
  TString output_file_name = "outputs/Control_Histograms" + filename_suffix + ".root";
  TFile* f_input = TFile::Open( input_file_name ); TFile* f_output = TFile::Open( output_file_name, "RECREATE" );
  TTree* tree = 0;
  
  int binning = 10;
  TH1::SetDefaultSumw2();
  
  //Want to plot ALL variables (inside the 2 different variable vectors !)
  vector<TString> total_var_list;
  for(int i=0; i<v_cut_name.size(); i++)
  {
    total_var_list.push_back(v_cut_name[i].Data());
  }
  for(int i=0; i<var_list.size(); i++)
  {
    total_var_list.push_back(var_list[i].Data());
  }
  
  int nof_histos_to_create = ((sample_list.size() - 1) * total_var_list.size() * syst_list.size()) + total_var_list.size();
  
  for(int ichan=0; ichan<channel_list.size(); ichan++)
  {
    cout<<endl<<"################################"<<endl;
    cout<<" *** CHANNEL "<<channel_list[ichan]<<" ***"<<endl;
    cout<<endl<<"--- Going to create "<<nof_histos_to_create<<" histograms ! (* 4 channels = "<<nof_histos_to_create*4<<" in total)"<<endl;
    cout<<"--- This might take a while... !"<<endl;
    cout<<"################################"<<endl<<endl;
    
    for(int ivar=0; ivar<total_var_list.size(); ivar++)
    {
      cout<<"--- Processing variable : "<<total_var_list[ivar]<<endl;
      //Info contained in tree leaves. Need to create histograms first
      for(int isample = 0; isample < sample_list.size(); isample++)
      {
        if(use_pseudodata_CR_plots && sample_list[isample] == "Data") {continue;}
        
        for(int isyst=0; isyst<syst_list.size(); isyst++)
        {
          if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
          else if(fakes_from_data && (sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only
          
          if((sample_list[isample].Contains("Data") || sample_list[isample].Contains("Fakes")) && syst_list[isyst] != "") {continue;}
          //if(sample_list[isample].Contains("ttZ") && syst_list[isyst] == "Q2") {continue;} //incompatible w/ ttZMad
          
          TH1F* h_tmp = 0;
          if(total_var_list[ivar] == "mWt") 								{h_tmp = new TH1F( "","", binning, 0, 150 );}
          else if(total_var_list[ivar] == "METpt")						{h_tmp = new TH1F( "","", binning, 0, 400 );}
          else if(total_var_list[ivar] == "m3l")							{h_tmp = new TH1F( "","", binning, 80, 180 );}
          else if(total_var_list[ivar] == "ZCandMass") 					{h_tmp = new TH1F( "","", binning, 75, 105 );}
          else if(total_var_list[ivar] == "deltaPhilb") 					{h_tmp = new TH1F( "","", binning, -4, 4 );}
          else if(total_var_list[ivar] == "Zpt") 							{h_tmp = new TH1F( "","", binning, 0, 150 );}
          else if(total_var_list[ivar] == "ZEta")			 				{h_tmp = new TH1F( "","", binning, -4, 4 );}
          else if(total_var_list[ivar] == "asym") 						{h_tmp = new TH1F( "","", binning, -3, 3 );}
          else if(total_var_list[ivar] == "mtop") 						{h_tmp = new TH1F( "","", binning, 60, 210 );}
          else if(total_var_list[ivar] == "btagDiscri") 					{h_tmp = new TH1F( "","", binning, 0.4, 1.2 );}
          else if(total_var_list[ivar] == "etaQ")							{h_tmp = new TH1F( "","", binning, -4, 4 );}
          else if(total_var_list[ivar] == "nJets_CSVM")						{h_tmp = new TH1F( "","", 3, 0, 3 );}
          else if(total_var_list[ivar] == "AddLepPT")						{h_tmp = new TH1F( "","", binning, 0, 150 );}
          else if(total_var_list[ivar] == "AddLepETA")					{h_tmp = new TH1F( "","", binning, -4, 4 );}
          else if(total_var_list[ivar] == "LeadJetPT")					{h_tmp = new TH1F( "","", binning, 0, 150 );}
          else if(total_var_list[ivar] == "LeadJetEta")					{h_tmp = new TH1F( "","", binning, -4, 4 );}
          else if(total_var_list[ivar] == "dPhiZMET")						{h_tmp = new TH1F( "","", binning, -4, 4 );}
          else if(total_var_list[ivar] == "dPhiSMFCNCtop")						{h_tmp = new TH1F( "","", binning, -7, 7 );}
          else if(total_var_list[ivar] == "dPhiZAddLep")					{h_tmp = new TH1F( "","", binning, -4, 4 );}
          else if(total_var_list[ivar] == "dRAddLepBFromTop")				{h_tmp = new TH1F( "","", binning, 0, 1 );}
          else if(total_var_list[ivar] == "dRZAddLep")					{h_tmp = new TH1F( "","", binning, 0, 1 );}
          else if(total_var_list[ivar] == "dRZTop")						{h_tmp = new TH1F( "","", binning, 0, 1 );}
          else if(total_var_list[ivar] == "dRSMFCNCtop")						{h_tmp = new TH1F( "","", binning, 0, 10 );}
          else if(total_var_list[ivar] == "dRZc")						{h_tmp = new TH1F( "","", binning, 0, 7 );}
          else if(total_var_list[ivar] == "dRZb")						{h_tmp = new TH1F( "","", binning, 0, 7 );}
          else if(total_var_list[ivar] == "dRWlepc")						{h_tmp = new TH1F( "","", binning, 0, 7 );}
          else if(total_var_list[ivar] == "dPhiWlepb")						{h_tmp = new TH1F( "","", binning, -7, 7 );}
          else if(total_var_list[ivar] == "TopPT")						{h_tmp = new TH1F( "","", binning, 0, 150 );}
          else if(total_var_list[ivar] == "nJets")						{h_tmp = new TH1F( "","", 5, 1, 6 );}
          else if(total_var_list[ivar] == "ptQ")							{h_tmp = new TH1F( "","", binning, 0, 150 );}
          else if(total_var_list[ivar] == "dRjj")							{h_tmp = new TH1F( "","", binning, 0, 1 );}
          else if(total_var_list[ivar] == "AdditionalEleIso")				{h_tmp = new TH1F( "","", binning, 0, 0.5 );}
          else if(total_var_list[ivar] == "AdditionalMuonIso")			{h_tmp = new TH1F( "","", binning, 0, 0.5 );}
          else if(total_var_list[ivar] == "AddLepPT")						{h_tmp = new TH1F( "","", binning, 0, 150 );}
          else if(total_var_list[ivar] == "dPhi_ZBJet")						{h_tmp = new TH1F( "","", binning, -4, 4 );}
          else if(total_var_list[ivar] == "dR_ZBJet")						{h_tmp = new TH1F( "","", binning, 0, 1 );}
          else if(total_var_list[ivar] == "FCNCtop_M")						{h_tmp = new TH1F( "","", binning, 0, 500 );}
          else if(total_var_list[ivar] == "LeadingJetCSV")						{h_tmp = new TH1F( "","", binning, 0, 2 );}
          else if(total_var_list[ivar] == "SecondJetCSV")						{h_tmp = new TH1F( "","", binning, 0, 2 );}
          else if(total_var_list[ivar] == "NMediumBjets")						{h_tmp = new TH1F( "","", 6, -0.5, 5.5 );}
          else if(total_var_list[ivar] == "dR_ZBJet")						{h_tmp = new TH1F( "","", binning, 0, 1 );}
          else if(total_var_list[ivar] == "M_AddLepBJet")						{h_tmp = new TH1F( "","", binning, 0, 200 );}
          
          else {cout<<endl<<__LINE__<<BOLD(FRED(" --> !!Unknown variable!! Correct name or add it here"))<<endl<<endl;}
          
          TString tree_name = "Control_" + sample_list[isample];
          if(syst_list[isyst] != "") {tree_name+= "_" + syst_list[isyst];}
          if(!f_input->GetListOfKeys()->Contains(tree_name.Data()) && sample_list[isample] != "Data" ) {cout<<tree_name<<" : not found !"<<endl; continue;}
          tree = (TTree*) f_input->Get(tree_name.Data());
          //cout<<__LINE__<<endl;
          
          float weight = 0, tmp = 0, i_channel = 9;
          tree->SetBranchAddress(total_var_list[ivar], &tmp); //One variable at a time
          tree->SetBranchAddress("Weight", &weight);
          tree->SetBranchAddress("channelInt", &i_channel);
          
          int tree_nentries = tree->GetEntries();
          
          for(int ientry = 0; ientry<tree_nentries; ientry++)
          {
            weight = 0; tmp = 0; i_channel = 9;
            tree->GetEntry(ientry); //Read event
             //NB : No need to re-apply variables cuts here, as the control_tree is only filled with events that pass these cuts
            
            if(channel_list[ichan] == "uuu" && i_channel!= 0) {continue;}
            else if(channel_list[ichan] == "uue" && i_channel!= 1) {continue;}
            else if(channel_list[ichan] == "eeu" && i_channel!= 2) {continue;}
            else if(channel_list[ichan] == "eee" && i_channel!= 3) {continue;}
            else if(channel_list[ichan] == "9") {cout<<__LINE__<<" : problem !"<<endl;}
            
            h_tmp->Fill(tmp, weight); //Fill histogram -- weight already re-scaled to desired lumi in Create_Control_Trees !
          }
          
          //--- RE-SCALING (NB : bin content & error !)
          //If fakes from data, re-scale histos to expected MC fake yields via function Compute_Fake_Ratio
          float fake_ratio = 0;
          if(fakes_from_data && sample_list[isample] == "Fakes" )
          {
            fake_ratio = Compute_Fake_Ratio(channel_list[ichan], false); h_tmp->Scale(fake_ratio);
          }
          
          //Re-scale to desired luminosity, unless it's data
          if(sample_list[isample] != "Data")
          {
            h_tmp->Scale(luminosity_rescale);
          }
          
          TString output_histo_name = "Control_"+ channel_list[ichan] + "_" + total_var_list[ivar];
          output_histo_name+= "_" + sample_list[isample];
          if(syst_list[isyst] != "") {output_histo_name+= "_" + syst_list[isyst];}
          f_output->cd();
          //h_tmp->Write(output_histo_name.Data(), TObject::kOverwrite);
          h_tmp->Write(output_histo_name.Data());
          
          delete h_tmp;
        } //end syst loop
      } //end sample loop
    } //end var loop
  } //end channel loop
  
  
  f_input->Close();
  f_output->Close();
}


//-----------------------------------------------------------------------------------------
//                                     _               _           _
//    _ __    ___    ___   _   _    __| |   ___     __| |   __ _  | |_    __ _
//   | '_ \  / __|  / _ \ | | | |  / _` |  / _ \   / _` |  / _` | | __|  / _` |
//   | |_) | \__ \ |  __/ | |_| | | (_| | | (_) | | (_| | | (_| | | |_  | (_| |
//   | .__/  |___/  \___|  \__,_|  \__,_|  \___/   \__,_|  \__,_|  \__|  \__,_|
//   |_|
//-----------------------------------------------------------------------------------------

//Generates pseudo data histograms in the "control histograms file". Warning : named 'Data' !
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
int theMVAtool::Generate_PseudoData_Histograms_For_Control_Plots(bool fakes_from_data)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("--- Generate PseudoData Histos for CR Plots ---")<<endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  if(!fakes_from_data) {cout<<FYEL("--- Using fakes from MC ---")<<endl;}
  else {cout<<FYEL("--- Using fakes from data ---")<<endl;}
  
  TRandom3 therand(0);
  
  mkdir("outputs",0777);
  
  TString input_name = "outputs/Control_Histograms" + filename_suffix + ".root";
  TFile* file = 0;
  file = TFile::Open( input_name.Data(), "UPDATE");
  if(file == 0) {cout<<BOLD(FRED("--- ERROR : Control_Histograms file not found ! Exit !"))<<endl; return 0;}
  
  
  //Want to plot ALL variables (inside the 2 different variable vectors !)
  vector<TString> total_var_list;
  for(int i=0; i<v_cut_name.size(); i++)
  {
    total_var_list.push_back(v_cut_name[i].Data());
  }
  for(int i=0; i<var_list.size(); i++)
  {
    total_var_list.push_back(var_list[i].Data());
  }
  
  for(int ichan=0; ichan<channel_list.size(); ichan++)
  {
    TH1F *h_sum = 0, *h_tmp = 0;
    
    for(int ivar=0; ivar<total_var_list.size(); ivar++)
    {
      cout<<"--- "<<total_var_list[ivar]<<endl;
      
      h_sum = 0;
      
      for(int isample = 0; isample < sample_list.size(); isample++)
      {
        if(sample_list[isample].Contains("Data") || sample_list[isample].Contains("DATA")  ) {continue;} //Not using real data !!
        if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
        else if(fakes_from_data && (sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only
        if(sample_list[isample].Contains("NP_overlay_ST_FCNC_zut")) {continue; } // no signal in data
        h_tmp = 0;
        TString histo_name = "Control_" + channel_list[ichan] + "_" + total_var_list[ivar] + "_" + sample_list[isample];
        if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
        h_tmp = (TH1F*) file->Get(histo_name.Data());
        if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
        else {h_sum->Add(h_tmp);}
      }
      
      int nofbins = h_sum->GetNbinsX();
      
      for(int i=0; i<nofbins; i++)
      {
        double bin_content = h_sum->GetBinContent(i+1); //cout<<"Initial content = "<<bin_content<<endl;
        double new_bin_content = therand.Poisson(bin_content); //cout<<"New content = "<<new_bin_content<<endl;
        h_sum->SetBinContent(i+1, new_bin_content);
        h_sum->SetBinError(i+1, sqrt(new_bin_content)); //Poissonian error
      }
      
      file->cd();
      TString output_histo_name = "Control_" + channel_list[ichan] + "_" + total_var_list[ivar] + "_Data";
      h_sum->Write(output_histo_name, TObject::kOverwrite);
    } //end var loop
  } //end channel loop
  
  file->Close();
  
  cout<<"--- Done with generation of pseudo-data for CR"<<endl; return 0;
}



//Generate pseudo-data histograms from MC, using TRandom::Poisson to simulate statistical fluctuations
//Used to simulate template fit to pseudo-data, to avoid using real data before pre-approval
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
int theMVAtool::Generate_PseudoData_Histograms_For_Templates(TString template_name, TString channel_)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mWt" || template_name == "m3l") {cout<<FYEL("--- Producing "<<template_name<<" PseudoData Templates ---")<<endl;}
  else {cout<<BOLD(FRED("--- ERROR : invalid template_name value ! Exit !"))<<endl; return 0;}
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  
  
  TRandom3 therand(0); //Randomization
  
  TString pseudodata_input_name = "outputs/Reader_" ;
  if(channel_.Contains("All")) pseudodata_input_name += template_name + filename_suffix + ".root";
  if(channel_.Contains("eee")) pseudodata_input_name += "eee_"+template_name + filename_suffix + ".root";
  if(channel_.Contains("eeu")) pseudodata_input_name += "eeu_"+template_name + filename_suffix + ".root";
  if(channel_.Contains("uue")) pseudodata_input_name += "uue_"+template_name + filename_suffix + ".root";
  if(channel_.Contains("uuu")) pseudodata_input_name += "uuu_"+template_name + filename_suffix + ".root";
  cout << pseudodata_input_name << endl;
  TFile* file = 0;
  file = TFile::Open( pseudodata_input_name.Data(), "UPDATE");
  if(file == 0) {cout<<BOLD(FRED("--- ERROR : Reader file not found ! Exit !"))<<endl; return 0;}
  
  if(channel_.Contains("All")){
    for(int ichan=0; ichan<channel_list.size(); ichan++)
    {
      TH1F *h_sum = 0, *h_tmp = 0;
      
      for(int isample = 0; isample < sample_list.size(); isample++)
      {
        if(sample_list[isample].Contains("Data") || sample_list[isample].Contains("WW") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("Fakes")) {continue;} //Fakes are stored under special names, see below
        // if(sample_list[isample].Contains("FCNCttbarZct")) {continue; } // no signal in data
        cout << "  -- sample " << sample_list[isample] << endl;
        h_tmp = 0;
        TString histo_name = template_name + "_" + channel_list[ichan] + "__" + sample_list[isample];
        cout << "  --- histo " << histo_name << endl;
        if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<endl<<BOLD(FRED("--- Empty histogram (Reader empty ?) ! Exit !"))<<endl<<endl; return 0;}
        h_tmp = (TH1F*) file->Get(histo_name.Data());
        // cout << "h_tmp " << h_tmp->GetNbinsX() << " " << h_tmp->GetBinContent(1) << endl;
        if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
        else {h_sum->Add(h_tmp);}
      }
      
      //If find "fake template"
      TString template_fake_name = "";
      if(channel_list[ichan] == "uuu" || channel_list[ichan] == "eeu") {template_fake_name = "FakeMu";}
      else {template_fake_name = "FakeEl";}
      h_tmp = 0;
      TString histo_name = template_name + "_" + channel_list[ichan] + "__" + template_fake_name;
      if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
      else
      {
        h_tmp = (TH1F*) file->Get(histo_name.Data());
        h_sum->Add(h_tmp);
      }
      
      if(h_sum == 0) {cout<<endl<<BOLD(FRED("--- Empty histogram (Reader empty ?) ! Exit !"))<<endl<<endl; return 0;}
      int nofbins = h_sum->GetNbinsX();
      
      for(int i=0; i<nofbins; i++)
      {
        double bin_content = h_sum->GetBinContent(i+1); //cout<<"bin "<<i+1<<endl; cout<<"initial content = "<<bin_content<<endl;
        double new_bin_content = therand.Poisson(bin_content);// cout<<"new content = "<<new_bin_content<<endl;
        h_sum->SetBinContent(i+1, new_bin_content);
        h_sum->SetBinError(i+1, sqrt(new_bin_content)); //Poissonian error
      }
      
      file->cd();
      TString output_histo_name = template_name + "_" + channel_list[ichan] + "__DATA";
      h_sum->Write(output_histo_name, TObject::kOverwrite);
      
    } //end channel loop
  }
  else{
    TH1F *h_sum = 0, *h_tmp = 0;
    
    for(int isample = 0; isample < sample_list.size(); isample++)
    {
      if(sample_list[isample].Contains("Data") || sample_list[isample].Contains("WW") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("Fakes")) {continue;} //Fakes are stored under special names, see below
      // if(sample_list[isample].Contains("FCNCttbarZct")) {continue; } // no signal in data
      cout << "  -- sample " << sample_list[isample] << endl;
      h_tmp = 0;
      TString histo_name = template_name + "_" + channel_ + "__" + sample_list[isample];
      cout << "  --- histo " << histo_name << endl;
      if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<endl<<BOLD(FRED("--- Empty histogram (Reader empty ?) ! Exit !"))<<endl<<endl; return 0;}
      h_tmp = (TH1F*) file->Get(histo_name.Data());
      // cout << "h_tmp " << h_tmp->GetNbinsX() << " " << h_tmp->GetBinContent(1) << endl;
      if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
      else {h_sum->Add(h_tmp);}
    }
    
    //If find "fake template"
    TString template_fake_name = "";
    if(channel_ == "uuu" || channel_ == "eeu") {template_fake_name = "FakeMu";}
    else {template_fake_name = "FakeEl";}
    h_tmp = 0;
    TString histo_name = template_name + "_" + channel_ + "__" + template_fake_name;
    if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
    else
    {
      h_tmp = (TH1F*) file->Get(histo_name.Data());
      h_sum->Add(h_tmp);
    }
    
    if(h_sum == 0) {cout<<endl<<BOLD(FRED("--- Empty histogram (Reader empty ?) ! Exit !"))<<endl<<endl; return 0;}
    int nofbins = h_sum->GetNbinsX();
    
    for(int i=0; i<nofbins; i++)
    {
      double bin_content = h_sum->GetBinContent(i+1); //cout<<"bin "<<i+1<<endl; cout<<"initial content = "<<bin_content<<endl;
      double new_bin_content = therand.Poisson(bin_content);// cout<<"new content = "<<new_bin_content<<endl;
      h_sum->SetBinContent(i+1, new_bin_content);
      h_sum->SetBinError(i+1, sqrt(new_bin_content)); //Poissonian error
    }
    
    file->cd();
    TString output_histo_name = template_name + "_" + channel_ + "__DATA";
    h_sum->Write(output_histo_name, TObject::kOverwrite);
    
    
  }
  file->Close();
  file->Close();
  cout<<"--- Done with generation of pseudo-data"<<endl<<endl; return 0;
}





//-----------------------------------------------------------------------------------------
//        _                                       _           _
//     __| |  _ __    __ _  __      __    _ __   | |   ___   | |_   ___
//    / _` | | '__|  / _` | \ \ /\ / /   | '_ \  | |  / _ \  | __| / __|
//   | (_| | | |    | (_| |  \ V  V /    | |_) | | | | (_) | | |_  \__ \
//    \__,_| |_|     \__,_|   \_/\_/     | .__/  |_|  \___/   \__| |___/
//                                       |_|
//-----------------------------------------------------------------------------------------

//Draw control plots with histograms created with Create_Control_Histograms
//Inspired from code PlotStack.C (cf. NtupleAnalysis/src/...)
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
int theMVAtool::Draw_Control_Plots(TString channel, bool fakes_from_data, bool allchannels)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("--- Draw Contol Plots ---")<<endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  
  TString input_file_name = "outputs/Control_Histograms" + filename_suffix + ".root";
  cout << "opening " << input_file_name << endl;
  TFile* f = 0;
  f = TFile::Open( input_file_name );
  if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}
  
  TH1::SetDefaultSumw2();
  
  mkdir("plots",0777); //Create directory if inexistant
  
  TH1F *h_tmp = 0, *h_data = 0, *h_np = 0;
  THStack *stack = 0;
  
  //Variable names to be displayed on plots
  //TString title_MET = "Missing E_{T} [GeV]";
  
  vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
  thechannellist.push_back("uuu");
  thechannellist.push_back("uue");
  thechannellist.push_back("eeu");
  thechannellist.push_back("eee");
  cout << "initialised channel list " << endl;
  
  //Load Canvas definition
  Load_Canvas_Style();
  
  //Want to plot ALL activated variables (inside the 2 different variable vectors !)
  vector<TString> total_var_list;
  for(int i=0; i<v_cut_name.size(); i++)
  {
    total_var_list.push_back(v_cut_name[i].Data());
  }
  for(int i=0; i<var_list.size(); i++)
  {
    total_var_list.push_back(var_list[i].Data());
  }
  cout << "all vars initialized" << endl;
  //---------------------
  //Loop on var > chan > sample > syst
  //Retrieve histograms, stack, compare, plot
  for(int ivar=0; ivar<total_var_list.size(); ivar++)
  {
    TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
    c1->SetBottomMargin(0.3);
    
    h_data = 0; stack = 0; h_np = 0;
    vector<TH1F*> v_MC_histo; //Store separately the histos for each MC sample --> stack them after loops
    
    //TLegend* qw = new TLegend(.80,.60,.95,.90);
    TLegend* qw = new TLegend(.85,.7,0.965,.915);
    qw->SetShadowColor(0);
    qw->SetFillColor(0);
    qw->SetLineColor(0);
    
    //---------------------------
    //ERROR VECTORS INITIALIZATION
    //---------------------------
    vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)
    //Only call this 'random' histogram here in order to get the binning used for the current variable --> can initialize the error vectors !
    TString histo_name = "Control_uue_"+ total_var_list[ivar] + "_Data";
    if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<__LINE__<<histo_name<<BOLD(FRED(" : not found ! Exit !"))<<endl; return 0;}
    h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
    int nofbins = h_tmp->GetNbinsX();
    cout << "nofbins " << nofbins << endl;
    for(int ibin=0; ibin<nofbins; ibin++)
    {
      v_eyl.push_back(0); v_eyh.push_back(0);
      v_exl.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2); v_exh.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2);
      v_x.push_back( (h_tmp->GetXaxis()->GetBinLowEdge(nofbins+1) - h_tmp->GetXaxis()->GetBinLowEdge(1) ) * ((ibin+1 - 0.5)/nofbins) + h_tmp->GetXaxis()->GetBinLowEdge(1));
      v_y.push_back(0);
    }
    
    cout << "all error vectors initialised " << endl;
    
    //---------------------------
    //RETRIEVE & SUM HISTOGRAMS, SUM ERRORS QUADRATICALLY
    //---------------------------
    
    int niter_chan = 0; //is needed to know if h_tmp must be cloned or added
    for(int ichan=0; ichan<thechannellist.size(); ichan++)
    {
      cout << "CHANNELS " << thechannellist[ichan] << endl;
      if(!allchannels && channel != thechannellist[ichan]) {
        cout << "all channels are on continuing.. " << endl;
        continue;
      } //If plot single channel
      
      for(int isample = 0; isample < sample_list.size(); isample++)
      {
        
        if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
        else if(fakes_from_data && (sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only
        
        bool isData = sample_list[isample] == "Data";
        bool isNP = sample_list[isample] == "NP_overlay_ST_FCNC_zut";
        cout<<thechannellist[ichan]<<" / "<<sample_list[isample]<<" / "<<endl;
        
        TH1F* histo_nominal = 0; //Nominal histogram <-> syst == "" //Create it here because it's also used in the (syst != "") loop
        
        h_tmp = 0; //Temporary storage of histogram
        
        histo_name = "Control_" + thechannellist[ichan] + "_"+ total_var_list[ivar] + "_" + sample_list[isample];
        if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
        h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
        cout<<"h_tmp = "<<h_tmp<<endl;
        cout<<__LINE__<<endl;
        
        
        if(isData) //DATA
        {
          cout << "data" << endl;
          if(niter_chan == 0) {h_data = (TH1F*) h_tmp->Clone();}
          else {h_data->Add(h_tmp);}
          continue;
        }
        else if(isNP)
        {
          cout << "NP" << endl;
          if(niter_chan == 0) {h_np = (TH1F*) h_tmp->Clone();}
          else {h_np->Add(h_tmp);}
          continue;
          
        }
        else //MC
        {
          //Normally the data sample is included in first position of the list, so for MC sample vectors -> need to use "isample - 1"
          //But let's make sure that it won't be a problem
          cout << "MC" << endl;
          if(isample-2 < 0)  {cout<<__LINE__<<BOLD(FRED(" : Try to access wrong address ! Exit !"))<<endl; return 0;}
          
          //Use color vector filled in main()
          cout << "styles" << endl;
          h_tmp->SetFillStyle(1001);
          h_tmp->SetFillColor(colorVector[isample-1]);
          h_tmp->SetLineColor(colorVector[isample-1]);
          
          cout << "chan i "  << niter_chan << endl;
          if(niter_chan == 0) {v_MC_histo.push_back(h_tmp);}
          else {
            cout << " addinghisto on place " << isample -2 << endl;
            v_MC_histo[isample-2]->Add(h_tmp);
            cout << " added histo on place " << isample -2<< endl;
          }
          
          
          histo_nominal = (TH1F*) h_tmp->Clone();
          cout << "looping over bins" << endl;
          for(int ibin=0; ibin<nofbins; ibin++) //Start at bin 1
          {
            //Lumi = 2.5% of bin content //WARNING : REPLACE by 6.2% FOR 2016 DATA
            v_eyl[ibin]+= pow(histo_nominal->GetBinContent(ibin+1)*0.025, 2);
            v_eyh[ibin]+= pow(histo_nominal->GetBinContent(ibin+1)*0.025, 2);
            //MC Stat error
            v_eyl[ibin]+= pow(histo_nominal->GetBinError(ibin+1), 2);
            v_eyh[ibin]+= pow(histo_nominal->GetBinError(ibin+1), 2);
            
            v_y[ibin]+= histo_nominal->GetBinContent(ibin+1); //This vector is used to know where to draw the error zone on plot (= on top of stack)
            
            if(ibin > 0) {continue;} //cout only first bin
            // cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
          }
        }
        cout << "data MC part done" <<endl;
        
        for(int isyst=0; isyst<syst_list.size(); isyst++)
        {
          if(isData || sample_list[isample].Contains("Fakes") ) {break;} //No syst
          if(syst_list[isyst] == "") {continue;} //Already done
          //if(sample_list[isample].Contains("ttZ") && syst_list[isyst].Contains("Q2") ) {continue;} //incompatible w/ ttZMad
          
          TH1F* histo_syst = 0; //Store the "systematic histograms"
          
          histo_name = "Control_" + thechannellist[ichan] + "_"+ total_var_list[ivar] + "_" + sample_list[isample] + "_" + syst_list[isyst];
          if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
          histo_syst = (TH1F*) f->Get(histo_name.Data())->Clone();
          
          //Add up here the different errors (quadratically), for each bin separately
          for(int ibin=0; ibin<nofbins; ibin++)
          {
            double tmp = 0;
            
            //For each systematic, compute (shifted-nominal), check the sign, and add quadratically to the corresponding bin error
            //--------------------------
            tmp = histo_syst->GetBinContent(ibin+1) - histo_nominal->GetBinContent(ibin+1);
            if(tmp>0) {v_eyh[ibin]+= pow(tmp,2);}
            else if(tmp<0) {v_eyl[ibin]+= pow(tmp,2);}
            //--------------------------
            
            if(ibin > 0) {continue;} //cout only first bin
            // cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
            
          }
          cout << "syst part" << endl;
        } //end syst loop
      } //end sample loop
      
      niter_chan++;
    } //end channel loop
    cout << "channels done " << endl;
    //---------------------------
    //CREATE STACK (MC)
    //---------------------------
    TH1F* histo_total_MC = 0; //Sum of all MC samples
    cout << "Stacking " << endl;
    //Stack all the MC nominal histograms (contained in v_MC_histo)
    for(int i=0; i<v_MC_histo.size(); i++)
    {
      if(stack == 0) {stack = new THStack; stack->Add(v_MC_histo[i]);}
      else {stack->Add(v_MC_histo[i]);}
      
      if(histo_total_MC == 0) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
      else {histo_total_MC->Add(h_tmp);}
      
      //NB : use (i+1) because sample_list also contains the data sample in first position
      //ADD LEGEND ENTRIES
     // if(sample_list[i+2] != "WW" && sample_list[i+1] != "ZZ") {qw->AddEntry(v_MC_histo[i], sample_list[i+1].Data() , "f");}
      //else if(sample_list[i] == "ttZ") {qw->AddEntry(v_MC_histo[i+1], "ttV" , "f");} //If activate both ttZ and ttW
     // else if(sample_list[i+1] == "ZZ") {qw->AddEntry(v_MC_histo[i], "VV" , "f");}
      qw->AddEntry(v_MC_histo[i], sample_list[i+2].Data() , "f");
    }
    
    if(h_data != 0) {qw->AddEntry(h_data, "Data" , "ep");}
    else {cout<<__LINE__<<BOLD(FRED(" : h_data is null"))<<endl;}
    if(h_np != 0) {qw->AddEntry(h_np, "NP_overlay_ST_FCNC_zut" , "ep");}
    else {cout<<__LINE__<<BOLD(FRED(" : h_np is null"))<<endl;}
    
    if(!stack) {cout<<__LINE__<<BOLD(FRED(" : stack is null"))<<endl;}
    
    //---------------------------
    //DRAW HISTOS
    //---------------------------
    
    c1->cd();
    cout << "drawing " << endl;
    //Set Yaxis maximum & min
    if(h_data != 0 && stack != 0 && h_np != 0 )
    {
      double max = 0.;
      if(h_data->GetMaximum() > h_np->GetMaximum()) {max = h_data->GetMaximum();}
      else {max = h_np ->GetMaximum();}
      if(max > stack->GetMaximum() ) {stack->SetMaximum(max+0.3*max);}
      else { stack->SetMaximum(stack->GetMaximum()+0.3*stack->GetMaximum()); }
    }
    stack->SetMinimum(0);
    
    //Draw stack
    if(stack != 0) {stack->Draw("HIST"); stack->GetXaxis()->SetLabelSize(0.0);}
    
    //Draw data
    if(h_data != 0)
    {
      h_data->SetMarkerStyle(20);
      h_data->SetMarkerSize(1.2);
      h_data->SetLineColor(1);
      h_data->Draw("e0psame");
    }
                                if(h_np != 0)
                                {
                                  // h_np->SetMarkerStyle(20);
                                  //  h_np->SetMarkerSize(1.2);
                                  h_np->SetLineColor(4);
                                  h_np->SetLineWidth(2);
                                  h_np->SetLineStyle(1);
                                  h_np->Draw("e0psame");
                                }
                                
    
    //---------------------------
    //DRAW SYST ERRORS ON PLOT
    //---------------------------
    
    //Need to take sqrt of total errors
    for(int ibin=0; ibin<nofbins; ibin++)
    {
      v_eyh[ibin] = pow(v_eyh[ibin], 0.5);
      v_eyl[ibin] = pow(v_eyl[ibin], 0.5);
      
      if(ibin > 0) {continue;} //cout only first bin
      // cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
    }
    
    //Use pointers to vectors : need to give the adress of first element (all other elements can then be accessed iteratively)
    double* eyl = &v_eyl[0];
    double* eyh = &v_eyh[0];
    double* exl = &v_exl[0];
    double* exh = &v_exh[0];
    double* x = &v_x[0];
    double* y = &v_y[0];
    
    //Create TGraphAsymmErrors with the error vectors / (x,y) coordinates --> Can superimpose it on plot
    TGraphAsymmErrors* gr = 0;
    gr = new TGraphAsymmErrors(nofbins,x,y,exl,exh,eyl,eyh);
    gr->SetFillStyle(3005);
    gr->SetFillColor(1);
    gr->Draw("e2 same"); //Superimposes the systematics uncertainties on stack
    //cout << __LINE__ << endl;
    
    //-------------------
    //CAPTIONS
    //-------------------
    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextSize(0.04);
    latex->SetTextAlign(31);
    latex->DrawLatex(0.45, 0.95, "CMS Preliminary");
    
    TLatex* latex2 = new TLatex();
    latex2->SetNDC();
    latex2->SetTextSize(0.04);
    latex2->SetTextAlign(31);
    //------------------
    float lumi = 2.2 * luminosity_rescale;
    TString lumi_ts = Convert_Number_To_TString(lumi);
    lumi_ts+= " fb^{-1} at #sqrt{s} = 13 TeV";
    latex2->DrawLatex(0.87, 0.95, lumi_ts.Data());
    //------------------
    
    TString info_data;
    if (channel=="eee")    info_data = "eee channel";
    else if (channel=="eeu")  info_data = "ee#mu channel";
    else if (channel=="uue")  info_data = "#mu#mu e channel";
    else if (channel=="uuu") info_data = "#mu#mu #mu channel";
    else if(allchannels) info_data = "eee, #mu#mu#mu, #mu#mue, ee#mu channels";
    
    TLatex* text2 = new TLatex(0.45,0.98, info_data);
    text2->SetNDC();
    text2->SetTextAlign(13);
    text2->SetX(0.18);
    text2->SetY(0.92);
    text2->SetTextFont(42);
    text2->SetTextSize(0.0610687);
    //text2->SetTextSizePixels(24);// dflt=28
    text2->Draw();
    qw->Draw();
    
    //--------------------------
    //DRAW DATA/MC RATIO
    //--------------------------
    //Create Data/MC ratio plot (bottom of canvas)
    if(h_data != 0 && v_MC_histo.size() != 0) //Need both data and MC
    {
      TPad *canvas_2 = new TPad("canvas_2", "canvas_2", 0.0, 0.0, 1.0, 1.0);
      canvas_2->SetTopMargin(0.7);
      canvas_2->SetFillColor(0);
      canvas_2->SetFillStyle(0);
      canvas_2->SetGridy(1);
      canvas_2->Draw();
      canvas_2->cd(0);
      
      TH1F * histo_ratio_data = (TH1F*) h_data->Clone();
      histo_ratio_data->Divide(histo_total_MC); //Ratio
      
      //if(total_var_list[ivar] == "METpt")   histo_ratio_data->GetXaxis()->SetTitle(title_MET.Data());
      histo_ratio_data->GetXaxis()->SetTitle(total_var_list[ivar].Data());
      
      histo_ratio_data->SetMinimum(0.0);
      histo_ratio_data->SetMaximum(2.0);
      histo_ratio_data->GetXaxis()->SetTitleOffset(1.2);
      histo_ratio_data->GetXaxis()->SetLabelSize(0.04);
      histo_ratio_data->GetYaxis()->SetLabelSize(0.03);
      histo_ratio_data->GetYaxis()->SetNdivisions(6);
      histo_ratio_data->GetYaxis()->SetTitleSize(0.03);
      histo_ratio_data->Draw("E1X0"); //Draw ratio points
      
      //Copy previous TGraphAsymmErrors
      TGraphAsymmErrors *thegraph_tmp = (TGraphAsymmErrors*) gr->Clone();
      double *theErrorX_h = thegraph_tmp->GetEXhigh();
      double *theErrorY_h = thegraph_tmp->GetEYhigh();
      double *theErrorX_l = thegraph_tmp->GetEXlow();
      double *theErrorY_l = thegraph_tmp->GetEYlow();
      double *theY        = thegraph_tmp->GetY() ;
      double *theX        = thegraph_tmp->GetX() ;
      
      //Divide error --> ratio
      for(int i=0; i<thegraph_tmp->GetN(); i++)
      {
        theErrorY_l[i] = theErrorY_l[i]/theY[i];
        theErrorY_h[i] = theErrorY_h[i]/theY[i];
        theY[i]=1; //To center the filled area around "1"
      }
      
      //--> Create new TGraphAsymmErrors
      TGraphAsymmErrors *thegraph_ratio = new TGraphAsymmErrors(thegraph_tmp->GetN(), theX , theY ,  theErrorX_l, theErrorX_h, theErrorY_l, theErrorY_h);
      thegraph_ratio->SetFillStyle(3005);
      thegraph_ratio->SetFillColor(1);
      thegraph_ratio->Draw("e2 same"); //Syst. error for Data/MC ; drawn on canvas2 (Data/MC ratio)
    }
    
    //Yaxis title
    if(stack!= 0) {stack->GetYaxis()->SetTitleSize(0.04); stack->GetYaxis()->SetTitle("Events");}
    
    //-------------------
    //OUTPUT
    //-------------------
    
    //Image name (.png)
    TString outputname = "plots/"+total_var_list[ivar]+"_"+channel+".png";
    if(channel == "" || allchannels) {outputname = "plots/"+total_var_list[ivar]+"_all.png";}
    
    //cout << __LINE__ << endl;
    if(c1!= 0) {c1->SaveAs(outputname.Data() );}
    //cout << __LINE__ << endl;
    
    delete c1; //Must free dinamically-allocated memory
  } //end var loop
  return 0;
}


int theMVAtool::Draw_ControlttZ_Plots(TString channel, bool fakes_from_data, bool allchannels)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("--- Draw Contol Plots ---")<<endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  
  TString input_file_name = "outputs/Control_Histograms" + filename_suffix + ".root";
  cout << "opening " << input_file_name << endl;
  TFile* f = 0;
  f = TFile::Open( input_file_name );
  if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}
  
  TH1::SetDefaultSumw2();
  
  mkdir("plots",0777); //Create directory if inexistant
  
  TH1F *h_tmp = 0, *h_data = 0, *h_np = 0, *h_ttZMad = 0, *h_ttZ=0;
  THStack *stack = 0;
  
  //Variable names to be displayed on plots
  //TString title_MET = "Missing E_{T} [GeV]";
  
  vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
  thechannellist.push_back("uuu");
  thechannellist.push_back("uue");
  thechannellist.push_back("eeu");
  thechannellist.push_back("eee");
  cout << "initialised channel list " << endl;
  
  //Load Canvas definition
  Load_Canvas_Style();
  
  //Want to plot ALL activated variables (inside the 2 different variable vectors !)
  vector<TString> total_var_list;
  for(int i=0; i<v_cut_name.size(); i++)
  {
    total_var_list.push_back(v_cut_name[i].Data());
  }
  for(int i=0; i<var_list.size(); i++)
  {
    total_var_list.push_back(var_list[i].Data());
  }
  cout << "all vars initialized" << endl;
  //---------------------
  //Loop on var > chan > sample > syst
  //Retrieve histograms, stack, compare, plot
  for(int ivar=0; ivar<total_var_list.size(); ivar++)
  {
    TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
    c1->SetBottomMargin(0.3);
    
    h_data = 0;  h_np = 0; h_ttZ = 0; h_ttZMad = 0;
   
    
    //TLegend* qw = new TLegend(.80,.60,.95,.90);
    TLegend* qw = new TLegend(.85,.7,0.965,.915);
    qw->SetShadowColor(0);
    qw->SetFillColor(0);
    qw->SetLineColor(0);
        //---------------------------
    
    //RETRIEVE & SUM HISTOGRAMS, SUM ERRORS QUADRATICALLY
    //---------------------------
    
    int niter_chan = 0; //is needed to know if h_tmp must be cloned or added
    for(int ichan=0; ichan<thechannellist.size(); ichan++)
    {
      cout << "CHANNELS " << thechannellist[ichan] << endl;
      if(!allchannels && channel != thechannellist[ichan]) {
        cout << "all channels are on continuing.. " << endl;
        continue;
      } //If plot single channel
      
      for(int isample = 0; isample < sample_list.size(); isample++)
      {
        
        if(!sample_list[isample].Contains("ttZ") && !sample_list[isample].Contains("ttZMad")) continue;
        
        
        cout<<thechannellist[ichan]<<" / "<<sample_list[isample]<<" / "<<endl;
        
        
        h_tmp = 0; //Temporary storage of histogram
        
         TString histo_name = "Control_" + thechannellist[ichan] + "_"+ total_var_list[ivar] + "_" + sample_list[isample];
        if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
        h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
        cout<<"h_tmp = "<<h_tmp<<endl;
        cout<<__LINE__<<endl;
        
        
        
        if(sample_list[isample].Contains("ttZ")){
          if(niter_chan == 0) {h_ttZ = (TH1F*) h_tmp->Clone();}
          else {h_ttZ->Add(h_tmp);}
        }
        if(sample_list[isample].Contains("ttZMad")){
          if(niter_chan == 0) {h_ttZMad = (TH1F*) h_tmp->Clone();}
          else {h_ttZMad->Add(h_tmp);}
        }
        
        
        cout << "data MC part done" <<endl;
        
      } //end sample loop
      
      niter_chan++;
    } //end channel loop
    cout << "channels done " << endl;
    //---------------------------
    //CREATE STACK (MC)
    //---------------------------
    
    
    
    c1->cd();
    cout << "drawing " << endl;
    //Set Yaxis maximum & min
    if(h_ttZ != 0 && h_ttZMad != 0 )
    {
      
      if(h_ttZ->GetMaximum() > h_ttZMad->GetMaximum() ) {h_ttZ->SetMaximum(h_ttZ->GetMaximum()+0.3*h_ttZ->GetMaximum());}
      else { h_ttZ->SetMaximum(h_ttZMad->GetMaximum()+0.3*h_ttZMad->GetMaximum()); }
    }
    h_ttZ->SetMinimum(0);
    
    //Draw stack
   // if(stack != 0) {stack->Draw("HIST"); stack->GetXaxis()->SetLabelSize(0.0);}
    
    if(h_ttZ != 0)
    {
      h_ttZ->SetMarkerStyle(20);
      h_ttZ->SetMarkerSize(1.2);
      h_ttZ->SetLineColor(kGreen);
      h_ttZ->DrawNormalized("h");
    }
    else{ cout << " ttZ not there !" << endl; }
    if(h_ttZMad != 0)
    {
      h_ttZMad->SetMarkerStyle(20);
      h_ttZMad->SetMarkerSize(1.2);
      h_ttZMad->SetLineColor(kRed);
      h_ttZMad->DrawNormalized("hsame");
    }
     else{ cout << " ttZMad not there !" << endl; }
    
    if(h_ttZ != 0) {qw->AddEntry(h_ttZ, "aMC ttZ" , "ep");}
    else {cout<<__LINE__<<BOLD(FRED(" : h_ttZ is null"))<<endl;}
    if(h_ttZMad != 0) {qw->AddEntry(h_ttZMad, "Madgraph ttZ" , "ep");}
    else {cout<<__LINE__<<BOLD(FRED(" : h_ttZmad is null"))<<endl;}
    
    
    //-------------------
    //CAPTIONS
    //-------------------
    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextSize(0.04);
    latex->SetTextAlign(31);
    latex->DrawLatex(0.45, 0.95, "CMS Preliminary");
    
    TLatex* latex2 = new TLatex();
    latex2->SetNDC();
    latex2->SetTextSize(0.04);
    latex2->SetTextAlign(31);
    //------------------
    float lumi = 2.2 * luminosity_rescale;
    TString lumi_ts = Convert_Number_To_TString(lumi);
    lumi_ts+= " fb^{-1} at #sqrt{s} = 13 TeV";
    latex2->DrawLatex(0.87, 0.95, lumi_ts.Data());
    //------------------
    
    TString info_data;
    if (channel=="eee")    info_data = "eee channel";
    else if (channel=="eeu")  info_data = "ee#mu channel";
    else if (channel=="uue")  info_data = "#mu#mu e channel";
    else if (channel=="uuu") info_data = "#mu#mu #mu channel";
    else if(allchannels) info_data = "eee, #mu#mu#mu, #mu#mue, ee#mu channels";
    
    TLatex* text2 = new TLatex(0.45,0.98, info_data);
    text2->SetNDC();
    text2->SetTextAlign(13);
    text2->SetX(0.18);
    text2->SetY(0.92);
    text2->SetTextFont(42);
    text2->SetTextSize(0.0610687);
    //text2->SetTextSizePixels(24);// dflt=28
    text2->Draw();
    qw->Draw();
    
    
    //Yaxis title
    if(h_ttZ!= 0) {h_ttZ->GetYaxis()->SetTitleSize(0.04); h_ttZ->GetYaxis()->SetTitle("Events");}
    
    //-------------------
    //OUTPUT
    //-------------------
    
    //Image name (.png)
    TString outputname = "plots/ttZ"+total_var_list[ivar]+"_"+channel+"norm.png";
    if(channel == "" || allchannels) {outputname = "plots/ttZ"+total_var_list[ivar]+"_allnorm.png";}
    
    //cout << __LINE__ << endl;
    if(c1!= 0) {c1->SaveAs(outputname.Data() );}
    //cout << __LINE__ << endl;
    
    delete c1; //Must free dinamically-allocated memory
  } //end var loop
  return 0;
}


//Draw control plots with histograms created with Create_Control_Histograms
//Inspired from code PlotStack.C (cf. NtupleAnalysis/src/...)
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
int theMVAtool::Draw_Control_Templates(TString channel, bool fakes_from_data, bool allchannels, TString template_name)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("--- Draw Contol Plots ---")<<endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  
  TString input_file_name = "outputs/Reader_" + template_name + filename_suffix + ".root";
  cout << "opening " << input_file_name << endl;
  TFile* f = 0;
  f = TFile::Open( input_file_name );
  if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}
  
  TH1::SetDefaultSumw2();
  
  mkdir("plots",0777); //Create directory if inexistant
  
  TH1F *h_tmp = 0, *h_data = 0, *h_np=0;
  THStack *stack = 0;
  
  //Variable names to be displayed on plots
  //TString title_MET = "Missing E_{T} [GeV]";
  
  vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
  thechannellist.push_back("uuu");
  thechannellist.push_back("uue");
  thechannellist.push_back("eeu");
  thechannellist.push_back("eee");
  cout << "initialised channel list " << thechannellist.size() << " channels"<< endl;
  
  //Load Canvas definition
  Load_Canvas_Style();
  
  //Want to plot ALL activated variables (inside the 2 different variable vectors !)
  vector<TString> total_var_list;
  total_var_list.clear();
  total_var_list.push_back("BDT");
  //---------------------
  //Loop on var > chan > sample > syst
  //Retrieve histograms, stack, compare, plot
  for(int ivar=0; ivar<total_var_list.size(); ivar++)
  {
    cout << "VARIABLE " << total_var_list[ivar] <<endl;
    TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
    c1->SetBottomMargin(0.3);
    
    h_data = 0; stack = 0, h_np = 0;
    vector<TH1F*> v_MC_histo; //Store separately the histos for each MC sample --> stack them after loops
    vector<TString> v_MC_title;
    //TLegend* qw = new TLegend(.80,.60,.95,.90);
    TLegend* qw = new TLegend(.85,.7,0.965,.915);
    qw->SetShadowColor(0);
    qw->SetFillColor(0);
    qw->SetLineColor(0);
    
    //---------------------------
    //ERROR VECTORS INITIALIZATION
    //---------------------------
    vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)
    //Only call this 'random' histogram here in order to get the binning used for the current variable --> can initialize the error vectors !
    TString histo_name = "BDT_uuu__NP_overlay_ST_FCNC_zut";
    //histo_name += total_var_list[ivar] + "_uuu__FCNCttbarZct";
    if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<__LINE__<<histo_name<<BOLD(FRED(" : not found ! Exit !"))<<endl; return 0;}
    h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
    int nofbins = h_tmp->GetNbinsX();
    cout << "nofbins " << nofbins << endl;
    for(int ibin=0; ibin<nofbins; ibin++)
    {
      v_eyl.push_back(0); v_eyh.push_back(0);
      v_exl.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2); v_exh.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2);
      v_x.push_back( (h_tmp->GetXaxis()->GetBinLowEdge(nofbins+1) - h_tmp->GetXaxis()->GetBinLowEdge(1) ) * ((ibin+1 - 0.5)/nofbins) + h_tmp->GetXaxis()->GetBinLowEdge(1));
      v_y.push_back(0);
    }
    
    cout << "all error vectors initialised " << endl;
    
    //---------------------------
    //RETRIEVE & SUM HISTOGRAMS, SUM ERRORS QUADRATICALLY
    //---------------------------
    
    int niter_chan = 0; //is needed to know if h_tmp must be cloned or added
    for(int ichan=0; ichan<thechannellist.size(); ichan++)
    {
      cout << "CHANNEL " << thechannellist[ichan] << endl;
      if(!allchannels && channel != thechannellist[ichan]) {
        cout << "all channels are on continuing.. or trying to acces a wrong channel" << endl;
        continue;
      } //If plot single channel
      
      for(int isample = 0; isample < sample_list.size(); isample++)
      {
        cout << "SAMPLE: " << sample_list[isample] << endl;
        
        if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
        else if(fakes_from_data && (sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("WW") ) ) {continue;} //Fakes from data only
        
        bool isData = sample_list[isample].Contains("Data") ;
        if(isData) { sample_list[isample] ="DATA";}
        if(sample_list[isample].Contains("Fakes")){
          cout << "setting " <<  sample_list[isample];
          if(channel_list[ichan].Contains("uuu") || channel_list[ichan].Contains("eeu")) sample_list[isample] = "FakeMu";
          else sample_list[isample] = "FakeEl";
          
          cout << " to " <<sample_list[isample] << endl;

        }
        bool isNP = sample_list[isample].Contains("NP_overlay_ST_FCNC_zut");
        cout<<thechannellist[ichan]<<" / "<<sample_list[isample]<<" / "<<endl;
        
        TH1F* histo_nominal = 0; //Nominal histogram <-> syst == "" //Create it here because it's also used in the (syst != "") loop
        
        h_tmp = 0; //Temporary storage of histogram
        
        histo_name = total_var_list[ivar] + "_" + thechannellist[ichan] +"__"+ sample_list[isample];
        if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
        h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
        cout<<"h_tmp = "<<h_tmp<<endl;
        cout<<__LINE__<<endl;
        
        
        if(isData) //DATA
        {
          cout << "data" << endl;
          if(niter_chan == 0) {h_data = (TH1F*) h_tmp->Clone();}
          else {h_data->Add(h_tmp);}
          continue;
        }
        else if(isNP){
          cout << "NP" << endl;
          if(niter_chan == 0) {h_np = (TH1F*) h_tmp->Clone();}
          else {h_np->Add(h_tmp);}
          h_np->SetLineColor(4);
          continue;
        }
        else //MC
        {
          //Normally the data sample is included in first position of the list, so for MC sample vectors -> need to use "isample - 1"
          //But let's make sure that it won't be a problem
          cout << "MC" << endl;
          //if(isample-2 < 0)  {cout<<__LINE__<<BOLD(FRED(" : Try to access wrong address ! Exit !"))<<endl; return 0;}
          
          //Use color vector filled in main()
          cout << "styles" << endl;
          h_tmp->SetFillStyle(1001);
          h_tmp->SetFillColor(colorVector[isample-1]);
          h_tmp->SetLineColor(colorVector[isample-1]);
          
          cout << "chan i "  << niter_chan << endl;
          if(niter_chan == 0) {v_MC_histo.push_back(h_tmp); v_MC_title.push_back(sample_list[isample]);}
          else {
            cout << " addinghisto on place " << isample -1 << endl;
            v_MC_histo[isample-1]->Add(h_tmp);
          //  v_MC_title.push_back[sample_list[isample]];
            cout << " added histo on place " << isample -1 << endl;
          }
          
          
          histo_nominal = (TH1F*) h_tmp->Clone();
          cout << "looping over bins" << endl;
          for(int ibin=0; ibin<nofbins; ibin++) //Start at bin 1
          {
            //Lumi = 2.5% of bin content //WARNING : REPLACE by 6.2% FOR 2016 DATA
            v_eyl[ibin]+= pow(histo_nominal->GetBinContent(ibin+1)*0.025, 2);
            v_eyh[ibin]+= pow(histo_nominal->GetBinContent(ibin+1)*0.025, 2);
            //MC Stat error
            v_eyl[ibin]+= pow(histo_nominal->GetBinError(ibin+1), 2);
            v_eyh[ibin]+= pow(histo_nominal->GetBinError(ibin+1), 2);
            
            v_y[ibin]+= histo_nominal->GetBinContent(ibin+1); //This vector is used to know where to draw the error zone on plot (= on top of stack)
            
            if(ibin > 0) {continue;} //cout only first bin
            // cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
          }
        }
        cout << "data MC part done" <<endl;
        
       for(int isyst=0; isyst<syst_list.size(); isyst++)
        {
          if(isData || sample_list[isample].Contains("Fakes") ) {break;} //No syst
          if(syst_list[isyst] == "") {continue;} //Already done
          //if(sample_list[isample].Contains("ttZ") && syst_list[isyst].Contains("Q2") ) {continue;} //incompatible w/ ttZMad
          
          TH1F* histo_syst = 0; //Store the "systematic histograms"
          
          histo_name = total_var_list[ivar] + "_" + thechannellist[ichan] +"__"+ sample_list[isample]; "_" + syst_list[isyst];
          if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
          histo_syst = (TH1F*) f->Get(histo_name.Data())->Clone();
          
          //Add up here the different errors (quadratically), for each bin separately
          for(int ibin=0; ibin<nofbins; ibin++)
          {
            double tmp = 0;
            
            //For each systematic, compute (shifted-nominal), check the sign, and add quadratically to the corresponding bin error
            //--------------------------
            tmp = histo_syst->GetBinContent(ibin+1) - histo_nominal->GetBinContent(ibin+1);
            if(tmp>0) {v_eyh[ibin]+= pow(tmp,2);}
            else if(tmp<0) {v_eyl[ibin]+= pow(tmp,2);}
            //--------------------------
            
            if(ibin > 0) {continue;} //cout only first bin
            // cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
            
          }
          cout << "syst part" << endl;
        } //end syst loop
      } //end sample loop
      
      niter_chan++;
    } //end channel loop
    cout << "channels done " << endl;
    //---------------------------
    //CREATE STACK (MC)
    //---------------------------
    TH1F* histo_total_MC = 0; //Sum of all MC samples
    cout << "Stacking " << endl;
    //Stack all the MC nominal histograms (contained in v_MC_histo)
    for(int i=0; i<v_MC_histo.size(); i++)
    {
      if(stack == 0) {stack = new THStack; stack->Add(v_MC_histo[i]);}
      else {stack->Add(v_MC_histo[i]);}
      
      if(histo_total_MC == 0) {histo_total_MC = (TH1F*) v_MC_histo[i]->Clone();}
      else {histo_total_MC->Add(h_tmp);}
      
      //NB : use (i+2) because sample_list also contains the data sample, signal in first position
      //ADD LEGEND ENTRIES
      qw->AddEntry(v_MC_histo[i], sample_list[i+2].Data() , "f");
     // if(sample_list[i+1] != "WW" && sample_list[i+1] != "ZZ") {qw->AddEntry(v_MC_histo[i], sample_list[i+1].Data() , "f");}
      //else if(sample_list[i] == "ttZ") {qw->AddEntry(v_MC_histo[i+1], "ttV" , "f");} //If activate both ttZ and ttW
     // else if(sample_list[i+1] == "ZZ") {qw->AddEntry(v_MC_histo[i], "VV" , "f");}
    }
    
    if(h_data != 0) {qw->AddEntry(h_data, "Data" , "ep");}
    else {cout<<__LINE__<<BOLD(FRED(" : h_data is null"))<<endl;}
    
    if(h_np != 0) {qw->AddEntry(h_np, "FCNC ST Zut" , "ep");}
    else {cout<<__LINE__<<BOLD(FRED(" : h_np is null"))<<endl;}
    
    
    if(!stack) {cout<<__LINE__<<BOLD(FRED(" : stack is null"))<<endl;}
    
    //---------------------------
    //DRAW HISTOS
    //---------------------------
    
    c1->cd();
    cout << "drawing " << endl;
    //Set Yaxis maximum & min
    if(h_data != 0 && stack != 0 && h_np !=0)
    {
      double max = 0.;
      if(h_data->GetMaximum() > h_np->GetMaximum()) {max = h_data->GetMaximum();}
      else {max = h_np ->GetMaximum();}
      if(max > stack->GetMaximum() ) {stack->SetMaximum(max+0.3*max);}
      else stack->SetMaximum(stack->GetMaximum()+0.3*stack->GetMaximum());
    }
    stack->SetMinimum(0);
    
    //Draw stack
    if(stack != 0) {stack->Draw("HIST"); stack->GetXaxis()->SetLabelSize(0.0);}
    
    //Draw data
    if(h_data != 0)
    {
      h_data->SetMarkerStyle(20);
      h_data->SetMarkerSize(1.2);
      h_data->SetLineColor(1);
      h_data->Draw("e0psame");
    }
    
    if(h_np != 0)
    {
     // h_np->SetMarkerStyle(20);
    //  h_np->SetMarkerSize(1.2);
      h_np->SetLineColor(4);
      h_np->SetLineWidth(2);
      h_np->SetLineStyle(1);
      h_np->Draw("e0psame");
    }
    
    //---------------------------
    //DRAW SYST ERRORS ON PLOT
    //---------------------------
    
    //Need to take sqrt of total errors
    for(int ibin=0; ibin<nofbins; ibin++)
    {
      v_eyh[ibin] = pow(v_eyh[ibin], 0.5);
      v_eyl[ibin] = pow(v_eyl[ibin], 0.5);
      
      if(ibin > 0) {continue;} //cout only first bin
      // cout<<"x = "<<v_x[ibin]<<endl;    cout<<", y = "<<v_y[ibin]<<endl;    cout<<", eyl = "<<v_eyl[ibin]<<endl;    cout<<", eyh = "<<v_eyh[ibin]<<endl; cout<<", exl = "<<v_exl[ibin]<<endl;    cout<<", exh = "<<v_exh[ibin]<<endl;
    }
    
    //Use pointers to vectors : need to give the adress of first element (all other elements can then be accessed iteratively)
    double* eyl = &v_eyl[0];
    double* eyh = &v_eyh[0];
    double* exl = &v_exl[0];
    double* exh = &v_exh[0];
    double* x = &v_x[0];
    double* y = &v_y[0];
    
    //Create TGraphAsymmErrors with the error vectors / (x,y) coordinates --> Can superimpose it on plot
    TGraphAsymmErrors* gr = 0;
    gr = new TGraphAsymmErrors(nofbins,x,y,exl,exh,eyl,eyh);
    gr->SetFillStyle(3005);
    gr->SetFillColor(1);
    gr->Draw("e2 same"); //Superimposes the systematics uncertainties on stack
    //cout << __LINE__ << endl;
    
    //-------------------
    //CAPTIONS
    //-------------------
    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextSize(0.04);
    latex->SetTextAlign(31);
    latex->DrawLatex(0.45, 0.95, "CMS Preliminary");
    
    TLatex* latex2 = new TLatex();
    latex2->SetNDC();
    latex2->SetTextSize(0.04);
    latex2->SetTextAlign(31);
    //------------------
    float lumi = 2.2 * luminosity_rescale;
    TString lumi_ts = Convert_Number_To_TString(lumi);
    lumi_ts+= " fb^{-1} at #sqrt{s} = 13 TeV";
    latex2->DrawLatex(0.87, 0.95, lumi_ts.Data());
    //------------------
    
    TString info_data;
    if (channel=="eee")    info_data = "eee channel";
    else if (channel=="eeu")  info_data = "ee#mu channel";
    else if (channel=="uue")  info_data = "#mu#mu e channel";
    else if (channel=="uuu") info_data = "#mu#mu #mu channel";
    else if(allchannels) info_data = "eee, #mu#mu#mu, #mu#mue, ee#mu channels";
    
    TLatex* text2 = new TLatex(0.45,0.98, info_data);
    text2->SetNDC();
    text2->SetTextAlign(13);
    text2->SetX(0.18);
    text2->SetY(0.92);
    text2->SetTextFont(42);
    text2->SetTextSize(0.0610687);
    //text2->SetTextSizePixels(24);// dflt=28
    text2->Draw();
    qw->Draw();
    
    //--------------------------
    //DRAW DATA/MC RATIO
    //--------------------------
    //Create Data/MC ratio plot (bottom of canvas)
    if(h_data != 0 && v_MC_histo.size() != 0) //Need both data and MC
    {
      TPad *canvas_2 = new TPad("canvas_2", "canvas_2", 0.0, 0.0, 1.0, 1.0);
      canvas_2->SetTopMargin(0.7);
      canvas_2->SetFillColor(0);
      canvas_2->SetFillStyle(0);
      canvas_2->SetGridy(1);
      canvas_2->Draw();
      canvas_2->cd(0);
      
      TH1F * histo_ratio_data = (TH1F*) h_data->Clone();
      histo_ratio_data->Divide(histo_total_MC); //Ratio
      
      //if(total_var_list[ivar] == "METpt")   histo_ratio_data->GetXaxis()->SetTitle(title_MET.Data());
      histo_ratio_data->GetXaxis()->SetTitle(total_var_list[ivar].Data());
      
      histo_ratio_data->SetMinimum(0.0);
      histo_ratio_data->SetMaximum(2.0);
      histo_ratio_data->GetXaxis()->SetTitleOffset(1.2);
      histo_ratio_data->GetXaxis()->SetLabelSize(0.04);
      histo_ratio_data->GetYaxis()->SetLabelSize(0.03);
      histo_ratio_data->GetYaxis()->SetNdivisions(6);
      histo_ratio_data->GetYaxis()->SetTitleSize(0.03);
      histo_ratio_data->Draw("E1X0"); //Draw ratio points
      
      //Copy previous TGraphAsymmErrors
      TGraphAsymmErrors *thegraph_tmp = (TGraphAsymmErrors*) gr->Clone();
      double *theErrorX_h = thegraph_tmp->GetEXhigh();
      double *theErrorY_h = thegraph_tmp->GetEYhigh();
      double *theErrorX_l = thegraph_tmp->GetEXlow();
      double *theErrorY_l = thegraph_tmp->GetEYlow();
      double *theY        = thegraph_tmp->GetY() ;
      double *theX        = thegraph_tmp->GetX() ;
      
      //Divide error --> ratio
      for(int i=0; i<thegraph_tmp->GetN(); i++)
      {
        theErrorY_l[i] = theErrorY_l[i]/theY[i];
        theErrorY_h[i] = theErrorY_h[i]/theY[i];
        theY[i]=1; //To center the filled area around "1"
      }
      
      //--> Create new TGraphAsymmErrors
      TGraphAsymmErrors *thegraph_ratio = new TGraphAsymmErrors(thegraph_tmp->GetN(), theX , theY ,  theErrorX_l, theErrorX_h, theErrorY_l, theErrorY_h);
      thegraph_ratio->SetFillStyle(3005);
      thegraph_ratio->SetFillColor(1);
      thegraph_ratio->Draw("e2 same"); //Syst. error for Data/MC ; drawn on canvas2 (Data/MC ratio)
    }
    
    //Yaxis title
    if(stack!= 0) {stack->GetYaxis()->SetTitleSize(0.04); stack->GetYaxis()->SetTitle("Events");}
    
    //-------------------
    //OUTPUT
    //-------------------
    
    //Image name (.png)
    TString outputname = "plots/"+total_var_list[ivar]+"_"+channel+".png";
    if(channel == "" || allchannels) {outputname = "plots/"+total_var_list[ivar]+"_all.png";}
    
    //cout << __LINE__ << endl;
    if(c1!= 0) {c1->SaveAs(outputname.Data() );}
    //cout << __LINE__ << endl;
    
    delete c1; //Must free dinamically-allocated memory
  } //end var loop
  return 0;
}





//Plot stacked BDT templates VS pseudo-data
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
int theMVAtool::Plot_Templates(TString channel, TString template_name, bool allchannels)
{
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  if(template_name == "BDT" || template_name == "BDTttZ" || template_name == "mWt" || template_name == "m3l") {cout<<FYEL("--- Producing "<<template_name<<" Template Plots ---")<<endl;}
  else {cout<<FRED("--- ERROR : invalid template_name value !")<<endl;}
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  
  TString input_name = "outputs/Reader_" + template_name + filename_suffix + ".root";
  TFile* file_input = 0;
  file_input = TFile::Open( input_name.Data() );
  if(file_input == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}
  
  mkdir("plots",0777);
  TH1::SetDefaultSumw2();
  
  
  vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
  thechannellist.push_back("uuu");
  thechannellist.push_back("uue");
  thechannellist.push_back("eeu");
  thechannellist.push_back("eee");
  
  TH1F *h_sum_MC = 0, *h_tmp = 0, *h_sum_data = 0, *h_signal = 0, *h_fakes = 0, *h_prompt = 0;
  
  for(int ichan=0; ichan<thechannellist.size(); ichan++)
  {
    if(!allchannels && channel != thechannellist[ichan]) {continue;}
    
    //All MC samples
    for(int isample = 0; isample < sample_list.size(); isample++)
    {
      if(sample_list[isample].Contains("Data") || sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("WW") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("Fakes")) {continue;} //See below for fakes
      //if(sample_list[isample].Contains("FCNCttbarZct")) {continue; } // no signal in data
      h_tmp = 0;
      TString histo_name = template_name + "_" + thechannellist[ichan] + "__" + sample_list[isample];
      if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl; continue;}
      else
      {
        h_tmp = (TH1F*) file_input->Get(histo_name.Data());
        if(h_sum_MC == 0) {h_sum_MC = (TH1F*) h_tmp->Clone();}
        else {h_sum_MC->Add(h_tmp);}
        if(h_prompt == 0) {h_prompt = (TH1F*) h_tmp->Clone();}
        else {h_prompt->Add(h_tmp);}
      }
    }
    //Special case : Fakes
    TString template_fake_name = "";
    if(thechannellist[ichan] == "uuu" || thechannellist[ichan] == "eeu") {template_fake_name = "FakeMu";}
    else {template_fake_name = "FakeEl";}
    h_tmp = 0;
    TString histo_name = template_name + "_" + thechannellist[ichan] + "__" + template_fake_name;
    if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
    else
    {
      h_tmp = (TH1F*) file_input->Get(histo_name.Data());
      if(h_sum_MC == 0) {h_sum_MC = (TH1F*) h_tmp->Clone();}
      else {h_sum_MC->Add(h_tmp);}
      if(h_fakes == 0) {h_fakes = (TH1F*) h_tmp->Clone();}
      else {h_fakes->Add(h_tmp);}
    }
    
    //DATA
    h_tmp = 0;
    histo_name = template_name + "_" + thechannellist[ichan] + "__DATA";
    if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
    else
    {
      h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
      if(h_sum_data == 0) {h_sum_data = (TH1F*) h_tmp->Clone();}
      else {h_sum_data->Add(h_tmp);}
    }
    
    //signal
    h_tmp = 0;
    histo_name = template_name + "_" + thechannellist[ichan] + "__NP_overlay_ST_FCNC_zut";
    if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
    else
    {
      h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
      if(h_signal == 0) {h_signal = (TH1F*) h_tmp->Clone();}
      else {h_signal->Add(h_tmp);}
    }
    
  } //end channel loop
  
  if(h_sum_MC == 0 || h_sum_data == 0 || h_signal == 0) {cout<<endl<<BOLD(FRED("--- Empty histogram ! Exit !"))<<endl<<endl; return 0;}
  
  //Make sure there are no negative bins
  for(int ibin = 1; ibin<h_sum_MC->GetNbinsX()+1; ibin++)
  {
    if(h_sum_MC->GetBinContent(ibin) < 0) {h_sum_MC->SetBinContent(ibin, 0);}
    if(h_sum_data->GetBinContent(ibin) < 0) {h_sum_data->SetBinContent(ibin, 0);}
    if(h_signal->GetBinContent(ibin) < 0) {h_signal->SetBinContent(ibin, 0);}
  }
  
  //Set Yaxis maximum & minimum
  if(h_sum_data->GetMaximum() > h_sum_MC->GetMaximum() ) {h_sum_MC->SetMaximum(h_sum_data->GetMaximum()+0.3*h_sum_data->GetMaximum());}
  else h_sum_MC->SetMaximum(h_sum_MC->GetMaximum()+0.3*h_sum_MC->GetMaximum());
  h_sum_MC->SetMinimum(0);
  
  h_sum_MC->SetLineColor(kRed);
  h_sum_MC->SetLineWidth(2);
  h_signal->SetLineColor(kGreen);
  h_signal->SetLineWidth(2);
  h_fakes->SetLineColor(kMagenta);
  h_prompt->SetLineColor(kBlack);
  
  //Canvas definition
  Load_Canvas_Style();
  TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
  
  //Legend
  TLegend* leg = new TLegend(.85,.7,0.965,.915);
  leg->SetHeader("");
  leg->AddEntry(h_sum_MC,"MC","L");
  leg->AddEntry(h_signal,"Signal","L");
  leg->AddEntry(h_fakes,"Fakes","L");
  leg->AddEntry(h_prompt,"Prompt","L");
  if(h_sum_data != 0) {leg->AddEntry(h_sum_data, "Data" , "lep");}
  
  //Draw
  h_sum_MC->Draw("hist");
  h_signal->Draw("hist,same");
  h_fakes->Draw("hist,same");
  h_prompt->Draw("hist,same");
  h_sum_data->Draw("e0psame");
  leg->Draw("same");
  
  //Output
  TString output_plot_name = "plots/" + template_name +"_template_" + channel+ filename_suffix + ".png";
  if(channel == "" || allchannels) {output_plot_name = "plots/" + template_name +"_template_all" + filename_suffix + ".png";}
  
  c1->SaveAs(output_plot_name.Data());
  
  delete c1; delete leg; return 0;
}



int theMVAtool::Draw_Control_Plots_isis(TString channel, bool fakes_from_data, bool allchannels)
{
  
  bool debug = true;
  
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("--- Draw Contol Plots ---")<<endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  
  TString input_file_name = "outputs/Control_Histograms" + filename_suffix + ".root";
  TFile* f = 0;
  f = TFile::Open( input_file_name );
  if(f == 0) {cout<<endl<<BOLD(FRED("--- File not found ! Exit !"))<<endl<<endl; return 0;}
  if(debug) cout << "INPUT FILE " << input_file_name << endl;
  TH1::SetDefaultSumw2();
  
  mkdir("plots",0777);
  TH1::SetDefaultSumw2();
  
  
  vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
  thechannellist.push_back("uuu");
  thechannellist.push_back("uue");
  thechannellist.push_back("eeu");
  thechannellist.push_back("eee");
  
  TH1F *h_sum_MC = 0, *h_tmp = 0, *h_sum_data = 0, *histo_nominal = 0;
  THStack *stack = 0 ;
  vector<TH1F*> v_MC_histo; //Store separately the histos for each MC sample --> stack them after loops
  TH1F* histo_syst = 0;
  //vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)
  
  Load_Canvas_Style();
  
  
  vector<TString> total_var_list;
  for(int i=0; i<v_cut_name.size(); i++)
  {
    total_var_list.push_back(v_cut_name[i].Data());
  }
  for(int i=0; i<var_list.size(); i++)
  {
    total_var_list.push_back(var_list[i].Data());
  }
  if(debug){
    
    for(int i=0; i<total_var_list.size(); i++)
    {
      cout << "i=" << i << " var: " << total_var_list[i] << endl;
    }
    
    
  }

  
  for(int ivar=0; ivar<2 ; ivar++){ //total_var_list.size()
//    if(debug) cout << "Making a plot for " << total_var_list[ivar] << endl;
    // reinitialise for each var
    h_sum_data = 0;
    h_sum_MC = 0;
    stack = 0;
    v_MC_histo.clear();
    histo_nominal = 0;
    vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors (used to create the TGraphError)
    bool initialised = false;
    for(int ichan=0; ichan<thechannellist.size(); ichan++)
    {
      if(!allchannels && channel != thechannellist[ichan]) {continue;}
      
      if(debug) cout << " - Looking at channel " << thechannellist[ichan] << endl;
      TString histo_name = "";
      
      
      //All MC samples
      
      for(int isample = 0; isample < sample_list.size(); isample++)
      {
        debug = true;
        h_tmp = 0;
        if(!fakes_from_data && sample_list[isample].Contains("Fakes") ) {continue;} //Fakes from MC only
        else if(fakes_from_data && (sample_list[isample].Contains("Zjets") || sample_list[isample].Contains("TTJets") || sample_list[isample].Contains("WW") )){continue;} //Fakes from data only
        

        histo_name = "Control_" + thechannellist[ichan] + "_"+ total_var_list[ivar] + "_" + sample_list[isample];
        if(!f->GetListOfKeys()->Contains(histo_name)) {cout<<histo_name<<" : not found !"<<endl; continue;}
        if(debug) cout << "   - Histo " << histo_name << endl ;
        h_tmp = (TH1F*) f->Get(histo_name);
        if(debug) cout << "   - Sample " << sample_list[isample] << endl;
        h_tmp->SetFillStyle(1001);
        h_tmp->SetLineColor(colorVector[isample]);
        int nofbins = h_tmp->GetNbinsX();
        if(debug){
          for(int ibin=1 ; ibin<nofbins ; ibin++)
          {
            cout << ibin << " content " << h_tmp->GetBinContent(ibin) << endl;
          }
        }
        debug = false;
        
        if(!sample_list[isample].Contains("Data") ) {//see below for data
          if(debug) cout << " -- MC" << endl;
          h_tmp->SetFillColor(colorVector[isample]);
          
          if(h_sum_MC == 0) {
            h_sum_MC = (TH1F*) h_tmp->Clone();
            if(debug) cout << "      - Creating MC histo " << endl;
          }
          else {
            h_sum_MC->Add(h_tmp);
            if(debug) cout << "      - Adding MC histo " << endl;
          }
          if(v_MC_histo.size() != sample_list.size())
          {
            v_MC_histo.push_back(h_tmp);
            if(debug) cout << "      - Creating MC histo to vector for sample " << sample_list[isample] << endl;
          }
          else
          {
            //if(debug) cout << "      - MC histo name : " << v_MC_histo[isample-1]->GetName() << endl;
            v_MC_histo[isample-1]->Add(h_tmp);
            if(debug) cout << "      - Adding MC histo to vector for sample " << sample_list[isample] <<  " place " << isample-1 << endl;
            
          }
          // initialise
          if(!initialised){
            for(int ibin=0; ibin<nofbins; ibin++)
            {
              v_eyl.push_back(0); v_eyh.push_back(0);
              v_exl.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2); v_exh.push_back(h_tmp->GetXaxis()->GetBinWidth(ibin+1) / 2);
              v_x.push_back( (h_tmp->GetXaxis()->GetBinLowEdge(nofbins+1) - h_tmp->GetXaxis()->GetBinLowEdge(1) ) * ((ibin+1 - 0.5)/nofbins) + h_tmp->GetXaxis()->GetBinLowEdge(1));
              v_y.push_back(0);
            }
            initialised = true;
          }

          
        }// MC
        else if(sample_list[isample].Contains("Data")){
          if(debug) cout << " -- data "<< endl;
                  h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
         if(h_sum_data == 0) {h_sum_data = (TH1F*) h_tmp->Clone(); if(debug) cout << "      - Creating data histo " << endl;}
         else {h_sum_data->Add(h_tmp); if(debug) cout << "      - Adding data histo " << endl;}
         
        }//data
        
        
        for(int isyst=0; isyst<2; isyst++)//syst_list.size()
        {
          if(sample_list[isample].Contains("Data") || sample_list[isample].Contains("Fakes") ) {break;} //No syst for data and fakes
          if(syst_list[isyst] == "") {
            continue;
          } //Already done
          if(debug) cout << " -- Systematic " << syst_list[isyst]<< " Getting " ;
          
          histo_syst = 0; //Store the "systematic histograms"
          
          histo_name = "Control_" + thechannellist[ichan] + "_"+ total_var_list[ivar] + "_" + sample_list[isample] + "_" + syst_list[isyst];
          TString histo_name_nom = "Control_" + thechannellist[ichan] + "_"+ total_var_list[ivar] + "_" + sample_list[isample] ;
          cout << histo_name << endl;
          if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found !"<<endl; continue;}
          if(!f->GetListOfKeys()->Contains(histo_name_nom.Data())) {cout<<histo_name_nom<<" : not found !"<<endl; continue;}
          histo_syst = (TH1F*) f->Get(histo_name.Data());
          
          histo_nominal = (TH1F*) f->Get(histo_name_nom.Data());
          
          //Add up here the different errors (quadratically), for each bin separately
          for(int ibin=0; ibin<nofbins; ibin++)
          {
            double tmpdouble = 0;
            
            //For each systematic, compute (shifted-nominal), check the sign, and add quadratically to the corresponding bin error
            //--------------------------
            tmpdouble = histo_syst->GetBinContent(ibin+1) - histo_nominal->GetBinContent(ibin+1);
            //if(debug) cout << "    tmpdouble " << tmpdouble << " syst " << histo_syst->GetBinContent(ibin+1) << " nom " <<  histo_nominal->GetBinContent(ibin+1)<< endl;
            if(tmpdouble>0) {v_eyh[ibin]+= pow(tmpdouble,2);}
            else if(tmpdouble<0) {v_eyl[ibin]+= pow(tmpdouble,2);}
            //--------------------------
            
            
           // cout<<"ibin= " << ibin << ", x = "<<v_x[ibin];    cout<<", y = "<<v_y[ibin];    cout<<", eyl = "<<v_eyl[ibin];    cout<<", eyh = "<<v_eyh[ibin]; cout<<", exl = "<<v_exl[ibin];    cout<<", exh = "<<v_exh[ibin]<<endl;
            
          }
        } //end syst loop
        
        
        
        
        
        
      } // end  samples
      
    } //end channel loop
    
     if(h_sum_MC == 0 || h_sum_data == 0) {cout<<endl<<BOLD(FRED("--- Empty histogram ! Exit !"))<<endl<<endl; return 0;}
     
    /*
     
     //Set Yaxis maximum & minimum
     if(h_sum_data->GetMaximum() > h_sum_MC->GetMaximum() ) {h_sum_MC->SetMaximum(h_sum_data->GetMaximum()+0.3*h_sum_data->GetMaximum());}
     else h_sum_MC->SetMaximum(h_sum_MC->GetMaximum()+0.3*h_sum_MC->GetMaximum());
     h_sum_MC->SetMinimum(0);
     
     h_sum_MC->SetLineColor(kRed);
     
     //Canvas definition
     Load_Canvas_Style();
     TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
     
     //Legend
     TLegend* leg = new TLegend(.85,.7,0.965,.915);
     leg->SetHeader("");
     leg->AddEntry(h_sum_MC,"MC","L");
     if(h_sum_data != 0) {leg->AddEntry(h_sum_data, "Data" , "lep");}
     
     //Draw
     h_sum_MC->Draw("hist");
     h_sum_data->Draw("e0psame");
     leg->Draw("same");
     
     //Output
     TString output_plot_name = "plots/" + template_name +"_template_" + channel+ filename_suffix + ".png";
     if(channel == "" || allchannels) {output_plot_name = "plots/" + template_name +"_template_all" + filename_suffix + ".png";}
     
     c1->SaveAs(output_plot_name.Data());
     
     delete c1; delete leg;*/
  } // ivar loop
  return 0;
}





