#include "cameracinematic.h"
#include "ui_Editor.h"
//#include "ui_form.h"

CameraCinematic::CameraCinematic(QWidget *parent)
    : QMainWindow(parent)
    , dbcCinematicCamera(this)
    , camModel(this)
    , camSkin(this)
    , hProc(this)
    , ui(new Ui::CameraCinematic)
{
    ui->setupUi(this);
    ui->tab->setCurrentIndex(0);
    selectedTable = ui->pos_table;

    createFileMenu();
    createEditMenu();
    createToolBar();

    ui->label_speedRatio->setText(QString::number((float)ui->speedRatio->value()/1000.0f));

    ui->label_accelerationRatio->setText(QString("Ratio %1 ms/f").arg((float)ui->accelerationRatio->value()));
    ui->label_accelerationPercent->setText(QString("Percent %1%").arg(ui->accelerationPercent->value()));

    updateTimeView = new QTimer(this);

    ui->pos_interpolation->setCurrentIndex(1);
    ui->tar_interpolation->setCurrentIndex(1);
    ui->roll_interpolation->setCurrentIndex(1);

    QIntValidator *validator_number = new QIntValidator(0,300000,this);
    ui->editAnimationLength->setValidator(validator_number);

    /* Begin QTableWindget Option */
    connect(ui->addVect, &QPushButton::clicked, [=]() {addRow();});
    connect(ui->removeVect, &QPushButton::clicked, [=]() {removeRow();});
    connect(ui->brushTime, &QPushButton::clicked, [=]() {brushTime();});
    connect(ui->upRow, &QPushButton::clicked, [=]() {moveRow(true);});
    connect(ui->downRow, &QPushButton::clicked, [=]() {moveRow(false);});
    /* End QTableWindget Option */

    connect(ui->tab, &QTabWidget::currentChanged, [=](int c) {
        switch (c) {
        case 0: selectedTable = ui->pos_table; break;
        case 1: selectedTable = ui->tar_table; break;
        case 2: selectedTable = ui->roll_table; break;
        }

        updateRowList();
        updateVectorList();
    });

    connect(ui->generate, &QPushButton::clicked, [=]() {generateFile();});
    connect(ui->editName, &QLineEdit::textChanged, [=]() {updateModelInfo();});
    connect(ui->editAnimationLength, &QLineEdit::textChanged, [=]() {updateModelInfo();});

    connect(ui->pos_table, &QTableWidget::itemSelectionChanged, [=]() {sendVectors();});
    connect(ui->tar_table, &QTableWidget::itemSelectionChanged, [=]() {sendVectors();});
    connect(ui->roll_table, &QTableWidget::itemSelectionChanged, [=]() {sendVectors();});

    connect(ui->pos_table, &QTableWidget::itemDoubleClicked, [=](QTableWidgetItem* i) { savePosItemText = i->text(); });
    connect(ui->pos_table, &QTableWidget::itemChanged, [=](QTableWidgetItem* i) {
        if (ignoreStack)
            return;

        if (i->text() != savePosItemText)
        {
            QVector<QString> toPush;
            toPush.push_back("1");
            toPush.push_back("0");
            toPush.push_back(QString::number(i->row()));
            toPush.push_back(QString::number(i->column()));
            toPush.push_back(savePosItemText);

            pushActionToArrayUndo(toPush);
        }
    });

    connect(ui->tar_table, &QTableWidget::itemDoubleClicked, [=](QTableWidgetItem* i) { saveTarItemText = i->text(); });
    connect(ui->tar_table, &QTableWidget::itemChanged, [=](QTableWidgetItem* i) {
        if (ignoreStack)
            return;

        if (i->text() != saveTarItemText)
        {
            QVector<QString> toPush;
            toPush.push_back("1");
            toPush.push_back("0");
            toPush.push_back(QString::number(i->row()));
            toPush.push_back(QString::number(i->column()));
            toPush.push_back(saveTarItemText);

            pushActionToArrayUndo(toPush);
        }
    });

    connect(ui->roll_table, &QTableWidget::itemDoubleClicked, [=](QTableWidgetItem* i) { saveRollItemText = i->text(); });
    connect(ui->roll_table, &QTableWidget::itemChanged, [=](QTableWidgetItem* i) {
        if (ignoreStack)
            return;

        if (i->text() != saveRollItemText)
        {
            QVector<QString> toPush;
            toPush.push_back("1");
            toPush.push_back("0");
            toPush.push_back(QString::number(i->row()));
            toPush.push_back(QString::number(i->column()));
            toPush.push_back(saveRollItemText);

            pushActionToArrayUndo(toPush);
        }
    });

    connect(ui->showPositions, &QCheckBox::stateChanged, [=]() {sendVectors();});
    connect(ui->showTargets, &QCheckBox::stateChanged, [=]() {sendVectors();});

    connect(ui->curvePositions, &QCheckBox::stateChanged, [=]() {sendCurves();});
    connect(ui->curveTargets, &QCheckBox::stateChanged, [=]() {sendCurves();});
    connect(ui->curveOnTop, &QCheckBox::stateChanged, [=]() {sendCurves();});

    connect(ui->displayAxe, &QComboBox::currentIndexChanged, [=](int index) {ui->graphicsView->changeDisplay(index); sendVectors();});
    connect(ui->graphicsView, SIGNAL(savePosition(QVector3D,bool)), this, SLOT(storePosition(QVector3D,bool)));
    connect(ui->graphicsView, SIGNAL(createPoint(QVector3D,bool)), this, SLOT(storePosition(QVector3D,bool)));
    connect(ui->graphicsView, SIGNAL(selectedItem(int,int,int)), this, SLOT(selectItem(int,int,int)));

    connect(ui->pos_interpolation, &QComboBox::currentIndexChanged, [=](int index) {interpolation[0] = (quint16)index; sendVectors();});
    connect(ui->tar_interpolation, &QComboBox::currentIndexChanged, [=](int index) {interpolation[1] = (quint16)index; sendVectors();});
    connect(ui->roll_interpolation, &QComboBox::currentIndexChanged, [=](int index) {interpolation[2] = (quint16)index; sendVectors();});

    connect(updateTimeView, SIGNAL(timeout()), this, SLOT(updateCinematic()));
    connect(ui->viewTimeCheck, &QCheckBox::stateChanged, [=]() {
        ui->viewTime->setEnabled(ui->viewTimeCheck->isChecked());
        updateViewTime(ui->viewTime->value());
    });

    connect(ui->viewTime, &QSlider::valueChanged, [=](int value) {
        ui->label_viewTime->setText(QString("%1 ms").arg(value));
        updateViewTime(value);
    });

    connect(ui->viewTimeStart, &QPushButton::clicked, [=]() {
       ui->viewTimeStart->setEnabled(false);
       ui->viewTime->setEnabled(false);

       ui->viewTime->setValue(0);
       ui->viewTime->setMaximum(ui->editAnimationLength->text().toInt());

       StartTime = time();
       OffTime = StartTime;
       EndTime = StartTime + ui->viewTime->maximum();

       updateTimeView->start(20);
    });

    connect(&camSkin, SIGNAL(skinDone()), this, SLOT(skinFileDone()));
    connect(&camModel, SIGNAL(runDone()), this, SLOT(m2FileDone()));
    connect(&camModel, SIGNAL(readDone()), this, SLOT(m2FileRead()));
    connect(&camModel, SIGNAL(updateDone()), this, SLOT(m2FileUpdate()));

    connect(ui->dbcCameraList, &QComboBox::currentIndexChanged, [=]() {updateDBCInfo(false);});

    connect(ui->clientPosition, &QPushButton::clicked, [=]() { getClientLocation(); });
    connect(ui->selectedPosition, &QPushButton::clicked, [=]() {createPointFromStoredPosition();});

    connect(ui->alignVector, &QPushButton::clicked, [=]() {alignVector();});
    connect(ui->normalizeSpeed, &QPushButton::clicked, [=]() {normalizeSpeed();});
    connect(ui->speedRatio, &QSlider::valueChanged, [=](int value) { ui->label_speedRatio->setText(QString::number((float)value/1000.0f)); });
    connect(ui->accelerationRatio, &QSlider::valueChanged, [=](int value) {
        ui->label_accelerationRatio->setText(QString("Ratio %1 ms/f").arg((float)value));
        sendCurves();
    });
    connect(ui->accelerationPercent, &QSlider::valueChanged, [=](int value) {
        ui->label_accelerationPercent->setText(QString("Percent %1%").arg(value));
        sendCurves();
    });
    connect(ui->accelerationPosition, &QCheckBox::stateChanged, [=]() {sendCurves();});
    connect(ui->accelerationTarget, &QCheckBox::stateChanged, [=]() {sendCurves();});

    updateDBC();
    updateTableColor();
    updateRowList();
    updateVectorList();
}

