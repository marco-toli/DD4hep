#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TRandom.h"
#include "DD4hep/Printout.h"
#include "DD4hep/Objects.h"
#include "DD4hep/Factories.h"
#include "DDG4/Geant4Particle.h"
#include "DDG4/Geant4Data.h"

#include <vector>

const float sampling_fraction=100.;

void SCEPCALplot() {


  // read in libraries that define the classes
  Long_t result;
  char text[1024];
  const char* dd4hep = gSystem->Getenv("DD4hepINSTALL");
  snprintf(text,sizeof(text)," -I%s/include -D__DD4HEP_DDEVE_EXCLUSIVE__ -Wno-shadow -g -O0",dd4hep);
  gSystem->AddIncludePath(text);
  TString fname = "libDDG4IO";
  const char* io_lib = gSystem->FindDynamicLibrary(fname,kTRUE);
  result = gSystem->Load("libDDG4IO");
  result = gSystem->Load("libDDEvePlugins");


  // open data and output file for histograms
  const char* inputfilename="/data/users/eno/dd4hep/DD4hep/DDDetectors/compact/testSCEPCAL.root";
  const char* outputfilename="testSCEPCAL_hist.root";


  // define histograms
  TH1F *hgenPsize = new TH1F("hgenPsize","number of generator particles",600,0.,40000);
  TH1F *hgenPdgID = new TH1F("hgenpdgID","pdgID of generator particles",600,-1000,1000);


  // get Tree
  TFile *f = new TFile(inputfilename);
  TTreeReader myReader("EVENT",f);

  // get objects
  TTreeReaderValue<std::vector<dd4hep::sim::Geant4Particle*>> myMCParticles(myReader, "MCParticles");
  TTreeReaderValue<std::vector<dd4hep::sim::Geant4Calorimeter::Hit*>> myEcalBarrelHits(myReader, "EcalBarrelHits");

  // loop over events
  unsigned int evtCounter = 0;
  while (myReader.Next() && evtCounter<3){
    cout << "Event " << evtCounter++ << endl;

    // some gen particle plots
    std::cout<<" number of gen particles "<< myMCParticles->size() <<std::endl;
    dd4hep::sim::Geant4Particle* itmp = myMCParticles->at(0);
    double px = itmp->psx;
    double py = itmp->psy;
    double pT=sqrt(px*px+py*py);
    std::cout<<"  first particle id pt is "<<itmp->pdgID<<" "<<pT/1000.<<std::endl;

    for(int ipart=0; ipart < myMCParticles->size(); ipart++){
        hgenPdgID->Fill((myMCParticles->at(ipart))->pdgID);
    }
    hgenPsize->Fill( myMCParticles->size());


    std::cout<<" number of barrel hits is "<<myEcalBarrelHits->size()<<std::endl;
    double Edep=0.;
    for(int iHit=0; iHit < myEcalBarrelHits->size(); iHit++){
      dd4hep::sim::Geant4Calorimeter::Hit* itmp=myEcalBarrelHits->at(iHit);
      if(iHit<5) std::cout<<" Hit "<<iHit<<" has E "<<(itmp->energyDeposit)/1000.<<std::endl;
      Edep+=(itmp->energyDeposit)/1000.;

    }
    std::cout<<" total energy deposited in calorimeter is "<<Edep<<std::endl;




  }
 



  
 
 
  f->Close();

  TFile * out = new TFile(outputfilename,"RECREATE");
  hgenPsize->Write();
  hgenPdgID->Write();
  out->Close();

}


