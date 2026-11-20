// Known-answer driver for mayhem/test.sh: reads one file with the named VTK reader and prints
// what it parsed (point/cell counts, bounds, first array name), so test.sh can compare against
// golden values.
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataSet.h"
#include "vtkGLTFReader.h"
#include "vtkGenericDataObjectReader.h"
#include "vtkNew.h"
#include "vtkOBJReader.h"
#include "vtkPLYReader.h"
#include "vtkPointData.h"
#include "vtkSTLReader.h"
#include "vtkSmartPointer.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLPolyDataReader.h"

#include <cstdio>
#include <cstring>

static void count(vtkDataObject* obj, long long& np, long long& nc, double b[6], int& blocks)
{
  if (auto* ds = vtkDataSet::SafeDownCast(obj))
  {
    np += ds->GetNumberOfPoints();
    nc += ds->GetNumberOfCells();
    double bb[6];
    ds->GetBounds(bb);
    for (int i = 0; i < 3; ++i)
    {
      if (bb[2 * i] < b[2 * i]) b[2 * i] = bb[2 * i];
      if (bb[2 * i + 1] > b[2 * i + 1]) b[2 * i + 1] = bb[2 * i + 1];
    }
    ++blocks;
  }
  else if (auto* cds = vtkCompositeDataSet::SafeDownCast(obj))
  {
    vtkSmartPointer<vtkCompositeDataIterator> it;
    it.TakeReference(cds->NewIterator());
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
    {
      count(it->GetCurrentDataObject(), np, nc, b, blocks);
    }
  }
}

int main(int argc, char** argv)
{
  if (argc != 3)
  {
    std::fprintf(stderr, "usage: %s legacy|vtu|vtp|ply|stl|obj|gltf <file>\n", argv[0]);
    return 2;
  }
  const char* kind = argv[1];
  const char* path = argv[2];
  vtkSmartPointer<vtkAlgorithm> alg;
  if (!std::strcmp(kind, "legacy"))
  {
    auto r = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    r->SetFileName(path);
    alg = r;
  }
  else if (!std::strcmp(kind, "vtu"))
  {
    auto r = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    r->SetFileName(path);
    alg = r;
  }
  else if (!std::strcmp(kind, "vtp"))
  {
    auto r = vtkSmartPointer<vtkXMLPolyDataReader>::New();
    r->SetFileName(path);
    alg = r;
  }
  else if (!std::strcmp(kind, "ply"))
  {
    auto r = vtkSmartPointer<vtkPLYReader>::New();
    r->SetFileName(path);
    alg = r;
  }
  else if (!std::strcmp(kind, "stl"))
  {
    auto r = vtkSmartPointer<vtkSTLReader>::New();
    r->SetFileName(path);
    alg = r;
  }
  else if (!std::strcmp(kind, "obj"))
  {
    auto r = vtkSmartPointer<vtkOBJReader>::New();
    r->SetFileName(path);
    alg = r;
  }
  else if (!std::strcmp(kind, "gltf"))
  {
    auto r = vtkSmartPointer<vtkGLTFReader>::New();
    r->SetFileName(path);
    alg = r;
  }
  else
  {
    std::fprintf(stderr, "unknown kind %s\n", kind);
    return 2;
  }
  alg->Update();
  long long np = 0, nc = 0;
  int blocks = 0;
  double b[6] = { 1e300, -1e300, 1e300, -1e300, 1e300, -1e300 };
  count(alg->GetOutputDataObject(0), np, nc, b, blocks);
  std::printf("blocks=%d points=%lld cells=%lld", blocks, np, nc);
  if (np > 0)
  {
    std::printf(" bounds=%g,%g,%g,%g,%g,%g", b[0], b[1], b[2], b[3], b[4], b[5]);
  }
  if (auto* ds = vtkDataSet::SafeDownCast(alg->GetOutputDataObject(0)))
  {
    std::printf(" point_arrays=%d", ds->GetPointData()->GetNumberOfArrays());
  }
  std::printf("\n");
  return 0;
}