CameraCinematic::~CameraCinematic() { delete ui; }

void CameraCinematic::updateCinematic()
{
    uint64_t CurTime = time();
    uint64_t add = ( CurTime >= EndTime ) ? ui->viewTime->maximum() : CurTime - StartTime;

    ui->viewTime->setValue(add);
    updateViewTime(add);

    if ( CurTime >= EndTime )
    {
        updateTimeView->stop();
        ui->viewTime->setEnabled(true);
        ui->viewTimeStart->setEnabled(true);
    }
}

void CameraCinematic::getClientLocation()
{
    hProc.run();
    QVector<QString> toUndo;
    toUndo.push_back("3");

    if (!hProc.getError().isEmpty())
    {
        ui->output->setText(hProc.getError());
        return;
    }

    int row = ui->rowList->currentIndex();
    int vec = ui->vectorList->currentIndex();

    ignoreStack = true;
    for (int n = 0; n < 3; ++n)
    {
        int column = (n + 1) + (vec * 3);
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignmentFlag::AlignCenter);

        float coord = hProc.getCoord(n, facing, origin,
                                     ui->applyOrientationClient->isChecked(),
                                     ui->applyOffsetClient->isChecked());

        /* Create undo stack */
        toUndo.push_back(QString::number(ui->tab->currentIndex()));
        toUndo.push_back(QString::number(row));
        toUndo.push_back(QString::number(column));
        toUndo.push_back(selectedTable->item(row, column)->text());
        /* End undo stack */

        item->setText(QString::number(coord));

        selectedTable->setItem(row, column, item);
    }
    ignoreStack = false;

    pushActionToArrayUndo(toUndo);
    sendVectors();
}

bool CameraCinematic::callAutoSave(bool ignore)
{
    if ( ui->pos_table->rowCount() == 0
         && ui->tar_table->rowCount() == 0
         && ui->roll_table->rowCount() == 0 )
    {
        return true;
    }

    ignoreAutoSave = ignore;

    QMessageBox* autoSave = new QMessageBox(QMessageBox::Question, "CameraCinematic - AutoSave",
                                            tr("Did you want to save your progress\nbefore close the application ?"),
                                            QMessageBox::Yes | QMessageBox::No);
    autoSave->setWindowFlags(Qt::WindowTitleHint | Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::CustomizeWindowHint);

    switch(autoSave->exec())
    {
    case QMessageBox::Yes:
        if (generateFile())
            return true;

        return false;
        break;
    case QMessageBox::No:
        return true;
        break;
    }

    return false;
}

