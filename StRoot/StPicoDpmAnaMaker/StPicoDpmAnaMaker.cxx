#include <vector>

#include "TClonesArray.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"

#include "StThreeVectorF.hh"
#include "StLorentzVectorF.hh"

#include "StPicoDstMaker/StPicoDst.h"
#include "StPicoDstMaker/StPicoDstMaker.h"
#include "StPicoDstMaker/StPicoEvent.h"
#include "StPicoDstMaker/StPicoTrack.h"
#include "StPicoDstMaker/StPicoBTofPidTraits.h"

#include "StPicoHFMaker/StPicoHFEvent.h"
#include "StPicoHFMaker/StHFCuts.h"
#include "StPicoHFMaker/StHFPair.h"
#include "StPicoHFMaker/StHFTriplet.h"

#include "StRoot/StRefMultCorr/StRefMultCorr.h"
#include "StRoot/StEventPlane/StEventPlane.h"

#include "StPicoDpmAnaMaker.h"
#include "StDpmHists.h"
#include "SystemOfUnits.h"
ClassImp(StPicoDpmAnaMaker)

// _________________________________________________________
StPicoDpmAnaMaker::StPicoDpmAnaMaker(char const* name, StPicoDstMaker* picoMaker, 
				     StRefMultCorr* grefmultCorrUtil, StEventPlane* eventPlaneMaker,
				     char const* outputBaseFileName, char const* inputHFListHFtree = "") :
  StPicoHFMaker(name, picoMaker, outputBaseFileName, inputHFListHFtree),
  mGRefMultCorrUtil(grefmultCorrUtil), mEventPlane(eventPlaneMaker), mDecayChannel(kChannel1)
{
}

// _________________________________________________________
StPicoDpmAnaMaker::~StPicoDpmAnaMaker() {
  // destructor
}

// _________________________________________________________
int StPicoDpmAnaMaker::InitHF() {
  // -- INITIALIZE USER HISTOGRAMS ETC HERE -------------------
  //    add them to the output list mOutList which is automatically written

  // EXAMPLE //  mOutList->Add(new TH1F(...));
  // EXAMPLE //  TH1F* hist = static_cast<TH1F*>(mOutList->Last());
  ntp_DMeson = new TNtuple("ntp","DMeson Tree","flag:dca1:dca2:dca3:dcaMax:pt1:pt2:pt3:theta_hs:decayL_hs:pt_hs:mass_hs:eta_hs:phi_hs:dV0Max_hs:kaonTof");
  mHists = new StDpmHists();
  return kStOK;
}

// _________________________________________________________
void StPicoDpmAnaMaker::ClearHF(Option_t *opt="") {
  return;
}

// _________________________________________________________
int StPicoDpmAnaMaker::FinishHF() {
  if( isMakerMode() != StPicoHFMaker::kWrite ){
    ntp_DMeson->Write();
    mHists->closeFile();
  }
  
  return kStOK;
}

// _________________________________________________________
int StPicoDpmAnaMaker::MakeHF() {
  // -- process event
  //    ADD YOUR PROCESSING CODE HERE
  //    ... it is usefull to use the methods below
  //     - createCandidates()
  //     - analyzeCandidates()
  if (isMakerMode() == StPicoHFMaker::kWrite) {
    createCandidates();
  }
  else if (isMakerMode() == StPicoHFMaker::kRead) {
    // -- the reading back of the perviously written trees happens in the background
    analyzeCandidates();
  }
  else if (isMakerMode() == StPicoHFMaker::kAnalyze) {
    createCandidates();
    analyzeCandidates();
  }

  return kStOK;
}

