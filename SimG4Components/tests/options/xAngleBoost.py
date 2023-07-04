from Gaudi.Configuration import INFO, DEBUG
from GaudiKernel import SystemOfUnits as units
from GaudiKernel import PhysicalConstants as constants

from Configurables import ApplicationMgr
ApplicationMgr().EvtSel = 'NONE'
ApplicationMgr().EvtMax = 2
ApplicationMgr().OutputLevel = INFO

from Configurables import k4LegacyDataSvc
podioevent = k4LegacyDataSvc("EventDataSvc")
ApplicationMgr().ExtSvc += [podioevent]


from Configurables import MomentumRangeParticleGun
guntool = MomentumRangeParticleGun()
guntool.ThetaMin = 45 * constants.pi / 180.
guntool.ThetaMax = 135 * constants.pi / 180.
guntool.PhiMin = 0.
guntool.PhiMax = 2. * constants.pi
guntool.MomentumMin = 10. * units.GeV
guntool.MomentumMax = 10. * units.GeV
guntool.PdgCodes = [11]

from Configurables import GenAlg
gun = GenAlg()
gun.SignalProvider = guntool
gun.hepmc.Path = "hepmc"
ApplicationMgr().TopAlg += [gun]


from Configurables import HepMCToEDMConverter
hepmc_converter = HepMCToEDMConverter()
hepmc_converter.hepmc.Path = "hepmc"
hepmc_converter.GenParticles.Path = "GenParticles"
ApplicationMgr().TopAlg += [hepmc_converter]


from Configurables import SimG4CrossingAngleBoost
xAngleBoost = SimG4CrossingAngleBoost('xAngleBoost')
xAngleBoost.InParticles = 'GenParticles'
xAngleBoost.OutParticles = 'BoostedParticles'
xAngleBoost.CrossingAngle = 0.015  # rad
xAngleBoost.OutputLevel = DEBUG
ApplicationMgr().TopAlg += [xAngleBoost]


from Configurables import PodioLegacyOutput
output = PodioLegacyOutput("output")
output.filename = "output_xAngleBoost.root"
output.outputCommands = ["keep *"]
ApplicationMgr().TopAlg += [output]