void CameraCinematic::pushActionToArrayRedo(QVector<QString> toStore)
{
    /* toStore structure :
     *
     * countAction
     * action_n
     *
     * action_ {
     *     table, row, column, data
     * }
     */

    // remove stack above
    if (redo_selector >= 0)
        for (int e = 0; e <= redo_selector; ++e)
            redoStack.removeFirst();

    // always return to top of stack
    redo_selector = -1;
    redoAct->setEnabled(true);

    redoStack.push_front(toStore);

    // max 10 undo action
    if (redoStack.size() == MAX_UNDO+1)
        redoStack.remove(MAX_UNDO);
}

void CameraCinematic::pushActionToArrayUndo(QVector<QString> toStore)
{
    /* toStore structure :
     *
     * countAction
     * action_n
     *
     * action_ {
     *     table, row, column, data
     * }
     */

    // remove stack above
    if (undo_selector >= 0)
    {
        for (int e = 0; e <= undo_selector; ++e)
            undoStack.removeFirst();

        redoStack.clear();
        redoAct->setEnabled(false);
    }

    // always return to top of stack
    undo_selector = -1;

    undoStack.push_front(toStore);
    undoAct->setEnabled(true);

    // max 10 undo action
    if (undoStack.size() == MAX_UNDO+1)
        undoStack.remove(MAX_UNDO);
}

void CameraCinematic::applyUndo()
{
    undo_selector++;

    if (undo_selector == undoStack.size())
    {
        undo_selector--;
        return;
    }

    if (undo_selector == undoStack.size() - 1)
        undoAct->setEnabled(false);

    ignoreStack = true;

    if (undo_selector >= 0 && undo_selector < undoStack.size() && !undoStack.isEmpty())
    {
        QVector<QString> toStore;
        int cAct = undoStack[undo_selector][0].toUInt();
        toStore.push_back(QString::number(cAct));

        for (int action = 0; action < cAct; ++action)
        {
            int table = undoStack[undo_selector][action*4 + 1].toUInt();
            int row = undoStack[undo_selector][action*4 + 2].toUInt();
            int column = undoStack[undo_selector][action*4 + 3].toUInt();
            toStore.push_back(QString::number(table));
            toStore.push_back(QString::number(row));
            toStore.push_back(QString::number(column));

            QTableWidget* t;
            switch (table)
            {
            case 0:
                t = ui->pos_table;
                break;
            case 1:
                t = ui->tar_table;
                break;
            case 2:
                t = ui->roll_table;
                break;
            default:
                return;
                break;
            }

            if (QTableWidgetItem* i = t->item(row, column))
            {
                toStore.push_back(i->text());
                i->setText(undoStack[undo_selector][action*4 + 4]);
            }
        }

        pushActionToArrayRedo(toStore);
        sendVectors();
    }

    ignoreStack = false;
}

void CameraCinematic::applyRedo()
{
    redo_selector++;

    if (redo_selector == redoStack.size())
    {
        redo_selector--;
        return;
    }

    if (redo_selector == redoStack.size() - 1)
        redoAct->setEnabled(false);

    undo_selector--;
    undoAct->setEnabled(true);

    if (undo_selector < -1)
        undo_selector = -1;

    ignoreStack = true;

    if (redo_selector >= 0 && redo_selector < redoStack.size() && !redoStack.isEmpty())
    {
        int cAct = redoStack[redo_selector][0].toUInt();

        for (int action = 0; action < cAct; ++action)
        {
            int table = redoStack[redo_selector][action*4 + 1].toUInt();
            int row = redoStack[redo_selector][action*4 + 2].toUInt();
            int column = redoStack[redo_selector][action*4 + 3].toUInt();

            QTableWidget* t;
            switch (table)
            {
            case 0:
                t = ui->pos_table;
                break;
            case 1:
                t = ui->tar_table;
                break;
            case 2:
                t = ui->roll_table;
                break;
            default:
                return;
                break;
            }

            if (QTableWidgetItem* i = t->item(row, column))
            {
                i->setText(redoStack[redo_selector][action*4 + 4]);
            }
        }

        sendVectors();
    }

    ignoreStack = false;
}

void CameraCinematic::closeEvent(QCloseEvent *event)
{
    if (ignoreAutoSave)
        return;

    if (callAutoSave())
    {
        event->accept();
        return;
    }

    event->ignore();
    return;
}

void CameraCinematic::selectItem(int type, int row, int vec)
{
    if ( type <= 3 && type >= 0 )
        ui->tab->setCurrentIndex(type);

    if ( row <= ui->rowList->count()-1 && row >= 0 )
        ui->rowList->setCurrentIndex(row);

    if ( vec <= ui->vectorList->count()-1 && vec >= 0 )
        ui->vectorList->setCurrentIndex(vec);
}

void CameraCinematic::storePosition(QVector3D pos, bool create)
{
    ui->sceneX->setText(QString("X : %1").arg(pos.x()));
    ui->sceneY->setText(QString("Y : %1").arg(pos.y()));
    ui->sceneZ->setText(QString("Z : %1").arg(pos.z()));

    switch (ui->displayAxe->currentIndex())
    {
    case 0: // X, Y
        ui->sceneZ->setText("Z : null");
        break;
    case 1: // X, Z
        ui->sceneY->setText("Y : null");
        break;
    case 2: // Y, Z
        ui->sceneX->setText("X : null");
        break;
    }

    if ( create )
        createPointFromStoredPosition();
}

