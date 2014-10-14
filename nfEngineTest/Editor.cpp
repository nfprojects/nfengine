#include "stdafx.hpp"
#include "Editor.hpp"

using namespace NFE;
using namespace NFE::Render;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

Entity* g_pSelectedEntity = nullptr;

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
    if (g_pSelectedEntity == nullptr || value == nullptr)
        return;

    Vector pos = g_pSelectedEntity->GetPosition();
    Vector vel = g_pSelectedEntity->GetVelocity();
    Vector angVel = g_pSelectedEntity->GetAngularVelocity();

    switch ((int)clientData)
    {
        case EditionType::Name:
        {
            const char* str = *(const char**)value;
            g_pSelectedEntity->SetName(str);
            break;
        }

        case EditionType::PosX:
        {
            Vector pos = g_pSelectedEntity->GetPosition();
            pos[0] = *(const float*)value;
            g_pSelectedEntity->SetPosition(pos);
            break;
        }

        case EditionType::PosY:
        {
            Vector pos = g_pSelectedEntity->GetPosition();
            pos[1] = *(const float*)value;
            g_pSelectedEntity->SetPosition(pos);
            break;
        }

        case EditionType::PosZ:
        {
            Vector pos = g_pSelectedEntity->GetPosition();
            pos[2] = *(const float*)value;
            g_pSelectedEntity->SetPosition(pos);
            break;
        }

        case EditionType::Orientation:
        {
            Matrix matrix = g_pSelectedEntity->GetMatrix();

            Quaternion quat = Quaternion((const float*)value);
            Matrix rotMatrix = MatrixFromQuaternion(quat);
            matrix.r[0] = rotMatrix.r[0];
            matrix.r[1] = rotMatrix.r[1];
            matrix.r[2] = rotMatrix.r[2];
            g_pSelectedEntity->SetMatrix(matrix);
            break;
        }

        case EditionType::VelocityX:
        {
            vel[0] = *(const float*)value;
            g_pSelectedEntity->SetVelocity(vel);
            break;
        }

        case EditionType::VelocityY:
        {
            vel[1] = *(const float*)value;
            g_pSelectedEntity->SetVelocity(vel);
            break;
        }

        case EditionType::VelocityZ:
        {
            vel[2] = *(const float*)value;
            g_pSelectedEntity->SetVelocity(vel);
            break;
        }

        case EditionType::AngularVelocityX:
        {
            angVel[0] = *(const float*)value;
            g_pSelectedEntity->SetAngularVelocity(angVel);
            break;
        }

        case EditionType::AngularVelocityY:
        {
            angVel[1] = *(const float*)value;
            g_pSelectedEntity->SetAngularVelocity(angVel);
            break;
        }

        case EditionType::AngularVelocityZ:
        {
            angVel[2] = *(const float*)value;
            g_pSelectedEntity->SetAngularVelocity(angVel);
            break;
        }
    }

}

void TW_CALL Demo_EntityGetCallback(void* value, void* clientData)
{
    if (g_pSelectedEntity == nullptr || value == nullptr)
        return;

    Vector pos = g_pSelectedEntity->GetPosition();
    Vector vel = g_pSelectedEntity->GetVelocity();
    Vector angVel = g_pSelectedEntity->GetAngularVelocity();

    switch ((size_t)clientData)
    {
        case EditionType::Name:
        {
            TwCopyCDStringToLibrary((char**)value, g_pSelectedEntity->GetName());
            break;
        }

        case EditionType::SegmentName:
        {
            Segment* pSegment = g_pSelectedEntity->GetSceneSegment();

            if (pSegment)
                TwCopyCDStringToLibrary((char**)value, pSegment->GetName());
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
            Matrix matrix = g_pSelectedEntity->GetMatrix();
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
    if (g_pSelectedEntity)
    {
        SceneManager* pScene = g_pSelectedEntity->GetScene();
        pScene->EnqueueDeleteEntity(g_pSelectedEntity);
    }
}

void Demo_InitEditorBar()
{
    TwBar* pBar = TwNewBar("Editor");
    {
        TwDefine("Editor iconified=true color='50 50 50' alpha=200 refresh=0.02");
        TwAddVarCB(pBar, "Name", TW_TYPE_CDSTRING, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::Name, 0);
        TwAddVarCB(pBar, "Segment", TW_TYPE_CDSTRING, 0, Demo_EntityGetCallback,
                   (void*)EditionType::SegmentName, 0);

        TwAddSeparator(pBar, "Position & Orientation", 0);
        TwAddVarCB(pBar, "Position X", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::PosX, "precision=5");
        TwAddVarCB(pBar, "Position Y", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::PosY, "precision=5");
        TwAddVarCB(pBar, "Position Z", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::PosZ, "precision=5");
        TwAddVarCB(pBar, "Orientation", TW_TYPE_QUAT4F, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::Orientation, "precision=4");

        TwAddSeparator(pBar, "Velocity", 0);
        TwAddVarCB(pBar, "Vel X", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::VelocityX, "precision=5");
        TwAddVarCB(pBar, "Vel Y", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::VelocityY, "precision=5");
        TwAddVarCB(pBar, "Vel Z", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::VelocityZ, "precision=5");

        TwAddSeparator(pBar, "Angular velocity", 0);
        TwAddVarCB(pBar, "Ang Vel X", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::AngularVelocityX, "precision=5");
        TwAddVarCB(pBar, "Ang Vel Y", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::AngularVelocityY, "precision=5");
        TwAddVarCB(pBar, "Ang Vel Z", TW_TYPE_FLOAT, Demo_EntitySetCallback, Demo_EntityGetCallback,
                   (void*)EditionType::AngularVelocityZ, "precision=5");

        TwAddSeparator(pBar, "Actions", 0);
        TwAddButton(pBar, "Delete", Demo_EntityDeleteCallback, 0, 0);
    }
}