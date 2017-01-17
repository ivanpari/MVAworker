//
//  BDT_FCNC.cpp
//  
//
//  Created by Isis Van Parijs on 25/07/16.
//
//

#include "BDT_FCNC.h"
#include "MVAworker.h"


using namespace std;

int main(int argc, char* argv[])
{
  
  int verbose = 2;
  
  // What are you doing?
  bool doTraining = false;
  bool doReading = false;
  bool doCR = false;
  bool doCRtree = false;
  double cutValue = -999;
  bool doBDTplotAll = false;
  bool doBDTplot = false;
  bool doCRhisto = false;
  bool doPSdataCR = false;
  bool doPSdataBDT = false;
  bool RealDataTemplate = false;
  bool doBDTdraw = false;
  bool doBDTdrawAll = false;
  bool OpenGui = false;
  bool addNonTraining = false;
  
  if(verbose > 2)
  {
    cout << " The list of arguments are: " << endl;
    for (int n_arg=1; n_arg<argc; n_arg++)
    {
      std::cout << "  - arg number " << n_arg << " is " << argv[n_arg] << std::endl;
    }
  }
  for(int iarg = 0; iarg < argc && argc>1 ; iarg++){
    std::string argval=argv[iarg];
    if(argval=="--help" || argval =="--h"){
      cout << "--Train" << endl;
      cout << "--Read" << endl;
      cout << "--CRcut: needs input from reading" << endl;
      cout << "--CRtree: needs input from CRcut and Read" << endl;
      cout << "--CRhisto: needs input from CRcut and Read" << endl;
      cout << "--PsDataCR: needs input from CRhisto" << endl;
      cout << "--PsDataBDT: needs input from Read" << endl;
      cout << "--PlotBDTall" << endl;
      cout << "--PlotBDT" << endl;
      cout << "--DrawBDTall: needs input from CRhisto" << endl;
      cout << "--DrawBDT: needs input from CRhisto" << endl;
      cout << "--RealDataTemplate" << endl;
      cout << "--OpenGui" << endl;
      cout << "--allReadSamples" << endl;
      return 0;
    }
    else if(argval == "--Train"){ doTraining = true; doReading = false; doCR = false; doCRtree = false; }
    else if(argval == "--Read"){ doTraining = false; doReading = true; doCR = false; doCRtree = false;}
    else if(argval == "--CRcut"){ doTraining = false; doReading = false; doCR = true; doCRtree = false;}
    else if(argval == "--CRtree"){ doTraining = false; doReading = false; doCR = false; doCRtree = true; iarg++;  cutValue = std::stod(argv[iarg]); }
    else if(argval == "--CRhisto"){ doTraining = false; doReading = false; doCR = false; doCRtree = false; doCRhisto = true; }
    else if(argval == "--PlotBDTall"){doBDTplotAll = true; }
    else if(argval == "--PlotBDT"){doBDTplot = true; }
    else if(argval == "--DrawBDTall"){doBDTdrawAll = true; }
    else if(argval == "--DrawBDT"){doBDTdraw = true; }
    else if(argval == "--PsDataCR"){doPSdataCR = true; }
    else if(argval == "--PsDataBDT"){doPSdataBDT = true;}
    else if(argval == "--RealDataTemplate"){ RealDataTemplate = true;}
    else if(argval == "--OpenGui"){OpenGui = true; }
    else if(argval == "--allReadSamples"){addNonTraining = true;}
    else if(argval == ""){cerr << "ERROR no arguments given" << endl; return 0; }
  }

  
  
  
  ///// Lists
  std::vector<TString> SampleList;
  std::vector<TString> VarList;
  std::vector<TString> ChanList;
  std::vector<int> ColorList;
  std::vector<TString> cutVarList;
  std::vector<TString> SystList;
  
  //fill channels
  ChanList.push_back("uuu");
  ChanList.push_back("eee");
  ChanList.push_back("uue");
  ChanList.push_back("eeu");
  
  // fill datasets for data
  SampleList.push_back("Data");
  
  // fill datasets for signal
  SampleList.push_back("tZq");     ColorList.push_back(kGreen+2);
  
  //fill datasets for backgrounds
  SampleList.push_back("WZjets");  ColorList.push_back(11);
  SampleList.push_back("ZZ");      ColorList.push_back(kYellow);
  SampleList.push_back("ttZ");     ColorList.push_back(kRed);
  
  if(addNonTraining){
  // SampleList.push_back("ST_tW");         ColorList.push_back(kBlack);
  // SampleList.push_back("ST_tW_antitop"); ColorList.push_back(kBlack);
   SampleList.push_back("ttW");           ColorList.push_back(kRed+1);
  
  //FAKES
  //SampleList.push_back("Fakes");             ColorList.push_back(kAzure-2);
  //-- THESE 3 SAMPLES MUST BE THE LAST OF THE SAMPLE LIST FOR THE READER TO KNOW WHICH ARE THE MC FAKE SAMPLES !
  SampleList.push_back("DYjets");             ColorList.push_back(kAzure-2);
  SampleList.push_back("TT");                 ColorList.push_back(kRed-1);
  SampleList.push_back("WW");                 ColorList.push_back(kYellow);
  }
  
  cout << "CONSIDERED SAMPLES" <<endl;
  for(int i = 0 ; i < SampleList.size(); i++){ cout << " --" << SampleList[i] << endl;}
  
  
  // fill BDT variables
  VarList.push_back("ZCandMass"); //useless
  VarList.push_back("deltaPhilb");
  VarList.push_back("Zpt");
  VarList.push_back("ZEta");
  VarList.push_back("asym");
  VarList.push_back("btagDiscri");
  VarList.push_back("etaQ");
  VarList.push_back("AddLepPT");
  VarList.push_back("AddLepETA");
  VarList.push_back("LeadJetPT");
  VarList.push_back("LeadJetEta");
  VarList.push_back("dPhiZMET");
  VarList.push_back("dPhiZAddLep");
  VarList.push_back("dRAddLepBFromTop");
  VarList.push_back("dRZAddLep");
  VarList.push_back("ptQ"); //--not useful

  
  
  // cuts to be applied on the trees
  TString set_MET_cut = ">30";
  TString set_mTW_cut = "";
  TString set_NJets_cut = ">1"; //ONLY STRICT SIGN (> / < / ==)
  TString set_NBJets_cut = "==1"; //ONLY STRICT SIGN (> / < / ==)
  
  cutVarList.push_back("METpt");
  cutVarList.push_back("mTW");
  cutVarList.push_back("NJets");
  cutVarList.push_back("NBJets");

  
  // systematics are only used in the reader
  //-------------------
  SystList.push_back("");
  
  //Affect the variable distributions
  SystList.push_back("JER__plus");
  SystList.push_back("JER__minus");
  SystList.push_back("JES__plus");
  SystList.push_back("JES__minus");
  
  //Affect the event weight
  //SystList.push_back("Q2__plus"); //cf. Reader : not included properly in ttZ yet !
  //SystList.push_back("Q2__minus");
  SystList.push_back("PU__plus");
  SystList.push_back("PU__minus");
  SystList.push_back("MuEff__plus");
  SystList.push_back("MuEff__minus");
  SystList.push_back("EleEff__plus");
  SystList.push_back("EleEff__minus");
  
  // set lumi
  double Lumi = 2.26; // in fb^-1
  bool FakeData = false;
  bool RealData = false;
  
  // START ANALYSING  --> done by MVAworker
  // MVAworker(std::vector <TString>, std::vector <TString>,std::vector <TString>, std::vector <int> );
  MVAworker* MVAtool =new MVAworker(VarList, SampleList, ChanList, ColorList, SystList, cutVarList, FakeData, RealData);
  MVAtool->SetCuts(set_MET_cut,set_mTW_cut, set_NJets_cut, set_NBJets_cut); 
  MVAtool->Set_Luminosity(Lumi); 
  
  // Do training
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: TrainTestEvaluate" << endl;
  if(doTraining){
    for(unsigned int iChan=0; iChan < ChanList.size(); iChan++){
      cout << "BDT_FCNC: handling channel " << ChanList[iChan] << endl;
      MVAtool->TrainTestEvaluate(ChanList[iChan],OpenGui);
    }
  }
  else{
    cout << "WARNING: not doing Training, Testing and evaluating, using previously made training samples" << endl;
  }
  cout << "**************************************************" <<endl;
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: Create theta templates" << endl;
  if(doReading){
  MVAtool->Read("BDT", false, false);
  }
  else cout << "BDT_FCNC:: no reading has been preformed" << endl;
  cout << "**************************************************" <<endl;
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: do CR" << endl;
  float cutCR ;
  if(doCR){
    cutCR = MVAtool->DetermineCR();
  }
  else {cout << "BDT_FCNC:: no CR determined" << endl; cutCR = cutValue; }
  cout << "**************************************************" <<endl;
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: do CR trees" << endl;
  if(doCRtree){
    bool cutBDT = true; 
    MVAtool->ControlTrees(FakeData, cutBDT, cutCR);
    
  }
  else cout << "BDT_FCNC:: no CRtree made" << endl;
  cout << "**************************************************" <<endl;
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: Control plots for each channel in a root file" << endl;
  if(doCRhisto){
    for(unsigned int iChan = 0 ; iChan < ChanList.size() ; iChan++)
    {
      MVAtool->ControlHisto(ChanList[iChan], FakeData); 
    }
  }
  else  cout << "BDT_FCNC:: no new control plots for each channel in a root file" << endl;
  cout << "**************************************************" <<endl;
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: Pseudo data generation for CR plots " << endl;
  if(doPSdataCR){
    for(unsigned int iChan = 0 ; iChan < ChanList.size() ; iChan++)
    {
      MVAtool->PseudoDataCR(ChanList[iChan], FakeData);
    }
  }
  else  cout << "BDT_FCNC:: no Pseudo data generated for CRplots" << endl;
  cout << "**************************************************" <<endl;
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: Pseudo data generation for BDT template " << endl;
  if(doPSdataBDT && !RealDataTemplate){
    for(unsigned int iChan = 0 ; iChan < ChanList.size() ; iChan++)
    {
      MVAtool->PseudoDataBDT(ChanList[iChan]);
    }
  }
  else if(RealDataTemplate) cout << "BDT_FCNC:: no Pseudo data generated for BDT, real data used" << endl;
  else  cout << "BDT_FCNC:: no Pseudo data generated for BDT" << endl;
    cout << "**************************************************" <<endl;
  
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: Plot control plots for each channel " << endl;
  if(doBDTplot){
    for(unsigned int iChan = 0 ; iChan < ChanList.size() ; iChan++)
    {
      MVAtool->PlotBDT(ChanList[iChan]); // plot BDT of MC and PsData
    }
  }
  else cout << "BDT_FCNC:: no controlplots plotted " << endl;
  cout << "**************************************************" <<endl;
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: Draw control plots for each channel " << endl;
  if(doBDTdraw){
    for(unsigned int iChan = 0 ; iChan < ChanList.size() ; iChan++)
    {
      MVAtool->PlotCR(ChanList[iChan],FakeData, false);  // draw plots from BDT CR
    }
  }
  else cout << "BDT_FCNC:: no controlplots drawn " << endl;
  cout << "**************************************************" <<endl;
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: Plot control plots for all channels " << endl;
  if(doBDTplotAll){
    MVAtool->PlotBDTall();
  }
  else cout << "BDT_FCNC:: no controlplots plotted " << endl;
  cout << "**************************************************" <<endl;
  cout << "**************************************************" <<endl;
  cout << "BDT_FCNC:: Draw control plots for all channels " << endl;
  if(doBDTdrawAll){

    MVAtool->PlotCR("",FakeData, true);
  }
  else cout << "BDT_FCNC:: no controlplots drawn " << endl;
  cout << "**************************************************" <<endl;
  
  
  return 0;
  
}
