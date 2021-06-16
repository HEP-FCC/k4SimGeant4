from Gaudi.Configuration import *

# Data service
from Configurables import FCCDataSvc
podioevent = FCCDataSvc("EventDataSvc")

# DD4hep geometry service
from Configurables import GeoSvc
from os import environ, path
detector_path = environ.get("FCCDETECTORS", "")
detectors = ['Detector/DetFCChhBaseline1/compact/FCChh_DectEmptyMaster.xml',
             'Detector/DetFCChhECalInclined/compact/FCChh_ECalBarrel_upstream.xml']
geoservice = GeoSvc("GeoSvc", detectors=[path.join(detector_path, detector) for detector in detectors],
                    OutputLevel = WARNING)

# Geant4 service
# Configures the Geant simulation: geometry, physics list and user actions
from Configurables import SimG4Svc
geantservice = SimG4Svc("SimG4Svc", detector='SimG4DD4hepDetector', physicslist="SimG4FtfpBert", actions="SimG4FullSimActions")
geantservice.g4PostInitCommands += ["/run/setCut 0.1 mm"]

# Geant4 algorithm
# Translates EDM to G4Event, passes the event to G4, writes out outputs via tools
# and a tool that saves the calorimeter hits
from Configurables import SimG4Alg, SimG4SaveCalHits, SimG4SingleParticleGeneratorTool
saveecaltool = SimG4SaveCalHits("saveECalBarrelHits",readoutNames = ["ECalBarrelEta"])
saveecaltool.CaloHits.Path = "ECalBarrelHits"
from Configurables import SimG4SingleParticleGeneratorTool
pgun=SimG4SingleParticleGeneratorTool("SimG4SingleParticleGeneratorTool",saveEdm=True,
                                      particleName = "e-", energyMin = 50000, energyMax = 50000, etaMin = 0, etaMax = 0, phiMin = -0.1, phiMax = 0.1,
                                      OutputLevel = DEBUG)
# next, create the G4 algorithm, giving the list of names of tools ("XX/YY")
geantsim = SimG4Alg("SimG4Alg",
                    outputs= ["SimG4SaveCalHits/saveECalBarrelHits"],
                    eventProvider = pgun,
                    OutputLevel = DEBUG)


from Configurables import CreateCaloCells
createcellsBarrel = CreateCaloCells("CreateCaloCellsBarrel",
                                    doCellCalibration=False,
                                    addPosition=True,
                                    addCellNoise=False, filterCellNoise=False)
createcellsBarrel.hits.Path="ECalBarrelHits"
createcellsBarrel.cells.Path="ECalBarrelCells"

from Configurables import EnergyInCaloLayers
energy_in_layers = EnergyInCaloLayers("energyInLayers",
                                      readoutName="ECalBarrelEta",
                                      numLayers=8,
                                      # sampling fraction is given as the energy correction will be applied on
                                      # calibrated cells
                                      samplingFractions=[0.12125] + [0.14283] + [0.16354] + [0.17662] + [0.18867] +
                                                        [0.19890] + [0.20637] + [0.20802],
                                      OutputLevel=VERBOSE)
energy_in_layers.deposits.Path = "ECalBarrelCells"
energy_in_layers.particle.Path = "GenParticles"

#CPU information
from Configurables import AuditorSvc, ChronoAuditor
chra = ChronoAuditor()
audsvc = AuditorSvc()
audsvc.Auditors = [chra]
geantsim.AuditExecute = True
energy_in_layers.AuditExecute = True

# ApplicationMgr
from Configurables import ApplicationMgr
ApplicationMgr( TopAlg = [geantsim, createcellsBarrel, energy_in_layers],
                EvtSel = 'NONE',
                EvtMax = 10,
                # order is important, as GeoSvc is needed by G4SimSvc
                ExtSvc = [podioevent, geoservice, geantservice, audsvc],
                OutputLevel = DEBUG)
