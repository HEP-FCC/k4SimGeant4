#include "MaterialScan_2D_genericAngle.h"
#include "k4Interface/IGeoSvc.h"

#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/Service.h"

#include "DD4hep/Detector.h"
#include "DD4hep/Printout.h"
#include "DDRec/MaterialManager.h"
#include "DDRec/Vector3D.h"

#include "TFile.h"
#include "TMath.h"
#include "TTree.h"
#include "TVector3.h"

MaterialScan_2D_genericAngle::MaterialScan_2D_genericAngle(const std::string& name, ISvcLocator* svcLoc) : Service(name, svcLoc),
m_geoSvc("GeoSvc", name) {}

StatusCode MaterialScan_2D_genericAngle::initialize() {
  if (Service::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  
  if (!m_geoSvc) {
    error() << "Unable to find Geometry Service." << endmsg;
    return StatusCode::FAILURE;
  }

  std::list<std::string> allowed_angleDef = {"eta", "theta", "thetaRad", "cosTheta"};
  if (std::find(allowed_angleDef.begin(), allowed_angleDef.end(), m_angleDef) == allowed_angleDef.end()){
    error() << "Non valid angleDef option given. Use either 'eta', 'theta', 'thetaRad' or 'cosTheta'!" << endmsg;
    return StatusCode::FAILURE;
  }

  SmartIF<IRndmGenSvc> randSvc;
  randSvc = service("RndmGenSvc");
  StatusCode sc = m_flatAngleDist.initialize(randSvc, Rndm::Flat(0., m_angleBinning));
  if (sc == StatusCode::FAILURE) {
    error() << "Unable to initialize random number generator." << endmsg;
    return sc;
  }

  std::unique_ptr<TFile> rootFile(TFile::Open(m_filename.value().c_str(), "RECREATE"));
  // no smart pointers possible because TTree is owned by rootFile (root mem management FTW!)
  TTree* tree = new TTree("materials", "");
  double angle = 0;
  double phi = 0;
  double angleRndm = 0;
  unsigned nMaterials = 0;
  std::unique_ptr<std::vector<double>> nX0(new std::vector<double>);
  std::unique_ptr<std::vector<double>> nLambda(new std::vector<double>);
  std::unique_ptr<std::vector<double>> matDepth(new std::vector<double>);
  std::unique_ptr<std::vector<std::string>> material(new std::vector<std::string>);
  auto nX0Ptr = nX0.get();
  auto nLambdaPtr = nLambda.get();
  auto matDepthPtr = matDepth.get();
  auto materialPtr = material.get();

  tree->Branch("angle", &angleRndm);
  tree->Branch("phi", &phi);
  tree->Branch("nMaterials", &nMaterials);
  tree->Branch("nX0", &nX0Ptr);
  tree->Branch("nLambda", &nLambdaPtr);
  tree->Branch("matDepth", &matDepthPtr);
  tree->Branch("material", &materialPtr);

  auto lcdd = m_geoSvc->getDetector();
  dd4hep::rec::MaterialManager matMgr(lcdd->detector(m_envelopeName).volume());
  dd4hep::rec::Vector3D beginning(0, 0, 0);
  auto boundaryVol = lcdd->detector(m_envelopeName).volume()->GetShape();
  std::array<Double_t, 3> pos = {0, 0, 0};
  std::array<Double_t, 3> dir = {0, 0, 0};
  TVector3 vec(0, 0, 0);

  for (angle = m_angleMin; angle < m_angleMax; angle += m_angleBinning) {
    std::cout << m_angleDef << ": " << angle << std::endl;

    for (int iPhi = 0; iPhi < m_nPhi; ++iPhi) {
      nX0->clear();
      nLambda->clear();
      matDepth->clear();
      material->clear();

      std::map<dd4hep::Material, double> phiMaterialsBetween; // For phi scan

      phi = -M_PI + (0.5+iPhi)/m_nPhi * 2 * M_PI;
      angleRndm = angle+0.5*m_angleBinning;

      if(m_angleDef=="eta")
        vec.SetPtEtaPhi(1, angleRndm, phi);
      else if(m_angleDef=="theta")
        vec.SetPtThetaPhi(1, angleRndm/360.0*2*M_PI, phi);
      else if(m_angleDef=="thetaRad")
        vec.SetPtThetaPhi(1, angleRndm, phi);
      else if(m_angleDef=="cosTheta")
        vec.SetPtThetaPhi(1, acos(angleRndm), phi);
          
      auto n = vec.Unit();
      dir = {n.X(), n.Y(), n.Z()};
      // if the start point (beginning) is inside the material-scan envelope (e.g. if envelope is world volume)
      double distance = boundaryVol->DistFromInside(pos.data(), dir.data());
      // if the start point (beginning) is not inside the envelope
      if (distance == 0) {
        distance = boundaryVol->DistFromOutside(pos.data(), dir.data());
      }
      dd4hep::rec::Vector3D end(dir[0] * distance, dir[1] * distance, dir[2] * distance);
      debug() << "Calculating material between 0 and (" << end.x() << ", " << end.y() << ", " << end.z()
              << ") <=> " << m_angleDef << " = " << angle << ", phi =  " << phi << endmsg;
      const dd4hep::rec::MaterialVec& materials = matMgr.materialsBetween(beginning, end);
      for (unsigned i = 0, n_materials = materials.size(); i < n_materials; ++i) {
        phiMaterialsBetween[materials[i].first] += materials[i].second;
      }
      nMaterials = phiMaterialsBetween.size();
      for (auto matpair : phiMaterialsBetween) {
        TGeoMaterial* mat = matpair.first->GetMaterial();
        material->push_back(mat->GetName());
        matDepth->push_back(matpair.second);
        nX0->push_back(matpair.second / mat->GetRadLen());
        nLambda->push_back(matpair.second / mat->GetIntLen());
      }
      tree->Fill();
    }
  }
  tree->Write();
  rootFile->Close();

  return StatusCode::SUCCESS;
}

StatusCode MaterialScan_2D_genericAngle::finalize() { return StatusCode::SUCCESS; }

DECLARE_COMPONENT(MaterialScan_2D_genericAngle)
