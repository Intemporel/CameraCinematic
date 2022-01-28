#include "m2.h"

/* EIntemporel
 *
 * Big Thanks to Schlumpf for the run() procedure and Stoneharry for the help
 * https://github.com/stoneharry/Misc-WoW-Stuff/blob/master/Camera%20Generator/camera.cpp
 *
 */

void M2::run() // used for save / generate
{
    if (name.isEmpty())
        return;

    if (isRead)
    {
        name = path ;
    }
    else
    {
        QSettings setting("WOW-EDITOR", "CameraCinematic");
        QString path;

        setting.beginGroup("WORK-SETTINGS");
        path = setting.value("folder-path", QDir::currentPath()).toString();
        setting.endGroup();

        name = path + "/" + name + ".m2";
    }

    FILE * model = fopen(name.toLocal8Bit().data(), "wb+");

    writeHeader(model);
    writeAnimations(model);
    writeBones(model);
    writeCameras(model);

    fclose(model);

    emit runDone();
}

void M2::writeHeader(FILE *model)
{
    write(model,0x000,_CHR('0','2','D','M'));
    write(model,0x004,0x00000108);
    write(model,0x008,0x00000005);
    write(model,0x00c,0x00000130);
    write(model,0x01c,0x00000001);
    write(model,0x020,0x00000140);
    write(model,0x02c,0x00000001);
    write(model,0x030,0x00000180);
    write(model,0x034,0x00000001);
    write(model,0x038,0x000001e0);
    write(model,0x044,0x00000001);
    write(model,0x0a0,0x7F7FFFFF);
    write(model,0x0a4,0x7F7FFFFF);
    write(model,0x0a8,0x7F7FFFFF);
    write(model,0x0ac,0xFF7FFFFF);
    write(model,0x0b0,0xFF7FFFFF);
    write(model,0x0b4,0xFF7FFFFF);
    write(model,0x110,0x00000001);
    write(model,0x114,0x00000200);
    write(model,0x118,0x00000001);
    write(model,0x11c,0x000001f0);
    write(model,0x130,_CHR('_','M','A','C'));
}

void M2::writeAnimations(FILE * model)
{
    write(model,0x144,animationLength);
    write(model,0x14c,0x00000020);
    write(model,0x150,0x00007fff);
    write(model,0x160,0x7F7FFFFF);
    write(model,0x164,0x7F7FFFFF);
    write(model,0x168,0x7F7FFFFF);
    write(model,0x16c,0xFF7FFFFF);
    write(model,0x170,0xFF7FFFFF);
    write(model,0x174,0xFF7FFFFF);
    write(model,0x17c,0x0000FFFF);
}

void M2::writeBones(FILE * model)
{
    write(model,0x180,0xFFFFFFFF);
    write(model,0x188,0x0000FFFF);
    write(model,0x190,0xFFFF0000);
    write(model,0x1a4,0xFFFF0000);
    write(model,0x1b8,0xFFFF0000);
    write(model,0x1e0,0x0000FFFF);
}

void M2::writeCameras(FILE * model)
{
    float fov = 0.785398f;
    float farclip = 27.7778f;
    float nearclip = 0.222222f;

    VecF<3> position {0.0f, 0.0f, 1.0f};
    VecF<3> target {0.0f, 0.0f, 2.0f};

    write(model,0x1f0,0x0000FFFF);

    write(model,0x200,0xFFFFFFFF);
    write(model,0x204,fov);
    write(model,0x208,farclip);
    write(model,0x20c,nearclip);
    write(model,0x210,interpolation[0]);
    write(model,0x212,(std::uint16_t)0xFFFF);
    write(model,0x214,0x00000001);
    write(model,0x218,0x00000270);
    write(model,0x21c,0x00000001);
    write(model,0x220,0x00000278);
    write(model,0x224,position);
    write(model,0x230,interpolation[1]);
    write(model,0x232,(std::uint16_t)0xFFFF);
    write(model,0x234,0x00000001);
    write(model,0x238,0x00000280);
    write(model,0x23c,0x00000001);
    write(model,0x240,0x00000288);
    write(model,0x244,target);
    write(model,0x250,interpolation[2]);
    write(model,0x252,(std::uint16_t)0xFFFF);
    write(model,0x254,0x00000001);
    write(model,0x258,0x00000290);
    write(model,0x25c,0x00000001);
    write(model,0x260,0x00000298);

    const auto pos_timestamps = 0x2a0;
    const auto pos_data = pos_timestamps + positions.size() * sizeof(uint32_t);

    const auto tar_timestamps = pos_data + positions.size() * sizeof(VecF<9>);
    const auto tar_data = tar_timestamps + targets.size() * sizeof(uint32_t);

    const auto roll_timestamps = tar_data + targets.size() * sizeof(VecF<9>);
    const auto roll_data = roll_timestamps + rolls.size() * sizeof(uint32_t);

    write(model,0x270,positions.size());
    write(model,0x274,pos_timestamps);
    write(model,0x278,positions.size());
    write(model,0x27c,pos_data);
    write(model,0x280,targets.size());
    write(model,0x284,tar_timestamps);
    write(model,0x288,targets.size());
    write(model,0x28c,tar_data);
    write(model,0x290,rolls.size());
    write(model,0x294,roll_timestamps);
    write(model,0x298,rolls.size());
    write(model,0x29c,roll_data);

    for(std::size_t i=0;i<positions.size();++i)
    {
        write(model,pos_timestamps+sizeof(uint32_t)*i,positions[i].stamp);
        write(model,pos_data+sizeof(VecF<9>)*i,positions[i].data);
    }

    for(std::size_t i=0;i<targets.size();++i)
    {
        write(model,tar_timestamps+sizeof(uint32_t)*i,targets[i].stamp);
        write(model,tar_data+sizeof(VecF<9>)*i,targets[i].data);
    }

    for(std::size_t i=0;i<rolls.size();++i)
    {
        write(model,roll_timestamps+sizeof(uint32_t)*i,rolls[i].stamp);
        write(model,roll_data+sizeof(VecF<3>)*i,rolls[i].data);
    }
}

