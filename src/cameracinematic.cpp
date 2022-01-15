#include "cameracinematic.h"
#include "ui_form.h"

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

    ui->pos_interpolation->setCurrentIndex(1);
    ui->tar_interpolation->setCurrentIndex(1);
    ui->roll_interpolation->setCurrentIndex(1);

    QIntValidator *validator_number = new QIntValidator(0,300000,this);
    ui->editAnimationLength->setValidator(validator_number);

    connect(ui->addVect, &QPushButton::clicked, [=]() {addRow();});
    connect(ui->removeVect, &QPushButton::clicked, [=]() {removeRow();});
    connect(ui->tab, &QTabWidget::currentChanged, [=](int c) {
        switch (c) {
        case 0: selectedTable = ui->pos_table; break;
        case 1: selectedTable = ui->tar_table; break;
        case 2: selectedTable = ui->roll_table; break;
        }

        updateRowList();
        updateVectorList();
    });

    connect(ui->applyOrientation, &QPushButton::clicked, [=]() {
        int row = ui->rowList->currentIndex();
        int vec = ui->vectorList->currentIndex();

        QTableWidgetItem* itemX = selectedTable->item(row, 1+(vec*3));
        QTableWidgetItem* itemY = selectedTable->item(row, 2+(vec*3));

        applyItemOrientation(itemX, itemY);
    });
    connect(ui->applyOrientation, &QPushButton::clicked, [=]() {
        int row = ui->rowList->currentIndex();
        int vec = ui->vectorList->currentIndex();

        QVector<QTableWidgetItem*> items;

        for (int n = 1; n < 4; ++n)
            items.push_back(selectedTable->item(row, n+(vec*3)));

        applyItemOffset(items);
    });

    connect(ui->generate, &QPushButton::clicked, [=]() {generateFile();});
    connect(ui->editName, &QLineEdit::textChanged, [=]() {updateModelInfo();});
    connect(ui->editAnimationLength, &QLineEdit::textChanged, [=]() {updateModelInfo();});

    connect(ui->pos_table, &QTableWidget::itemSelectionChanged, [=]() {sendVectors();});
    connect(ui->tar_table, &QTableWidget::itemSelectionChanged, [=]() {sendVectors();});
    connect(ui->roll_table, &QTableWidget::itemSelectionChanged, [=]() {sendVectors();});

    connect(ui->showPositions, &QCheckBox::stateChanged, [=]() {sendVectors();});
    connect(ui->showTargets, &QCheckBox::stateChanged, [=]() {sendVectors();});
    connect(ui->showRolls, &QCheckBox::stateChanged, [=]() {sendVectors();});

    connect(ui->displayAxe, &QComboBox::currentIndexChanged, [=](int index) {ui->graphicsView->changeDisplay(index); sendVectors();});

    connect(ui->pos_interpolation, &QComboBox::currentIndexChanged, [=](int index) {interpolation[0] = (quint16)index;});
    connect(ui->tar_interpolation, &QComboBox::currentIndexChanged, [=](int index) {interpolation[1] = (quint16)index;});
    connect(ui->roll_interpolation, &QComboBox::currentIndexChanged, [=](int index) {interpolation[2] = (quint16)index;});

    connect(&camSkin, SIGNAL(skinDone()), this, SLOT(skinFileDone()));
    connect(&camModel, SIGNAL(runDone()), this, SLOT(m2FileDone()));
    connect(&camModel, SIGNAL(readDone()), this, SLOT(m2FileRead()));
    connect(&camModel, SIGNAL(updateDone()), this, SLOT(m2FileUpdate()));

    connect(ui->dbcRefresh, &QPushButton::clicked, [=]() {updateDBC();});
    connect(ui->dbcCameraList, &QComboBox::currentIndexChanged, [=]() {updateDBCInfo(false);});

    connect(ui->applyWindowName, &QPushButton::clicked, [=]() {
        hProc.setWindowName(ui->windowName->text());
        hProc.run();
        QString Error = hProc.getError();

        if (!Error.isEmpty())
        {
            ui->output->append(Error);
            return;
        }

        if ( hProc.getSuccess() )
            ui->output->append("Success: Find the wow application");
    });
    connect(ui->clientPosition, &QPushButton::clicked, [=]() {
        hProc.run();
        QString Error = hProc.getError();

        if (!Error.isEmpty())
        {
            ui->output->append(Error);
            return;
        }

        const char* coord[3] = { "x: ", "y: ", "z: " };
        for (int n = 0; n < 3; n++)
            ui->output->append(coord[n] + QString::number(hProc.getCoord(n,
                                                                         facing, ui->applyOrientationClient->isChecked(),
                                                                         origin, ui->applyOffsetClient->isChecked())));

        int row = ui->rowList->currentIndex();
        int vec = ui->vectorList->currentIndex();

        for (int n = 1; n < 4; ++n)
        {
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignmentFlag::AlignCenter);
            item->setText(QString::number(hProc.getCoord(n-1,
                                                         facing, ui->applyOrientationClient->isChecked(),
                                                         origin, ui->applyOffsetClient->isChecked())));

            selectedTable->setItem(row, n + (vec * 3), item);
        }
    });
    connect(ui->selectModel, &QPushButton::clicked, [=]() {
        modelPath = QFileDialog::getOpenFileName(this,
                                                tr("Open CinematicCamera"),
                                                "/",
                                                tr("Model Files (*.m2 *.mdx)"));


        if ( !modelPath.isEmpty() )
        {
            QString skinPath = modelPath;
            camSkin.setPath(skinPath.replace(".m2", "00.skin"));
            camModel.setPath(modelPath);
            camModel.read();
            updateRowList();
            sendVectors();
        }
    });

    updateDBC();
    updateRowList();
    updateVectorList();
}

