#include <gtest/gtest.h>

#include <vectormath/cpp/vectormath_aos.h>

#include <dlib/hash.h>

#include <resource/resource.h>

#include "../gameobject.h"

#include "gameobject/test/input/test_gameobject_input_ddf.h"

using namespace Vectormath::Aos;

class InputTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        dmGameObject::Initialize();

        m_InputCounter = 0;

        m_UpdateContext.m_DT = 1.0f / 60.0f;

        dmResource::NewFactoryParams params;
        params.m_MaxResources = 16;
        params.m_Flags = RESOURCE_FACTORY_FLAGS_EMPTY;
        m_Factory = dmResource::NewFactory(&params, "build/default/src/gameobject/test/input");
        m_Register = dmGameObject::NewRegister(0, 0);
        dmGameObject::RegisterResourceTypes(m_Factory, m_Register);
        dmGameObject::RegisterComponentTypes(m_Factory, m_Register);
        m_Collection = dmGameObject::NewCollection(m_Factory, m_Register, 1024);

        dmResource::FactoryResult e = dmResource::RegisterType(m_Factory, "it", this, ResInputTargetCreate, ResInputTargetDestroy, 0);
        ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);

        uint32_t resource_type;
        e = dmResource::GetTypeFromExtension(m_Factory, "it", &resource_type);
        ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);
        dmGameObject::ComponentType it_type;
        it_type.m_Name = "it";
        it_type.m_ResourceType = resource_type;
        it_type.m_Context = this;
        it_type.m_CreateFunction = CompInputTargetCreate;
        it_type.m_DestroyFunction = CompInputTargetDestroy;
        it_type.m_OnInputFunction = CompInputTargetOnInput;
        it_type.m_InstanceHasUserData = true;
        dmGameObject::Result result = dmGameObject::RegisterComponentType(m_Register, it_type);
        ASSERT_EQ(dmGameObject::RESULT_OK, result);
    }

    virtual void TearDown()
    {
        dmGameObject::DeleteCollection(m_Collection);
        dmResource::DeleteFactory(m_Factory);
        dmGameObject::DeleteRegister(m_Register);
        dmGameObject::Finalize();
    }

    static dmResource::CreateResult ResInputTargetCreate(dmResource::HFactory factory, void* context, const void* buffer, uint32_t buffer_size, dmResource::SResourceDescriptor* resource, const char* filename);
    static dmResource::CreateResult ResInputTargetDestroy(dmResource::HFactory factory, void* context, dmResource::SResourceDescriptor* resource);

    static dmGameObject::CreateResult CompInputTargetCreate(dmGameObject::HCollection collection,
                                                             dmGameObject::HInstance instance,
                                                             void* resource,
                                                             void* world,
                                                             void* context,
                                                             uintptr_t* user_data);
    static dmGameObject::CreateResult CompInputTargetDestroy(dmGameObject::HCollection collection,
                                                              dmGameObject::HInstance instance,
                                                              void* world,
                                                              void* context,
                                                              uintptr_t* user_data);
    static dmGameObject::InputResult CompInputTargetOnInput(dmGameObject::HInstance instance,
                                            const dmGameObject::InputAction* input_action,
                                            void* context,
                                            uintptr_t* user_data);

public:

    uint32_t m_InputCounter;

    dmGameObject::UpdateContext m_UpdateContext;
    dmGameObject::HRegister m_Register;
    dmGameObject::HCollection m_Collection;
    dmResource::HFactory m_Factory;
};

dmResource::CreateResult InputTest::ResInputTargetCreate(dmResource::HFactory factory, void* context, const void* buffer, uint32_t buffer_size, dmResource::SResourceDescriptor* resource, const char* filename)
{
    TestGameObjectDDF::InputTarget* obj;
    dmDDF::Result e = dmDDF::LoadMessage<TestGameObjectDDF::InputTarget>(buffer, buffer_size, &obj);
    if (e == dmDDF::RESULT_OK)
    {
        resource->m_Resource = (void*) obj;
        return dmResource::CREATE_RESULT_OK;
    }
    else
    {
        return dmResource::CREATE_RESULT_UNKNOWN;
    }
}

dmResource::CreateResult InputTest::ResInputTargetDestroy(dmResource::HFactory factory, void* context, dmResource::SResourceDescriptor* resource)
{
    dmDDF::FreeMessage((void*) resource->m_Resource);
    return dmResource::CREATE_RESULT_OK;
}

dmGameObject::CreateResult InputTest::CompInputTargetCreate(dmGameObject::HCollection collection,
                                                         dmGameObject::HInstance instance,
                                                         void* resource,
                                                         void* world,
                                                         void* context,
                                                         uintptr_t* user_data)
{
    return dmGameObject::CREATE_RESULT_OK;
}

