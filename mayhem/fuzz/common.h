// Shared helpers for the VTK fuzz harnesses.
#pragma once
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkCompositeDataIterator.h"
#include "vtkGenericCell.h"
#include "vtkNew.h"
#include "vtkObject.h"
#include "vtkOutputWindow.h"
#include "vtkSmartPointer.h"

#include <cstddef>
#include <cstdint>

// VTK error/warning macros print to stderr through vtkOutputWindow; silence them once.
// This is process-global but never changed by any input, so it is read-only state.
static inline void vtkFuzzQuiet()
{
  static bool done = false;
  if (!done)
  {
    vtkObject::GlobalWarningDisplayOff();
    done = true;
  }
}

// Walk every cell of a dataset, so lazily-built connectivity and cell storage are exercised.
static inline void vtkFuzzTouchDataSet(vtkDataSet* ds)
{
  if (!ds)
  {
    return;
  }
  vtkIdType nc = ds->GetNumberOfCells();
  vtkIdType np = ds->GetNumberOfPoints();
  vtkNew<vtkGenericCell> cell;
  for (vtkIdType i = 0; i < nc && i < 4096; ++i)
  {
    ds->GetCell(i, cell);
    (void)ds->GetCellType(i);
  }
  double b[6];
  if (np > 0)
  {
    ds->GetBounds(b);
  }
}

static inline void vtkFuzzTouch(vtkDataObject* obj)
{
  if (!obj)
  {
    return;
  }
  if (auto* ds = vtkDataSet::SafeDownCast(obj))
  {
    vtkFuzzTouchDataSet(ds);
    return;
  }
  if (auto* cds = vtkCompositeDataSet::SafeDownCast(obj))
  {
    vtkSmartPointer<vtkCompositeDataIterator> it;
    it.TakeReference(cds->NewIterator());
    int n = 0;
    for (it->InitTraversal(); !it->IsDoneWithTraversal() && n < 256; it->GoToNextItem(), ++n)
    {
      vtkFuzzTouchDataSet(vtkDataSet::SafeDownCast(it->GetCurrentDataObject()));
    }
  }
}