void CameraCinematic::getFileInformation()
{
    QString modelPath = QFileDialog::getOpenFileName(this, tr("Open Model Camera"), "/",
                                                     tr("Model Files (*.m2 *.mdx)"));
    QString path = QFileInfo(modelPath).absoluteDir().absolutePath();

    if ( modelPath.isEmpty() )
        return;

    QSettings setting("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("MODEL-SETTINGS");
    setting.setValue("model", modelPath);
    setting.setValue("name", modelPath.remove(path).remove(".m2"));
    setting.setValue("path", path);
    setting.endGroup();
}

void CameraCinematic::openModelFile()
{
    QString path;
    QString skinPath;

    QSettings setting("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("MODEL-SETTINGS");
        path = setting.value("model").toString();
        skinPath = path;
    setting.endGroup();

    camSkin.setPath(skinPath.replace(".m2", "00.skin"));
    camModel.setPath(path);
    camModel.read();

    updateTableColor();
    updateRowList();
    sendVectors();

    undoAct->setEnabled(false);
    undoStack.clear();
}

/* Begin QTableWindget Option */

void CameraCinematic::addRow()
{
    selectedTable->insertRow(selectedTable->rowCount());

    for (int i = 0; i < selectedTable->columnCount(); ++i)
    {
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignmentFlag::AlignCenter);

        if ( i == 0 ) item->setText(QString("0"));
        else item->setText(QString("0.0"));

        selectedTable->setItem(selectedTable->rowCount()-1, i, item);
    }

    updateTableColor();
    updateRowList();
    sendVectors();
}

void CameraCinematic::removeRow()
{
    QList<QTableWidgetItem*> pos_items = selectedTable->selectedItems();

    if (pos_items.isEmpty())
        return;

    for (QTableWidgetItem* item : pos_items)
        selectedTable->removeRow(item->row());

    updateRowList();
    sendVectors();
}

void CameraCinematic::brushTime()
{
    QVector<float> lenght;
    float total = 0.0f;
    float t = 0.0f;

    if ( selectedTable->rowCount() == 0 )
        return;

    if ( selectedTable == ui->pos_table ) lenght = ui->graphicsView->getLenghtPositions();
    else lenght = ui->graphicsView->getLenghtTargets();

    if (lenght.isEmpty())
        return;

    for (int row = 0; row < lenght.size(); ++row)
        total += lenght[row];

    selectedTable->item(0, 0)->setText("0");

    for (int row = 0; row < selectedTable->rowCount() - 1; ++row )
    {
        t += animationLength * lenght[row] / total;
        selectedTable->item(row+1, 0)->setText(QString::number((int)t));
    }

    sendCurves();
}

void CameraCinematic::moveRow(bool up)
{
    if ( selectedTable->selectedItems().isEmpty() )
        return;

    QTableWidgetItem* item = selectedTable->selectedItems()[0];
    int selectedRow = item->row();
    int selectedColumn = item->column();

    int direction = (up) ? -1 : 1;

    if ( selectedRow == 0 && direction == -1 )
        return;

    if ( selectedRow == selectedTable->rowCount()-1 && direction == 1 )
        return;

    QVector<QString> itemsRow;
    QVector<QString> newRow;

    for ( int column = 0; column < selectedTable->columnCount(); ++column )
    {
        itemsRow.push_back(selectedTable->item(selectedRow, column)->text());
        newRow.push_back(selectedTable->item(selectedRow + direction, column)->text());
    }


    for ( int column = 0; column < selectedTable->columnCount(); ++column )
    {
        selectedTable->item(selectedRow + direction, column)->setText(itemsRow[column]);
        selectedTable->item(selectedRow, column)->setText(newRow[column]);
    }

    QModelIndex index = selectedTable->model()->index(selectedRow + direction, selectedColumn);
    selectedTable->setCurrentIndex(index);
    selectedTable->setFocus();

    updateRowList();
    sendVectors();
}

/* End QTableWindget Option */

void CameraCinematic::updateModelInfo()
{
    name = ui->editName->text();
    animationLength = ui->editAnimationLength->text().toUInt();

    ui->viewTime->setMaximum(animationLength);
    ui->viewTime->setSingleStep(animationLength/100);

    //ui->name->setText(name + ".m2 | " + name + "00.skin");
    updateDBCInfo();
}

void CameraCinematic::updateDBCInfo(bool tryToFoundName)
{
    QVector<std::uint32_t> dbcList = dbcCinematicCamera.getListID();
    int index = 0;

    if (tryToFoundName)
    {
        for (int i = 0; i < ui->dbcCameraList->count(); ++i)
        {
            if ( ui->dbcCameraList->itemText(i).contains(name) )
            {
                ui->dbcCameraList->setCurrentIndex(i);
                index = i;
                break;
            }
        }
    }

    if (dbcList.isEmpty())
        return;

    QVector<float> dbcValues = dbcCinematicCamera.getVectorByID(dbcList[index]);

    origin[0] = dbcValues[0];
    origin[1] = dbcValues[1];
    origin[2] = dbcValues[2];
    facing = dbcValues[3];

    ui->dbc_information->setText(QString("Selected DBC Information : OriginX : %1 | OriginY : %2 | OriginZ : %3 | OriginFacing : %4")
                                    .arg(dbcValues[0]).arg(dbcValues[1]).arg(dbcValues[2]).arg(dbcValues[3]));
}

bool CameraCinematic::generateFile()
{
    if (name.isEmpty())
    {
        ui->output->setText("Error: You must define a name");
        return false;
    }

    if (animationLength == 0)
    {
        ui->output->setText("Error: You must set an animation length");
        return false;
    }

    camSkin.setName(name);
    camSkin.run();

    std::vector<timestampedValue<VecF<9>>> positions;
    for (int p = 0; p < ui->pos_table->rowCount(); ++p)
    {
        timestampedValue<VecF<9>> current;
        VecF<9> t;

        ui->pos_table->selectRow(p);
        QList<QTableWidgetItem*> pos_items = ui->pos_table->selectedItems();

        for (QTableWidgetItem* item : pos_items)
        {
            if (item->column() == 0) current.stamp = item->text().toUInt();
            else t[item->column()-1] = item->text().toFloat();
        }

        current.data = t;

        positions.push_back(current);
    }

    std::vector<timestampedValue<VecF<9>>> targets;
    for (int p = 0; p < ui->tar_table->rowCount(); ++p)
    {
        timestampedValue<VecF<9>> current;
        VecF<9> t;

        ui->tar_table->selectRow(p);
        QList<QTableWidgetItem*> tar_items = ui->tar_table->selectedItems();

        for (QTableWidgetItem* item : tar_items)
        {
            if (item->column() == 0) current.stamp = item->text().toUInt();
            else t[item->column()-1] = item->text().toFloat();
        }

        current.data = t;
        targets.push_back(current);
    }

    std::vector<timestampedValue<VecF<3>>> rolls;
    for (int p = 0; p < ui->roll_table->rowCount(); ++p)
    {
        timestampedValue<VecF<3>> current;
        VecF<3> t;

        ui->roll_table->selectRow(p);
        QList<QTableWidgetItem*> roll_items = ui->roll_table->selectedItems();

        for (QTableWidgetItem* item : roll_items)
        {
            if (item->column() == 0) current.stamp = item->text().toUInt();
            else t[item->column()-1] = item->text().toFloat();
        }

        current.data = t;
        rolls.push_back(current);
    }

    camModel.setPositions(positions);
    camModel.setTargets(targets);
    camModel.setRolls(rolls);

    camModel.setName(name);
    camModel.setInterpolation(interpolation);
    camModel.setAnimationLength(animationLength);
    camModel.run();

    return true;
}

void CameraCinematic::updateTableColor()
{
    QColor posColor, tarColor;

    QSettings setting("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("COLOR-SETTINGS");
    posColor = QColor(setting.value("pos-prim").toString());
    tarColor = QColor(setting.value("tar-prim").toString());
    setting.endGroup();

    QFont font = ui->tab->tabBar()->font();
    font.setPointSize(13);
    ui->tab->tabBar()->setFont(font);

    ui->tab->tabBar()->setTabTextColor(0, posColor);
    ui->tab->tabBar()->setTabTextColor(1, tarColor);

    posColor.setAlphaF(.1f);
    tarColor.setAlphaF(.1f);

    for (int c = 0; c < 10; ++c)
        for (int r = 0; r < ui->pos_table->rowCount(); ++r)
            if ( r%2 != 0 )
                if (QTableWidgetItem* i = ui->pos_table->item(r, c))
                    i->setBackground(posColor);

    for (int c = 0; c < 10; ++c)
        for (int r = 0; r < ui->tar_table->rowCount(); ++r)
            if ( r%2 != 0 )
                if (QTableWidgetItem* i = ui->tar_table->item(r, c))
                    i->setBackground(tarColor);
}

void CameraCinematic::skinFileDone()
{ ui->output->setText("Success: skin file is generated"); }

void CameraCinematic::m2FileDone()
{ ui->output->setText("Success: m2 file is generated"); }

void CameraCinematic::m2FileRead()
{
    for (int i = 0; i < 3; ++i)
        interpolation[i] = camModel.getInterpolation(i);

    ui->editName->setText(camModel.getName());
    ui->editAnimationLength->setText(QString::number(camModel.getAnimationLength()));
    ui->pos_interpolation->setCurrentIndex(interpolation[0]);
    ui->tar_interpolation->setCurrentIndex(interpolation[1]);
    ui->roll_interpolation->setCurrentIndex(interpolation[2]);

    updateModelInfo();
    ui->graphicsView->clear();

    ui->pos_table->setRowCount(0);
    ui->tar_table->setRowCount(0);
    ui->roll_table->setRowCount(0);

    std::vector<timestampedValue<VecF<9>>> positions = camModel.getPositions();
    std::vector<timestampedValue<VecF<9>>> targets = camModel.getTargets();
    std::vector<timestampedValue<VecF<3>>> rolls = camModel.getRolls();

    for (std::uint32_t i = 0; i < positions.size(); ++i)
    {
        ui->pos_table->insertRow(ui->pos_table->rowCount());

        for (int n = 0; n < ui->pos_table->columnCount(); ++n)
        {
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignmentFlag::AlignCenter);

            if ( n == 0 ) item->setText(QString::number(positions[i].stamp));
            else item->setText(QString::number(positions[i].data[n-1], 'G', 5));

             ui->pos_table->setItem(ui->pos_table->rowCount()-1, n, item);
        }
    }

    for (std::uint32_t i = 0; i < targets.size(); ++i)
    {
        ui->tar_table->insertRow(ui->tar_table->rowCount());

        for (int n = 0; n < ui->tar_table->columnCount(); ++n)
        {
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignmentFlag::AlignCenter);

            if ( n == 0 ) item->setText(QString::number(targets[i].stamp));
            else item->setText(QString::number(targets[i].data[n-1], 'G', 5));

             ui->tar_table->setItem(ui->tar_table->rowCount()-1, n, item);
        }
    }

    for (std::uint32_t i = 0; i < rolls.size(); ++i)
    {
        ui->roll_table->insertRow(ui->roll_table->rowCount());

        for (int n = 0; n < ui->roll_table->columnCount(); ++n)
        {
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignmentFlag::AlignCenter);

            if ( n == 0 ) item->setText(QString::number(rolls[i].stamp));
            else item->setText(QString::number(rolls[i].data[n-1], 'G', 5));

             ui->roll_table->setItem(ui->roll_table->rowCount()-1, n, item);
        }
    }

    ui->output->setText("Success: m2 file is loaded");
}

