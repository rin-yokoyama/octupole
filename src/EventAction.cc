//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B1/src/EventAction.cc
/// \brief Implementation of the B1::EventAction class

#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"

#include "InitParticleEventInfo.hh"
namespace B1
{

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

  EventAction::EventAction(std::shared_ptr<RunAction> runAction)
      : runAction_(runAction)
  {
  }

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

  void EventAction::BeginOfEventAction(const G4Event *)
  {
    frontSi_ = 0;
    SiMap_.clear();
    CsIMap_.clear();
  }

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

  void EventAction::EndOfEventAction(const G4Event *anEvent)
  {
    auto info = (InitParticleEventInfo *)anEvent->GetUserInformation();
    runAction_->AddEventInfo(info->GetProtonEnergy(), info->GetThetaLab(), info->GetPhiLab());
    for (const auto &si : SiMap_)
    {
      runAction_->AddEdep("Si", si.second, si.first);
    }
    for (const auto &csi : CsIMap_)
    {
      runAction_->AddEdep("CsI", csi.second, csi.first);
    }
    if (frontSi_ > 0)
      runAction_->AddEdep("front", frontSi_, 0);
    // delete info;
    //  accumulate statistics in run action
    runAction_->IncrementEvent();
  }

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

  void EventAction::AddSiEdep(const G4double &eDep, G4int copyNum)
  {
    SiMap_[copyNum] = SiMap_[copyNum] + eDep;
  }

  void EventAction::AddCsIEdep(const G4double &eDep, G4int copyNum)
  {
    CsIMap_[copyNum] = CsIMap_[copyNum] + eDep;
  }

  void EventAction::AddFrontEdep(const G4double &eDep)
  {
    frontSi_ += eDep;
  }
}
