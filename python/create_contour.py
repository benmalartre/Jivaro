from pxr import Usd, Sdf, UsdGeom
import UsdNpr

stage = usdviewApi.stage

camera = UsdGeom.Camera.Define(stage, "/camera")
camera.CreateProjectionAttr().Set(UsdGeom.Tokens.perspective)
op = camera.AddTranslateOp(UsdGeom.XformOp.PrecisionFloat)
op.Set((0.0,6.0,60.0))

contour = UsdNpr.Contour.Define(stage, "/contour")

viewpoint = contour.GetContourViewPointRel()
viewpoint.SetTargets([camera.GetPrim().GetPath()])

surfaces = contour.GetContourSurfacesRel()
print(surfaces)

meshes = [prim.GetPath() for prim in stage.TraverseAll() if prim.GetTypeName() == "Mesh"]
surfaces.SetTargets(meshes)


