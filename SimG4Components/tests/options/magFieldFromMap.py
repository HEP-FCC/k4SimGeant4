import os

testfile = open("testfield.txt", "w")
testfile.write("% Dimension:          2\n")
testfile.write("% Nodes:              3\n")
testfile.write("0.05 -49.875 1.1873149775644519E-7 0 1.7788740730633446E-6 1.7828320550114816E-6\n")
testfile.write("0.05 -49.625 6.65904798008334E-8 0 3.466329209253203E-6 3.4669687738602493E-6\n")
testfile.write("0.05 -49.375 1.4449461845226574E-8 0 5.414489435220289E-6 5.414508715577041E-6")
testfile.close()


from Gaudi.Configuration import INFO, DEBUG
from GaudiKernel import SystemOfUnits as units
from GaudiKernel import PhysicalConstants as constants

from Configurables import ApplicationMgr
ApplicationMgr().EvtSel = 'NONE'
ApplicationMgr().EvtMax = 2
ApplicationMgr().OutputLevel = INFO


from Configurables import GeoSvc
geoservice = GeoSvc("GeoSvc")
path_to_detectors = os.environ.get("FCCDETECTORS", "")
detectors_to_use = [
    'Detector/DetFCCeeIDEA-LAr/compact/FCCee_DectMaster.xml',
]
geoservice.detectors = [os.path.join(path_to_detectors, _det) for _det in detectors_to_use]
geoservice.OutputLevel = INFO
ApplicationMgr().ExtSvc += [geoservice]


from Configurables import SimG4MagneticFieldFromMapTool
field = SimG4MagneticFieldFromMapTool("SimG4MagneticFieldFromMapTool")
field.MapFile = "testfield.txt"
field.FieldOn = True
field.IntegratorStepper = "ClassicalRK4"
field.OutputLevel = DEBUG


from Configurables import SimG4Svc
geantservice = SimG4Svc("SimG4Svc")
geantservice.magneticField = field
geantservice.OutputLevel = DEBUG
ApplicationMgr().ExtSvc += [geantservice]
