#ifndef DBCREADER_H
#define DBCREADER_H

#include <QDir>
#include <QSettings>

enum CinematicCameraStruct {
    ID,
    MODEL,
    SOUND_ID,
    ORIGIN_X,
    ORIGIN_Y,
    ORIGIN_Z,
    ORIGIN_FACING
};


class DBCReader
{
    //Q_OBJECT

public:
    explicit DBCReader(QObject*) {};
    virtual ~DBCReader() {};

    void getHeader() {
        if (!dbcExist())
            return;

        dbc = fopen(getPath().toLocal8Bit().data(), "rb");

        if (dbc)
        {
            fread(magic.data(), sizeof (std::uint32_t), 1, dbc);
            fread(&record_count, sizeof (std::uint32_t), 1, dbc);
            fread(&field_count, sizeof (std::uint32_t), 1, dbc);
            fread(&record_size, sizeof (std::uint32_t), 1, dbc);
            fread(&string_block_size, sizeof (std::uint32_t), 1, dbc);
            fclose(dbc);

            BLOCK_VALUE_SIZE = ( record_count * record_size );

            string_sort.clear();
            stringByID.clear();
            listID.clear();
            stringByPos.clear();
            vectorByID.clear();
        }
    }

    void generateStringSort()
    {
        dbc = fopen(getPath().toLocal8Bit().data(), "rb");

        if (dbc)
        {            
            for (std::uint32_t i = 0; i < record_count; ++i)
            {
                std::uint32_t id;
                std::uint32_t value;
                fseek(dbc, HEADER_SIZE + ( i * record_size ), SEEK_SET);
                fread(&id, sizeof (std::uint32_t), 1, dbc);
                fread(&value, sizeof (std::uint32_t), 1, dbc);
                string_sort << value;
                stringByPos[id] = value;
                listID.push_back(id);
            }

            std::sort(string_sort.begin(), string_sort.end());

            std::uint32_t count = 1;
            for (std::uint32_t i = 0; i < record_count; ++i)
            {
                fseek(dbc, HEADER_SIZE + BLOCK_VALUE_SIZE + count, SEEK_SET);
                std::uint32_t lenght;

                lenght = ( ( i < record_count - 1 ) ? string_sort[i+1] : string_block_size ) - string_sort[i];
                char buffer[lenght];

                fread(&buffer, sizeof (char), lenght, dbc);
                stringByID[string_sort[i]] = QString(buffer).remove("Cameras\\").remove(".mdx");

                count += lenght;
            }

            fclose(dbc);
        }
    }

    void generateVectorByID()
    {
        dbc = fopen(getPath().toLocal8Bit().data(), "rb");

        if (dbc)
        {
            for (std::uint32_t i = 0; i < record_count; ++i)
            {
                std::uint32_t id;
                float originX;
                float originY;
                float originZ;
                float originFacing;

                fseek(dbc, HEADER_SIZE + ( i * record_size ), SEEK_SET);
                fread(&id, sizeof (std::uint32_t), 1, dbc);
                fseek(dbc, 2 * sizeof (std::uint32_t), SEEK_CUR);

                fread(&originX, sizeof (float), 1, dbc);
                fread(&originY, sizeof (float), 1, dbc);
                fread(&originZ, sizeof (float), 1, dbc);
                fread(&originFacing, sizeof (float), 1, dbc);

                vectorByID[id].push_back(originX);
                vectorByID[id].push_back(originY);
                vectorByID[id].push_back(originZ);
                vectorByID[id].push_back(originFacing);
            }

            fclose(dbc);
        }
    }

    QVector<std::uint32_t> getListID() { return listID; };
    std::uint32_t getRecordCount() { return record_count; };
    QString getStringByID(int id) { return stringByID[stringByPos[id]]; };
    QVector<float> getVectorByID(int id) { return vectorByID[id]; };

    bool dbcExist() {
        if (QFile(getPath()).exists())
            return true;

        return false;
    }

    QString getPath()
    {
        QString path;
        QSettings setting("WOW-EDITOR", "CameraCinematic");
        setting.beginGroup("DBC-SETTINGS");
        path = setting.value("folder-path", QDir::currentPath()).toString();
        setting.endGroup();

        return path + "/" + dbc_name;
    };

private:
    FILE * dbc;
    QString dbc_name = "CinematicCamera.dbc";
    QList<std::uint32_t> string_sort;
    QMap<std::uint32_t, QString> stringByID;
    QVector<std::uint32_t> listID;
    QMap<std::uint32_t, std::uint32_t> stringByPos;
    QMap<std::uint32_t, QVector<float>> vectorByID;

    /* Header */
    std::string magic;
    std::uint32_t record_count;
    std::uint32_t field_count;
    std::uint32_t record_size;
    std::uint32_t string_block_size;

    std::uint32_t HEADER_SIZE = ( 5 * sizeof (std::uint32_t) );
    std::uint32_t BLOCK_VALUE_SIZE;
};

#endif // DBCREADER_H
