/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
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
 **  @file   dialoglayoutsettings.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 1, 2015
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

#include "dialoglayoutsettings.h"
#include "ui_dialoglayoutsettings.h"
#include "../core/vapplication.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vmisc/vsettings.h"
#include "../vmisc/vmath.h"
#include "../vlayout/vlayoutgenerator.h"

#include <QMessageBox>
#include <QPushButton>
#include <QPrinterInfo>

//---------------------------------------------------------------------------------------------------------------------
DialogLayoutSettings::DialogLayoutSettings(VLayoutGenerator *generator, QWidget *parent, bool disableSettings)
    : VAbstractLayoutDialog(parent), disableSettings(disableSettings), ui(new Ui::DialogLayoutSettings), oldPaperUnit(Unit::Mm),
      oldLayoutUnit(Unit::Mm), generator(generator), isInitialized(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    qApp->Seamly2DSettings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    //moved from ReadSettings - well...it seems it can be done once only (i.e. constructor) because Init funcs dont
    //even cleanse lists before adding
    InitPaperUnits();
    InitLayoutUnits();
    InitTemplates(ui->comboBoxTemplates);
    MinimumPaperSize();
    MinimumLayoutSize();
    InitPrinter();

    //in export console mode going to use defaults
    if (disableSettings == false)
    {
        ReadSettings();
    }
    else
    {
        RestoreDefaults();
    }

    connect(ui->comboBoxPrinter, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogLayoutSettings::PrinterMargins);

    connect(ui->comboBoxTemplates, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogLayoutSettings::TemplateSelected);
    connect(ui->comboBoxPaperSizeUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogLayoutSettings::ConvertPaperSize);

    connect(ui->doubleSpinBoxPaperWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &DialogLayoutSettings::PaperSizeChanged);
    connect(ui->doubleSpinBoxPaperHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &DialogLayoutSettings::PaperSizeChanged);

    connect(ui->doubleSpinBoxPaperWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &DialogLayoutSettings::FindTemplate);
    connect(ui->doubleSpinBoxPaperHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &DialogLayoutSettings::FindTemplate);

    connect(ui->doubleSpinBoxPaperWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &DialogLayoutSettings::CorrectMaxFileds);
    connect(ui->doubleSpinBoxPaperHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &DialogLayoutSettings::CorrectMaxFileds);

    connect(ui->checkBoxIgnoreFileds, &QCheckBox::stateChanged, this, &DialogLayoutSettings::IgnoreAllFields);

    connect(ui->toolButtonPortrait, &QToolButton::toggled, this, &DialogLayoutSettings::Swap);
    connect(ui->toolButtonLandscape, &QToolButton::toggled, this, &DialogLayoutSettings::Swap);
    connect(ui->comboBoxLayoutUnit,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogLayoutSettings::ConvertLayoutSize);

    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    connect(bOk, &QPushButton::clicked, this, &DialogLayoutSettings::DialogAccepted);

    QPushButton *bRestoreDefaults = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(bRestoreDefaults, &QPushButton::clicked, this, &DialogLayoutSettings::RestoreDefaults);
}