void M2::read()
{
    read_name = QFileInfo(QFile(path.toLocal8Bit().data()).fileName()).fileName().remove(".m2");

    FILE * file = fopen(path.toLocal8Bit().data(), "rb");

    if ( file )
    {
        std::uint32_t ofsAnimations;
        fseek(file, 0x20, SEEK_SET);
        fread(&ofsAnimations, sizeof (std::uint32_t), 1, file);

        /* Animation Header */
        fseek(file, ofsAnimations, SEEK_SET);
        fseek(file, sizeof (std::uint32_t), SEEK_CUR);
        fread(&read_animationLength, sizeof (std::uint32_t), 1, file);

        std::uint32_t ofsCameras;
        fseek(file, 0x114, SEEK_SET);
        fread(&ofsCameras, sizeof (std::uint32_t), 1, file);

        /* Camera Header */
        fseek(file, ofsCameras, SEEK_SET);
        fseek(file, (4 * sizeof (std::uint32_t)), SEEK_CUR);
        for (int i = 0; i < 3; ++i)
        {
            // get interpolation
            fread(&read_interpolation[i], sizeof (std::uint16_t), 1, file);
            // get offset timestamp structure
            fseek(file, (sizeof (std::uint16_t) + sizeof (std::uint32_t)), SEEK_CUR);
            fread(&read_ofsTimestamp_struct[i], sizeof (std::uint32_t), 1, file);
            // get offset values structure
            fseek(file, sizeof (std::uint32_t), SEEK_CUR);
            fread(&read_ofsValues_struct[i], sizeof (std::uint32_t), 1, file);

            fseek(file, (3 * sizeof (std::uint32_t)), SEEK_CUR);
        }

        /* Get number of each and offset */
        for (int i = 0; i < 3; ++i)
        {
            // get number values & offset where timestamps is stored
            fseek(file, read_ofsTimestamp_struct[i], SEEK_SET);
            fread(&read_nVec[i], sizeof (std::uint32_t), 1, file);
            fread(&read_ofsTimestamp[i], sizeof (std::uint32_t), 1, file);
            // get offset where values is stored
            fseek(file, (read_ofsValues_struct[i] + sizeof (std::uint32_t)), SEEK_SET);
            fread(&read_ofsValues[i], sizeof (std::uint32_t), 1, file);
        }

        positions.clear();
        targets.clear();
        rolls.clear();

        /* Populate std::vector */
        for (int i = 0; i < 3; ++i)
        {
            for (std::uint32_t y = 0; y < read_nVec[i]; ++y)
            {
                if ( i < 2 )
                {
                    timestampedValue<VecF<9>> current;
                    fseek(file, (read_ofsTimestamp[i] + (y * sizeof (std::uint32_t))), SEEK_SET);
                    fread(&current.stamp, sizeof (std::uint32_t), 1, file);

                    VecF<9> t;
                    fseek(file, (read_ofsValues[i] + (y * sizeof (t))), SEEK_SET);
                    for (int n = 0; n < 9; ++n)
                        fread(&t[n], sizeof (float), 1, file);

                    current.data = t;

                    if ( i == 0 ) positions.push_back(current);
                    else targets.push_back(current);
                }
                else
                {
                    timestampedValue<VecF<3>> current;
                    fseek(file, (read_ofsTimestamp[i] + (y * sizeof (std::uint32_t))), SEEK_SET);
                    fread(&current.stamp, sizeof (std::uint32_t), 1, file);

                    VecF<3> t;
                    fseek(file, (read_ofsValues[i] + (y * sizeof (t))), SEEK_SET);
                    for (int n = 0; n < 3; n++)
                        fread(&t[n], sizeof (float), 1, file);

                    current.data = t;
                    rolls.push_back(current);
                }
            }
        }

        emit readDone();
        fclose(file);
        isRead = true;
    }
}

