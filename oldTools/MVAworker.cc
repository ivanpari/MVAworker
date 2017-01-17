//
//  MVAworker.cc
//
//
//  Created by Isis Van Parijs on 25/07/16.
//
//

#include "MVAworker.h"
#include "Tools.h"
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
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h> // to be able to use mkdir

using namespace std;


// default constructor
MVAworker::MVAworker()
{
  cout<<"### ERROR : USE THE OVERLOADED CONSTRUCTOR ! ###"<<endl;
  stopProg = true;
  
}



// The mva worker initialiser
MVAworker::MVAworker(std::vector <TString> VarList, std::vector <TString> SampleList,std::vector <TString> ChanList , std::vector <int> ColorList, std::vector <TString> SystList, std::vector <TString> cutVarList, bool FakesData, bool RealData)
{
  // get all necessary ingredients
  for(unsigned int iSample = 0; iSample < SampleList.size(); iSample++){
    SampleList_.push_back(SampleList[iSample]);
  }
  for(unsigned int iChan = 0; iChan < ChanList.size(); iChan++){
    ChanList_.push_back(ChanList[iChan]);
  }
  for(unsigned int iCol = 0; iCol < ColorList.size(); iCol++){
    ColorList_.push_back(ColorList[iCol]);
  }
  for (unsigned int iVar  = 0; iVar < VarList.size(); iVar++) {
    VarList_.push_back(VarList[iVar]);
    vVar.push_back(0);
  }
  for (unsigned int icVar  = 0; icVar < cutVarList.size(); icVar++) {
    cutVarList_.push_back(cutVarList[icVar]);
    vcutVar.push_back(0);
  }
  for (unsigned int iSys  = 0; iSys < SystList.size(); iSys++) {
    SystList_.push_back(SystList[iSys]);
  }
  
  MVAreader = new TMVA::Reader( "!Color:!Silent" );
  
  
  // for the cuts 
  appliedcuts = "";
  cut_MET=""; cut_mTW=""; cut_NJets=""; cut_NBJets="";
  METpt = 0; mTW = 0; NJets = 0; NBJets = 0;
  
  
  stopProg = false;
  if(verbose > 0) cout << "MVAworker:: MVA worker initialised" << endl;
  
  
}

// lumi scaling
//Set the luminosity re-scaling factor to be used thoughout the code
void MVAworker::Set_Luminosity(double desired_luminosity = 2.26)
{
  double current_luminosity = 2.26; //2015 data / 7.6.x Ntuples --- TO BE CHANGED IN 8.0 / 2016 data!
  this->luminosity_rescale = desired_luminosity / current_luminosity;
  
  cout<<endl<<endl<<endl<<endl<<endl<<"############################################"<<endl;
  cout<<"--- Using luminosity scale factor : "<<desired_luminosity<<" / "<<current_luminosity<<" = "<<luminosity_rescale<<" ! ---"<<endl<<endl;
  cout<<"############################################"<<endl<<endl<<endl<<endl<<endl<<endl;
  
}

void MVAworker::SetCuts(TString set_MET_cut, TString set_mTW_cut, TString set_NJets_cut, TString set_NBJets_cut)
{
  // take the cuts from the input
  
  cut_MET=set_MET_cut;
  cut_mTW=set_mTW_cut;
  cut_NJets=set_NJets_cut;
  cut_NBJets=set_NBJets_cut;
  
  //Jets cuts need to be "strict", in order to avoid several naming issues
  if(cut_NJets.Contains("<=") || cut_NJets.Contains(">=") || cut_NBJets.Contains("<=") || cut_NBJets.Contains(">="))
  {
    cerr<<endl<<endl<<"*** ERROR : jet cuts need to be defined strictly (no >= or <=) ! ***"<<endl<<endl;
    stopProg = true;
  }
  //Make sure that the "equal to" sign is written properly
  else
  {
    if(cut_NJets.Contains("=") && !cut_NJets.Contains("=="))
    {
      cut_NJets = "==" + Convert_Number_To_TString(Find_Number_In_TString(cut_NJets));
      cout<<"I have changed cut_NJets to : "<<cut_NJets<<endl<<endl;
    }
    //Make sure that the "equal to" sign is written properly
    if(cut_NBJets.Contains("=") && !cut_NBJets.Contains("=="))
    {
      cut_NBJets = "==" + Convert_Number_To_TString(Find_Number_In_TString(cut_NBJets));
      cout<<"I have changed cut_NBJets to : "<<cut_NBJets<<endl<<endl;
    }
  }
  appliedcuts = "";
  TString tmpCut = "";
  if(cut_MET != "")
  {
    tmpCut = "_MET" + Convert_Number_To_TString(Find_Number_In_TString(cut_MET));
    appliedcuts+= tmpCut;
  }
  if(cut_mTW != "")
  {
    tmpCut = "_mTW" + Convert_Number_To_TString(Find_Number_In_TString(cut_mTW));
    appliedcuts+= tmpCut;
  }
  if(cut_NJets != "")
  {
    tmpCut = "_NJets" + Convert_Sign_To_Word(cut_NJets) + Convert_Number_To_TString(Find_Number_In_TString(cut_NJets));
    appliedcuts+= tmpCut;
  }
  if(cut_NBJets != "")
  {
    tmpCut = "_NBJets" + Convert_Sign_To_Word(cut_NBJets) + Convert_Number_To_TString(Find_Number_In_TString(cut_NBJets));
    appliedcuts+= tmpCut;
  }
}