//---------------------------------------------------------------------------------------------------------------------
DialogLayoutSettings::~DialogLayoutSettings()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
qreal DialogLayoutSettings::GetPaperHeight() const
{
    return UnitConvertor(ui->doubleSpinBoxPaperHeight->value(), oldPaperUnit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetPaperHeight(qreal value)
{
    ui->doubleSpinBoxPaperHeight->setMaximum(FromPixel(QIMAGE_MAX, PaperUnit()));
    ui->doubleSpinBoxPaperHeight->setValue(UnitConvertor(value, Unit::Px, PaperUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
qreal DialogLayoutSettings::GetPaperWidth() const
{
    return UnitConvertor(ui->doubleSpinBoxPaperWidth->value(), oldPaperUnit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetPaperWidth(qreal value)
{
    ui->doubleSpinBoxPaperWidth->setMaximum(FromPixel(QIMAGE_MAX, PaperUnit()));
    ui->doubleSpinBoxPaperWidth->setValue(UnitConvertor(value, Unit::Px, PaperUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
qreal DialogLayoutSettings::GetShift() const
{
    return UnitConvertor(ui->doubleSpinBoxShift->value(), oldLayoutUnit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetShift(qreal value)
{
    ui->doubleSpinBoxShift->setValue(UnitConvertor(value, Unit::Px, LayoutUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
qreal DialogLayoutSettings::GetLayoutWidth() const
{
    return UnitConvertor(ui->doubleSpinBoxLayoutWidth->value(), oldLayoutUnit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetLayoutWidth(qreal value)
{
    ui->doubleSpinBoxLayoutWidth->setValue(UnitConvertor(value, Unit::Px, LayoutUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF DialogLayoutSettings::GetFields() const
{
    QMarginsF fields;
    fields.setLeft(UnitConvertor(ui->doubleSpinBoxLeftField->value(), oldLayoutUnit, Unit::Px));
    fields.setRight(UnitConvertor(ui->doubleSpinBoxRightField->value(), oldLayoutUnit, Unit::Px));
    fields.setTop(UnitConvertor(ui->doubleSpinBoxTopField->value(), oldLayoutUnit, Unit::Px));
    fields.setBottom(UnitConvertor(ui->doubleSpinBoxBottomField->value(), oldLayoutUnit, Unit::Px));
    return fields;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetFields(const QMarginsF &value)
{
    ui->doubleSpinBoxLeftField->setValue(UnitConvertor(value.left(), Unit::Px, LayoutUnit()));
    ui->doubleSpinBoxRightField->setValue(UnitConvertor(value.right(), Unit::Px, LayoutUnit()));
    ui->doubleSpinBoxTopField->setValue(UnitConvertor(value.top(), Unit::Px, LayoutUnit()));
    ui->doubleSpinBoxBottomField->setValue(UnitConvertor(value.bottom(), Unit::Px, LayoutUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
Cases DialogLayoutSettings::GetGroup() const
{
    if (ui->radioButtonThreeGroups->isChecked())
    {
        return Cases::CaseThreeGroup;
    }
    else if (ui->radioButtonTwoGroups->isChecked())
    {
        return Cases::CaseTwoGroup;
    }
    else
    {
        return Cases::CaseDesc;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void DialogLayoutSettings::SetGroup(const Cases &value)
{
    switch (value)
    {
        case Cases::CaseThreeGroup:
            ui->radioButtonThreeGroups->setChecked(true);
            break;
        case Cases::CaseTwoGroup:
            ui->radioButtonTwoGroups->setChecked(true);
            break;
        case Cases::CaseDesc:
        default:
            ui->radioButtonDescendingArea->setChecked(true);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogLayoutSettings::GetRotate() const
{
    return ui->groupBoxRotate->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetRotate(bool state)
{
    ui->groupBoxRotate->setChecked(state);
}

//---------------------------------------------------------------------------------------------------------------------
int DialogLayoutSettings::GetIncrease() const
{
    return ui->comboBoxIncrease->currentText().toInt();
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
bool DialogLayoutSettings::SetIncrease(int increase)
{
    int index = ui->comboBoxIncrease->findText(QString::number(increase));
    bool failed = (index == -1);
    if (failed)
    {
        const QString def = QString::number(VSettings::GetDefLayoutRotationIncrease());// Value by default
        index = ui->comboBoxIncrease->findText(def);
    }

    ui->comboBoxIncrease->setCurrentIndex(index);
    return failed;
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogLayoutSettings::GetAutoCrop() const
{
    return ui->checkBoxAutoCrop->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetAutoCrop(bool autoCrop)
{
    ui->checkBoxAutoCrop->setChecked(autoCrop);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogLayoutSettings::IsSaveLength() const
{
    return ui->checkBoxSaveLength->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetSaveLength(bool save)
{
    ui->checkBoxSaveLength->setChecked(save);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogLayoutSettings::IsUnitePages() const
{
    return ui->checkBoxUnitePages->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetUnitePages(bool save)
{
    ui->checkBoxUnitePages->setChecked(save);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogLayoutSettings::IsStripOptimization() const
{
    return ui->groupBoxStrips->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetStripOptimization(bool save)
{
    ui->groupBoxStrips->setChecked(save);
}

//---------------------------------------------------------------------------------------------------------------------
quint8 DialogLayoutSettings::GetMultiplier() const
{
    return static_cast<quint8>(ui->spinBoxMultiplier->value());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetMultiplier(const quint8 &value)
{
    ui->spinBoxMultiplier->setValue(static_cast<int>(value));
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogLayoutSettings::IsIgnoreAllFields() const
{
    return ui->checkBoxIgnoreFileds->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetIgnoreAllFields(bool value)
{
    ui->checkBoxIgnoreFileds->setChecked(value);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogLayoutSettings::IsTextAsPaths() const
{
    return ui->checkBoxTextAsPaths->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetTextAsPaths(bool value)
{
    ui->checkBoxTextAsPaths->setChecked(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogLayoutSettings::SelectedPrinter() const
{
    return ui->comboBoxPrinter->currentText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::TemplateSelected()
{
    SheetSize(Template());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::FindTemplate()
{
    const qreal width = ui->doubleSpinBoxPaperWidth->value();
    const qreal height = ui->doubleSpinBoxPaperHeight->value();
    QSizeF size(width, height);

    const Unit paperUnit = PaperUnit();

    const int max = static_cast<int>(PaperSizeTemplate::Custom);
    for (int i=0; i < max; ++i)
    {
        const QSizeF tmplSize = GetTemplateSize(static_cast<PaperSizeTemplate>(i), paperUnit);
        if (size == tmplSize)
        {
            ui->comboBoxTemplates->blockSignals(true);
            const int index = ui->comboBoxTemplates->findData(i);
            if (index != -1)
            {
                ui->comboBoxTemplates->setCurrentIndex(index);
            }
            ui->comboBoxTemplates->blockSignals(false);
            return;
        }
    }

    ui->comboBoxTemplates->blockSignals(true);
    const int index = ui->comboBoxTemplates->findData(max);
    if (index != -1)
    {
        ui->comboBoxTemplates->setCurrentIndex(index);
    }
    ui->comboBoxTemplates->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::ConvertPaperSize()
{
    const Unit paperUnit = PaperUnit();
    const qreal width = ui->doubleSpinBoxPaperWidth->value();
    const qreal height = ui->doubleSpinBoxPaperHeight->value();

    const qreal left = ui->doubleSpinBoxLeftField->value();
    const qreal right = ui->doubleSpinBoxRightField->value();
    const qreal top = ui->doubleSpinBoxTopField->value();
    const qreal bottom = ui->doubleSpinBoxBottomField->value();

    ui->doubleSpinBoxPaperWidth->blockSignals(true);
    ui->doubleSpinBoxPaperHeight->blockSignals(true);
    ui->doubleSpinBoxPaperWidth->setMaximum(FromPixel(QIMAGE_MAX, paperUnit));
    ui->doubleSpinBoxPaperHeight->setMaximum(FromPixel(QIMAGE_MAX, paperUnit));
    ui->doubleSpinBoxPaperWidth->blockSignals(false);
    ui->doubleSpinBoxPaperHeight->blockSignals(false);

    const qreal newWidth = UnitConvertor(width, oldPaperUnit, paperUnit);
    const qreal newHeight = UnitConvertor(height, oldPaperUnit, paperUnit);

    const qreal newLeft = UnitConvertor(left, oldPaperUnit, paperUnit);
    const qreal newRight = UnitConvertor(right, oldPaperUnit, paperUnit);
    const qreal newTop = UnitConvertor(top, oldPaperUnit, paperUnit);
    const qreal newBottom = UnitConvertor(bottom, oldPaperUnit, paperUnit);

    oldPaperUnit = paperUnit;
    CorrectPaperDecimals();
    MinimumPaperSize();

    ui->doubleSpinBoxPaperWidth->setValue(newWidth);
    ui->doubleSpinBoxPaperHeight->setValue(newHeight);

    ui->doubleSpinBoxLeftField->setValue(newLeft);
    ui->doubleSpinBoxRightField->setValue(newRight);
    ui->doubleSpinBoxTopField->setValue(newTop);
    ui->doubleSpinBoxBottomField->setValue(newBottom);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogLayoutSettings::SelectPaperUnit(const QString& units)
{
    qint32 indexUnit = ui->comboBoxPaperSizeUnit->findData(units);
    if (indexUnit != -1)
    {
        ui->comboBoxPaperSizeUnit->setCurrentIndex(indexUnit);
    }
    return indexUnit != -1;
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogLayoutSettings::SelectLayoutUnit(const QString &units)
{
    qint32 indexUnit = ui->comboBoxLayoutUnit->findData(units);
    if (indexUnit != -1)
    {
        ui->comboBoxLayoutUnit->setCurrentIndex(indexUnit);
    }
    return indexUnit != -1;
}

//---------------------------------------------------------------------------------------------------------------------
qreal DialogLayoutSettings::LayoutToPixels(qreal value) const
{
    return UnitConvertor(value, LayoutUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
qreal DialogLayoutSettings::PageToPixels(qreal value) const
{
    return UnitConvertor(value, PaperUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogLayoutSettings::MakeGroupsHelp()
{
    //that is REALLY dummy ... can't figure fast how to automate generation... :/
    return tr("\n\tThree groups: big, middle, small = 0;\n\tTwo groups: big, small = 1;\n\tDescending area = 2");
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::showEvent(QShowEvent *event)
{
    QDialog::showEvent( event );
    if ( event->spontaneous() )
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    setMaximumSize(size());
    setMinimumSize(size());

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::ConvertLayoutSize()
{
    const Unit unit = LayoutUnit();
    const qreal layoutWidth = ui->doubleSpinBoxLayoutWidth->value();
    const qreal shift = ui->doubleSpinBoxShift->value();

    ui->doubleSpinBoxLayoutWidth->setMaximum(FromPixel(QIMAGE_MAX, unit));
    ui->doubleSpinBoxShift->setMaximum(FromPixel(QIMAGE_MAX, unit));

    const qreal newLayoutWidth = UnitConvertor(layoutWidth, oldLayoutUnit, unit);
    const qreal newShift = UnitConvertor(shift, oldLayoutUnit, unit);

    oldLayoutUnit = unit;
    CorrectLayoutDecimals();
    MinimumLayoutSize();

    ui->doubleSpinBoxLayoutWidth->setValue(newLayoutWidth);
    ui->doubleSpinBoxShift->setValue(newShift);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::PaperSizeChanged()
{
    if (ui->doubleSpinBoxPaperHeight->value() > ui->doubleSpinBoxPaperWidth->value())
    {
        ui->toolButtonPortrait->blockSignals(true);
        ui->toolButtonPortrait->setChecked(true);
        ui->toolButtonPortrait->blockSignals(false);
    }
    else
    {
        ui->toolButtonLandscape->blockSignals(true);
        ui->toolButtonLandscape->setChecked(true);
        ui->toolButtonLandscape->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogLayoutSettings::SelectTemplate(const PaperSizeTemplate& id)
{
    int index = ui->comboBoxTemplates->findData(static_cast<VIndexType>(id));
    if (index > -1)
    {
        ui->comboBoxTemplates->setCurrentIndex(index);
    }

    return (index > -1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::Swap(bool checked)
{
    if (checked)
    {
        const qreal width = ui->doubleSpinBoxPaperWidth->value();
        const qreal height = ui->doubleSpinBoxPaperHeight->value();

        ui->doubleSpinBoxPaperWidth->blockSignals(true);
        ui->doubleSpinBoxPaperWidth->setValue(height);
        ui->doubleSpinBoxPaperWidth->blockSignals(false);

        ui->doubleSpinBoxPaperHeight->blockSignals(true);
        ui->doubleSpinBoxPaperHeight->setValue(width);
        ui->doubleSpinBoxPaperHeight->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::DialogAccepted()
{
    SCASSERT(generator != nullptr)
    generator->SetLayoutWidth(GetLayoutWidth());
    generator->SetCaseType(GetGroup());
    generator->SetPaperHeight(GetPaperHeight());
    generator->SetPaperWidth(GetPaperWidth());
    generator->SetShift(static_cast<quint32>(qFloor(GetShift())));
    generator->SetRotate(GetRotate());
    generator->SetRotationIncrease(GetIncrease());
    generator->SetAutoCrop(GetAutoCrop());
    generator->SetSaveLength(IsSaveLength());
    generator->SetUnitePages(IsUnitePages());
    generator->SetStripOptimization(IsStripOptimization());
    generator->SetMultiplier(GetMultiplier());
    generator->SetTestAsPaths(IsTextAsPaths());

    if (IsIgnoreAllFields())
    {
        generator->SetPrinterFields(false, QMarginsF());
    }
    else
    {
        QPrinterInfo printer = QPrinterInfo::printerInfo(ui->comboBoxPrinter->currentText());
        if (printer.isNull())
        {
            generator->SetPrinterFields(true, GetFields());
        }
        else
        {
            const QMarginsF minFields = MinPrinterFields();
            const QMarginsF fields = GetFields();
            if (fields.left() < minFields.left() || fields.right() < minFields.right() ||
                fields.top() < minFields.top() || fields.bottom() < minFields.bottom())
            {
                QMessageBox::StandardButton answer;
                answer = QMessageBox::question(this, tr("Wrong fields."),
                                               tr("Margins go beyond printing. \n\nApply settings anyway?"),
                                               QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
                if (answer == QMessageBox::No)
                {
                    if (fields.left() < minFields.left())
                    {
                        ui->doubleSpinBoxLeftField->setValue(UnitConvertor(minFields.left(), Unit::Px, LayoutUnit()));
                    }

                    if (fields.right() < minFields.right())
                    {
                        ui->doubleSpinBoxRightField->setValue(UnitConvertor(minFields.right(), Unit::Px, LayoutUnit()));
                    }

                    if (fields.top() < minFields.top())
                    {
                        ui->doubleSpinBoxTopField->setValue(UnitConvertor(minFields.top(), Unit::Px, LayoutUnit()));
                    }

                    if (fields.bottom() < minFields.bottom())
                    {
                        ui->doubleSpinBoxBottomField->setValue(UnitConvertor(minFields.bottom(), Unit::Px,
                                                                             LayoutUnit()));
                    }

                    generator->SetPrinterFields(true, GetFields());
                }
                else
                {
                    generator->SetPrinterFields(false, GetFields());
                }
            }
            else
            {
                generator->SetPrinterFields(true, GetFields());
            }
        }
    }

    //don't want to break visual settings when cmd used
    if (disableSettings == false)
    {
        WriteSettings();
    }
    accepted();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::RestoreDefaults()
{
    ui->comboBoxTemplates->blockSignals(true);
    ui->comboBoxTemplates->setCurrentIndex(0);//A0
    TemplateSelected();
    ui->comboBoxTemplates->blockSignals(false);

    ui->comboBoxPrinter->blockSignals(true);
    InitPrinter();
    ui->comboBoxPrinter->blockSignals(false);

    SetLayoutWidth(VSettings::GetDefLayoutWidth());
    SetShift(VSettings::GetDefLayoutShift());
    SetGroup(VSettings::GetDefLayoutGroup());
    SetRotate(VSettings::GetDefLayoutRotate());
    SetIncrease(VSettings::GetDefLayoutRotationIncrease());
    SetFields(GetDefPrinterFields());
    SetIgnoreAllFields(VSettings::GetDefIgnoreAllFields());
    SetMultiplier(VSettings::GetDefMultiplier());

    CorrectMaxFileds();
    IgnoreAllFields(ui->checkBoxIgnoreFileds->isChecked());

    ui->checkBoxTextAsPaths->setChecked(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::PrinterMargins()
{
    QPrinterInfo printer = QPrinterInfo::printerInfo(ui->comboBoxPrinter->currentText());
    if (not printer.isNull())
    {
        SetFields(GetPrinterFields(QSharedPointer<QPrinter>(new QPrinter(printer))));
    }
    else
    {
        SetFields(QMarginsF());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::CorrectMaxFileds()
{
    const qreal width = ui->doubleSpinBoxPaperWidth->value();
    const qreal height = ui->doubleSpinBoxPaperHeight->value();

    // 80%/2 of paper size for each field
    const qreal widthField = (width*80.0/100.0)/2.0;
    const qreal heightField = (height*80.0/100.0)/2.0;

    ui->doubleSpinBoxLeftField->setMaximum(widthField);
    ui->doubleSpinBoxRightField->setMaximum(widthField);
    ui->doubleSpinBoxTopField->setMaximum(heightField);
    ui->doubleSpinBoxBottomField->setMaximum(heightField);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::IgnoreAllFields(int state)
{
    ui->doubleSpinBoxLeftField->setDisabled(state);
    ui->doubleSpinBoxRightField->setDisabled(state);
    ui->doubleSpinBoxTopField->setDisabled(state);
    ui->doubleSpinBoxBottomField->setDisabled(state);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::InitPaperUnits()
{
    ui->comboBoxPaperSizeUnit->addItem(tr("Millimeters"), QVariant(UnitsToStr(Unit::Mm)));
    ui->comboBoxPaperSizeUnit->addItem(tr("Centimeters"), QVariant(UnitsToStr(Unit::Cm)));
    ui->comboBoxPaperSizeUnit->addItem(tr("Inches"), QVariant(UnitsToStr(Unit::Inch)));
    ui->comboBoxPaperSizeUnit->addItem(tr("Pixels"), QVariant(UnitsToStr(Unit::Px)));

    // set default unit
    oldPaperUnit = StrToUnits(qApp->Seamly2DSettings()->GetUnit());
    const qint32 indexUnit = ui->comboBoxPaperSizeUnit->findData(qApp->Seamly2DSettings()->GetUnit());
    if (indexUnit != -1)
    {
        ui->comboBoxPaperSizeUnit->setCurrentIndex(indexUnit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::InitLayoutUnits()
{
    ui->comboBoxLayoutUnit->addItem(tr("Centimeters"), QVariant(UnitsToStr(Unit::Cm)));
    ui->comboBoxLayoutUnit->addItem(tr("Millimeters"), QVariant(UnitsToStr(Unit::Mm)));
    ui->comboBoxLayoutUnit->addItem(tr("Inches"), QVariant(UnitsToStr(Unit::Inch)));

    // set default unit
    oldLayoutUnit = StrToUnits(qApp->Seamly2DSettings()->GetUnit());
    const qint32 indexUnit = ui->comboBoxLayoutUnit->findData(qApp->Seamly2DSettings()->GetUnit());
    if (indexUnit != -1)
    {
        ui->comboBoxLayoutUnit->setCurrentIndex(indexUnit);
    }
}


//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::InitPrinter()
{
    ui->comboBoxPrinter->clear();
    QStringList printerNames;
    printerNames = QPrinterInfo::availablePrinterNames();

    ui->comboBoxPrinter->addItems(printerNames);

    if (ui->comboBoxPrinter->count() == 0)
    {
        ui->comboBoxPrinter->addItem(tr("None", "Printer"));
    }
    else
    {
        QString defPrinterName;
        defPrinterName = QPrinterInfo::defaultPrinterName();

        const int index = ui->comboBoxPrinter->findText(defPrinterName);
        if(index != -1)
        {
            ui->comboBoxPrinter->setCurrentIndex(index);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogLayoutSettings::MakeHelpTemplateList()
{
    QString out = "\n";

    auto cntr = static_cast<VIndexType>(PaperSizeTemplate::A0);
    foreach(const auto& v,  VAbstractLayoutDialog::pageFormatNames)
    {
        if (cntr <= static_cast<int>(PaperSizeTemplate::Roll44in))// Don't include custom template
        {
            out += "\t"+v+" = "+ QString::number(cntr++)+"\n";
        }
    }
    return out;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF DialogLayoutSettings::Template()
{
    PaperSizeTemplate temp;
    temp = static_cast<PaperSizeTemplate>(ui->comboBoxTemplates->currentData().toInt());

    const Unit paperUnit = PaperUnit();

    switch (temp)
    {
        case PaperSizeTemplate::A0:
        case PaperSizeTemplate::A1:
        case PaperSizeTemplate::A2:
        case PaperSizeTemplate::A3:
        case PaperSizeTemplate::A4:
        case PaperSizeTemplate::Letter:
            SetAdditionalOptions(false);
            return GetTemplateSize(temp, paperUnit);
        case PaperSizeTemplate::Legal:
        case PaperSizeTemplate::Roll24in:
        case PaperSizeTemplate::Roll30in:
        case PaperSizeTemplate::Roll36in:
        case PaperSizeTemplate::Roll42in:
        case PaperSizeTemplate::Roll44in:
            SetAdditionalOptions(true);
            return GetTemplateSize(temp, paperUnit);
        case PaperSizeTemplate::Custom:
            return GetTemplateSize(temp, paperUnit);
        default:
            break;
    }
    return QSizeF();
}

/**
 * @brief DialogLayoutSettings::TemplateSize
 * @param tmpl
 * @param unit
 * @return
 */
QSizeF DialogLayoutSettings::GetTemplateSize(const PaperSizeTemplate &tmpl, const Unit &unit) const
{
    qreal width = 0;
    qreal height = 0;

    switch (tmpl)
    {
        case PaperSizeTemplate::Custom:
            width = ui->doubleSpinBoxPaperWidth->value();
            height = ui->doubleSpinBoxPaperHeight->value();
            return RoundTemplateSize(width, height, unit);
        default:
            return VAbstractLayoutDialog::GetTemplateSize(tmpl, unit);
    }
}




//---------------------------------------------------------------------------------------------------------------------
QMarginsF DialogLayoutSettings::MinPrinterFields() const
{
    QPrinterInfo printer = QPrinterInfo::printerInfo(ui->comboBoxPrinter->currentText());
    if (not printer.isNull())
    {
        QSharedPointer<QPrinter> pr = QSharedPointer<QPrinter>(new QPrinter(printer));
        return GetMinPrinterFields(pr);
    }
    else
    {
        return QMarginsF();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF DialogLayoutSettings::GetDefPrinterFields() const
{
    QPrinterInfo printer = QPrinterInfo::printerInfo(ui->comboBoxPrinter->currentText());
    if (not printer.isNull())
    {
        return GetPrinterFields(QSharedPointer<QPrinter>(new QPrinter(printer)));
    }
    else
    {
        return QMarginsF();
    }
}

//---------------------------------------------------------------------------------------------------------------------
Unit DialogLayoutSettings::PaperUnit() const
{
    return StrToUnits(ui->comboBoxPaperSizeUnit->currentData().toString());
}

//---------------------------------------------------------------------------------------------------------------------
Unit DialogLayoutSettings::LayoutUnit() const
{
    return StrToUnits(ui->comboBoxLayoutUnit->currentData().toString());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::CorrectPaperDecimals()
{
    switch (oldPaperUnit)
    {
        case Unit::Cm:
        case Unit::Mm:
        case Unit::Px:
            ui->doubleSpinBoxPaperWidth->setDecimals(2);
            ui->doubleSpinBoxPaperHeight->setDecimals(2);

            ui->doubleSpinBoxLeftField->setDecimals(4);
            ui->doubleSpinBoxRightField->setDecimals(4);
            ui->doubleSpinBoxTopField->setDecimals(4);
            ui->doubleSpinBoxBottomField->setDecimals(4);
            break;
        case Unit::Inch:
            ui->doubleSpinBoxPaperWidth->setDecimals(5);
            ui->doubleSpinBoxPaperHeight->setDecimals(5);

            ui->doubleSpinBoxLeftField->setDecimals(5);
            ui->doubleSpinBoxRightField->setDecimals(5);
            ui->doubleSpinBoxTopField->setDecimals(5);
            ui->doubleSpinBoxBottomField->setDecimals(5);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::CorrectLayoutDecimals()
{
    switch (oldLayoutUnit)
    {
        case Unit::Cm:
        case Unit::Mm:
        case Unit::Px:
            ui->doubleSpinBoxLayoutWidth->setDecimals(2);
            ui->doubleSpinBoxShift->setDecimals(2);
            break;
        case Unit::Inch:
            ui->doubleSpinBoxLayoutWidth->setDecimals(5);
            ui->doubleSpinBoxShift->setDecimals(5);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::MinimumPaperSize()
{
    const qreal value = UnitConvertor(1, Unit::Px, oldPaperUnit);
    ui->doubleSpinBoxPaperWidth->setMinimum(value);
    ui->doubleSpinBoxPaperHeight->setMinimum(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::MinimumLayoutSize()
{
    const qreal value = UnitConvertor(1, Unit::Px, oldLayoutUnit);
    ui->doubleSpinBoxLayoutWidth->setMinimum(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::ReadSettings()
{
    const VSettings *settings = qApp->Seamly2DSettings();
    SetLayoutWidth(settings->GetLayoutWidth());
    SetShift(settings->GetLayoutShift());

    const qreal width = UnitConvertor(settings->GetLayoutPaperWidth(), Unit::Px, LayoutUnit());
    const qreal height = UnitConvertor(settings->GetLayoutPaperHeight(), Unit::Px, LayoutUnit());
    SheetSize(QSizeF(width, height));
    SetGroup(settings->GetLayoutGroup());
    SetRotate(settings->GetLayoutRotate());
    SetIncrease(settings->GetLayoutRotationIncrease());
    SetAutoCrop(settings->GetLayoutAutoCrop());
    SetSaveLength(settings->GetLayoutSaveLength());
    SetUnitePages(settings->GetLayoutUnitePages());
    SetFields(settings->GetFields(GetDefPrinterFields()));
    SetIgnoreAllFields(settings->GetIgnoreAllFields());
    SetStripOptimization(settings->GetStripOptimization());
    SetMultiplier(settings->GetMultiplier());
    SetTextAsPaths(settings->GetTextAsPaths());

    FindTemplate();

    CorrectMaxFileds();
    IgnoreAllFields(ui->checkBoxIgnoreFileds->isChecked());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::WriteSettings() const
{
    VSettings *settings = qApp->Seamly2DSettings();
    settings->SetLayoutWidth(GetLayoutWidth());
    settings->SetLayoutGroup(GetGroup());
    settings->SetLayoutPaperHeight(GetPaperHeight());
    settings->SetLayoutPaperWidth(GetPaperWidth());
    settings->SetLayoutShift(GetShift());
    settings->SetLayoutRotate(GetRotate());
    settings->SetLayoutRotationIncrease(GetIncrease());
    settings->SetLayoutAutoCrop(GetAutoCrop());
    settings->SetLayoutSaveLength(IsSaveLength());
    settings->SetLayoutUnitePages(IsUnitePages());
    settings->SetFields(GetFields());
    settings->SetIgnoreAllFields(IsIgnoreAllFields());
    settings->SetStripOptimization(IsStripOptimization());
    settings->SetMultiplier(GetMultiplier());
    settings->SetTextAsPaths(IsTextAsPaths());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SheetSize(const QSizeF &size)
{
    oldPaperUnit = PaperUnit();
    ui->doubleSpinBoxPaperWidth->setMaximum(FromPixel(QIMAGE_MAX, oldPaperUnit));
    ui->doubleSpinBoxPaperHeight->setMaximum(FromPixel(QIMAGE_MAX, oldPaperUnit));

    ui->doubleSpinBoxPaperWidth->setValue(size.width());
    ui->doubleSpinBoxPaperHeight->setValue(size.height());

    CorrectPaperDecimals();
    PaperSizeChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutSettings::SetAdditionalOptions(bool value)
{
    SetAutoCrop(value);
    SetSaveLength(value);
    SetUnitePages(value);
    SetStripOptimization(value);
}
