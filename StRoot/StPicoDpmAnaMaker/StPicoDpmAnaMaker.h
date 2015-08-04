#ifndef StPicoDpmAnaMaker_h
#define StPicoDpmAnaMaker_h

#include "StPicoHFMaker/StPicoHFMaker.h"
#include "TNtuple.h"

/* **************************************************
 *  Sample class fo HF picoDST analysis
 * --------------------------------------------------
 * 
 *  For more info look also in the .h files in StPicoDpmAnaMaker/
 *     StPicoDpmAnaMaker/StPicoDpmAnaMaker.h      <-- Base Class for analysis
 *     StPicoDpmAnaMaker/StPicoHFEvent.h      <-- Holds candidates for one event (written to Tree)
 *     StPicoDpmAnaMaker/StHFCuts.h           <-- Cuts, can be set in run macro
 *     StPicoDpmAnaMaker/StHFPair.h           <-- Holds a pair candidate of a two body decay
 *     StPicoDpmAnaMaker/StHFTriplet.h        <-- Holds a triplet of a three body decay
 *
 *  Usage:
 *   - Implement
 *        InitHF()
 *        MakeHF()
 *        ClearHF()
 *        FinishHF()
 *
 *  - Do not ovewrite Init, Make, Clear, Finish which are inhertited from StPicoDpmAnaMaker via StMaker 

 *  - Set StHFCuts class via setHFBaseCuts(...) in run macro
 *
 *  - Set use mode of StPicoDpmAnaMaker class  via setMakerMode(...)
 *     use enum of StPicoDpmAnaMaker::eMakerMode
 *      StPicoDpmAnaMaker::kAnalyze - don't write candidate trees, just fill histograms
 *      StPicoDpmAnaMaker::kWrite   - write candidate trees
 *      StPicoDpmAnaMaker::kRead    - read candidate trees and fill histograms
 *
 *  - Set decay mode of analysis via setDecayMode(...)
 *     use enum of StPicoHFEvent::eHFEventMode (see there for more info)
 *      StPicoHFEvent::kTwoParticleDecay,
 *      StPicoHFEvent::kThreeParticleDecay
 *      StPicoHFEvent::kTwoAndTwoParticleDecay
 *
 *  - Implement these track selection methods used to fill vectors for 'good' identified particles
 *      (methods from StHFCuts utility class can/should be used)
 *       isPion
 *       isKaon
 *       isProton
 *
 *  --------------------------------------------------
 *  
 *  Initial Authors:  
 *           
 *        **Michael Lomnitz
 * 
 *  ** Code Maintainer
 *
 * **************************************************
 */

class TH1F;
class TH2F;
class TH3F;

class StPicoDst;
class StPicoDstMaker;
class StPicoEvent;
class StPicoTrack;
class StPicoHFEvent;

class StHFPair;
class StHFTriplet;
class StHFCuts;
class StDpmHists;

class StRefMultCorr;
class StEventPlane;

class StPicoDpmAnaMaker : public StPicoHFMaker 
{
 public:
  StPicoDpmAnaMaker(char const* name, StPicoDstMaker* picoMaker,
		    StRefMultCorr* grefmultCorrUtil, StEventPlane* eventPlaneMaker,
		    char const* outputBaseFileName, char const* inputHFListHFtree);
  virtual ~StPicoDpmAnaMaker();
  
  virtual Int_t InitHF();
  virtual Int_t MakeHF();
  virtual void  ClearHF(Option_t *opt);
  virtual Int_t FinishHF();
  virtual bool isCloseTracks(StPicoTrack const*, StPicoTrack const*,StThreeVectorF const & , float) const;
  // -- ADOPT DECAY CHANNELS, if wished ------------------- 
  void setDecayChannel(unsigned int u) { mDecayChannel = u; }
  bool loadEventPlaneCorr(StEventPlane const *mEventPlane);
  bool isHistoTriplet(StHFTriplet const* const);


  enum eDecayChannel {kChannel1, kChannel2, kChannel3};

 protected:
  virtual bool isPion(StPicoTrack const*) const;
  virtual bool isKaon(StPicoTrack const*) const;
  virtual bool isProton(StPicoTrack const*) const;
  
 private:
  StRefMultCorr* mGRefMultCorrUtil;
  StEventPlane* mEventPlane;
  int createCandidates();
  int analyzeCandidates();

  // -- private members --------------------------

  unsigned int mDecayChannel;

  // -- ADD USER MEMBERS HERE ------------------- 
  TNtuple *ntp_DMeson;
  StDpmHists* mHists;

  // -- ADD USER MEMBERS HERE -------------------

  ClassDef(StPicoDpmAnaMaker, 1)
};

#endif