void CameraCinematic::m2FileUpdate()
{ ui->output->setText("Success: m2 file is updated"); }

void CameraCinematic::alignVector()
{
    int row = ui->rowList->currentIndex();
    int vec = ui->vectorList->currentIndex();
    int mirror;

    QVector<QString> toUndo;
    toUndo.push_back("3");

    if ( vec == 0 ) vec = 1;

    mirror = ( vec == 1 ) ? 2 : 1;

    QVector3D self = QVector3D(selectedTable->item(row, 1 + X)->text().toFloat(),
                               selectedTable->item(row, 1 + Y)->text().toFloat(),
                               selectedTable->item(row, 1 + Z)->text().toFloat());

    QVector3D cur = QVector3D(selectedTable->item(row, 1 + (3 * vec) + X)->text().toFloat(),
                              selectedTable->item(row, 1 + (3 * vec) + Y)->text().toFloat(),
                              selectedTable->item(row, 1 + (3 * vec) + Z)->text().toFloat());

    QVector3D mir = QVector3D(selectedTable->item(row, 1 + (3 * mirror) + X)->text().toFloat(),
                              selectedTable->item(row, 1 + (3 * mirror) + Y)->text().toFloat(),
                              selectedTable->item(row, 1 + (3 * mirror) + Z)->text().toFloat());

    float distanceSelfCur = distance3D(self, cur);
    float distanceSelfMir = distance3D(self, mir);

    QVector3D offset = QVector3D(self.x()-cur.x(),
                        self.y()-cur.y(),
                        self.z()-cur.z()).operator*=(((distanceSelfMir*100.0)/distanceSelfCur)/100);

    QVector3D newVec = QVector3D(self.x() + offset.x(),
                                 self.y() + offset.y(),
                                 self.z() + offset.z());

    ignoreStack = true;
    for (int i = 0 ; i < 3 ; ++i)
    {
        int column = 1 + (3 * mirror) + i;

        /* Create undo stack */
        toUndo.push_back(QString::number(ui->tab->currentIndex()));
        toUndo.push_back(QString::number(row));
        toUndo.push_back(QString::number(column));
        toUndo.push_back(selectedTable->item(row, column)->text());
        /* End undo stack*/

        selectedTable->item(row, column)->setText(QString::number(newVec[i]));
    }
    ignoreStack = false;

    pushActionToArrayUndo(toUndo);
    sendVectors();
}

