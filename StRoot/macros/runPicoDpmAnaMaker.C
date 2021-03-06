
/* **************************************************
 *   Run StPicoDpmAnaMaker in different modes
 * --------------------------------------------------
 *   
 *  - Different modes to use the  class
 *    - StPicoHFMaker::kAnalyze - don't write candidate trees, just fill histograms
 *        inputFile : fileList of PicoDst files or single picoDst file
 *        outputFile: baseName for outfile 
 *    - StPicoHFMaker::kWrite   - write candidate trees
 *        inputFile : path to single picoDist file
 *        outputFile: baseName for outfile 
 *    - StPicoHFMaker::kRead    - read candidate trees and fill histograms
 *        inputFile : fileList of PicoDst files
 *        outputFile: baseName for outfile 
 *
 * --------------------------------------------------
 *  Authors:  Xin Dong        (xdong@lbl.gov)
 *            Michael Lomnitz (mrlomnitz@lbl.gov)
 *            Mustafa Mustafa (mmustafa@lbl.gov)
 *            Jochen Thaeder  (jmthader@lbl.gov)
 *
 * **************************************************
 */

#include "TROOT.h"
#include "TSystem.h"
#include "TChain.h"


# ifndef __CINT__

#include "StMaker.h"
#include "StChain.h"

#include "StMuDSTMaker/COMMON/macros/loadSharedLibraries.C"

#include "StPicoDstMaker/StPicoDstMaker.h"
#include "StPicoHFMaker/StPicoHFEvent.h"
#include "StPicoHFMaker/StHFCuts.h"

#include "StRoot/StRefMultCorr/StRefMultCorr.h"
#include "StRoot/StRefMultCorr/CentralityMaker.h"
#include "StRoot/StEventPlane/StEventPlane.h"
#include "StPicoDpmAnaMaker/StPicoDpmAnaMaker.h"

#else
class StChain;
#endif

class StMaker;
class StChain;
class StPicoDstMaker;
class StRefMultCorr;
class StEventPlane;

StChain *chain;