dmGameObject::CreateResult InputTest::CompInputTargetDestroy(dmGameObject::HCollection collection,
                                                          dmGameObject::HInstance instance,
                                                          void* world,
                                                          void* context,
                                                          uintptr_t* user_data)
{
    return dmGameObject::CREATE_RESULT_OK;
}

dmGameObject::InputResult InputTest::CompInputTargetOnInput(dmGameObject::HInstance instance,
                                        const dmGameObject::InputAction* input_action,
                                        void* context,
                                        uintptr_t* user_data)
{
    InputTest* self = (InputTest*) context;

    if (input_action->m_ActionId == dmHashString64("test_action"))
    {
        self->m_InputCounter++;
        return dmGameObject::INPUT_RESULT_CONSUMED;
    }
    else
    {
        assert(0);
        return dmGameObject::INPUT_RESULT_UNKNOWN_ERROR;
    }
}

TEST_F(InputTest, TestComponentInput)
{
    dmGameObject::HInstance go = dmGameObject::New(m_Collection, "component_input.goc");
    ASSERT_NE((void*) 0, (void*) go);

    dmGameObject::AcquireInputFocus(m_Collection, go);

    dmGameObject::UpdateResult r;

    ASSERT_EQ(0U, m_InputCounter);

    dmGameObject::InputAction action;
    action.m_ActionId = dmHashString64("test_action");
    action.m_Value = 1.0f;
    action.m_Pressed = 1;
    action.m_Released = 0;
    action.m_Repeated = 1;

    r = dmGameObject::DispatchInput(m_Collection, &action, 1);

    ASSERT_EQ(1U, m_InputCounter);
    ASSERT_EQ(dmGameObject::UPDATE_RESULT_OK, r);

    action.m_ActionId = dmHashString64("test_action");
    action.m_Value = 0.0f;
    action.m_Pressed = 0;
    action.m_Released = 1;
    action.m_Repeated = 0;

    r = dmGameObject::DispatchInput(m_Collection, &action, 1);

    ASSERT_EQ(2U, m_InputCounter);
    ASSERT_EQ(dmGameObject::UPDATE_RESULT_OK, r);
}

TEST_F(InputTest, TestComponentInput2)
{
    dmGameObject::HInstance go = dmGameObject::New(m_Collection, "component_input2.goc");
    ASSERT_NE((void*) 0, (void*) go);

    dmGameObject::AcquireInputFocus(m_Collection, go);

    dmGameObject::InputAction action;
    action.m_ActionId = dmHashString64("test_action");
    action.m_Value = 1.0f;
    action.m_Pressed = 1;
    action.m_Released = 0;
    action.m_Repeated = 1;

    dmGameObject::UpdateResult r = dmGameObject::DispatchInput(m_Collection, &action, 1);

    ASSERT_EQ(dmGameObject::UPDATE_RESULT_OK, r);
}

TEST_F(InputTest, TestComponentInput3)
{
    dmGameObject::HInstance go = dmGameObject::New(m_Collection, "component_input3.goc");
    ASSERT_NE((void*) 0, (void*) go);

    dmGameObject::AcquireInputFocus(m_Collection, go);

    dmGameObject::InputAction action;
    action.m_ActionId = dmHashString64("test_action");
    action.m_Value = 1.0f;
    action.m_Pressed = 1;
    action.m_Released = 0;
    action.m_Repeated = 1;

    dmGameObject::UpdateResult r = dmGameObject::DispatchInput(m_Collection, &action, 1);

    ASSERT_EQ(dmGameObject::UPDATE_RESULT_UNKNOWN_ERROR, r);
}

TEST_F(InputTest, TestDeleteFocusInstance)
{
    dmGameObject::HInstance go = dmGameObject::New(m_Collection, "component_input.goc");
    ASSERT_NE((void*) 0, (void*) go);

    dmGameObject::AcquireInputFocus(m_Collection, go);

    dmGameObject::Delete(m_Collection, go);
    dmGameObject::PostUpdate(m_Collection);

    dmGameObject::UpdateResult r;

    dmGameObject::InputAction action;
    action.m_ActionId = dmHashString64("test_action");
    action.m_Value = 1.0f;
    action.m_Pressed = 1;
    action.m_Released = 0;
    action.m_Repeated = 1;

    r = dmGameObject::DispatchInput(m_Collection, &action, 1);
    ASSERT_EQ(dmGameObject::UPDATE_RESULT_OK, r);

    ASSERT_EQ(dmGameObject::UPDATE_RESULT_OK, r);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    int ret = RUN_ALL_TESTS();
    return ret;
}