void CameraCinematic::normalizeSpeed()
{
    int row = ui->rowList->currentIndex();
    int vec = ui->vectorList->currentIndex();
    int mirror;

    QVector<QString> toUndo;
    toUndo.push_back("3");

    if ( vec == 0 ) vec = 1;

    mirror = ( vec == 1 ) ? 2 : 1;

    QVector3D self = QVector3D(selectedTable->item(row, 1 + X)->text().toFloat(),
                               selectedTable->item(row, 1 + Y)->text().toFloat(),
                               selectedTable->item(row, 1 + Z)->text().toFloat());

    QVector3D cur = QVector3D(selectedTable->item(row, 1 + (3 * vec) + X)->text().toFloat(),
                              selectedTable->item(row, 1 + (3 * vec) + Y)->text().toFloat(),
                              selectedTable->item(row, 1 + (3 * vec) + Z)->text().toFloat());

    QVector3D mir = QVector3D(selectedTable->item(row, 1 + (3 * mirror) + X)->text().toFloat(),
                              selectedTable->item(row, 1 + (3 * mirror) + Y)->text().toFloat(),
                              selectedTable->item(row, 1 + (3 * mirror) + Z)->text().toFloat());

    float distanceSelfCur = distance3D(self, cur);
    float distanceSelfMir = distance3D(self, mir);

    float ratio;
    float speedRatio = (float)ui->speedRatio->value() / 1000.0f;

    if ( ((distanceSelfCur*100.0f)/distanceSelfMir)/100.0f < 1.0f-speedRatio )
        ratio = (((distanceSelfCur*100.0f)/distanceSelfMir)/100.0f) * (1.0f-speedRatio);
    else if ( ((distanceSelfCur*100.0f)/distanceSelfMir)/100.0f > 1.0f+speedRatio )
        ratio = (((distanceSelfCur*100.0f)/distanceSelfMir)/100.0f) * (1.0f+speedRatio);
    else ratio = ((distanceSelfCur*100.0f)/distanceSelfMir)/100.0f;

    QVector3D offMir = QVector3D(mir.x() - self.x(),
                                 mir.y() - self.y(),
                                 mir.z() - self.z()).operator*=(ratio);

    QVector3D newVec = QVector3D(self.x() + offMir.x(),
                                 self.y() + offMir.y(),
                                 self.z() + offMir.z());

    ignoreStack = true;
    for (int i = 0 ; i < 3 ; ++i)
    {
        int column = 1 + (3 * mirror) + i;

        /* Create undo stack */
        toUndo.push_back(QString::number(ui->tab->currentIndex()));
        toUndo.push_back(QString::number(row));
        toUndo.push_back(QString::number(column));
        toUndo.push_back(selectedTable->item(row, column)->text());
        /* End undo stack*/

        selectedTable->item(row, column)->setText(QString::number(newVec[i]));
    }
    ignoreStack = false;

    pushActionToArrayUndo(toUndo);
    sendVectors();
}

