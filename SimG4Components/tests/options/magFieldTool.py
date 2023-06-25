import os

testcompact = open('testdet.xml', 'w')
testcompact.write('<?xml version="1.0" encoding="UTF-8"?>\n')
testcompact.write('<lccdd xmlns:compact="http://www.lcsim.org/schemas/compact/1.0"\n')
testcompact.write('       xmlns:xs="http://www.w3.org/2001/XMLSchema"\n')
testcompact.write('       xs:noNamespaceSchemaLocation="http://www.lcsim.org/schemas/compact/1.0/compact.xsd">\n\n')
testcompact.write('  <info name="Test-Det"\n')
testcompact.write('        title="Test-Det"\n')
testcompact.write('        author="Bender Bending Rodríguez"\n')
testcompact.write('        url="no"\n')
testcompact.write('        status="development"\n')
testcompact.write('        version="0.0">\n\n')
testcompact.write('    <comment>\n')
testcompact.write('      Compact file of Test-Det.\n')
testcompact.write('    </comment>\n')
testcompact.write('  </info>\n\n')
testcompact.write('  <materials>\n')
testcompact.write('    <element Z="1" formula="H" name="H" >\n')
testcompact.write('      <atom type="A" unit="g/mol" value="1.00794" />\n')
testcompact.write('    </element>\n')
testcompact.write('    <material formula="H" name="Hydrogen" state="gas" >\n')
testcompact.write('      <RL type="X0" unit="cm" value="752776" />\n')
testcompact.write('      <NIL type="lambda" unit="cm" value="421239" />\n')
testcompact.write('      <D type="density" unit="g/cm3" value="8.3748e-05" />\n')
testcompact.write('      <composite n="1" ref="H" />\n')
testcompact.write('    </material>\n\n')
testcompact.write('    <material name="Vacuum">\n')
testcompact.write('      <D type="density" unit="g/cm3" value="0.00000001" />\n')
testcompact.write('      <fraction n="1" ref="H" />\n')
testcompact.write('    </material>\n\n')
testcompact.write('    <material name="Air">\n')
testcompact.write('      <D type="density" unit="g/cm3" value="0.0012"/>\n')
testcompact.write('      <fraction n="1" ref="H"/>\n')
testcompact.write('    </material>\n')
testcompact.write('  </materials>\n\n')
testcompact.write('  <define>\n')
testcompact.write('    <constant name="world_size" value="25*m"/>\n')
testcompact.write('    <constant name="world_x" value="world_size"/>\n')
testcompact.write('    <constant name="world_y" value="world_size"/>\n')
testcompact.write('    <constant name="world_z" value="world_size"/>\n')
testcompact.write('  </define>\n\n')
testcompact.write('  <fields>\n')
testcompact.write('    <field name="QC1L1_field_ED" type="MultipoleMagnet" Z="0.0*tesla">\n')
testcompact.write('        <position y="0*cm" x="0*cm" z="0*cm"/>\n')
testcompact.write('        <rotation x="0.0" y="0.0" z="0.0"/>\n')
testcompact.write('        <coefficient coefficient="0*tesla"/>\n')
testcompact.write('        <coefficient coefficient="(-1)*(45.6)*(-0.273)/0.3*tesla/m"/>\n')
testcompact.write('        <shape type="Tube" rmin="0.*cm" rmax="150*cm" dz="2000*cm" />\n')
testcompact.write('    </field>\n')
testcompact.write('  </fields>\n')
testcompact.write('</lccdd>\n')
testcompact.close()


from Gaudi.Configuration import INFO, DEBUG

from Configurables import ApplicationMgr
ApplicationMgr().EvtSel = 'NONE'
ApplicationMgr().EvtMax = 1
ApplicationMgr().OutputLevel = INFO
ApplicationMgr().StopOnSignal = True
ApplicationMgr().ExtSvc += ['RndmGenSvc']

# Detector geometry
from Configurables import GeoSvc
geoservice = GeoSvc("GeoSvc")
geoservice.detectors = ['testdet.xml']
geoservice.OutputLevel = INFO
ApplicationMgr().ExtSvc += [geoservice]

# Magnetic field
from Configurables import SimG4MagneticFieldTool
field = SimG4MagneticFieldTool("MagneticFieldTool")
field.FieldOn = True
field.IntegratorStepper = "ClassicalRK4"
field.OutputLevel = DEBUG

# Geant4 service
from Configurables import SimG4Svc
geantservice = SimG4Svc("SimG4Svc")
geantservice.detector = "SimG4DD4hepDetector"
geantservice.physicslist = "SimG4FtfpBert"
geantservice.actions = "SimG4FullSimActions"
geantservice.magneticField = field
geantservice.OutputLevel = INFO
ApplicationMgr().ExtSvc += [geantservice]
