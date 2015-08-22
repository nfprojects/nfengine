#include "PCH.hpp"
#include "Editor.hpp"

#include <cstdint>

using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

Entity* gSelectedEntity = nullptr;

// client data passed to AntTweakBar callbacks
struct EditionType
{
    enum
    {
        Name = 0,
        SegmentName,
        PosX, PosY, PosZ,
        Orientation,
        VelocityX, VelocityY, VelocityZ,
        AngularVelocityX, AngularVelocityY, AngularVelocityZ,
    };
};

void TW_CALL Demo_EntitySetCallback(const void* value, void* clientData)
{
    //myVariable = *(const MyVariableType *)value;
    if (gSelectedEntity == nullptr || value == nullptr)
        return;

    Vector pos = gSelectedEntity->GetPosition();
    Vector vel = gSelectedEntity->GetVelocity();
    Vector angVel = gSelectedEntity->GetAngularVelocity();
    size_t data = reinterpret_cast<std::size_t>(clientData);

    switch (data)
    {
        case EditionType::Name:
        {
            const char* str = *(const char**)value;
            gSelectedEntity->SetName(str);
            break;
        }

        case EditionType::PosX:
        {
            Vector pos = gSelectedEntity->GetPosition();
            pos[0] = *(const float*)value;
            gSelectedEntity->SetPosition(pos);
            break;
        }

        case EditionType::PosY:
        {
            Vector pos = gSelectedEntity->GetPosition();
            pos[1] = *(const float*)value;
            gSelectedEntity->SetPosition(pos);
            break;
        }

        case EditionType::PosZ:
        {
            Vector pos = gSelectedEntity->GetPosition();
            pos[2] = *(const float*)value;
            gSelectedEntity->SetPosition(pos);
            break;
        }

        case EditionType::Orientation:
        {
            Matrix matrix = gSelectedEntity->GetMatrix();

            Quaternion quat = Quaternion((const float*)value);
            Matrix rotMatrix = MatrixFromQuaternion(quat);
            matrix.r[0] = rotMatrix.r[0];
            matrix.r[1] = rotMatrix.r[1];
            matrix.r[2] = rotMatrix.r[2];
            gSelectedEntity->SetMatrix(matrix);
            break;
        }

        case EditionType::VelocityX:
        {
            vel[0] = *(const float*)value;
            gSelectedEntity->SetVelocity(vel);
            break;
        }

        case EditionType::VelocityY:
        {
            vel[1] = *(const float*)value;
            gSelectedEntity->SetVelocity(vel);
            break;
        }

        case EditionType::VelocityZ:
        {
            vel[2] = *(const float*)value;
            gSelectedEntity->SetVelocity(vel);
            break;
        }

        case EditionType::AngularVelocityX:
        {
            angVel[0] = *(const float*)value;
            gSelectedEntity->SetAngularVelocity(angVel);
            break;
        }

        case EditionType::AngularVelocityY:
        {
            angVel[1] = *(const float*)value;
            gSelectedEntity->SetAngularVelocity(angVel);
            break;
        }

        case EditionType::AngularVelocityZ:
        {
            angVel[2] = *(const float*)value;
            gSelectedEntity->SetAngularVelocity(angVel);
            break;
        }
    }

}

void TW_CALL Demo_EntityGetCallback(void* value, void* clientData)
{
    if (gSelectedEntity == nullptr || value == nullptr)
        return;

    Vector pos = gSelectedEntity->GetPosition();
    Vector vel = gSelectedEntity->GetVelocity();
    Vector angVel = gSelectedEntity->GetAngularVelocity();

    switch ((size_t)clientData)
    {
        case EditionType::Name:
        {
            TwCopyCDStringToLibrary((char**)value, gSelectedEntity->GetName());
            break;
        }

        case EditionType::SegmentName:
        {
            Segment* segment = gSelectedEntity->GetSceneSegment();

            if (segment)
                TwCopyCDStringToLibrary((char**)value, segment->GetName());
            else
                TwCopyCDStringToLibrary((char**)value, "[NULL]");

            break;
        }

        case EditionType::PosX:
        {
            *(float*)value = pos[0];
            break;
        }

        case EditionType::PosY:
        {
            *(float*)value = pos[1];
            break;
        }

        case EditionType::PosZ:
        {
            *(float*)value = pos[2];
            break;
        }

        case EditionType::Orientation:
        {
            Matrix matrix = gSelectedEntity->GetMatrix();
            matrix.r[3] = Vector(0.0f, 0.0f, 0.0f, 1.0f); // set position to 0
            Quaternion quat = QuaternionFromMatrix(matrix);
            VectorStore(quat, (Float4*)value);
            break;
        }

        case EditionType::VelocityX:
        {
            *(float*)value = vel[0];
            break;
        }

        case EditionType::VelocityY:
        {
            *(float*)value = vel[1];
            break;
        }

        case EditionType::VelocityZ:
        {
            *(float*)value = vel[2];
            break;
        }

        case EditionType::AngularVelocityX:
        {
            *(float*)value = angVel[0];
            break;
        }

        case EditionType::AngularVelocityY:
        {
            *(float*)value = angVel[1];
            break;
        }

        case EditionType::AngularVelocityZ:
        {
            *(float*)value = angVel[2];
            break;
        }
    }
}


void TW_CALL Demo_EntityDeleteCallback(void* clientData)
{
    if (gSelectedEntity)
    {
        SceneManager* scene = gSelectedEntity->GetScene();
        scene->EnqueueDeleteEntity(gSelectedEntity);
    }
}

void Demo_InitEditorBar()
{
    TwBar* bar = TwNewBar("Editor");
    {
        TwDefine("Editor iconified=true color='50 50 50' alpha=200 refresh=0.02");
        TwAddVarCB(bar, "Name", TW_TYPE_CDSTRING, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::Name, 0);
        TwAddVarCB(bar, "Segment", TW_TYPE_CDSTRING, 0, Demo_EntityGetCallback,
                   (void*)EditionType::SegmentName, 0);

        TwAddSeparator(bar, "Position & Orientation", 0);
        TwAddVarCB(bar, "Position X", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::PosX, "precision=5");
        TwAddVarCB(bar, "Position Y", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::PosY, "precision=5");
        TwAddVarCB(bar, "Position Z", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::PosZ, "precision=5");
        TwAddVarCB(bar, "Orientation", TW_TYPE_QUAT4F, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::Orientation, "precision=4");

        TwAddSeparator(bar, "Velocity", 0);
        TwAddVarCB(bar, "Vel X", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::VelocityX, "precision=5");
        TwAddVarCB(bar, "Vel Y", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::VelocityY, "precision=5");
        TwAddVarCB(bar, "Vel Z", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::VelocityZ, "precision=5");

        TwAddSeparator(bar, "Angular velocity", 0);
        TwAddVarCB(bar, "Ang Vel X", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::AngularVelocityX, "precision=5");
        TwAddVarCB(bar, "Ang Vel Y", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::AngularVelocityY, "precision=5");
        TwAddVarCB(bar, "Ang Vel Z", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::AngularVelocityZ, "precision=5");

        TwAddSeparator(bar, "Actions", 0);
        TwAddButton(bar, "Delete", Demo_EntityDeleteCallback, 0, 0);
    }
}