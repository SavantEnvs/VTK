// VTK XML readers (IO/XML + IO/XMLParser): inline/appended/base64/raw data, zlib/lz4/lzma
// decompression. The first byte selects the reader class; the rest is the XML document.
#include "common.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLMultiBlockDataReader.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLRectilinearGridReader.h"
#include "vtkXMLStructuredGridReader.h"
#include "vtkXMLTableReader.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLHyperTreeGridReader.h"

#include <string>

template <class R>
static void run(const char* buf, size_t len)
{
  vtkNew<R> reader;
  reader->ReadFromInputStringOn();
  reader->SetInputString(std::string(buf, len));
  reader->Update();
  vtkFuzzTouch(reader->GetOutputDataObject(0));
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  vtkFuzzQuiet();
  if (size < 1)
  {
    return -1;
  }
  const char* buf = reinterpret_cast<const char*>(data + 1);
  size_t len = size - 1;
  switch (data[0] % 8)
  {
    case 0: run<vtkXMLUnstructuredGridReader>(buf, len); break;
    case 1: run<vtkXMLPolyDataReader>(buf, len); break;
    case 2: run<vtkXMLImageDataReader>(buf, len); break;
    case 3: run<vtkXMLRectilinearGridReader>(buf, len); break;
    case 4: run<vtkXMLStructuredGridReader>(buf, len); break;
    case 5: run<vtkXMLTableReader>(buf, len); break;
    case 6: run<vtkXMLMultiBlockDataReader>(buf, len); break;
    default: run<vtkXMLHyperTreeGridReader>(buf, len); break;
  }
  return 0;
}
