//by Nicolas (inspired from Jeremy's code)

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
	theMVAtool(std::vector<TString >, std::vector<TString >, std::vector<TString>, std::vector<TString>, std::vector<int>, std::vector<TString>, std::vector<TString>, std::vector<bool>, int);
	~theMVAtool(){delete reader;};

	void Set_Luminosity(double); //Set the luminosity re-scaling factor to be used thoughout the code
	void Train_Test_Evaluate(TString, TString); //Train, Test, Evaluate BDT with MC samples
	float Compute_Fake_Ratio(TString, bool); //Computes ratio of fakes in MC compared to data, to re-scale mTW template of fakes from data in Read()
	int Read(TString, bool, bool, bool); //Produce templates of BDT, mTW (or else ?)
	float Determine_Control_Cut(); //Determine at which discriminant value the cut should be applied, in order to keep mainly bkg
	void Create_Control_Trees(bool, bool, double, bool); //Create new trees with events passing the cuts
	void Create_Control_Histograms(bool, bool); //Use the trees created with Create_Control_Trees to create histograms in same file
	int Generate_PseudoData_Histograms_For_Control_Plots(bool); //Idem, for replacing data and be able to plot control plots
	int Generate_PseudoData_Histograms_For_Templates(TString,TString); //Generate pseudo-data from templates -> can simulate template fit without looking at real data
	int Draw_Control_Plots(TString, bool, bool); //Draw control plots from the histograms obtained with Create_Control_Histograms()
  int Draw_ControlttZ_Plots(TString, bool, bool);
  int Draw_Control_Plots_isis(TString, bool, bool); //Draw control plots from the histograms obtained with Create_Control_Histograms()
	int Plot_Templates(TString, TString, bool);
  int Draw_Control_Templates(TString , bool , bool , TString );
  void SetPermutations(TString, TString);

  

//Members
	TMVA::Reader *reader;

	std::vector<TString> sample_list;
	std::vector<TString> data_list;
	std::vector<TString> var_list; std::vector<float> vec_variables; //Contains as many floats as there are variables in var_list
	std::vector<TString> syst_list;
	std::vector<TString> channel_list;
	std::vector<TString> v_cut_name; std::vector<TString> v_cut_def; std::vector<float> v_cut_float; std::vector<bool> v_cut_IsUsedForBDT;
	std::vector<int> colorVector;

	int nbin; //Control number of bins in BDT histogram
	double luminosity_rescale;

	TString filename_suffix;

	bool stop_program;
};

#endif
