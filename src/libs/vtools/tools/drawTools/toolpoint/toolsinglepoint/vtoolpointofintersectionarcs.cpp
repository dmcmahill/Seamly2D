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
 **  @file   vtoolpointofintersectionarcs.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 5, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Seamly2D project
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

#include "vtoolpointofintersectionarcs.h"

#include <QLineF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <new>

#include "../../../../dialogs/tools/dialogpointofintersectionarcs.h"
#include "../../../../visualization/line/vistoolpointofintersectionarcs.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/varc.h"
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

const QString VToolPointOfIntersectionArcs::ToolType = QStringLiteral("pointOfIntersectionArcs");

//---------------------------------------------------------------------------------------------------------------------
VToolPointOfIntersectionArcs::VToolPointOfIntersectionArcs(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                                           const quint32 &firstArcId, const quint32 &secondArcId,
                                                           CrossCirclesPoint pType, const Source &typeCreation,
                                                           QGraphicsItem *parent)
    : VToolSinglePoint(doc, data, id, QColor(qApp->Settings()->getPointNameColor()), parent)
    , firstArcId(firstArcId)
    , secondArcId(secondArcId)
    , crossPoint(pType)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogPointOfIntersectionArcs> dialogTool = m_dialog.objectCast<DialogPointOfIntersectionArcs>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetFirstArcId(firstArcId);
    dialogTool->SetSecondArcId(secondArcId);
    dialogTool->SetCrossArcPoint(crossPoint);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