void runPicoDpmAnaMaker(const Char_t *inputFile="test.list", const Char_t *outputFile="outputBaseName",  unsigned int makerMode = 0 /*kAnalyze*/, 
			const Char_t *badRunListFileName = "picoList_bad_MB.list", const Char_t *treeName = "picoHFtree",
			   const Char_t *productionBasePath = "/project/projectdirs/starprod/picodsts/Run14/AuAu/200GeV/physics2/P15ic",
			   unsigned int decayChannel = 0 /* kChannel0 */) { 
  // -- Check STAR Library. Please set SL_version to the original star library used in the production 
  //    from http://www.star.bnl.gov/devcgi/dbProdOptionRetrv.pl
  string SL_version = "SL15c";
  string env_SL = getenv ("STAR");
  if (env_SL.find(SL_version)==string::npos) {
      cout<<"Environment Star Library does not match the requested library in runPicoDpmAnaMaker.C. Exiting..."<<endl;
      exit(1);
  }

  Int_t nEvents = 10000000;
  //Int_t nEvents = 2000;
	
  gROOT->LoadMacro("$STAR/StRoot/StMuDSTMaker/COMMON/macros/loadSharedLibraries.C");
  loadSharedLibraries();

  gSystem->Load("StBTofUtil");
  gSystem->Load("StPicoDstMaker");
  gSystem->Load("StPicoPrescales");
  gSystem->Load("StRefMultCorr");
  gSystem->Load("StEventPlane");

  gSystem->Load("StPicoCutsBase");
  gSystem->Load("StPicoHFMaker");
  gSystem->Load("StPicoHFMyAnaMaker");
  gSystem->Load("StPicoDpmAnaMaker");

  chain = new StChain();

  // ========================================================================================
  //makerMode    = StPicoHFMaker::kAnalyze;
  // ========================================================================================
  
  cout << "Maker Mode    " << makerMode << endl;

  TString sInputFile(inputFile);
  TString sInputListHF("");  
  TString sProductionBasePath(productionBasePath);
  TString sTreeName(treeName);

  if (makerMode == StPicoHFMaker::kAnalyze) {
    if (!sInputFile.Contains(".list") && !sInputFile.Contains("picoDst.root")) {
      cout << "No input list or picoDst root file provided! Exiting..." << endl;
      exit(1);
    }
  }
  else if (makerMode == StPicoHFMaker::kWrite) {
    if (!sInputFile.Contains("picoDst.root")) {
      cout << "No input picoDst root file provided! Exiting..." << endl;
      exit(1);
    }
  }
  else if (makerMode == StPicoHFMaker::kRead) {
   if (!sInputFile.Contains(".list")) {
      cout << "No input list provided! Exiting..." << endl;
      exit(1);
   }

   // -- prepare filelist for picoDst from hfTrees
   sInputListHF = sInputFile;
   sInputFile = "tmpPico.list";
   TString command = "sed 's|" + sTreeName + ".root|picoDst.root|g' " + sInputListHF + " > " + sInputFile;
   cout << "COMMAND : " << command << endl; 
   gSystem->Exec(command.Data());
   command = "sed -i 's|^.*" + sTreeName + "|" + sProductionBasePath + "|g' " + sInputFile; // + " > " + sInputFile;
   cout << "COMMAND : " << command << endl; 
   gSystem->Exec(command.Data());
  }
  else {
    cout << "Unknown makerMode! Exiting..." << endl;
    exit(1);
  }
  
  StPicoDstMaker* picoDstMaker = new StPicoDstMaker(0, sInputFile, "picoDstMaker");
  StRefMultCorr* grefmultCorrUtil  = CentralityMaker::instance()->getgRefMultCorr();
  StEventPlane*  eventPlaneMaker = new StEventPlane("eventPlaneMaker",picoDstMaker,grefmultCorrUtil);
  
  StPicoDpmAnaMaker* picoDpmAnaMaker = new StPicoDpmAnaMaker("picoDpmAnaMaker", picoDstMaker, 
							     grefmultCorrUtil, eventPlaneMaker,
							     outputFile, sInputListHF);
  picoDpmAnaMaker->setMakerMode(makerMode);

  StHFCuts* hfCuts = new StHFCuts("hfBaseCuts");
  picoDpmAnaMaker->setHFBaseCuts(hfCuts);

  // ---------------------------------------------------
  // -- Set Base cuts for HF analysis

  // -- File name of bad run list
  hfCuts->setBadRunListFileName(badRunListFileName);

  // -- ADD USER CUTS HERE ----------------------------

  hfCuts->setCutVzMax(6.);
  hfCuts->setCutVzVpdVzMax(3.);
  //hfCuts->setCutTriggerWord(0x1F);

  hfCuts->setCutNHitsFitMax(15); 
  hfCuts->setCutRequireHFT(true);
  // hfCuts->setCutNHitsFitnHitsMax(0.52);
  // ---------------------------------------------------

  // -- Channel1
  picoDpmAnaMaker->setDecayMode(StPicoHFEvent::kThreeParticleDecay);
  picoDpmAnaMaker->setDecayChannel(StPicoDpmAnaMaker::kChannel1);

  // -- ADD USER CUTS HERE ----------------------------
  // --- Lomnitz topological cuts
  hfCuts->setCutSecondaryTriplet(0.01, 0.01,0.01,
				 0.005, 0.2, 				 
				 0.995, 1.7, 2.1);
				 //				 0.008, 0.008, 0.008);
  // --- Lomnitz cuts to remove noise from ghosting
  //Single track pt
  hfCuts->setCutPtRange(0.6,10.0,StHFCuts::kPion);
  hfCuts->setCutPtRange(0.6,10.0,StHFCuts::kKaon);
  //TPC setters
  hfCuts->setCutTPCNSigmaPion(3.0);
  hfCuts->setCutTPCNSigmaKaon(3.0);
  //TOF setters, need to set pt range as well
  hfCuts->setCutTOFDeltaOneOverBeta(0.05, StHFCuts::kKaon);
  hfCuts->setCutPtotRangeHybridTOF(0.6,10.0,StHFCuts::kKaon);
  // ========================================================================================

  // ========================================================================================

  chain->Init();
  cout << "chain->Init();" << endl;
  int total = picoDstMaker->chain()->GetEntries();
  cout << " Total entries = " << total << endl;
  if(nEvents>total) nEvents = total;
  for (Int_t i=0; i<nEvents; i++) {
    if(i%10000==0)
      cout << "Working on eventNumber " << i << endl;
    
    chain->Clear();
    int iret = chain->Make(i);
    
    if (iret) { cout << "Bad return code!" << iret << endl; break;}
    
    total++;
  }
  
  cout << "****************************************** " << endl;
  cout << "Work done... now its time to close up shop!"<< endl;
  cout << "****************************************** " << endl;
  chain->Finish();
  cout << "****************************************** " << endl;
  cout << "total number of events  " << nEvents << endl;
  cout << "****************************************** " << endl;
  
  delete chain;

  // -- clean up if in read mode
  //if (makerMode == StPicoHFMaker::kRead)
    //gSystem->Exec(Form("rm -f %s", sInputFile.Data()));
}