// _________________________________________________________
int StPicoDpmAnaMaker::createCandidates() {
  // --- Lomnitz
  // Creating candidates for D+- 3 body decay
  // D+- -> K+2Pi decay
  // --- 
  for (unsigned short idxPion1 = 0; idxPion1 < mIdxPicoPions.size(); ++idxPion1) {
    StPicoTrack const *pion1 = mPicoDst->track(mIdxPicoPions[idxPion1]);
    // -- Pion selection
    if( !isPion(pion1) ) continue;
    
    for (unsigned short idxPion2 = idxPion1+1; idxPion2 < mIdxPicoPions.size(); ++idxPion2) {
      StPicoTrack const *pion2 = mPicoDst->track(mIdxPicoPions[idxPion2]);
      // -- Pion selection
      if( !isPion(pion2) ) continue;

      if ( pion1->charge() != pion2->charge() ) continue;
      if ( !isCloseTracks(pion1,pion2,mPrimVtx, mBField)) continue;

      for (unsigned short idxKaon = 0; idxKaon < mIdxPicoKaons.size(); ++idxKaon) {
	StPicoTrack const *kaon = mPicoDst->track(mIdxPicoKaons[idxKaon]);
	// -- Kaon selection
	// -- TPC
	if( !isKaon(kaon) ) continue;
	// -- TOF
	if( !mHFCuts->isHybridTOFHadron(kaon, mHFCuts->getTofBetaBase(kaon), StHFCuts::kKaon) ) continue;
	
	if( kaon->charge() == pion1->charge() ) continue;
	if (mIdxPicoKaons[idxKaon] == mIdxPicoPions[idxPion1]|| mIdxPicoKaons[idxKaon] == mIdxPicoPions[idxPion2] || mIdxPicoPions[idxPion1] == mIdxPicoPions[idxPion2]) 
	  continue;
	// -- Making triplet
	StHFTriplet triplet(pion1,pion2,kaon,mHFCuts->getHypotheticalMass(StHFCuts::kPion),mHFCuts->getHypotheticalMass(StHFCuts::kPion),mHFCuts->getHypotheticalMass(StHFCuts::kKaon), mIdxPicoPions[idxPion1],mIdxPicoPions[idxPion2],mIdxPicoKaons[idxKaon], mPrimVtx, mBField);
	if (!mHFCuts->isGoodSecondaryVertexTriplet(triplet)) 
	  continue;
	mPicoHFEvent->addHFSecondaryVertexTriplet(&triplet);

      }  // for (unsigned short idxKaon = 0; idxKaon < mIdxPicoKaons.size(); ++idxKaon)
    } // for (unsigned short idxPion2 = idxPion1+1; idxPion2 < mIdxPicoPions.size(); ++idxPion2)
  } // for (unsigned short idxPion1 = 0; idxPion1 < mIdxPicoPions.size(); ++idxPion1)

 return kStOK;
}

