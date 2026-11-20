// PLY reader (IO/PLY): header, element/property/list parsing, ASCII and binary.
#include "common.h"
#include "vtkPLYReader.h"
#include "vtkPolyData.h"

#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  vtkFuzzQuiet();
  vtkNew<vtkPLYReader> reader;
  reader->ReadFromInputStringOn();
  reader->SetInputString(std::string(reinterpret_cast<const char*>(data), size));
  reader->Update();
  vtkFuzzTouch(reader->GetOutput());
  return 0;
}
