// glTF 2.0 reader (IO/Geometry/vtkGLTFReader + vtkGLTFDocumentLoader): .gltf JSON with data: URIs
// and .glb binary containers (the reader sniffs the 'glTF' magic itself), accessors/buffer views,
// animations, skins and morph targets applied to the geometry.
#include "common.h"
#include "vtkGLTFReader.h"
#include "vtkMemoryResourceStream.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  vtkFuzzQuiet();
  vtkNew<vtkMemoryResourceStream> stream;
  stream->SetBuffer(data, size);
  vtkNew<vtkGLTFReader> reader;
  reader->SetStream(stream);
  reader->ApplyDeformationsToGeometryOn();
  reader->UpdateInformation();
  for (vtkIdType i = 0; i < reader->GetNumberOfAnimations() && i < 64; ++i)
  {
    reader->EnableAnimation(i);
  }
  reader->Update();
  vtkFuzzTouch(reader->GetOutputDataObject(0));
  return 0;
}
