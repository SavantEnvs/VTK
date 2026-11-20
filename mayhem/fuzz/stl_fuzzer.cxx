// STL reader (IO/Geometry/vtkSTLReader): ASCII and binary STL, point merging, solid labels.
#include "common.h"
#include "vtkMemoryResourceStream.h"
#include "vtkPolyData.h"
#include "vtkSTLReader.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  vtkFuzzQuiet();
  vtkNew<vtkMemoryResourceStream> stream;
  stream->SetBuffer(data, size);
  vtkNew<vtkSTLReader> reader;
  reader->SetStream(stream);
  reader->MergingOn();
  reader->ScalarTagsOn();
  reader->Update();
  vtkFuzzTouch(reader->GetOutput());
  return 0;
}
