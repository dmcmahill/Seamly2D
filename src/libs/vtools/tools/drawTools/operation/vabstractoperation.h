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
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#ifndef VABSTRACTOPERATION_H
#define VABSTRACTOPERATION_H

#include <QtGlobal>
#include <qcompilerdetection.h>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QGraphicsLineItem>

#include "../vdrawtool.h"
#include "../vwidgets/vsimplecurve.h"
#include "../vwidgets/vsimplepoint.h"

struct SourceItem
{
    quint32 id{NULL_ID};
    QString alias{};
    QString lineType{"solidLine"};
    QString lineWidth{"1.00"};
    QString color{"black"};
};

Q_DECLARE_METATYPE(SourceItem)
Q_DECLARE_TYPEINFO(SourceItem, Q_MOVABLE_TYPE);

QVector<quint32>     sourceToObjects(const QVector<SourceItem> &source);

struct DestinationItem
{
    quint32 id;
    qreal mx{1};
    qreal my{1};
    bool showPointName{true};
};

// FIXME. I don't know how to use QGraphicsItem properly, so just took first available finished class.
// QGraphicsItem itself produce case where clicking on empty space produce call to QGraphicsItem.
// And i don't know how to fix it.
class VAbstractOperation : public VDrawTool, public QGraphicsLineItem
{
    Q_OBJECT
    // Fix warning "Class implements the interface QGraphicsItem but does not list it
    // in Q_INTERFACES. qobject_cast to QGraphicsItem will not work!"
    Q_INTERFACES(QGraphicsItem)

public:
    virtual             ~VAbstractOperation() Q_DECL_EQ_DEFAULT;

    static const QString TagItem;
    static const QString TagSource;
    static const QString TagDestination;

    virtual QString      getTagName() const Q_DECL_OVERRIDE;

    QString              Suffix() const;
    void                 setSuffix(const QString &suffix);

    virtual void         GroupVisibility(quint32 object, bool visible) Q_DECL_OVERRIDE;
    virtual void         paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

    virtual bool         isPointNameVisible(quint32 id) const Q_DECL_OVERRIDE;
    virtual void         setPointNameVisiblity(quint32 id, bool visible) Q_DECL_OVERRIDE;

    virtual void         setPointNamePosition(quint32 id, const QPointF &pos) Q_DECL_OVERRIDE;

    static void          ExtractData(const QDomElement &domElement, QVector<SourceItem> &source,
                                     QVector<DestinationItem> &destination);

public slots:
    virtual void         FullUpdateFromFile() Q_DECL_OVERRIDE;

    virtual void         AllowHover(bool enabled) Q_DECL_OVERRIDE;
    virtual void         AllowSelecting(bool enabled) Q_DECL_OVERRIDE;
    virtual void         EnableToolMove(bool move) Q_DECL_OVERRIDE;

    void                 AllowPointHover(bool enabled);
    void                 AllowPointSelecting(bool enabled);

    void                 AllowPointLabelHover(bool enabled);
    void                 AllowPointLabelSelecting(bool enabled);

    void                 AllowSplineHover(bool enabled);
    void                 AllowSplineSelecting(bool enabled);

    void                 AllowSplinePathHover(bool enabled);
    void                 AllowSplinePathSelecting(bool enabled);

    void                 AllowArcHover(bool enabled);
    void                 AllowArcSelecting(bool enabled);

    void                 AllowElArcHover(bool enabled);
    void                 AllowElArcSelecting(bool enabled);

    virtual void         ToolSelectionType(const SelectionType &type) Q_DECL_OVERRIDE;
    virtual void         Disable(bool disable, const QString &draftBlockName) Q_DECL_OVERRIDE;
    void                 ObjectSelected(bool selected, quint32 objId);
    void                 deletePoint();
    void                 pointNamePositionChanged(const QPointF &pos, quint32 labelId);

protected:
    QString                          suffix;
    QVector<SourceItem>              source;
    QVector<DestinationItem>         destination;
    QMap<quint32, VAbstractSimple *> operatedObjects;

                         VAbstractOperation(VAbstractPattern *doc, VContainer *data, quint32 id,
                                            const QString &suffix, const QVector<SourceItem> &source,
                                            const QVector<DestinationItem> &destination,
                                            QGraphicsItem *parent = nullptr);

    virtual void         AddToFile() Q_DECL_OVERRIDE;
    virtual void         ReadToolAttributes(const QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void         SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;


    virtual void         updatePointNameVisibility(quint32 id, bool visible) Q_DECL_OVERRIDE;
    void                 updatePointNamePosition(quint32 id, const QPointF &pos);

    void                 SaveSourceDestination(QDomElement &tag);

    template <typename T>
    void                 ShowToolVisualization(bool show);

    void                 InitCurve(quint32 id, VContainer *data, GOType curveType, SceneObject sceneType);

    template <typename T>
    static void          initOperationToolConnections(VMainGraphicsScene *scene, T *tool);

    void                 InitOperatedObjects();
    QString              complexPointToolTip(quint32 itemId) const;
    QString              complexCurveToolTip(quint32 itemId) const;

private:
    Q_DISABLE_COPY(VAbstractOperation)

    void                 AllowCurveHover(bool enabled, GOType type);
    void                 AllowCurveSelecting(bool enabled, GOType type);
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void VAbstractOperation::ShowToolVisualization(bool show)
{
    if (show)
    {
        if (vis.isNull())
        {
            AddVisualization<T>();
            SetVisualization();
        }
        else
        {
            if (T *visual = qobject_cast<T *>(vis))
            {
                visual->show();
            }
        }
    }
    else
    {
        delete vis;
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void VAbstractOperation::initOperationToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);

    QObject::connect(scene, &VMainGraphicsScene::EnablePointItemHover,          tool, &T::AllowPointHover);
    QObject::connect(scene, &VMainGraphicsScene::EnablePointItemSelection,      tool, &T::AllowPointSelecting);
    QObject::connect(scene, &VMainGraphicsScene::enableTextItemHover,           tool, &T::AllowPointLabelHover);
    QObject::connect(scene, &VMainGraphicsScene::enableTextItemSelection,       tool, &T::AllowPointLabelSelecting);

    QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemHover,         tool, &T::AllowSplineHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemSelection,     tool, &T::AllowSplineSelecting);

    QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemHover,     tool, &T::AllowSplinePathHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemSelection, tool, &T::AllowSplinePathSelecting);

    QObject::connect(scene, &VMainGraphicsScene::EnableArcItemHover,            tool, &T::AllowArcHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableArcItemSelection,        tool, &T::AllowArcSelecting);

    QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemHover,          tool, &T::AllowElArcHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemSelection,      tool, &T::AllowElArcSelecting);
}

#endif // VABSTRACTOPERATION_H
