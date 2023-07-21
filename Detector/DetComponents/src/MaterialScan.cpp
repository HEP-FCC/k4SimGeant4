#include "MaterialScan.h"
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

MaterialScan::MaterialScan(const std::string& name, ISvcLocator* svcLoc) : Service(name, svcLoc),
m_geoSvc("GeoSvc", name) {}

StatusCode MaterialScan::initialize() {
  if (Service::initialize().isFailure()) {
    return StatusCode::FAILURE;
  }
  
  if (!m_geoSvc) {
    error() << "Unable to find Geometry Service." << endmsg;
    return StatusCode::FAILURE;
  }

  if(m_angleDef not in ["eta", "theta", "thetaRad", "cosTheta"]){
    error() << "Non valid angleDef option given. Use either 'eta', 'theta', 'thetaRad' or 'cosTheta'!" << endmsg;
    return StatusCode::FAILURE;
  }

  SmartIF<IRndmGenSvc> randSvc;
  randSvc = service("RndmGenSvc");
  StatusCode sc = m_flatPhiDist.initialize(randSvc, Rndm::Flat(0., M_PI / 2.));
  if (sc == StatusCode::FAILURE) {
    error() << "Unable to initialize random number generator." << endmsg;
    return sc;
  }
  sc = m_flatAngleDist.initialize(randSvc, Rndm::Flat(0., m_angleBinning));
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

  tree->Branch("angle", &angle);
  tree->Branch("nMaterials", &nMaterials);
  tree->Branch("nX0", &nX0Ptr);
  tree->Branch("nLambda", &nLambdaPtr);
  tree->Branch("matDepth", &matDepthPtr);
  tree->Branch("material", &materialPtr);

  auto lcdd = m_geoSvc->lcdd();
  dd4hep::rec::MaterialManager matMgr(lcdd->detector(m_envelopeName).volume());
  dd4hep::rec::Vector3D beginning(0, 0, 0);
  auto boundaryVol = lcdd->detector(m_envelopeName).volume()->GetShape();
  std::array<Double_t, 3> pos = {0, 0, 0};
  std::array<Double_t, 3> dir = {0, 0, 0};
  TVector3 vec(0, 0, 0);
  for (angle = m_angleMin; angle < m_angleMax; angle += m_angleBinning) {
    nX0->clear();
    nLambda->clear();
    matDepth->clear();
    material->clear();

    std::map<dd4hep::Material, double> phiAveragedMaterialsBetween;
    for (int iPhi = 0; iPhi < m_nPhiTrials; ++iPhi) {
      std::cout << m_angleDef << ": " << angle << std::endl;      
      phi = m_flatPhiDist();
      angleRndm = angle + m_flatAngleDist();

      match m_angleDef:
        case "eta":
          vec.SetPtEtaPhi(1, angleRndm, phi);
        case "theta":
          vec.SetPtThetaPhi(1, angleRndm/360.0*2*M_PI, phi);
        case "thetaRad":
          vec.SetPtThetaPhi(1, angleRndm, phi);
        case "cosTheta":
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
        phiAveragedMaterialsBetween[materials[i].first] += materials[i].second / static_cast<double>(m_nPhiTrials);
      }
    }
    nMaterials = phiAveragedMaterialsBetween.size();
    for (auto matpair : phiAveragedMaterialsBetween) {
      TGeoMaterial* mat = matpair.first->GetMaterial();
      material->push_back(mat->GetName());
      matDepth->push_back(matpair.second);
      nX0->push_back(matpair.second / mat->GetRadLen());
      nLambda->push_back(matpair.second / mat->GetIntLen());
    }
    tree->Fill();
  }
  tree->Write();
  rootFile->Close();
  return StatusCode::SUCCESS;
}

StatusCode MaterialScan::finalize() { return StatusCode::SUCCESS; }

DECLARE_COMPONENT(MaterialScan)
