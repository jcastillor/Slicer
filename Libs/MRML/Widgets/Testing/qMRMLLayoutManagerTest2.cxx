/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>
#include <QWidget>

// Slicer includes
#include "qMRMLLayoutManager.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>

// VTK includes
#include <vtkNew.h>

// Common test driver includes
#include "qMRMLWidgetCxxTests.h"

namespace
{

// --------------------------------------------------------------------------
bool checkViewArrangement(int line, qMRMLLayoutManager* layoutManager,
                          vtkMRMLLayoutNode * layoutNode, int expectedViewArrangement)
{
  if (layoutManager->layout() != expectedViewArrangement ||
      layoutNode->GetViewArrangement() != expectedViewArrangement)
    {
    std::cerr << "Line " << line << " - Add scene failed:\n"
              << " expected ViewArrangement: " << expectedViewArrangement << "\n"
              << " current ViewArrangement: " << layoutNode->GetViewArrangement() << "\n"
              << " current layout: " << layoutManager->layout() << std::endl;
    return false;
    }
  return true;
}

} // end of anonymous namespace

// --------------------------------------------------------------------------
int qMRMLLayoutManagerTest2(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  QWidget w;
  w.show();
  qMRMLLayoutManager* layoutManager = new qMRMLLayoutManager(&w, &w);

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;

  {
    vtkNew<vtkMRMLScene> scene;
    applicationLogic->SetMRMLScene(scene.GetPointer());
    layoutManager->setMRMLScene(scene.GetPointer());
    if (layoutManager->mrmlScene() != scene.GetPointer())
      {
      std::cerr << __LINE__ << " Problem with setMRMLScene()" << std::endl;
      return EXIT_FAILURE;
      }

    layoutManager->setMRMLScene(0);
    applicationLogic->SetMRMLScene(0);

    int current = scene->GetReferenceCount();
    int expected = 1;
    if (current != expected)
      {
      std::cerr << __LINE__ << " Problem with MRMLScene reference count !\n"
                << "  current: " << current << "\n"
                << "  expected: " << expected << std::endl;
      return EXIT_FAILURE;
      }
  }

  vtkMRMLLayoutNode* layoutNode = 0;
  {
    vtkNew<vtkMRMLScene> scene;
    vtkNew<vtkMRMLLayoutNode> newLayoutNode;

    // The view arrangement can be set before the view descriptions are registered, but it will log warning
    TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
    newLayoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
    TESTING_OUTPUT_ASSERT_WARNINGS_END();
    
    layoutNode = vtkMRMLLayoutNode::SafeDownCast(scene->AddNode(newLayoutNode.GetPointer()));
    applicationLogic->SetMRMLScene(scene.GetPointer());
    layoutManager->setMRMLScene(scene.GetPointer());
  }

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }

  layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutCompareView);
  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutCompareView))
    {
    return EXIT_FAILURE;
    }

  vtkMRMLScene * scene = layoutManager->mrmlScene();

  scene->StartState(vtkMRMLScene::ImportState);
  scene->EndState(vtkMRMLScene::ImportState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutCompareView))
    {
    return EXIT_FAILURE;
    }

  scene->StartState(vtkMRMLScene::ImportState);
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  scene->EndState(vtkMRMLScene::ImportState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }

  scene->StartState(vtkMRMLScene::CloseState);
  scene->EndState(vtkMRMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }

  scene->StartState(vtkMRMLScene::CloseState);
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  scene->EndState(vtkMRMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager,layoutNode,  vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  // The layout is changed to none only if vtkMRMLScene::Clear() is called
  scene->StartState(vtkMRMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  // Imitates what vtkMRMLScene::Clear() would have done:
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutNone);

  // and restore it back
  scene->EndState(vtkMRMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }
  return app.exec();
}

