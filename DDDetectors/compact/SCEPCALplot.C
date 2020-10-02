#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TRandom.h"

const float sampling_fraction=100.;

void SCEPCALplot() {

  const char* inputfilename="testSCEPCAL.root";
  const char* outputfilename="testSCEPCAL_hist.root";

  TH1F *hgenPsize = new TH1F("hgenPsize","number of generator particles",600,0.,40000);


  TFile *f = new TFile(inputfilename);
  TTree *t1 = (TTree*)f->Get("EVENT");

  vector<float> *inputMomentum = new vector<float>;
  int genP_size;  



  t1->SetBranchAddress("MCParticles/@size",&genP_size);


  Int_t nentries = (Int_t)t1->GetEntries();
  for(Int_t i=0;i<nentries; i++) {
    t1->GetEntry(i);
    std::cout<<" number of gen particles "<<genP_size<<std::endl;
    hgenPsize->Fill(genP_size);
		    
  }

  f->Close();

  TFile * out = new TFile(outputfilename,"RECREATE");
  hgenPsize->Write();

  out->Close();

}


