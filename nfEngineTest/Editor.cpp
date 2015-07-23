#include "PCH.hpp"
#include "Editor.hpp"
#include "Test.hpp"

using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

EntityID gSelectedEntity = -1;

// client data passed to AntTweakBar callbacks
enum class EditionType : int
{
    PosX, PosY, PosZ,
    Orientation,
    VelocityX, VelocityY, VelocityZ,
    AngularVelocityX, AngularVelocityY, AngularVelocityZ,
};

void TW_CALL Demo_EntitySetCallback(const void* value, void* clientData)
{
    if (value == nullptr)
        return;

    EditionType editionType = static_cast<EditionType>(reinterpret_cast<int>(clientData));
    TransformComponent* transform = gEntityManager->GetComponent<TransformComponent>(gSelectedEntity);
    BodyComponent* body = gEntityManager->GetComponent<BodyComponent>(gSelectedEntity);

    if (transform != nullptr)
    {
        switch ((EditionType)(int)clientData)
        {
            case EditionType::PosX:
            {
                Vector pos = transform->GetPosition();
                pos[0] = *(const float*)value;
                transform->SetPosition(pos);
                break;
            }
            case EditionType::PosY:
            {
                Vector pos = transform->GetPosition();
                pos[1] = *(const float*)value;
                transform->SetPosition(pos);
                break;
            }
            case EditionType::PosZ:
            {
                Vector pos = transform->GetPosition();
                pos[2] = *(const float*)value;
                transform->SetPosition(pos);
                break;
            }

            case EditionType::Orientation:
            {
                Matrix matrix = transform->GetMatrix();

                Quaternion quat = Quaternion((const float*)value);
                Matrix rotMatrix = MatrixFromQuaternion(quat);
                matrix.r[0] = rotMatrix.r[0];
                matrix.r[1] = rotMatrix.r[1];
                matrix.r[2] = rotMatrix.r[2];
                transform->SetMatrix(matrix);
                break;
            }
        }
    }

    if (body != nullptr)
    {
        switch ((EditionType)(int)clientData)
        {
            case EditionType::VelocityX:
            {
                Vector vel = body->GetVelocity();
                vel[0] = *(const float*)value;
                body->SetVelocity(vel);
                break;
            }
            case EditionType::VelocityY:
            {
                Vector vel = body->GetVelocity();
                vel[1] = *(const float*)value;
                body->SetVelocity(vel);
                break;
            }
            case EditionType::VelocityZ:
            {
                Vector vel = body->GetVelocity();
                vel[2] = *(const float*)value;
                body->SetVelocity(vel);
                break;
            }

            case EditionType::AngularVelocityX:
            {
                Vector angVel = body->GetAngularVelocity();
                angVel[0] = *(const float*)value;
                body->SetAngularVelocity(angVel);
                break;
            }
            case EditionType::AngularVelocityY:
            {
                Vector angVel = body->GetAngularVelocity();
                angVel[1] = *(const float*)value;
                body->SetAngularVelocity(angVel);
                break;
            }
            case EditionType::AngularVelocityZ:
            {
                Vector angVel = body->GetAngularVelocity();
                angVel[2] = *(const float*)value;
                body->SetAngularVelocity(angVel);
                break;
            }
        }
    }
}

void TW_CALL Demo_EntityGetCallback(void* value, void* clientData)
{
    if (value == nullptr)
        return;

    EditionType editionType = static_cast<EditionType>(reinterpret_cast<int>(clientData));
    TransformComponent* transform = gEntityManager->GetComponent<TransformComponent>(gSelectedEntity);
    BodyComponent* body = gEntityManager->GetComponent<BodyComponent>(gSelectedEntity);

    Vector pos, vel, angVel;
    
    if (transform != nullptr)
        pos = transform->GetPosition();

    if (body != nullptr)
    {
        vel = body->GetVelocity();
        angVel = body->GetAngularVelocity();
    }

    switch (editionType)
    {
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
            if (transform != nullptr)
            {
                Matrix matrix = transform->GetMatrix();
                matrix.r[3] = Vector(0.0f, 0.0f, 0.0f, 1.0f);
                Quaternion quat = QuaternionFromMatrix(matrix);
                VectorStore(quat, (Float4*)value);
            }
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
    if (gSelectedEntity != -1)
    {
        gEntityManager->RemoveEntity(gSelectedEntity);
        gSelectedEntity = -1;
    }
}

void Demo_InitEditorBar()
{
    TwBar* bar = TwNewBar("Editor");
    {
        TwDefine("Editor iconified=true color='50 50 50' alpha=200 refresh=0.02");

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