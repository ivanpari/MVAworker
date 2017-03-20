

#ifndef theMVAtool_h
#define theMVAtool_h

/* BASH COLORS */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST
#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

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

class theMVAtool
{

public :

//Methods
	theMVAtool();
  theMVAtool(std::vector<TString >, std::vector<TString >, std::vector<TString>, std::vector<TString>, std::vector<TString>, std::vector<TString>, std::vector<bool>, int, std::string, TString);
	~theMVAtool(){delete reader;};

	
	void Train_Test_Evaluate(TString, TString); //Train, Test, Evaluate BDT with MC samples
	void Read(TString); //Produce templates of BDT, mTW (or else ?)
  void PSDataCreator(TString, TString, TString);
  Double_t Determine_Control_Cut(TString, TString, TString);
 
//Members
	TMVA::Reader *reader;

  TString placeOutputTraining;
  TString placeOfWeights; 
  TString placeOutputReading;
	std::vector<TString> sample_list;
  std::vector<TString> sample_listread;
	std::vector<TString> data_list;

	std::vector<TString> var_list; std::vector<float> vec_variables; //Contains as many floats as there are variables in var_list
	std::vector<TString> channel_list;
	std::vector<TString> v_cut_name; std::vector<TString> v_cut_def; std::vector<float> v_cut_float; std::vector<bool> v_cut_IsUsedForBDT;
  std::string PlaceOfTuples;
  TString region_name;
TString filename_suffix;

	int nbin; //Control number of bins in BDT histogram
	double luminosity_rescale;

	

	bool stop_program;
};

#endif
