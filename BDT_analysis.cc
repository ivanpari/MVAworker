#include "theMVAtool.h"

using namespace std;

int main(int argc, char* argv[])
{
  //-------------------
  std::vector<TString> thesamplelist;
  std::vector<TString > thevarlist; //Variables used in BDT
  std::vector<TString > thesystlist;
  std::vector<TString > thechannellist;
  std::vector<TString > set_v_cut_name;
  std::vector<TString > set_v_cut_def;
  std::vector<bool > set_v_cut_IsUsedForBDT;
  std::vector<int> v_color; //sample <-> color
  //-------------------
  
  
  
  int verbose = 2;
  
  // What are you doing?
  bool doJetAssignment = false;
  bool doTraining = false;
  bool doReading = false;
  bool doCRcut = false;
  bool doCRtree = false;
  double cutValue = -999;
  bool doBDTplotAll = false;
  bool doBDTplot = false;
  bool doCRhisto = false;
  bool doPSdataTemplate = false;
  bool doBDTdraw = false;
  bool doBDTdrawAll = false;
  bool OpenGui = false;
  bool IncludeFakes = true;
  bool doPlotting = false;
  bool doPlottingPerChannel = false;
  TString bdt_type = "BDT"; //'BDT' or 'BDTttZ' depending on the region (for theta disambiguation) for training!
  TString template_name = "mWt"; //Either 'BDT', 'BDTttZ', 'mWt' or 'm3l'
  bool fakes_summed_channels = false; //Sum uuu/eeu & eee/uue --> Double the fake stat. ! ONLY FOR TEMPLATES NOT CR
  bool doBDTControl = false;
  
  //Used to re-scale every weights in the code by a lumi factor. (NB : default value is 2015 / 7.6.x lumi = 2.26 !)
  double set_luminosity =36;// 20; //in fb-1 lumi to be expected
  //Binning to be used for *template* production
  int nofbin_templates = 10;
  
  //Use MC fakes or data-driven fakes)
  bool fakes_from_data = false;
  
  //If true, use real data sample to create *templates* (BDT, mWt, ...) / else, use pseudodata !
  bool real_data_templates = false;
  
  //If true, creates templates for different regions & sets of cuts --> Optimization studies. Cuts can be tuned below, in the "Optimization" part.
  bool do_optimization_scan = false; //Not properly implemented in the main() yet !
  
  float cut_BDT_CR =  -999;
  bool cut_on_BDT = false;
  bool use_pseudodata_CR_plots = false;
  
  
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
      cout << "* Settings * " <<endl;
      cout << "--RealData: do not use PSdata" << endl;
      cout << "--DataDrivenFakes: do not use MC fakes" << endl;
      cout << "--SetLumi lumi: rescale the lumi to" << endl;
      cout << "--SetBins bins: set nb of bins for the template" << endl;
      cout << "--SetVerbose Verbose: set details of output statments" << endl;
      cout << "--SetBDTcut cut" << endl;
      cout << endl;
      cout << "* Methods in order of execution * " <<endl;
      cout << "--Train Template" << endl;
      cout << "--Read Template SumFakes/NoSumFakes: do reading of the trained template. Sum fakes per 2 for increase of stats. Needs input from Training" << endl;
      cout << "--PsDataTemplate Template: needs input from Read" << endl;
      cout << "--BDTcontrol Template" << endl;
      cout << "--CRcut: determine CR cut, needs input from Read" << endl;
      cout << "--CRtree CutOnBDT/NoCutOnBDT UsePSdata/NoPSdata: needs input from CRcut and Read" << endl;
      cout << "--CRhisto UsePSdata/NoPSdata: needs input from CRtree" << endl;
      cout << "--Plot All/NotAll Template" << endl;
      
      
      return 0;
    }
    else if(argval =="--Plot"){
      iarg++; argval = argv[iarg];
      if(argval =="All") doPlotting = true;
      if(argval == "NotAll") doPlottingPerChannel = true;
      iarg++;
      template_name = argv[iarg];
    }
    else if(argval == "--Train"){ doTraining = true; iarg++; bdt_type = argv[iarg]; cout << bdt_type << endl; }
    else if(argval == "--Read"){
      doReading = true;
      iarg++;
      template_name = argv[iarg];
      bdt_type = argv[iarg];
      iarg++;
      argval=argv[iarg];
      if(argval == "SumFakes") {fakes_summed_channels = true;}
      if(argval == "NoSumFakes") {fakes_summed_channels = false;}
    }
    else if(argval == "--CRcut"){ doCRcut = true; }
    else if(argval == "--CRtree"){
      doCRtree = true;
      iarg++; argval=argv[iarg];
      if(argval =="CutOnBDT") {cut_on_BDT = true;}
      else if(argval =="NoCutOnBDT"){cut_on_BDT = false;}
      iarg++; argval=argv[iarg];
      if(argval=="UsePSdata") {use_pseudodata_CR_plots = true;  }
      else if(argval=="NoPsdata") {use_pseudodata_CR_plots = false;  }
    }
    else if(argval == "--CRhisto"){
      doCRhisto = true;
      iarg++; argval=argv[iarg];
      if(argval=="UsePSdata") {use_pseudodata_CR_plots = true; fakes_from_data = false; }
      else if(argval=="NoPsdata") {use_pseudodata_CR_plots = false; fakes_from_data = true; }
    }
    else if(argval == "--PlotBDTall"){doBDTplotAll = true; }
    else if(argval == "--PlotBDT"){doBDTplot = true; }
    else if(argval == "--DrawBDTall"){doBDTdrawAll = true; }
    else if(argval == "--DrawBDT"){doBDTdraw = true; }
    else if(argval == "--RealData"){ real_data_templates = true; }
    else if(argval == "--DataDrivenFakes"){ fakes_from_data = true;}
    else if(argval == "--OpenGui"){OpenGui = true; }
    else if(argval == "--IncludeFakes"){IncludeFakes = true;}
    else if(argval == "--SetLumi"){ iarg++; set_luminosity = std::stoi(argv[iarg]); }
    else if(argval == "--SetBins"){ iarg++; nofbin_templates = std::stoi(argv[iarg]); }
    else if(argval == "--PsDataTemplate") {doPSdataTemplate = true; iarg++; template_name = argv[iarg]; bdt_type = argv[iarg];  }
    else if(argval == "--BDTcontrol") {doBDTControl = true; iarg++; template_name = argv[iarg]; bdt_type = argv[iarg];  }
    else if(argval == "--SetVerbose"){iarg++; verbose = std::stoi(argv[iarg]);}
    else if(argval == "--SetBDTcut"){iarg++; cut_BDT_CR = std::stof(argv[iarg]);}
    else if(argval == ""){cerr << "ERROR no arguments given" << endl; return 0; }
  }
  
  
  
  
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
  cout<<FYEL("---TEMPLATE --- ")<< template_name <<  endl;
  cout<<FYEL("---BDT type --- ")<< bdt_type <<  endl;
  cout<<BOLD(FYEL("##################################"))<<endl<<endl;
  
  
  
  
  //-----------------------------------------------------------------------------------------
  //                 _                       _     _
  //    ___    ___  | |_      ___    _ __   | |_  (_)   ___    _ __    ___
  //   / __|  / _ \ | __|    / _ \  | '_ \  | __| | |  / _ \  | '_ \  / __|
  //   \__ \ |  __/ | |_    | (_) | | |_) | | |_  | | | (_) | | | | | \__ \
  //   |___/  \___|  \__|    \___/  | .__/   \__| |_|  \___/  |_| |_| |___/
  //                                |_|
  //-----------------------------------------------------------------------------------------
  
  //Specify here the cuts that you wish to apply (propagated to training, reading, ...). To dis-activate a cut, just set it to "". Use "==" for equality. Don't use "||".
  //ex: set_v_cut_name.push_back("NBJets"); set_v_cut_def.push_back(">0 && <4");
  //NB : * WZ CR --> >0j & ==0bj // * ttZ CR --> >1j & >1bj // * SR ---> >0j & ==1bj //
  //-------------------
  set_v_cut_name.push_back("met_Pt");                  set_v_cut_def.push_back(">30");            set_v_cut_IsUsedForBDT.push_back(true);
  //set_v_cut_name.push_back("mWt");                    set_v_cut_def.push_back(">20");            set_v_cut_IsUsedForBDT.push_back(true);
  if(template_name.Contains("BDT") || bdt_type.Contains("BDT")){
    set_v_cut_name.push_back("nJets");                  set_v_cut_def.push_back(">1");            set_v_cut_IsUsedForBDT.push_back(false);
    set_v_cut_name.push_back("nJets_CSVL");                 set_v_cut_def.push_back(">=1");            set_v_cut_IsUsedForBDT.push_back(false);
  }
  else if(template_name.Contains("mWt") || bdt_type.Contains("mWt")){
    set_v_cut_name.push_back("nJets");                  set_v_cut_def.push_back(">0 && <3");            set_v_cut_IsUsedForBDT.push_back(false);
    set_v_cut_name.push_back("nJets_CSVL");                 set_v_cut_def.push_back("==0");            set_v_cut_IsUsedForBDT.push_back(false);
  }
  
  
  //-----------------------------------------------------------------------------------------
  //           _                                      _
  //     ___  | |__     __ _   _ __    _ __     ___  | |  ___
  //    / __| | '_ \   / _` | | '_ \  | '_ \   / _ \ | | / __|
  //   | (__  | | | | | (_| | | | | | | | | | |  __/ | | \__ \
  //    \___| |_| |_|  \__,_| |_| |_| |_| |_|  \___| |_| |___/
  //
  //-----------------------------------------------------------------------------------------
  
  //-------------------
  thechannellist.push_back("uuu");
  thechannellist.push_back("uue");
  thechannellist.push_back("eeu");
  thechannellist.push_back("eee");
  //-------------------
  
  //-----------------------------------------------------------------------------------------
  //                                      _
  //    ___    __ _   _ __ ___    _ __   | |   ___   ___
  //   / __|  / _` | | '_ ` _ \  | '_ \  | |  / _ \ / __|
  //   \__ \ | (_| | | | | | | | | |_) | | | |  __/ \__ \
  //   |___/  \__,_| |_| |_| |_| | .__/  |_|  \___| |___/
  //                             |_|
  //-----------------------------------------------------------------------------------------
  
  //-------------------
  //Sample order is important in function Read (so it knows which are the fake samples it must sum) and in Draw_Control_Plots (see explanation in code)
  //DATA --- THE DATA SAMPLE MUST BE UNIQUE AND IN FIRST POSITION
  //thesamplelist.push_back("Data");
  
  //Signal
  thesamplelist.push_back("NP_overlay_ST_FCNC_zut");             v_color.push_back(kRed+1);
  //  thesamplelist.push_back("TT_TopZkZct");             v_color.push_back(kRed+1);
  
  //BKG
  thesamplelist.push_back("tZq");             v_color.push_back(kGreen+2);
  thesamplelist.push_back("WZTo3LNu");          v_color.push_back(11);
  thesamplelist.push_back("ZZTo4L");              v_color.push_back(kYellow);
  thesamplelist.push_back("ttZ");             v_color.push_back(kRed);
  thesamplelist.push_back("STtW_top");         v_color.push_back(kBlack);
  thesamplelist.push_back("STtW_atop"); v_color.push_back(kBlack);
  thesamplelist.push_back("ttHToNonbb");           v_color.push_back(kRed+1);
  thesamplelist.push_back("tWll");                v_color.push_back(kCyan);
  thesamplelist.push_back("TTWJetsToLNu_amc");                v_color.push_back(kCyan);
  thesamplelist.push_back("tWll");                v_color.push_back(kCyan);
  thesamplelist.push_back("ZZZ_amc");                v_color.push_back(kCyan);
  
  //FAKES
  // thesamplelist.push_back("Fakes");           v_color.push_back(kAzure-2); //Data-driven (DD)
  //-- THESE 3 SAMPLES MUST BE THE LAST OF THE SAMPLE LIST FOR THE READER TO KNOW WHICH ARE THE MC FAKE SAMPLES !
    thesamplelist.push_back("Zjets50");          v_color.push_back(kAzure-2); //MC
    thesamplelist.push_back("TTJetsDilep");              v_color.push_back(kRed-1); //MC
    thesamplelist.push_back("Zjets50");              v_color.push_back(kYellow); //MC
  //-------------------
  
  //-----------------------------------------------------------------------------------------
  //    ____    ____    _____                            _           _       _
  //   | __ )  |  _ \  |_   _|   __   __   __ _   _ __  (_)   __ _  | |__   | |   ___   ___
  //   |  _ \  | | | |   | |     \ \ / /  / _` | | '__| | |  / _` | | '_ \  | |  / _ \ / __|
  //   | |_) | | |_| |   | |      \ V /  | (_| | | |    | | | (_| | | |_) | | | |  __/ \__ \
  //   |____/  |____/    |_|       \_/    \__,_| |_|    |_|  \__,_| |_.__/  |_|  \___| |___/
  //
  //-----------------------------------------------------------------------------------------
  
  //-------------------
  //NB : treat leaves/variables "Weight" and "Channel" separately
  
  
  thevarlist.push_back("FCNCtop_M");
  //  thevarlist.push_back("bdisc_jet_1");
  //  thevarlist.push_back("Wlep_M");
  thevarlist.push_back("nJets_CSVM");
  //  thevarlist.push_back("bdis_jet_2");
  //  thevarlist.push_back("asym");--> check definiation
  
  thevarlist.push_back("dPhiSMFCNCtop"); // corelated with dPhiZaddlep
  thevarlist.push_back("dRSMFCNCtop");
  // thevarlist.push_back("dRZAddLep");
  thevarlist.push_back("dRZc");
  thevarlist.push_back("dRZb");
  thevarlist.push_back("dRWlepc");
  thevarlist.push_back("dPhiWlepb"); // angle between l from W and sm b
  // thevarlist.push_back("etaQ"); // charge asymetry defined as q(W)*|etaW|,
  
  
  
  
  
  //-------------------
  
  //-----------------------------------------------------------------------------------------
  //                        _                                _     _
  //    ___   _   _   ___  | |_    ___   _ __ ___     __ _  | |_  (_)   ___   ___
  //   / __| | | | | / __| | __|  / _ \ | '_ ` _ \   / _` | | __| | |  / __| / __|
  //   \__ \ | |_| | \__ \ | |_  |  __/ | | | | | | | (_| | | |_  | | | (__  \__ \
  //   |___/  \__, | |___/  \__|  \___| |_| |_| |_|  \__,_|  \__| |_|  \___| |___/
  //          |___/
  //-----------------------------------------------------------------------------------------
  
  //-------------------
  thesystlist.push_back(""); //Always keep it activated
  
  //Affect the variable distributions
  // thesystlist.push_back("JER__plus"); thesystlist.push_back("JER__minus");
  //thesystlist.push_back("JES__plus"); thesystlist.push_back("JES__minus");
  
  //Affect the event weight
  //thesystlist.push_back("Q2__plus"); thesystlist.push_back("Q2__minus"); //NB : not included in ttZMad --> Use ttZ Madgraph for training, amcatnlo for the rest
  //thesystlist.push_back("PU__plus"); thesystlist.push_back("PU__minus");
  //thesystlist.push_back("MuEff__plus"); thesystlist.push_back("MuEff__minus");
  //thesystlist.push_back("EleEff__plus"); thesystlist.push_back("EleEff__minus");
  //B-tag syst
  /*thesystlist.push_back("btag_lf__plus"); thesystlist.push_back("btag_lf__minus");
   thesystlist.push_back("btag_hf__plus"); thesystlist.push_back("btag_hf__minus");
   thesystlist.push_back("btag_hfstats1__plus"); thesystlist.push_back("btag_hfstats1__minus");
   thesystlist.push_back("btag_lfstats1__plus"); thesystlist.push_back("btag_lfstats1__minus");
   thesystlist.push_back("btag_hfstats2__plus"); thesystlist.push_back("btag_hfstats2__minus");
   thesystlist.push_back("btag_lfstats2__plus"); thesystlist.push_back("btag_lfstats2__minus");
   thesystlist.push_back("btag_cferr1__plus"); thesystlist.push_back("btag_cferr1__minus");
   thesystlist.push_back("btag_cferr2__plus"); thesystlist.push_back("btag_cferr2__minus");*/
  
  //-------------------
  
  
  //-----------------------------------------------------------------------------------------
  //     __                          _     _                                    _   _
  //    / _|  _   _   _ __     ___  | |_  (_)   ___    _ __       ___    __ _  | | | |  ___
  //   | |_  | | | | | '_ \   / __| | __| | |  / _ \  | '_ \     / __|  / _` | | | | | / __|
  //   |  _| | |_| | | | | | | (__  | |_  | | | (_) | | | | |   | (__  | (_| | | | | | \__ \
  //   |_|    \__,_| |_| |_|  \___|  \__| |_|  \___/  |_| |_|    \___|  \__,_| |_| |_| |___/
  //
  //-----------------------------------------------------------------------------------------
  //(NB : Train_Test_Evaluate, Read, and Create_Control_Trees all need to be run on the full variable list)
  
  
  
  //#############################################
  //  CREATE INSTANCE OF CLASS & INITIALIZE
  //#############################################
  theMVAtool* MVAtool = new theMVAtool(thevarlist, thesamplelist, thesystlist, thechannellist, v_color, set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, nofbin_templates);
  if(MVAtool->stop_program) {return 1;}
  MVAtool->Set_Luminosity(set_luminosity);
  
  //#############################################
  // TRAINING
  //#############################################
  if(doTraining){
    for(int i=0; i<thechannellist.size(); i++)
    {
      //TString bdt_type = "BDT"; //'BDT' or 'BDTttZ' depending on the region (for theta disambiguation)
      MVAtool->Train_Test_Evaluate(thechannellist[i], bdt_type);
    }
  }
  //#############################################
  //  READING --- TEMPLATES CREATION
  //#############################################
  //   TString template_name = "mWt"; //Either 'BDT', 'BDTttZ', 'mWt' or 'm3l'
  if(doReading){
    MVAtool->Read(template_name, fakes_from_data, real_data_templates, fakes_summed_channels);
    
  }
  
  if(!real_data_templates && doPSdataTemplate) {
    MVAtool->Generate_PseudoData_Histograms_For_Templates(template_name, "All");
    for(int ichan= 0 ; ichan < thechannellist.size() ; ichan++){
      MVAtool->Generate_PseudoData_Histograms_For_Templates(template_name, thechannellist[ichan]);
    }
  }
  
  
  if(doBDTControl)
  {
    
    MVAtool->Draw_Control_Templates("uuu", fakes_from_data, false, template_name);
    MVAtool->Draw_Control_Templates("uue", fakes_from_data, false, template_name);
    MVAtool->Draw_Control_Templates("eeu", fakes_from_data, false, template_name);
    MVAtool->Draw_Control_Templates("eee", fakes_from_data, false, template_name);
    
    
  }
  //#############################################
  //  CONTROL TREES & HISTOGRAMS
  //#############################################
  if(doCRcut) cut_BDT_CR = MVAtool->Determine_Control_Cut();
  // bool cut_on_BDT = false; bool use_pseudodata_CR_plots = false;
  
  if(doCRtree) MVAtool->Create_Control_Trees(fakes_from_data, cut_on_BDT, cut_BDT_CR, use_pseudodata_CR_plots);
  if(doCRhisto){
    MVAtool->Create_Control_Histograms(fakes_from_data, use_pseudodata_CR_plots); //NB : very long ! You should only activate necessary syst./var. !
    
    if(use_pseudodata_CR_plots) {MVAtool->Generate_PseudoData_Histograms_For_Control_Plots(fakes_from_data);}
  }
  //#############################################
  //  DRAW PLOTS
  //#############################################
  if(doPlottingPerChannel){
    for(int i=0; i<thechannellist.size(); i++)
    {
      MVAtool->Draw_ControlttZ_Plots(thechannellist[i], fakes_from_data, false); //Draw plots for the BDT CR
      // MVAtool->Plot_Templates(thechannellist[i], template_name, false); //Plot the BDT distributions of MC & pseudo-data templates
    }
  }
  if(doPlotting){
    cout << "*** Control plots ***" << endl;
    //  MVAtool->Draw_Control_Plots_isis("", fakes_from_data, true); //Sum of 4 channels
    cout << "*** Templates ***" << endl;
    MVAtool->Plot_Templates("", template_name, true); //Sum of 4 channels
  }
  
  
  
  //-------------------------------------------
  //    _____   _   _   ____
  //   | ____| | \ | | |  _ \
  //   |  _|   |  \| | | | | |
  //   | |___  | |\  | | |_| |
  //   |_____| |_| \_| |____/
  //
  //-------------------------------------------
  return 0;
}
