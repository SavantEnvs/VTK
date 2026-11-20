// Legacy .vtk reader (IO/Legacy): vtkGenericDataObjectReader dispatches to the polydata,
// unstructured/structured grid, rectilinear grid, image, table, graph, tree and composite readers,
// ASCII and binary.
#include "common.h"
#include "vtkGenericDataObjectReader.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  vtkFuzzQuiet();
  if (size > 0x7fffffff) // SetInputString takes an int length
  {
    return -1;
  }
  vtkNew<vtkGenericDataObjectReader> reader;
  reader->ReadFromInputStringOn();
  reader->SetInputString(reinterpret_cast<const char*>(data), static_cast<int>(size));
  reader->ReadAllScalarsOn();
  reader->ReadAllVectorsOn();
  reader->ReadAllNormalsOn();
  reader->ReadAllTensorsOn();
  reader->ReadAllColorScalarsOn();
  reader->ReadAllTCoordsOn();
  reader->ReadAllFieldsOn();
  reader->Update();
  vtkFuzzTouch(reader->GetOutput());
  return 0;
}