/*
void M2::update()
{
    if ( !isRead )
        return;

    FILE * file = fopen(path.toLocal8Bit().data(), "rb+");

    if ( file )
    {
        std::uint32_t ofsAnimations;
        fseek(file, 0x20, SEEK_SET);
        fread(&ofsAnimations, sizeof (std::uint32_t), 1, file);

        fseek(file, ofsAnimations, SEEK_SET);
        fseek(file, sizeof (std::uint32_t), SEEK_CUR);
        fwrite(&animationLength, sizeof (std::uint32_t), 1, file);

        std::uint32_t ofsCameras;
        fseek(file, 0x114, SEEK_SET);
        fread(&ofsCameras, sizeof (std::uint32_t), 1, file);

        fseek(file, ofsCameras, SEEK_SET);
        fseek(file, (4 * sizeof (std::uint32_t)), SEEK_CUR);

        for (int i = 0; i < 3; ++i)
        {
            fwrite(&interpolation[i], sizeof (std::uint16_t), 1, file);
            fseek(file, (7 * sizeof (std::uint32_t) + sizeof (std::uint16_t)), SEEK_CUR);
        }

        read_nVec[0] = positions.size();
        read_nVec[1] = targets.size();
        read_nVec[2] = rolls.size();

        const auto pos_timestamps = read_ofsTimestamp[0];
        const auto pos_data = pos_timestamps + positions.size() * sizeof(uint32_t);

        const auto tar_timestamps = pos_data + positions.size() * sizeof(VecF<9>);
        const auto tar_data = tar_timestamps + targets.size() * sizeof(uint32_t);

        const auto roll_timestamps = tar_data + targets.size() * sizeof(VecF<9>);
        const auto roll_data = roll_timestamps + rolls.size() * sizeof(uint32_t);

        fseek(file, read_ofsTimestamp_struct[0], SEEK_SET);
        fwrite(&read_nVec[0], sizeof (std::uint32_t), 1, file);
        fwrite(&pos_timestamps, sizeof (std::uint32_t), 1, file);
        fwrite(&read_nVec[0], sizeof (std::uint32_t), 1, file);
        fwrite(&pos_data, sizeof (std::uint32_t), 1, file);
        fwrite(&read_nVec[1], sizeof (std::uint32_t), 1, file);
        fwrite(&tar_timestamps, sizeof (std::uint32_t), 1, file);
        fwrite(&read_nVec[1], sizeof (std::uint32_t), 1, file);
        fwrite(&tar_data, sizeof (std::uint32_t), 1, file);
        fwrite(&read_nVec[2], sizeof (std::uint32_t), 1, file);
        fwrite(&roll_timestamps, sizeof (std::uint32_t), 1, file);
        fwrite(&read_nVec[2], sizeof (std::uint32_t), 1, file);
        fwrite(&roll_data, sizeof (std::uint32_t), 1, file);

        for(std::size_t i=0;i<positions.size();++i)
        {
            fseek(file, pos_timestamps + sizeof (uint32_t) * i, SEEK_SET);
            fwrite(&positions[i].stamp, sizeof (std::uint32_t), 1, file);
            fseek(file, pos_data + sizeof (VecF<9>) * i, SEEK_SET);
            fwrite(&positions[i].data, sizeof (VecF<9>), 1, file);
        }

        for(std::size_t i=0;i<targets.size();++i)
        {
            fseek(file, tar_timestamps + sizeof (uint32_t) * i, SEEK_SET);
            fwrite(&targets[i].stamp, sizeof (std::uint32_t), 1, file);
            fseek(file, tar_data + sizeof (VecF<9>) * i, SEEK_SET);
            fwrite(&targets[i].data, sizeof (VecF<9>), 1, file);
        }

        for(std::size_t i=0;i<rolls.size();++i)
        {
            fseek(file, roll_timestamps + sizeof (uint32_t) * i, SEEK_SET);
            fwrite(&rolls[i].stamp, sizeof (std::uint32_t), 1, file);
            fseek(file, roll_data + sizeof (VecF<3>) * i, SEEK_SET);
            fwrite(&rolls[i].data, sizeof (VecF<3>), 1, file);
        }
    }

    emit updateDone();
}
*/