void CameraCinematic::sendVectors()
{    
    QVector<QVector<QVector<float>>> pos;
    QVector<QVector<QVector<float>>> tar;
    //QVector<QVector<QVector<float>>> roll;

    for (int row = 0; row < ui->pos_table->rowCount(); ++row)
    {
        QVector<QVector<float>> f;
        pos.push_back(f);

        for (int vec = 0; vec < 3; ++vec)
        {
            QVector<float> f;
            pos[row].push_back(f);

            for (int i = 0; i < 3; ++i)
            {
                pos[row][vec].push_back( ui->pos_table->item(row, 1 + (vec * 3) + i)->text().toFloat() );
            }
        }
    }

    for (int row = 0; row < ui->tar_table->rowCount(); ++row)
    {
        QVector<QVector<float>> f;
        tar.push_back(f);

        for (int vec = 0; vec < 3; ++vec)
        {
            QVector<float> f;
            tar[row].push_back(f);

            for (int i = 0; i < 3; ++i)
            {
                tar[row][vec].push_back( ui->tar_table->item(row, 1 + (vec * 3) + i)->text().toFloat() );
            }
        }
    }

    /*for (int row = 0; row < ui->roll_table->rowCount(); ++row)
    {
        QVector<QVector<float>> f;
        QVector<float> g;
        roll.push_back(f);
        roll[row].push_back(g);

        for (int i = 0; i < 3; ++i)
        {
            tar[row][0].push_back( ui->roll_table->item(row, 1 + i)->text().toFloat() );
        }
    }*/

    bool show[2] = {
        ui->showPositions->isChecked(),
        ui->showTargets->isChecked()//,
        //ui->showRolls->isChecked()
    };

    ui->graphicsView->setVectors(interpolation, show, pos, tar/*, roll*/);
    sendCurves();
}

void CameraCinematic::sendCurves()
{
    bool curve[2] = {
        (ui->showPositions->isChecked()) ? ui->curvePositions->isChecked() : false,
        (ui->showTargets->isChecked()) ? ui->curveTargets->isChecked() : false
    };

    bool acc[2] = {
        (ui->showPositions->isChecked()) ? ui->accelerationPosition->isChecked() : false,
        (ui->showTargets->isChecked()) ? ui->accelerationTarget->isChecked() : false
    };

    QVector<int> stampPos;
    QVector<int> stampTar;

    for (int row = 0; row < ui->pos_table->rowCount(); ++row)
        stampPos.push_back(ui->pos_table->item(row, 0)->text().toUInt());

    for (int row = 0; row < ui->tar_table->rowCount(); ++row)
        stampTar.push_back(ui->tar_table->item(row, 0)->text().toUInt());

    ui->graphicsView->setCurves(curve, interpolation,
                                stampPos, stampTar,
                                (float)ui->accelerationRatio->value(),
                                ui->accelerationPercent->value(), acc,
                                ui->curveOnTop->isChecked());

    updateViewTime(ui->viewTime->value());

    /* display info */

    QVector<float> lghPos = ui->graphicsView->getLenghtPositions();
    QVector<float> lghTar = ui->graphicsView->getLenghtTargets();

    if (lghPos.size() > 0)
    {
        float ttPos = 0.0f;

        for (int i = 0; i < lghPos.size(); ++i)
            ttPos += lghPos[i];

        float avgPos = (float)animationLength / ttPos;

        ui->totalLenghtPos->setText(QString("Total Pos Lenght : %1").arg(ttPos));
        ui->avgLenghtPos->setText(QString("Avg Pos : %1 ms/f").arg(avgPos));
    }

    if (lghTar.size() > 0)
    {
        float ttTar = 0.0f;

        for (int i = 0; i < lghTar.size(); ++i)
            ttTar += lghTar[i];

        float avgTar = (float)animationLength / ttTar;

        ui->totalLenghtTar->setText(QString("Total Tar Lenght : %1").arg(ttTar));
        ui->avgLenghtTar->setText(QString("Avg Tar : %1 ms/f").arg(avgTar));
    }
}

void CameraCinematic::updateDBC()
{
    if (!dbcCinematicCamera.dbcExist())
    {
        ui->output->setText("Error: Can't found the DBC [CinematicCamera.dbc] in the application folder");
    }
    else
    {
        dbcCinematicCamera.getHeader();
        dbcCinematicCamera.generateStringSort();
        dbcCinematicCamera.generateVectorByID();

        ui->output->setText("DBC: CinematicCamera.dbc is correctly loaded");
        QVector<std::uint32_t> listRecord = dbcCinematicCamera.getListID();

        ui->dbcCameraList->clear();
        for (int i = 0; i < listRecord.size(); ++i)
        {
            std::uint32_t id = listRecord[i];
            ui->dbcCameraList->addItem(QString("(%1) %2")
                               .arg(id)
                               .arg(dbcCinematicCamera.getStringByID(id)));
        }
    }
}

void CameraCinematic::updateRowList()
{
    ui->rowList->clear();
    for (int i = 0; i < selectedTable->rowCount(); ++i)
        ui->rowList->addItem(QString::number(i+1));

    if ( ui->rowList->count() > 0 )
        ui->rowList->setCurrentIndex(ui->rowList->count()-1);
}

void CameraCinematic::updateVectorList()
{
    QString vectorName[3] = {"Self","In","Out"};
    ui->vectorList->clear();
    if ( ui->tab->currentIndex() < 2 )
        for (int n = 0; n < 3 ; ++n)
            ui->vectorList->addItem(vectorName[n]);
    else
        ui->vectorList->addItem(vectorName[0]);
}