VToolPointOfIntersectionArcs *VToolPointOfIntersectionArcs::Create(QSharedPointer<DialogTool> dialog,
                                                                   VMainGraphicsScene *scene, VAbstractPattern *doc,
                                                                   VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogPointOfIntersectionArcs> dialogTool = dialog.objectCast<DialogPointOfIntersectionArcs>();
    SCASSERT(not dialogTool.isNull())
    const quint32 firstArcId = dialogTool->GetFirstArcId();
    const quint32 secondArcId = dialogTool->GetSecondArcId();
    const CrossCirclesPoint pType = dialogTool->GetCrossArcPoint();
    const QString pointName = dialogTool->getPointName();
    VToolPointOfIntersectionArcs *point = Create(0, pointName, firstArcId, secondArcId, pType, 5, 10, scene, doc,
                                                 data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
VToolPointOfIntersectionArcs *VToolPointOfIntersectionArcs::Create(const quint32 _id, const QString &pointName,
                                                                   const quint32 &firstArcId,
                                                                   const quint32 &secondArcId, CrossCirclesPoint pType,
                                                                   qreal mx, qreal my,
                                                                   VMainGraphicsScene *scene, VAbstractPattern *doc,
                                                                   VContainer *data, const Document &parse,
                                                                   const Source &typeCreation)
{
    const QSharedPointer<VArc> firstArc = data->GeometricObject<VArc>(firstArcId);
    const QSharedPointer<VArc> secondArc = data->GeometricObject<VArc>(secondArcId);

    const QPointF point = FindPoint(firstArc.data(), secondArc.data(), pType);
    quint32 id = _id;
    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(new VPointF(point, pointName, mx, my));
    }
    else
    {
        data->UpdateGObject(id, new VPointF(point, pointName, mx, my));
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::PointOfIntersectionArcs, doc);
        VToolPointOfIntersectionArcs *point = new VToolPointOfIntersectionArcs(doc, data, id, firstArcId,
                                                                               secondArcId, pType, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(firstArc->getIdTool());
        doc->IncrementReferens(secondArc->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VToolPointOfIntersectionArcs::FindPoint(const VArc *arc1, const VArc *arc2, const CrossCirclesPoint pType)
{
    QPointF p1, p2;
    const QPointF centerArc1 = static_cast<QPointF>(arc1->GetCenter());
    const QPointF centerArc2 = static_cast<QPointF>(arc2->GetCenter());
    const int res = VGObject::IntersectionCircles(centerArc1, arc1->GetRadius(), centerArc2, arc2->GetRadius(), p1, p2);

    QLineF r1Arc1(centerArc1, p1);
    r1Arc1.setLength(r1Arc1.length()+10);

    QLineF r1Arc2(centerArc2, p1);
    r1Arc2.setLength(r1Arc2.length()+10);

    QLineF r2Arc1(centerArc1, p2);
    r2Arc1.setLength(r2Arc1.length()+10);

    QLineF r2Arc2(centerArc2, p2);
    r2Arc2.setLength(r2Arc2.length()+10);

    switch(res)
    {
        case 2:
        {
            int localRes = 0;
            bool flagP1 = false;

            if (arc1->IsIntersectLine(r1Arc1)  && arc2->IsIntersectLine(r1Arc2))
            {
                ++localRes;
                flagP1 = true;
            }

            if (arc1->IsIntersectLine(r2Arc1)  && arc2->IsIntersectLine(r2Arc2))
            {
                ++localRes;
            }

            switch(localRes)
            {
                case 2:
                    if (pType == CrossCirclesPoint::FirstPoint)
                    {
                        return p1;
                    }
                    else
                    {
                        return p2;
                    }
                case 1:
                    if (flagP1)
                    {
                        return p1;
                    }
                    else
                    {
                        return p2;
                    }
                case 0:
                default:
                    return QPointF();
            }

            break;
        }
        case 1:
            if (arc1->IsIntersectLine(r1Arc1) && arc2->IsIntersectLine(r1Arc2))
            {
                return p1;
            }
            else
            {
                return QPointF();
            }
        case 3:
        case 0:
        default:
            break;
    }
    return QPointF();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolPointOfIntersectionArcs::FirstArcName() const
{
    return VAbstractTool::data.GetGObject(firstArcId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolPointOfIntersectionArcs::SecondArcName() const
{
    return VAbstractTool::data.GetGObject(secondArcId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolPointOfIntersectionArcs::GetFirstArcId() const
{
    return firstArcId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetFirstArcId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        firstArcId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolPointOfIntersectionArcs::GetSecondArcId() const
{
    return secondArcId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetSecondArcId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        secondArcId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
CrossCirclesPoint VToolPointOfIntersectionArcs::GetCrossCirclesPoint() const
{
    return crossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetCrossCirclesPoint(const CrossCirclesPoint &value)
{
    crossPoint = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolPointOfIntersectionArcs>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::RemoveReferens()
{
    const auto firstArc = VAbstractTool::data.GetGObject(firstArcId);
    const auto secondArc = VAbstractTool::data.GetGObject(secondArcId);

    doc->DecrementReferens(firstArc->getIdTool());
    doc->DecrementReferens(secondArc->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogPointOfIntersectionArcs>(event, id);
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogPointOfIntersectionArcs> dialogTool = m_dialog.objectCast<DialogPointOfIntersectionArcs>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName, dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrFirstArc, QString().setNum(dialogTool->GetFirstArcId()));
    doc->SetAttribute(domElement, AttrSecondArc, QString().setNum(dialogTool->GetSecondArcId()));
    doc->SetAttribute(domElement, AttrCrossPoint, QString().setNum(static_cast<int>(dialogTool->GetCrossArcPoint())));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrFirstArc, firstArcId);
    doc->SetAttribute(tag, AttrSecondArc, secondArcId);
    doc->SetAttribute(tag, AttrCrossPoint, static_cast<int>(crossPoint));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::ReadToolAttributes(const QDomElement &domElement)
{
    firstArcId = doc->GetParametrUInt(domElement, AttrFirstArc, NULL_ID_STR);
    secondArcId = doc->GetParametrUInt(domElement, AttrSecondArc, NULL_ID_STR);
    crossPoint = static_cast<CrossCirclesPoint>(doc->GetParametrUInt(domElement, AttrCrossPoint, "1"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolPointOfIntersectionArcs *visual = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
        SCASSERT(visual != nullptr)

        visual->setArc1Id(firstArcId);
        visual->setArc2Id(secondArcId);
        visual->setCrossPoint(crossPoint);
        visual->RefreshGeometry();
    }
}
