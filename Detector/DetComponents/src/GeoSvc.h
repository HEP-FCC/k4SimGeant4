#ifndef GEOSVC_H
#define GEOSVC_H

// Interface
#include "k4Interface/IGeoSvc.h"

// Gaudi
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/ServiceHandle.h"

// DD4Hep
#include "DD4hep/Detector.h"

// Geant4
#include "G4RunManager.hh"
#include "G4VUserDetectorConstruction.hh"

class GeoSvc : public extends<Service, IGeoSvc> {

public:
  /// Default constructor
  GeoSvc(const std::string& name, ISvcLocator* svc);

  /// Destructor
  virtual ~GeoSvc();
  /// Initialize function
  virtual StatusCode initialize() final;
  /// Finalize function
  virtual StatusCode finalize() final;
  /// This function generates the DD4hep geometry
  StatusCode buildDD4HepGeo();
  /// This function generates the Geant4 geometry
  StatusCode buildGeant4Geo();
  // receive DD4hep Geometry
  virtual dd4hep::DetElement getDD4HepGeo() override;
  [[deprecated("Use getDetector() instead")]]
  virtual dd4hep::Detector* lcdd() override;
  virtual dd4hep::Detector* getDetector() override;
  virtual std::string constantAsString(std::string const& name) override;
  // receive Geant4 Geometry
  virtual G4VUserDetectorConstruction* getGeant4Geo() override;

private:
  /// Pointer to the interface to the DD4hep geometry
  dd4hep::Detector* m_dd4hepgeo;
  /// Pointer to the detector construction of DDG4
  std::shared_ptr<G4VUserDetectorConstruction> m_geant4geo;
  /// XML-files with the detector description
  Gaudi::Property<std::vector<std::string>> m_xmlFileNames{this, "detectors", {}, "Detector descriptions XML-files"};
  /// mapping of sensitive detector names
  Gaudi::Property<std::map<std::string, std::string>> m_sensitive_types{this, "sensitiveTypes", {{"tracker", "SimpleTrackerSD"}, {"calorimeter", "SimpleCalorimeterSD"}}};
  /// Whether to create the geant4 geometry or not
  Gaudi::Property<bool> m_buildGeant4Geo{this, "EnableGeant4Geo", true, "If True the DD4hep geometry is converted for Geant4 Simulations"};

};

#endif  // GEOSVC_H
