#include "theMVAtool.h"
// run as
// ./main --Train BDT --Read BDT --Region singletop --Coupling Zct --PSData BDT

using namespace std;

int main(int argc, char* argv[])
{
  //-------------------
  std::vector<TString> thesamplelist;
  std::vector<TString> thesamplelist_forreading;
  std::vector<TString > thevarlist; //Variables used in BDT
  std::vector<TString > thevarlisttree;
  std::vector<TString > thechannellist;
  std::vector<TString > set_v_cut_name;
  std::vector<TString > set_v_cut_def;
  std::vector<bool > set_v_cut_IsUsedForBDT;
  //-------------------
  
  
  
  int verbose = 2;
  std::string PlaceOfTuples = "Ntuples/"; //170223/MVAtrees/";
  // What are you doing?
  double number_of_fakes = 3;
  bool doTraining = false;
  bool doReading = false;
  bool doCRcut = false;
  bool createPSData = false;
  TString psdata = "";
  TString bdt_type = "BDT"; //'BDT' or 'BDTttZ' depending on the region (for theta disambiguation) for training!
  TString region_name = "singletop";
  int nofbin_templates = 10;
  TString coupling = "Zut";
  
  //Use MC fakes or data-driven fakes)
  bool fakes_from_data = false;
  
  //If true, use real data sample to create *templates* (BDT, mWt, ...) / else, use pseudodata !
  bool real_data_templates = false;
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
      cout << "* Methods in order of execution * " <<endl;
      cout << "--Train Template" << endl;
      cout << "--Read Template" << endl;
      cout << "--Region region" << endl;
      cout << "--Coupling coupling" << endl;
       return 0;
    }
    else if(argval == "--Train"){ doTraining = true; iarg++; bdt_type = argv[iarg]; cout << bdt_type << endl; }
     else if(argval == "--Read"){ doReading = true; iarg++; bdt_type = argv[iarg]; cout << bdt_type << endl;}
    else if(argval == "--Region"){
      iarg++;
      region_name = argv[iarg];
    }
      else if(argval == "--Coupling"){
      iarg++;
      coupling = argv[iarg];
    }
    
    else if(argval == ""){cerr << "ERROR no arguments given" << endl; return 0; }
  }
  
  
  bdt_type = bdt_type + "_" + region_name + "_" + coupling;
  
  cout<<endl<<BOLD(FYEL("##################################"))<<endl;
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
  if( bdt_type.Contains("BDT")){
    // tt signal nJets > 1, nBJets >= 1
    // ST signal nJets == 1, nBJets == 1
    //set_v_cut_name.push_back("nJets");                  set_v_cut_def.push_back(">1");            set_v_cut_IsUsedForBDT.push_back(false);
    // set_v_cut_name.push_back("nJets_CSVL");                 set_v_cut_def.push_back(">=1");            set_v_cut_IsUsedForBDT.push_back(false);
    if(region_name.Contains("singletop")){
      set_v_cut_name.push_back("MVA_region");                 set_v_cut_def.push_back("==0");            set_v_cut_IsUsedForBDT.push_back(false);
      // set_v_cut_name.push_back("MVA_Wlep_Charge");            set_v_cut_def.push_back(">0");              set_v_cut_IsUsedForBDT.push_back(false);
      
    }
    
    if(region_name.Contains("toppair")) set_v_cut_name.push_back("MVA_region");                 set_v_cut_def.push_back("==1");            set_v_cut_IsUsedForBDT.push_back(false);
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
  thesamplelist_forreading.push_back("data");
  
  
  //Signal
  //thesamplelist.push_back("NP_overlay_ST_FCNC_zut");             v_color.push_back(kRed+1);
  //  thesamplelist.push_back("TT_TopZkZct");             v_color.push_back(kRed+1);
  if(coupling.Contains("Zct")){
    if(region_name.Contains("singletop")){
      thesamplelist.push_back("NP_overlay_ST_FCNC_zct");
     thesamplelist_forreading.push_back("NP_overlay_TT_FCNC-T2ZJ_aTleptonic_ZToll_kappa_zct");
      thesamplelist_forreading.push_back("NP_overlay_TT_FCNC-aT2ZJ_Tleptonic_ZToll_kappa_zct");
    }
    else{
      //thesamplelist_forreading.push_back("NP_overlay_ST_FCNC_zct");
      thesamplelist.push_back("NP_overlay_ST_FCNC_zct");
      thesamplelist.push_back("NP_overlay_TT_FCNC-aT2ZJ_Tleptonic_ZToll_kappa_zct");
      thesamplelist.push_back("NP_overlay_TT_FCNC-T2ZJ_aTleptonic_ZToll_kappa_zct");
    }
  }
  
  if(coupling.Contains("Zut")) {
    if(region_name.Contains("singletop")){
      thesamplelist.push_back("NP_overlay_ST_FCNC_zut");
      //thesamplelist.push_back("NP_overlay_TT_FCNC-aT2ZJ_Tleptonic_ZToll_kappa_zut");
      //thesamplelist.push_back("NP_overlay_TT_FCNC_T2ZJ_aTleptonic_ZToll_kappa_zut");
      thesamplelist_forreading.push_back("NP_overlay_TT_FCNC-aT2ZJ_Tleptonic_ZToll_kappa_zut");
      thesamplelist_forreading.push_back("NP_overlay_TT_FCNC_T2ZJ_aTleptonic_ZToll_kappa_zut");
    }
    else{
     // thesamplelist_forreading.push_back("NP_overlay_ST_FCNC_zut");
      thesamplelist.push_back("NP_overlay_ST_FCNC_zut");
      thesamplelist.push_back("NP_overlay_TT_FCNC_T2ZJ_aTleptonic_ZToll_kappa_zut");
      thesamplelist.push_back("NP_overlay_TT_FCNC-aT2ZJ_Tleptonic_ZToll_kappa_zut");
    }
  }
  
  //BKG
 
     thesamplelist.push_back("WZTo3LNu_amc");
    //thesamplelist.push_back("WZTo3LNu");
    //thesamplelist.push_back("WZTo3LNu_0Jets_MLL-4To50"); empty
    // thesamplelist.push_back("WZTo3LNu_0Jets_MLL50");
    /* thesamplelist.push_back("WZTo3LNu_1Jets_MLL-4To50");
     thesamplelist.push_back("WZTo3LNu_1Jets_MLL50");
     thesamplelist.push_back("WZTo3LNu_2Jets_MLL-4To50");
     thesamplelist.push_back("WZTo3LNu_2Jets_MLL50");
     thesamplelist.push_back("WZTo3LNu_3Jets_MLL-4To50");
     thesamplelist.push_back("WZTo3LNu_3Jets_MLL50");*/
    thesamplelist.push_back("ZZTo4L");
    thesamplelist.push_back("TTZToLLNuNu_amc");
    thesamplelist.push_back("tZq_amc");
    thesamplelist_forreading.push_back("TTWJetsToLNu_amc");
    //thesamplelist_forreading.push_back("ttZ");
   // thesamplelist_forreading.push_back("TTZToQQ_amc");
    thesamplelist_forreading.push_back("WZZ_amc");
    
    // hesamplelist.push_back("STs_amc"); // empty
    // thesamplelist_forreading.push_back("STt_top"); // empty
    //thesamplelist_forreading.push_back("STt_atop"); // empty
     thesamplelist_forreading.push_back("STtW_top"); // empty
      thesamplelist_forreading.push_back("STtW_atop"); // empty
    thesamplelist_forreading.push_back("ttHToNonbb");
    thesamplelist_forreading.push_back("ttHTobb");
    thesamplelist_forreading.push_back("tWll");
    //thesamplelist_forreading.push_back("ttWJets");
    thesamplelist_forreading.push_back("ZZZ_amc");
  
  thesamplelist_forreading.push_back("tHq");
  
  
    //FAKES
    thesamplelist_forreading.push_back("fake");
   thesamplelist_forreading.push_back("TTJets_powheg");
  thesamplelist_forreading.push_back("DYJets50_amc");
  thesamplelist_forreading.push_back("fake_80X_nonpromptinW");
  thesamplelist_forreading.push_back("fake_80X_nonpromptinZ");
 // thesamplelist_forreading.push_back("Zjets50_1Jets");
//  thesamplelist_forreading.push_back("Zjets50_3Jets");
 // thesamplelist_forreading.push_back("Zjets50_4Jets");
  //thesamplelist_forreading.push_back("WZJTo3LNu_amc");
 // thesamplelist_forreading.push_back("WZTo3LNu");
 // thesamplelist_forreading.push_back("WZTo3LNu_amc");
  //other signal
 // thesamplelist_forreading.push_back("TTJets_powheg_80Xnonpromptcorrect");
 // thesamplelist_forreading.push_back("TTJets_powheg_80Xnonpromptwrong");
 /* thesamplelist_forreading.push_back("WZTo3LNu_amc_new");
  thesamplelist_forreading.push_back("WZTo3LNu_amc_new_80Xbb");
  thesamplelist_forreading.push_back("WZTo3LNu_amc_new_80Xcc");
  thesamplelist_forreading.push_back("WZTo3LNu_amc_new_80Xlight");*/
  
  
  
  /*    thesamplelist_forreading.push_back("NP_overlay_ST_FCNC_zct_nopt");
      thesamplelist_forreading.push_back("NP_overlay_TT_FCNC-T2ZJ_aTleptonic_ZToll_kappa_zct_nopt");
      thesamplelist_forreading.push_back("NP_overlay_TT_FCNC-aT2ZJ_Tleptonic_ZToll_kappa_zct_nopt");
  
      thesamplelist_forreading.push_back("NP_overlay_ST_FCNC_zut_nopt");

      thesamplelist_forreading.push_back("NP_overlay_TT_FCNC-aT2ZJ_Tleptonic_ZToll_kappa_zut_nopt");
      thesamplelist_forreading.push_back("NP_overlay_TT_FCNC_T2ZJ_aTleptonic_ZToll_kappa_zut_nopt");
  */
  
//  thesamplelist_forreading.push_back("fake_80X_nonpromptinW");
 // thesamplelist_forreading.push_back("fake_80X_nonpromptinZ");
  
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
  //NB : treat leaves/variables "Weight" and "Channel" and "region" separately in MVAtool
  if(coupling.Contains("Zut") && region_name.Contains("toppair")){
    //    thevarlist.push_back("MVA_Zboson_pt");
    thevarlist.push_back("MVA_SMtop_eta");
    thevarlist.push_back("MVA_mlb");
    thevarlist.push_back("MVA_dPhiWlepb");
    thevarlist.push_back("MVA_deltaRWlepJet_min");
    thevarlist.push_back("MVA_Zboson_M");
    thevarlist.push_back("MVA_dPhiZWlep");
    thevarlist.push_back("MVA_NJets_CSVv2M");
    thevarlist.push_back("MVA_dRWlepb");
    thevarlist.push_back("MVA_FCNCtop_M");
    thevarlist.push_back("MVA_dRZc");
    thevarlist.push_back("MVA_dRSMjetLightjet");
  }
  else if(coupling.Contains("Zut") && region_name.Contains("singletop")){
    
    thevarlist.push_back("MVA_SMtop_eta");
    thevarlist.push_back("MVA_mlb");
    thevarlist.push_back("MVA_dPhiWlepb");
    thevarlist.push_back("MVA_charge_asym");
    thevarlist.push_back("MVA_bdiscCSVv2_jet_0");
    thevarlist.push_back("MVA_dRWlepb");
    thevarlist.push_back("MVA_TotalHt_lep");
    //thevarlist.push_back("MVA_TotalInvMass_lep");
    thevarlist.push_back("MVA_dRZWlep");
    thevarlist.push_back("MVA_ptWQ");
  }
  else   if(coupling.Contains("Zct") && region_name.Contains("singletop")){
    thevarlist.push_back("MVA_TotalInvMass_lep");
    // thevarlist.push_back("MVA_Zboson_pt");
    thevarlist.push_back("MVA_mlb");
    thevarlist.push_back("MVA_bdiscCSVv2_jet_0");
    thevarlist.push_back("MVA_SMtop_eta");
    thevarlist.push_back("MVA_dPhiWlepb");
    thevarlist.push_back("MVA_dRZWlep");
    //   thevarlist.push_back("MVA_SMtop_rap");
    //  thevarlist.push_back("MVA_Zboson_M");
    
  }
  else if(region_name.Contains("toppair") && coupling.Contains("Zct") ){
    //   thevarlist.push_back("MVA_SMtop_rap");
    //  thevarlist.push_back("MVA_FCNCtop_rap");
    thevarlist.push_back("MVA_NJets_CSVv2M");
    thevarlist.push_back("MVA_mlb");
    thevarlist.push_back("MVA_FCNCtop_M");
    thevarlist.push_back("MVA_dRWlepb");
    thevarlist.push_back("MVA_TotalInvMass");
    thevarlist.push_back("MVA_Bdis_Lightjet");
    thevarlist.push_back("MVA_dRZc");
    thevarlist.push_back("MVA_dRSMjetLightjet");
    thevarlist.push_back("MVA_dRZWlep");
    thevarlist.push_back("MVA_dPhiZWlep");
    //thevarlist.push_back("MVA_SMtop_rap");
    // check me
    thevarlist.push_back("MVA_Zboson_M");
    thevarlist.push_back("MVA_dRZb");
    // thevarlist.push_back("MVA_TotalInvMass_jet");
  }
  
  
  /*
  thevarlist.push_back("MVA_Zboson_pt");
  thevarlist.push_back("MVA_SMtop_eta");
  thevarlist.push_back("MVA_mlb");
  thevarlist.push_back("MVA_dPhiWlepb");
  thevarlist.push_back("MVA_charge_asym");
  thevarlist.push_back("MVA_TotalInvMass_lep");
  thevarlist.push_back("MVA_bdiscCSVv2_jet_0");
 thevarlist.push_back("MVA_deltaRWlepJet_max");
 thevarlist.push_back("MVA_deltaRWlepJet_min");
  thevarlist.push_back("MVA_ptWQ");
  thevarlist.push_back("MVA_Zboson_M");
  thevarlist.push_back("MVA_SMtop_rap");
  thevarlist.push_back("MVA_Wlep_Charge");

  thevarlist.push_back("MVA_dPhiZWlep");
  thevarlist.push_back("MVA_NJets_CSVv2M");
  thevarlist.push_back("MVA_SMtop_M");
thevarlist.push_back("MVA_dRWlepb");
thevarlist.push_back("MVA_TotalHt_lep");
  thevarlist.push_back("MVA_bdiscCSVv2_jet_1");
  thevarlist.push_back("MVA_FCNCtop_M");
  thevarlist.push_back("MVA_dRZc");
  thevarlist.push_back("MVA_dPhiZb");
  thevarlist.push_back("MVA_dRSMFCNCtop");
  thevarlist.push_back("MVA_dRZb");
  thevarlist.push_back("MVA_FCNCtop_eta");


  thevarlist.push_back("MVA_Bdis_Lightjet");
  thevarlist.push_back("MVA_dRSMjetLightjet");
 thevarlist.push_back("MVA_dRZWlep");
  thevarlist.push_back("MVA_TotalInvMass_jet");
//}
*/
  
  
  
 
  
  
  
 
  
  
  
  
  
  
  
  
  
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
  theMVAtool* MVAtool = new theMVAtool(thevarlist, thesamplelist, thesamplelist_forreading, thechannellist,  set_v_cut_name, set_v_cut_def, set_v_cut_IsUsedForBDT, nofbin_templates, PlaceOfTuples, region_name);
  cout << "Place of tuples " << PlaceOfTuples << endl;
  if(MVAtool->stop_program) {return 1;}
  
  
  //#############################################
  // TRAINING
  //#############################################
  if(doTraining){
    for(int i=0; i<thechannellist.size(); i++)
    {
      MVAtool->Train_Test_Evaluate(thechannellist[i], bdt_type);
    }
  }
  if(doReading){
    MVAtool->Read(bdt_type);
    
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