//Training - Testing is done per channel
void MVAworker::TrainTestEvaluate(TString channel, bool Gui)
{
  // Load MVA libraries
  TMVA::Tools::Instance();
  if(verbose > 1 ) cout << "MVAworker:: MVA libs loaded" << endl;
  
  // Set output files
  std::string pathBDT = "outputs";
  mkdir(pathBDT.c_str(),0777);
  TString fout = "outputs/BDT";
  if(channel != "") {fout += "_" + channel; }
  else { cerr << "ERROR: no channel selected (MVAworker)" << endl; stopProg = true; }
  fout += appliedcuts;
  fout += ".root";
  TFile* OutFile = TFile::Open(fout, "RECREATE");
  if(verbose > 0) cout << "MVAworker:: Output file " << fout << " made" << endl;
  
  // Make the factory
  TMVA::Factory* factory = new TMVA::Factory( "BDT", OutFile, "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );
  if(verbose >1) cout << "MVAworker:: TMVA factory created " << endl;
  
  // Take the input variables for training
  if(verbose > 1) cout << "MVAworker:: variables used for training: " << endl;
  for(unsigned int iVar = 0; iVar < VarList_.size() ; iVar++){
    factory->AddVariable(VarList_[iVar].Data(),'F');
    cout << "    -- " << VarList_[iVar] << endl;
  }
  
  
  // put here spectators when cuts are set on variables
  // variables used to cut
  factory->AddSpectator( "METpt", "METpt", 'F');
  factory->AddSpectator( "mTW", "mTW", 'F');
  
  // if the variables are active or spectator  ! NEVER AT THE SAME TIME !
  // Special case : NJets & NBJets can be "active" or spectator variables
  if(cut_NJets.Contains("=")) {
    factory->AddSpectator( "NJets", "NJets", 'F');
  }
  else {factory->AddVariable("NJets", 'F');}
  if(cut_NBJets.Contains("=")) {
    factory->AddSpectator( "NBJets", "NBJets", 'F');
  }
  else {factory->AddVariable("NBJets", 'F');}
  
  
  
  //TFile *tempFile(0); // create empty file pointer
  
  // loop over the samples
  if(verbose>0) cout << "MVAworker:: loop over the samples for training " << endl;
  for(unsigned int iSample = 0; iSample < SampleList_.size(); iSample++){
    if(SampleList_[iSample].Contains("Data")){ continue; } // don't train on data
    //Take the input trees
    TString inputfile = "Ntuples/FCNCNTuple_" + SampleList_[iSample] + ".root";
    TFile* file_input = TFile::Open( inputfile.Data() );
    TTree* tree = (TTree*)file_input->Get("Default");
    
    // global event weights per tree (see below for setting event-wise weights)
    Double_t signalWeight     = 1.0;
    Double_t backgroundWeight = 1.0;
    
    if(verbose>0) cout << "MVAworker:: sample " << SampleList_[iSample] << endl;
    if(SampleList_[iSample] == "tZq") {
      factory->AddSignalTree ( tree, signalWeight );
      factory->SetSignalWeightExpression( "fabs(Weight)" );
    }
    else {
      factory->AddBackgroundTree( tree, backgroundWeight );
      factory->SetBackgroundWeightExpression( "fabs(Weight)" );
    }
  }
  // put cuts in the training
  TCut mycuts = "";
  TCut mycutb = "";
  // the channels are defined with integers in the same ntuples
  if(channel != "all")
  {
    if(channel == "uuu")         		{mycuts = "Channel==0"; mycutb = "Channel==0";}
    else if(channel == "uue" )     		{mycuts = "Channel==1"; mycutb = "Channel==1";}
    else if(channel == "eeu"  )     	{mycuts = "Channel==2"; mycutb = "Channel==2";}
    else if(channel == "eee"   )     	{mycuts = "Channel==3"; mycutb = "Channel==3";}
    else 								{cerr << "WARNING : wrong channel name while training " << endl;}
  }
  
  // add cuts to the training
  TString tmp = "";
  //Adds additionnal cuts. Works with simple TStrings
  if(cut_MET != "")	 		{tmp = "METpt"     + cut_MET;}
  if(cut_mTW != "") 			{tmp+= " && mTW"   + cut_mTW;}
  if(cut_NJets != "") 		{tmp+= " && NJets" + cut_NJets;}
  if(cut_NBJets != "")		{tmp+= " && NBJets"+ cut_NBJets;}
  
  if(tmp != "") {mycuts+= tmp; mycutb+= tmp;}
  
  
  // Prepare for training
  factory->PrepareTrainingAndTestTree( mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );
  if(verbose>1) cout << "MVAworker:: Factory prepared for training" << endl;
  
  // Label the output weights differently for each channel and cuts applied
  TString Method = channel + appliedcuts;
  
  // Book the method
  factory->BookMethod( TMVA::Types::kBDT, Method.Data(), "!H:!V:NTrees=100:MinNodeSize=15:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:IgnoreNegWeightsInTraining=True" );
  if(verbose>1) cout << "MVAworker:: Factory method booked for training" << endl;
  
  
  // Go work in the output file
  OutFile->cd();
  
  // Train MVA for all methods specified
  factory->TrainAllMethods();
  if(verbose>1) cout << "MVAworker:: Factory trained for all methods" << endl;
  
  // Test MVA for all methods specified
  factory->TestAllMethods();
  if(verbose>1) cout << "MVAworker:: Factory tested for all methods" << endl;
  
  // Evaluate MVA for all methods specified
  factory->EvaluateAllMethods();
  if(verbose>1) cout << "MVAworker:: Factory evaluated for all methods" << endl;
  
  
  // close the output file
  OutFile->Close();
  delete OutFile;
  if(verbose>0){
    cout << "_________________________________________________________" << endl;
    cout << "MVAworker:: Wrote root file: " << fout << endl;
    cout << "MVAworker:: TMVA training, testing and evaluating is done" << endl;
    cout << "_________________________________________________________" << endl;
  }
  
  if(Gui) {
    cout << "___________________________INSTRUCTIONS______________________________" << endl;
    cout << "Open the TMVA gui interactively in ROOT" << endl;
    cout << "root -l " << endl;
    cout << "TMVA::TMVAGui(\""<< fout <<"\")" << endl;
    cout << "_________________________________________________________" << endl;
  }
}

//produce the templates for theta
void MVAworker::Read(TString method, bool FakesData, bool RealData)
{
  if (method == "BDT" && verbose > 0 ) {
    cout << "MVAworker:: producing the BDT templates" << endl;
  }
  else if(verbose>0){
    cerr << " ERROR: wrong template name (MVAworker)" << endl;
    stopProg = true;
  }
  
  if (!FakesData && verbose > 0) {
    cout << "MVAworker:: using fakes from MC" << endl;
  }
  else if(verbose>0){
    cout << "MVAworker:: using fakes from data" << endl;
  }
  if (!RealData && verbose > 0) {
    cout << "MVAworker:: using pseudodata" << endl;
  }
  else if(verbose>0){
    cout << "MVAworker:: using real data" << endl;
  }
  
  
  // Set output files
  std::string pathBDT = "outputs";
  mkdir(pathBDT.c_str(),0777);
  TString fout = "outputs/Reader";
  fout += appliedcuts;
  fout += ".root";
  TFile* OutFile = TFile::Open(fout, "RECREATE");
  if(verbose > 0) cout << "MVAworker:: Output file " << fout << " made" << endl;
  
  // create empty histogram pointers
  //TH1F* hist_BDT(0), *hist_BDTG(0);
  
  //initialise MVA reader
  MVAreader = new TMVA::Reader("!Color:!Silent");
  if(verbose>1) cout << "MVAworker:: MVA reader initialised " << endl;
  
  
  // variables to be read that were used for training
  for(int iVar=0; iVar<VarList_.size(); iVar++)
  {
    MVAreader->AddVariable(VarList_[iVar].Data(), &(vVar[iVar]));
    if(verbose>1) cout<<"MVAworker:: Added variable "<<VarList_[iVar]<<endl;
  }
  // add spectator variables here
  MVAreader->AddSpectator( "METpt", &METpt);
  MVAreader->AddSpectator( "mTW", &mTW);
  
  // Special case : NJets & NBJets can either be used in training or not
  if(cut_NJets.Contains("=") ) {MVAreader->AddSpectator("NJets", &NJets);}
  else {MVAreader->AddVariable("NJets", &NJets);}
  if(cut_NBJets.Contains("=")) {MVAreader->AddSpectator( "NBJets", &NBJets);}
  else {MVAreader->AddVariable("NBJets", &NBJets);}

  
  // Get the MVA methods weights
  TString dir    = "weights/";
  
  // Book MVA method
  if(method == "BDT")
  {
    // get the weights for each channel
    for(int iChan=0; iChan<ChanList_.size(); iChan++)
    {
      // get the method
      TString MVA_method_name = "BDT_" + ChanList_[iChan] + appliedcuts + TString(" method");
      // weights related to BDT branches
      TString weightfile = dir + "BDT_" + ChanList_[iChan] + appliedcuts + TString(".weights.xml");
      
      MVAreader->BookMVA( MVA_method_name, weightfile );
      if(verbose>1) cout << "MVAworker:: MVA booked for reading" << endl;
    }
  }
  
  //empty hist pointers
  TH1F *hist_uuu = 0, *hist_uue = 0, *hist_eeu = 0, *hist_eee = 0;
  //TH1F *h_sum_fake = 0;
  
  
  // loop over systematics
  for (unsigned int iSys = 0 ; iSys < SystList_.size(); iSys++) {
    if(verbose>1) cout << "MVAworker:: systematics: " << SystList_[iSys] << endl;
    
    // loop over the samples
    for(unsigned int iSample=0; iSample < SampleList_.size(); iSample++){
      if(!RealData && SampleList_[iSample].Contains("Data")) {continue;} //Don't use the real data
      
      //Take the input trees
      TString inputfile = "Ntuples/FCNCNTuple_" + SampleList_[iSample] + ".root";
      TFile* file_input = TFile::Open( inputfile.Data() );
      if(verbose>0) cout << "MVAworker:: running over " << inputfile << endl;
      
      // Make histograms
      if (method == "BDT") //create histogram for each channel (-1 = bkg, +1 = signal)
      {
        hist_uuu     = new TH1F( "MVA_BDT_uuu",           "MVA_BDT_uuu",           nBinsBDT, -1, 1 );
        hist_uue     = new TH1F( "MVA_BDT_uue",           "MVA_BDT_uue",           nBinsBDT, -1, 1 );
        hist_eeu     = new TH1F( "MVA_BDT_eeu",           "MVA_BDT_eeu",           nBinsBDT, -1, 1 );
        hist_eee     = new TH1F( "MVA_BDT_eee",           "MVA_BDT_eee",           nBinsBDT, -1, 1 );
        
      }
      // such that error bars are correct
      hist_uuu->Sumw2(); hist_uue->Sumw2(); hist_eeu->Sumw2(); hist_eee->Sumw2();
      
      TTree* tree(0);
      TString treeName = "";
      
      // for some systematics the treename will change
      if(SystList_[iSys]== "JER__plus" || SystList_[iSys] == "JER__minus" || SystList_[iSys]== "JES__plus" || SystList_[iSys] == "JES__minus") {
        treeName = SystList_[iSys].Data();
      }
      else { treeName = "Default"; }
      
      //default tree
      tree = (TTree*) file_input->Get(treeName);
      if(verbose>1) cout << "MVAworker:: got tree with name " << treeName << endl;
      
      // get the BDT variable branches of the tree
      for(int iVar=0; iVar<VarList_.size(); iVar++)
      {
        tree->SetBranchAddress(VarList_[iVar].Data(), &vVar[iVar]);
      }
      //For the 'cut var list', use specific class members to avoid mistakes.
      for(int i=0; i<cutVarList_.size(); i++)
      {
        if(cutVarList_[i] == "METpt") 	{tree->SetBranchAddress(cutVarList_[i].Data(), &METpt);}
        if(cutVarList_[i] == "mTW") 	{tree->SetBranchAddress(cutVarList_[i].Data(), &mTW);}
        if(cutVarList_[i] == "NJets") 	{tree->SetBranchAddress(cutVarList_[i].Data(), &NJets);}
        if(cutVarList_[i] == "NBJets") {tree->SetBranchAddress(cutVarList_[i].Data(), &NBJets);}
      }
      
      
      // the channels are defined with integers, get the channel out of the tree
      float i_channel;
      tree->SetBranchAddress("Channel", &i_channel);
      // for systematics
      
      float weight;
      //For some systematics, only the weight changes
      if(SystList_[iSys] == "" || SystList_[iSys].Contains("JER") || SystList_[iSys].Contains("JES"))	{tree->SetBranchAddress("Weight", &weight);}
      else if(SystList_[iSys] == "PU__plus") 		tree->SetBranchAddress("PU__plus", &weight);
      else if(SystList_[iSys] == "PU__minus")		tree->SetBranchAddress("PU__minus", &weight);
      else if(SystList_[iSys] == "Q2__plus") 		tree->SetBranchAddress("Q2__plus", &weight);
      else if(SystList_[iSys] == "Q2__minus") 		tree->SetBranchAddress("Q2__minus", &weight);
      else if(SystList_[iSys] == "MuEff__plus") 		tree->SetBranchAddress("MuEff__plus", &weight);
      else if(SystList_[iSys] == "MuEff__minus") 	tree->SetBranchAddress("MuEff__minus", &weight);
      else if(SystList_[iSys] == "EleEff__plus") 	tree->SetBranchAddress("EleEff__plus", &weight);
      else if(SystList_[iSys] == "EleEff__minus")	tree->SetBranchAddress("EleEff__minus", &weight);
      else {cout<<"Problem : Wrong systematic name"<<endl;}
      

      
      if(verbose>1) cout << "MVAworker:: all branches are set" << endl ;
      if(verbose>0) cout << "MVAworker:: Processing " << tree->GetEntries() << " events" << std::endl;
      // loop over the events
      int nEvtsAfter = 0;
      for (int ievt = 1 ; ievt < tree->GetEntries(); ievt++) {
        // initialise some variables
        weight = 0;
        i_channel = 9;
        
        //cout << "ievt " << ievt << endl;
        // get the event entry
        tree->GetEntry(ievt);
        
        
        // Apply cuts for the reader as well (should be consistent with trainer)
        // Skip events that don't meet the conditions
        float cut_tmp = 0;
        
        if(cut_MET != "")
        {
          cut_tmp = Find_Number_In_TString(cut_MET);
          if(cut_MET.Contains(">=") && METpt < cut_tmp) {continue;}
          else if(cut_MET.Contains("<=") && METpt > cut_tmp) {continue;}
          else if(cut_MET.Contains(">") && METpt <= cut_tmp) {continue;}
          else if(cut_MET.Contains("<") && METpt >= cut_tmp) {continue;}
          //else if(cut_MET.Contains("==") && METpt != cut_tmp) {continue;}
        }
        if(cut_mTW != "")
        {
          cut_tmp = Find_Number_In_TString(cut_mTW);
          if(cut_mTW.Contains(">=") && mTW < cut_tmp) {continue;}
          else if(cut_mTW.Contains("<=") && mTW > cut_tmp) {continue;}
          else if(cut_mTW.Contains(">") && mTW <= cut_tmp) {continue;}
          else if(cut_mTW.Contains("<") && mTW >= cut_tmp) {continue;}
          //else if(cut_mTW.Contains("==") && mTW != cut_tmp) {continue;}
        }
        if(cut_NJets != "")
        {
          cut_tmp = Find_Number_In_TString(cut_NJets);
          if(cut_NJets.Contains(">=") && NJets < cut_tmp) {continue;}
          else if(cut_NJets.Contains("<=") && NJets > cut_tmp) {continue;}
          else if(cut_NJets.Contains(">") && NJets <= cut_tmp) {continue;}
          else if(cut_NJets.Contains("<") && NJets >= cut_tmp) {continue;}
          else if(cut_NJets.Contains("==") && NJets != cut_tmp) {continue;}
        }
        if(cut_NBJets != "")
        {
          cut_tmp = Find_Number_In_TString(cut_NBJets);
          if(cut_NBJets.Contains(">=") && NBJets < cut_tmp) {continue;}
          else if(cut_NBJets.Contains("<=") && NBJets > cut_tmp) {continue;}
          else if(cut_NBJets.Contains(">") && NBJets <= cut_tmp) {continue;}
          else if(cut_NBJets.Contains("<") && NBJets >= cut_tmp) {continue;}
          else if(cut_NBJets.Contains("==") && NBJets != cut_tmp) {continue;}
        }

        nEvtsAfter++;
        
        // rescale to desired lumi
        weight *= luminosity_rescale;
        
        
        // fill histograms
        if (method == "BDT")
        {
          if(i_channel == 0) {
            hist_uuu->Fill( MVAreader->EvaluateMVA( "BDT_uuu"+appliedcuts+ " method"), weight);
          }
          else if(i_channel == 1) {
            hist_uue->Fill( MVAreader->EvaluateMVA( "BDT_uue"+appliedcuts+" method"), weight);
          }
          else if(i_channel == 2) {
            hist_eeu->Fill( MVAreader->EvaluateMVA( "BDT_eeu"+appliedcuts+" method"), weight);
          }
          else if(i_channel == 3) {
            hist_eee->Fill( MVAreader->EvaluateMVA( "BDT_eee"+appliedcuts+" method"), weight);
          }
          else if(i_channel == 9 || weight == 0) {
            cerr <<"MVAworker: problem with filling BDT histograms "<<endl;
          }
        }
      } // end event loop
      
      // write histograms
      OutFile->cd();
      
      // Introduce the theta name convention
      // <observable>__<process>[__<uncertainty>__(plus,minus)]
      TString output_histo_name = "";
      TString syst_name = "";
      if(SystList_[iSys] != "") syst_name = "__" + SystList_[iSys];
      TString sample_name = SampleList_[iSample];
      if(RealData) {sample_name = "DATA";} //THETA CONVENTION
      
      
      if (method == "BDT")
      {
        output_histo_name = "BDT_uuu__" + sample_name + syst_name;
        hist_uuu->Write(output_histo_name.Data());
        output_histo_name = "BDT_uue__" + sample_name + syst_name;
        hist_uue->Write(output_histo_name.Data());
        output_histo_name = "BDT_eeu__" + sample_name + syst_name;
        hist_eeu->Write(output_histo_name.Data());
        output_histo_name = "BDT_eee__" + sample_name + syst_name;
        hist_eee->Write(output_histo_name.Data());
      }
      
      
      if(verbose>0) cout << "MVAworker:: done running over " << inputfile << endl;
      if(verbose>0) cout <<"MVAworker:: nEvts after cuts " << nEvtsAfter << endl;
      
      
      
    } // end sample loop
    if(verbose>1) cout << "MVAworker:: Done with "<<SystList_[iSys]<<" syst"<<endl;
  } // end systematics loop
  OutFile -> Close();
  delete OutFile;
  if(verbose>0){
    cout << "_________________________________________________________" << endl;
    std::cout << "MVAworker:: Created root file: \""<<fout<<"\" containing the output histograms" << std::endl;
    std::cout << "MVAworker Reader() is done!" << std::endl << std::endl;
    cout << "_________________________________________________________" << endl;
  }
  
}


float MVAworker::DetermineCR()
{
  // take the output from the reader
  TString FileIN = "outputs/Reader" + appliedcuts + ".root";
  TFile* fIn = TFile::Open(FileIN.Data());
  if(verbose>1) cout << "Opened " << FileIN << endl; 
  
  // set empty pointer
  TH1F *h_sum_bkg(0), *h_sig(0), *h_tmp(0);
  
  TString HistName = "";
  
  
  // loop over the samples and get the needed histograms
  for(unsigned int iSample = 0; iSample < SampleList_.size(); iSample++){
    if (verbose>1) {
      cout << "MVAworker:: looking at " << SampleList_[iSample] << endl;
    }
    // skip data
    
    // loop over channels because cuts can differ
    for(unsigned int iChan =0 ; iChan < ChanList_.size(); iChan++){
      //reset the temp hist
      h_tmp = 0;
      if(SampleList_[iSample].Contains("Data")) {continue;}
      // take the histogram
      HistName =  "BDT_" + ChanList_[iChan] + "__" + SampleList_[iSample];
      // check if it exists
      if(!fIn->GetListOfKeys()->Contains(HistName.Data())) {
        cerr << "ERROR: histogram " << HistName << " doesn't exist" << endl;
        continue;
      }
      cout << HistName << endl;
      h_tmp = (TH1F*) fIn->Get(HistName.Data())->Clone();
      
      
      if(SampleList_[iSample] == "tZq")
      {
        if(h_sig == 0) {h_sig = (TH1F*) h_tmp->Clone();}
        else {h_sig->Add(h_tmp);}
      }
      else
      {
        if(h_sum_bkg == 0) {h_sum_bkg = (TH1F*) h_tmp->Clone();}
        else {h_sum_bkg->Add(h_tmp);}
      }
      
      
    }
  } // end sample list
  
  // Make S+B histo for significance calculation
  TH1F* h_total = (TH1F*) h_sum_bkg->Clone();
  h_total->Add(h_sig);
  
  // start looking for the cut where there is most background --> will determine CR
  double sig_over_total = 100; //initialize to unreasonable value
  int bin_cut = -1; //initialize to false value
  int nofbins = h_total->GetNbinsX();
  
  for(int ibin=nofbins; ibin>0; ibin--)
  {
    //Search the bin w/ lowest sig/total, while keeping enough bkg events (criterion needs to be optimized/tuned)
    if(verbose>2){
      cout << "ibin " << ibin << endl;
      cout << "h_sig->Integral(1, ibin)= " << h_sig->Integral(1, ibin) << endl;
      cout << "h_total->Integral(1, ibin)= " << h_total->Integral(1, ibin) << endl;
      cout << "h_sum_bkg->Integral(1, ibin)= " << h_sum_bkg->Integral(1, ibin) << endl;
    }
    double sig = h_sig->Integral(1, ibin) / h_total->Integral(1, ibin);
    double back = h_sum_bkg->Integral(1, ibin) / h_sum_bkg->Integral();
    if(verbose>2)  cout << "sig " << sig << " back " << back << endl;
    if( sig < sig_over_total && back >= 0.6 )
    {
      bin_cut = ibin;
      sig_over_total = h_sig->Integral(1, bin_cut) / h_total->Integral(1,bin_cut);
      if(verbose > 1) {
         cout << "bin_cut " << bin_cut << " sig over total " << sig_over_total << " cutvalue " << h_total->GetBinLowEdge(bin_cut+1) <<  endl;
      }
    }
  }
  
  double cut = h_total->GetBinLowEdge(bin_cut+1); //Get the BDT cut value to apply to create a BDT CR control tree

  
  
  // Make things pretty
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
  c->SaveAs("outputs/Signal_Background_BDT"+appliedcuts+".png");
  
  //Cout some results
  cout<<"---------------------------------------"<<endl;
  cout<<"* Cut Value = "<<cut<<endl;
  cout<<"-> BDT_CR defined w/ all events inside bins [1 ; "<<bin_cut<<"] of the BDT distribution!"<<endl<<endl;
  cout<<"* Signal integral = "<<h_sig->Integral(1, bin_cut)<<" / Total integral "<<h_total->Integral(1, bin_cut)<<endl;
  cout<<"Signal contamination in CR --> Sig/Total = "<<sig_over_total<<endl;
  cout<<"Bkg(CR) / Bkg(Total) = "<<h_sum_bkg->Integral(1,bin_cut) / h_sum_bkg->Integral()<<endl;
  cout<<"---------------------------------------"<<endl<<endl;
  
  //for(int i=0; i<h_sig->GetNbinsX(); i++) {cout<<"bin content "<<i+1<<" = "<<h_sig->GetBinContent(i+1)<<endl;} //If want to verify that the signal is computed correctly
  //clean up after yourself
  fIn->Close();
  delete c;
  delete leg;
  delete l;
  
  return cut;

  
}


void MVAworker::ControlTrees(bool FakesData, bool doCRcut , double CRcut)
{
  if(!FakesData) {cout<<"--- Using fakes from MC ---"<<endl;}
  else {cout<<"--- Using fakes from data ---"<<endl;}
  if(doCRcut) {cout<<endl<<"--- Creating control tree WITH cut on BDT value ---"<<endl<<" Cut value = "<< CRcut<<endl<<endl;}
  else if(!doCRcut) {cout<<endl<<"--- Creating control tree WITHOUT cut on BDT value ---"<<endl;}
  
  
  // initialise reader to read the BDTs,
  //initialise MVA reader
  MVAreader = new TMVA::Reader("!Color:!Silent");
  
  if(verbose>1) cout << "MVAworker:: MVA reader initialised " << endl;
  
  
  // variables to be read that were used for training
  for(int iVar=0; iVar<VarList_.size(); iVar++)
  {
    MVAreader->AddVariable(VarList_[iVar].Data(), &(vVar[iVar]));
    if(verbose>1) cout<<"MVAworker:: Added variable "<<VarList_[iVar]<<endl;
  }
  // add spectator variables here
  MVAreader->AddSpectator( "METpt", &METpt);
  MVAreader->AddSpectator( "mTW", &mTW);
  
  // Special case : NJets & NBJets can either be used in training or not
  if(cut_NJets.Contains("=") ) {MVAreader->AddSpectator("NJets", &NJets);}
  else {MVAreader->AddVariable("NJets", &NJets);}
  if(cut_NBJets.Contains("=")) {MVAreader->AddSpectator( "NBJets", &NBJets);}
  else {MVAreader->AddVariable("NBJets", &NBJets);}
  
  
  // Get the MVA methods weights
  TString dir    = "weights/";
  
  // Book MVA method
  
  // get the weights for each channel
  for(int iChan=0; iChan<ChanList_.size(); iChan++)
  {
    // get the method
    TString MVA_method_name = "BDT_" + ChanList_[iChan] + appliedcuts + TString(" method");
    // weights related to BDT branches
    TString weightfile = dir + "BDT_" + ChanList_[iChan] + appliedcuts + TString(".weights.xml");
    
    MVAreader->BookMVA( MVA_method_name, weightfile );
    if(verbose>1) cout << "MVAworker:: MVA booked for reading" << endl;
  }
  
  // loop over systematics
  for (unsigned int iSys = 0 ; iSys < SystList_.size(); iSys++) {
    if(verbose>1) cout << "MVAworker:: systematics: " << SystList_[iSys] << endl;
    
    // loop over the samples
    for(unsigned int iSample=0; iSample < SampleList_.size(); iSample++){
      if((SampleList_[iSample].Contains("Data") || SampleList_[iSample].Contains("Fakes")) && SystList_[iSys]!="") {continue;} // no systematics for data
      //Take the input files
      TString inputfile = "Ntuples/FCNCNTuple_" + SampleList_[iSample] + ".root";
      TFile* file_input = TFile::Open( inputfile.Data() );
      if(verbose>0) cout << "MVAworker:: running over " << inputfile << endl;
      
      // Create output files
      TString outputFileName = "outputs/Control_Trees" + appliedcuts + ".root";
      TFile* outputFile = TFile::Open( outputFileName, "UPDATE" );
      
      //Create new output tree, that will be filled only with events verifying MVA<cut
      TTree *tree(0), *tree_control(0);
      tree_control = new TTree("tree_control", "Control Tree");
      
      // sey the variables
      for(int iVar=0; iVar<VarList_.size(); iVar++)
      {
        TString var_type = VarList_[iVar] + "/F";
        tree_control->Branch(VarList_[iVar].Data(), &(vVar[iVar]), var_type.Data());
      }
      //For the 'cut var list', use specific class members to avoid mistakes.
      tree_control->Branch("METpt", &METpt, "METpt/F");
      tree_control->Branch("mTW", &mTW, "mTW/F");
      tree_control->Branch("NJets", &NJets, "NJets/F");
      tree_control->Branch("NBJets", &NBJets, "NBJets/F");
      float weight; float i_channel;
      tree_control->Branch("Weight", &weight, "weight/F"); //Give it the same name regardless of the systematic
      tree_control->Branch("Channel", &i_channel, "i_channel/F");
      
      // input tree
      TString treeName = "";
      // for some systematics the treename will change
      if(SystList_[iSys]== "JER__plus" || SystList_[iSys] == "JER__minus" || SystList_[iSys]== "JES__plus" || SystList_[iSys] == "JES__minus") {
        treeName = SystList_[iSys].Data();
      }
      else { treeName = "Default"; }
      
      //default tree
      tree = (TTree*) file_input->Get(treeName);
      if(verbose>1) cout << "MVAworker:: got tree with name " << treeName << endl;
      
      // get the BDT variable branches of the tree
     if(verbose >2) cout << "VarList_.size() " << VarList_.size()<< " vVar " << vVar.size() << endl;
      for(int iVar=0; iVar<VarList_.size(); iVar++)
      {
        if(verbose >2)  cout << iVar << " " << VarList_[iVar] << " " << vVar[iVar] <<  endl;
        tree->SetBranchAddress(VarList_[iVar].Data(), &vVar[iVar]);
      }
      //For the 'cut var list', use specific class members to avoid mistakes.
      for(int i=0; i<cutVarList_.size(); i++)
      {
        if(cutVarList_[i] == "METpt") 	{tree->SetBranchAddress(cutVarList_[i].Data(), &METpt);}
        if(cutVarList_[i] == "mTW") 	{tree->SetBranchAddress(cutVarList_[i].Data(), &mTW);}
        if(cutVarList_[i] == "NJets") 	{tree->SetBranchAddress(cutVarList_[i].Data(), &NJets);}
        if(cutVarList_[i] == "NBJets") {tree->SetBranchAddress(cutVarList_[i].Data(), &NBJets);}
      }
      
      
      // the channels are defined with integers, get the channel out of the tree
      tree->SetBranchAddress("Channel", &i_channel);
      // for systematics
      
      
      //For some systematics, only the weight changes
      if(SystList_[iSys] == "" || SystList_[iSys].Contains("JER") || SystList_[iSys].Contains("JES"))	{tree->SetBranchAddress("Weight", &weight);}
      else if(SystList_[iSys] == "PU__plus") 		tree->SetBranchAddress("PU__plus", &weight);
      else if(SystList_[iSys] == "PU__minus")		tree->SetBranchAddress("PU__minus", &weight);
      else if(SystList_[iSys] == "Q2__plus") 		tree->SetBranchAddress("Q2__plus", &weight);
      else if(SystList_[iSys] == "Q2__minus") 		tree->SetBranchAddress("Q2__minus", &weight);
      else if(SystList_[iSys] == "MuEff__plus") 		tree->SetBranchAddress("MuEff__plus", &weight);
      else if(SystList_[iSys] == "MuEff__minus") 	tree->SetBranchAddress("MuEff__minus", &weight);
      else if(SystList_[iSys] == "EleEff__plus") 	tree->SetBranchAddress("EleEff__plus", &weight);
      else if(SystList_[iSys] == "EleEff__minus")	tree->SetBranchAddress("EleEff__minus", &weight);
      else {cout<<"Problem : Wrong systematic name"<<endl;}
      
      
      
      if(verbose>1) cout << "MVAworker:: all branches are set" << endl ;
      if(verbose>0) cout << "MVAworker:: Processing " << tree->GetEntries() << " events" << std::endl;
      
      // loop over the events
      int nEvtsAfter = 0;
      for (int ievt = 1 ; ievt < tree->GetEntries(); ievt++) {
        // initialise some variables
        weight = 0;
        i_channel = 9;
        
        //cout << "ievt " << ievt << endl;
        // get the event entry
        tree->GetEntry(ievt);
        
        
        // Apply cuts for the reader as well (should be consistent with trainer)
        // Skip events that don't meet the conditions
        float cut_tmp = 0;
        
        if(cut_MET != "")
        {
          cut_tmp = Find_Number_In_TString(cut_MET);
          if(cut_MET.Contains(">=") && METpt < cut_tmp) {continue;}
          else if(cut_MET.Contains("<=") && METpt > cut_tmp) {continue;}
          else if(cut_MET.Contains(">") && METpt <= cut_tmp) {continue;}
          else if(cut_MET.Contains("<") && METpt >= cut_tmp) {continue;}
          //else if(cut_MET.Contains("==") && METpt != cut_tmp) {continue;}
        }
        if(cut_mTW != "")
        {
          cut_tmp = Find_Number_In_TString(cut_mTW);
          if(cut_mTW.Contains(">=") && mTW < cut_tmp) {continue;}
          else if(cut_mTW.Contains("<=") && mTW > cut_tmp) {continue;}
          else if(cut_mTW.Contains(">") && mTW <= cut_tmp) {continue;}
          else if(cut_mTW.Contains("<") && mTW >= cut_tmp) {continue;}
          //else if(cut_mTW.Contains("==") && mTW != cut_tmp) {continue;}
        }
        if(cut_NJets != "")
        {
          cut_tmp = Find_Number_In_TString(cut_NJets);
          if(cut_NJets.Contains(">=") && NJets < cut_tmp) {continue;}
          else if(cut_NJets.Contains("<=") && NJets > cut_tmp) {continue;}
          else if(cut_NJets.Contains(">") && NJets <= cut_tmp) {continue;}
          else if(cut_NJets.Contains("<") && NJets >= cut_tmp) {continue;}
          else if(cut_NJets.Contains("==") && NJets != cut_tmp) {continue;}
        }
        if(cut_NBJets != "")
        {
          cut_tmp = Find_Number_In_TString(cut_NBJets);
          if(cut_NBJets.Contains(">=") && NBJets < cut_tmp) {continue;}
          else if(cut_NBJets.Contains("<=") && NBJets > cut_tmp) {continue;}
          else if(cut_NBJets.Contains(">") && NBJets <= cut_tmp) {continue;}
          else if(cut_NBJets.Contains("<") && NBJets >= cut_tmp) {continue;}
          else if(cut_NBJets.Contains("==") && NBJets != cut_tmp) {continue;}
        }
        
        if(doCRcut){
          if(i_channel == 0 && MVAreader->EvaluateMVA( "BDT_uuu"+appliedcuts+" method") > CRcut) 		{continue;}
          else if(i_channel == 1 && MVAreader->EvaluateMVA( "BDT_uue"+appliedcuts+" method") > CRcut) 	{continue;}
          else if(i_channel == 2 && MVAreader->EvaluateMVA( "BDT_eeu"+appliedcuts+" method") > CRcut) 	{continue;}
          else if(i_channel == 3 && MVAreader->EvaluateMVA( "BDT_eee"+appliedcuts+" method") > CRcut) 	{continue;}
          
        }
        
        nEvtsAfter++;
        
        // rescale to desired lumi
        weight *= luminosity_rescale;
        tree_control->Fill();
      } // end event loop
      
      // write output trees
      outputFile->cd();
    
      
      //NB : theta name convention = <observable>__<process>__<uncertainty>[__(plus,minus)]
      TString output_tree_name = "Control_" + SampleList_[iSample];
      if (SystList_[iSys] != "") {output_tree_name+= "_" + SystList_[iSys];}
      
      tree_control->Write(output_tree_name.Data(), TObject::kOverwrite);
      
      delete tree_control;
      outputFile->Close();
      if(verbose>1) cout<<"Done with "<<SampleList_[iSample]<<" sample"<<endl;

      
    } // samples
    if(verbose>1) cout<<"Done with "<<SystList_[iSys]<<" syst"<<endl;
  } // syst
  cout << "_________________________________________________________" << endl;
  std::cout << "MVAworker:: Created root file containing the output trees" << std::endl;
  std::cout << "MVAworker:: Create_Control_Trees() is done!" << std::endl << std::endl;
  cout << "_________________________________________________________" << endl;
  
}


void MVAworker::ControlHisto(TString channel, bool FakesData)
{
  cout<<endl<<"#####################"<<endl;
  if(!FakesData) {cout<<"--- Using fakes from MC ---"<<endl;}
  else {cout<<"--- Using fakes from data ---"<<endl;}
  cout<<"#####################"<<endl<<endl;
  
  TString input_file_name = "outputs/Control_Trees" + appliedcuts + ".root";
  TString output_file_name = "outputs/Control_Histograms" + appliedcuts + ".root";
  TFile* f_input = TFile::Open( input_file_name ); TFile* f_output = TFile::Open( output_file_name, "UPDATE" );
  TTree* tree = 0;
  TH1F* h_tmp = 0;
  
  int binning = 5;
  
  //Want to plot ALL variables (inside the 2 different variable vectors !)
  vector<TString> total_VarList_;
  for(int i=0; i<cutVarList_.size(); i++)
  {
    total_VarList_.push_back(cutVarList_[i].Data());
  }
  for(int i=0; i<VarList_.size(); i++)
  {
    total_VarList_.push_back(VarList_[i].Data());
  }
  
  int nof_histos_to_create = ((SampleList_.size() - 1) * total_VarList_.size() * SystList_.size()) + total_VarList_.size();
  
  cout<<"################################"<<endl;
  cout<<" *** CHANNEL "<<channel<<" ***"<<endl;
  cout<<endl<<"--- Going to create "<<nof_histos_to_create<<" histograms ! (* 4 channels = "<<nof_histos_to_create*4<<" in total)"<<endl;
  cout<<"--- This might take a while... !"<<endl<<endl;
  cout<<"################################"<<endl;
  
  for(int iVar=0; iVar<total_VarList_.size(); iVar++)
  {
    cout<<"--- Processing variable : "<<total_VarList_[iVar]<<endl;
    //Info contained in tree leaves. Need to create histograms first
    for(int iSample = 0; iSample < SampleList_.size(); iSample++)
    {
      cout<<"--- Processing "<<SampleList_[iSample]<<endl<<endl;
      
      for(int iSys=0; iSys<SystList_.size(); iSys++)
      {
        if(!FakesData && SampleList_[iSample].Contains("Fakes") ) {continue;} //Fakes from MC only
        else if(FakesData && (SampleList_[iSample].Contains("DY") || SampleList_[iSample].Contains("TT") || SampleList_[iSample].Contains("WW") ) ) {continue;} //Fakes from data only
        
        if((SampleList_[iSample].Contains("Data") || SampleList_[iSample].Contains("Fakes")) && SystList_[iSys] != "") {continue;}
        
        
        h_tmp = 0;
        if(total_VarList_[iVar] == "mTW") 							{h_tmp = new TH1F( "","", binning, 0, 150 );}
        else if(total_VarList_[iVar] == "METpt")						{h_tmp = new TH1F( "","", binning, 0, 150 );}
        else if(total_VarList_[iVar] == "ZCandMass") 					{h_tmp = new TH1F( "","", binning, 70, 110 );}
        else if(total_VarList_[iVar] == "deltaPhilb") 				{h_tmp = new TH1F( "","", binning, -4, 4 );}
        else if(total_VarList_[iVar] == "Zpt") 						{h_tmp = new TH1F( "","", binning, 0, 150 );}
        else if(total_VarList_[iVar] == "ZEta")			 			{h_tmp = new TH1F( "","", binning, -4, 4 );}
        else if(total_VarList_[iVar] == "asym") 						{h_tmp = new TH1F( "","", binning, -3, 3 );}
        else if(total_VarList_[iVar] == "mtop") 						{h_tmp = new TH1F( "","", binning, 60, 210 );}
        else if(total_VarList_[iVar] == "btagDiscri") 				{h_tmp = new TH1F( "","", binning, 0.4, 2.4 );}
        else if(total_VarList_[iVar] == "btagDiscri_subleading")		{h_tmp = new TH1F( "","", binning, 0, 1 );}
        else if(total_VarList_[iVar] == "etaQ")						{h_tmp = new TH1F( "","", binning, -4, 4 );}
        else if(total_VarList_[iVar] == "NBJets")						{h_tmp = new TH1F( "","", 3, 0, 3 );}
        else if(total_VarList_[iVar] == "AddLepPT")					{h_tmp = new TH1F( "","", binning, 0, 150 );}
        else if(total_VarList_[iVar] == "AddLepETA")					{h_tmp = new TH1F( "","", binning, -4, 4 );}
        else if(total_VarList_[iVar] == "LeadJetPT")					{h_tmp = new TH1F( "","", binning, 0, 150 );}
        else if(total_VarList_[iVar] == "LeadJetEta")					{h_tmp = new TH1F( "","", binning, -4, 4 );}
        else if(total_VarList_[iVar] == "dPhiZMET")					{h_tmp = new TH1F( "","", binning, -4, 4 );}
        else if(total_VarList_[iVar] == "dPhiZAddLep")				{h_tmp = new TH1F( "","", binning, -4, 4 );}
        else if(total_VarList_[iVar] == "dRAddLepBFromTop")			{h_tmp = new TH1F( "","", binning, 0, 1 );}
        else if(total_VarList_[iVar] == "dRZAddLep")					{h_tmp = new TH1F( "","", binning, 0, 1 );}
        else if(total_VarList_[iVar] == "dRZTop")						{h_tmp = new TH1F( "","", binning, 0, 1 );}
        else if(total_VarList_[iVar] == "TopPT")						{h_tmp = new TH1F( "","", binning, 0, 150 );}
        else if(total_VarList_[iVar] == "NJets")						{h_tmp = new TH1F( "","", 5, 1, 6 );}
        else if(total_VarList_[iVar] == "ptQ")						{h_tmp = new TH1F( "","", binning, 0, 150 );}
        else if(total_VarList_[iVar] == "dRjj")						{h_tmp = new TH1F( "","", binning, 0, 1 );}
        else {cout<<"Unknown variable"<<endl;}
        
        h_tmp->Sumw2(); //force the storage and computation of the sum of the square of weights per bin (rather than just take srt(bin_content))
        
        
        //get input tree
        TString tree_name = "Control_" + SampleList_[iSample];
        if(SystList_[iSys] != "") {tree_name+= "_" + SystList_[iSys];}
        if(!f_input->GetListOfKeys()->Contains(tree_name.Data())) {cout<<tree_name<<" : ERROR"<<endl; continue;}
        tree = (TTree*) f_input->Get(tree_name.Data());
        
        float weight = 0, tmp = 0, i_channel = 9;
        tree->SetBranchAddress(total_VarList_[iVar], &tmp); //One variable at a time
        tree->SetBranchAddress("Weight", &weight);
        tree->SetBranchAddress("Channel", &i_channel);
        
        int tree_nentries = tree->GetEntries();
        
        for(int ientry = 0; ientry<tree_nentries; ientry++)
        {
          //reset vars
          weight = 0; tmp = 0; i_channel = 9;
          tree->GetEntry(ientry); //Read event
          
          //NB : No need to re-apply variables cuts here, as the control_tree is only filled with events that pass these cuts
          
          //safety
          if(channel == "uuu" && i_channel!= 0) {continue;}
          else if(channel == "uue" && i_channel!= 1) {continue;}
          else if(channel == "eeu" && i_channel!= 2) {continue;}
          else if(channel == "eee" && i_channel!= 3) {continue;}
          else if(channel == "9") {cout<<__LINE__<<" : ERROR !"<<endl;}
          
          h_tmp->Fill(tmp, weight); //Fill histogram -- weight already re-scaled to desired lumi in Create_Control_Trees !
        }
        
        TString output_histo_name = "Control_"+ channel + "_" + total_VarList_[iVar];
        output_histo_name+= "_" + SampleList_[iSample];
        //output_histo_name+= "_" + channel;
        if(SystList_[iSys] != "") {output_histo_name+= "_" + SystList_[iSys];}
        f_output->cd();
        h_tmp->Write(output_histo_name.Data(), TObject::kOverwrite);
      } //end syst loop
    } //end sample loop
  } //end var loop
  
  f_input->Close();
  f_output->Close();

  
}


void MVAworker::PseudoDataCR(TString channel , bool FakesData)
{
  cout<<endl<<"#####################"<<endl;
  if(!FakesData) {cout<<"--- Using fakes from MC ---"<<endl;}
  else {cout<<"--- Using fakes from data ---"<<endl;}
  cout<<"#####################"<<endl<<endl;
  
  TRandom3 therand(0);
  
  TString input_name = "outputs/Control_Histograms" + appliedcuts + ".root";
  TFile* file = TFile::Open( input_name.Data(), "UPDATE");
  cout<<endl<<"--- GENERATION OF PSEUDODATA IN "<<file->GetName()<<" ! ---"<<endl<<endl;
  
  TH1F *h_sum = 0, *h_tmp = 0;
  
  //file->ls(); //output the content of the file
  
  for(int iVar=0; iVar<VarList_.size(); iVar++)
  {
    cout<<"--- "<<VarList_[iVar]<<endl;
    
    h_sum = 0;
    
    // get the sum of all the samples to be used as expectation value of the poisson
    for(int iSample = 0; iSample < SampleList_.size(); iSample++)
    {
      if(!FakesData && SampleList_[iSample].Contains("Fakes") ) {continue;} //Fakes from MC only
      else if(FakesData && (SampleList_[iSample].Contains("DY") || SampleList_[iSample].Contains("TT") || SampleList_[iSample].Contains("WW") ) ) {continue;} //Fakes from data only
      
      h_tmp = 0;
      TString histo_name = "Control_" + channel + "_" + VarList_[iVar] + "_" + SampleList_[iSample];
      if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : ERROR"<<endl; continue;}
      h_tmp = (TH1F*) file->Get(histo_name.Data())->Clone();
      if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
      else {h_sum->Add(h_tmp);}
    }
    
    int nofbins = h_sum->GetNbinsX();
    
    for(int i=0; i<nofbins; i++)
    {
      int bin_content = h_sum->GetBinContent(i+1); cout<<"Initial content = "<<bin_content<<endl;
      int new_bin_content = therand.Poisson(bin_content); cout<<"New content = "<<new_bin_content<<endl;
      h_sum->SetBinContent(i+1, new_bin_content);
    }
    
    file->cd();
    TString output_histo_name = "Control_" + channel + "_" + VarList_[iVar] + "_Data";
    h_sum->Write(output_histo_name, TObject::kOverwrite);
  }
  
  file->Close();
  
  cout<<"--- Done with generation of pseudo-data for CR"<<endl;
}


void MVAworker::PseudoDataBDT(TString channel)
{
  
  TRandom3 therand(0); //Randomization
  
  TString pseudodata_input_name = "outputs/Reader" + appliedcuts + ".root";
  TFile* file = TFile::Open( pseudodata_input_name.Data(), "UPDATE");
  cout<<endl<<"--- GENERATION OF PSEUDODATA IN "<<file->GetName()<<" ! ---"<<endl<<endl;
  
  TH1F *h_sum = 0, *h_tmp = 0;
  
  //file_input->ls(); //output the content of the file
  
  for(int iSample = 0; iSample < SampleList_.size(); iSample++)
  {
    if(SampleList_[iSample].Contains("Data") || SampleList_[iSample].Contains("WW") || SampleList_[iSample].Contains("TT") || SampleList_[iSample].Contains("DY") || SampleList_[iSample].Contains("Fakes")) {continue;} //Fakes are stored under special names, see below
    
    h_tmp = 0;
    TString histo_name = "BDT_" + channel + "__" + SampleList_[iSample];
    if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : problem"<<endl; continue;}
    h_tmp = (TH1F*) file->Get(histo_name.Data())->Clone();
    if(h_sum == 0) {h_sum = (TH1F*) h_tmp->Clone();}
    else {h_sum->Add(h_tmp);}
  }
  
  /*
  //If find "fake template"
  TString template_fake_name = "";
  if(channel == "uuu" || channel == "eeu") {template_fake_name = "FakeMu";}
  else {template_fake_name = "FakeEl";}
  h_tmp = 0;
  TString histo_name = "BDT_" + channel + "__" + template_fake_name;
  if(!file->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found (probably bc fakes are not used)"<<endl;}
  else
  {
    h_tmp = (TH1F*) file->Get(histo_name.Data())->Clone();
    h_sum->Add(h_tmp);
  }
  */
  int nofbins = h_sum->GetNbinsX();
  
  for(int i=0; i<nofbins; i++)
  {
    int bin_content = h_sum->GetBinContent(i+1); //cout<<"initial content = "<<bin_content<<endl;
    int new_bin_content = therand.Poisson(bin_content); //cout<<"new content = "<<new_bin_content<<endl;
    h_sum->SetBinContent(i+1, new_bin_content);
  }
  
  file->cd();
  TString output_histo_name = "BDT_" + channel + "__DATA";
  h_sum->Write(output_histo_name, TObject::kOverwrite);
  
  file->Close();
 
  
  cout<<"--- Done with generation of pseudo-data"<<endl;



 }



void MVAworker::PlotBDT(TString channel)
{
  TString input_name = "outputs/Reader" + appliedcuts + ".root";
  TFile* file_input = TFile::Open( input_name.Data() );
  
  TH1F *h_sum_MC = 0, *h_tmp = 0, *h_sum_data = 0;
  
  for(int iSample = 0; iSample < SampleList_.size(); iSample++)
  {
    if(SampleList_[iSample].Contains("Data") || SampleList_[iSample].Contains("DY") || SampleList_[iSample].Contains("WW") || SampleList_[iSample].Contains("TT") || SampleList_[iSample].Contains("Fakes")) {continue;} //See below for fakes
    
    h_tmp = 0;
    TString histo_name = "BDT_" + channel + "__" + SampleList_[iSample];
    if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : ERROR"<<endl; continue;}
    h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
    if(h_sum_MC == 0) {h_sum_MC = (TH1F*) h_tmp->Clone();}
    else {h_sum_MC->Add(h_tmp);}
  }
  /*
  //If find "fake template"
  TString template_fake_name = "";
  if(channel == "uuu" || channel == "eeu") {template_fake_name = "FakeMu";}
  else {template_fake_name = "FakeEl";}
  h_tmp = 0;
  TString histo_name = "BDT_" + channel + "__" + template_fake_name;
  if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : ERROR"<<endl;}
  else
  {
    h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
    h_sum_MC->Add(h_tmp);
  }
  */
  //data
  h_tmp = 0;
  TString histo_name = "BDT_" + channel + "__DATA";
  h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
  if(h_sum_data == 0) {h_sum_data = (TH1F*) h_tmp->Clone();}
  else {h_sum_data->Add(h_tmp);}
  
  
  //Canvas definition
  Load_Canvas_Style();
  
  h_sum_MC->SetLineColor(kRed);
  
  TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
  h_sum_MC->Draw("hist");
  h_sum_data->Draw("epsame");
  TString output_plot_name = "plots/BDT_template_" + channel+ appliedcuts + ".png";
  c1->SaveAs(output_plot_name.Data());
  
  delete c1;

  
}


void MVAworker::PlotCR(TString channel, bool FakesData, bool allchannels)
{
  cout<<endl<<"#####################"<<endl;
  if(!FakesData) {cout<<"--- Using fakes from MC ---"<<endl;}
  else {cout<<"--- Using fakes from data ---"<<endl;}
  cout<<"#####################"<<endl<<endl;
  
  TString input_file_name = "outputs/Control_Histograms" + appliedcuts + ".root";
  TFile* f = TFile::Open( input_file_name );
  TH1F *h_tmp = 0, *h_data = 0;
  THStack *stack = 0;
  
  //Variable names to be displayed on plots
  TString title_MET = "Missing E_{T} [GeV]";
  
  vector<TString> thechannellist; //Need 2 channel lists to be able to plot both single channels and all channels summed
  thechannellist.push_back("uuu");
  thechannellist.push_back("uue");
  thechannellist.push_back("eeu");
  thechannellist.push_back("eee");
  //thechannellist.push_back("");
  
  //Canvas definition
  Load_Canvas_Style();
  
  //get everything out and fill vectors
  // Loop on variables
  for(unsigned int iVar = 0; iVar < VarList_.size() ; iVar++){
    //set canvars
    TCanvas* c1 = new TCanvas("c1","c1", 1000, 800);
    c1->SetBottomMargin(0.3);
    
    // reset pointers
    h_data = 0; stack = 0;
    vector<TH1F*> v_MC_histo;
    //Idem for each systematics
    vector<TH1F*> v_MC_histo_JER_plus; vector<TH1F*> v_MC_histo_JER_minus;
    vector<TH1F*> v_MC_histo_JES_plus; vector<TH1F*> v_MC_histo_JES_minus;
    vector<TH1F*> v_MC_histo_PU_plus; vector<TH1F*> v_MC_histo_PU_minus;
    vector<TH1F*> v_MC_histo_Q2_plus; vector<TH1F*> v_MC_histo_Q2_minus;
    vector<TH1F*> v_MC_histo_MuEff_plus; vector<TH1F*> v_MC_histo_MuEff_minus;
    vector<TH1F*> v_MC_histo_EleEff_plus; vector<TH1F*> v_MC_histo_EleEff_minus;
    
    // set legend
    TLegend* qw = 0;
    qw = new TLegend(.80,.60,.95,.90);
    qw->SetShadowColor(0);
    qw->SetFillColor(0);
    qw->SetLineColor(0);
    
    vector<double> v_eyl, v_eyh, v_exl, v_exh, v_x, v_y; //Contain the systematic errors
    //for cloning
    int niterChan = 0;
    
    // channel loop if not all channels to be summed
    for(unsigned int iChan = 0; iChan < ChanList_.size() ; iChan++)
    {
      if(!allchannels && channel != ChanList_[iChan]) continue;
      
      // loop over samples
      for(unsigned int iSample = 0 ; iSample < SampleList_.size() ; iSample++)
      {
        // loop over systematics
        for(unsigned int iSys = 0; iSys < SystList_.size(); iSys ++)
        {
          // no systematics for data or fakes
          bool isData = SampleList_[iSample].Contains("Data");
          if((isData || SampleList_[iSample].Contains("Fakes")) && SystList_[iSys] != "") {continue;}
          if(verbose >1) cout<<ChanList_[iChan]<<" / "<<SampleList_[iSample]<<" / "<<SystList_[iSys]<<endl;
          
          // reset tmp histo
          h_tmp = 0;
          
          // get histo
          TString histo_name = "Control_" + ChanList_[iChan] + "_"+ VarList_[iVar] + "_" + SampleList_[iSample];
          if(SystList_[iSys] != "") {histo_name+= "_" + SystList_[iSys];}
          if(!f->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : ERROR"<<endl; continue;}
          
          h_tmp = (TH1F*) f->Get(histo_name.Data())->Clone();
          
          // add all data and do nothing else with it
          if(isData)
          {
            if(h_data == 0) {h_data = (TH1F*) h_tmp->Clone();}
            else {h_data->Add(h_tmp);}
            continue;
          }
          //SINCE WE USE ONLY ONE SampleList_ FOR DATA AND MC, IT MAKES A DIFFERENCE WHETHER DATA SAMPLE IS ACTIVATED OR NOT
          //if indeed we also run on data, then for the following samples (= MC) we need to do iterator-= 1 in order to start the MC-dedicated vector at 0 !
          if(SampleList_[0].Contains("Data")) {iSample = iSample - 1;} //If the SampleList_ contains the data

          // making things pretty
          h_tmp->SetFillStyle(1001);
          h_tmp->SetFillColor(ColorList_[iSample]);
          h_tmp->SetLineColor(ColorList_[iSample]);
          
          if(!isData && SystList_[iSys] == "" && niterChan==0)
          {
            v_MC_histo.push_back(h_tmp);
            
          }
          else if(!isData && SystList_[iSys] == "") //niterChan != 0
          {
            v_MC_histo[iSample]->Add(h_tmp);
          }
          
          else if(!isData && niterChan==0) // SystList_[iSys] != ""
          {
            if(SystList_[iSys] == "JER__plus") {v_MC_histo_JER_plus.push_back(h_tmp);}
            else if(SystList_[iSys] == "JER__minus") {v_MC_histo_JER_minus.push_back(h_tmp);}
            else if(SystList_[iSys] == "JES__plus") {v_MC_histo_JES_plus.push_back(h_tmp);}
            else if(SystList_[iSys] == "JES__minus") {v_MC_histo_JES_minus.push_back(h_tmp);}
            else if(SystList_[iSys] == "PU__plus") {v_MC_histo_PU_plus.push_back(h_tmp);}
            else if(SystList_[iSys] == "PU__minus") {v_MC_histo_PU_minus.push_back(h_tmp);}
            else if(SystList_[iSys] == "Q2__plus") {v_MC_histo_Q2_plus.push_back(h_tmp);}
            else if(SystList_[iSys] == "Q2__minus") {v_MC_histo_Q2_minus.push_back(h_tmp);}
            else if(SystList_[iSys] == "MuEff__plus") {v_MC_histo_MuEff_plus.push_back(h_tmp);}
            else if(SystList_[iSys] == "MuEff__minus") {v_MC_histo_MuEff_minus.push_back(h_tmp);}
            else if(SystList_[iSys] == "EleEff__plus") {v_MC_histo_EleEff_plus.push_back(h_tmp);}
            else if(SystList_[iSys] == "EleEff__minus") {v_MC_histo_EleEff_minus.push_back(h_tmp);}
            else if(SystList_[iSys] != "") {cout<<"Unknow systematic name"<<endl;}
          }
          else if(!isData)// SystList_[iSys] != "" && niterChan != 0
          {
            if(SystList_[iSys] == "JER__plus") {v_MC_histo_JER_plus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "JER__minus") {v_MC_histo_JER_minus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "JES__plus") {v_MC_histo_JES_plus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "JES__minus") {v_MC_histo_JES_minus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "PU__plus") {v_MC_histo_PU_plus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "PU__minus") {v_MC_histo_PU_minus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "Q2__plus") {v_MC_histo_Q2_plus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "Q2__minus") {v_MC_histo_Q2_minus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "MuEff__plus") {v_MC_histo_MuEff_plus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "MuEff__minus") {v_MC_histo_MuEff_minus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "EleEff__plus") {v_MC_histo_EleEff_plus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] == "EleEff__minus") {v_MC_histo_EleEff_minus[iSample]->Add(h_tmp);}
            else if(SystList_[iSys] != "") {cout<<"Unknow systematic name"<<endl;}
            
          }
          
          if(SampleList_[0].Contains("Data")) {iSample = iSample + 1;} //If we have de-incremented iSample (cf. above), then we need to re-increment it here so we don't get an infinite loop !!
          
        } // sys loop
      } // sample loop
      niterChan++;
    }// channel loop
    
    // Stack MC for this iVar
    for(int i=0; i<v_MC_histo.size(); i++)
    {
      if(stack == 0) {stack = new THStack; stack->Add(v_MC_histo[i]);}
      else {stack->Add(v_MC_histo[i]);}
      
      //NB : use (i+1) because SampleList_ also contains the data sample in first position
      if(SampleList_[i+1] != "WW" && SampleList_[i+1] != "ZZ") {qw->AddEntry(v_MC_histo[i], SampleList_[i+1].Data() , "f");} //Need to keep ordered so that SampleList_[i] <-> vector_MC[i]
      else if(SampleList_[i+1] == "ZZ") {qw->AddEntry(v_MC_histo[i], "VV" , "f");}
    }
    
    if(!stack) {cout<<__LINE__<<" : stack is null"<<endl;} // if empty pointer
    
    // Draw stack options
    c1->cd();
    
    //Set Yaxis maximum
    if(h_data != 0 && stack != 0)
    {
      if(h_data->GetMaximum() > stack->GetMaximum() ) {stack->SetMaximum(h_data->GetMaximum()+0.3*h_data->GetMaximum());}
      else stack->SetMaximum(stack->GetMaximum()+0.3*stack->GetMaximum());
    }
    
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
    else {cout<<__LINE__<<" : h_data is null"<<endl;}
    
    
    if(verbose>1) cout << "MVAworker:: start with systematics " << endl;
    //--------------------------
    //MC SYSTEMATICS PLOT
    //--------------------------
    //Create a temporary TH1F* in order to put all the systematics into it via SetBinError, then create a TGraphError from this TH1F*
    //Also used to compute the Data/MC ratio
    TH1F* histo_syst_MC = 0;
    TH1F* histo_syst_MC_JER_plus = 0;  TH1F* histo_syst_MC_JER_minus = 0;
    TH1F* histo_syst_MC_JES_plus = 0;  TH1F* histo_syst_MC_JES_minus = 0;
    TH1F* histo_syst_MC_PU_plus = 0;  TH1F* histo_syst_MC_PU_minus = 0;
    TH1F* histo_syst_MC_Q2_plus = 0;  TH1F* histo_syst_MC_Q2_minus = 0;
    TH1F* histo_syst_MC_MuEff_plus = 0;  TH1F* histo_syst_MC_MuEff_minus = 0;
    TH1F* histo_syst_MC_EleEff_plus = 0;  TH1F* histo_syst_MC_EleEff_minus = 0;
    for(unsigned int imc=0; imc < v_MC_histo.size(); imc++) //Clone or Add histograms
    {
      if(histo_syst_MC == 0) {histo_syst_MC = (TH1F*) v_MC_histo[imc]->Clone();}
      else {histo_syst_MC->Add(v_MC_histo[imc]);}
      
      //cout<<"v_MC_histo[imc]->GetBinContent(1) = "<<v_MC_histo[imc]->GetBinContent(1)<<endl;
      //cout<<"histo_syst_MC->GetBinContent(1) = "<<histo_syst_MC->GetBinContent(1)<<endl;
      
      if(v_MC_histo_JER_plus.size() == v_MC_histo.size()) //If the syst. is taken into account, then both vectors should have same size
      {
        if(histo_syst_MC_JER_plus == 0) {histo_syst_MC_JER_plus = (TH1F*) v_MC_histo_JER_plus[imc]->Clone();}
        else {histo_syst_MC_JER_plus->Add(v_MC_histo_JER_plus[imc]);}
        if(histo_syst_MC_JER_minus == 0) {histo_syst_MC_JER_minus = (TH1F*) v_MC_histo_JER_minus[imc]->Clone();}
        else {histo_syst_MC_JER_minus->Add(v_MC_histo_JER_minus[imc]);}
      }
      
      if(v_MC_histo_JES_plus.size() == v_MC_histo.size())
      {
        if(histo_syst_MC_JES_plus == 0) {histo_syst_MC_JES_plus = (TH1F*) v_MC_histo_JES_plus[imc]->Clone();}
        else {histo_syst_MC_JES_plus->Add(v_MC_histo_JES_plus[imc]);}
        if(histo_syst_MC_JES_minus == 0) {histo_syst_MC_JES_minus = (TH1F*) v_MC_histo_JES_minus[imc]->Clone();}
        else {histo_syst_MC_JES_minus->Add(v_MC_histo_JES_minus[imc]);}
      }
      if(v_MC_histo_PU_plus.size() == v_MC_histo.size())
      {
        if(histo_syst_MC_PU_plus == 0) {histo_syst_MC_PU_plus = (TH1F*) v_MC_histo_PU_plus[imc]->Clone();}
        else {histo_syst_MC_PU_plus->Add(v_MC_histo_PU_plus[imc]);}
        if(histo_syst_MC_PU_minus == 0) {histo_syst_MC_PU_minus = (TH1F*) v_MC_histo_PU_minus[imc]->Clone();}
        else {histo_syst_MC_PU_minus->Add(v_MC_histo_PU_minus[imc]);}
      }
      if(v_MC_histo_Q2_plus.size() == v_MC_histo.size())
      {
        if(histo_syst_MC_Q2_plus == 0) {histo_syst_MC_Q2_plus = (TH1F*) v_MC_histo_Q2_plus[imc]->Clone();}
        else {histo_syst_MC_Q2_plus->Add(v_MC_histo_Q2_plus[imc]);}
        if(histo_syst_MC_Q2_minus == 0) {histo_syst_MC_Q2_minus = (TH1F*) v_MC_histo_Q2_minus[imc]->Clone();}
        else {histo_syst_MC_Q2_minus->Add(v_MC_histo_Q2_minus[imc]);}
      }
      if(v_MC_histo_MuEff_plus.size() == v_MC_histo.size())
      {
        if(histo_syst_MC_MuEff_plus == 0) {histo_syst_MC_MuEff_plus = (TH1F*) v_MC_histo_MuEff_plus[imc]->Clone();}
        else {histo_syst_MC_MuEff_plus->Add(v_MC_histo_MuEff_plus[imc]);}
        if(histo_syst_MC_MuEff_minus == 0) {histo_syst_MC_MuEff_minus = (TH1F*) v_MC_histo_MuEff_minus[imc]->Clone();}
        else {histo_syst_MC_MuEff_minus->Add(v_MC_histo_MuEff_minus[imc]);}
      }
      if(v_MC_histo_EleEff_plus.size() == v_MC_histo.size())
      {
        if(histo_syst_MC_EleEff_plus == 0) {histo_syst_MC_EleEff_plus = (TH1F*) v_MC_histo_EleEff_plus[imc]->Clone();}
        else {histo_syst_MC_EleEff_plus->Add(v_MC_histo_EleEff_plus[imc]);}
        if(histo_syst_MC_EleEff_minus == 0) {histo_syst_MC_EleEff_minus = (TH1F*) v_MC_histo_EleEff_minus[imc]->Clone();}
        else {histo_syst_MC_EleEff_minus->Add(v_MC_histo_EleEff_minus[imc]);}
      }
    }
    int nofbin = histo_syst_MC->GetNbinsX();
    
    //Add up here the different errors (quadratically), for each bin separately
    // assumed that the errors are uncorrelated
    for(int ibin=1; ibin<nofbin+1; ibin++) //Start at bin 1
    {
      double err_up = 0;
      double err_low = 0;
      double tmp = 0;
      
      //For each systematic, compute (shifted-nominal), check the sign, and add quadratically to the corresponding error
      //--------------------------
      
      //JER
      if(histo_syst_MC_JER_plus != 0)
      {
        tmp = histo_syst_MC_JER_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
        tmp = histo_syst_MC_JER_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
      }
      //JES
      if(histo_syst_MC_JES_plus != 0)
      {
        tmp = histo_syst_MC_JES_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
        tmp = histo_syst_MC_JES_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
      }
      //PU
      if(histo_syst_MC_PU_plus != 0)
      {
        tmp = histo_syst_MC_PU_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
        tmp = histo_syst_MC_PU_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
      }
      //Q2 - Scale
      if(histo_syst_MC_Q2_plus != 0)
      {
        tmp = histo_syst_MC_Q2_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
        tmp = histo_syst_MC_Q2_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
      }
      //Muon Efficiency SF
      if(histo_syst_MC_MuEff_plus != 0)
      {
        tmp = histo_syst_MC_MuEff_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
        tmp = histo_syst_MC_MuEff_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
      }
      //Electron Efficiency SF
      if(histo_syst_MC_EleEff_plus != 0)
      {
        tmp = histo_syst_MC_EleEff_plus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
        tmp = histo_syst_MC_EleEff_minus->GetBinContent(ibin) - histo_syst_MC->GetBinContent(ibin);
        if(tmp>0) {tmp = pow(tmp,2); err_up+= tmp;}
        else if(tmp<0) {tmp = pow(tmp,2); err_low+= tmp;}
      }
      //Luminosity (set to 4% of bin content)
      err_up+= pow(histo_syst_MC->GetBinContent(ibin)*0.04, 2);
      err_low+= pow(histo_syst_MC->GetBinContent(ibin)*0.04, 2);
      //MC Statistical uncertainty
      err_up+= 	pow(histo_syst_MC->GetBinError(ibin), 2);
      err_low+= 	pow(histo_syst_MC->GetBinError(ibin), 2);
      
      if(verbose>1) cout<<"histo_syst_MC->GetBinError(ibin) = "<<histo_syst_MC->GetBinError(ibin)<<endl;
      
      //--------------------------
      //Take sqrt
      err_up = pow(err_up, 0.5); //cout<<"err_up = "<<err_up<<endl;
      err_low = pow(err_low, 0.5); //cout<<"err_low = "<<err_low<<endl;
      
      //Fill error vectors (one per bin)
      v_eyh.push_back(err_up);
      v_eyl.push_back(err_low);
      v_exl.push_back(histo_syst_MC->GetXaxis()->GetBinWidth(ibin) / 2);
      v_exh.push_back(histo_syst_MC->GetXaxis()->GetBinWidth(ibin) / 2);
      v_x.push_back( (histo_syst_MC->GetXaxis()->GetBinLowEdge(nofbin+1) - histo_syst_MC->GetXaxis()->GetBinLowEdge(1) ) * ((ibin - 0.5)/nofbin) + histo_syst_MC->GetXaxis()->GetBinLowEdge(1));
      v_y.push_back(histo_syst_MC->GetBinContent(ibin)); //see warning above about THStack and negative weights
      
      //if(ibin > 1) {continue;} //display only first bin
      //cout<<"x = "<<v_x[ibin-1]<<endl;    cout<<", y = "<<v_y[ibin-1]<<endl;    cout<<", eyl = "<<v_eyl[ibin-1]<<endl;    cout<<", eyh = "<<v_eyh[ibin-1]<<endl; cout<<", exl = "<<v_exl[ibin-1]<<endl;    cout<<", exh = "<<v_exh[ibin-1]<<endl;
    }
    
    //Pointers to vectors : need to give the adress of first element (all other elements then can be accessed iteratively)
    double* eyl = &v_eyl[0];
    double* eyh = &v_eyh[0];
    double* exl = &v_exl[0];
    double* exh = &v_exh[0];
    double* x = &v_x[0];
    double* y = &v_y[0];
    
    //Create TGraphAsymmErrors with the error vectors / (x,y) coordinates
    TGraphAsymmErrors* gr = 0;
    gr = new TGraphAsymmErrors(nofbin,x,y,exl,exh,eyl,eyh);
    gr->SetFillStyle(3005);
    gr->SetFillColor(1);
    gr->Draw("e2 same"); //Superimposes the systematics uncertainties on stack
    //cout << __LINE__ << endl;
    
    
    //-------------------
    //LEGEND AND CAPTIONS
    //-------------------
    
    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextSize(0.04);
    latex->SetTextAlign(31);
    latex->DrawLatex(0.45, 0.95, "CMS Preliminary");
    
    // cout << __LINE__ << endl;
    
    TLatex* latex2 = new TLatex();
    latex2->SetNDC();
    latex2->SetTextSize(0.04);
    latex2->SetTextAlign(31);
    //------------------
    int lumi = 2.26 * luminosity_rescale;
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
    //DATA OVER BACKGROUND RATIO
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
      
      histo_ratio_data->Divide(histo_syst_MC);
      
      //if(VarList_[iVar] == "METpt")   histo_ratio_data->GetXaxis()->SetTitle(title_MET.Data());
      histo_ratio_data->GetXaxis()->SetTitle(VarList_[iVar].Data());
      
      histo_ratio_data->SetMinimum(0.0);
      histo_ratio_data->SetMaximum(2.0);
      histo_ratio_data->GetXaxis()->SetTitleOffset(1.2);
      histo_ratio_data->GetXaxis()->SetLabelSize(0.04);
      histo_ratio_data->GetYaxis()->SetLabelSize(0.03);
      histo_ratio_data->GetYaxis()->SetNdivisions(6);
      histo_ratio_data->GetYaxis()->SetTitleSize(0.03);
      histo_ratio_data->Draw("E1X0");
      
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
    //TString outputname = "plots"+filename_suffix+"/"+VarList_[iVar]+"_"+channel+".png";
    TString outputname = "plots/"+VarList_[iVar]+"_"+channel+".png";
    if(channel == "" || allchannels) {outputname = "plots/"+VarList_[iVar]+"_all.png";}
    
    //cout << __LINE__ << endl;
    if(c1!= 0) {c1->SaveAs(outputname.Data() );}
    //cout << __LINE__ << endl;
    
    delete c1; //Must free dinamically-allocated memory
    
  } // variable loop

}

void MVAworker::PlotBDTall()
{
  TString input_name = "outputs/Reader" + appliedcuts + ".root";
  TFile* file_input = TFile::Open( input_name.Data() );
  
  // definen empty pointers
  TH1F *h_sum_MC = 0, *h_tmp = 0, *h_sum_data = 0;
  
  for(int ichan=0; ichan<ChanList_.size(); ichan++)
  {
    // MC samples
    for(int iSample = 0; iSample < SampleList_.size(); iSample++)
    {
      if(SampleList_[iSample].Contains("Data") || SampleList_[iSample].Contains("DY") || SampleList_[iSample].Contains("WW") || SampleList_[iSample].Contains("TT") || SampleList_[iSample].Contains("Fakes")) {continue;} //See below for fakes
      
      // reset the tmp histo
      h_tmp = 0;
      TString histo_name = "BDT_" + ChanList_[ichan] + "__" + SampleList_[iSample];
      if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cerr<<histo_name<<" : ERROR"<<endl; continue;}
      h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
      if(h_sum_MC == 0) {h_sum_MC = (TH1F*) h_tmp->Clone();}
      else {h_sum_MC->Add(h_tmp);}
    }
    
    //If find "fake template"
    //TString template_fake_name = "";
    //if(ChanList_[ichan] == "uuu" || ChanList_[ichan] == "eeu") {template_fake_name = "FakeMu";}
    //else {template_fake_name = "FakeEl";}
    //h_tmp = 0;
    //TString histo_name = "BDT_" + ChanList_[ichan] + "__" + template_fake_name;
    //if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cout<<histo_name<<" : not found"<<endl;}
    //else
    //{
    //  h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
    //  h_sum_MC->Add(h_tmp);
    //}
    
    // Get data
    h_tmp = 0;
    TString histo_name = "BDT_" + ChanList_[ichan] + "__DATA";
    if(!file_input->GetListOfKeys()->Contains(histo_name.Data())) {cerr<<histo_name<<" : ERROR"<<endl; continue;}
    h_tmp = (TH1F*) file_input->Get(histo_name.Data())->Clone();
    if(h_sum_data == 0) {h_sum_data = (TH1F*) h_tmp->Clone();}
    else {h_sum_data->Add(h_tmp);}
  }
  
  //Canvas definition
  Load_Canvas_Style();
  
  TCanvas* c1 = new TCanvas("c1","BDT all channels", 1000, 800);
  gStyle->SetOptStat(0);
  h_sum_MC->GetXaxis()->SetTitle("BDT");
  h_sum_MC->SetLineColor(kBlue);
  h_sum_data->SetLineColor(kGreen);
  h_sum_MC->Draw("hist");
  h_sum_data->Draw("epsame");
  TLegend* leg = new TLegend(0.7,0.75,0.88,0.85);
  leg->SetHeader("");
  leg->AddEntry("h_sum_data","Data","L");
  leg->AddEntry("h_sum_MC","MC","L");
  leg->Draw();

  std::string pathPNG = "plots";
  mkdir(pathPNG.c_str(),0777);
  TString output_plot_name = "plots/BDT_template_all" + appliedcuts + ".png";
  c1->SaveAs(output_plot_name.Data());
  
  delete leg;
  delete c1;
  
}