/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
 *                                                                         *
 ***************************************************************************
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 **************************************************************************

 ************************************************************************
 **
 **  @file   vtoolpointofintersection.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vtoolpointofintersection.h"

#include <QPointF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <new>

#include "../../../../dialogs/tools/dialogpointofintersection.h"
#include "../../../../visualization/line/vistoolpointofintersection.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/visualization.h"
#include "../../../vabstracttool.h"
#include "../../vdrawtool.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;

const QString VToolPointOfIntersection::ToolType = QStringLiteral("pointOfIntersection");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolPointOfIntersection constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param firstPointId id first line point.
 * @param secondPointId id second line point.
 * @param typeCreation way we create this tool.
 * @param parent parent object.
 */
VToolPointOfIntersection::VToolPointOfIntersection(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                                   const quint32 &firstPointId, const quint32 &secondPointId,
                                                   const Source &typeCreation, QGraphicsItem *parent)
    : VToolSinglePoint(doc, data, id, QColor(qApp->Settings()->getPointNameColor()), parent)
    , firstPointId(firstPointId)
    , secondPointId(secondPointId)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolPointOfIntersection::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogPointOfIntersection> dialogTool = m_dialog.objectCast<DialogPointOfIntersection>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetFirstPointId(firstPointId);
    dialogTool->SetSecondPointId(secondPointId);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @return the created tool
 */
VToolPointOfIntersection *VToolPointOfIntersection::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                                           VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogPointOfIntersection> dialogTool = dialog.objectCast<DialogPointOfIntersection>();
    SCASSERT(not dialogTool.isNull())
    const quint32 firstPointId = dialogTool->GetFirstPointId();
    const quint32 secondPointId = dialogTool->GetSecondPointId();
    const QString pointName = dialogTool->getPointName();
    VToolPointOfIntersection *point = Create(0, pointName, firstPointId, secondPointId, 5, 10, true, scene, doc,
                                             data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param _id tool id, 0 if tool doesn't exist yet.
 * @param pointName point name.
 * @param firstPointId id first line point.
 * @param secondPointId id second line point.
 * @param mx label bias x axis.
 * @param my label bias y axis.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 * @return the created tool
 */
VToolPointOfIntersection *VToolPointOfIntersection::Create(const quint32 _id, const QString &pointName,
                                                           const quint32 &firstPointId, const quint32 &secondPointId,
                                                           qreal mx, qreal my, bool showPointName,
                                                           VMainGraphicsScene *scene, VAbstractPattern *doc,
                                                           VContainer *data, const Document &parse,
                                                           const Source &typeCreation)
{
    const QSharedPointer<VPointF> firstPoint = data->GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> secondPoint = data->GeometricObject<VPointF>(secondPointId);

    QPointF point(firstPoint->x(), secondPoint->y());
    quint32 id = _id;
    VPointF *p = new VPointF(point, pointName, mx, my);
    p->setShowPointName(showPointName);

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
    }
    else
    {
        data->UpdateGObject(id, p);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::PointOfIntersection, doc);
        VToolPointOfIntersection *point = new VToolPointOfIntersection(doc, data, id, firstPointId,
                                                                       secondPointId, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(firstPoint->getIdTool());
        doc->IncrementReferens(secondPoint->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolPointOfIntersection::FirstPointName() const
{
    return VAbstractTool::data.GetGObject(firstPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolPointOfIntersection::SecondPointName() const
{
    return VAbstractTool::data.GetGObject(secondPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement value of reference.
 */
void VToolPointOfIntersection::RemoveReferens()
{
    const auto firstPoint = VAbstractTool::data.GetGObject(firstPointId);
    const auto secondPoint = VAbstractTool::data.GetGObject(secondPointId);

    doc->DecrementReferens(firstPoint->getIdTool());
    doc->DecrementReferens(secondPoint->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VToolPointOfIntersection::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogPointOfIntersection>(event, id);
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolPointOfIntersection::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogPointOfIntersection> dialogTool = m_dialog.objectCast<DialogPointOfIntersection>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName, dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrFirstPoint, QString().setNum(dialogTool->GetFirstPointId()));
    doc->SetAttribute(domElement, AttrSecondPoint, QString().setNum(dialogTool->GetSecondPointId()));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersection::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrFirstPoint, firstPointId);
    doc->SetAttribute(tag, AttrSecondPoint, secondPointId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersection::ReadToolAttributes(const QDomElement &domElement)
{
    firstPointId = doc->GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
    secondPointId = doc->GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersection::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolPointOfIntersection *visual = qobject_cast<VisToolPointOfIntersection *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(firstPointId);
        visual->setPoint2Id(secondPointId);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolPointOfIntersection::GetSecondPointId() const
{
    return secondPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersection::SetSecondPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        secondPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersection::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolPointOfIntersection>(show);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolPointOfIntersection::GetFirstPointId() const
{
    return firstPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersection::SetFirstPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        firstPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}
