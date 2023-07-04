import os
from Gaudi.Configuration import INFO, DEBUG
from GaudiKernel.SystemOfUnits import tesla, m, cm

from Configurables import ApplicationMgr
ApplicationMgr().EvtSel = 'NONE'
ApplicationMgr().EvtMax = 1
ApplicationMgr().OutputLevel = INFO
ApplicationMgr().StopOnSignal = True
ApplicationMgr().ExtSvc += ['RndmGenSvc']

# Detector geometry
from Configurables import GeoSvc
geoservice = GeoSvc("GeoSvc")
path_to_detectors = os.environ.get("FCCDETECTORS", "")
detectors_to_use = [
    'Detector/DetFCCeeIDEA-LAr/compact/FCCee_DectMaster.xml',
]
geoservice.detectors = [os.path.join(path_to_detectors, det)
                        for det in detectors_to_use]
geoservice.OutputLevel = INFO
ApplicationMgr().ExtSvc += [geoservice]

# Magnetic field
from Configurables import SimG4ConstantMagneticFieldTool
field = SimG4ConstantMagneticFieldTool("SimG4ConstantMagneticFieldTool")
field.FieldComponentZ = -2 * tesla
field.FieldRMax = 150 * cm
field.FieldOn = True
field.IntegratorStepper="ClassicalRK4"
field.OutputLevel = INFO

# Geant4 service
from Configurables import SimG4Svc
geantservice = SimG4Svc("SimG4Svc")
geantservice.detector = "SimG4DD4hepDetector"
geantservice.physicslist = "SimG4FtfpBert"
geantservice.actions = "SimG4FullSimActions"
geantservice.magneticField = field
geantservice.OutputLevel = INFO
ApplicationMgr().ExtSvc += [geantservice]

# Magnetic field probes
from Configurables import MagFieldScanner
magfieldscanner = MagFieldScanner("MagFieldScanner")
magfieldscanner.outFilePath = "hello.root"
magfieldscanner.xyPlaneProbes = [
#   xMax,    yMax, z
    [160*cm, 1600, 0],
    [1600, 1600, -100],
    [1600, 1600, 100],
    [1600, 1600, -2100],
    [1600, 1600, 2100],
    [1600, 1600, -20100],
    [1600, 1600, 20100],
]
magfieldscanner.zPlaneProbes = [
#   zMin,     zMax,  rMax,   phi (angle from x-axis, in radians)
    [-20.1*m, 20100, 160*cm, 1.57079632679],
    [-20100, 20100, 1600, 0.],
]
magfieldscanner.tubeProbes = [
#   zMin,     zMax,  r
    [-20.1*m, 20100, 0],
    [-20.1*m, 20100, 10],
    [-20.1*m, 20100, 1600],
]
magfieldscanner.OutputLevel = DEBUG
ApplicationMgr().ExtSvc += [magfieldscanner]
