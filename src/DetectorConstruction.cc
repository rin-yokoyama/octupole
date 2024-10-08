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
/// \file B1/src/DetectorConstruction.cc
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "ExpConstants.hh"
#include "G4Material.hh"
#include "G4VisAttributes.hh"

namespace B1
{

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

  G4VPhysicalVolume *DetectorConstruction::Construct()
  {
    // Get nist material manager
    G4NistManager *nist = G4NistManager::Instance();

    // Option to switch on/off checking of volumes overlaps
    //
    G4bool checkOverlaps = true;

    //
    // World
    //
    G4double world_sizeXY = B1::kWorldSize;
    G4double world_sizeZ = B1::kWorldSize;
    G4Material *world_mat = nist->FindOrBuildMaterial("G4_AIR");
    G4Material *det_mat = nist->FindOrBuildMaterial("G4_Galactic");

    auto solidWorld = new G4Box("World",                                                    // its name
                                0.5 * world_sizeXY, 0.5 * world_sizeXY, 0.5 * world_sizeZ); // its size

    auto logicWorld = new G4LogicalVolume(solidWorld, // its solid
                                          world_mat,  // its material
                                          "World");   // its name

    auto physWorld = new G4PVPlacement(nullptr,         // no rotation
                                       G4ThreeVector(), // at (0,0,0)
                                       logicWorld,      // its logical volume
                                       "World",         // its name
                                       nullptr,         // its mother  volume
                                       false,           // no boolean operation
                                       0,               // copy number
                                       checkOverlaps);  // overlaps checking

    auto solidDet = new G4Box("Detector",                                                 // its name
                              0.1 * world_sizeXY, 0.1 * world_sizeXY, 0.1 * world_sizeZ); // its size
    auto logicDet = new G4LogicalVolume(solidDet, det_mat, "detectorMother");
    G4VisAttributes *invisibleAttributes = new G4VisAttributes();
    invisibleAttributes->SetVisibility(false);
    logicDet->SetVisAttributes(invisibleAttributes);

    G4Material *gps = nullptr;
    {
      const int ncomp = 3;
      const G4double density = 5.3 * g / cm3;
      gps = new G4Material("LaGPS", density, ncomp);
      auto elemGd = new G4Element("Gadolinium", "Gd", 64., 157.25 * g / mole);
      auto elemSi = new G4Element("Silicon", "Si", 14., 28.0 * g / mole);
      auto elemO = new G4Element("Gadolinium", "Gd", 8., 16.00 * g / mole);
      gps->AddElement(elemGd, 2);
      gps->AddElement(elemSi, 2);
      gps->AddElement(elemO, 5);
    }

    /// Target
    G4Material *target_mat = nist->FindOrBuildMaterial("G4_POLYETHYLENE");
    auto tubs = new G4Tubs("Target", 0, B1::kTargetRadius, B1::kTargetThickness * 0.5, 2 * M_PI, 2 * M_PI);
    auto logicTarget = new G4LogicalVolume(tubs, target_mat, "Target");
    G4ThreeVector target_pos;
    new G4PVPlacement(nullptr, target_pos, logicTarget, "target", logicWorld, false, 0, checkOverlaps);

    /// Si strips
    G4Material *si_mat = nist->FindOrBuildMaterial("G4_Si");

    const G4double si_strip_width = B1::kSiSize / B1::kNSiStrips;
    std::vector<G4ThreeVector> pos_vec;
    for (int i = 0; i < B1::kNSiStrips; ++i)
    {
      pos_vec.emplace_back(G4ThreeVector(B1::kSiXOffset, B1::kSiYOffset + si_strip_width * i, B1::kSiZOffset));
    }

    auto siStripSolid = new G4Box("Strip", 0.5 * B1::kSiSize, 0.5 * si_strip_width, 0.5 * B1::kSiThickness);

    auto siStripLogic = new G4LogicalVolume(siStripSolid, // its solid
                                            si_mat,       // its material
                                            "SiStrip");   // its name

    {
      G4int i_strip = 0;
      for (const auto &vec : pos_vec)
      {
        new G4PVPlacement(nullptr,        // no rotation
                          vec,            // at position
                          siStripLogic,   // its logical volume
                          "SiStrip",      // its name
                          logicDet,       // its mother  volume
                          false,          // no boolean operation
                          i_strip,        // copy number
                          checkOverlaps); // overlaps checking
        ++i_strip;
      }
    }
    // Set siStip as scoring volume
    //
    fScoringVolume = siStripLogic;

    // front Si
    auto SiSolid = new G4Box("Si", 0.5 * B1::kSiSize, 0.5 * B1::kSiSize, 0.5 * B1::kFrontSiThickness);
    auto SiLogic = new G4LogicalVolume(SiSolid, si_mat, "Si");
    G4ThreeVector SiPos(B1::kSiXOffset, B1::kSiYOffset + 0.5 * B1::kSiSize, B1::kSiZOffset - 2. * B1::kFrontSiThickness);
    new G4PVPlacement(nullptr, SiPos, SiLogic, "Si", logicDet, false, 0, checkOverlaps);
    G4VisAttributes *SiAttributes = new G4VisAttributes();
    SiAttributes->SetColor(1, 0, 0);
    SiLogic->SetVisAttributes(SiAttributes);

    /// CsI
    auto CsISolid = new G4Box("CsI", 0.5 * B1::kCsISize, 0.5 * B1::kCsISize, 0.5 * B1::kCsIThickness);
    auto CsILogic = new G4LogicalVolume(CsISolid, // its solid
                                        gps,      // its material
                                        "CsI");   // its name
    std::vector<G4ThreeVector> CsIPosVec;
    CsIPosVec.emplace_back(G4ThreeVector(B1::kSiXOffset + 0.5 * B1::kCsISize, B1::kSiYOffset + 0.5 * B1::kSiSize + 0.5 * B1::kCsISize, B1::kSiZOffset + 0.5 * B1::kCsIThickness + B1::kCsIZOffset));
    CsIPosVec.emplace_back(G4ThreeVector(B1::kSiXOffset - 0.5 * B1::kCsISize, B1::kSiYOffset + 0.5 * B1::kSiSize + 0.5 * B1::kCsISize, B1::kSiZOffset + 0.5 * B1::kCsIThickness + B1::kCsIZOffset));
    CsIPosVec.emplace_back(G4ThreeVector(B1::kSiXOffset + 0.5 * B1::kCsISize, B1::kSiYOffset + 0.5 * B1::kSiSize - 0.5 * B1::kCsISize, B1::kSiZOffset + 0.5 * B1::kCsIThickness + B1::kCsIZOffset));
    CsIPosVec.emplace_back(G4ThreeVector(B1::kSiXOffset - 0.5 * B1::kCsISize, B1::kSiYOffset + 0.5 * B1::kSiSize - 0.5 * B1::kCsISize, B1::kSiZOffset + 0.5 * B1::kCsIThickness + B1::kCsIZOffset));

    {
      G4int i_crystal = 0;
      for (const auto &vec : CsIPosVec)
      {
        new G4PVPlacement(nullptr,        // no rotation
                          vec,            // at position
                          CsILogic,       // its logical volume
                          "CsI",          // its name
                          logicDet,       // its mother  volume
                          false,          // no boolean operation
                          i_crystal,      // copy number
                          checkOverlaps); // overlaps checking
        ++i_crystal;
      }
    }
    //
    // always return the physical World
    //
    new G4PVPlacement(&B1::kRotation, B1::kPosition, logicDet, "detector", logicWorld, false, 0);

    return physWorld;
  }

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
