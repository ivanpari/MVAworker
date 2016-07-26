//
//  MVAworker.hpp
//  
//
//  Created by Isis Van Parijs on 25/07/16.
//
//

#ifndef MVAworker_h
#define MVAworker_h



#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2F.h>
#include <TH2.h>
#include <TH1F.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TString.h>
#include <TLorentzVector.h>
#include "TTree.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TString.h"
#include "TColor.h"
#include "TStopwatch.h"
#include "TCut.h"

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <cmath>
#include<sstream>

#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TMVA/TMVAGui.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"

class MVAworker
{
public:
  //methods
  MVAworker();
  MVAworker(std::vector <TString>, std::vector <TString>,std::vector <TString>, std::vector <int> , std::vector<TString>, std::vector<TString>, bool, bool);
  ~MVAworker(){delete MVAreader;};
  
  void TrainTestEvaluate(TString,bool);
  void Evaluate(TString,bool);
  void Read(TString, bool, bool); // input from trainer
  float DetermineCR(); // input from reader
  void ControlTrees(bool, bool,double); // input from determine cr and reader
  void ControlHisto(TString, bool);  // input from control trees
  void PseudoDataCR(TString, bool); // input from control histo
  void PseudoDataBDT(TString); // input from reader
  void PlotBDTall(); // input from control histo and pseudo data
  void PlotBDT(TString); // input from control histo and pseudo data
  void PlotCR(TString, bool, bool); // input from plot bdt
  
  // settings
  void Set_Luminosity(double); //Set the luminosity re-scaling factor to be used thoughout the code
  void SetCuts(TString, TString, TString, TString); //Set the cut values
  
  //members
  TMVA::Reader *MVAreader;
  
  std::vector <TString> SampleList_;
  std::vector <TString> ChanList_;
  std::vector <TString> VarList_;
  std::vector <float> vVar;
  std::vector <TString> cutVarList_;
  std::vector <float> vcutVar;
  std::vector <int> ColorList_;
  std::vector <TString> SystList_; 
  int verbose = 2;
  double luminosity_rescale; 
  int nBinsBDT = 10; 
  TString appliedcuts;
  float METpt; float mTW; float NJets; float NBJets;
  
  TString cut_MET;
  TString cut_mTW;
  TString cut_NJets;
  TString cut_NBJets;
  
  bool stopProg; 
  
  
};



#endif