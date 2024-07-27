#include "SimG4Alg.h"

// FCCSW
#include "SimG4Interface/ISimG4Svc.h"

// Geant
#include "G4Event.hh"

DECLARE_COMPONENT(SimG4Alg)

SimG4Alg::SimG4Alg(const std::string& aName, ISvcLocator* aSvcLoc) : Gaudi::Algorithm(aName, aSvcLoc),
m_geantSvc("SimG4Svc", aName) {
  declareProperty("eventProvider", m_eventTool, "Handle for tool that creates the G4Event");
}
SimG4Alg::~SimG4Alg() {}

StatusCode SimG4Alg::initialize() {
  if (Gaudi::Algorithm::initialize().isFailure()) return StatusCode::FAILURE;
  if (!m_geantSvc) {
    error() << "Unable to locate Geant Simulation Service" << endmsg;
    return StatusCode::FAILURE;
  }
  for (auto& saveTool : m_saveTools) {
    if(!saveTool.retrieve()) {
      error() << "Unable to retrieve the output saving tool " << saveTool
              << endmsg;
      return StatusCode::FAILURE;
    }
  }
  if (!m_eventTool.retrieve()) {
    error() << "Unable to retrieve the G4Event provider " << m_eventTool << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode SimG4Alg::execute(const EventContext&) const {
  // first translate the event
  G4Event* event = m_eventTool->g4Event();

  if (!event) {
    error() << "Unable to retrieve G4Event from " << m_eventTool << endmsg;
    return StatusCode::FAILURE;
  }
  m_geantSvc->processEvent(*event).ignore();
  G4Event* constevent;
  m_geantSvc->retrieveEvent(constevent).ignore();
  for (auto& tool : m_saveTools) {
    tool->saveOutput(*constevent).ignore();
  }
  m_geantSvc->terminateEvent().ignore();
  return StatusCode::SUCCESS;
}

StatusCode SimG4Alg::finalize() { return Gaudi::Algorithm::finalize(); }