// _________________________________________________________
int StPicoDpmAnaMaker::analyzeCandidates() {

  //Loading event centrality and event plane bins
  if (!mGRefMultCorrUtil){
    LOG_WARN << " No mGRefMultCorrUtil! Skip! " << endl;
    return kStWarn;
  }
  StThreeVectorF const pVtx = mPicoDst->event()->primaryVertex();
  mGRefMultCorrUtil->init(mPicoDst->event()->runId());
  mGRefMultCorrUtil->initEvent(mPicoDst->event()->grefMult(), pVtx.z(), mPicoDst->event()->ZDCx()) ;
  int const centrality  = mGRefMultCorrUtil->getCentralityBin9();
  if (centrality < 0 || centrality > 8) return kStOk;
  const float reweight = mGRefMultCorrUtil->getWeight();
  //Event plane 
  if (!loadEventPlaneCorr(mEventPlane)){
    LOG_WARN << "Event plane calculations unavalable! Skipping" << endm;
    //mFailedRunnumber = mPicoDst->event()->runId();
    return kStOK;
  }
  float const eventPlane = mEventPlane->getEventPlane();
  int const eventPlane_bin = (int)((eventPlane) / 0.3141592) ;
  if (eventPlane_bin < 0  ||  eventPlane_bin > 9) return kStOk;
  mHists->addEventPlane(centrality, eventPlane, mEventPlane->getResolutionRandom(), mEventPlane->getResolutionEta());
  // --- Analyze previously constructed candidates and output to ntuple
  // -- Decay channel1
  TClonesArray const * aCandidates= mPicoHFEvent->aHFSecondaryVertices();
  if( mPicoHFEvent->nHFSecondaryVertices() >0 ){
    for (unsigned int idx = 0; idx <  mPicoHFEvent->nHFSecondaryVertices(); ++idx) {

      StHFTriplet const* triplet = static_cast<StHFTriplet*>(aCandidates->At(idx));
      StPicoTrack const* pion1 = mPicoDst->track(triplet->particle1Idx());
      StPicoTrack const* pion2 = mPicoDst->track(triplet->particle2Idx());
      StPicoTrack const* kaon = mPicoDst->track(triplet->particle3Idx());
      
      // Greates distance between tracks
      float const dcaDaughters_12 = triplet->dcaDaughters12();
      float const dcaDaughters_23 = triplet->dcaDaughters23();
      float const dcaDaughters_13 = triplet->dcaDaughters31();
      float dcaMax = dcaDaughters_12 > dcaDaughters_13 ? dcaDaughters_12 : dcaDaughters_13;
      dcaMax = dcaMax > dcaDaughters_23 ? dcaMax : dcaDaughters_23;
      
      float kaonTOF = 0;
      if( mHFCuts->getTofBetaBase(kaon) > 0){
	float const tofBeta = mHFCuts->getTofBetaBase(kaon);
	float const ptot    = kaon->dcaGeometry().momentum().mag();
	float const betaInv = sqrt(ptot*ptot + pow(mHFCuts->getHypotheticalMass(StHFCuts::kKaon),2.0)) / ptot;
	kaonTOF = 1.0/tofBeta - betaInv;
      }
      float const pt1=pion1->gPt();
      float  const pt2=pion2->gPt();
      float  const pt3=kaon->gPt();

      // -- Flag D plus and Dminus
      float flag = -99.;
      if( pion1->charge()>0 && kaon->charge()<0 ) flag=0.; // -- D+
      if( pion1->charge()<0 && kaon->charge()>0 ) flag=1.; // -- D-
      
      //Event plane stuff
      int trkIndex[3] = { triplet->particle1Idx() , triplet->particle2Idx(), triplet->particle3Idx() };
      float psi = mEventPlane->getEventPlane(3, trkIndex);
      float dPhi = triplet->phi() - psi;
      
      while(dPhi < 0) dPhi += TMath::Pi();
      while(dPhi >= TMath::Pi()) dPhi -= TMath::Pi();
      
      if ( isHistoTriplet(triplet) ){
	mHists->fillUnlikeSign(triplet, centrality, dPhi, reweight);
      }

      int ii=0;
      float ntVar[30];
      float const dca1 = triplet->particle1Dca();
      float const dca2 = triplet->particle2Dca();
      float const dca3 = triplet->particle3Dca();
      float pt=sqrt(pow(triplet->px(),2.0)+pow(triplet->py(),2.0));
      // ---
      // Saving to NTUPLE
      ntVar[ii++] = flag;
      ntVar[ii++] = triplet->particle1Dca();
      ntVar[ii++] = triplet->particle2Dca();
      ntVar[ii++] = triplet->particle3Dca();
      ntVar[ii++] = dcaMax;
      ntVar[ii++] = pt1;
      ntVar[ii++] = pt2;
      ntVar[ii++] = pt3;
      ntVar[ii++] = triplet->pointingAngle();
      ntVar[ii++] = triplet->decayLength();
      ntVar[ii++] = pt;
      ntVar[ii++] = triplet->m();
      ntVar[ii++] = triplet->eta();
      ntVar[ii++] = triplet->phi();
      ntVar[ii++] = triplet->dV0Max();
      ntVar[ii++] = kaonTOF;
      ntp_DMeson->Fill(ntVar);
    } // for (unsigned int idx = 0; idx <  mPicoHFEvent->nHFSecondaryVertices(); ++idx) {
  }
 return kStOK;
}