CameraCinematic::~CameraCinematic() { delete ui; }

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

void CameraCinematic::updateModelInfo()
{
    name = ui->editName->text();
    animationLength = ui->editAnimationLength->text().toUInt();

    ui->name->setText(name + ".m2 | " + name + "00.skin");
    updateDBCInfo();
}

void CameraCinematic::updateDBCInfo(bool check)
{
    QVector<std::uint32_t> dbcList = dbcCinematicCamera.getListID();

    if ( check )
        for (int i = 0; i < ui->dbcCameraList->count(); ++i)
            if ( ui->dbcCameraList->itemText(i).contains(name) )
                ui->dbcCameraList->setCurrentIndex(i);

    QVector<float> dbcValues = dbcCinematicCamera.getVectorByID(dbcList[ui->dbcCameraList->currentIndex()]);

    if (dbcValues.isEmpty())
        return;

    origin[0] = dbcValues[0];
    origin[1] = dbcValues[1];
    origin[2] = dbcValues[2];
    facing = dbcValues[3];

    ui->originX->setText(QString::number(dbcValues[0]));
    ui->originY->setText(QString::number(dbcValues[1]));
    ui->originZ->setText(QString::number(dbcValues[2]));
    ui->originFacing->setText(QString::number(dbcValues[3]));
}

void CameraCinematic::generateFile()
{
    bool error = false;

    if (name.isEmpty())
    {
        error = true;
        ui->output->append("Error: You must define a name");
    }

    if (animationLength == 0)
    {
        error = true;
        ui->output->append("Error: You must set an animation length");
    }

    if ( error )
        return;

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
}

void CameraCinematic::skinFileDone()
{ ui->output->append("Success: skin file is generated"); }

void CameraCinematic::m2FileDone()
{ ui->output->append("Success: m2 file is generated"); }

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

    ui->output->append("Success: m2 file is loaded");
}

void CameraCinematic::m2FileUpdate()
{ ui->output->append("Success: m2 file is updated"); }

void CameraCinematic::sendVectors()
{    
    QVector<QVector<QVector<float>>> pos;
    QVector<QVector<QVector<float>>> tar;
    QVector<QVector<QVector<float>>> roll;

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

    for (int row = 0; row < ui->roll_table->rowCount(); ++row)
    {
        QVector<QVector<float>> f;
        QVector<float> g;
        roll.push_back(f);
        roll[row].push_back(g);

        for (int i = 0; i < 3; ++i)
        {
            tar[row][0].push_back( ui->roll_table->item(row, 1 + i)->text().toFloat() );
        }
    }

    bool show[3] = {
        ui->showPositions->isChecked(),
        ui->showTargets->isChecked(),
        ui->showRolls->isChecked()
    };

    ui->graphicsView->setVectors(interpolation, show, pos, tar, roll);
}

void CameraCinematic::updateDBC()
{
    if ( !dbcCinematicCamera.dbcExist() )
    {
        ui->output->append("Error: Can't found the DBC [CinematicCamera.dbc] in the application folder");
    }
    else
    {
        dbcCinematicCamera.getHeader();
        dbcCinematicCamera.generateStringSort();
        dbcCinematicCamera.generateVectorByID();

        ui->output->append("DBC: CinematicCamera.dbc is correctly loaded");
        ui->dbcRefresh->setEnabled(false);

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
    ui->vectorList->clear();
    if ( ui->tab->currentIndex() < 2 )
        for (int n = 0; n < 3 ; ++n)
            ui->vectorList->addItem(QString("Vec%1").arg(n+1));
    else
        ui->vectorList->addItem("Vec1");
}

void CameraCinematic::applyItemOrientation(QTableWidgetItem * itemX, QTableWidgetItem* itemY)
{
    float valX = itemX->text().toFloat();
    float valY = itemY->text().toFloat();

    itemX->setText(QString::number( (valX*cos(facing)-valY*sin(facing)) ));
    itemY->setText(QString::number( (valX*sin(facing)+valY*cos(facing)) ));
}

void CameraCinematic::applyItemOffset(QVector<QTableWidgetItem*> items)
{
    for (int n = 0; n < 3; ++n)
    {
        QTableWidgetItem* item = items[n];
        float val = item->text().toFloat();

        item->setText(QString::number(val-origin[n]));
    }
}
