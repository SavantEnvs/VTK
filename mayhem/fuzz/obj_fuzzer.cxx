// Wavefront OBJ reader (IO/Geometry/vtkOBJReader): vertices, normals, tcoords, faces, groups.
#include "common.h"
#include "vtkMemoryResourceStream.h"
#include "vtkOBJReader.h"
#include "vtkPolyData.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  vtkFuzzQuiet();
  vtkNew<vtkMemoryResourceStream> stream;
  stream->SetBuffer(data, size);
  vtkNew<vtkOBJReader> reader;
  reader->SetStream(stream);
  reader->Update();
  vtkFuzzTouch(reader->GetOutput());
  return 0;
}