// _________________________________________________________
bool StPicoDpmAnaMaker::isPion(StPicoTrack const * const trk) const {
  // -- good pion
  return ( mHFCuts->isGoodTrack(trk) && mHFCuts->isTPCPion(trk) );
}

// _________________________________________________________
bool StPicoDpmAnaMaker::isKaon(StPicoTrack const * const trk) const {
  // -- good kaon
  return (mHFCuts->isGoodTrack(trk) && mHFCuts->isTPCKaon(trk) );
} 

// _________________________________________________________
bool StPicoDpmAnaMaker::isProton(StPicoTrack const * const trk) const {
  // -- good proton
  return (mHFCuts->isGoodTrack(trk) && ( mHFCuts->isTPCProton(trk) || mHFCuts->isTOFProton(trk) ));
}
// _________________________________________________________
bool StPicoDpmAnaMaker::isCloseTracks(StPicoTrack const * const trk1, StPicoTrack const * const trk2, StThreeVectorF const & vtx, float bField) const {
  
  StPhysicalHelixD p1Helix = trk1->dcaGeometry().helix();
  StPhysicalHelixD p2Helix = trk2->dcaGeometry().helix();
  p1Helix.moveOrigin(p1Helix.pathLength(vtx));
  p2Helix.moveOrigin(p2Helix.pathLength(vtx));
  if( ( p1Helix.origin()-vtx ).mag()>0.2 || ( p2Helix.origin()-vtx ).mag()>0.2 ) return false;
  //Requires loading constants
  StThreeVectorF const p1Mom = p1Helix.momentum(bField * kilogauss);
  StThreeVectorF const p2Mom = p2Helix.momentum(bField * kilogauss);
  StPhysicalHelixD const p1StraightLine(p1Mom, p1Helix.origin(), 0, trk1->charge());
  StPhysicalHelixD const p2StraightLine(p2Mom, p2Helix.origin(), 0, trk2->charge());
  //DCA
  pair<double, double> const ss = p1StraightLine.pathLengths(p2StraightLine);
  StThreeVectorF const p1AtDcaToP2 = p1StraightLine.at(ss.first);
  StThreeVectorF const p2AtDcaToP1 = p2StraightLine.at(ss.second);
  int const dca = (p1AtDcaToP2-p1AtDcaToP2).mag();
  if(dca > 0.01) return false;
// -- good pair
  return true;
}
// _________________________________________________________
bool StPicoDpmAnaMaker::loadEventPlaneCorr(StEventPlane const * mEventPlane)
{
  //needs to implement, will currently break maker
  if (!mEventPlane)
    {
      LOG_WARN << "No EventPlane ! Skipping! " << endm;
      return kFALSE;
    }
  if (!mEventPlane->getAcceptEvent())
    {
      // LOG_WARN << "StPicoMixedEvent::THistograms and TProiles NOT found! shoudl check the input Qvector files From HaoQiu ! Skipping this run! " << endm;
      return kFALSE;
    }
  return kTRUE;
}
// _________________________________________________________
bool StPicoDpmAnaMaker::isHistoTriplet(StHFTriplet const* const triplet)
{
  StPicoTrack const* pion1 = mPicoDst->track(triplet->particle1Idx());
  StPicoTrack const* pion2 = mPicoDst->track(triplet->particle2Idx());
  StPicoTrack const* kaon = mPicoDst->track(triplet->particle3Idx());
 
  float const pt1=pion1->gPt();
  float  const pt2=pion2->gPt();
  float  const pt3=kaon->gPt();
  return cos(triplet->pointingAngle()) > 0.998 && 
    triplet->dcaDaughters12() < 0.008 && triplet->dcaDaughters23() < 0.008 && 
    triplet->dcaDaughters31() < 0.008 && 
    triplet->particle1Dca() > 0.01 && triplet->particle2Dca() > 0.01 && 
    triplet->particle3Dca() > 0.008 && pt1 > 0.8 && pt2>0.8 && pt3>0.6 &&
    triplet->dV0Max()<0.02;
}
