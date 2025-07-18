/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"

// Terminologies includes
#include "qSlicerTerminologyItemDelegate.h"
#include "vtkSlicerTerminologiesModuleLogic.h"

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>
#include <QColor>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractModuleWidget.h"

// MRML includes
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin::qSlicerSubjectHierarchyAbstractPlugin(QObject* parent)
  : Superclass(parent)
  , m_Name(QString())
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin::~qSlicerSubjectHierarchyAbstractPlugin() = default;

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::name() const
{
  if (m_Name.isEmpty())
  {
    qCritical() << Q_FUNC_INFO << ": Empty plugin name";
  }
  return this->m_Name;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::setName(QString name)
{
  Q_UNUSED(name);
  qCritical() << Q_FUNC_INFO << ": Cannot set plugin name by method, only in constructor";
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyAbstractPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID) const
{
  Q_UNUSED(itemID);

  // Default implementation applies to plugins that do not define roles, only functions and/or levels
  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyAbstractPlugin::roleForPlugin() const
{
  // Default implementation applies to plugins that do not define roles, only functions and/or levels
  return QString("N/A");
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyAbstractPlugin::helpText() const
{
  // No need to define this function if there is no help text for a plugin
  return QString();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyAbstractPlugin::icon(vtkIdType itemID)
{
  Q_UNUSED(itemID);

  // Default implementation applies to plugins that do not define roles, only functions and/or levels
  // If there is no role, then there is no icon to set
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyAbstractPlugin::visibilityIcon(int visible)
{
  Q_UNUSED(visible);

  // Default implementation applies to plugins that do not define roles, only functions and/or levels
  // If there is no role, then there is no visibility icon to set
  return QIcon();
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::canEditProperties(vtkIdType itemID)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
  }
  vtkMRMLNode* node = shNode->GetItemDataNode(itemID);
  if (!node)
  {
    // default implementation can only edit associated nodes
    return false;
  }
  double confidence = 0.0;
  QString moduleForEditProperties = qSlicerApplication::application()->nodeModule(node, &confidence);
  return !moduleForEditProperties.isEmpty() && confidence > 0.0;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::editProperties(vtkIdType itemID)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  qSlicerApplication::application()->openNodeModule(shNode->GetItemDataNode(itemID));
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyAbstractPlugin::itemContextMenuActions() const
{
  return QList<QAction*>();
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyAbstractPlugin::sceneContextMenuActions() const
{
  return QList<QAction*>();
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyAbstractPlugin::visibilityContextMenuActions() const
{
  return QList<QAction*>();
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyAbstractPlugin::transformContextMenuActions() const
{
  return QList<QAction*>();
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyAbstractPlugin::viewContextMenuActions() const
{
  return QList<QAction*>();
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyAbstractPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID /*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/) const
{
  Q_UNUSED(node);
  Q_UNUSED(parentItemID);

  // Only role plugins can add node to the hierarchy, so default is 0
  return 0.0;
}

//----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy(vtkMRMLNode* nodeToAdd, vtkIdType parentItemID)
{
  if (!nodeToAdd)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid node to add";
    return false;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
  }

  // If parent is invalid, then add it under the scene
  if (!parentItemID)
  {
    parentItemID = shNode->GetSceneItemID();
  }

  // Create subject hierarchy item with added node
  vtkIdType addedItemID = shNode->CreateItem(parentItemID, nodeToAdd, this->m_Name.toUtf8());
  if (!addedItemID)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to add subject hierarchy item for data node " << nodeToAdd->GetName();
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyAbstractPlugin::canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID) const
{
  Q_UNUSED(itemID);
  Q_UNUSED(parentItemID);

  // Many plugins do not perform steps additional to the default
  // when reparenting inside the hierarchy, so return 0 by default
  return 0.0;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::reparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
  }

  shNode->SetItemParent(itemID, parentItemID);
  return true;
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::displayedItemName(vtkIdType itemID) const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QString();
  }

  return QString(shNode->GetItemName(itemID).c_str());
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::tooltip(vtkIdType itemID) const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QString();
  }

  // Display node type and level in the tooltip
  QString tooltipString("");
  vtkMRMLNode* dataNode = shNode->GetItemDataNode(itemID);
  if (dataNode)
  {
    tooltipString.append(dataNode->GetNodeTagName());
  }

  return tooltipString;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }

  shNode->SetItemDisplayVisibility(itemID, visible);
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyAbstractPlugin::getDisplayVisibility(vtkIdType itemID) const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0;
  }

  return shNode->GetItemDisplayVisibility(itemID);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::setDisplayColor(vtkIdType itemID, QColor color, QMap<int, QVariant> terminologyMetaData)
{
  Q_UNUSED(itemID);
  Q_UNUSED(color);
  Q_UNUSED(terminologyMetaData);
}

//-----------------------------------------------------------------------------
QColor qSlicerSubjectHierarchyAbstractPlugin::getDisplayColor(vtkIdType itemID, QMap<int, QVariant>& terminologyMetaData) const
{
  Q_UNUSED(itemID);
  Q_UNUSED(terminologyMetaData);

  // Default is transparent
  return QColor(0, 0, 0, 0);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::setColorAndTerminologyToDisplayableNode(vtkIdType itemID,
                                                                                    QColor color,
                                                                                    QMap<int, QVariant> terminologyMetaData,
                                                                                    bool useSelectedColor,
                                                                                    bool disableScalarVisibility)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }

  // Get display node
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!displayableNode)
  {
    qCritical() << Q_FUNC_INFO << ": Unable to find node for subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return;
  }
  vtkMRMLDisplayNode* displayNode = displayableNode->GetDisplayNode();
  if (!displayNode)
  {
    qCritical() << Q_FUNC_INFO << ": No display node";
    return;
  }

  // Set terminology metadata
  if (terminologyMetaData.contains(qSlicerTerminologyItemDelegate::TerminologyRole))
  {
    vtkSlicerTerminologiesModuleLogic::SetTerminologyEntryAsString(displayableNode, terminologyMetaData[qSlicerTerminologyItemDelegate::TerminologyRole].toString().toStdString());
  }
  if (terminologyMetaData.contains(qSlicerTerminologyItemDelegate::DefaultTerminologyRole))
  {
    vtkSlicerTerminologiesModuleLogic::SetDefaultTerminologyEntryAsString(displayableNode,
                                                                          terminologyMetaData[qSlicerTerminologyItemDelegate::DefaultTerminologyRole].toString().toStdString());
  }
  if (terminologyMetaData.contains(qSlicerTerminologyItemDelegate::NameRole))
  {
    displayableNode->SetName(terminologyMetaData[qSlicerTerminologyItemDelegate::NameRole].toString().toUtf8().constData());
  }
  if (terminologyMetaData.contains(qSlicerTerminologyItemDelegate::NameAutoGeneratedRole))
  {
    displayableNode->SetAttribute(vtkSlicerTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName(),
                                  terminologyMetaData[qSlicerTerminologyItemDelegate::NameAutoGeneratedRole].toString().toUtf8().constData());
  }
  if (terminologyMetaData.contains(qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole))
  {
    displayableNode->SetAttribute(vtkSlicerTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName(),
                                  terminologyMetaData[qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole].toString().toUtf8().constData());
  }

  // Set color
  double* oldColorArray = displayNode->GetColor();
  QColor oldColor = QColor::fromRgbF(oldColorArray[0], oldColorArray[1], oldColorArray[2]);
  if (oldColor != color)
  {
    if (useSelectedColor)
    {
      displayNode->SetSelectedColor(color.redF(), color.greenF(), color.blueF());
    }
    else
    {
      displayNode->SetColor(color.redF(), color.greenF(), color.blueF());
    }
    if (disableScalarVisibility)
    {
      // Solid color is set, therefore disable scalar visibility
      // (otherwise color would come from the scalar value and colormap).
      displayNode->SetScalarVisibility(false);
    }
    // Trigger update of color swatch
    shNode->ItemModified(itemID);
  }
}

//-----------------------------------------------------------------------------
QColor qSlicerSubjectHierarchyAbstractPlugin::colorAndTerminologyFromDisplayableNode(vtkIdType itemID, QMap<int, QVariant>& terminologyMetaData, bool useSelectedColor) const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QColor(0, 0, 0, 0);
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QColor(0, 0, 0, 0);
  }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return QColor(0, 0, 0, 0);
  }

  if (scene->IsImporting())
  {
    // During import SH node may be created before the segmentation is read into the scene,
    // so don't attempt to access the segment yet
    return QColor(0, 0, 0, 0);
  }

  // Get display node
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!displayableNode)
  {
    qCritical() << Q_FUNC_INFO << ": Unable to find node for subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return QColor(0, 0, 0, 0);
  }
  vtkMRMLDisplayNode* displayNode = displayableNode->GetDisplayNode();
  if (!displayNode)
  {
    // this is normal when the markups node is being created
    return QColor(0, 0, 0, 0);
  }

  // Get terminology metadata
  terminologyMetaData.clear();
  terminologyMetaData[qSlicerTerminologyItemDelegate::TerminologyRole] = QString::fromStdString(vtkSlicerTerminologiesModuleLogic::GetTerminologyEntryAsString(displayableNode));
  terminologyMetaData[qSlicerTerminologyItemDelegate::DefaultTerminologyRole] =
    QString::fromStdString(vtkSlicerTerminologiesModuleLogic::GetDefaultTerminologyEntryAsString(displayableNode));
  terminologyMetaData[qSlicerTerminologyItemDelegate::NameRole] = displayableNode->GetName();
  // If auto generated flags are not initialized, then set them to the default
  // (color: on, name: off - this way color will be set from the selector but name will not)
  bool nameAutoGenerated = false;
  if (displayableNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName()))
  {
    nameAutoGenerated = QVariant(displayableNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName())).toBool();
  }
  terminologyMetaData[qSlicerTerminologyItemDelegate::NameAutoGeneratedRole] = nameAutoGenerated;
  bool colorAutoGenerated = true;
  if (displayableNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName()))
  {
    colorAutoGenerated = QVariant(displayableNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName())).toBool();
  }
  terminologyMetaData[qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole] = colorAutoGenerated;

  // Get and return color
  double* colorArray{ nullptr };
  if (useSelectedColor)
  {
    colorArray = displayNode->GetSelectedColor();
  }
  else
  {
    colorArray = displayNode->GetColor();
  }
  return QColor::fromRgbF(colorArray[0], colorArray[1], colorArray[2]);
}

//--------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::isThisPluginOwnerOfItem(vtkIdType itemID) const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
  }

  return !shNode->GetItemOwnerPluginName(itemID).compare(this->m_Name.toUtf8().constData());
}

//--------------------------------------------------------------------------
qSlicerAbstractModuleWidget* qSlicerSubjectHierarchyAbstractPlugin::switchToModule(QString moduleName)
{
  // Find module with name
  qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module(moduleName);
  qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
  if (!moduleWithAction)
  {
    qCritical() << Q_FUNC_INFO << ": Module with name '" << moduleName << "' not found";
    return nullptr;
  }

  // Switch to module
  moduleWithAction->widgetRepresentation(); // Make sure it's created before showing
  moduleWithAction->action()->trigger();

  // Get module widget
  return dynamic_cast<qSlicerAbstractModuleWidget*>(moduleWithAction->widgetRepresentation());
}

//--------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::hideAllContextMenuActions() const
{
  QList<QAction*> allActions;
  allActions << this->sceneContextMenuActions();
  allActions << this->itemContextMenuActions();
  allActions << this->transformContextMenuActions();
  allActions << this->visibilityContextMenuActions();
  allActions << this->viewContextMenuActions();

  for (QAction* const action : allActions)
  {
    action->setVisible(false);
  }
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::showItemInView(vtkIdType itemID, vtkMRMLAbstractViewNode* viewNode, vtkIdList* vtkNotUsed(allItemsToShow))
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
  }
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!displayableNode)
  {
    return false;
  }
  displayableNode->CreateDefaultDisplayNodes();
  vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(displayableNode->GetDisplayNode());
  if (!displayNode || !displayNode->IsShowModeDefault())
  {
    return false;
  }
  if (viewNode)
  {
    // Show in specific view
    MRMLNodeModifyBlocker blocker(displayNode);
    if (!displayNode->GetVisibility())
    {
      displayNode->SetVisibility(true);
      // This was hidden in all views, show it only in the currently selected view
      displayNode->RemoveAllViewNodeIDs();
    }
    displayNode->AddViewNodeID(viewNode->GetID());
    if (displayNode->GetOpacity() <= 0.0)
    {
      displayNode->SetOpacity(1.0);
    }
    if (vtkMRMLSliceNode::SafeDownCast(viewNode))
    {
      displayNode->SetVisibility2D(true);
    }
    if (vtkMRMLViewNode::SafeDownCast(viewNode))
    {
      displayNode->SetVisibility3D(true);
    }
  }
  else
  {
    // Show in all views
    MRMLNodeModifyBlocker blocker(displayNode);
    displayNode->RemoveAllViewNodeIDs();
    displayNode->SetVisibility(true);
    if (displayNode->GetOpacity() <= 0.0)
    {
      displayNode->SetOpacity(1.0);
    }
    displayNode->SetVisibility2D(true);
    displayNode->SetVisibility3D(true);
  }
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(QAction* action, int section, int weight /*=0*/, double weightAdjustment /*=0.0*/)
{
  if (!action)
  {
    qWarning() << Q_FUNC_INFO << " failed: invalid action";
    return;
  }
  action->setProperty("section", section + weight * 0.01 + weightAdjustment * 0.0001);
}
