#include <pxr/usd/usdAnimX/fileFormat.h>
#include <pxr/usd/usdAnimX/types.h>
#include <pxr/usd/usdAnimX/desc.h>
#include <pxr/usd/usdAnimX/data.h>  
#include <pxr/usd/usdAnimX/keyframe.h>


// animx dirty test
//----------------------------------------------------------------------------
static
pxr::UsdStageRefPtr
TestAnimXFromFile(const std::string& filename, CurveEditorUI* curveEditor)
{
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename);
  pxr::SdfLayerHandleVector layers = stage->GetLayerStack();
  for (auto& layer : layers) {
    if (layer->GetFileFormat()->GetFormatId() == pxr::UsdAnimXFileFormatTokens->Id) {
      curveEditor->SetLayer(layer);
      break;
    }
  }
  return stage;
}

static
pxr::UsdStageRefPtr 
TestAnimX(CurveEditorUI* curveEditor)
{
  pxr::SdfLayerRefPtr rootLayer = pxr::SdfLayer::CreateAnonymous("shot.usda");
  pxr::SdfLayerRefPtr geomLayer = pxr::SdfLayer::CreateAnonymous("geom.usda");
  pxr::SdfLayerRefPtr animLayer = pxr::SdfLayer::CreateAnonymous("anim.animx");

  rootLayer->InsertSubLayerPath(geomLayer->GetIdentifier());
  rootLayer->InsertSubLayerPath(animLayer->GetIdentifier());
     
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(rootLayer->GetIdentifier());
  stage->SetStartTimeCode(1);
  stage->SetEndTimeCode(100);

  stage->SetEditTarget(geomLayer);
  std::cout << "SET METADATAS DONE..." << std::endl;

  pxr::SdfPath primPath("/Cube");
  pxr::UsdGeomCube cube =
    pxr::UsdGeomCube::Define(stage, primPath);

  stage->SetEditTarget(animLayer);

  pxr::UsdAnimXFileFormatConstPtr fileFormat = 
    pxr::TfStatic_cast<pxr::UsdAnimXFileFormatConstPtr>(animLayer->GetFileFormat());
  pxr::SdfAbstractDataConstPtr datas = fileFormat->GetData(&(*animLayer));
  pxr::UsdAnimXDataPtr animXDatas = 
    pxr::TfDynamic_cast<pxr::UsdAnimXDataPtr>(
      pxr::TfConst_cast<pxr::SdfAbstractDataPtr>(datas));
  std::cout << "[ANIMX] DATAS : " << animXDatas->GetCurrentCount() << std::endl;

    
  std::cout << "[ANIMX] FILE FORMAT : " << fileFormat->GetFormatId() << std::endl;
  
  animXDatas->AddPrim(primPath);
  pxr::UsdAnimXOpDesc opDesc;
  opDesc.defaultValue = pxr::VtValue((double)1.0);
  opDesc.target = pxr::TfToken("size");
  opDesc.dataType = pxr::AnimXGetTokenFromSdfValueTypeName(opDesc.defaultValue.GetType());
  opDesc.name = pxr::TfToken("sizeOp");
  animXDatas->AddOp(primPath, opDesc);

  pxr::UsdAnimXCurveDesc curveDesc;
  curveDesc.name = pxr::TfToken("size");
  curveDesc.postInfinityType = pxr::UsdAnimXTokens->cycle;
  curveDesc.preInfinityType = pxr::UsdAnimXTokens->cycle;

  pxr::UsdAnimXKeyframeDesc keyframeDesc;
  keyframeDesc.time = 1;
  keyframeDesc.data[0] = 1.0;
  keyframeDesc.data[1] = (double)adsk::TangentType::Fixed;
  keyframeDesc.data[2] = 24.0;
  keyframeDesc.data[3] = 0.0;
  keyframeDesc.data[4] = (double)adsk::TangentType::Fixed;
  keyframeDesc.data[5] = 24.0;
  keyframeDesc.data[6] = 0.0;
  keyframeDesc.data[7] = 1.0;
  curveDesc.keyframes.push_back(keyframeDesc);

  keyframeDesc.time = 12;
  keyframeDesc.data[0] = 1.0;
  curveDesc.keyframes.push_back(keyframeDesc);

  keyframeDesc.time = 25;
  keyframeDesc.data[0] = 10.0;
  curveDesc.keyframes.push_back(keyframeDesc);

  keyframeDesc.time = 50;
  keyframeDesc.data[0] = 1.0;
  curveDesc.keyframes.push_back(keyframeDesc);

  animXDatas->AddFCurve(primPath, opDesc.target, curveDesc);
  animLayer->Export("C:/Users/graph/Documents/bmal/src/Amnesie/assets/test.animx");
  curveEditor->SetLayer(pxr::SdfLayerHandle(animLayer));
  stage->SetEditTarget(geomLayer);

  return stage;

  /*
  for (size_t i = 0; i < _filenames.size(); ++i)
  {
    std::string filename = _filenames[i];
    if (pxr::UsdStage::IsSupportedFile(filename) &&
      pxr::ArchGetFileLength(filename.c_str()) != -1)
    {
      pxr::SdfLayerRefPtr subLayer = SdfLayer::FindOrOpen(filename);
      _layers.push_back(subLayer);
      _rootLayer->InsertSubLayerPath(subLayer->GetIdentifier());
    }
  }
  

  //pxr::SdfLayerRefPtr rootLayer = pxr::SdfLayer::FindOrOpen(_filename);
  //_stage = pxr::UsdStage::Open(rootLayer);
  _stage = pxr::UsdStage::Open(_rootLayer->GetIdentifier());
  _stage->SetEditTarget(_rootLayer);
  */
}