void CameraCinematic::updateViewTime(int value)
{
    if ( ui->viewTimeCheck->isChecked() == false )
    {
        ui->graphicsView->clearViewLine();
        return;
    }

    int rowPos = 0;
    int rowTar = 0;
    float fPos = 0.0f;
    float fTar = 0.0f;

    if ( ui->pos_table->rowCount() == 0 ||
         ui->tar_table->rowCount() == 0 )
        return;

    for ( int row = 0; row < ui->pos_table->rowCount(); ++row )
    {
        if ( ui->pos_table->item(row, 0)->text().toInt() <= value )
            rowPos = row;
    }

    for ( int row = 0; row < ui->tar_table->rowCount(); ++row )
    {
        if ( ui->tar_table->item(row, 0)->text().toInt() <= value )
            rowTar = row;
    }

    if ( rowPos < ui->pos_table->rowCount()-1 )
    {
        int rangePos = ui->pos_table->item(rowPos, 0)->text().toInt();
        int diff = value - rangePos;

        fPos = (float) diff / (float) (ui->pos_table->item(rowPos+1, 0)->text().toInt() - rangePos);
    }

    if ( rowTar < ui->tar_table->rowCount()-1 )
    {
        int rangeTar = ui->tar_table->item(rowTar, 0)->text().toInt();
        int diff = value - rangeTar;

        fTar = (float) diff / (float) (ui->tar_table->item(rowTar+1, 0)->text().toInt() - rangeTar);
    }


    ui->graphicsView->setViewLine(rowPos, rowTar, fPos, fTar, interpolation);
}

void CameraCinematic::createPointFromStoredPosition()
{
    QVector3D pos;
    QVector<QString> toUndo;
    toUndo.push_back("2");

    if (ui->sceneX->text().remove("X : ") != "null")
        pos.setX(ui->sceneX->text().remove("X : ").toFloat());

    if (ui->sceneY->text().remove("Y : ") != "null")
        pos.setY(ui->sceneY->text().remove("Y : ").toFloat());

    if (ui->sceneZ->text().remove("Z : ") != "null")
        pos.setZ(ui->sceneZ->text().remove("Z : ").toFloat());

    bool isNull[3] {
        (ui->sceneX->text().remove("X : ") == "null"),
        (ui->sceneY->text().remove("Y : ") == "null"),
        (ui->sceneZ->text().remove("Z : ") == "null")
    };

    int row = ui->rowList->currentIndex();
    int vec = ui->vectorList->currentIndex();

    ignoreStack = true;
    for (int n = 0; n < 3; ++n)
    {
        if (!isNull[n])
        {
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignmentFlag::AlignCenter);
            item->setText(QString::number(pos[n]));
            int column = (n + 1) + (vec * 3);

            /* Create undo stack */
            toUndo.push_back(QString::number(ui->tab->currentIndex()));
            toUndo.push_back(QString::number(row));
            toUndo.push_back(QString::number(column));
            toUndo.push_back(selectedTable->item(row, column)->text());
            /* End undo stack */

            selectedTable->setItem(row, column, item);
        }
    }
    ignoreStack = false;

    pushActionToArrayUndo(toUndo);
    sendVectors();
}

void CameraCinematic::createFileMenu()
{
    newFileAct = new QAction(tr("New File"), ui->menuFile);
    newFileAct->setIcon(QIcon(":/icons/icons/file-add.svg"));
    connect(newFileAct, &QAction::triggered, [=]() {
        if (callAutoSave(true))
        {
            ui->editName->setText("");
            ui->editAnimationLength->setText("");
            ui->pos_interpolation->setCurrentIndex(0);
            ui->tar_interpolation->setCurrentIndex(0);
            ui->roll_interpolation->setCurrentIndex(0);

            //updateModelInfo();
            ui->graphicsView->clear();

            ui->pos_table->setRowCount(0);
            ui->tar_table->setRowCount(0);
            ui->roll_table->setRowCount(0);

            ui->tab->setCurrentIndex(0);

            updateRowList();
            updateVectorList();
        }
    });

    openFileAct = new QAction(tr("Open File"), ui->menuFile);
    openFileAct->setIcon(QIcon(":/icons/icons/file.svg"));
    connect(openFileAct, &QAction::triggered, [this]() {
        getFileInformation();
        openModelFile();
    });

    refreshDBCAct = new QAction(tr("Refresh DBC"), ui->menuFile);
    refreshDBCAct->setIcon(QIcon(":/icons/icons/refresh.svg"));
    connect(refreshDBCAct, &QAction::triggered, [=]() {
        updateDBC();
        updateDBCInfo();
    });

    settingsAct = new QAction(tr("Settings"), this);
    settingsAct->setIcon(QIcon(":/icons/icons/settings.svg"));
    connect(settingsAct, &QAction::triggered, [this]() {
        settings = new Settings(this);
        settings->show();

        connect(settings, &QDialog::finished, [=]() {
            updateTableColor();
            sendVectors();
        });
    });

    closeAct = new QAction(tr("Close"), this);
    closeAct->setIcon(QIcon(":/icons/icons/error.svg"));
    connect(closeAct, &QAction::triggered, [this]() {
        if (callAutoSave(true))
            close();
    });

    ui->menuFile->addAction(newFileAct);
    ui->menuFile->addAction(openFileAct);
    ui->menuFile->addSeparator();
    ui->menuFile->addAction(refreshDBCAct);
    ui->menuFile->addAction(settingsAct);
    ui->menuFile->addAction(closeAct);
}

void CameraCinematic::createEditMenu()
{
    undoAct = new QAction(tr("Undo"), ui->menuEdit);
    undoAct->setIcon(QIcon(":/icons/icons/undo.svg"));
    undoAct->setEnabled(false);
    undoAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    connect(undoAct, &QAction::triggered, [this]() {
        applyUndo();
    });

    redoAct = new QAction(tr("Redo"), ui->menuEdit);
    redoAct->setIcon(QIcon(":/icons/icons/redo.svg"));
    redoAct->setEnabled(false);
    redoAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    connect(redoAct, &QAction::triggered, [this]() {
        applyRedo();
    });

    ui->menuEdit->addAction(undoAct);
    ui->menuEdit->addAction(redoAct);
}

void CameraCinematic::createToolBar()
{
    ui->toolBar->addAction(newFileAct);
    ui->toolBar->addAction(openFileAct);
    //ui->toolBar->addAction(generateAct):
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(undoAct);
    ui->toolBar->addAction(redoAct);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(closeAct);
}
